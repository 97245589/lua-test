local require, math, load, error, string = require, math, load, error, string
local table, pairs, ipairs = table, pairs, ipairs

local skill_env = {
    rand = math.random,
    me = nil,
    targ = nil,
    depth = 0,
    buff = nil,
    p = nil,
    world = nil,
    battle_info = {
        id = 0,
        buffs = {},
        bufflap = {},
        statelap = {},
        attrs = {
            hp = 10,
            atk = 10,
            def = 0
        }
    }
}

local cfg = require "config"
local skill_cfg = cfg.skill
local buff_cfg = cfg.buff

local init_cfg = function()
    local init_skill_cfg = function()
        for skill_id, val in pairs(skill_cfg) do
            local action = val.action
            val.action = load(action, "skill_cfg skillid:" .. skill_id, "t", skill_env)
        end
    end
    local init_buff_cfg = function()
        for buffid, val in pairs(buff_cfg) do
            local action = val.action
            if action then
                val.action = load(action, "buff_cfg action buffid:" .. buffid, "t", skill_env)
            end
        end
    end
    init_skill_cfg()
    init_buff_cfg()
end
init_cfg()

skill_env.start_skill = function(me, skillid, params)
    local world = skill_env.world
    if not world then
        return
    end
    local start_skill_cb = world.cbs.start_skill_cb
    if start_skill_cb and not start_skill_cb() then
        return
    end

    local the_cfg = skill_cfg[skillid]
    params = params or the_cfg.params
    if not the_cfg.targ then
        skill_env.exec_skill(me, nil, skillid, params)
        return
    end

    local targs = world.find_targ(me.id, table.unpack(the_cfg.targ))
    if not targs then
        return
    end
    for _, targid in ipairs(targs) do
        local targ = world.players[targid]
        if targ then
            skill_env.exec_skill(me, targ, skillid, params)
        end
    end
end

skill_env.exec_skill = function(me, targ, skillid, params)
    local bme, btarg = skill_env.me, skill_env.targ
    local bp = skill_env.p

    skill_env.depth = skill_env.depth + 1
    skill_env.me, skill_env.targ = me, targ

    local the_cfg = skill_cfg[skillid]
    skill_env.p = params

    local func = the_cfg.action
    func()

    skill_env.p = bp
    skill_env.depth = skill_env.depth - 1
    skill_env.me, skill_env.targ = bme, btarg
end

skill_env.gen_uid = function(obj, tid, info)
    local num_max = 1000
    local uid = tid * num_max
    for i = 1, num_max do
        uid = uid + 1
        if not obj[uid] then
            return uid
        end
        if i == num_max then
            error(string.format("uid too max %s %d %d", info, tid, num_max))
            return
        end
    end
end

skill_env.get_targ = function(me)
    if me == skill_env.me then
        return skill_env.targ
    else
        return skill_env.me
    end
end

skill_env.get_attr = function(side, attr)
    local attrs = side.attrs
    return attrs[attr] or 0
end

skill_env.get_final_attr = function(side, attr)
    local v = skill_env.get_attr(side, attr)
    local diff = skill_env.get_attr(side, attr .. "_diff")
    local per = skill_env.get_attr(side, attr .. "_per")
    return (v + diff) * (1 + per)
end

skill_env.formu_harm = function(me, targ, rate)
    rate = rate or 1
    local m_atk = skill_env.get_final_attr(me, "atk")
    local t_def = skill_env.get_final_attr(targ, "def")
    local decr = m_atk - t_def
    -- print(decr, rate)
    decr = decr * rate
    targ.attrs.hp = targ.attrs.hp - decr

    local world = skill_env.world
    local harm_cb = world.cbs.harm_cb
    if harm_cb then
        harm_cb(me, targ, decr)
    end
end

skill_env.real_harm = function(me, val)
    me.attrs.hp = me.attrs.hp - val
end

return {
    skill_env = skill_env
}
