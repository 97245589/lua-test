local type, setmetatable, getmetatable, rawget, rawset = type, setmetatable, getmetatable, rawget, rawset
local table, pairs, next, string = table, pairs, next, string
local tmove, tinsert, ssub = table.move, table.insert, string.sub
local clone, split = clone, split

local path_cache = {
    [''] = {}
}

local fill_dirty_obj = function(obj, dirty_obj, patharr)
    print("fill", dump(patharr), dump(obj))
    for i = 1, #patharr do
        local nk = patharr[i]
        obj = obj[nk]
        if not dirty_obj[nk] then
            local objnid = obj.id
            if objnid == nk then
                dirty_obj[nk] = {
                    id = objnid
                }
            else
                dirty_obj[nk] = {}
            end
        end
        dirty_obj = dirty_obj[nk]
    end
    return dirty_obj
end
local dirty_delete = function(dirtys, id, patharr, k)
    local deletes = dirtys.deletes
    local objs = dirtys.objs
    if not deletes[id] then
        deletes[id] = {}
    end
    local delete_obj = fill_dirty_obj(objs[id], deletes[id], patharr)
    delete_obj[k] = {
        id = k
    }
    local updates = dirtys.updates
    if updates[id] then
        dirtys.push_update(id, updates[id])
        updates[id] = nil
    end
end
local dirty_update = function(dirtys, id, patharr, k, v)
    local updates = dirtys.updates
    local deletes = dirtys.deletes
    local objs = dirtys.objs
    if not updates[id] then
        updates[id] = {}
    end
    local update_obj = fill_dirty_obj(objs[id], updates[id], patharr)
    if type(v) == "table" then
        v = clone(v)
    end
    update_obj[k] = v
    if deletes[id] then
        dirtys.push_delete(id, deletes[id])
        deletes[id] = nil
    end
end
local newindexcb = function(dirtys, id, path, k, v)
    if not dirtys.updates then
        dirtys.updates = {}
    end
    if not dirtys.deletes then
        dirtys.deletes = {}
    end
    if not path_cache[path] then
        path_cache[path] = split(path, "|")
    end
    local patharr = path_cache[path]
    if v == nil then
        dirty_delete(dirtys, id, patharr, k)
    else
        dirty_update(dirtys, id, patharr, k, v)
    end
end

local create_path = function(parent_path, name)
    if not parent_path then
        return ""
    end

    return parent_path .. "|" .. name
end

local M = {}
M.create_obj_syn = function(dirtys)
    local m = {}
    local newindex = function(tb, k, v)
        if type(k) == "number" or ssub(k, #k) ~= "_" then
            k = k .. "_"
        end
        if type(v) == "table" and not v.__ID then
            v = m.create_syn(v, tb.__ID, tb.__PATH, k)
        end
        rawset(tb, k, v)
        return k, v
    end
    local MT = {
        __index = function(tb, k)
            k = k .. "_"
            local val = rawget(tb, k)
            if "table" == type(val) and not val.__ID then
                rawset(tb, k, m.create_syn(val, tb.__ID, tb.__PATH, k))
            end
            return rawget(tb, k)
        end,
        __newindex = function(tb, k, v)
            k, v = newindex(tb, k, v)
            newindexcb(dirtys, tb.__ID, tb.__PATH, k, v)
        end
    }
    m.create_syn = function(obj, id, parent_path, name)
        if getmetatable(obj) then
            return obj
        end
        if obj.__ID then
            return setmetatable(obj, MT)
        end

        local newobj = {
            __PATH = create_path(parent_path, name),
            __ID = id
        }
        for k, v in pairs(obj) do
            newindex(newobj, k, v)
        end
        return setmetatable(newobj, MT)
    end
    return m
end

return M
