#pragma once

class IsaDrv {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const char *msg) : std::runtime_error(msg)
        {
        
        }
    };

    IsaDrv();
   ~IsaDrv();

    std::uint8_t    inb             (std::uint8_t port) const;
    void            outb            (std::uint8_t port, std::uint8_t data) const;

private:
    struct IoctlInputBuffer {
        std::uint16_t port;
        std::uint8_t  data;
    };

    struct IoctlOutputBuffer {
        std::uint8_t  data;
    };

    static DWORD    install_drv     ();
    static DWORD    uninstall_drv   ();
    static DWORD    start_drv       ();
    static DWORD    stop_drv        ();

    static bool     load_drv        ();
    static bool     unload_drv      ();

    HANDLE          drv_handle;
};