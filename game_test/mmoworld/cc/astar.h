#ifndef __ASTAR_H__
#define __ASTAR_H__

#include <cstddef>
#include <cstdint>
#include <vector>
using std::vector;

struct Pos {
  int16_t x_, y_;
};
struct Pos_equal {
  bool operator()(const Pos &lhs, const Pos &rhs) const {
    return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_;
  }
};

struct Pos_hash {
  size_t operator()(const Pos &p) const { return p.x_ ^ (p.y_ << 2); }
};

struct State {
  int16_t x_, y_;
  int cost_, weigh_;
  bool operator<(const State &rhs) const { return this->weigh_ > rhs.weigh_; }
};

struct Pre {
  int16_t x_, y_;
  int32_t cost_;
};

struct World;

struct Astar {
  World &world_;
  int quick_;

  Astar(World &world);
  void find(Pos s, Pos e, vector<Pos> &ret);

  void quick_find(Pos s, Pos e, vector<Pos> &ret);
  void short_find(Pos s, Pos e, vector<Pos> &ret);

  static int eval_cost(Pos s, Pos e);
  static bool pos_equal(Pos p1, Pos p2);
};

#endif