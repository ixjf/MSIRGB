-- Based on Tweaking4Fun's Strobe effect

-- User-changeable variables
local flashes_per_second = 20
local pulse_time = 0.6 -- how long each flashing pulse will take, in seconds (repeat flashes_per_second flashes pulse_time times)
local pulse_delay = 1000 -- how long between each flashing pulse, in milliseconds
local flash_colour = {["r"] = 0x2, ["g"] = 0x7, ["b"] = 0xf}

--
local flash = require('utils.custom_flash')

local defaultInvertedColourChannels = Lighting.GetDefaultColourChannelsInvertedSetting()

while true do
    Lighting.BatchBegin()
    Lighting.SetRChannelInverted(defaultInvertedColourChannels)
    Lighting.SetGChannelInverted(defaultInvertedColourChannels)
    Lighting.SetBChannelInverted(defaultInvertedColourChannels)
    Lighting.SetFlashingSpeed(0)
    Lighting.SetBreathingModeEnabled(false)
    
    for i = 1, 8 do
        Lighting.SetColour(i, flash_colour.r, flash_colour.g, flash_colour.b)
    end
    Lighting.BatchEnd()

    flash(flashes_per_second, pulse_time)
    os.sleep(pulse_delay)
end
