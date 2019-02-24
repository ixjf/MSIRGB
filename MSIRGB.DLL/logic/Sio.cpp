#include "pch.h"
#include "Sio.h"

namespace logic {
    Sio::Sio()
        : drv(new IsaDrv)
    {
    }

    std::uint8_t Sio::read_uint8_from_bank(std::uint8_t bank, std::uint8_t index) const
    {
        enter_extended_function_mode();

        enter_bank(bank);

        std::uint8_t ret = read_uint8(index);

        exit_extended_function_mode();

        return ret;
    }

    void Sio::write_uint8_to_bank(std::uint8_t bank, std::uint8_t index, std::uint8_t data) const
    {
        enter_extended_function_mode();

        enter_bank(bank);

        write_uint8(index, data);

        exit_extended_function_mode();
    }
    
    
    void Sio::enter_extended_function_mode() const
    {
        drv->outb(0x4E, 0x87);
        drv->outb(0x4E, 0x87);
    }

    void Sio::exit_extended_function_mode() const
    {
        drv->outb(0x4E, 0xAA);
    }

    void Sio::enter_bank(std::uint8_t bank) const
    {
        drv->outb(0x4E, 0x07);
        drv->outb(0x4F, bank);
    }

    std::uint8_t Sio::read_uint8(std::uint8_t index) const
    {
        drv->outb(0x4E, index);
        return drv->inb(0x4F);
    }

    void Sio::write_uint8(std::uint8_t index, std::uint8_t data) const
    {
        drv->outb(0x4E, index);
        drv->outb(0x4F, data);
    }
}
