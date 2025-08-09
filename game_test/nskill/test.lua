local require = require
require "util"
local print, rawdump, clone = print, rawdump, clone
local skill_env = require"skill".skill_env
require "buff"

local unit = {
    id = 1,
    buffs = {},
    bufflap = {},
    statelap = {},
    eve = {},
    attrs = {
        hp = 10,
        atk = 10,
        def = 0,
        speed = 10,
        suck = nil, -- 吸血
        anti_suck = nil, -- 反吸血
        counterattack = nil, -- 反击
        hit = nil, -- 连击
        dodge = nil, -- 闪避
        crit = nil, -- 暴击
        stun = nil, -- 击晕
        damage = nil, -- 伤害提升百分比
        damaged = nil -- 伤害减免百分比
    },
    skills = {
        [1] = {}
    }
}

local team1 = clone(unit)
team1.id = 1
team1.attrs = {
    hp = 10,
    atk = 10,
    def = 10,
    speed = 10,
    stun = 1
}
team1.skills = {
    [1001] = {}
}
local team2 = clone(unit)
team2.id = 2
team2.attrs = {
    hp = 10,
    atk = 10,
    def = 10,
    speed = 100
}
team2.skills = {
    [2001] = {"atk_diff", 10}
}
local team_a = {
    [1] = team1,
    [2] = team2
}

local team101 = clone(unit)
team101.id = 101
team101.attrs = {
    hp = 10,
    atk = 10,
    def = 10,
    speed = 1000
}
team101.skills = {
    [101001] = {}
}
local team102 = clone(unit)
team102.id = 102
team102.attrs = {
    hp = 10,
    atk = 10,
    def = 10,
    speed = 10
}
team102.skills = {
    [102001] = {}
}
local team_b = {
    [101] = team101,
    [102] = team102
}

skill_env.start_skill(team_a, team_b)
print(rawdump(team101))
