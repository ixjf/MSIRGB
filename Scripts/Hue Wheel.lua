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

function HsvToRgb(h, s, v)
    if s == 0.0 then return v, v, v end

    local i = math.floor(h * 6.0)

    local f = (h * 6.0) - i 
    local p, q, t = v * (1.0 - s), v * (1.0 - s * f), v * (1.0 - s * (1.0 - f))
    
    i = i % 6

    if i == 0 then return v, t, p end
    if i == 1 then return q, v, p end
    if i == 2 then return p, v, t end
    if i == 3 then return p, q, v end
    if i == 4 then return t, p, v end
    if i == 5 then return v, p, q end
end

-- Adapted from nagisa/msi-rgb's Hue Wheel effect
Lighting.SetStepDuration(511)
Lighting.SetFlashingSpeed(0)
Lighting.SetBreathingModeEnabled(false)

local i = 0
while true do
    local r, g, b = HsvToRgb((i % 96.0) / 96.0, 0.9, 1.0)

    r = tonumber(("%x"):format(r * 15):rep(2), 16)
    g = tonumber(("%x"):format(g * 15):rep(2), 16)
    b = tonumber(("%x"):format(b * 15):rep(2), 16)

    Lighting.SetColour(1, r, g, b)
    Lighting.SetColour(2, r, g, b)
    Lighting.SetColour(3, r, g, b)
    Lighting.SetColour(4, r, g, b)

    os.sleep(20)

    i = i + 0.35
end