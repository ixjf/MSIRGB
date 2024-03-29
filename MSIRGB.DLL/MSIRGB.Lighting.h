#pragma once

#include "logic\Lighting.h"

using namespace System;
using namespace System::Windows::Media;

namespace MSIRGB {
    public ref class Lighting {
    public:
        enum class FlashingSpeed : Byte {
            Disabled = static_cast<Byte>(logic::Lighting::FlashingSpeed::Disabled),
            Speed1 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed1),
            Speed2 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed2),
            Speed3 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed3),
            Speed4 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed4),
            Speed5 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed5),
            Speed6 = static_cast<Byte>(logic::Lighting::FlashingSpeed::Speed6)
        };

        enum class ErrorCode : int {
            DriverLoadFailed = static_cast<int>(logic::Lighting::ErrorCode::DriverLoadFailed),
            MotherboardVendorNotSupported = static_cast<int>(logic::Lighting::ErrorCode::MotherboardVendorNotSupported),
            MotherboardModelNotSupported = static_cast<int>(logic::Lighting::ErrorCode::MotherboardModelNotSupported),
            MotherboardModelMayOrMayNotBeSupported = static_cast<int>(logic::Lighting::ErrorCode::MotherboardModelMayOrMayNotBeSupported),
            LoadFailed = static_cast<int>(logic::Lighting::ErrorCode::LoadFailed),
        };

        ref class Exception : public ::Exception {
        public:
            Exception(ErrorCode ec) : ::Exception(""), ec(ec)
            {

            }

            ErrorCode GetErrorCode()
            {
                return ec;
            }

        private:
            ErrorCode ec;
        };

        literal UInt16 STEP_DURATION_MAX_VALUE = static_cast<UInt16>(logic::Lighting::STEP_DURATION_MAX_VALUE);

        Lighting(Boolean ignoreMbCheck)
        {
            try {
                lighting = new logic::Lighting(ignoreMbCheck);
            }
            catch (logic::Lighting::Exception &e) {
                throw gcnew Exception(static_cast<ErrorCode>(e.error_code()));
            }
        }

        ~Lighting()
        {
            delete lighting;
        }

        bool BatchBegin()
        {
            return lighting->batch_begin();
        }

        void SetLedEnabled(bool enable)
        {
            lighting->set_led_enabled(enable);
        }

        bool SetColour(Byte index, Color colour)
        {
            return lighting->set_colour(index, logic::Lighting::Colour{ colour.R, colour.G, colour.B });
        }

        Nullable<Color> GetColour(Byte index)
        {
            std::optional<logic::Lighting::Colour> colour = lighting->get_colour(index);

            if (!colour) {
                return Nullable<Color>();
            }

            return Nullable<Color>(Color::FromRgb(colour->r, colour->g, colour->b));
        }

        Boolean GetDefaultColourChannelsInvertedSetting()
        {
            return lighting->get_default_colour_channels_inverted_setting();
        }

        void SetRChannelInverted(Boolean inverted)
        {
            lighting->set_r_channel_inverted(inverted);
        }

        void SetGChannelInverted(Boolean inverted)
        {
            lighting->set_g_channel_inverted(inverted);
        }

        void SetBChannelInverted(Boolean inverted)
        {
            lighting->set_b_channel_inverted(inverted);
        }

        Boolean IsRChannelInverted()
        {
            return lighting->is_r_channel_inverted();
        }

        Boolean IsGChannelInverted()
        {
            return lighting->is_g_channel_inverted();
        }

        Boolean IsBChannelInverted()
        {
            return lighting->is_b_channel_inverted();
        }

        bool SetBreathingModeEnabled(Boolean enabled)
        {
            return lighting->set_breathing_mode_enabled(enabled);
        }

        Boolean IsBreathingModeEnabled()
        {
            return lighting->is_breathing_mode_enabled();
        }

        Boolean SetStepDuration(UInt16 stepDuration)
        {
            return lighting->set_step_duration(stepDuration);
        }

        UInt16 GetStepDuration()
        {
            return lighting->get_step_duration();
        }

        void SetFlashingSpeed(FlashingSpeed flashSpeed)
        {
            lighting->set_flash_speed(static_cast<logic::Lighting::FlashingSpeed>(flashSpeed));
        }

        FlashingSpeed GetFlashingSpeed()
        {
            return static_cast<FlashingSpeed>(lighting->get_flash_speed());
        }

        bool BatchEnd()
        {
            return lighting->batch_end();
        }

    private:
        logic::Lighting *lighting;
    };
}
