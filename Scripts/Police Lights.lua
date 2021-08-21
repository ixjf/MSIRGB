-- User-changeable variables
local flashes_per_second = 15
local pulse_time = 0.4 -- how long each flashing pulse will take, in seconds (repeat flashes_per_second flashes pulse_time times)
local pulse_delay = 1 -- how long between each flashing pulse, in milliseconds
local flash_colour = {["r"] = 0x2, ["g"] = 0xf, ["b"] = 0xf}
local alt_flash_colour = {["r"] = 0xf, ["g"] = 0xf, ["b"] = 0x2}

--
local flash = require('utils.custom_flash')

Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

local alt_color = true

local function change_colour()
    if alt_color then
        Lighting.BatchBegin()
        for i = 1, 8 do
            Lighting.SetColour(i, flash_colour.r, flash_colour.g, flash_colour.b)
        end
        Lighting.BatchEnd()
        alt_color = false
    else
        Lighting.BatchBegin()
        for i = 1, 8 do
            Lighting.SetColour(i, alt_flash_colour.r, alt_flash_colour.g, alt_flash_colour.b)
        end
        Lighting.BatchEnd()
        alt_color = true
    end
end

while true do
    change_colour()
    flash(flashes_per_second, pulse_time)
    os.sleep(pulse_delay)
end
