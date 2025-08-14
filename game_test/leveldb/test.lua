require "util"
local format = string.format
local leveldb = require("lleveldb")

local db = leveldb.create_lleveldb("db")

db:put("hello", "world")
print(db:get("hello"))
db:del("hello")
print(db:get("hello"))

for i = 1, 10 do
    db:put("hello" .. i, "world" .. i)
end

local ret = db:iter("ha", "hz")
print_v(ret)

local t = os.time()
local n = 100000
for i = 1, n do
    db:put("hello" .. i, "world" .. i)
end
for i = 1, n do
    db:get("hello" .. i)
end
print(format("%s put get times cost %s", n, os.time() - t))
