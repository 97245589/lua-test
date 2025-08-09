local require, type, print = require, type, print
local pairs, next = pairs, next
require "util"
local print_v, dump, clone = print_v, dump, clone
local syn = require "syn1"

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

local push_update = function(id, update)
    print("push update", id, dump(update))
end
local push_delete = function(id, delete)
    print("push delete", id, dump(delete))
end
local players = {}
local dirtys = {
    updates = nil,
    deletes = nil,
    objs = players,
    push_update = push_update,
    push_delete = push_delete
}
local tick_dirtys = function()
    -- print("tick_dirtys", dump(dirtys.updates), dump(dirtys.deletes))
    for id, update in pairs(dirtys.updates) do
        push_update(id, update)
    end
    for id, delete in pairs(dirtys.deletes) do
        push_delete(id, delete)
    end
    dirtys.updates = nil
    dirtys.deletes = nil
end

local player_syn = syn.create_obj_syn(dirtys)
local player = {}
player = player_syn.create_syn(player, 100)
players[100] = player

player.obj = {
    id = 10,
    name = "hello"
}
player.map = {
    [10] = {
        id = 10,
        num = 10
    }
}
player.map[10] = nil
tick_dirtys()
