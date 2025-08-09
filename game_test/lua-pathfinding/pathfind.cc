#include "pathfind.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <unordered_set>
using std::cout;
using std::endl;
using std::ostringstream;
using std::unordered_set;

#include "world.h"

static const int16_t weighl = 100;
static const int16_t weighh = 141;
static const vector<Pos> directs = {{0, 1}, {0, -1}, {1, 0},  {-1, 0},
                                    {1, 1}, {-1, 1}, {1, -1}, {-1, -1}};

static unordered_map<Pos, int8_t, Pos_hash, Pos_equal> dir_idx_map = {
    {{0, 1}, 0}, {{0, -1}, 1}, {{1, 0}, 2}, {{-1, 0}, 3}};

struct Pos_direct_cmp {
  bool operator()(const Pos &lhs, const Pos &rhs) {
    int dx = end.x_ - start.x_;
    int dy = end.y_ - start.y_;
    if (dx) dx = dx > 0 ? 1 : -1;
    if (dy) dy = dy > 0 ? 1 : -1;
    int l = abs(lhs.x_ - dx) + abs(lhs.y_ - dy);
    int r = abs(rhs.x_ - dx) + abs(rhs.y_ - dy);
    return l < r;
  }
  Pos start;
  Pos end;
};

Path_find::Path_find(World &world) : world_(world) {
  jp_cache_ =
      Jp_cache(world_.map_len_, vector<array<int16_t, 4>>(world_.map_wid_));
}

bool Path_find::walkable(int16_t x, int16_t y) {
  if (!side_check(x, y)) return false;
  if (world_.is_block(x, y)) return false;
  return true;
}

bool Path_find::side_check(int16_t x, int16_t y) {
  if (x < 0 || x >= world_.map_len_ || y < 0 || y >= world_.map_wid_)
    return false;
  return true;
}

int Path_find::eval_cost(Pos start, Pos end) {
  double dis = pow((start.x_ - end.x_), 2) + pow((start.y_ - end.y_), 2);
  dis = sqrt(dis);
  return dis * 100;
}

void Path_find::dump_line_cache(Pos dir, string &ret) {
  ostringstream oss;
  for (int16_t i = 0; i < world_.map_len_; ++i) {
    for (int16_t j = 0; j < world_.map_wid_; ++j) {
      oss << jp_cache_[i][j][dir_idx_map[dir]] << " ";
    }
    oss << endl;
  }
  ret = oss.str();
  oss.clear();
}

void Path_find::add_jp_cache(Pos p, Pos dir, int16_t len) {
  jp_cache_[p.x_][p.y_][dir_idx_map[dir]] = len;
}

void Path_find::force_neighbor(Pos p, Pos direct, function<void(Pos, Pos)> cb) {
  int16_t x1 = p.x_ + direct.y_;
  int16_t y1 = p.y_ + direct.x_;
  int16_t x11 = x1 + direct.x_;
  int16_t y11 = y1 + direct.y_;
  if (side_check(x1, y1) && side_check(x11, y11)) {
    if (!walkable(x1, y1) && walkable(x11, y11)) {
      cb({x11, y11},
         {(int16_t)(direct.x_ + direct.y_), (int16_t)(direct.y_ + direct.x_)});
    }
  }

  int16_t x2 = p.x_ - direct.y_;
  int16_t y2 = p.y_ - direct.x_;
  int16_t x21 = x2 + direct.x_;
  int16_t y21 = y2 + direct.y_;
  if (side_check(x2, y2) && side_check(x21, y21)) {
    if (!walkable(x2, y2) && walkable(x21, y21)) {
      cb({x21, y21},
         {(int16_t)(direct.x_ - direct.y_), (int16_t)(direct.y_ - direct.x_)});
    }
  }
}

void Path_find::line_jp_cache(Pos p, Pos direct) {
  Pos q = p;
  while (side_check(q.x_, q.y_) && !walkable(q.x_, q.y_)) {
    jp_cache_[q.x_][q.y_] = {};
    q.x_ += direct.x_;
    q.y_ += direct.y_;
  }
  if (!side_check(q.x_, q.y_)) return;
  Pos t = q;
  int i = 0;
  while (1) {
    if (!walkable(t.x_, t.y_)) {
      --i;
      for (int16_t j = 0; j <= i; ++j) {
        Pos t2{(int16_t)(q.x_ + direct.x_ * j),
               (int16_t)(q.y_ + direct.y_ * j)};
        add_jp_cache(t2, direct, i - j);
      }
      line_jp_cache(t, direct);
      return;
    }

    bool is_fn = false;
    force_neighbor(t, direct, [&](Pos p1, Pos p2) { is_fn = true; });
    if (is_fn && i) {
      for (int16_t j = 0; j <= i; ++j) {
        Pos t2{(int16_t)(q.x_ + direct.x_ * j),
               (int16_t)(q.y_ + direct.y_ * j)};
        add_jp_cache(t2, direct, i - j);
      }
      line_jp_cache(t, direct);
      return;
    }
    t.x_ += direct.x_;
    t.y_ += direct.y_;
    ++i;
  }
}

void Path_find::block_jp_cache(Pos lb, Pos rt) {
  int min_x = lb.x_ - 1;
  if (min_x < 0) min_x = 0;
  int max_x = rt.x_ + 1;
  if (max_x >= world_.map_len_) max_x = world_.map_len_ - 1;
  int min_y = lb.y_ - 1;
  if (min_y < 0) min_y = 0;
  int max_y = rt.y_ + 1;
  if (max_y >= world_.map_wid_) max_y = world_.map_wid_ - 1;

  for (int16_t i = min_x; i <= max_x; ++i) {
    line_jp_cache({i, 0}, {0, 1});
    line_jp_cache({i, (int16_t)(world_.map_wid_ - 1)}, {0, -1});
  }
  for (int16_t i = min_y; i <= max_y; ++i) {
    line_jp_cache({0, i}, {1, 0});
    line_jp_cache({(int16_t)(world_.map_len_ - 1), i}, {-1, 0});
  }
}

void Path_find::init_jp_cache() {
  jp_cache_ =
      Jp_cache(world_.map_len_, vector<array<int16_t, 4>>(world_.map_wid_));
  for (int16_t i = 0; i < world_.map_len_; ++i) {
    line_jp_cache({i, 0}, {0, 1});
    line_jp_cache({i, (int16_t)(world_.map_wid_ - 1)}, {0, -1});
  }
  for (int16_t j = 0; j < world_.map_wid_; ++j) {
    line_jp_cache({0, j}, {1, 0});
    line_jp_cache({(int16_t)(world_.map_len_ - 1), j}, {-1, 0});
  }
}

void Path_find::cal_weigh(Pathfind_state &state, Pos end) {
  if (Pos_equal()({state.x_, state.y_}, end)) {
    state.weigh_ = 0;
    return;
  }
  int cost = eval_cost({state.x_, state.y_}, end);
  if (!search_strategy_) {
    state.weigh_ = cost + state.cost_;
  } else {
    state.weigh_ = cost;
  }
}

void Path_find::add_jp(Pathfind_state state, Pos pre) {
  if (Pos_equal()({state.x_, state.y_}, {pre.x_, pre.y_})) return;
  auto &open_list = *p_open_list_;
  auto &pres = *p_pres_;
  if (pres.find({state.x_, state.y_}) != pres.end()) return;
  open_list.push(state);
  pres[{state.x_, state.y_}] = pre;
  // cout << state.x_ << "," << state.y_ << "|" << (int)state.direct_x_ << ","
  //      << (int)state.direct_y_ << "|" << state.weigh_ << endl;
}

bool Path_find::find_end(Pos p, Pos end, Pos direct) {
  if (Pos_equal()(p, end)) return true;
  if (p.x_ != end.x_ && p.y_ != end.y_) return false;

  int dx = end.x_ - p.x_;
  int dy = end.y_ - p.y_;
  if (dx) dx = dx > 0 ? 1 : -1;
  if (dy) dy = dy > 0 ? 1 : -1;
  if (dx != direct.x_ || dy != direct.y_) return false;

  int16_t len = jp_cache_[p.x_][p.y_][dir_idx_map[{direct.x_, direct.y_}]];
  if (len <= 0) return false;
  Pos new_p{(int16_t)(p.x_ + dx * len), (int16_t)(p.y_ + dy * len)};
  if (p.x_ <= end.x_ && p.y_ <= end.y_ && end.x_ <= new_p.x_ &&
      end.y_ <= new_p.y_)
    return true;
  if (p.x_ >= end.x_ && p.y_ >= end.y_ && end.x_ >= new_p.x_ &&
      end.y_ >= new_p.y_)
    return true;
  return false;
}

bool Path_find::add_force_neighbor(Pathfind_state s, Pos end, Pos direct) {
  bool b = false;
  force_neighbor({s.x_, s.y_}, {direct.x_, direct.y_}, [&](Pos p, Pos dir) {
    auto &pres = *p_pres_;
    if (pres.find({p.x_, p.y_}) != pres.end()) return;
    Pathfind_state new_s{p.x_, p.y_,           s.cost_ + weighh,
                         0,    (int8_t)dir.x_, (int8_t)dir.y_};
    cal_weigh(new_s, end);
    add_jp(new_s, {s.x_, s.y_});
    b = true;
  });
  return b;
}

bool Path_find::add_jp_to_openlist(Pathfind_state p, Pos end, Pos direct) {
  Pathfind_state s = p;
  int len = jp_cache_[s.x_][s.y_][dir_idx_map[{s.direct_x_, s.direct_y_}]];
  if (len <= 0) return false;

  s.x_ += direct.x_ * len;
  s.y_ += direct.y_ * len;
  s.cost_ += weighl * len;

  bool b = add_force_neighbor(s, end, direct);
  auto &pres = *p_pres_;
  if (b) {
    if (pres.end() == pres.find({s.x_, s.y_})) {
      s.direct_x_ = direct.x_;
      s.direct_y_ = direct.y_;
      cal_weigh(s, end);
      add_jp(s, {p.x_, p.y_});
    }
  }
  return b;
}

bool Path_find::step(Pathfind_state start, Pos end, Pos direct) {
  start.direct_x_ = direct.x_;
  start.direct_y_ = direct.y_;
  Pathfind_state s = start;
  while (1) {
    if (!direct.x_ || !direct.y_) {
      if (!walkable(s.x_, s.y_)) return false;
      if (find_end({s.x_, s.y_}, end, direct)) {
        add_jp({end.x_, end.y_, 0, 0}, {start.x_, start.y_});
        return true;
      }
      if (add_jp_to_openlist(s, end, direct)) return true;
      return false;
    } else {
      bool b = false;
      auto &pres = *p_pres_;
      b = add_force_neighbor(s, end, {direct.x_, 0}) || b;
      b = add_force_neighbor(s, end, {0, direct.y_}) || b;
      if (b) {
        if (!Pos_equal()({s.x_, s.y_}, {start.x_, start.y_}) &&
            pres.end() == pres.find({s.x_, s.y_})) {
          pres[{s.x_, s.y_}] = {start.x_, start.y_};
        }
        int x = s.x_;
        int y = s.y_;
        int dx = direct.x_;
        int dy = direct.y_;
        bool b1 = side_check(x + dx, y) && !walkable(x + dx, y);
        bool b2 = side_check(x, y + dy) && !walkable(x, y + dy);
        bool b3 = side_check(x + dx, y + dy) && walkable(x + dx, y + dy);
        if ((b1 || b2) && b3) return true;
      }

      s.x_ += direct.x_;
      s.y_ += direct.y_;
      s.cost_ += weighh;
      if (!walkable(s.x_, s.y_)) return false;
      if (Pos_equal()({s.x_, s.y_}, end)) {
        s.weigh_ = 0;
        add_jp(s, {start.x_, start.y_});
        return true;
      }

      bool r = false;
      r = step(s, end, {direct.x_, 0}) || r;
      r = step(s, end, {0, direct.y_}) || r;
      if (r) {
        if (pres.end() == pres.find({s.x_, s.y_})) {
          pres[{s.x_, s.y_}] = {start.x_, start.y_};
          s.x_ += direct.x_;
          s.y_ += direct.y_;
          s.cost_ += weighh;
          cal_weigh(s, end);
          if (walkable(s.x_, s.y_) && pres.end() == pres.find({s.x_, s.y_})) {
            add_jp(s, {start.x_, start.y_});
          }
          return true;
        }
      }
    }
  }
}

void Path_find::jps(Pos start, Pos end, vector<Pos> &result) {
  if (!walkable(start.x_, start.y_) || !walkable(end.x_, end.y_)) return;

  unordered_set<Pos, Pos_hash, Pos_equal> close_list;
  priority_queue<Pathfind_state> open_list;
  int weigh = eval_cost(start, end);
  open_list.push({start.x_, start.y_, 0, weigh, 0, 0});
  unordered_map<Pos, Pos, Pos_hash, Pos_equal> pres;
  p_open_list_ = &open_list;
  p_pres_ = &pres;

  while (!open_list.empty()) {
    Pathfind_state t = open_list.top();
    open_list.pop();
    if (close_list.end() != close_list.find({t.x_, t.y_})) continue;
    close_list.insert({t.x_, t.y_});
    if (Pos_equal()(end, {t.x_, t.y_})) {
      // cout << "find_end" << endl;
      result.push_back(end);
      Pos p = end;
      while (pres.end() != pres.find(p)) {
        p = pres[p];
        // cout << p.x_ << "|" << p.y_ << endl;
        result.push_back(p);
      }
      return;
    }

    int8_t direct_x = t.direct_x_;
    int8_t direct_y = t.direct_y_;
    vector<Pos> dps;
    if (!direct_x && !direct_y) {
      Pos_direct_cmp p;
      p.start = start;
      p.end = end;
      dps = directs;
      std::sort(dps.begin(), dps.end(), p);
    } else if (direct_x && direct_y) {
      dps.push_back({direct_x, 0});
      dps.push_back({0, direct_y});
      dps.push_back({direct_x, direct_y});
    } else {
      dps.push_back({direct_x, direct_y});
    }

    for (auto dp : dps) {
      step(t, end, dp);
    }
  }
  p_open_list_ = nullptr;
  p_pres_ = nullptr;
}

void Path_find::short_search(Pos start, Pos end, vector<Pos> &result) {
  auto tmp = search_strategy_;
  search_strategy_ = 0;
  jps(start, end, result);
  search_strategy_ = tmp;
}

void Path_find::quick_search(Pos start, Pos end, vector<Pos> &result) {
  auto tmp = search_strategy_;
  search_strategy_ = 1;
  jps(start, end, result);
  search_strategy_ = tmp;
}
