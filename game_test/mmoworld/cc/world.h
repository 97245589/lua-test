#ifndef __WORLD_H__
#define __WORLD_H__

#include <cstdint>
#include <functional>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "aoi.h"
#include "astar.h"

using std::function;
using std::set;
using std::tuple;
using std::unordered_map;
using std::unordered_set;
using std::vector;

struct Player_pos {
  float px_, py_, dx_, dy_, speed_;
  const char* modestring_;
};

struct Player_dis {
  int64_t player_id_;
  float dis_;
  bool operator<(const Player_dis& rhs) const {
    if (dis_ != rhs.dis_) return dis_ < rhs.dis_;
    return player_id_ < rhs.player_id_;
  }
};

struct View {
  unordered_set<int64_t> adds_, updates_, deletes_;
};

struct World {
  int16_t len_, wid_;
  vector<vector<int8_t>> block_;

  aoi_space aoi_;
  Astar astar_;

  unordered_map<int64_t, Player_pos> pos_infos_;
  unordered_map<int64_t, unordered_set<int64_t>> player_views_;

  unordered_map<int64_t, View> view_infos_;
  unordered_map<int64_t, array<float, 2>> tick_update_poses_;

  World(int16_t len, int16_t wid);
  tuple<int16_t, int16_t> get_len_wid();

  void set_block(int16_t x, int16_t y, int8_t m);
  int8_t get_block(int16_t x, int16_t y);
  bool check_pos(int16_t x, int16_t y);
  bool check_block(int16_t x, int16_t y);

  void update_pos(int64_t id, const char* modestring, float px, float py,
                  float dx, float dy, float speed);

  void tick(int64_t diff_tm);
  void update_pos_all(int64_t diff_tm);
  void aoi_info();
  void handle_view();

  using vtype = const array<float, 2>&;
  void find_target(int64_t playerid, set<Player_dis>& ret,
                   function<bool(vtype, vtype, vtype)> cb);
};

#endif