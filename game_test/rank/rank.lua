local require, os, pairs, math = require, os, pairs, math
local setmetatable = setmetatable
require "util"
local print, rawdump = print, rawdump
local lrank = require "lrank"

local enums = {
    rank_type1 = 1,
    rank_type2 = 2,

    rank_event_type1 = 1
}

local rank_events = {}

local mgr = {}

mgr.create_rank = function(rankid, event, num, db_rank)
    local ret = {
        rankid = rankid,
        event = event
    }
    local rank_core = lrank.create_lrank(num)
    ret.add_rank = function(id, score, time)
        rank_core:add_rank(id, score, time or os.time())
    end
    ret.info = function(num, me_id)
        num = num or 1000
        return rank_core:rank_info(num, me_id)
    end
    ret.dump = function()
        print(rank_core:dump())
    end
    ret.db_data = function()
        return rank_core:db_data()
    end
    ret.close = function()
        print("rank close", rankid)
        if event then
            rank_events[event][rankid] = nil
        end
    end

    if db_rank then
        for i = 1, #db_rank, 3 do
            local id = db_rank[i]
            local score = db_rank[i + 1]
            local time = db_rank[i + 2]
            ret.add_rank(id, score, time)
        end
    end

    if event then
        if not rank_events[event] then
            rank_events[event] = {}
        end
        rank_events[event][rankid] = ret
    end

    return ret
end

mgr.trigger_rank = function(event, id, score)
    local rank_event = rank_events[event]
    if not rank_event then
        return
    end
    for rankid, rank in pairs(rank_event) do
        rank.add_rank(id, score)
    end
end

local test = function()
    local random = math.random
    local rank = mgr.create_rank(1, 1, 1000)
    local t1 = os.time()
    for i = 1, 1000000 do
        rank.add_rank(random(2000), random(100), i)
    end
    print(os.time() - t1)

    local info = rank.info()
    print(info.me_rank, #info.ranks)
    print(#rank.db_data())

    info = rank.info(10, 100)
    print(rawdump(info))
end
-- test()

local test1 = function()
    local random = math.random
    local rank = mgr.create_rank(enums.rank_type1, enums.rank_event_type1, 20)
    -- rank.close()
    for i = 1, 1000 do
        mgr.trigger_rank(enums.rank_event_type1, random(50), random(20))
    end
    rank.dump()
end
test1()
