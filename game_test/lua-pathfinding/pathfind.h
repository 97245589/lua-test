#ifndef __PATHFIND_H__
#define __PATHFIND_H__

#include <algorithm>
#include <array>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

using std::array;
using std::function;
using std::priority_queue;
using std::string;
using std::unordered_map;
using std::vector;

struct World;

struct Pos {
  int16_t x_, y_;
};
struct Pos_hash {
  size_t operator()(const Pos &lhs) const { return lhs.x_ << 16 & lhs.y_; }
};
struct Pos_equal {
  bool operator()(const Pos &lhs, const Pos &rhs) const {
    if (lhs.x_ != rhs.x_) return false;
    return lhs.y_ == rhs.y_;
  }
};

struct Pathfind_state {
  int16_t x_, y_;
  int32_t cost_, weigh_;  // cost从起点的消耗值 weigh_预估值决定寻路策略
  int8_t direct_x_, direct_y_;
  bool operator<(const Pathfind_state &rhs) const {
    return weigh_ > rhs.weigh_;
  }
};

using Jp_cache = vector<vector<array<int16_t, 4>>>;
struct Path_find {
  World &world_;
  Jp_cache jp_cache_;
  int8_t search_strategy_;  // 0默认最短lujing 1快速路径

  priority_queue<Pathfind_state> *p_open_list_;
  unordered_map<Pos, Pos, Pos_hash, Pos_equal> *p_pres_;

  Path_find(World &world);
  bool walkable(int16_t x, int16_t y);
  bool side_check(int16_t x, int16_t y);
  inline static int eval_cost(Pos start, Pos end);

  void dump_line_cache(Pos dir, string &ret);
  void add_jp_cache(Pos p, Pos dir, int16_t len);
  void force_neighbor(Pos p, Pos direct, function<void(Pos, Pos)> cb);
  void line_jp_cache(Pos p, Pos direct);
  void block_jp_cache(Pos lb, Pos rt);
  void init_jp_cache();

  void cal_weigh(Pathfind_state &state, Pos end);
  void add_jp(Pathfind_state state, Pos pre);
  bool find_end(Pos p, Pos end, Pos direct);
  bool add_force_neighbor(Pathfind_state p, Pos end, Pos direct);
  bool add_jp_to_openlist(Pathfind_state p, Pos end, Pos direct);
  bool step(Pathfind_state s, Pos end, Pos direct);
  void jps(Pos start, Pos end, vector<Pos> &result);

  void quick_search(Pos start, Pos end, vector<Pos> &result);
  void short_search(Pos start, Pos end, vector<Pos> &result);
};

#endif