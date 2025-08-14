local tinsert, tconcat = table.insert, table.concat

local tdump = function(root)
    local cache = {
        [root] = "."
    }
    local function _dump(t, space, name)
        local temp = {}
        for k, v in pairs(t) do
            local key = tostring(k)
            if cache[v] then
                tinsert(temp, "+" .. key .. " {" .. cache[v] .. "}")
            elseif type(v) == "table" then
                local new_key = name .. "." .. key
                cache[v] = new_key
                tinsert(temp, "+" .. key .. _dump(v, space .. (next(t, k) and "|" or " ") .. srep(" ", #key), new_key))
            else
                tinsert(temp, "+" .. key .. " [" .. tostring(v) .. "]")
            end
        end
        return tconcat(temp, "\n" .. space)
    end
    local str = "\n" .. _dump(root, "", "")
    return str
end
print_s = function(v)
    print(tdump(v))
end

local odump = function(v, k)
    k = k or "root"
    local tmp = {}
    local cache = {}
    local _dump

    local pack_k = function(k)
        local r
        if type(k) == "number" then
            r = "[" .. k .. "]"
        elseif type(k) == "string" then
            r = k
        end
        return r
    end

    _dump = function(v, space, k)
        k = k or ""
        local nspace = space .. "    "
        local k = space .. pack_k(k) .. " = "
        if type(v) == "number" then
            tinsert(tmp, k .. v .. ",")
        elseif type(v) == "string" then
            tinsert(tmp, k .. '"' .. v .. '",')
        elseif type(v) == "table" then
            if cache[v] then
                tinsert(tmp, k .. cache[v])
                return
            end
            cache[v] = k
            tinsert(tmp, k .. "{")
            for vk, vv in pairs(v) do
                _dump(vv, nspace, vk)
            end
            tinsert(tmp, space .. "},")
        end
    end

    _dump(v, "", k)
    return tconcat(tmp, "\n")
end
dump = odump
local dump = dump
print_v = function(v, name)
    print(dump(v, name))
end

local tclone
tclone = function(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for key, value in pairs(object) do
            new_table[_copy(key)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end

    return _copy(object)
end
clone = tclone

split = function(str, sp)
    sp = sp or " "
    if type(sp) == "number" then
        sp = string.char(sp)
    end

    local patt = string.format("[^%s]+", sp)
    -- print(patt)
    local arr = {}
    for k in string.gmatch(str, patt) do
        table.insert(arr, k)
    end
    return arr
end