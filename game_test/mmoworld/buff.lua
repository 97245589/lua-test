local require, os, print, type = require, os, print, type
local table, pairs, ipairs = table, pairs, ipairs

local skill_env = require"skill".skill_env
local buffconfig = require"config".buff

skill_env.buff_attr = function(attr, val)
    local buff = skill_env.buff
    if not buff.attrs then
        buff.attrs = {}
    end

    local battrs = buff.attrs
    if not battrs[attr] then
        battrs[attr] = 0
    end
    battrs[attr] = battrs[attr] + val
end

local on_add_buff = function(buff)
    local me = skill_env.me
    local attr_cal = function()
        local battrs = buff.attrs
        if not battrs then
            return
        end

        local attrs = me.attrs
        for attr, val in pairs(battrs) do
            if not attrs[attr] then
                attrs[attr] = 0
            end
            attrs[attr] = attrs[attr] + val
        end
    end
    local buffid_lap = function()
        local bufflap = me.bufflap
        local buffid = buff.buffid
        if not bufflap[buffid] then
            bufflap[buffid] = 0
        end
        bufflap[buffid] = bufflap[buffid] + 1
    end
    local statelap = function()
        local statelap = me.statelap
        local buffid = buff.buffid

        local parse_type = function(tp)
            local btype = buffconfig[buffid][tp]
            if not btype then
                return
            end
            local t = type(btype)
            if t == "string" then
                if not statelap[btype] then
                    statelap[btype] = 0
                end
                statelap[btype] = statelap[btype] + 1
            elseif t == "table" then
                for _, v in ipairs(btype) do
                    if not statelap[v] then
                        statelap[v] = 0
                    end
                    statelap[v] = statelap[v] + 1
                end
            end
        end
        parse_type("type1")
        parse_type("type2")
        parse_type("type3")
    end

    attr_cal()
    buffid_lap()
    statelap()
end

local on_del_buff = function(buff)
    local me = skill_env.me
    local attr_cal = function()
        local battrs = buff.attrs
        if not battrs then
            return
        end

        local attrs = me.attrs
        for attr, val in pairs(battrs) do
            attrs[attr] = attrs[attr] - val
        end
    end
    local buffid_lap = function()
        local bufflap = me.bufflap
        local buffid = buff.buffid
        bufflap[buffid] = bufflap[buffid] - 1
    end
    local statelap = function()
        local statelap = me.statelap
        local buffid = buff.buffid

        local parse_type = function(tp)
            local btype = buffconfig[buffid][tp]
            if not btype then
                return
            end
            local t = type(btype)
            if t == "string" then
                statelap[btype] = statelap[btype] - 1
            elseif t == "table" then
                for _, v in ipairs(btype) do
                    statelap[v] = statelap[v] - 1
                end
            end
        end
        parse_type("type1")
        parse_type("type2")
        parse_type("type3")
    end

    attr_cal()
    buffid_lap()
    statelap()
end

local add_buff_check = function(me, buffid)
    return true
end

skill_env.add_buff = function(me, buffid, last_tm, params, src)
    if not add_buff_check(me, buffid) then
        return
    end
    local bme, btarg = skill_env.me, skill_env.targ
    local bbuff, bp = skill_env.buff, skill_env.p

    skill_env.me, skill_env.targ = me, skill_env.get_targ(me)
    local buff_uid = skill_env.gen_uid(me.buffs, buffid, "buffs")

    local buff = {
        buff_uid = buff_uid,
        buffid = buffid,
        params = params,
        end_tm = os.time() + last_tm,
        src = src and src.id or me.id
    }

    skill_env.buff, skill_env.p = buff, params
    local func = buffconfig[buffid].action
    if func then
        func()
    end

    me.buffs[buff_uid] = buff
    on_add_buff(buff)

    skill_env.me, skill_env.targ = bme, btarg
    skill_env.buff, skill_env.p = bbuff, bp
end

skill_env.tick_buff = function(me, now_tm)
    local mebuffs = me.buffs
    if not mebuffs then
        return
    end

    local bme, btarg = skill_env.me, skill_env.targ
    local bbuff, bp = skill_env.buff, skill_env.p

    skill_env.me, skill_env.targ = me, nil
    for buffuid, buff in pairs(mebuffs) do
        if now_tm > buff.end_tm then
            on_del_buff(buff)
            mebuffs[buffuid] = nil
            goto END
        end

        skill_env.buff = buff
        skill_env.p = buff.params
        local tick_func = buffconfig[buff.buffid].tick_action
        if tick_func then
            tick_func()
        end
        ::END::
    end

    skill_env.me, skill_env.targ = bme, btarg
    skill_env.buff, skill_env.p = bbuff, bp
end
