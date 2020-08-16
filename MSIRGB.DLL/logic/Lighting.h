#pragma once

#include "pch.h"
#include "Sio.h"

namespace logic {
    class Lighting
    {
    public:
        enum class ErrorCode {
            DriverLoadFailed,
            MotherboardVendorNotSupported,
            MotherboardModelNotSupported,
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
        };

        static const std::uint16_t  STEP_DURATION_MAX_VALUE = 511;


        Lighting(bool ignore_mb_check);
        ~Lighting();

        void                    enter_critical_section() const;
        void                    leave_critical_section() const;

        bool                    batch_begin();

        void                    set_led_enabled(bool enable);
        bool                    set_colour(std::uint8_t index, Colour colour);
        std::optional<Colour>   get_colour(std::uint8_t index) const;
        bool                    set_breathing_mode_enabled(bool enable);
        bool                    is_breathing_mode_enabled() const;
        bool                    set_step_duration(std::uint16_t step_duration);
        std::uint16_t           get_step_duration() const;
        void                    set_flash_speed(FlashingSpeed flash_speed);
        FlashingSpeed           get_flash_speed() const;

        bool                    batch_end();

    private:
        enum class MbCompatError {
            Ok,
            UnsupportedVendor,
            UnsupportedModel,
        };

        enum MbFlags : uint8_t {
            NONE,
            INVERTED_COLOUR_CHANNELS // FIXME: name probably not accurate. also changes another setting in 0xFD that I don't know what it is (see batch_commit)
            // seems to change rainbow mode settings
            // TODO: is there anything else affected by INVERTED_COLOUR_CHANNELS? I think channels inverted is the only change for these motherboards
        };

        MbCompatError           check_supported_mb();

        void                    batch_commit();

        struct Batch {
            std::unordered_map<std::uint8_t, Colour>        colours;
            std::optional<bool>                             enabled;
            std::optional<bool>                             breathing_mode_enabled;
            std::optional<std::uint16_t>                    step_duration;
            std::optional<FlashingSpeed>                    flash_speed;
        };

        MbFlags                                             mb_flags;

        HANDLE                                              csection_mutex;

        std::unique_ptr<Sio>                                sio;

        bool                                                batch_calls;
        Batch                                               curr_batch;
    };
}