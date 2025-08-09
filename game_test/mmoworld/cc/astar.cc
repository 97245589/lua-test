#include "astar.h"

#include <cmath>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "world.h"

using namespace std;

static const int LWEIGH = 100;
static const int HWEIGH = 141;
static const vector<Pre> DIRECT_COST = {
    {0, 1, LWEIGH}, {0, -1, LWEIGH}, {1, 0, LWEIGH},  {-1, 0, LWEIGH},
    {1, 1, HWEIGH}, {-1, 1, HWEIGH}, {1, -1, HWEIGH}, {-1, -1, HWEIGH}};

Astar::Astar(World &world) : world_(world) {}

int Astar::eval_cost(Pos s, Pos e) {
  double dis = pow(s.x_ - e.x_, 2) + pow(s.y_ - e.y_, 2);
  dis = sqrt(dis);
  return dis * 100;
}

bool Astar::pos_equal(Pos p1, Pos p2) {
  return p1.x_ == p2.x_ && p1.y_ == p2.y_;
}

void Astar::find(Pos s, Pos e, vector<Pos> &ret) {
  if (world_.get_block(s.x_, s.y_) != 0 || world_.get_block(e.x_, e.y_) != 0)
    return;

  unordered_set<Pos, Pos_hash, Pos_equal> close_list;
  unordered_map<Pos, Pre, Pos_hash, Pos_equal> pres;
  priority_queue<State> open_list;

  int weigh = eval_cost(s, e);
  State st{.x_ = s.x_, .y_ = s.y_, .cost_ = 0, .weigh_ = weigh};
  open_list.push(st);

  while (!open_list.empty()) {
    State t = open_list.top();
    open_list.pop();

    if (pos_equal({.x_ = t.x_, .y_ = t.y_}, e)) {
      Pos p{.x_ = t.x_, .y_ = t.y_};
      ret.push_back(p);
      while (pres.end() != pres.find(p)) {
        auto it = pres.find(p);
        p = {.x_ = it->second.x_, .y_ = it->second.y_};
        ret.push_back(p);
      }
      return;
    }

    close_list.insert({.x_ = t.x_, .y_ = t.y_});
    for (const auto &d : DIRECT_COST) {
      State t1{.x_ = (int16_t)(t.x_ + d.x_),
               .y_ = (int16_t)(t.y_ + d.y_),
               .cost_ = t.cost_ + d.cost_};

      if (world_.check_block(t1.x_, t1.y_)) continue;
      if (close_list.end() != close_list.find({.x_ = t1.x_, .y_ = t1.y_}))
        continue;

      if (quick_) {
        t1.weigh_ =
            pos_equal({t1.x_, t1.y_}, e) ? 0 : eval_cost({t1.x_, t1.y_}, e);
      } else {
        t1.weigh_ = pos_equal({t1.x_, t1.y_}, e)
                        ? 0
                        : t1.cost_ + eval_cost({t1.x_, t1.y_}, e);
      }

      auto it = pres.find({t1.x_, t1.y_});
      if (it == pres.end() || it->second.cost_ > t1.cost_) {
        open_list.push(t1);
        pres[{.x_ = t1.x_, .y_ = t1.y_}] = {
            .x_ = t.x_, .y_ = t.y_, .cost_ = t1.cost_};
      }
    }
  }
}

void Astar::quick_find(Pos s, Pos e, vector<Pos> &ret) {
  quick_ = 1;
  find(s, e, ret);
  quick_ = 0;
}

void Astar::short_find(Pos s, Pos e, vector<Pos> &ret) { find(s, e, ret); }