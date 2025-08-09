local require, load, print, rawdump = require, load, print, rawdump
local table, pairs, error, string = table, pairs, error, string

local skill_env = {
    me = nil,
    targ = nil,
    depth = 0,
    buff = nil,
    p = nil
}
local cfg = require "config"
local skill_cfg, buff_cfg = cfg.skill, cfg.buff

local init_cfg = function()
    for skill_id, val in pairs(skill_cfg) do
        local action = val.action
        if action then
            val.action = load(action, "skill_cfg--> skillid:" .. skill_id, "t", skill_env)
        end
    end

    for buff_id, val in pairs(buff_cfg) do
        local a_str = val.action
        if a_str then
            val.action = load(a_str, "action buff_cfg buffid:" .. buff_id, "t", skill_env)
        end

        local t_str = val.tick_action
        if t_str then
            val.tick_action = load(t_str, "tick_action buff_cfg buffid:" .. buff_id, "t", skill_env)
        end
    end
end
init_cfg()

local death_check = function(side)
    return side.attrs.hp <= 0
end

skill_env.exec_skill = function(me, targ, skillid, params)
    local the_cfg = skill_cfg[skillid]
    local func = the_cfg.action
    if not func then
        return
    end
    skill_env.env_restore(function()
        skill_env.depth = skill_env.depth + 1
        if skill_env.depth > 2 then
            return
        end
        print("exec_skill", me.id, skillid, skill_env.depth)
        skill_env.me = me
        skill_env.targ = targ
        local p = params or the_cfg.params[1]
        skill_env.p = p
        func()
    end)
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

skill_env.trigger_eve = function(me, eve, targ)
    local buffids = me.eve[eve]
    if not buffids then
        return
    end

    for _, params_arr in pairs(buffids) do
        for _, arr in pairs(params_arr) do
            local func = arr[1]
            if func == skill_env.exec_skill then
                print("trigger_eve - exec_skill", skill_env.depth)
                func(me, targ, table.unpack(arr, 4))
            else
                func(table.unpack(arr, 2))
            end
        end
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

skill_env.cause_harm = function(me, targ, rate)
    rate = rate or 1
    local m_atk = skill_env.get_final_attr(me, "atk")
    local t_def = skill_env.get_final_attr(targ, "def")
    local decr = m_atk - t_def
    decr = decr * rate
    if decr > 0 then
        targ.attrs.hp = targ.attrs.hp - decr
    end
    print("cause_harm", me.id, targ.id, decr, targ.attrs.hp)
    skill_env.trigger_eve(me, "damage", targ)
    skill_env.trigger_eve(targ, "damaged", me)
end

skill_env.real_harm = function(me, val)
    if val > 0 then
        me.attrs.hp = me.attrs.hp - val
    end
end

skill_env.env_restore = function(cb)
    local tmp = {
        me = skill_env.me,
        targ = skill_env.targ,
        depth = skill_env.depth,
        buff = skill_env.buff,
        p = skill_env.p
    }
    cb()
    for k, v in pairs(tmp) do
        skill_env[k] = v
    end
end

skill_env.reset = function()
    skill_env.me = nil
    skill_env.targ = nil
    skill_env.depth = 0
    skill_env.buff = nil
    skill_env.p = nil
end

return {
    skill_env = skill_env
}
