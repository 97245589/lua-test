#include "range.h"

#include <iostream>
using namespace std;

double Range::get_cosv_2vec(vtype v1, vtype v2) {
  double m1 = sqrt(v1[0] * v1[0] + v1[1] * v1[1]);
  double m2 = sqrt(v2[0] * v2[0] + v2[1] * v2[1]);

  double ret = (v1[0] * v2[0] + v1[1] * v2[1]) / (m1 * m2);
  return ret;
}

double Range::get_cosv_3vec(vtype v1, vtype v2, vtype v3) {
  vtype v12 = {v2[0] - v1[0], v2[1] - v1[1]};
  vtype v13 = {v3[0] - v1[0], v3[1] - v1[1]};
  return get_cosv_2vec(v12, v13);
}

bool Range::in_circular(vtype p, double r, vtype p1) {
  return r * r >=
         (p[0] - p1[0]) * (p[0] - p1[0]) + (p[1] - p1[1]) * (p[1] - p1[1]);
}

bool Range::in_sector(vtype p, vtype d, double r, double ang, vtype p1) {
  if (!in_circular(p, r, p1)) return false;

  vtype dp = {p[0] + d[0], p[1] + d[1]};
  double cosv = get_cosv_3vec(p, dp, p1);
  double cosa = cos(rad(ang));
  return cosv >= cosa;
}

bool Range::in_rectange(vtype p, vtype d, double l, double w, vtype p1) {
  vtype dp = {p[0] + d[0], p[1] + d[1]};
  double cosv = get_cosv_3vec(p, dp, p1);
  double mp1 =
      sqrt((p1[0] - p[0]) * (p1[0] - p[0]) + (p1[1] - p[1]) * (p1[1] - p[1]));
  double rx = mp1 * cosv;
  double ry = mp1 * sin(acos(cosv));

  if (0 <= rx && rx <= l && 0 <= ry && ry <= w) return true;
  return false;
}