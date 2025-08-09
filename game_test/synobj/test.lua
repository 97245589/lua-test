local require, type = require, type
local pairs, next = pairs, next
require "util"
local print_v, dump, clone = print_v, dump, clone
local syn = require "syn"

local handle_dirty
handle_dirty = function(tb)
    for k, v in pairs(tb) do
        if type(v) == "table" then
            tb[k] = handle_dirty(v)
        end
    end
    if next(tb) then
        return tb
    else
        return nil
    end
end

local fill_update
fill_update = function(p, update)
    for k, v in pairs(update) do
        if type(v) == "table" and type(p[k]) == "table" then
            fill_update(p[k], v)
        else
            p[k] = v
        end
    end
end
local is_delete_ele = function(k, tb)
    if k ~= tb.id then
        return false
    end
    local i = 0
    for k, v in pairs(tb) do
        i = i + 1
        if i > 1 then
            return false
        end
    end
    return true
end
local fill_delete
fill_delete = function(p, delete)
    for k, v in pairs(delete) do
        if is_delete_ele(k, v) then
            p[k] = nil
        else
            fill_delete(p[k], v)
        end
    end
end

local players = {}
local dirtys = {
    objs = players
}
local player_syn = syn.create_obj_syn(dirtys)
local player = {
    item = {}
}
player = player_syn.create_syn(player, 100)
players[100] = player

local item = player.item
item[100] = {
    id = 100,
    map = {
        [10] = {
            id = 10
        },
        [20] = {
            id = 20
        }
    }
}

local test1 = function()
    item[100].map[20] = nil
    -- print(dump(dirtys.updates, "updates"), dump(dirtys.deletes, "deletes"))
    local player1 = clone(player)
    fill_update(player1, dirtys.updates[100])
    fill_delete(player1, dirtys.deletes[100])
    print_v(player1)
end
test1()

local test2 = function()
    item[100] = nil
    item[100] = {
        id = 100
    }
    handle_dirty(dirtys)
    local player1 = {
        item = {}
    }
    fill_update(player1, dirtys.updates[100])
    print_v(player1)
end
-- test2()

