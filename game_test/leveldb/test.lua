require "util"
local format = string.format
local leveldb = require("lleveldb")

local test1 = function()
    local db = leveldb.create_lleveldb("db", 1024 * 1024)
    db:put("hello", "world")
    print(db:get("hello"))
    db:del("hello")
    print(db:get("hello"))

    for i = 1, 10 do
        db:put("hello" .. i, "world" .. i)
    end

    local ret = db:iter("ha", "hz")
    print_v(ret)
end

local test2 = function()
    local db = leveldb.create_lleveldb("db", 1024 * 1024)
    local arr = {}
    for i = 1, 5 do
        table.insert(arr, "hello" .. i)
        table.insert(arr, "world" .. i)
        table.insert(arr, "world" .. i)
        table.insert(arr, "hello" .. i)
    end
    print(#arr)
    db:batch(arr)
    print(dump(db:get_all()))
    print(dump(db:pre_get("hello1")))
end
test2()
