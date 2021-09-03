-- User-changeable variables
local speed = 130 -- milliseconds between each iteration of 'colours'
local colours = {
    {0x6, 0x2, 0xa}, 
    {0x6, 0x3, 0xa}, 
    {0x6, 0x4, 0xa}, 
    {0x7, 0x6, 0x9}, 
    {0x7, 0x6, 0x8}, 
    {0x6, 0x6, 0x6}, 
    {0x5, 0x5, 0x5}, 
    {0x3, 0x3, 0x3}}

--
local n = 1

local defaultInvertedColourChannels = Lighting.GetDefaultColourChannelsInvertedSetting()

while true do
    Lighting.BatchBegin()
    Lighting.SetRChannelInverted(defaultInvertedColourChannels)
    Lighting.SetGChannelInverted(defaultInvertedColourChannels)
    Lighting.SetBChannelInverted(defaultInvertedColourChannels)
    Lighting.SetBreathingModeEnabled(false)
    Lighting.SetFlashingSpeed(0)
    Lighting.SetStepDuration(511)

    for i = 1, 8 do
        Lighting.SetColour(i, colours[n][1], colours[n][2], colours[n][3])
    end
    
    Lighting.BatchEnd()
    n = (n % #colours) + 1
    os.sleep(speed)
end
