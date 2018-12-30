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

// TODO: Change std::couts to file log?
namespace logic {
    Sio::Sio(bool ignore_mb_check)
    {
        // Check MB support
        if (!ignore_mb_check && !has_supported_mb()) {
            throw Exception(ErrorCode::MotherboardNotSupported);
        }
        
        // Attempt to load driver
        try {
            drv.reset(new IsaDrv);
        }
        catch (IsaDrv::Exception &) {
            throw Exception(ErrorCode::DriverLoadFailed);
        }

        // Set chip LED config

        // Enable RGB control
        // Sets bit 5 and set bit 4 to 0
        // All other bits seem to do nothing, but they MAY have a purpose on certain boards
        std::uint8_t val_at_2c = chip_read_cell_from_bank(0x09, 0x2C);
        chip_write_cell_to_bank(0x09, 0x2C, (val_at_2c & 0b11100111) | 0b10000);

        // E0 = 0b11100000 (these 3 bits enable RGB, the remaining 5 have unknown purpose)
        std::uint8_t val_at_e0 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE0);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xE0, val_at_e0 | 0b11100000);

        // 0xFD in bank 12h seems to be related to some rainbow mode
        // but it is apparently not supported on my board, so I can't test it.
        // I think it's only supported on other MBs that have RGB headers but are
        // different somehow - those function differently from the MBs supported
        // here.

        // Make sure RGB channels are not inverted
        // Also make sure some 'weird' fade in behaviour that happens in some boards is disabled
        std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);

        // Invert value is second group of 3 bits from the left in 0xFF: BGR, in this order, from leftmost bit to rightmost bit
        // If bit is set, channel is inverted
        val_at_ff &= 0b11100011;

        // Fade in value is first 3 bits: BGR, in this order, from leftmost bit to rightmost bit
        // If bit is set, fade in is disabled
        val_at_ff |= 0b11100000;

        chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, val_at_ff);
    }

    // By enabling the LEDs, flashing mode is disabled
    // If flashing mode is set, the function does nothing as to avoid overwriting settings
    // Disabling the LEDs overwrites breathing and flashing modes
    void Sio::set_led_enabled(bool enable) const
    {
        if (enable) {
            // Turn on header
            std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);
            chip_write_cell_to_bank(RGB_LED_BANK, 0xFF, val_at_ff | 0b00000010);

            // Enabling the LEDs involves setting flashing bits to 000
            std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);

            // If flashing mode is set (LEDs are not disabled), do not overwrite
            if ((val_at_e4 & 0b111) != 0b001) {
                return;
            }

            chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, val_at_e4 & 0b11111000);
        }
        else {
            // Disabling the LEDs involves disabling breathing and flashing modes
            // So you need to reset them after you disable the LEDs
            std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);
            chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, val_at_e4 & 0b11110000 | 0b1);
        }
    }

    void Sio::set_colour(ColourIndex index, Colour colour) const
    {
        set_led_enabled(true);

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

        chip_write_cell_to_bank(RGB_LED_BANK, 0xF0 + (static_cast<std::uint8_t>(index) - 1), colour.r);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xF4 + (static_cast<std::uint8_t>(index) - 1), colour.g);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xF8 + (static_cast<std::uint8_t>(index) - 1), colour.b);
    }

    Sio::Colour Sio::get_colour(ColourIndex index) const
    {
        // See Sio::set_colour

        std::uint8_t r = chip_read_cell_from_bank(RGB_LED_BANK, 0xF0 + (static_cast<std::uint8_t>(index) - 1));
        std::uint8_t g = chip_read_cell_from_bank(RGB_LED_BANK, 0xF4 + (static_cast<std::uint8_t>(index) - 1));
        std::uint8_t b = chip_read_cell_from_bank(RGB_LED_BANK, 0xF8 + (static_cast<std::uint8_t>(index) - 1));

        return Colour(r, g, b);
    }

    // Breathing mode is only enabled if flashing mode is not enabled
    // Flashing mode overrides breathing mode, so if flashing is enabled, don't even try to enable breathing mode
    bool Sio::set_breathing_mode_enabled(bool enable) const
    {
        if (get_flash_speed() != FlashingSpeed::Disabled)
            return false;

        set_led_enabled(true);

        // Breathing mode is enabled by setting bit 5 (counting from the left) in 0xE4

        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);
        chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, enable ? (val_at_e4 | 0b00001000) : (val_at_e4 & 0b11110111));

        return true;
    }

    bool Sio::is_breathing_mode_enabled() const
    {
        // See Sio::set_breathing_mode_enabled

        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);

        return (val_at_e4 & 0b00001000) != 0b0;
    }

    // TODO: Step duration switches between specified colour and white??? Also it seems if more than two colours are set, it'll _randomly_ switch
    // between them
    void Sio::set_step_duration(std::uint16_t step_duration) const
    {
        if (step_duration > STEP_DURATION_MAX_VALUE) throw std::invalid_argument("step_duration"); // PANIC!

        set_led_enabled(true);

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

    std::uint16_t Sio::get_step_duration() const
    {
        // See Sio::set_step_duration

        std::uint8_t val_at_fe = chip_read_cell_from_bank(RGB_LED_BANK, 0xFE);

        std::uint8_t val_at_ff = chip_read_cell_from_bank(RGB_LED_BANK, 0xFF);

        return (std::uint16_t)(((val_at_ff & 0b1) << 8) | val_at_fe);
    }

    // Sets the flashing speed only if LEDs are enabled
    // Disables breathing mode if flashing is enabled because otherwise it'll just override breathing anyway
    // (at least then we know it's not enabled)
    void Sio::set_flash_speed(FlashingSpeed flash_speed) const
    {
        if (flash_speed != FlashingSpeed::Disabled && is_breathing_mode_enabled()) {
            set_breathing_mode_enabled(false);
        }

        // Flashing can be controlled through cell E4
        // 3 rightmost bits define flashing speed
        // 001 = LEDs disabled
        // 000 = always on
        // any other value = flashing (different speed)

        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);

        // If LEDs are disabled, do not do anything, else it will turn on the LEDs
        if ((val_at_e4 & 0b111) == 0b001) {
            return;
        }

        set_led_enabled(true);

        chip_write_cell_to_bank(RGB_LED_BANK, 0xE4, (flash_speed == FlashingSpeed::Disabled ? (val_at_e4 & 0b11111000) : (val_at_e4 & 0b000 | (static_cast<std::uint8_t>(flash_speed) + 1))));
    }

    Sio::FlashingSpeed Sio::get_flash_speed() const
    {
        // See Sio::set_flash_speed

        std::uint8_t val_at_e4 = chip_read_cell_from_bank(RGB_LED_BANK, 0xE4);

        std::uint8_t flash_speed = val_at_e4 & 0b00000111;

        return (flash_speed == 0 || flash_speed == 1) ? FlashingSpeed::Disabled : static_cast<FlashingSpeed>(flash_speed - 1);
    }

    bool Sio::has_supported_mb()
    {
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
        else if (info[L"Product"].find(L"7A38") != std::wstring::npos &&
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

    // Should work on all 6 NCT chips (NCT6797(D), NCT6795(D), NCT5565, NCT5567) of the supported motherboards
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
}
