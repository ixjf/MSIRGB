-- Adapted from nagisa/msi-rgb's Hue Wheel effect

-- Variables
local saturation = 0.933
local value = 1.0

local delay = 80 -- delay between each colour update, in milliseconds
local colour_step = 1.1

--
Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

local i = 0
while true do
    local r, g, b = Lighting.ColourUtils.HSVtoRGB((i % 98.0) / 98.0, saturation, value)

    r = tonumber(("%x"):format(r * 15), 16)
    g = tonumber(("%x"):format(g * 15), 16)
    b = tonumber(("%x"):format(b * 15), 16)

    Lighting.BatchBegin()
    for i = 1, 8 do
        Lighting.SetColour(i, r, g, b)
    end
    Lighting.BatchEnd()

    os.sleep(delay)

    i = i + colour_step
end

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
