local require = require
require "util"
local print_v, print, dump = print_v, print, dump
local syn = require "syn"

local players = {}
local dirtys = {
    objs = players,
    push_update = function(id, update)
        print("push update", id, dump(update))
    end,
    push_delete = function(id, delete)
        print("push delete", id, dump(delete))
    end
}
local player_syn = syn.create_obj_syn(dirtys)
local player = {
    id = 1,
    name = "haha"
}
player = player_syn.create_syn(player, 1)
players[1] = player

player.item = {
    [100] = {
        id = 100,
        num = 100
    }
}
dirtys.updates = nil
player.item[100].num = 200
print_v(dirtys.updates)
player.item[100] = nil
print_v(dirtys.deletes)
