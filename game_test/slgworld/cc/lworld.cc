extern "C" {
#include "lauxlib.h"
#include "lua.h"
}
#include <iostream>

#include "world.h"
using namespace std;

static const char *LWORLD_META = "LWORLD_META";

struct Lworld {
  static int lworld_gc(lua_State *L);
  static void lworld_meta(lua_State *L);
  static int create_lworld(lua_State *L);

  static int get_len_wid(lua_State *L);
  static int set_block(lua_State *L);
  static int get_block(lua_State *L);
  static int get_empty_pos(lua_State *L);
  static int add_entity(lua_State *L);
  static int update_entity(lua_State *L);
  static int remove_entity(lua_State *L);
  static int path_find(lua_State *L);
  static int add_watch(lua_State *L);
  static int delete_watch(lua_State *L);
  static int get_view(lua_State *L);
  static int add_troop(lua_State *L);
  static int troop_back(lua_State *L);
  static int delete_troop(lua_State *L);
  static int troop_tick(lua_State *L);
  static int dump(lua_State *L);
  static int dump_troop(lua_State *L);
};

int Lworld::troop_tick(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);

  int64_t diff_tm = lua_tointeger(L, 2);
  world.troop_tick(diff_tm);

  lua_createtable(L, 0, 0);
  auto &arrives = world.arrives_;
  if (arrives.size() > 0) {
    lua_createtable(L, arrives.size(), 0);
    int i = 0;
    for (auto id : arrives) {
      lua_pushinteger(L, id);
      lua_rawseti(L, -2, ++i);
    }
    lua_setfield(L, -2, "arrives");
    arrives.clear();
  }
  auto &collision = world.collisions_;
  if (collision.size() > 0) {
    lua_createtable(L, collision.size() * 2, 0);
    int i = 0;
    for (auto &coll : collision) {
      lua_pushinteger(L, coll.id1_);
      lua_rawseti(L, -2, ++i);
      lua_pushinteger(L, coll.id2_);
      lua_rawseti(L, -2, ++i);
    }
    lua_setfield(L, -2, "collision");
    collision.clear();
  }

  return 1;
}

int Lworld::add_troop(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);
  luaL_checktype(L, 6, LUA_TNUMBER);
  int16_t sx = lua_tointeger(L, 2);
  int16_t sy = lua_tointeger(L, 3);
  int16_t ex = lua_tointeger(L, 4);
  int16_t ey = lua_tointeger(L, 5);
  int64_t troopid = lua_tointeger(L, 6);
  world.add_troop(sx, sy, ex, ey, troopid);
  return 0;
}
int Lworld::troop_back(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  int64_t troopid = lua_tointeger(L, 2);
  world.troop_back(troopid);
  return 0;
}
int Lworld::delete_troop(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  int64_t troopid = lua_tointeger(L, 2);
  world.delete_troop(troopid);
  return 0;
}

int Lworld::get_empty_pos(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);
  luaL_checktype(L, 6, LUA_TNUMBER);
  int16_t ltx = lua_tointeger(L, 2);
  int16_t lty = lua_tointeger(L, 3);
  int16_t rbx = lua_tointeger(L, 4);
  int16_t rby = lua_tointeger(L, 5);
  int16_t size = lua_tointeger(L, 6);
  int16_t ox, oy;
  bool ret = world.get_empty_pos(ltx, lty, rbx, rby, size, ox, oy);
  if (ret) {
    lua_pushinteger(L, ox);
    lua_pushinteger(L, oy);
    return 2;
  }
  return 0;
}

int Lworld::add_entity(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);
  luaL_checktype(L, 6, LUA_TNUMBER);

  int16_t ltx = lua_tointeger(L, 2);
  int16_t lty = lua_tointeger(L, 3);
  int16_t rbx = lua_tointeger(L, 4);
  int16_t rby = lua_tointeger(L, 5);
  int64_t id = lua_tointeger(L, 6);

  world.add_entity(ltx, lty, rbx, rby, id);
  return 0;
}

int Lworld::update_entity(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);
  luaL_checktype(L, 6, LUA_TNUMBER);

  int16_t ltx = lua_tointeger(L, 2);
  int16_t lty = lua_tointeger(L, 3);
  int16_t rbx = lua_tointeger(L, 4);
  int16_t rby = lua_tointeger(L, 5);
  int64_t id = lua_tointeger(L, 6);

  world.update_entity(ltx, lty, rbx, rby, id);
  return 0;
}

int Lworld::remove_entity(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  luaL_checktype(L, 5, LUA_TNUMBER);
  luaL_checktype(L, 6, LUA_TNUMBER);

  int16_t ltx = lua_tointeger(L, 2);
  int16_t lty = lua_tointeger(L, 3);
  int16_t rbx = lua_tointeger(L, 4);
  int16_t rby = lua_tointeger(L, 5);
  int64_t id = lua_tointeger(L, 6);
  world.remove_entity(ltx, lty, rbx, rby, id);
  return 0;
}

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
  int64_t id = lua_tointeger(L, 4);
  world.set_block(x, y, id);
  return 0;
}

int Lworld::get_block(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);

  int16_t x = lua_tointeger(L, 2);
  int16_t y = lua_tointeger(L, 3);
  int64_t b = world.get_block(x, y);
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

int Lworld::add_watch(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);
  luaL_checktype(L, 3, LUA_TNUMBER);
  luaL_checktype(L, 4, LUA_TNUMBER);
  int16_t x = lua_tointeger(L, 2);
  int16_t y = lua_tointeger(L, 3);
  int64_t id = lua_tointeger(L, 4);
  world.add_watch(x, y, id);
  return 0;
}

int Lworld::delete_watch(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  luaL_checktype(L, 2, LUA_TNUMBER);

  int64_t id = lua_tointeger(L, 2);
  world.delete_watch(id);
  return 0;
}

int Lworld::get_view(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  auto &views = world.views_;
  if (0 == views.size()) return 0;

  lua_createtable(L, 0, views.size());
  for (auto &&[id, view] : views) {
    lua_createtable(L, 0, 3);

    auto &adds = view.adds_;
    if (adds.size() > 0) {
      lua_createtable(L, adds.size(), 0);
      int i = 0;
      for (auto iid : adds) {
        lua_pushinteger(L, iid);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "adds");
    }
    auto &updates = view.updates_;
    if (updates.size() > 0) {
      lua_createtable(L, updates.size(), 0);
      int i = 0;
      for (auto iid : updates) {
        lua_pushinteger(L, iid);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "updates");
    }
    auto &deletes = view.deletes_;
    if (deletes.size() > 0) {
      lua_createtable(L, deletes.size(), 0);
      int i = 0;
      for (auto iid : deletes) {
        lua_pushinteger(L, iid);
        lua_rawseti(L, -2, ++i);
      }
      lua_setfield(L, -2, "deletes");
    }
    lua_rawseti(L, -2, id);
  }
  views.clear();
  return 1;
}

int Lworld::dump(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  string s;
  world.dump(s);
  lua_pushlstring(L, s.c_str(), s.size());
  return 1;
}

int Lworld::dump_troop(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  World &world = **pp;
  string s;
  world.dump_troop(s);
  lua_pushlstring(L, s.c_str(), s.size());
  return 1;
}

int Lworld::lworld_gc(lua_State *L) {
  World **pp = (World **)luaL_checkudata(L, 1, LWORLD_META);
  delete *pp;
  return 0;
}

void Lworld::lworld_meta(lua_State *L) {
  if (luaL_newmetatable(L, LWORLD_META)) {
    luaL_Reg l[] = {
        {"get_len_wid", get_len_wid},     {"set_block", set_block},
        {"get_block", get_block},         {"path_find", path_find},
        {"get_empty_pos", get_empty_pos}, {"add_entity", add_entity},
        {"update_entity", update_entity}, {"remove_entity", remove_entity},
        {"add_watch", add_watch},         {"delete_watch", delete_watch},
        {"get_view", get_view},           {"add_troop", add_troop},
        {"troop_back", troop_back},       {"delete_troop", delete_troop},
        {"troop_tick", troop_tick},       {"dump", dump},
        {"dump_troop", dump_troop},       {NULL, NULL}};
    luaL_newlib(L, l);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, lworld_gc);
    lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
}

int Lworld::create_lworld(lua_State *L) {
  luaL_checktype(L, 1, LUA_TNUMBER);
  luaL_checktype(L, 2, LUA_TNUMBER);

  uint16_t map_len = lua_tointeger(L, 1);
  uint16_t map_wid = lua_tointeger(L, 2);

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