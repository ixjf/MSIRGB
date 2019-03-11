-- temperature minimum and max, applies for gpu and cpu
local temp = {35, 65}

local hue_min = 0.0
local hue_max = 0.6
local delay = 100 -- in milliseconds

Lighting.BatchBegin()
Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)
Lighting.BatchEnd()

while true do
    if Aida64.IsInstalledAndRunning() then
        local hw_temp = Aida64.GetSensorValue("TGPU1DIO")
        local percent = (hw_temp - temp[1]) / (temp[2] - temp[1])
        local color = hue_max - percent * (hue_max - hue_min)

        local r, g, b = Lighting.ColourUtils.HSVtoRGB(color, 1.0, 1.0)
        r = tonumber(("%x"):format(r * 15), 16)
        g = tonumber(("%x"):format(g * 15), 16)
        b = tonumber(("%x"):format(b * 15), 16)

        Lighting.BatchBegin()
        for i = 1, 8 do
            Lighting.SetColour(i, r, g, b)
        end
        Lighting.BatchEnd()
    end
    os.sleep(delay)
end
