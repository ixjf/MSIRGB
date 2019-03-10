-- ISC License
--
-- Copyright © 2017, Simonas Kazlauskas
-- Copyright © 2018, Pedro Fanha
--
-- Permission to use, copy, modify, and/or distribute this software for any
-- purpose with or without fee is hereby granted, provided that the above1
-- copyright notice and this permission notice appear in all copies.
--
-- THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
-- REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
-- INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
-- LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
-- OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.

-- cpu, gpu, fan or hue
local mode = 'hue'

-- min and max speed of the CPU fan
local fan = {700, 1800}
-- temperature min and max, applies for gpu and cpu
local temp = {35, 65}

-- max CPU temperature and color for alarm
local cpu = {}
cpu['max_temp'] = 60
cpu['r'] = 0
cpu['g'] = 0
cpu['b'] = 255

-- max GPU temperature and color for alarm
local gpu = {}
gpu['max_temp'] = 70
gpu['r'] = 0
gpu['g'] = 255
gpu['b'] = 0

-- max MB temperature and color for alarm
local mb = {}
mb['max_temp'] = 50
mb['r'] = 255
mb['g'] = 255
mb['b'] = 255

-- HSV color range
local color_min = 0
local color_max = 200
-- Steps for the "hue" mode
local color_steps = 0.5
local delay = 500

Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

-- https://gist.github.com/GigsD4X/8513963
function HSVToRGB(hue, saturation, value)
    -- Returns the RGB equivalent of the given HSV-defined color
    -- (adapted from some code found around the web)

    -- Get the hue sector
    local hue_sector = math.floor( hue / 60 );
    local hue_sector_offset = ( hue / 60 ) - hue_sector;

    local p = value * ( 1 - saturation );
    local q = value * ( 1 - saturation * hue_sector_offset );
    local t = value * ( 1 - saturation * ( 1 - hue_sector_offset ) );

    if hue_sector == 0 then
        return value, t, p;
    elseif hue_sector == 1 then
        return q, value, p;
    elseif hue_sector == 2 then
        return p, value, t;
    elseif hue_sector == 3 then
        return p, q, value;
    elseif hue_sector == 4 then
        return t, p, value;
    elseif hue_sector == 5 then
        return value, p, q;
    end;
end;

-- Blinks between red and configured hardware-color - the "alarm"
function Alarm(r, g, b)
    for i = 1, 8 do
        Lighting.SetColour(i, 255, 0, 0)
    end
    os.sleep(200)
    for i = 1, 8 do
        Lighting.SetColour(i, r, g, b)
    end
    os.sleep(200)
end

if Aida64.IsInstalledAndRunning() then
    min = temp[1]
    max = temp[2]
    if mode == 'cpu' then
        hardware = 'TCPU'
    elseif mode == 'gpu' then
        hardware = 'TGPU1DIO'
        if not Aida64.GetSensorValue(hardware) then
            print('GPU temparature is only available in paid version of AIDA64!')
            print('Switching to hue mode...')
            mode = 'hue'
        end
    elseif mode == 'fan' then
        hardware = 'FCPU'
        min = fan[1]
        max = fan[2]
    end
else
    print('AIDA64 is not running!')
    print('Switching to hue mode...')
    mode = 'hue'
end

local color = 0
local alarm = false
while true do
    if Aida64.IsInstalledAndRunning() then
        cpu['curr_temp'] = Aida64.GetSensorValue('TCPU')
        mb['curr_temp'] = Aida64.GetSensorValue('TMOBO')
        gpu['curr_temp'] = Aida64.GetSensorValue('TGPU1DIO')

        -- temperature alarm
        local devices = {cpu, gpu, mb}
        for devicecount=1,3 do
            local device = devices[devicecount]
            -- check if we really got the value (non-paid version for gpu temparature)
            if device['curr_temp'] then
                if device['curr_temp'] >= device['max_temp'] then
                    Alarm(device['r'], device['g'], device['b'])
                    alarm = true
                end
            end
        end
    end

    if alarm == false then
        if mode ~= 'hue' then
            if Aida64.IsInstalledAndRunning() then
                local hardware_value = Aida64.GetSensorValue(hardware)
                local percent = (math.max(hardware_value, min) - min) / (max - min)
                color = color_max - percent * (color_max - color_min)
                color = math.max(color, color_min)
            else
                print('AIDA64 is not running!')
            end
        else
            color = color + color_steps
            if color == 360 then
                color = 0
            end
        end

        -- finally, set the color; for all modes
        local r, g, b = HSVToRGB(color, 1.0, 1.0)
        r = tonumber(("%x"):format(r * 15):rep(2), 16)
        g = tonumber(("%x"):format(g * 15):rep(2), 16)
        b = tonumber(("%x"):format(b * 15):rep(2), 16)

        for i = 1, 8 do
            Lighting.SetColour(i, r, g, b)
        end
        os.sleep(delay)
    end
    alarm = false
end
