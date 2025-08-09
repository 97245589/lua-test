local require = require
require "util"
local print, rawdump = print, rawdump
local pb = require "pb"
local protoc = require "protoc"

protoc:load [[
    message Phone {
        optional string name = 1;
        optional int64 phonenumber = 2;
    }
    message Person {
        optional string name = 1;
        optional int32 age = 2;
        repeated Phone contacts = 3;
    }
]]

local data = {
    name = "ilse",
    age = 18,
    contacts = {{
        name = "aclice",
        phonenumber = 11111
    }, {
        name = "bob",
        phonenumber = 22222
    }}
}

local bin = pb.encode("Person", data)
print(#bin)

local data2 = pb.decode("Person", bin)
print(rawdump(data2))
