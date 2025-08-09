local require = require
require "buff"
local print, rawdump = print, rawdump
require "util"
local skill_env = require"skill".skill_env

--[[
local side = {
    id = 1,
    buffs = {}, --buff实体
    bufflap = {}, --buffid堆叠次数
    statelap = {}, --状态堆叠次数
    eve = {}, -- 事件
    attrs = {} --属性
}

local buff = {
    buff_uid = 1, buffid = 1, params = {}, end_tm=0, src=0 --buffid 参数 结束时间 来源
    attrs = {} --附加属性
}
]]

local me = {
    id = 1,
    buffs = {},
    bufflap = {},
    statelap = {},
    eve = {},
    attrs = {
        hp = 10,
        atk = 10,
        def = 0
    }
}

local targ = {
    id = 2,
    buffs = {},
    bufflap = {},
    statelap = {},
    eve = {},
    attrs = {
        hp = 10,
        atk = 5,
        def = 5
    }
}

require "skill"

local test1 = function()
    skill_env.exec_skill(me, targ, 1)
    print(rawdump(me))
end

local test2 = function()
    skill_env.exec_skill(me, targ, 2)
    print(rawdump(targ))
end

local test3 = function()
    skill_env.exec_skill(me, targ, 3)
    for i = 1, 2 do
        skill_env.tick_buff(targ, os.time() + i * 2)
    end
    print(rawdump(targ, "targ"))
end

local test4 = function()
    -- skill_env.exec_skill(targ, me, 41)
    skill_env.exec_skill(me, targ, 4)
    skill_env.exec_skill(me, targ, 2)
    -- print(rawdump(me, "me"))
    -- print(rawdump(targ, "targ"))
end

local test5 = function()
    skill_env.exec_skill(me, targ, 5)
    -- print(rawdump(me, "me"))

    skill_env.exec_skill(me, targ, 2)
    print(rawdump(me, "me"))
    print(rawdump(targ, "targ"))
end

test4()
