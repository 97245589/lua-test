#ifndef __AOI_H__
#define __AOI_H__

#include <array>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct object {
  int64_t id_;
  int version_;
  int mode_;
  array<float, 2> last_;
  array<float, 2> position_;
};

struct hot_pair {
  shared_ptr<object> watcher_;
  shared_ptr<object> marker_;
  int watcher_version_;
  int marker_version_;
};

struct aoi_space {
  static constexpr float AOI_RADIS = 10;
  static constexpr float AOI_RADIS2 = AOI_RADIS * AOI_RADIS;
  static constexpr int MODE_WATCHER = 1;
  static constexpr int MODE_MARKER = 2;
  static constexpr int MODE_MOVE = 4;
  static constexpr int MODE_DROP = 8;

  static float DIST2(array<float, 2> &p1, array<float, 2> &p2) {
    return (p1[0] - p2[0]) * (p1[0] - p2[0]) +
           (p1[1] - p2[1]) * (p1[1] - p2[1]);
  }
  static bool is_near(array<float, 2> &p1, array<float, 2> &p2) {
    return DIST2(p1, p2) < aoi_space::AOI_RADIS2 * 0.25f;
  }

  unordered_set<shared_ptr<object>> watcher_static_;
  unordered_set<shared_ptr<object>> marker_static_;
  unordered_set<shared_ptr<object>> watcher_move_;
  unordered_set<shared_ptr<object>> marker_move_;

  unordered_map<int64_t, shared_ptr<object>> objects_;
  list<hot_pair> hot_;
  unordered_map<int64_t, unordered_set<int64_t>> result_;

  inline float dist2(object &p1, object &p2);

  bool change_mode(shared_ptr<object> obj, bool set_watcher, bool set_marker);
  shared_ptr<object> get_object_ptr(const int64_t &id);
  void aoi_update(const int64_t &id, const char *modestring,
                  array<float, 2> &pos);

  void gen_pair(const shared_ptr<object> &watcher,
                const shared_ptr<object> &marker);
  void gen_pair_list(unordered_set<shared_ptr<object>> &watcher,
                     unordered_set<shared_ptr<object>> &marker);
  void set_push();
  void flush_pair();
  void aoi_message();
};

#endif