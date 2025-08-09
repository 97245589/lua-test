#include "aoi.h"

float aoi_space::dist2(object &p1, object &p2) {
  return DIST2(p1.position_, p2.position_);
}

shared_ptr<object> aoi_space::get_object_ptr(const int64_t &id) {
  if (objects_.find(id) == objects_.end()) {
    objects_[id] = make_shared<object>();
  }
  auto p = objects_[id];
  p->id_ = id;
  return p;
}

bool aoi_space::change_mode(shared_ptr<object> obj, bool set_watcher,
                            bool set_marker) {
  bool change = false;
  if (obj->mode_ == 0) {
    if (set_watcher) {
      obj->mode_ = MODE_WATCHER;
    }
    if (set_marker) {
      obj->mode_ |= MODE_MARKER;
    }
    return true;
  }
  if (set_watcher) {
    if (!(obj->mode_ & MODE_WATCHER)) {
      obj->mode_ |= MODE_WATCHER;
      change = true;
    }
  } else {
    if (obj->mode_ & MODE_WATCHER) {
      obj->mode_ &= ~MODE_WATCHER;
      change = true;
    }
  }
  if (set_marker) {
    if (!(obj->mode_ & MODE_MARKER)) {
      obj->mode_ |= MODE_MARKER;
      change = true;
    }
  } else {
    if (obj->mode_ & MODE_MARKER) {
      obj->mode_ &= ~MODE_MARKER;
      change = true;
    }
  }
  return change;
}

void aoi_space::aoi_update(const int64_t &id, const char *modestring,
                           array<float, 2> &pos) {
  auto obj = this->get_object_ptr(id);
  int i;
  bool set_watcher = false;
  bool set_marker = false;

  for (int i = 0; modestring[i]; ++i) {
    char m = modestring[i];
    switch (m) {
      case 'w':
        set_watcher = true;
        break;
      case 'm':
        set_marker = true;
        break;
      case 'd':
        if (!(obj->mode_ & MODE_DROP)) {
          obj->mode_ = MODE_DROP;
          objects_.erase(id);
        }
        return;
    }
  }

  if (obj->mode_ & MODE_DROP) {
    obj->mode_ &= ~MODE_DROP;
  }

  obj->position_ = pos;
  bool changed = change_mode(obj, set_watcher, set_marker);
  if (changed || !is_near(pos, obj->last_)) {
    obj->last_ = pos;
    obj->mode_ |= MODE_MOVE;
    ++obj->version_;
  }
}

void aoi_space::flush_pair() {
  hot_.remove_if([&](hot_pair &p) {
    if (p.watcher_->version_ != p.watcher_version_ ||
        p.marker_->version_ != p.marker_version_ ||
        (p.watcher_->mode_ & MODE_DROP) || (p.marker_->mode_ & MODE_DROP)) {
      return true;
    }

    auto distance2 = this->dist2(*p.watcher_, *p.marker_);
    if (distance2 > AOI_RADIS2 * 4) return true;

    if (distance2 < AOI_RADIS2) {
      result_[p.watcher_->id_].insert(p.marker_->id_);
      return true;
    }

    return false;
  });
}

void aoi_space::set_push() {
  for (const auto &it : objects_) {
    auto &obj = it.second;
    int mode = obj->mode_;
    if (mode & MODE_WATCHER) {
      if (mode & MODE_MOVE) {
        watcher_move_.insert(obj);
        obj->mode_ &= ~MODE_MOVE;
      } else {
        watcher_static_.insert(obj);
      }
    }

    if (mode & MODE_MARKER) {
      if (mode & MODE_MOVE) {
        marker_move_.insert(obj);
        obj->mode_ &= ~MODE_MOVE;
      } else {
        marker_static_.insert(obj);
      }
    }
  }
}

void aoi_space::gen_pair(const shared_ptr<object> &watcher,
                         const shared_ptr<object> &marker) {
  if (watcher->id_ == marker->id_) return;

  float distance2 = dist2(*watcher, *marker);
  if (distance2 < AOI_RADIS2) {
    result_[watcher->id_].insert(marker->id_);
    return;
  }

  if (distance2 > AOI_RADIS2 * 4) return;

  hot_pair p;
  p.watcher_ = watcher;
  p.marker_ = marker;
  p.watcher_version_ = watcher->version_;
  p.marker_version_ = marker->version_;
  hot_.push_front(std::move(p));
}

void aoi_space::gen_pair_list(unordered_set<shared_ptr<object>> &watcher,
                              unordered_set<shared_ptr<object>> &marker) {
  for (auto &itw : watcher) {
    for (auto &itm : marker) {
      gen_pair(itw, itm);
    }
  }
}

void aoi_space::aoi_message() {
  result_.clear();
  flush_pair();
  watcher_static_.clear();
  watcher_move_.clear();
  marker_static_.clear();
  marker_move_.clear();
  set_push();
  gen_pair_list(watcher_static_, marker_move_);
  gen_pair_list(watcher_move_, marker_static_);
  gen_pair_list(watcher_move_, marker_move_);
  // for (auto &it : result_) {
  //   cout << it.first << "------>";
  //   for (auto &itt : it.second) {
  //     cout << itt << " ";
  //   }
  //   cout << endl;
  // }
}

// int main() {
//   unique_ptr<aoi_space> pa(new aoi_space());
//   for (int i = 0; i < 10; ++i) {
//     array<float, 2> arr = {i, i};
//     pa->aoi_update(to_string(i), "wm", arr);
//   }
//   pa->aoi_message();
//   array<float, 2> arr = {0, 0};
//   pa->aoi_update(to_string(5), "d", arr);
//   pa->aoi_message();
// }