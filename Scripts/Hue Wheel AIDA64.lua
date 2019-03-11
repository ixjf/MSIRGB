-- cpu, gpu, fan or hue
local mode = "hue"

-- min and max speed of the CPU fan
local fan = {700, 2400}
-- temperature min and max, applies for gpu and cpu
local temp = {35, 65}

-- max CPU temperature and color for alarm
local cpu = {}
cpu["max_temp"] = 60
cpu["r"] = 0
cpu["g"] = 0
cpu["b"] = 0xf

-- max GPU temperature and color for alarm
local gpu = {}
gpu["max_temp"] = 70
gpu["r"] = 0
gpu["g"] = 0xf
gpu["b"] = 0

-- max MB temperature and color for alarm
local mb = {}
mb["max_temp"] = 50
mb["r"] = 0xf
mb["g"] = 0xf
mb["b"] = 0xf

--
--
--
--
--
--
--
--
--
-- HSV color range (degrees)
local hue_min = 0.0
local hue_max = 0.60
-- Steps for the "hue" mode
local color_steps = 0.001
local delay = 80 -- in milliseconds

Lighting.BatchBegin()
Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)
Lighting.BatchEnd()

-- Blinks between red and configured hardware-color - the "alarm"
local function Alarm(r, g, b)
    Lighting.BatchBegin()
    for i = 1, 8 do
        Lighting.SetColour(i, 0xf, 0, 0)
    end
    Lighting.BatchEnd()
    os.sleep(200)
    Lighting.BatchBegin()
    for i = 1, 8 do
        Lighting.SetColour(i, r, g, b)
    end
    Lighting.BatchEnd()
    os.sleep(200)
end

local min = 0
local max = 0

if Aida64.IsInstalledAndRunning() then
    min = temp[1]
    max = temp[2]
    if mode == "cpu" then
        hardware = "TCPU"
    elseif mode == "gpu" then
        hardware = "TGPU1DIO"
        if not Aida64.GetSensorValue(hardware) then
            print("GPU temparature is only available in paid version of AIDA64!")
            print("Switching to hue mode...")
            mode = "hue"
        end
    elseif mode == "fan" then
        hardware = "FCPU"
        min = fan[1]
        max = fan[2]
    end
else
    if mode ~= "hue" then
        print("AIDA64 is not running!")
        print("Switching to hue mode...")
        mode = "hue"
    end
end

local color = 0
local alarm = false
while true do
    if Aida64.IsInstalledAndRunning() then
        cpu["curr_temp"] = Aida64.GetSensorValue("TCPU")
        mb["curr_temp"] = Aida64.GetSensorValue("TMOBO")
        gpu["curr_temp"] = Aida64.GetSensorValue("TGPU1DIO")

        -- temperature alarm
        local devices = {cpu, gpu, mb}
        for devicecount = 1, 3 do
            local device = devices[devicecount]
            -- check if we really got the value (non-paid version for gpu temparature)
            if device["curr_temp"] then
                if device["curr_temp"] >= device["max_temp"] then
                    Alarm(device["r"], device["g"], device["b"])
                    alarm = true
                end
            end
        end
    end

    if alarm == false then
        if mode ~= "hue" then
            if Aida64.IsInstalledAndRunning() then
                local hardware_value = Aida64.GetSensorValue(hardware)
                local percent = (math.max(hardware_value, min) - min) / (max - min)
                color = hue_max - percent * (hue_max - hue_min)
                color = math.max(color, hue_min)
            else
                print('AIDA64 is not running!')
                print('Switching to hue mode...')
                mode = 'hue'
            end
        else
            color = color + color_steps
            if color == 1.0 then
                color = 0
            end
        end

        -- finally, set the color for all modes
        local r, g, b = Lighting.ColourUtils.HSVtoRGB(color, 1.0, 1.0)
        r = tonumber(("%x"):format(r * 15), 16)
        g = tonumber(("%x"):format(g * 15), 16)
        b = tonumber(("%x"):format(b * 15), 16)

        Lighting.BatchBegin()
        for i = 1, 8 do
            Lighting.SetColour(i, r, g, b)
        end
        Lighting.BatchEnd()
        os.sleep(delay)
    end
    alarm = false
end
