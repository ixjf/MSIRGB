#pragma once

#include "pch.h"
#include "IsaDrv.h"

namespace logic {
    class Sio {
    public:
        enum class ErrorCode {
            DriverLoadFailed,
            MotherboardNotSupported,
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

        enum class FlashingSpeed : std::uint8_t {
            Disabled,
            Speed1,
            Speed2,
            Speed3,
            Speed4,
            Speed5,
            Speed6
        };

        struct Colour {
            std::uint8_t r;
            std::uint8_t g;
            std::uint8_t b;

            Colour(std::uint8_t r, std::uint8_t g, std::uint8_t b)
                : r(r), g(g), b(b)
            {
            
            }
        };

        static const std::uint16_t  STEP_DURATION_MAX_VALUE = 511;

        Sio(bool ignore_mb_check);
       ~Sio() = default;

        // LEDs are enabled in every 'set' call because changing some settings (breathing or flashing)
        // inevitably turns them on. I decided to explicitly enable them on every call for consistency

        // Disabling the LEDs overwrites flashing and breathing modes. Those must be reset after enabling the LEDs again
        void                    set_led_enabled                     (bool enable) const;
        void                    set_colour                          (ColourIndex index, Colour colour) const;
        Colour                  get_colour                          (ColourIndex index) const;
        bool                    set_breathing_mode_enabled          (bool enable) const;
        bool                    is_breathing_mode_enabled           () const;
        void                    set_step_duration                   (std::uint16_t step_duration) const;
        std::uint16_t           get_step_duration                   () const;
        void                    set_flash_speed                     (FlashingSpeed flash_speed) const;
        FlashingSpeed           get_flash_speed                     () const;

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
}