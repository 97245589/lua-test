local lworld = require "lworld"

local mgr = {}

mgr.create_lworld = function(len, wid)
    local core = lworld.create_lworld(len, wid)

    local world = {
        tick_tm = nil,
        entitys = {},
        troops = {},
        cbs = {}
    }

    world.get_len_wid = function()
        return core:get_len_wid()
    end
    world.set_block = function(x, y, id)
        return core:set_block(x, y, id)
    end
    world.get_block = function(x, y)
        return core:get_block(x, y)
    end
    world.path_find = function(sx, sy, ex, ey)
        return core:path_find(sx, sy, ex, ey)
    end
    world.get_empty_pos = function(ltx, lty, rbx, rby, size)
        return core:get_empty_pos(ltx, lty, rbx, rby, size)
    end
    world.add_entity = function(ltx, lty, rbx, rby, id)
        return core:add_entity(ltx, lty, rbx, rby, id)
    end
    world.update_entity = function(ltx, lty, rbx, rby, id)
        return core:update_entity(ltx, lty, rbx, rby, id)
    end
    world.remove_entity = function(ltx, lty, rbx, rby, id)
        return core:remove_entity(ltx, lty, rbx, rby, id)
    end
    world.add_watch = function(x, y, id)
        return core:add_watch(x, y, id)
    end
    world.delete_watch = function(id)
        return core:delete_watch(id)
    end
    world.get_view = function()
        return core:get_view()
    end
    world.dump = function()
        return core:dump()
    end
    world.dump_troop = function()
        return core:dump_troop()
    end
    world.add_troop = function(sx, sy, ex, ey, troopid)
        return core:add_troop(sx, sy, ex, ey, troopid)
    end
    world.troop_back = function(troopid)
        return core:troop_back(troopid)
    end
    world.delete_troop = function(troopid)
        return core:delete_troop(troopid)
    end
    world.troop_tick = function(diff_tm)
        return core:troop_tick(diff_tm)
    end
    world.entity_enter_world = function(world_id, obj)
        obj.id = world_id
        world.entitys[world_id] = obj
        world.add_entity(obj.ltx, obj.lty, obj.rbx, obj.rby, world_id)
    end
    world.enter_troop = function(troopid, obj)
        obj.id = troopid
        world.add_troop(obj.sx, obj.sy, obj.ex, obj.ey, troopid)
        world.troops[troopid] = obj
    end
    world.gen_world_id = function(ltx, lty, etype)
        return ltx << 32 | lty << 16 | etype
    end
    world.info_by_worldid = function(id)
        local ltx = id >> 32 | 0xffff
        local lty = id >> 16 | 0xffff
        local etype = id & 0xffff
        return ltx, lty, etype
    end

    return world
end

return mgr
