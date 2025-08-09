extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <iostream>
using namespace std;
#include "range.h"
#include "world.h"

static const char *LWORLD_META = "LWORLD_META";

struct Lworld {
  static int lworld_gc(lua_State *L);
  static void lworld_meta(lua_State *L);
  static int create_lworld(lua_State *L);

  static int get_len_wid(lua_State *L);
  static int set_block(lua_State *L);
  static int get_block(lua_State *L);
  static int path_find(lua_State *L);

  static int update_pos(lua_State *L);
  static int world_tick(lua_State *L);

  static int find_targ(lua_State *L);
};

int Lworld::get_len_wid(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  auto [len, wid] = world.get_len_wid();
  lua_pushinteger(L, len);
  lua_pushinteger(L, wid);
  return 2;
}

int Lworld::set_block(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);

  int16_t x = lua_tointeger(L, 2);
  int16_t y = lua_tointeger(L, 3);
  int8_t b = lua_tointeger(L, 4);
  world.set_block(x, y, b);
  return 0;
}

int Lworld::get_block(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);

  int16_t x = lua_tointeger(L, 2);
  int16_t y = lua_tointeger(L, 3);
  int8_t b = world.get_block(x, y);
  if (b >= 0) {
    lua_pushinteger(L, b);
    return 1;
  }
  return 0;
}

int Lworld::path_find(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;

  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);

  int16_t sx = lua_tointeger(L, 2);
  int16_t sy = lua_tointeger(L, 3);
  int16_t ex = lua_tointeger(L, 4);
  int16_t ey = lua_tointeger(L, 5);
  bool quick = lua_toboolean(L, 6);

  vector<Pos> ret;
  if (quick) {
    world.astar_.quick_find({.x_ = sx, .y_ = sy}, {.x_ = ex, .y_ = ey}, ret);
  } else {
    world.astar_.short_find({.x_ = sx, .y_ = sy}, {.x_ = ex, .y_ = ey}, ret);
  }
  // cout << "---- ret.size:" << ret.size() << endl;
  int size = ret.size();
  if (size <= 2) return 0;
  lua_createtable(L, size * 2, 0);
  int i = size * 2;
  for (auto p : ret) {
    lua_pushinteger(L, p.y_);
    lua_rawseti(L, -2, i--);
    lua_pushinteger(L, p.x_);
    lua_rawseti(L, -2, i--);
  }

  return 1;
}

int Lworld::update_pos(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;

  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TSTRING);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);

  int64_t id = lua_tointeger(L, 2);
  const char *mode = lua_tostring(L, 3);
  float px = lua_tonumber(L, 4);
  float py = lua_tonumber(L, 5);
  float dx = lua_tonumber(L, 6);
  float dy = lua_tonumber(L, 7);
  float speed = lua_tonumber(L, 8);
  array<float, 2> p = {px, py};
  world.update_pos(id, mode, px, py, dx, dy, speed);
  return 0;
}

int Lworld::world_tick(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;

  luaL_checktype(L, 2, LUA_TNUMBER);
  int64_t diff_tm = lua_tointeger(L, 2);

  lua_settop(L, 0);

  world.tick(diff_tm);

  auto &tick_update_poses = world.tick_update_poses_;
  auto &view_infos = world.view_infos_;
  lua_createtable(L, 0, 2);
  int i = 0;
  lua_createtable(L, 3 * tick_update_poses.size(), 0);
  for (auto &&[playerid, pos] : tick_update_poses) {
    lua_pushinteger(L, playerid);
    lua_rawseti(L, -2, ++i);
    lua_pushnumber(L, pos[0]);
    lua_rawseti(L, -2, ++i);
    lua_pushnumber(L, pos[1]);
    lua_rawseti(L, -2, ++i);
  }
  lua_setfield(L, -2, "update_poses");

  lua_createtable(L, 0, view_infos.size());
  for (auto &&[playerid, view] : view_infos) {
    lua_createtable(L, 0, 0);

    auto &adds = view.adds_;
    if (adds.size() > 0) {
      lua_createtable(L, adds.size(), 0);
      int i = 0;
      for (auto id : adds) {
        lua_pushinteger(L, id);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "adds");
    }
    auto &updates = view.updates_;
    if (updates.size() > 0) {
      lua_createtable(L, updates.size(), 0);
      int i = 0;
      for (auto id : updates) {
        lua_pushinteger(L, id);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "updates");
    }
    auto &deletes = view.deletes_;
    if (deletes.size() > 0) {
      lua_createtable(L, deletes.size(), 0);
      int i = 0;
      for (auto id : deletes) {
        lua_pushinteger(L, id);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "deletes");
    }

    lua_rawseti(L, -2, playerid);
  }
  lua_setfield(L, -2, "views");

  return 1;
}

int Lworld::find_targ(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);

  int64_t playerid = lua_tointeger(L, 2);

  int type = lua_tointeger(L, 3);
  set<Player_dis> ret;

  if (1 == type) {
    luaL_checktype(L, 4, LUA_TNUMBER);
    double r = lua_tonumber(L, 4);
    world.find_target(playerid, ret,
                      [=](World::vtype p1, World::vtype d1, World::vtype p2) {
                        if (Range::in_circular(p1, r, p2)) return true;
                        return false;
                      });
  } else if (2 == type) {
    luaL_checktype(L, 4, LUA_TNUMBER);
    luaL_checktype(L, 5, LUA_TNUMBER);
    double r = lua_tonumber(L, 4);
    double ang = lua_tonumber(L, 5);
    world.find_target(playerid, ret,
                      [=](World::vtype p1, World::vtype d1, World::vtype p2) {
                        if (Range::in_sector(p1, d1, r, ang, p2)) return true;
                        return false;
                      });
  } else if (3 == type) {
    luaL_checktype(L, 4, LUA_TNUMBER);
    luaL_checktype(L, 5, LUA_TNUMBER);
    double len = lua_tonumber(L, 4);
    double wid = lua_tonumber(L, 5);
    world.find_target(playerid, ret,
                      [=](World::vtype p1, World::vtype d1, World::vtype p2) {
                        if (Range::in_rectange(p1, d1, len, wid, p2))
                          return true;
                        return false;
                      });
  }

  if (ret.size() == 0) return 0;
  lua_createtable(L, ret.size(), 0);

  int i = 0;
  for (auto info : ret) {
    lua_pushinteger(L, info.player_id_);
    lua_rawseti(L, -2, ++i);
  }
  return 1;
}

void Lworld::lworld_meta(lua_State *L) {
  if (luaL_newmetatable(L, LWORLD_META)) {
    luaL_Reg l[] = {{"get_len_wid", get_len_wid}, {"set_block", set_block},
                    {"get_block", get_block},     {"path_find", path_find},
                    {"update_pos", update_pos},   {"world_tick", world_tick},
                    {"find_targ", find_targ},     {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lworld_gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Lworld::lworld_gc(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  delete *pp;
  return 0;
}

int Lworld::create_lworld(lua_State *L) {
  luaL_checktype(L, 1, LUA_TNUMBER);
  luaL_checktype(L, 2, LUA_TNUMBER);
  int16_t map_len = lua_tointeger(L, 1);
  int16_t map_wid = lua_tointeger(L, 2);

  World *p = new World(map_len, map_wid);
  World **pp = (World **)lua_newuserdata(L, sizeof(p));
  *pp = p;
  lworld_meta(L);
  return 1;
}

static const struct luaL_Reg funcs[] = {
    {"create_lworld", Lworld::create_lworld}, {NULL, NULL}};

extern "C" {
LUAMOD_API int luaopen_lworld(lua_State *L) {
  luaL_newlib(L, funcs);
  return 1;
}
}