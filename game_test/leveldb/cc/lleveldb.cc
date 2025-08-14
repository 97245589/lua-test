extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <iostream>
#include <string>
using namespace std;

#include "leveldb/cache.h"
#include "leveldb/db.h"
#include "leveldb/filter_policy.h"
#include "leveldb/write_batch.h"

static const char *LLEVELDB_META = "LLEVELDB_META";

struct Leveldb_data {
  leveldb::DB *db_;
  leveldb::Cache *cache_;
  const leveldb::FilterPolicy *filter_;
};

struct Lleveldb {
  static int create_lleveldb(lua_State *L);
  static void lleveldb_meta(lua_State *L);
  static int lleveldb_gc(lua_State *L);

  static int get(lua_State *L);
  static int put(lua_State *L);
  static int del(lua_State *L);
  static int iter_all(lua_State *L);
  static int batch(lua_State *L);
  static int pre_iter(lua_State *L);
};

int Lleveldb::pre_iter(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  size_t len;
  const char *ps = luaL_checklstring(L, 2, &len);
  string start = {ps, len};
  string limit = start + char(0xff);

  lua_createtable(L, 0, 0);
  leveldb::ReadOptions options;
  options.fill_cache = false;
  leveldb::Iterator *it = db->NewIterator(options);
  for (it->Seek(start); it->Valid() && it->key().ToString() < limit;
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

int Lleveldb::batch(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  luaL_checktype(L, 2, LUA_TTABLE);
  uint32_t len = lua_rawlen(L, 2);
  if (len <= 0 || len % 2 != 0) {
    return luaL_error(L, "leveldb batch table len err");
  }

  leveldb::WriteBatch batch;
  for (int i = 0; i < len; i += 2) {
    lua_settop(L, 2);
    lua_rawgeti(L, 2, i + 1);
    size_t lk, lv;
    const char *pk = lua_tolstring(L, 3, &lk);
    lua_rawgeti(L, 2, i + 2);
    const char *pv = lua_tolstring(L, 4, &lv);
    batch.Put({pk, lk}, {pv, lv});
  }
  db->Write(leveldb::WriteOptions(), &batch);
  return 0;
}

int Lleveldb::iter_all(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  lua_createtable(L, 0, 0);
  leveldb::ReadOptions options;
  options.fill_cache = false;
  leveldb::Iterator *it = db->NewIterator(options);
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
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
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  size_t len;
  const char *ps = luaL_checklstring(L, 2, &len);
  string str;
  leveldb::Status s = db->Get(leveldb::ReadOptions(), {ps, len}, &str);
  if (s.ok()) {
    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
  } else {
    return 0;
  }
}

int Lleveldb::put(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  size_t klen, vlen;
  const char *k = luaL_checklstring(L, 2, &klen);
  const char *v = luaL_checklstring(L, 3, &vlen);
  db->Put(leveldb::WriteOptions(), {k, klen}, {v, vlen});
  return 0;
}

int Lleveldb::del(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  leveldb::DB *db = p->db_;

  size_t len;
  const char *ps = luaL_checklstring(L, 2, &len);
  db->Delete(leveldb::WriteOptions(), {ps, len});
  return 0;
}

int Lleveldb::lleveldb_gc(lua_State *L) {
  Leveldb_data *p = (Leveldb_data *)luaL_checkudata(L, 1, LLEVELDB_META);
  delete p->db_;
  delete p->cache_;
  delete p->filter_;
}

void Lleveldb::lleveldb_meta(lua_State *L) {
  if (luaL_newmetatable(L, LLEVELDB_META)) {
    luaL_Reg l[] = {{"get", get},     {"put", put},
                    {"del", del},     {"get_all", iter_all},
                    {"batch", batch}, {"pre_get", pre_iter},
                    {NULL, NULL}};
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
  uint32_t cache_len = luaL_checkinteger(L, 2);

  leveldb::DB *db;
  leveldb::Options options;
  options.block_cache = leveldb::NewLRUCache(cache_len);
  options.filter_policy = leveldb::NewBloomFilterPolicy(10);
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, name, &db);

  if (!status.ok()) {
    return luaL_error(L, "leveldb open err");
  }

  Leveldb_data *pl = (Leveldb_data *)lua_newuserdata(L, sizeof(Leveldb_data));
  pl->db_ = db;
  pl->cache_ = options.block_cache;
  pl->filter_ = options.filter_policy;

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