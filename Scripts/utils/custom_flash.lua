local function flash(speed, time)
    -- speed: flashes/sec
    -- time: seconds
    local num_flashes = speed * time
    local delay = (time * 1000) / (num_flashes * 2)

    for i = 1, num_flashes do
        Lighting.SetAllEnabled(true)
        os.sleep(delay)
        Lighting.SetAllEnabled(false)
        os.sleep(delay)
    end
end

return flash