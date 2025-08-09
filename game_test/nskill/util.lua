local table, pairs, ipairs = table, pairs, ipairs
local type, setmetatable, getmetatable = type, setmetatable, getmetatable

local trawdump = function(v, k)
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
            table.insert(tmp, k .. v .. ",")
        elseif type(v) == "string" then
            table.insert(tmp, k .. '"' .. v .. '",')
        elseif type(v) == "table" then
            if cache[v] then
                table.insert(tmp, k .. cache[v])
                return
            end
            cache[v] = k
            table.insert(tmp, k .. "{")
            for vk, vv in pairs(v) do
                _dump(vv, nspace, vk)
            end
            table.insert(tmp, space .. "},")
        end
    end

    _dump(v, "", k)
    return table.concat(tmp, "\n")
end
rawdump = trawdump

local tclone;
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
