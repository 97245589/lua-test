#ifndef __RANGE_H__
#define __RANGE_H__

#include <array>
#include <cmath>
using std::array;

struct Range {
  using vtype = const array<float, 2> &;

  static double rad(double ang) { return ang / 180 * M_PI; }

  static double get_cosv_2vec(vtype v1, vtype v2);

  static double get_cosv_3vec(vtype v1, vtype v2, vtype v3);

  static bool in_circular(vtype p, double r, vtype p1);

  static bool in_sector(vtype p, vtype d, double r, double ang, vtype p1);

  static bool in_rectange(vtype p, vtype d, double l, double w, vtype p1);
};

#endif