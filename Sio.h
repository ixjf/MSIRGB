#pragma once

#include "IsaDrv.h"

class Sio {
public:
    class Exception : public std::runtime_error {
    public:
        Exception(const char *msg) : std::runtime_error(msg)
        {
            
        }
    };

    enum class Channel : std::uint8_t {
        R = 1,
        G,
        B
    };

    enum class ColourIndex : std::uint8_t {
        Colour1 = 1,
        Colour2,
        Colour3,
        Colour4
    };

    static const std::uint32_t  COLOUR_MAX_VALUE        = 0xFFFFFF;
    static const std::uint16_t  STEP_DURATION_MAX_VALUE = 0x1FF;
    static const std::uint8_t   FLASHING_MAX_VALUE      = 6;

    Sio(bool ignore_mb_check);
   ~Sio() = default;

    void                    set_led_disabled                    () const;
    void                    set_colour                          (ColourIndex index, std::uint32_t colour) const;
    void                    set_channel_inverted                (Channel c, bool enable) const;
    void                    set_breathing_mode_enabled          (bool enable) const;
    void                    set_step_duration                   (std::uint16_t step_duration) const;
    void                    set_flash_speed                     (std::uint8_t flash_speed) const;
    void                    set_fade_in_enabled                 (Channel c, bool enable) const;

private:
    static bool             has_supported_mb                    ();

    // Should work on all NCT chips - all supported MBs have NCT chips as well
    void                    chip_enter_extended_function_mode   () const;
    void                    chip_exit_extended_function_mode    () const;
    void                    chip_enter_bank                     (std::uint8_t bank) const;
    std::uint8_t            chip_read                           (std::uint8_t index) const;
    void                    chip_write                          (std::uint8_t index, std::uint8_t data) const;

    std::uint8_t            chip_read_cell_from_bank            (std::uint8_t bank, std::uint8_t index) const;
    void                    chip_write_cell_to_bank             (std::uint8_t bank, std::uint8_t index, std::uint8_t data) const;
    std::uint8_t            chip_read_cell_from_global          (std::uint8_t index) const;
    void                    chip_write_cell_to_global           (std::uint8_t index, std::uint8_t data) const;

    std::unique_ptr<IsaDrv> drv;
};