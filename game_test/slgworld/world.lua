require "util"
local world_mgr = require "world_mgr"

local world
local world_cfg = {
    len = 1000,
    wid = 1000,
    cbs = {
        arrive_cb = function(troopid)
            print("arrive", troopid, rawdump(world.troops[troopid]))
        end,
        collision_cb = function(troop1id, troop2id)
            print("collision", troop1id, troop2id)
        end,
        view_cb = function(view)
            for wid, obj in pairs(view) do
                print(wid, rawdump(obj))
            end
        end
    }
}

world = world_mgr.create_lworld(world_cfg.len, world_cfg.wid)

local init_world = function()
    -- world.cbs.arrive_cb = function(troopid)
    -- end

    -- world.cbs.collision_cb = function(troop1id, troop2id)
    -- end

    for name, cb in pairs(world_cfg.cbs) do
        local wcb = world.cbs[name]
        if wcb then
            world.cbs[name] = function(...)
                wcb(...)
                cb(...)
            end
        else
            world.cbs[name] = cb
        end
    end
end
init_world()

return {
    world = world
}
