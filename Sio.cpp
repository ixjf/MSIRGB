#include "pch.h"
#include "Sio.h"
#include "wmi_helper.h"

const auto INDEX_REG    = 0x4E; // All Mystic Light-supported MBs's chips are found at this port
const auto DATA_REG     = 0x4F; // INDEX_REG+1
const auto RGB_LED_BANK = 0x12;

// This should support boards that don't have a separate LED control chip (like Renesas).
// If the MB uses the integrated NCT controller chip for LEDs, this should
// work, and control both MB and header LEDs at the same time.
// This also only works if the MB has RGB headers, since MBs without RGB headers
// control backlight, etc LEDs differently (although still through NCT).
//
// For now, this will only work without --ignore-check for MBs compatible with
// Mystic Light. There are other boards which this may be compatible with, though.
//

Sio::Sio(bool ignore_mb_check)
{
    try {
        drv.reset(new IsaDrv);
    }
    catch (IsaDrv::Exception &e) {
        throw Exception(e.what());
    }

    if (!ignore_mb_check && !has_supported_mb()) {
        throw Exception("Motherboard currently not supported");
    }

    // Enable flashing mode support
    // Sets bit 5 and set bit 4 to 0
    // All other bits seem to do nothing, but they MAY have a purpose on certain boards
    std::uint8_t val_at_2c = chip_read_cell_from_bank(0x09, 0x2C);
    chip_write_cell_to_bank(0x09, 0x2C, (val_at_2c & 0b11100111) | 0b10000);

    // Enable RGB control
    // E0 = 0b11100000 (these 3 bits enable RGB, the remaining 5 have unknown purpose)
    std::uint8_t val_at_e0 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE0);
    chip_write_cell_to_bank(RGB_LED_BANK, 0xE0, val_at_e0 | 0b11100000);

    // 0xFD in bank 12h seems to be related to some rainbow mode
    // but it is apparently not supported on my board, so I can't test it.
    // I think it's only supported on other MBs that have RGB headers but are
    // different somehow - those function differently from the MBs supported
    // here.

    // Turn on header
    std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);
    chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, val_at_ff | 0b00000010);
}

void Sio::set_led_disabled() const
{
    // LEDs are disabled by setting flashing bits to 001 & breathing_mode to 0
    // I.e. flashing and breathing_mode are disabled

    std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);
    chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, val_at_e4 & 0b11110000 | 0b1);
}

void Sio::set_colour(ColourIndex index, std::uint32_t colour) const
{
    if (colour > COLOUR_MAX_VALUE) return; // PANIC!

    //
    //
    // F0 RR RR RR RR GG GG GG GG BB BB BB BB
    // ---------------------------------------------------
    //    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
    //
    //
    // F0-F3: each byte corresponds to the red component of colours 1-4, respectively
    // F4-F7: each byte corresponds to the green component of colours 1-4, respectively
    // F8-FB: each byte corresponds to the blue component of colours 1-4, respectively

    chip_write_cell_to_bank(RGB_LED_BANK, 0xF0 + (static_cast<std::uint8_t>(index) - 1), (std::uint8_t)(colour >> 16)); // Red
    chip_write_cell_to_bank(RGB_LED_BANK, 0xF4 + (static_cast<std::uint8_t>(index) - 1), (std::uint8_t)(colour >> 8)); // Green
    chip_write_cell_to_bank(RGB_LED_BANK, 0xF8 + (static_cast<std::uint8_t>(index) - 1), (std::uint8_t)(colour)); // Blue
}

void Sio::set_channel_inverted(Channel c, bool enable) const
{
    std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);
    // Invert value is second group of 3 bits from the left in 0xFF: BGR, in this order, from leftmost bit to rightmost bit
    // If bit is set, channel is inverted

    switch(c)
    {
    case Channel::R:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff | 0b00000100) : (val_at_ff & 0b11111011));

        break;

    case Channel::G:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff | 0b1000) : (val_at_ff & 0b11110111));

        break;

    case Channel::B:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff | 0b10000) : (val_at_ff & 0b11101111));

        break;
    }
}

void Sio::set_breathing_mode_enabled(bool enable) const
{
    // Breathing mode is enabled by setting bit 5 (counting from the left) in 0xE4

    if (enable)
    {
        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, val_at_e4 | 0b00001000);
    }
    else
    {
        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, val_at_e4 & 0b11110111);
    }
}

void Sio::set_step_duration(std::uint16_t step_duration) const
{
    if (step_duration > STEP_DURATION_MAX_VALUE) return; // TODO: PANIC!

    //
    //
    // F0                                           XX X
    // ---------------------------------------------------
    //    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
    //
    //
    // StepDuration is spread across FE and FF
    // FE contains first byte
    // FF contains the other byte (which has at most one bit set)
    // and other settings

    chip_write_cell_to_bank(RGB_LED_BANK, 0xFE, step_duration & 0x00FF);

    std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);
    chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, val_at_ff | ((step_duration >> 8) & 0b1));
}

void Sio::set_flash_speed(std::uint8_t flash_speed) const
{
    if (flash_speed > FLASHING_MAX_VALUE) return; // TODO: PANIC!

    // Flashing can be controlled through cell E4
    // 3 rightmost bits define flashing speed
    // 001 = disabled
    // 000 = always on
    // any other value = flashing (different speed)

    std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);

    chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, flash_speed == 0 ? (val_at_e4 & 0b11111000) : (val_at_e4 | (flash_speed + 1)));
}

void Sio::set_fade_in_enabled(Channel c, bool enable) const
{
    std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);
    // Fade in value is first 3 bits: BGR, in this order, from leftmost bit to rightmost bit
    // If bit is set, fade in is disabled

    switch(c)
    {
    case Channel::R:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff & 0b11011111) : (val_at_ff | 0b00100000));

        break;

    case Channel::G:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff & 0b10111111) : (val_at_ff | 0b01000000));

        break;

    case Channel::B:
        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, enable ? (val_at_ff & 0b01111111) : (val_at_ff | 0b10000000));

        break;
    }
}

bool Sio::has_supported_mb()
{
    try {
        auto info = wmi_query(L"Win32_BaseBoard", {L"Manufacturer", L"Product", L"Version"});

        if (info[L"Manufacturer"] != L"Micro-Star International Co., Ltd.") {
            return false;
        }

        static const std::list<std::wstring> supported_mbs = {
            L"7A40",
            L"7A39",
            L"7A38",
            L"7B78",
            L"7B77",
            L"7B79",
            L"7B73",
            L"7B61",
            L"7B54",
            L"7B49",
            L"7B48",
            L"7B45",
            L"7B44",
            L"7A59",
            L"7A57",
            L"7A68",
            L"7B40",
            L"7A94",
            L"7B09"
        };

        auto found = std::find_if(supported_mbs.begin(), 
                                  supported_mbs.end(),
                                  [&info](const std::wstring &mb_model) -> bool {
                                    return info[L"Product"].find(mb_model) != std::wstring::npos;
                                  });

        if (found == supported_mbs.end()) {
            return false;
        }

        if (info[L"Product"].find(L"7A38") != std::wstring::npos &&
            info[L"Version"].find(L"3.") == std::wstring::npos &&
            info[L"Version"].find(L"4.") == std::wstring::npos) {
            // MB model 7A38 but not revision 3.x or 4.x
            return false;
        }
        else if (info[L"Product"].find(L"7B79") != std::wstring::npos &&
                 info[L"Version"].find(L"1.") == std::wstring::npos &&
                 info[L"Version"].find(L"2.") == std::wstring::npos) {
            // MB model 7B79 but not revision 1.x or 2.x
            return false;
        }
        else {
            return true;
        }
    }
    catch (std::runtime_error &e) {
        std::cout << __FUNCTION__ << " " << e.what() << std::endl;
        return false;
    }
}

// Should work on all NCT chips
void Sio::chip_enter_extended_function_mode() const
{
    drv->outb(INDEX_REG, 0x87);
    drv->outb(INDEX_REG, 0x87);
}

void Sio::chip_exit_extended_function_mode() const
{
    drv->outb(INDEX_REG, 0xAA);
}

void Sio::chip_enter_bank(std::uint8_t bank) const
{
    drv->outb(INDEX_REG, 0x07);
    drv->outb(DATA_REG, bank);
}

std::uint8_t Sio::chip_read(std::uint8_t index) const
{
    drv->outb(INDEX_REG, index);
    return drv->inb(DATA_REG);
}

void Sio::chip_write(std::uint8_t index, std::uint8_t data) const
{
    drv->outb(INDEX_REG, index);
    drv->outb(DATA_REG, data);
}

std::uint8_t Sio::chip_read_cell_from_bank(std::uint8_t bank, std::uint8_t index) const
{
    chip_enter_extended_function_mode();

    chip_enter_bank(bank);

    std::uint8_t ret = chip_read(index);

    chip_exit_extended_function_mode();

    return ret;
}

void Sio::chip_write_cell_to_bank(std::uint8_t bank, std::uint8_t index, std::uint8_t data) const
{
    chip_enter_extended_function_mode();

    chip_enter_bank(bank);

    chip_write(index, data);

    chip_exit_extended_function_mode();
}

std::uint8_t Sio::chip_read_cell_from_global(std::uint8_t index) const
{
    chip_enter_extended_function_mode();

    std::uint8_t ret = chip_read(index);

    chip_exit_extended_function_mode();

    return ret;
}

void Sio::chip_write_cell_to_global(std::uint8_t index, std::uint8_t data) const
{
    chip_enter_extended_function_mode();

    chip_write(index, data);

    chip_exit_extended_function_mode();
}