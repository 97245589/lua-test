#ifndef __RANK_H__
#define __RANK_H__

#include <cstdint>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using std::endl;
using std::ostringstream;
using std::set;
using std::string;
using std::unordered_map;
using std::vector;

struct Rank_base {
  int64_t uid_;
  int64_t score_;
  int64_t time_;
};
struct Rank_base_cmp {
  bool operator()(const Rank_base &lhs, const Rank_base &rhs) const {
    if (lhs.score_ != rhs.score_) return lhs.score_ > rhs.score_;
    if (lhs.time_ != rhs.time_) return lhs.time_ < rhs.time_;
    return lhs.uid_ > rhs.uid_;
  }
};

using Rank_set = set<Rank_base, Rank_base_cmp>;
using Rank_set_it = Rank_set::iterator;
struct Rank {
  Rank_set ranks_;
  unordered_map<int64_t, Rank_set_it> rank_info_;
  int max_num_;

  void add_rank(const Rank_base &);
  void evict();
  void get_rank_info(int num, int64_t uid, vector<Rank_base> &ret,
                     int &me_rank);
  void dump(string &ret);
  Rank() : max_num_(999) {}
};
void Rank::add_rank(const Rank_base &base) {
  if (auto rank_info_it = rank_info_.find(base.uid_);
      rank_info_it != rank_info_.end()) {
    ranks_.erase(rank_info_it->second);
    rank_info_.erase(base.uid_);
  }
  auto [it, mark] = ranks_.insert(base);
  if (mark) rank_info_.insert({base.uid_, it});
  evict();
}
void Rank::evict() {
  if (rank_info_.size() > max_num_) {
    auto it = ranks_.rbegin();
    auto uid = it->uid_;
    rank_info_.erase(it->uid_);
    ranks_.erase(*it);
  }
}

void Rank::get_rank_info(int num, int64_t uid, vector<Rank_base> &ret,
                         int &me_rank) {
  int c = 0;
  bool find_me = false;
  if (uid > 0) find_me = true;
  for (auto it : ranks_) {
    ++c;
    if (c <= num) {
      ret.push_back(it);
    }
    if (c >= num && !find_me) return;
    if (find_me && it.uid_ == uid) {
      me_rank = c;
      find_me = false;
    }
  }
}

void Rank::dump(string &ret) {
  ostringstream oss;
  oss << "rank_size : " << ranks_.size() << endl;
  oss << "it_size : " << rank_info_.size() << endl;
  oss << "max_size : " << max_num_ << endl;
  for (auto rank_base : ranks_) {
    oss << rank_base.uid_ << "," << rank_base.score_ << "," << rank_base.time_
        << "|";
  }
  oss << endl;
  ret = std::move(oss.str());
}

#endif