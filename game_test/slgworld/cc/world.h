#ifndef __WORLD_H__
#define __WORLD_H__

#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::function;
using std::string;
using std::tuple;
using std::unordered_map;
using std::unordered_set;
using std::vector;

#include "astar.h"

using Cell = unordered_set<int64_t>;

struct View {
  unordered_set<int64_t> adds_, updates_, deletes_;
};

struct Collision {
  int64_t id1_, id2_;
};
struct Collision_equal {
  bool operator()(const Collision &lhs, const Collision &rhs) const {
    return lhs.id1_ == rhs.id1_ && lhs.id2_ == rhs.id2_;
  }
};
struct Collision_hash {
  size_t operator()(const Collision &rhs) const {
    size_t ret = rhs.id1_ ^ rhs.id2_;
    return ret;
  }
};

struct Troop {
  int16_t sx_, sy_, ex_, ey_;
  float nowx_, nowy_, beforex_, beforey_;
  int16_t idx_, back_;
  vector<Pos> paths_;
};

struct World {
  static constexpr int16_t CELL_SIZE = 10;
  int16_t len_, wid_;
  vector<vector<int64_t>> block_;
  vector<vector<Cell>> cells_;
  vector<vector<Cell>> watchers_;
  unordered_map<int64_t, Pos> watchers_pos_;
  Astar astar_;
  unordered_map<int64_t, View> views_;
  unordered_map<int64_t, Troop> troops_;
  unordered_map<int64_t, Pos> troop_poses_;
  unordered_map<Pos, unordered_set<int64_t>, Pos_hash, Pos_equal>
      troop_pos_uids_;

  unordered_set<int64_t> arrives_;
  unordered_set<Collision, Collision_hash, Collision_equal> collisions_;

  World(int16_t len, int16_t wid);
  tuple<int16_t, int16_t> get_len_wid();
  void set_block(int16_t x, int16_t y, int8_t m);
  int64_t get_block(int16_t x, int16_t y);
  bool check_pos(int16_t x, int16_t y);
  bool check_cell(int16_t x, int16_t y);
  bool pathfind_check(int16_t x, int16_t y);
  bool check_areaempty(int16_t x, int16_t y, int16_t size);
  bool get_empty_pos(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                     int16_t size, int16_t &x, int16_t &y);

  void add_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                  int64_t id);
  void update_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                     int64_t id);
  void remove_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                     int64_t id);

  void add_watch(int16_t x, int16_t y, int64_t id);
  void delete_watch(int64_t id);

  void cell_around_cb(int16_t cx, int16_t cy,
                      function<void(int16_t, int16_t)> cb);

  tuple<int16_t, int16_t> get_direct(Troop &t);
  void add_troop(int16_t sx, int16_t sy, int16_t ex, int16_t ey,
                 int64_t troopid);
  void troop_back(int64_t troopid);
  void delete_troop(int64_t troopid);
  void troop_dis(int64_t id, Troop &t, float dis);
  void troop_tick(int64_t difftm);
  void remove_troop_pos(int64_t id);
  void check_collision(Troop &t1, Troop &t2);

  void dump(string &ret);
  void dump_troop(string &ret);
};

#endif