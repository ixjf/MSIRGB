-- Based on Tweaking4Fun's Strobe effect

-- Variables
local flashes_per_second = 20
local pulse_time = 0.6 -- how long each flashing pulse will take, in seconds (repeat flashes_per_second flashes pulse_time times)
local pulse_delay = 1000 -- how long between each flashing pulse, in milliseconds
local flash_colour = {["r"] = 0x2, ["g"] = 0x7, ["b"] = 0xf}

--
local function flash(speed, time)
    -- speed: flashes/sec
    -- time: seconds
    local num_flashes = speed * time
    local delay = (time * 1000) / (num_flashes * 2)

    for i = 1, num_flashes do
        Lighting.SetAllEnabled(true)
        os.sleep(delay)
        Lighting.SetAllEnabled(false)
        os.sleep(delay)
    end
end

Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

for i = 1, 8 do
    Lighting.SetColour(i, flash_colour.r, flash_colour.g, flash_colour.b)
end

while true do
    flash(flashes_per_second, pulse_time)
    os.sleep(pulse_delay)
end
