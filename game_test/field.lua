require "util"

local pack = function(v)
    return v
end

local unpack = function(v)
    return v
end

local fields = {
    friends = ""
}

local pack_test = function()
    local pack_data = function(player)
        local ret = {}
        for k, v in pairs(fields) do
            local val = player[k]
            player[k] = nil
            ret[k] = val
        end
        ret.data = player
        return ret
    end

    local player = {
        role = {
            photoid = 10
        },
        pets = {
            [10] = {
                petid = 10
            }
        },
        friends = {
            [111] = {
                friendid = 111
            }
        },
        playerid = 10,
        acc = "acc"
    }
    return pack_data(player)
end
local db_player = pack_test()
print("db_player", dump(db_player))

local unpack_test = function(dbplayer)
    local handler = {
        getid = function(player)
            return player.playerid
        end,
        getacc = function(player)
            return player.acc
        end
    }

    local fill_filed = function(player, k)
        local bin = player.bin
        local v = bin[k]
        if v then
            player[k] = unpack(v)
            bin[k] = nil
        end
    end

    local fill_all = function(player)
        local bin = player.bin
        local data = unpack(bin.data)
        for k, v in pairs(data) do
            player[k] = v
        end
        bin.data = nil
        for k, v in pairs(bin) do
            if not rawget(player, k) then
                player[k] = unpack(v)
            end
            if not fields[k] then
                -- hdel
            end
        end
        player.bin = nil
    end

    local fill_data = function(player, k)
        local bin = player.bin

        local fv = fields[k]
        local bv = bin[k]

        if fv and bv then
            fill_filed(player, k)
        else
            fill_all(player)
        end

        return player[k]
    end

    local player = {
        bin = db_player
    }

    setmetatable(player, {
        __index = function(player, k)
            local func = handler[k]
            if func then
                return function()
                    return func(player, k)
                end
            end
            fill_data(player, k)
            return player[k]
        end
    })
    return player
end

local player = unpack_test(db_player)
print(dump(player))

local role = player.role
print(dump(player))

-- local friends = player.friends
-- print(dump(player))
-- local role = player.role
-- print(dump(player))
