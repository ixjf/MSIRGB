#include "pch.h"
#include "IsaDrv.h"

using namespace std::string_literals;

const auto DRV_FILE_NAME = L"inpoutx64.sys";
const auto DRV_DEVICE_NAME = L"inpoutx64";
const auto DRV_SVC_NAME = L"MSIRGB";

const auto IOCTL_READ_PORT_UCHAR  = CTL_CODE(0x9C40, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS);
const auto IOCTL_WRITE_PORT_UCHAR = CTL_CODE(0x9C40, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS);

IsaDrv::IsaDrv()
{
    if (!load_drv()) {
        throw Exception("Internal error: Driver failed to load");
    }

    drv_handle = CreateFile((L"\\\\.\\"s + DRV_DEVICE_NAME).c_str(),
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);

    if (drv_handle == INVALID_HANDLE_VALUE) {
        unload_drv();
        throw Exception("Internal error: Failed to open driver I/O");
    }
}

IsaDrv::~IsaDrv()
{
    CloseHandle(drv_handle);
    unload_drv();
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
        std::cout << __FUNCTION__ << " DeviceIoCtrl failed with error " << GetLastError() << std::endl;
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
        std::cout << __FUNCTION__ << " DeviceIoCtrl failed with error " << GetLastError() << std::endl;
    }
}

DWORD IsaDrv::install_drv()
{
    // Extract driver from EXE
    HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(RSRC_DRIVER), RT_RCDATA);
    HGLOBAL rsrc_data = LoadResource(NULL, resource);
    void *raw_data = LockResource(rsrc_data);
    DWORD raw_data_len = SizeofResource(NULL, resource);

    auto drv_filepath = std::filesystem::current_path().concat(L"\\"s + DRV_FILE_NAME);

    auto file = std::ofstream(drv_filepath, std::ofstream::out | 
                                            std::ofstream::binary | 
                                            std::ofstream::trunc);

    file.write(reinterpret_cast<const char *>(raw_data), raw_data_len);
    file.flush();
    file.close();

    // Create service for driver
    SC_HANDLE sc_manager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

    SC_HANDLE service = CreateService(sc_manager, 
                                      DRV_SVC_NAME, 
                                      DRV_SVC_NAME, 
                                      SERVICE_START | SERVICE_STOP | DELETE,
                                      SERVICE_KERNEL_DRIVER,
                                      SERVICE_DEMAND_START,
                                      SERVICE_ERROR_NORMAL,
                                      drv_filepath.native().c_str(),
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
            std::cout << __FUNCTION__ << " CreateService failed with error " << err << std::endl;
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
        std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

        CloseServiceHandle(sc_manager);
        return err;
    }

    if (!DeleteService(service)) {
        DWORD err = GetLastError();

        if (err == ERROR_SERVICE_DOES_NOT_EXIST) {
            err = ERROR_SUCCESS;
        }
        else {
            std::cout << __FUNCTION__ << " DeleteService failed with error " << err << std::endl;
        }

        CloseServiceHandle(service);
        CloseServiceHandle(sc_manager);
        return err;
    }
    else {
        CloseServiceHandle(service);
        CloseServiceHandle(sc_manager);

        // Delete driver file
        auto drv_filepath = std::filesystem::current_path().concat(L"\\"s + DRV_FILE_NAME);
        
        if (!std::filesystem::remove(drv_filepath)) {
            std::cout << __FUNCTION__ << " std::filesystem::remove failed" << std::endl;
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
        std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

        CloseServiceHandle(sc_manager);
        return err;
    }

    if (!StartService(service, 0, NULL)) {
        DWORD err = GetLastError();

        if (err == ERROR_SERVICE_ALREADY_RUNNING) {
            err = ERROR_SUCCESS;
        }
        else {
            std::cout << __FUNCTION__ << " StartService failed with error " << err << std::endl;
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
        std::cout << __FUNCTION__ << " OpenService failed with error " << err << std::endl;

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
            std::cout << __FUNCTION__ << " ControlService failed with error " << err << std::endl;
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
    return stop_drv() == ERROR_SUCCESS &&
           uninstall_drv() == ERROR_SUCCESS;
}