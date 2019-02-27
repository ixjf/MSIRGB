#include "pch.h"
#include "IsaDrv.h"
#include "module_helper.h"

using namespace std::string_literals;

const auto DRV_FILE_NAME = L"inpoutx64.sys";
const auto DRV_DEVICE_NAME = L"inpoutx64";
const auto DRV_SVC_NAME = L"inpoutx64";

const auto IOCTL_READ_PORT_UCHAR  = CTL_CODE(0x9C40, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS);
const auto IOCTL_WRITE_PORT_UCHAR = CTL_CODE(0x9C40, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS);

namespace logic {
    IsaDrv::IsaDrv()
    {
        op_mutex_handle = CreateMutex(NULL, false, L"Global\\MSIRGB_Driver_Mutex");

        if (op_mutex_handle == NULL) {
            throw Exception(ErrorCode::LoadFailed);
        }

        enter_critical_section();
        
        // Try to load the driver
        if (!load_drv()) {
            leave_critical_section();
            throw Exception(ErrorCode::LoadFailed);
        }

        create_open_driver_instance_counter();

        drv_handle = CreateFile((L"\\\\.\\"s + DRV_DEVICE_NAME).c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (drv_handle == INVALID_HANDLE_VALUE) {
            leave_critical_section();
            throw Exception(ErrorCode::LoadFailed);
        }

        inc_driver_instance_counter();

        leave_critical_section();
    }

    IsaDrv::~IsaDrv()
    {
        enter_critical_section();

        CloseHandle(drv_handle);

        if (dec_driver_instance_counter()) {
            unload_drv();
        }
        
        CloseHandle(drv_handle_count_sem);

        leave_critical_section();

        CloseHandle(op_mutex_handle);
    }

    void IsaDrv::enter_critical_section() const
    {
        WaitForSingleObject(op_mutex_handle, INFINITE);
    }

    void IsaDrv::leave_critical_section() const
    {
        ReleaseMutex(op_mutex_handle);
    }

    void IsaDrv::inc_driver_instance_counter() const
    {
        ReleaseSemaphore(drv_handle_count_sem, 1, NULL);
    }

    bool IsaDrv::dec_driver_instance_counter() const
    {
        return WaitForSingleObject(drv_handle_count_sem, 0) == WAIT_TIMEOUT; // returns true if this was the last instance
    }

    void IsaDrv::create_open_driver_instance_counter()
    {
        // We use a semaphore to keep track of how many handles to the driver service
        // are open at the same time. There can be at most two - one from the script service
        // and one from the GUI.
        SECURITY_DESCRIPTOR sd;
        
        if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
            throw Exception(ErrorCode::LoadFailed);
        }

        if (!SetSecurityDescriptorDacl(&sd, true, NULL, false)) {
            leave_critical_section();
            throw Exception(ErrorCode::LoadFailed);
        }

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = false;

        drv_handle_count_sem = CreateSemaphore(&sa, 0, 2, L"Global\\MSIRGB_Driver_Instance_Counter");

        if (drv_handle_count_sem == NULL) {
            leave_critical_section();
            throw Exception(ErrorCode::LoadFailed);
        }
    }

    std::uint8_t IsaDrv::inb(std::uint8_t port) const
    {
        enter_critical_section();

        IoctlInputBuffer input_buf;
        input_buf.port = static_cast<std::uint16_t>(port);
        input_buf.data = 0;

        IoctlOutputBuffer output_buf {0};

        if (!DeviceIoControl(drv_handle,
                             IOCTL_READ_PORT_UCHAR,
                             static_cast<LPVOID>(&input_buf),
                             sizeof(IoctlInputBuffer),
                             static_cast<LPVOID>(&output_buf),
                             sizeof(IoctlOutputBuffer),
                             NULL,
                             NULL)) {
            //std::cout << __FUNCTION__ << " DeviceIoCtrl failed with error " << GetLastError() << std::endl;
        }

        leave_critical_section();

        return output_buf.data;
    }

    void IsaDrv::outb(std::uint8_t port, std::uint8_t data) const
    {
        enter_critical_section();

        IoctlInputBuffer input_buf {0};
        input_buf.port = static_cast<std::uint16_t>(port);
        input_buf.data = data;

        if (!DeviceIoControl(drv_handle,
                             IOCTL_WRITE_PORT_UCHAR,
                             static_cast<LPVOID>(&input_buf),
                             sizeof(IoctlInputBuffer),
                             NULL,
                             0,
                             NULL,
                             NULL)) {
            //std::cout << __FUNCTION__ << " DeviceIoCtrl failed with error " << GetLastError() << std::endl;
        }

        leave_critical_section();
    }

    DWORD IsaDrv::install_drv()
    {
        // Extract driver from EXE
        HMODULE mod = get_current_hmodule();
        HRSRC resource = FindResource(mod, MAKEINTRESOURCE(RSRC_DRIVER), RT_RCDATA);
        HGLOBAL rsrc_data = LoadResource(mod, resource);
        void *raw_data = LockResource(rsrc_data);

        // Save it to System32\Drivers\<driver_name>.sys
        PWSTR sysPath;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &sysPath);

        auto drv_filepath = std::wstring(sysPath).append(L"\\drivers\\"s + DRV_FILE_NAME);

        CoTaskMemFree(sysPath);

        auto file = std::ofstream(drv_filepath, std::ofstream::out | 
                                                std::ofstream::binary | 
                                                std::ofstream::trunc);

        file.write(reinterpret_cast<const char *>(raw_data), SizeofResource(mod, resource));
        file.flush();
        file.close();

        // Create service for driver
        SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

        SC_HANDLE service = CreateService(sc_manager, 
                                          DRV_SVC_NAME, 
                                          DRV_SVC_NAME, 
                                          SERVICE_START | SERVICE_STOP,
                                          SERVICE_KERNEL_DRIVER,
                                          SERVICE_DEMAND_START,
                                          SERVICE_ERROR_NORMAL,
                                          drv_filepath.c_str(),
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL,
                                          NULL);

        if (service == NULL) {
            DWORD err = GetLastError();

            if (err == ERROR_SERVICE_EXISTS) {
                err = ERROR_SUCCESS;
            }
            else {
                //std::cout << __FUNCTION__ << " CreateService failed with error " << err << std::endl;
            }

            CloseServiceHandle(sc_manager);
            return err;
        }
        else {
            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return ERROR_SUCCESS;
        }
    }

    DWORD IsaDrv::uninstall_drv()
    {
        SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

        SC_HANDLE service = OpenService(sc_manager, DRV_SVC_NAME, DELETE);

        if (service == NULL) {
            DWORD err = GetLastError();
            //std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

            CloseServiceHandle(sc_manager);
            return err;
        }

        if (!DeleteService(service)) {
            DWORD err = GetLastError();

            if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
                err = ERROR_SUCCESS;
            }
            else {
                //std::cout << __FUNCTION__ << " DeleteService failed with error " << err << std::endl;
            }

            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return err;
        }
        else {
            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);

            // Delete driver file
            PWSTR sysPath;
            HRESULT hr = SHGetKnownFolderPath(FOLDERID_System, 0, NULL, &sysPath);

            auto drv_filepath = std::wstring(sysPath).append(L"\\Drivers\\"s + DRV_FILE_NAME);

            CoTaskMemFree(sysPath);
        
            if (!DeleteFile(drv_filepath.c_str())) {
                //std::cout << __FUNCTION__ << " std::filesystem::remove failed" << std::endl;
            }

            return ERROR_SUCCESS;
        }
    }

    DWORD IsaDrv::start_drv()
    {
        SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

        SC_HANDLE service = OpenService(sc_manager, DRV_SVC_NAME, SERVICE_START);

        if (service == NULL) {
            DWORD err = GetLastError();
            //std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

            CloseServiceHandle(sc_manager);
            return err;
        }

        if (!StartService(service, 0, NULL)) {
            DWORD err = GetLastError();

            if (err == ERROR_SERVICE_ALREADY_RUNNING) {
                err = ERROR_SUCCESS;
            }
            else {
                //std::cout << __FUNCTION__ << " StartService failed with error " << err << std::endl;
            }

            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return err;
        }
        else {
            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return ERROR_SUCCESS;
        }
    }

    DWORD IsaDrv::stop_drv()
    {
        SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

        SC_HANDLE service = OpenService(sc_manager, DRV_SVC_NAME, SERVICE_STOP);

        if (service == NULL) {
            DWORD err = GetLastError();
            //std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

            CloseServiceHandle(sc_manager);
            return err;
        }

        SERVICE_STATUS _;
        if (!ControlService(service, SERVICE_CONTROL_STOP, &_)) {
            DWORD err = GetLastError();

            if (err == ERROR_SERVICE_NOT_ACTIVE) {
                err = ERROR_SUCCESS;
            }
            else {
                //std::cout << __FUNCTION__ << " ControlService failed with error " << err << std::endl;
            }

            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return err;
        }
        else {
            CloseServiceHandle(service);
            CloseServiceHandle(sc_manager);
            return ERROR_SUCCESS;
        }
    }

    bool IsaDrv::load_drv()
    {
        if (install_drv() != ERROR_SUCCESS) {
            return false;
        }
        else {
            if (start_drv() != ERROR_SUCCESS) {
                uninstall_drv();
                return false;
            }
            else {
                return true;
            }
        }
    }

    bool IsaDrv::unload_drv()
    {
        return stop_drv() == ERROR_SUCCESS && uninstall_drv() == ERROR_SUCCESS;
    }
}