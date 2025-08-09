#include <cmath>
#include <iostream>
#include <sstream>
using namespace std;

#include "world.h"

static float DIST(float sx, float sy, float ex, float ey) {
  return sqrt((ex - sx) * (ex - sx) + (ey - sy) * (ey - sy));
}

World::World(int16_t len, int16_t wid) : len_(len), wid_(wid), astar_(*this) {
  block_ = vector<vector<int64_t>>(len, vector<int64_t>(wid, 0));
  cells_ = vector<vector<Cell>>(len / CELL_SIZE, vector<Cell>(wid / CELL_SIZE));
  watchers_ =
      vector<vector<Cell>>(len / CELL_SIZE, vector<Cell>(wid / CELL_SIZE));
}

tuple<int16_t, int16_t> World::get_len_wid() {
  return {block_.size(), block_[0].size()};
}

void World::set_block(int16_t x, int16_t y, int8_t m) {
  if (!check_pos(x, y)) return;
  block_[x][y] = m;
}

int64_t World::get_block(int16_t x, int16_t y) {
  if (!check_pos(x, y)) return 0;
  return block_[x][y];
}

bool World::check_pos(int16_t x, int16_t y) {
  if (x < 0 || x >= len_) return false;
  if (y < 0 || y >= wid_) return false;
  return true;
}

bool World::check_cell(int16_t x, int16_t y) {
  if (x < 0 || x >= len_ / CELL_SIZE) return false;
  if (y < 0 || y >= wid_ / CELL_SIZE) return false;
  return true;
}

bool World::pathfind_check(int16_t x, int16_t y) {
  if (!check_pos(x, y)) return false;
  if (block_[x][y] > 10) return false;
  return true;
}

bool World::check_areaempty(int16_t x, int16_t y, int16_t size) {
  for (int16_t i = 0; i < size; ++i) {
    for (int16_t j = 0; j < size; ++j) {
      int16_t nx = x + i;
      int16_t ny = y + j;
      if (!check_pos(nx, ny)) return false;
      if (get_block(nx, ny)) return false;
    }
  }
  return true;
}

bool World::get_empty_pos(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                          int16_t size, int16_t &ox, int16_t &oy) {
  if (ltx > rbx || lty > rby) return false;

  for (int x = ltx; x <= rbx - size + 1; ++x) {
    for (int y = lty; y <= rby - size + 1; ++y) {
      if (check_areaempty(x, y, size)) {
        ox = x;
        oy = y;
        return true;
      }
    }
  }
  return false;
}

void World::add_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                       int64_t id) {
  if (ltx > rbx || lty > rby) return;

  for (int16_t x = ltx; x <= rbx; ++x) {
    for (int16_t y = lty; y <= rby; ++y) {
      if (!check_pos(x, y)) return;
      if (get_block(x, y)) return;
    }
  }

  unordered_set<Pos, Pos_hash, Pos_equal> mark;
  for (int16_t x = ltx; x <= rbx; ++x) {
    for (int16_t y = lty; y <= rby; ++y) {
      block_[x][y] = id;
      int16_t cx = x / CELL_SIZE;
      int16_t cy = y / CELL_SIZE;
      cells_[cx][cy].insert(id);
      cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
        Pos p{.x_ = ncx, .y_ = ncy};
        if (auto it = mark.find(p); it != mark.end()) return;
        mark.insert(p);
        auto &watcher = watchers_[ncx][ncy];
        for (auto wid : watcher) {
          views_[wid].adds_.insert(id);
        }
      });
    }
  }
}

void World::update_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                          int64_t id) {
  if (ltx > rbx || lty > rby) return;
  unordered_set<Pos, Pos_hash, Pos_equal> mark;
  for (int16_t x = ltx; x <= rbx; ++x) {
    for (int16_t y = lty; y <= rby; ++y) {
      int16_t cx = x / CELL_SIZE;
      int16_t cy = y / CELL_SIZE;
      cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
        Pos p{.x_ = ncx, .y_ = ncy};
        if (auto it = mark.find(p); it != mark.end()) return;
        mark.insert(p);
        auto &watcher = watchers_[ncx][ncy];
        for (auto wid : watcher) {
          views_[wid].updates_.insert(id);
        }
      });
    }
  }
}

void World::remove_entity(int16_t ltx, int16_t lty, int16_t rbx, int16_t rby,
                          int64_t id) {
  if (ltx > rbx || lty > rby) return;
  for (int16_t x = ltx; x <= rbx; ++x) {
    for (int16_t y = lty; y <= rby; ++y) {
      if (!check_pos(x, y)) return;
      int64_t bid = get_block(x, y);
      if (bid != id) return;
    }
  }
  unordered_set<Pos, Pos_hash, Pos_equal> mark;
  for (int16_t x = ltx; x <= rbx; ++x) {
    for (int16_t y = lty; y <= rby; ++y) {
      block_[x][y] = 0;
      int16_t cx = x / CELL_SIZE;
      int16_t cy = y / CELL_SIZE;
      cells_[cx][cy].erase(id);
      cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
        Pos p{.x_ = ncx, .y_ = ncy};
        if (auto it = mark.find(p); it != mark.end()) return;
        mark.insert(p);
        auto &watcher = watchers_[ncx][ncy];
        for (auto wid : watcher) {
          views_[wid].deletes_.insert(id);
        }
      });
    }
  }
}

void World::add_watch(int16_t x, int16_t y, int64_t id) {
  if (!check_pos(x, y)) return;
  int16_t cx = x / CELL_SIZE;
  int16_t cy = y / CELL_SIZE;
  delete_watch(id);
  watchers_[cx][cy].insert(id);
  watchers_pos_[id] = {.x_ = cx, .y_ = cy};

  cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
    auto &cell = cells_[ncx][ncy];
    for (auto cid : cell) {
      views_[id].adds_.insert(cid);
    }
  });
}

void World::delete_watch(int64_t id) {
  if (auto it = watchers_pos_.find(id); it != watchers_pos_.end()) {
    auto p = it->second;
    watchers_[p.x_][p.y_].erase(id);
    watchers_pos_.erase(id);
  }
}

void World::cell_around_cb(int16_t cx, int16_t cy,
                           function<void(int16_t, int16_t)> cb) {
  for (int i = -1; i <= 1; ++i) {
    for (int j = -1; j <= 1; ++j) {
      int16_t ncx = cx + i;
      int16_t ncy = cy + i;
      if (!check_cell(ncx, ncy)) continue;
      cb(ncx, ncy);
    }
  }
}

void World::add_troop(int16_t sx, int16_t sy, int16_t ex, int16_t ey,
                      int64_t troopid) {
  if (!check_pos(sx, sy) || !check_pos(ex, ey)) return;
  if (get_block(sx, sy) || get_block(ex, ey)) return;

  auto &troop = troops_[troopid];
  troop.sx_ = sx;
  troop.sy_ = sy;
  troop.ex_ = ex;
  troop.ey_ = ey;
  troop.nowx_ = sx;
  troop.nowy_ = sy;
  troop.idx_ = 0;
  troop.back_ = 0;

  vector<Pos> poses;
  // for (int16_t i = 0; i < 999; ++i) poses.push_back({.x_ = i, .y_ = i});
  astar_.quick_find({.x_ = sx, .y_ = sy}, {.x_ = ex, .y_ = ey}, poses);
  std::reverse(poses.begin(), poses.end());
  troop.paths_ = std::move(poses);

  unordered_set<Pos, Pos_hash, Pos_equal> mark;
  for (auto &pos : troop.paths_) {
    int cx = pos.x_ / CELL_SIZE;
    int cy = pos.y_ / CELL_SIZE;
    cells_[cx][cy].insert(troopid);
    cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
      Pos p{.x_ = ncx, .y_ = ncy};
      if (auto it = mark.find(p); it != mark.end()) return;
      mark.insert(p);
      auto &watcher = watchers_[ncx][ncy];
      for (auto wid : watcher) {
        views_[wid].adds_.insert(troopid);
      }
    });
  }
}

void World::troop_back(int64_t troopid) {
  auto it = troops_.find(troopid);
  if (it == troops_.end()) return;
  it->second.back_ = 1;
}

void World::delete_troop(int64_t troopid) {
  auto it = troops_.find(troopid);
  if (it == troops_.end()) return;
  auto &troop = it->second;

  unordered_set<Pos, Pos_hash, Pos_equal> mark;
  for (auto &pos : troop.paths_) {
    int cx = pos.x_ / CELL_SIZE;
    int cy = pos.y_ / CELL_SIZE;
    cells_[cx][cy].erase(troopid);
    cell_around_cb(cx, cy, [&](int16_t ncx, int16_t ncy) {
      Pos p{.x_ = ncx, .y_ = ncy};
      if (auto it = mark.find(p); it != mark.end()) return;
      mark.insert(p);
      auto &watcher = watchers_[ncx][ncy];
      for (auto wid : watcher) {
        views_[wid].deletes_.insert(troopid);
      }
    });
  }
  troops_.erase(troopid);
  remove_troop_pos(troopid);
}

tuple<int16_t, int16_t> World::get_direct(Troop &t) {
  if (t.idx_ == t.paths_.size() - 1) return {0, 0};

  auto &p = t.paths_[t.idx_];
  auto &np = t.paths_[t.idx_ + 1];
  return {np.x_ - p.x_, np.y_ - p.y_};
}

void World::remove_troop_pos(int64_t id) {
  if (auto it = troop_poses_.find(id); it == troop_poses_.end()) return;
  Pos p = troop_poses_[id];
  troop_poses_.erase(id);
  troop_pos_uids_[p].erase(id);
  if (troop_pos_uids_[p].size() == 0) troop_pos_uids_.erase(p);
}

void World::troop_dis(int64_t id, Troop &t, float dis) {
  if (t.idx_ == t.paths_.size() - 1) return;
  auto &np = t.paths_[t.idx_ + 1];
  auto nx = np.x_;
  auto ny = np.y_;
  auto [dx, dy] = get_direct(t);
  float dist = DIST(t.nowx_, t.nowy_, nx, ny);
  if (dis >= dist) {
    ++t.idx_;
    t.nowx_ = nx;
    t.nowy_ = ny;
    if (t.idx_ == t.paths_.size() - 1) {
      arrives_.insert(id);
      return;
    } else {
      troop_dis(id, t, dis - dist);
      return;
    }
  } else {
    t.nowx_ += dx / dist * dis;
    t.nowy_ += dy / dist * dis;

    Pos p{.x_ = (int16_t)t.nowx_, .y_ = (int16_t)t.nowy_};
    remove_troop_pos(id);
    troop_poses_[id] = p;
    troop_pos_uids_[p].insert(id);
  }
}

void World::troop_tick(int64_t difftm) {
  for (auto &[id, troop] : troops_) {
    float dis = (float)difftm / 1000;
    troop_dis(id, troop, dis);
  }

  unordered_set<int64_t> mark;
  for (auto &[id, troop] : troops_) {
    if (auto it = troop_poses_.find(id); it == troop_poses_.end()) continue;
    if (mark.find(id) != mark.end()) continue;

    bool b = false;
    auto &pos = troop_poses_[id];

    if (auto it = troop_pos_uids_.find(pos); it != troop_pos_uids_.end()) {
      for (auto pid : it->second) {
        if (id >= pid) continue;
        if (mark.find(pid) != mark.end()) continue;
        collisions_.insert({.id1_ = id, .id2_ = pid});
        mark.insert(id);
        mark.insert(pid);
        break;
      }
    }
  }
}

void World::check_collision(Troop &t1, Troop &t2) {}

void World::dump(string &ret) {
  ostringstream oss;
  oss << "cells ----";
  for (int i = 0; i < len_ / CELL_SIZE; ++i) {
    for (int j = 0; j < wid_ / CELL_SIZE; ++j) {
      oss << i << "," << j << "," << cells_[i][j].size() << endl;
    }
  }

  oss << "watchers_ --------";
  for (int i = 0; i < len_ / CELL_SIZE; ++i) {
    for (int j = 0; j < wid_ / CELL_SIZE; ++j) {
      oss << i << "," << j << "," << watchers_[i][j].size() << endl;
    }
  }

  oss << "watchers_pos_ : " << watchers_pos_.size() << endl;
  ret = std::move(oss.str());
}

void World::dump_troop(string &ret) {
  ostringstream oss;
  for (auto &[id, troop] : troops_) {
    oss << "id:" << id << " " << troop.nowx_ << "," << troop.nowy_ << endl;
  }
  for (auto &[id, p] : troop_poses_) {
    oss << "troop_poses id:" << id << " " << p.x_ << "," << p.y_ << endl;
  }
  for (auto &[p, ids] : troop_pos_uids_) {
    oss << "troop_pos_uids:" << p.x_ << " " << p.y_;
    for (auto id : ids) {
      oss << " " << id;
    }
    oss << endl;
  }
  ret = std::move(oss.str());
}