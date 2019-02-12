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

-- Min and Max speed of the CPU fan
local fan = {700, 1800}
-- temperatur minimum and max, applies for gpu and cpu
local temp = {35, 65}

-- Max CPU temperature and color for alarm
local cpu = {}
cpu['max_temp'] = 60
cpu['r'] = 0
cpu['g'] = 0
cpu['b'] = 255

-- Max GPU temperature and color for alarm
local gpu = {}
gpu['max_temp'] = 70
gpu['r'] = 0
gpu['g'] = 255
gpu['b'] = 0

-- Max MB temperature and color for alarm
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

    local r
    local g
    local b
    if hue_sector == 0 then
        r, g, b = value, t, p;
    elseif hue_sector == 1 then
        r, g, b = q, value, p;
    elseif hue_sector == 2 then
        r, g, b = p, value, t;
    elseif hue_sector == 3 then
        r, g, b = p, q, value;
    elseif hue_sector == 4 then
        r, g, b = t, p, value;
    else
        r, g, b = value, p, q;
    end;
    r = tonumber(("%x"):format(r * 15):rep(2), 16)
    g = tonumber(("%x"):format(g * 15):rep(2), 16)
    b = tonumber(("%x"):format(b * 15):rep(2), 16)

    return r, g, b
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

function Color(value, temp1, temp2)
    local percent = (value - temp1) / (temp2 - temp1)
    local color = color_max - percent * (color_max - color_min)
    return color
end

local min = temp[1]
local max = temp[2]
if mode == 'cpu' then
    hardware = 'TCPU'
elseif mode == 'gpu' then
    hardware = 'TGPU1DIO'
elseif mode == 'fan' then
    hardware = 'FCPU'
    min = fan[1]
    max = fan[2]
end

local color = 0
local alarm = false
while true do
    if Aida64.IsInstalledAndRunning() then
        cpu['curr_temp'] = Aida64.GetSensorValue('TCPU')
        gpu['curr_temp'] = Aida64.GetSensorValue('TGPU1DIO')
        mb['curr_temp'] = Aida64.GetSensorValue('TMOBO')
    end
    if mode ~= 'hue' then
        if Aida64.IsInstalledAndRunning() then
            local hardware_value = Aida64.GetSensorValue(hardware)
            color = Color(hardware_value, min, max)
        end
    else
        color = color + color_steps
        if color == 360 then
            color = 0
        end
    end

    local devices = {cpu, gpu, mb}
    for devicecount=1,3 do
        if devices[devicecount]['curr_temp'] >= devices[devicecount]['max_temp'] then
            Alarm(devices[devicecount]['r'], devices[devicecount]['g'], devices[devicecount]['b'])
            alarm = true
        end
    end
    if alarm == false then
        local r, g, b = HSVToRGB(color, 1.0, 1.0)
        for i = 1, 8 do
            Lighting.SetColour(i, r, g, b)
        end
        os.sleep(delay)
    end
    alarm = false
end
