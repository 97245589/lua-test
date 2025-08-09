#include "world.h"

#include <cmath>
#include <iostream>
using namespace std;

World::World(int16_t len, int16_t wid) : len_(len), wid_(wid), astar_(*this) {
  block_ = vector<vector<int8_t>>(len, vector<int8_t>(wid, 0));
}

tuple<int16_t, int16_t> World::get_len_wid() {
  return {block_.size(), block_[0].size()};
}

void World::set_block(int16_t x, int16_t y, int8_t m) {
  if (!check_pos(x, y)) return;
  block_[x][y] = m;
}

int8_t World::get_block(int16_t x, int16_t y) {
  if (!check_pos(x, y)) return -1;
  return block_[x][y];
}

bool World::check_pos(int16_t x, int16_t y) {
  if (x < 0 || x >= len_) return false;
  if (y < 0 || y >= wid_) return false;
  return true;
}

bool World::check_block(int16_t x, int16_t y) {
  if (!check_pos(x, y)) return false;
  return block_[x][y] != 0;
}

void World::update_pos(int64_t id, const char *modestring, float px, float py,
                       float dx, float dy, float speed) {
  array<float, 2> p{px, py};
  for (int i = 0; modestring[i]; ++i) {
    char m = modestring[i];
    if (m == 'd') {
      pos_infos_.erase(id);
      player_views_.erase(id);
      view_infos_.erase(id);
      tick_update_poses_.erase(id);
      aoi_.aoi_update(id, "d", p);
      return;
    }
  }

  pos_infos_[id] = {.px_ = px,
                    .py_ = py,
                    .dx_ = dx,
                    .dy_ = dy,
                    .speed_ = speed,
                    .modestring_ = modestring};
  aoi_.aoi_update(id, modestring, p);
}

void World::tick(int64_t difftm) {
  view_infos_.clear();
  tick_update_poses_.clear();
  update_pos_all(difftm);
  aoi_info();
  handle_view();
}

void World::update_pos_all(int64_t diff_tm) {
  for (auto &[player_id, pos] : pos_infos_) {
    float dx = pos.dx_;
    float dy = pos.dy_;
    if (pos.speed_ < 1e-6) continue;
    if (0 == dx && 0 == dy) continue;
    float dis = (float)diff_tm / 1000 * pos.speed_;
    float xym = sqrt(dx * dx + dy * dy);

    auto [len, wid] = get_len_wid();
    float bpx = pos.px_;
    float bpy = pos.py_;
    float apx = bpx + dis * dx / xym;
    float apy = bpy + dis * dy / xym;
    if (apx < 0) apx = 0;
    if (apx > len - 1) apx = len - 1;
    if (apy < 0) apy = 0;
    if (apy > wid - 1) apy = wid - 1;
    if (abs(apx - bpx) < 1e-6 && abs(apy - bpy) < 1e-6) continue;

    pos.px_ = apx;
    pos.py_ = apy;

    array<float, 2> p{pos.px_, pos.py_};
    aoi_.aoi_update(player_id, pos.modestring_, p);
    tick_update_poses_[player_id] = {pos.px_, pos.py_};

    if (auto it = player_views_.find(player_id); it != player_views_.end()) {
      for (auto id : it->second) {
        if (auto it = pos_infos_.find(id); it == pos_infos_.end()) continue;
        view_infos_[id].updates_.insert(player_id);
      }
    }
  }
}

void World::aoi_info() {
  aoi_.aoi_message();
  auto &ret = aoi_.result_;

  for (auto &&[playerid, setids] : ret) {
    for (auto id : setids) {
      player_views_[playerid].insert(id);
      view_infos_[playerid].adds_.insert(id);
    }
  }
}

void World::handle_view() {
  for (auto &[playerid, setids] : player_views_) {
    array<float, 2> ppos;
    if (auto it = pos_infos_.find(playerid); it != pos_infos_.end()) {
      ppos = {it->second.px_, it->second.py_};
    } else {
      continue;
    }
    for (auto it = setids.begin(); it != setids.end();) {
      auto itt = pos_infos_.find(*it);
      if (itt == pos_infos_.end()) {
        auto &view_info_id = view_infos_[playerid];
        auto id = *it;
        view_info_id.deletes_.insert(id);
        view_info_id.adds_.erase(id);
        view_info_id.updates_.erase(id);
        it = setids.erase(it);
        continue;
      }
      array<float, 2> pos{itt->second.px_, itt->second.py_};

      auto distance = aoi_space::DIST2(ppos, pos);
      if (distance > aoi_space::AOI_RADIS2) {
        auto &view_info_id = view_infos_[playerid];
        auto id = *it;
        view_info_id.deletes_.insert(id);
        view_info_id.adds_.erase(id);
        view_info_id.updates_.erase(id);
        it = setids.erase(it);
      } else {
        ++it;
      }
    }
  }
}

void World::find_target(int64_t playerid, set<Player_dis> &ret,
                        function<bool(vtype, vtype, vtype)> cb) {
  if (auto it = player_views_.find(playerid); it == player_views_.end()) return;
  auto &ids = player_views_[playerid];
  auto &pos_info = pos_infos_[playerid];
  array<float, 2> p1{pos_info.px_, pos_info.py_};
  array<float, 2> d1{pos_info.dx_, pos_info.dy_};
  for (auto id : ids) {
    array<float, 2> p2{pos_infos_[id].px_, pos_infos_[id].py_};
    if (!cb(p1, d1, p2)) continue;
    ret.insert({.player_id_ = id, .dis_ = aoi_space::DIST2(p1, p2)});
  }
}