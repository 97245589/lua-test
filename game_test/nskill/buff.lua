local require, os, print, type = require, os, print, type
local table, pairs, ipairs, next = table, pairs, ipairs, next

local skill_env = require"skill".skill_env
local config = require "config"
local buffconfig = config.buff

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

skill_env.is_bufftype_exist = function(me, bufftype)
    local statelap = me.statelap
    local lap = statelap[bufftype]
    if lap and lap > 0 then
        return true
    end
    return false
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
    local del_eve = function()
        local buffeve = buff.eve
        if not buffeve then
            return
        end
        if me.eve[buffeve] then
            me.eve[buffeve][buff.buff_uid] = nil
            if not next(me.eve[buffeve]) then
                me.eve[buffeve] = nil
            end
        end
    end

    attr_cal()
    buffid_lap()
    statelap()
    del_eve()
end

local add_buff_check = function(me, buffid)
    return true
end

skill_env.add_buff = function(me, buffid, last_tm, params, src)
    if not add_buff_check(me, buffid) then
        return
    end
    src = src or me
    skill_env.env_restore(function()
        skill_env.me, skill_env.targ = me, src
        local buff_uid = skill_env.gen_uid(me.buffs, buffid, "buffs")

        local buff = {
            buff_uid = buff_uid,
            buffid = buffid,
            params = params,
            end_tm = skill_env.round + last_tm,
            src = src.id
        }

        print("addbuff", me.id, src.id, buffid, last_tm, params and table.unpack(params))
        skill_env.buff, skill_env.p = buff, params
        local func = buffconfig[buffid].action
        if func then
            func()
        end

        me.buffs[buff_uid] = buff
        on_add_buff(buff)
    end)
end

skill_env.remove_buff = function(me, buffuid)
    local buff = me.buffs[buffuid]
    if not buff then
        return
    end

    on_del_buff(buff)
    me.buffs[buffuid] = nil
end

skill_env.tick_buff = function(me, now_tm)
    local mebuffs = me.buffs
    if not mebuffs then
        return
    end

    skill_env.env_restore(function()
        skill_env.me = me
        for buffuid, buff in pairs(mebuffs) do
            skill_env.targ = buff.src
            if now_tm > buff.end_tm then
                skill_env.remove_buff(me, buffuid)
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
    end)
end

skill_env.reg_eve = function(eve, ...)
    local me = skill_env.me
    if not me.eve[eve] then
        me.eve[eve] = {}
    end
    local buff = skill_env.buff
    buff.eve = eve
    me.eve[eve][buff.buff_uid] = {...}
end
