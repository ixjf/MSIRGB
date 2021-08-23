-- User-changeable variables
local pumpkin_colour = {["r"] = 0xf, ["g"] = 0x2, ["b"] = 0x0}

--
local Timer = require('base.timer')

function effect()
    Lighting.SetFlashingSpeed(0)
    Lighting.SetBreathingModeEnabled(true)
    
    for i = 1, 8 do
        Lighting.SetColour(i, pumpkin_colour.r, pumpkin_colour.g, pumpkin_colour.b)
    end    
end

Timer.Set(effect, Timer.DefaultInterval)