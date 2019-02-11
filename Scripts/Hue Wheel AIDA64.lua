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
mode = 'hue'

-- Min and Max speed of the CPU fan
fan = {750, 1800}
-- temperatur minimum and max, applies for gpu and cpu
temp = {35, 65}

color_min = 0
color_max = 200

cpu_temp = 50
gpu_temp = 60
mb_temp = 50

cpu_r = 0
cpu_g = 0
cpu_b = 255

gpu_r = 0
gpu_g = 255
gpu_b = 0

mb_r = 255
mb_g = 255
mb_b = 255

color_steps = 0.5
delay = 500


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

-- Adapted from nagisa/msi-rgb's Hue Wheel effect
Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

function Color(value, temp1, temp2)
    percent = (value - temp1) / (temp2 - temp1)
    color = color_max - percent * (color_max - color_min)
    return color
end

local color = 0
while true do
    if Aida64.IsInstalledAndRunning() then
        if mode == 'fan' then
            rpm = Aida64.GetSensorValue('FCPU')
            color = Color(rpm, fan[1], fan[2])
        elseif mode == 'cpu' then
            hw_temp = Aida64.GetSensorValue('TCPU')
            color = Color(hw_temp, temp[1], temp[2])
        elseif mode == 'gpu' then
            hw_temp = Aida64.GetSensorValue('TGPU1DIO')
            color = Color(hw_temp, temp[1], temp[2])
        else
            while Aida64.GetSensorValue('TCPU') >= cpu_temp  do
                for i = 1, 8 do
                    Lighting.SetColour(i, 255, 0, 0)
                end
                os.sleep(200)
                for i = 1, 8 do
                    Lighting.SetColour(i, cpu_r, cpu_g, cpu_b)
                end
                os.sleep(200)
            end
            while Aida64.GetSensorValue('TGPU1DIO') >= gpu_temp do
                for i = 1, 8 do
                    Lighting.SetColour(i, 255, 0, 0)
                end
                os.sleep(200)
                for i = 1, 8 do
                    Lighting.SetColour(i, gpu_r, gpu_g, gpu_b)
                end
                os.sleep(200)
            end
            while Aida64.GetSensorValue('TMOBO') >= mb_temp do
                for i = 1, 8 do
                    Lighting.SetColour(i, 255, 0, 0)
                end
                os.sleep(200)
                for i = 1, 8 do
                    Lighting.SetColour(i, mb_r, mb_g, mb_b)
                end
                os.sleep(200)
            end
            color = color + color_steps
            if color == 360 then
                color = 0
            end
        end
    end

    local r, g, b = HSVToRGB(color, 1.0, 1.0)
    r = tonumber(("%x"):format(r * 15):rep(2), 16)
    g = tonumber(("%x"):format(g * 15):rep(2), 16)
    b = tonumber(("%x"):format(b * 15):rep(2), 16)

    for i = 1, 8 do
        Lighting.SetColour(i, r, g, b)
    end
    os.sleep(delay)
end
