extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <iostream>
#include <string>
using namespace std;

#include "leveldb/db.h"
#include "leveldb/write_batch.h"

static const char *LLEVELDB_META = "LLEVELDB_META";

struct Lleveldb {
  static int create_lleveldb(lua_State *L);
  static void lleveldb_meta(lua_State *L);
  static int lleveldb_gc(lua_State *L);

  static int get(lua_State *L);
  static int put(lua_State *L);
  static int del(lua_State *L);
  static int iter(lua_State *L);
};

int Lleveldb::iter(lua_State *L) {
  leveldb::DB **pp = (leveldb::DB **)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = *pp;

  size_t slen, llen;
  const char *ps = luaL_checklstring(L, 2, &slen);
  const char *pl = luaL_checklstring(L, 3, &llen);
  string slim = {pl, llen};

  lua_createtable(L, 0, 0);
  leveldb::Iterator *it = db->NewIterator(leveldb::ReadOptions());
  for (it->Seek({ps, slen}); it->Valid() && it->key().ToString() < slim;
       it->Next()) {
    const string &k = it->key().ToString();
    const string &v = it->value().ToString();
    lua_pushlstring(L, k.c_str(), k.size());
    lua_pushlstring(L, v.c_str(), v.size());
    lua_settable(L, -3);
  }
  delete it;
  return 1;
}

int Lleveldb::get(lua_State *L) {
  leveldb::DB **pp = (leveldb::DB **)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = *pp;

  size_t len;
  const char *p = luaL_checklstring(L, 2, &len);
  string str;
  leveldb::Status s = db->Get(leveldb::ReadOptions(), {p, len}, &str);
  if (s.ok()) {
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  } else {
    return 0;
  }
}

int Lleveldb::put(lua_State *L) {
  leveldb::DB **pp = (leveldb::DB **)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = *pp;

  size_t klen, vlen;
  const char *k = luaL_checklstring(L, 2, &klen);
  const char *v = luaL_checklstring(L, 3, &vlen);
  db->Put(leveldb::WriteOptions(), {k, klen}, {v, vlen});
  return 0;
}

int Lleveldb::del(lua_State *L) {
  leveldb::DB **pp = (leveldb::DB **)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = *pp;

  size_t len;
  const char *p = luaL_checklstring(L, 2, &len);
  db->Delete(leveldb::WriteOptions(), {p, len});
  return 0;
}

int Lleveldb::lleveldb_gc(lua_State *L) {
  leveldb::DB **pp = (leveldb::DB **)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = *pp;
  delete db;
}

void Lleveldb::lleveldb_meta(lua_State *L) {
  if (luaL_newmetatable(L, LLEVELDB_META)) {
    luaL_Reg l[] = {
        {"get", get}, {"put", put}, {"del", del}, {"iter", iter}, {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lleveldb_gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Lleveldb::create_lleveldb(lua_State *L) {
  size_t len;
  const char *p = luaL_checklstring(L, 1, &len);
  string name = {p, len};

  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, name, &db);

  if (!status.ok()) {
    return luaL_error(L, "leveldb open err");
  }

  leveldb::DB **pp = (leveldb::DB **)lua_newuserdata(L, sizeof(db));
  *pp = db;
  lleveldb_meta(L);
  return 1;
}

static const struct luaL_Reg funcs[] = {
    {"create_lleveldb", Lleveldb::create_lleveldb}, {NULL, NULL}};

extern "C" {
LUAMOD_API int luaopen_lleveldb(lua_State *L) {
  luaL_newlib(L, funcs);
  return 1;
}
}