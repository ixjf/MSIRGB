local Timer = {}

Timer.DefaultInterval = 3000 -- 3 seconds

function Timer.Set(f, ms) -- f should not be a long running function, but either way, ms is time (in milliseconds)
    -- between complete executions of f (so if f takes ~ ms or > ms, f will be called again after ms have ellapsed since the last
    -- return from f)
    while true do
        f()
        os.sleep(ms)
    end
end

return Timer