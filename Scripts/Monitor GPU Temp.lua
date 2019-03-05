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

-- temperatur minimum and max, applies for gpu and cpu
local temp = {35, 65}

local color_min = 0
local color_max = 200
local delay = 100

Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

-- https://gist.github.com/GigsD4X/8513963
function HSVToRGB( hue, saturation, value )
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

while true do
    if Aida64.IsInstalledAndRunning() then
        local hw_temp = Aida64.GetSensorValue('TGPU1DIO')
        local percent = (hw_temp - temp[1]) / (temp[2] - temp[1])
        local color = color_max - percent * (color_max - color_min)

        local r, g, b = HSVToRGB(color, 1.0, 1.0)
        r = tonumber(("%x"):format(r * 15):rep(2), 16)
        g = tonumber(("%x"):format(g * 15):rep(2), 16)
        b = tonumber(("%x"):format(b * 15):rep(2), 16)

        for i = 1, 8 do
            Lighting.SetColour(i, r, g, b)
        end
    end
    os.sleep(delay)
end
