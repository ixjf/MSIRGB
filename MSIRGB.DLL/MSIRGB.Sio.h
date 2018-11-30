#pragma once

#include "logic\Sio.h"

using namespace System;
using namespace System::Windows::Media;

namespace MSIRGB {
    public ref class Sio {
    public:
        enum class ColourIndex : Byte {
            Colour1 = static_cast<Byte>(logic::Sio::ColourIndex::Colour1),
            Colour2 = static_cast<Byte>(logic::Sio::ColourIndex::Colour2),
            Colour3 = static_cast<Byte>(logic::Sio::ColourIndex::Colour3),
            Colour4 = static_cast<Byte>(logic::Sio::ColourIndex::Colour4)
        };

        enum class Channel : Byte {
            R = static_cast<Byte>(logic::Sio::Channel::R),
            G = static_cast<Byte>(logic::Sio::Channel::G),
            B = static_cast<Byte>(logic::Sio::Channel::B)
        };

        enum class FlashingSpeed : Byte {
            Disabled = static_cast<Byte>(logic::Sio::FlashingSpeed::Disabled),
            Speed1 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed1),
            Speed2 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed2),
            Speed3 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed3),
            Speed4 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed4),
            Speed5 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed5),
            Speed6 = static_cast<Byte>(logic::Sio::FlashingSpeed::Speed6)
        };

        enum class ErrorCode : int {
            DriverLoadFailed = static_cast<int>(logic::Sio::ErrorCode::DriverLoadFailed),
            MotherboardNotSupported = static_cast<int>(logic::Sio::ErrorCode::MotherboardNotSupported)
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

        literal UInt16 STEP_DURATION_MAX_VALUE = static_cast<UInt16>(logic::Sio::STEP_DURATION_MAX_VALUE);

        Sio(Boolean ignoreMbCheck)
        {
            try {
                sio = new logic::Sio(ignoreMbCheck);
            }
            catch (logic::Sio::Exception &e) {
                throw gcnew Exception(static_cast<ErrorCode>(e.error_code()));
            }
        }

       ~Sio()
       {
            delete sio;
       }

       void SetLedEnabled(bool enable)
       {
            sio->set_led_enabled(enable);
       }

       void SetColour(ColourIndex index, Color colour)
       {
            sio->set_colour(static_cast<logic::Sio::ColourIndex>(index), logic::Sio::Colour(colour.R, colour.G, colour.B));
       }

       Color GetColour(ColourIndex index)
       {
            logic::Sio::Colour colour = sio->get_colour(static_cast<logic::Sio::ColourIndex>(index));

            return Color::FromArgb(0xFF, colour.r, colour.g, colour.b);
       }

       bool SetBreathingModeEnabled(Boolean enabled)
       {
           return sio->set_breathing_mode_enabled(enabled);
       }

       Boolean IsBreathingModeEnabled()
       {
            return sio->is_breathing_mode_enabled();
       }

       Boolean SetStepDuration(UInt16 stepDuration)
       {
           if (stepDuration > logic::Sio::STEP_DURATION_MAX_VALUE)
               return false;

           sio->set_step_duration(stepDuration);

            return true;
       }

       UInt16 GetStepDuration()
       {
            return sio->get_step_duration();
       }

       void SetFlashingSpeed(FlashingSpeed flashSpeed)
       {
            sio->set_flash_speed(static_cast<logic::Sio::FlashingSpeed>(flashSpeed));
       }

       FlashingSpeed GetFlashingSpeed()
       {
            return static_cast<FlashingSpeed>(sio->get_flash_speed());
       }

    private:
        logic::Sio *sio;
	};
}
