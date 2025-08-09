local table, pairs, ipairs, math, print = table, pairs, ipairs, math, print
local load, require, error, string = load, require, error, string

local skill_env = {
    table = table,
    round = 0,
    me = nil,
    targ = nil,
    depth = 0,
    buff = nil,
    p = nil,
    units = nil,
    team_a = nil,
    team_b = nil,
    melee = nil, -- 当前技能是否是群攻技能
    max_round = 5,
    atk_c = 0.5,
    def_c = 0.4,
    dmg_c = 1.2,
    crit_c = 1,
    break_def_c_ = 0.02, -- 未破防系数
    harm_float = {-5, 5}
}

local cfg = require "config"
local skill_cfg = cfg.skill
local buff_cfg = cfg.buff
local init_cfg = function()
    for skill_id, val in pairs(skill_cfg) do
        local action = val.action
        if action then
            val.action = load(action, "skillcfg action" .. skill_id, "t", skill_env)
        end
    end

    for buffid, val in pairs(buff_cfg) do
        local action = val.action
        if action then
            val.action = load(action, "buffcfg action" .. buffid, "t", skill_env)
        end
    end
end
init_cfg()

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

skill_env.get_attr = function(unit, attr)
    local attrs = unit.attrs
    return attrs[attr] or 0
end

skill_env.get_final_attr = function(unit, attr)
    local v = skill_env.get_attr(unit, attr)
    local diff = skill_env.get_attr(unit, attr .. "_diff")
    local per = skill_env.get_attr(unit, attr .. "_per")
    return (v + diff) * (1 + per)
end

skill_env.attr_probability = function(me, targ, attr)
    local diff = skill_env.attr_diff_val(me, targ, attr)

    if diff <= 0 then
        return false
    end
    local f = math.random()
    if f < diff then
        print("attr_probability succ", attr)
        return true
    end
end

skill_env.attr_diff_val = function(me, targ, attr)
    local anti_attr = "anti_" .. attr
    local me_attr = skill_env.get_attr(me, attr)
    local targ_dodge = skill_env.get_attr(targ, anti_attr)
    local diff = me_attr - targ_dodge
    return diff
end

local harm_formular = function(me, targ, input)
    local fdamage = input.fdamage or 0
    local damage_c = input.damage_c or 1

    local overwhelmed = false
    local atk_val = skill_env.get_final_attr(me, "atk")
    local def_val = skill_env.get_final_attr(targ, "def")
    if atk_val > def_val then
        overwhelmed = true
    end

    local harm_val
    if overwhelmed then
        harm_val = skill_env.dmg_c
        harm_val = harm_val * damage_c
        harm_val = harm_val * (atk_val * skill_env.atk_c - def_val * skill_env.def_c)
        local damage_diff = skill_env.get_attr(me, "damage") - skill_env.get_attr(targ, "damaged")
        if damage_diff > 0 then
            harm_val = harm_val * (1 + damage_diff)
        end
        harm_val = harm_val + fdamage
    else
        harm_val = atk_val
        harm_val = harm_val * damage_c
        harm_val = harm_val * skill_env.atk_c + skill_env.break_def_c_
    end

    local harm_float = skill_env.harm_float
    local f = math.random(harm_float[1], harm_float[2]) / 100
    harm_val = harm_val * (1 + f)
    return harm_val
end

skill_env.harm_formular = function(me, targ, input)
    local no_suck = input.no_suck
    local no_counterattack = input.no_counterattack
    local no_hit = input.no_hit
    local no_dodge = input.no_dodge
    local no_crit = input.no_crit
    local no_stun = input.no_stun
    if skill_env.melee then
        no_counterattack = 1
        no_hit = 1
        no_stun = 1
    end

    if not no_dodge and skill_env.attr_probability(targ, me, "dodge") then
        return
    end

    local harm_val = harm_formular(me, targ, input)

    if not no_crit and skill_env.attr_probability(me, targ, "crit") then
        harm_val = harm_val * (1 + skill_env.crit_c)
    end
    skill_env.real_harm(me, targ, harm_val)

    if not no_suck then
        local suck_rate = skill_env.attr_diff_val(me, targ, "suck")
        if suck_rate > 0 then
            local add_hp = harm_val * (1 + suck_rate)
            skill_env.add_hp(me, add_hp)
        end
    end

    if not no_hit and skill_env.attr_probability(me, targ, "hit") then
        skill_env.exec_targ_skill(me, targ, 1, {
            no_hit = 1,
            no_dodge = 1
        })
    end

    if not no_counterattack and skill_env.attr_probability(targ, me, "counterattack") then
        skill_env.exec_targ_skill(targ, me, 1, {
            no_hit = 1,
            no_dodge = 1
        })
    end

    if not no_stun and skill_env.attr_probability(me, targ, "stun") then
        skill_env.add_buff(targ, 2, 1, nil, me)
    end
end

skill_env.real_harm = function(src, dst, val)
    local attrs = dst.attrs
    attrs.hp = attrs.hp - val
    print("harm ----", src.id, dst.id, val, attrs.hp)
end

skill_env.is_enemy = function(me, targ)
    if me.id < 100 then
        if targ.id < 100 then
            return false
        end
    elseif me.id > 100 then
        if targ.id > 100 then
            return false
        end
    end
    return true
end

skill_env.add_hp = function(unit, hp)
    local attrs = unit.attrs
    attrs.hp = attrs.hp + hp
end

skill_env.is_die = function(unit)
    local attrs = unit.attrs
    if not attrs.hp then
        return true
    end
    if attrs.hp <= 0 then
        return true
    end
    return false
end

skill_env.start_skill = function(team_a, team_b)
    skill_env.units = {}
    skill_env.team_a = {}
    skill_env.team_b = {}
    local set_team = function(team, name)
        local tarr = skill_env[name]
        for id, unit in pairs(team) do
            skill_env.units[unit.id] = unit
            table.insert(tarr, unit)
        end
        table.sort(tarr, function(lhs, rhs)
            return lhs.id < rhs.id
        end)
    end
    set_team(team_a, "team_a")
    set_team(team_b, "team_b")
    skill_env.passive_skill()
    skill_env.round_skill()
    skill_env.env_reset()
end

skill_env.params_targets = function(id, is_me, is_forward, num)
    local team
    local pick_team = function()
        local info
        if id < 100 then
            if is_me then
                info = skill_env.team_a
            else
                info = skill_env.team_b
            end
        else
            if is_me then
                info = skill_env.team_b
            else
                info = skill_env.team_a
            end
        end
        for _, unit in ipairs(info) do
            if not skill_env.is_die(unit) then
                team = team or {}
                table.insert(team, unit)
            end
        end
    end
    pick_team()
    if not team then
        return
    end

    local ret = {}
    if num >= #team then
        for i = 1, #team do
            local unit = team[i]
            ret[unit.id] = unit
        end
        return ret
    end

    if is_forward then
        for i = 1, num do
            local unit = team[i]
            ret[unit.id] = unit
        end
        return ret
    else
        for i = num, 1, -1 do
            local unit = team[i]
            ret[unit.id] = unit
        end
        return ret
    end
end

skill_env.get_targets = function(id, skillid)
    local tcfg = skill_cfg[skillid]
    local cfgtargets = tcfg.targets
    local ttype = tcfg.type
    local params_targets = skill_env.params_targets

    if not cfgtargets then
        if ttype == "passive" or ttype == "active_buff" then
            return {skill_env.units[id]}
        end
        if ttype == "active_harm" then
            return params_targets(id, false, 1, 1)
        end
        return
    end

    local is_me = cfgtargets.me
    local is_forward = cfgtargets.forward
    local num = cfgtargets.num
    return params_targets(id, is_me, is_forward, num)
end

skill_env.exec_targ_skill = function(me, targ, skillid, params)
    local tcfg = skill_cfg[skillid]
    skill_env.me = me
    skill_env.targ = targ
    skill_env.p = params
    local action = tcfg.action
    action()
end

skill_env.exec_skill = function(me, skillid, params)
    skill_env.env_restore(function()
        skill_env.depth = skill_env.depth + 1
        if skill_env.depth > 2 then
            return
        end
        local scfg = skill_cfg[skillid]
        if not scfg.action then
            return
        end
        if skill_env.is_bufftype_exist(me, "stun") then
            return
        end

        local ctargets = scfg.targets
        if ctargets and ctargets.num and ctargets.num > 1 then
            skill_env.melee = true
        end
        local targs = skill_env.get_targets(me.id, skillid)
        print("exec_skill", me.id, skillid, skill_env.depth)
        if not targs then
            return
        end
        for id, targ in pairs(targs) do
            skill_env.exec_targ_skill(me, targ, skillid, params)
        end
    end)
end

skill_env.get_order = function()
    local arr = {}
    for id, unit in pairs(skill_env.units) do
        local speed = skill_env.get_final_attr(unit, "speed")
        table.insert(arr, {id, speed})
    end

    table.sort(arr, function(lhs, rhs)
        if lhs[2] ~= rhs[2] then
            return lhs[2] > rhs[2]
        end
        return lhs[1] < rhs[1]
    end)
    return arr
end

skill_env.exec_skill_type = function(me, stype)
    for skill_id, params in pairs(me.skills) do
        local stype_ = skill_cfg[skill_id].type
        if stype == stype_ then
            skill_env.exec_skill(me, skill_id, me.skills[skill_id])
        end
    end
end

skill_env.passive_skill = function()
    print("passive_skill start -----")
    local order = skill_env.get_order()
    for _, arr in ipairs(order) do
        local unit_id = arr[1]
        local me = skill_env.units[unit_id]

        skill_env.exec_skill_type(me, "passive")
    end
end

skill_env.one_round_skill = function()
    for id, unit in pairs(skill_env.units) do
        if not skill_env.is_die(unit) then
            skill_env.tick_buff(unit, skill_env.round)
        end
    end
    local order = skill_env.get_order()
    for _, arr in ipairs(order) do
        local unit_id = arr[1]
        local me = skill_env.units[unit_id]
        skill_env.exec_skill_type(me, "active_harm")
        skill_env.exec_skill_type(me, "active_buff")
    end
end

skill_env.round_skill = function()
    for i = 1, skill_env.max_round do
        print("roundskill start ---------", i)
        skill_env.round = skill_env.round + 1
        skill_env.one_round_skill()
    end
end

skill_env.trigger_eve = function(me, eve, targ)
    if skill_env.depth > 1 then
        return
    end

    local buffids = me.eve[eve]
    if not buffids then
        return
    end

    for _, params_arr in pairs(buffids) do
        for _, arr in pairs(params_arr) do
            local func = arr[1]
            if func == skill_env.exec_skill then
                print("trigger_eve - exec_skill")
                func(me, targ, table.unpack(arr, 4))
            else
                func(table.unpack(arr, 2))
            end
        end
    end
end

skill_env.env_restore = function(cb)
    local cp = {
        me = skill_env.me,
        targ = skill_env.targ,
        depth = skill_env.depth,
        buff = skill_env.buff,
        p = skill_env.p,
        melee = skill_env.melee
    }
    cb()
    for k, v in pairs(cp) do
        skill_env[k] = v
    end
end

skill_env.env_reset = function()
    skill_env.round = 0
    skill_env.me = nil
    skill_env.targ = nil
    skill_env.depth = 0
    skill_env.buff = nil
    skill_env.p = nil
    skill_env.units = nil
    skill_env.team_a = nil
    skill_env.team_b = nil
    skill_env.melee = nil
end

return {
    skill_env = skill_env
}
