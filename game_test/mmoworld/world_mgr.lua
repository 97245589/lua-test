local require, print = require, print
local lworld = require "lworld"

local mgr = {}

mgr.create_world = function(len, wid)
    if not len or not wid then
        return
    end
    local core = lworld.create_lworld(len, wid)

    local world = {
        tick_tm = nil,
        players = {},
        cbs = {
            enter_cb = nil,
            leave_cb = nil,
            start_skill_cb = nil,
            harm_cb = nil,
            tick_cb = nil
        },
        info = nil
    }

    world.get_len_wid = function()
        return core:get_len_wid()
    end
    world.set_block = function(x, y, m)
        return core:set_block(x, y, m)
    end
    world.update_pos = function(id, mode, px, py, dx, dy, speed)
        return core:update_pos(id, mode, px, py, dx, dy, speed)
    end
    world.world_tick = function(diff_tm)
        return core:world_tick(diff_tm)
    end
    world.path_find = function(sx, sy, ex, ey, quick)
        return core:path_find(sx, sy, ex, ey, quick)
    end

    --[[ ... 
        1, r  --circular
        2, r, ang --sector
        3 ,len, wid --rectange
    ]]
    world.find_targ = function(playerid, ...)
        return core:find_targ(playerid, ...)
    end

    --[[
        player_info = {id = 0, mode = "wm"
        px = 0, py = 0, dx = 0, dy = 0, speed = 0}
    ]]
    world.player_enter = function(playerid, player_info)
        if not player_info.mode then
            print("player enter error no mode", playerid)
            return
        end
        player_info.id = playerid
        local enter_cb = world.cbs.enter_cb
        if enter_cb and not enter_cb(playerid, player_info) then
            return
        end

        world.players[playerid] = player_info
        local id, mode, px, py, dx, dy, speed = playerid, player_info.mode, player_info.px, player_info.py,
            player_info.dx, player_info.dy, player_info.speed
        world.update_pos(id, mode, px, py, dx, dy, speed)
    end

    world.player_leave = function(playerid)
        world.players[playerid] = nil
        local leave_cb = world.cbs.leave_cb
        if leave_cb then
            leave_cb(playerid)
        end
        world.update_pos(playerid, "d", 0, 0)
    end
    return world
end

return mgr
