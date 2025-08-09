local world = require"world".world

local test = function()
    world.enter_troop(10, {
        sx = 0,
        sy = 0,
        ex = 5,
        ey = 0
    })
    world.enter_troop(20, {
        sx = 5,
        sy = 0,
        ex = 0,
        ey = 0
    })
    world.add_watch(1, 1, 1000)

    for i = 1, 10 do
        local view = world.get_view()
        if view then
            world.cbs.view_cb(view)
        end
        local ret = world.troop_tick(500)
        if ret.arrives then
            for _, id in ipairs(ret.arrives) do
                world.cbs.arrive_cb(id)
            end
        end
        if ret.collision then
            for i = 1, #ret.collision, 2 do
                local id1 = ret.collision[i]
                local id2 = ret.collision[i + 1]
                world.cbs.collision_cb(id1, id2)
            end
        end
    end
end
test()

local test_profile = function()
    local rand = math.random
    for i = 1, 10000 do
        world.enter_troop(i, {
            sx = rand(0, 99),
            sy = rand(0, 99),
            ex = rand(0, 99),
            ey = rand(0, 99)
        })
    end

    local ret = world.troop_tick(500)
    -- print(rawdump(ret.collision))
    local t1 = os.time()
    for i = 1, 1000 do
        local ret = world.troop_tick(500)
        -- print(#ret.collision)
    end
    print(os.time() - t1)
end
-- test_profile()
