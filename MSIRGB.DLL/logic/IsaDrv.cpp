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
        // We need to lock this section so that neither MSIRGB nor the script service
        // mess up between the load_drv call and opening a handle to the driver
        op_mutex_handle = CreateMutex(NULL, false, L"Global\\MSIRGB_Driver_Load_Lock");
        // initial ownership must be set to false above, else the script service
        // might grab a handle to the mutex before the GUI tries to release it
        // and then releasing the mutex won't work, because there are two handles to it

        if (op_mutex_handle == NULL) {
            throw Exception(ErrorCode::LoadFailed);
        }

        // Wait for control of the mutex
        WaitForSingleObject(op_mutex_handle, INFINITE);
        
        // Try to load the driver
        if (!load_drv()) {
            ReleaseMutex(op_mutex_handle);
            throw Exception(ErrorCode::LoadFailed);
        }

        // We use a semaphore to keep track of how many handles to the driver service
        // are open at the same time. There can be at most two - one from the script service
        // and one from the GUI.
        SECURITY_DESCRIPTOR sd;
        
        if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
            ReleaseMutex(op_mutex_handle);
            throw Exception(ErrorCode::LoadFailed);
        }

        if (!SetSecurityDescriptorDacl(&sd, true, NULL, false)) {
            ReleaseMutex(op_mutex_handle);
            throw Exception(ErrorCode::LoadFailed);
        }

        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = false;

        drv_handle_count_sem = CreateSemaphore(&sa, 0, 2, L"Global\\MSIRGB_Driver_Counter");

        if (drv_handle_count_sem == NULL) {
            ReleaseMutex(op_mutex_handle);
            throw Exception(ErrorCode::LoadFailed);
        }

        drv_handle = CreateFile((L"\\\\.\\"s + DRV_DEVICE_NAME).c_str(),
                                GENERIC_READ | GENERIC_WRITE,
                                0,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

        if (drv_handle == INVALID_HANDLE_VALUE) {
            ReleaseMutex(op_mutex_handle);
            unload_drv();
            throw Exception(ErrorCode::LoadFailed);
        }

        // Increase the count of handles open to the driver service by 1
        ReleaseSemaphore(drv_handle_count_sem, 1, NULL);

        ReleaseMutex(op_mutex_handle);
    }

    IsaDrv::~IsaDrv()
    {
        // We also need to lock this section. The script service could be deleting
        // the service while the GUI was trying to start it again.
        WaitForSingleObject(op_mutex_handle, INFINITE);

        CloseHandle(drv_handle);

        // If there are no more handles to the driver service, unload it
        // WaitForSingleObject decrements the count already when called
        if (WaitForSingleObject(drv_handle_count_sem, 0) == WAIT_TIMEOUT) {
            unload_drv();
        }
        
        CloseHandle(drv_handle_count_sem);

        ReleaseMutex(op_mutex_handle);

        CloseHandle(op_mutex_handle);
    }

    std::uint8_t IsaDrv::inb(std::uint8_t port) const
    {
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

        return output_buf.data;
    }

    void IsaDrv::outb(std::uint8_t port, std::uint8_t data) const
    {
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