/*
 *  TestExtentsAndDelta.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 02/01/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestExtentsAndDelta.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Circle.h"

#include <iterator>
#include <list>
#include <set>
#include <utility>

using namespace Primitives;
using namespace std;

template<typename I> static void drawDelta(int r, int extents[], const I &begin, const I &end) {
  // character space
  char c[2*r+2][2*r+2];
  memset(c, ' ', sizeof(c));
  // draw the reference circle: cross ...
  for (int d = 0; d <= r; d++) {
    c[r  ][r+d] = '.';
    c[r  ][r-d] = '.';
    c[r+d][r  ] = '.';
    c[r-d][r  ] = '.';
  }
  // ... and quadrants, using the extents array
  for (int d = 1; d <= r; d++) {
    int e = extents[d-1];
    for (int dd = 0; dd <= e; dd++) {
      c[r+d][r+dd] = '.';
      c[r+d][r-dd] = '.';
      c[r-d][r+dd] = '.';
      c[r-d][r-dd] = '.';
    }
  }
  
  int o = r+1;
  for (I i = begin; i != end; ++i) {
    c[o + i->y()][o + i->x()] = '#';
  }
  
  cerr << endl;
  for (int y = 2 * r + 1; y >= 0; --y) {
    for (int x = 0; x < 2 * r + 2; ++x) {
      cerr << c[y][x];
    }
    cerr << endl;
  }
}

static list<Point> checkDelta(int r) {
  int extents[r];
  Circle::makeExtents(r, extents);
  Circle circle(r);
  const list<Point> &points = circle.getDeltaForDirection(Neighbourhood::NW);

  drawDelta(r, extents, points.begin(), points.end());
  
  // check that the result is symmetrixal around (x == y), i.e., that for each
  // Point(x, y), Point(y, x) is also in the set
  for (list<Point>::const_iterator i = points.begin(); i != points.end(); ++i) {
    list<Point>::const_iterator found = find(points.begin(), points.end(), Point(i->y(), i->x()));
    REQUIRE(found != points.end());
  }

  return points;
}

TEST_CASE("deltas/diagonal", "diagonal deltas should behave as expected")
{
  cerr << "Checking diagonal deltas" << endl;
  for (int r = 1; r < 20; ++r) {
    checkDelta(r);
  }
}
