#ifndef __WORLD_H__
#define __WORLD_H__

#include "pathfind.h"

#include <vector>

using std::vector;

struct World {
  int16_t map_len_, map_wid_;
  Path_find path_find_;
  vector<vector<int8_t>> block_;

  World(int16_t map_len, int16_t map_wid);

  bool is_block(int16_t x, int16_t y);
};

#endif