#pragma once

#include "pch.h"

namespace logic {
    class IsaDrv {
    public:
        enum class ErrorCode {
            LoadFailed,
        };

        class Exception : public std::runtime_error {
        public:
            Exception(ErrorCode ec) : std::runtime_error(""), ec(ec)
            {

            }

            ErrorCode error_code()
            {
                return ec;
            }

        private:
            ErrorCode ec;
        };

        IsaDrv();
        ~IsaDrv();

        std::uint8_t    inb(std::uint8_t port) const;
        void            outb(std::uint8_t port, std::uint8_t data) const;

    private:
        struct IoctlInputBuffer {
            std::uint16_t port;
            std::uint8_t  data;
        };

        struct IoctlOutputBuffer {
            std::uint8_t  data;
        };

        void            inc_driver_instance_counter() const;
        bool            dec_driver_instance_counter() const;

        void            create_open_driver_instance_counter();

        DWORD           install_drv();
        DWORD           uninstall_drv();
        DWORD           start_drv();
        DWORD           stop_drv();

        bool            load_drv();
        bool            unload_drv();

        HANDLE          drv_handle;
        HANDLE          drv_handle_count_sem;
    };
}