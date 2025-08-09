extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include "rank.hpp"

static const char *LRANK_META = "LRANK_META";

struct Lrank {
  static int create_lrank(lua_State *L);
  static void lrank_meta(lua_State *L);
  static int lrank_gc(lua_State *L);

  static int add_rank(lua_State *L);
  static int rank_info(lua_State *L);
  static int dump(lua_State *L);
  static int db_data(lua_State *L);
};

int Lrank::db_data(lua_State *L) {
  Rank **pp = (Rank **)luaL_checkudata(L, 1, LRANK_META);
  auto &rank = **pp;

  auto &ranks = rank.ranks_;
  if (ranks.size() <= 0) return 0;

  int c = 0;
  lua_createtable(L, ranks.size() * 3, 0);
  for (auto &&data : ranks) {
    lua_pushinteger(L, data.uid_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, data.score_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, data.time_);
    lua_rawseti(L, -2, ++c);
  }
  return 1;
}

int Lrank::add_rank(lua_State *L) {
  Rank **pp = (Rank **)luaL_checkudata(L, 1, LRANK_META);
  auto &rank = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);

  int64_t id = lua_tointeger(L, 2);
  int64_t score = lua_tointeger(L, 3);
  int64_t time = lua_tointeger(L, 4);
  Rank_base base{.uid_ = id, .score_ = score, .time_ = time};
  rank.add_rank(base);
  return 0;
}

int Lrank::rank_info(lua_State *L) {
  Rank **pp = (Rank **)luaL_checkudata(L, 1, LRANK_META);
  auto &rank = **pp;

  int64_t get_num = lua_tointeger(L, 2);
  int64_t me_uid = lua_tointeger(L, 3);
  vector<Rank_base> ret;
  int me_rank = 0;
  rank.get_rank_info(get_num, me_uid, ret, me_rank);

  lua_createtable(L, 0, 2);
  lua_pushinteger(L, me_rank);
  lua_setfield(L, -2, "me_rank");

  lua_createtable(L, ret.size() * 2, 0);
  int c = 0;
  for (auto &&data : ret) {
    lua_pushinteger(L, data.uid_);
    lua_rawseti(L, -2, ++c);
    lua_pushinteger(L, data.score_);
    lua_rawseti(L, -2, ++c);
  }
  lua_setfield(L, -2, "ranks");

  return 1;
}

int Lrank::dump(lua_State *L) {
  Rank **pp = (Rank **)luaL_checkudata(L, 1, LRANK_META);
  auto &rank = **pp;

  string ret;
  rank.dump(ret);
  lua_pushlstring(L, ret.c_str(), ret.size());
  return 1;
}

int Lrank::lrank_gc(lua_State *L) {
  Rank **pp = (Rank **)luaL_checkudata(L, 1, LRANK_META);
  delete *pp;
  return 0;
}

void Lrank::lrank_meta(lua_State *L) {
  if (luaL_newmetatable(L, LRANK_META)) {
    luaL_Reg l[] = {{"add_rank", add_rank},
                    {"rank_info", rank_info},
                    {"db_data", db_data},
                    {"dump", dump},
                    {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lrank_gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Lrank::create_lrank(lua_State *L) {
  int max_num = lua_tointeger(L, 1);

  Rank *p = new Rank();
  if (max_num > 0) p->max_num_ = max_num;
  Rank **pp = (Rank **)lua_newuserdata(L, sizeof(p));
  *pp = p;
  lrank_meta(L);
  return 1;
}

static const struct luaL_Reg funcs[] = {{"create_lrank", Lrank::create_lrank},
                                        {NULL, NULL}};

extern "C" {
LUAMOD_API int luaopen_lrank(lua_State *L) {
  luaL_newlib(L, funcs);
  return 1;
}
}
