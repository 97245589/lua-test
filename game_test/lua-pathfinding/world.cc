#include "world.h"

World::World(int16_t map_len, int16_t map_wid)
    : map_len_(map_len),
      map_wid_(map_wid),
      path_find_(*this),
      block_(vector<vector<int8_t>>(map_len, vector<int8_t>(map_wid))) {}

bool World::is_block(int16_t x, int16_t y) { return block_[x][y]; }
