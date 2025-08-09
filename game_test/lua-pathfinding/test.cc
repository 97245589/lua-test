#include <memory>
#include <vector>

#include "pathfind.h"
#include "world.h"

using std::make_unique;
using std::unique_ptr;
using std::vector;

int main() {
  unique_ptr<World> world = make_unique<World>(1000, 1000);
  auto &path_find = world->path_find_;
  vector<Pos> result;
  path_find.quick_search({0, 0}, {100, 100}, result);
}