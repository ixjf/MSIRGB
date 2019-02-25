#include "pch.h"
#include "Lighting.h"
#include "Sio.h"
#include "wmi_helper.h"
#include "math_helper.h"

const std::uint8_t INDEX_REG    = 0x4E; // All ML-supported chips are found at this port
const std::uint8_t DATA_REG     = 0x4F; // INDEX_REG+1
const std::uint8_t RGB_BANK     = 0x12;
const std::uint8_t UNKNOWN_BANK = 0x09;

// This should support boards that don't have a separate LED control chip (like Renesas').
// If the MB uses the integrated NCT controller chip for LEDs, this should
// work, and control both MB and header LEDs at the same time.
// This also only works for specific boards, not all function the same way
// even if they use similar chips.

namespace logic {
    Lighting::Lighting(bool ignore_mb_check)
        : batch_calls(false), curr_batch(Batch {})
    {
        // Check MB support
        if (!ignore_mb_check && !has_supported_mb()) {
            throw Exception(ErrorCode::MotherboardNotSupported);
        }

        // Load SIO driver
        try {
            sio.reset(new Sio);
        }
        catch (IsaDrv::Exception &) {
            throw Exception(ErrorCode::DriverLoadFailed);
        }
    }

    bool Lighting::batch_begin()
    {
        if (batch_calls) {
            return false;
        }

        batch_calls = true;

        return true;
    }

    bool Lighting::batch_end()
    {
        if (batch_calls) {
            batch_commit();
            return true;
        }

        return false;
    }

    void Lighting::set_led_enabled(bool enable)
    {
        curr_batch.enabled = enable;

        if (!batch_calls) {
            batch_commit();
        }
    }

    static inline std::tuple<std::uint8_t, std::uint8_t> colour_cell_pos_from_index(std::uint8_t index)
    {
        // The nibble no. from 1 to 8 in order from 0xF0 to 0xF3, 0xF3 to 0xF7, 0xF8 to 0xFB
        // So colour 1 = nibble 2
        //    colour 2 = nibble 1
        //    colour 3 = nibble 4
        //    colour 4 = nibble 3
        // and so on...
        // (Nibble order is reversed in the chip)
        std::uint8_t nibble_from_index = (index % 2 == 0) ? (index - 1) : (index + 1);

        // The byte number relative to the start pos (0xF0, 0xF4, 0xF8)
        // So nibble_from_index = 1, byte = +0
        //    nibble_from_index = 2, byte = +0
        //    nibble_from_index = 3, byte = +1
        // and so on...
        std::uint8_t byte_no = fast_ceil(nibble_from_index, 2) - 1;

        // The nibble position relative to the start pos of the byte
        // 0 if the first nibble, 1 if the second nibble
        std::uint8_t nibble_pos = !(nibble_from_index % 2);

        return std::make_tuple(byte_no, nibble_pos);
    }

    bool Lighting::set_colour(std::uint8_t index, Lighting::Colour colour)
    {
        if (index < 1 || index > 8) {
            return false;
        }

        if (colour.r > 0x0F || colour.g > 0x0F || colour.b > 0x0F) {
            return false;
        }

        curr_batch.colours.insert_or_assign(index, colour);

        if (!batch_calls) {
            batch_commit();
        }
        
        return true;
    }

    std::optional<Lighting::Colour> Lighting::get_colour(std::uint8_t index) const
    {
        if (index < 1 || index > 8) {
            return std::nullopt;
        }

        // See Lighting::batch_commit for details

        auto [byte_no, nibble_pos] = colour_cell_pos_from_index(index);

        std::uint8_t r_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF0 + byte_no);
        std::uint8_t g_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF4 + byte_no);
        std::uint8_t b_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF8 + byte_no);

        std::uint8_t r = (r_cell >> (!nibble_pos * 4)) & 0x0F;
        std::uint8_t g = (g_cell >> (!nibble_pos * 4)) & 0x0F;
        std::uint8_t b = (b_cell >> (!nibble_pos * 4)) & 0x0F;

        return std::make_optional(Colour{r, g, b});
    }

    bool Lighting::set_breathing_mode_enabled(bool enable)
    {
        if (get_flash_speed() != FlashingSpeed::Disabled &&
            curr_batch.flash_speed != FlashingSpeed::Disabled) {
            return false;
        }

        curr_batch.breathing_mode_enabled = enable;

        if (!batch_calls) {
            batch_commit();
        }

        return true;
    }

    bool Lighting::is_breathing_mode_enabled() const
    {
        // See Lighting::batch_commit for details
        std::uint8_t val_at_e4 = sio->read_uint8_from_bank(RGB_BANK, 0xE4);

        return (val_at_e4 & 0b00001000) != 0b0;
    }

    bool Lighting::set_step_duration(std::uint16_t step_duration)
    {
        if (step_duration > STEP_DURATION_MAX_VALUE) {
            return false;
        }

        curr_batch.step_duration = step_duration;

        if (!batch_calls) {
            batch_commit();
        }

        return true;
    }

    std::uint16_t Lighting::get_step_duration() const
    {
        // See Lighting::batch_commit for details

        std::uint8_t val_at_fe = sio->read_uint8_from_bank(RGB_BANK, 0xFE);

        std::uint8_t val_at_ff = sio->read_uint8_from_bank(RGB_BANK, 0xFF);

        return (std::uint16_t)(((val_at_ff & 0b1) << 8) | val_at_fe);
    }

    void Lighting::set_flash_speed(Lighting::FlashingSpeed flash_speed)
    {
        if (flash_speed != FlashingSpeed::Disabled && is_breathing_mode_enabled()) {
            set_breathing_mode_enabled(false);
        }

        curr_batch.flash_speed = flash_speed;

        if (!batch_calls) {
            batch_commit();
        }
    }

    Lighting::FlashingSpeed Lighting::get_flash_speed() const
    {
        std::uint8_t val_at_e4 = sio->read_uint8_from_bank(RGB_BANK, 0xE4);

        std::uint8_t flash_speed = val_at_e4 & 0b00000111;

        return 
            (flash_speed == 0 || flash_speed == 1) ? 
                FlashingSpeed::Disabled : 
                static_cast<FlashingSpeed>(flash_speed - 1);
    }

    bool Lighting::has_supported_mb()
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

    void Lighting::batch_commit()
    {
        std::uint8_t val_at_e4 = sio->read_uint8_from_bank(RGB_BANK, 0xE4);
        std::uint8_t val_at_2c = sio->read_uint8_from_bank(UNKNOWN_BANK, 0x2C);
        std::uint8_t val_at_e0 = sio->read_uint8_from_bank(RGB_BANK, 0xE0);
        std::uint8_t val_at_ff = sio->read_uint8_from_bank(RGB_BANK, 0xFF);
        std::uint8_t val_at_fe = sio->read_uint8_from_bank(RGB_BANK, 0xFE);

        // Set LED enabled state
        if (curr_batch.enabled.has_value() && *(curr_batch.enabled) == false) {
            // Disabling the LEDs involves disabling breathing and flashing modes
            // These must be reset afterwards
            val_at_e4 &= 0b11110000; // (last 4 bits, in this order: breathing mode (1 bit), flashing mode (3 bits))
            val_at_e4 |= 0b1; // (flashing mode (3 bits) = 0b001 == disabled)
        }
        else {
            // Enable RGB
            // Sets bit 5 to 1 and bit 4 to 0 of 0x2C
            // All other bits seem to do nothing (at least by themselves), but they MAY have a purpose
            // on certain other boards (tested on MSI B450I GAMING PLUS AC)
            val_at_2c &= 0b11110111;
            val_at_2c |= 0b10000;

            // E0 = 0b11100000 (these 3 bits enable RGB channels, the remaining 5 have
            // unknown purpose)
            val_at_e0 |= 0b11100000;

            // 0xFD in bank 12h seems to be related to some rainbow mode
            // but it is apparently not supported on my board, so I can't test it.
            // I think it's only supported on other MBs that have RGB headers but are
            // different somehow - those function differently from the MBs supported
            // here.

            // Turn on RGB header
            val_at_ff |= 0b00000010;

            // Make sure RGB channels are not inverted
            // Also make sure some 'weird' fade in behaviour that happens in some boards is disabled

            // Invert value is second group of 3 bits from the left in 0xFF: BGR, in this order, from leftmost bit to rightmost bit
            // If bit is set, channel is inverted
            val_at_ff &= 0b11100011;

            // Fade in value is first 3 bits: BGR, in this order, from leftmost bit to rightmost bit
            // If bit is set, fade in is disabled
            val_at_ff |= 0b11100000;

            // Enabling the LEDs involves setting flashing bits to 000
            // but since this will override current flashing mode setting
            // we should only do this if flashing mode isn't
            // already enabled
            // if it is, then leds are enabled already, so we don't need to do this
            // 0b001 = LEDs disabled
            if ((val_at_e4 & 0b111) == 0b001) {
                val_at_e4 &= 0b11111000;
            }
        }

        // Update colours
        //
        //
        // F0 | RR RR RR RR GG GG GG GG BB BB BB BB XX XX XX XX
        // ---------------------------------------------------
        //      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        //
        // Colour indices:
        //    12 34 56 78 12 34 56 78 12 34 56 78
        //
        // F0-F3: each nibble correspond to the red component of colours 1-8 (as shown in the row above)
        // F4-F7: each nibble correspond to the green component of colours 1-8 (as shown in the row above)
        // F8-FB: each nibble correspond to the blue component of colours 1-8 (as shown in the row above)
        for (auto const &[index, colour] : curr_batch.colours) {
            auto [byte_no, nibble_pos] = colour_cell_pos_from_index(index);

            std::uint8_t r_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF0 + byte_no);
            std::uint8_t g_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF4 + byte_no);
            std::uint8_t b_cell = sio->read_uint8_from_bank(RGB_BANK, 0xF8 + byte_no);

            // nibble_pos * 4 is the offset in the byte
            // when we're talking about the clear mask (0x0F << (...)), we want to clear
            // the part which we want to insert into, so if nibble = 0 (left), then we
            // want the leftmost digit to be 0 (clear the contents)
            //
            // when we're talking about the value to insert, since 'Colour' represents a 12-bit depth
            // RGB colour, any value of 'Colour' will be at the right, so if we want to insert
            // it in the first nibble (left), we need to shift it 4 bits to the left, hence (!nibble_pos * 4) (inverts
            // it so that if nibble_pos = 0, !nibble_pos * 4 = 4, rather than = 0)
            std::uint8_t r = (r_cell & (0x0F << (nibble_pos * 4))) | (colour.r << (!nibble_pos * 4));
            std::uint8_t g = (g_cell & (0x0F << (nibble_pos * 4))) | (colour.g << (!nibble_pos * 4));
            std::uint8_t b = (b_cell & (0x0F << (nibble_pos * 4))) | (colour.b << (!nibble_pos * 4));

            sio->write_uint8_to_bank(RGB_BANK, 0xF0 + byte_no, r);
            sio->write_uint8_to_bank(RGB_BANK, 0xF4 + byte_no, g);
            sio->write_uint8_to_bank(RGB_BANK, 0xF8 + byte_no, b);
        }

        // Set breathing mode setting
        // Breathing mode is enabled by setting bit 5 (counting from the left) in 0xE4
        if (curr_batch.breathing_mode_enabled.has_value()) {
            if (*(curr_batch.breathing_mode_enabled) == true) {
                val_at_e4 |= 0b00001000;
            }
            else {
                val_at_e4 &= 0b11110111;
            }
        }

        // Set step duration setting
        //
        //
        // F0 |                                           XX X
        // ---------------------------------------------------
        //      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        //
        //
        // StepDuration is spread across FE and FF
        // FE contains first byte
        // FF contains the last bit
        // and other settings
        if (curr_batch.step_duration.has_value()) {
            std::uint16_t step_duration = *(curr_batch.step_duration);

            val_at_fe = step_duration & 0x00FF;

            val_at_ff |= ((step_duration >> 8) & 0b1);
        }

        // Set flashing speed setting
        // Flashing can be controlled through cell E4
        // 3 rightmost bits define flashing speed
        // 001 = LEDs disabled
        // 000 = always on
        // any other value = flashing (different speed)
        if (curr_batch.flash_speed.has_value()) {
            FlashingSpeed flash_speed = *(curr_batch.flash_speed);

            // If LEDs are disabled, do not do anything, else it will turn on the LEDs
            if ((val_at_e4 & 0b111) != 0b001) {
                val_at_e4 &= 0b11111000;

                if (flash_speed != FlashingSpeed::Disabled) {
                    val_at_e4 |= static_cast<std::uint8_t>(flash_speed) + 1;
                }
            }
        }

        // Write new values to the chip
        sio->write_uint8_to_bank(RGB_BANK, 0xE4, val_at_e4);
        sio->write_uint8_to_bank(UNKNOWN_BANK, 0x2C, val_at_2c);
        sio->write_uint8_to_bank(RGB_BANK, 0xE0, val_at_e0);
        sio->write_uint8_to_bank(RGB_BANK, 0xFF, val_at_ff);
        sio->write_uint8_to_bank(RGB_BANK, 0xFE, val_at_fe);

        // Disable batch state
        batch_calls = false;
        curr_batch = Batch {};
    }
}