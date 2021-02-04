#pragma once

#include "pch.h"
#include "IsaDrv.h"

namespace logic {
    class Sio {
    public:
        Sio();
        ~Sio() = default;

        // LEDs are enabled in every 'set' call because changing some settings (breathing or flashing)
        // inevitably turns them on. I decided to explicitly enable them on every call for consistency

        // Disabling the LEDs overwrites flashing and breathing modes. Those must be reset after enabling the LEDs again

        // Should work on all NCT chips - all supported MBs have NCT chips as well
        std::uint8_t            read_uint8_from_bank(std::uint8_t bank, std::uint8_t index) const;
        void                    write_uint8_to_bank(std::uint8_t bank, std::uint8_t index, std::uint8_t data) const;

        void                    debug_dump_bank(std::uint8_t bank) const;

    private:
        void                    enter_extended_function_mode() const;
        void                    exit_extended_function_mode() const;
        void                    enter_bank(std::uint8_t bank) const;
        std::uint8_t            read_uint8(std::uint8_t index) const;
        void                    write_uint8(std::uint8_t index, std::uint8_t data) const;

        std::unique_ptr<IsaDrv> drv;
    };
}