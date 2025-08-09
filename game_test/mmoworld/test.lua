require "util"
local world_mgr = require "world_mgr"
local skill = require "skill"
require "buff"
local skill_env = skill.skill_env

local world_cfg = {
    len = 20,
    wid = 20,
    cbs = {}
}
local world = world_mgr.create_world(world_cfg.len, world_cfg.wid)

local init_world = function()
    skill_env.world = world
    world.cbs.enter_cb = function(playerid, player_info)
        local battle_info = clone(skill_env.battle_info)
        for k, v in pairs(battle_info) do
            player_info[k] = player_info[k] or v
        end
        return true
    end
    world.cbs.harm_cb = function(src, dst, harm)
        print(src.id, dst.id, harm)
    end
    world.cbs.start_skill_cb = function()
        return true
    end
    world.cbs.leave_cb = function(playerid)
    end
    world.cbs.tick_cb = function()
    end

    for k, v in pairs(world_cfg.cbs) do
        if v then
            local wcb = world.cbs[k]
            if wcb then
                world.cbs[k] = function(...)
                    wcb(...)
                    v(...)
                end
            else
                world.cbs[k] = v
            end
        end
    end
end
init_world()

local test_path_find = function()
    local r = world.path_find(0, 0, 9, 9, 1)
    print(rawdump(r))
end
-- test_path_find()

local test_aoi = function()
    world.player_enter(100, {
        mode = "wm",
        px = 9,
        py = 9,
        dx = -1,
        dy = -1,
        speed = 1
    })
    world.world_tick(0)
    world.player_enter(200, {
        mode = "wm",
        px = 9,
        py = 9,
        dx = 1,
        dy = 1
    })
    for i = 1, 20 do
        local ret = world.world_tick(1000)
        print(rawdump(ret, i))
    end
end
-- test_aoi()

local test_skill = function()
    world.player_enter(100, {
        mode = "wm",
        px = 0,
        py = 0,
        dx = 1,
        dy = 0,
        attrs = {
            hp = 10,
            atk = 10,
            def = 0
        }
    })
    world.player_enter(200, {
        mode = "wm",
        px = 1,
        py = 0,
        attrs = {
            hp = 10,
            atk = 5,
            def = 5
        }
    })
    world.player_enter(300, {
        mode = "wm",
        px = 0,
        py = 1
    })
    world.world_tick(0)

    local me = world.players[100]
    local targ = world.players[200]
    -- skill_env.start_skill(me, 1)
    -- print(rawdump(targ))

    skill_env.start_skill(me, 1000)
    print(rawdump(me))
    skill_env.tick_buff(me, os.time() + 11)
    print(rawdump(me))
end
-- test_skill()

local test_ai = function()
    local ai_env = require"ai".ai_env
    ai_env.world = world
    ai_env.skill_env = skill_env

    world.player_enter(100, {
        aiid = 1,
        mode = "wm",
        px = 0,
        py = 0,
        speed = 1
    })
    local me = world.players[100]

    for i = 1, 20 do
        ai_env.me = me
        ai_env.exec_ai()
    end
end
test_ai()
