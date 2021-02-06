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

    void Sio::debug_dump_bank(std::uint8_t bank) const
    {
        std::ostringstream filename;
        filename << "msirgb_dump_bank_" << std::hex << static_cast<int>(bank) << "h_" << ".txt";

        std::fstream f(
            std::filesystem::temp_directory_path() / filename.str(), std::ios_base::out | std::ios_base::trunc);

        f << "   | 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f" << std::endl;

        for (uint8_t i = 0x0; i <= 0xF; i++) {
            f << std::hex << static_cast<int>(i) << "0 | ";

            for (uint8_t j = 0x0; j <= 0xF; j++) {
                enter_extended_function_mode();
                enter_bank(bank);
                    
                std::uint8_t val_at_ij = read_uint8((i << 4) + j);
                    
                exit_extended_function_mode();
                    
                f << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(val_at_ij) << " ";
            }

            f << std::endl;
        }
    }
}
