/*
 *  TestLine.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 06/02/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestLine.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"

#include "Count.h"

#include <iterator>
#include <list>
#include <set>
#include <utility>

#include <stdarg.h>

using namespace Primitives;
using namespace Images;
using namespace std;

struct Collector {
  Chain chain_;
  void operator()(int x, int y) {
    chain_.addPoint(x, y);
  }
};

static void verifyLine(int x0, int y0, int x1, int y1) {
  Collector collector;
  line(x0, y0, x1, y1, collector);
  Chain::const_iterator ci = collector.chain_.begin();
  REQUIRE(ci->x() == x0);
  REQUIRE(ci->y() == y0);

  Point p(x0, y0);
  for (++ci; ci != collector.chain_.end(); ++ci) {
    Point q(ci->x(), ci->y());
    REQUIRE(q.isNeighbour(p));
    p = q;
  }
  REQUIRE(p.x() == x1);
  REQUIRE(p.y() == y1);
}

TEST_CASE("lines/radial", "each line should start at the center and go outwards.")
{
  int step = 5;
  int nSteps = 3;
  int r = step * nSteps;
  for (int pos = -r; pos <= r; pos += step) {
    SECTION(count("pos=", pos), "") {
      int offset = 3;
      int wh = 2 * (r + offset);
      int c = r + offset;
      Bitmap bitmap(wh, wh);
      bitmap.clear();
      
      verifyLine(c, c, c + pos, offset);
      verifyLine(c, c, wh - offset, c + pos);
      verifyLine(c, c, c - pos, wh - offset);
      verifyLine(c, c, offset, c - pos);
      
      bitmap.line(c, c, c + pos, offset, true);
      bitmap.line(c, c, wh - offset, c + pos, true);
      bitmap.line(c, c, c - pos, wh - offset, true);
      bitmap.line(c, c, offset, c - pos, true);
      
      cerr << bitmap << endl;
    }
  }
}

TEST_CASE("line/thick", "thick line")
{
#define R 7
  Circle circle(R);
  
  int extents[R];
  Circle::makeExtents(R, extents);
  const list<Point> &deltaX = circle.getHorizontalDelta();
  const list<Point> &deltaXY = circle.getDiagonalDelta();
  const list<Point> &initial = circle.points();
    
  int step = 5;
  int nSteps = 3;
  int r = step * nSteps;
  for (int pos = -2*r; pos <= 2*r; pos += step) {
    SECTION(count("pos=", pos), "") {
      int offset = 3;
      int wh = 2 * (r + offset + 3 * R) + 1;
      int c = r + offset + 3 * R;
      Bitmap bitmap(wh, wh);
      bitmap.clear();
      Bitmap::Set set = bitmap.set(true);
      
      line(c, c, c + pos, c + 2*r, set, initial, deltaX, deltaXY);
      line(c, c, c - 2*r, c + pos, set, initial, deltaX, deltaXY);
      line(c, c, c - pos, c - 2*r, set, initial, deltaX, deltaXY);
      line(c, c, c + 2*r, c - pos, set, initial, deltaX, deltaXY);
      
      cerr << bitmap << endl;
    }
  }
}

TEST_CASE("line/thin/horizontal", "")
{
  Bitmap bitmap(10, 10);
  bitmap.clear();
  bitmap.line(2, 2, 8, 2, true);
  cerr << bitmap << endl;
}

TEST_CASE("line/thin/vertical", "")
{
  Bitmap bitmap(10, 10);
  bitmap.clear();
  bitmap.line(2, 2, 2, 8, true);
  cerr << bitmap << endl;
}

TEST_CASE("line/thick/horizontal", "")
{
  Bitmap bitmap(30, 30);
  bitmap.clear();
  Circle circle(7);
  bitmap.line(10, 10, 21, 10, true, circle);
  cerr << bitmap << endl;
}

TEST_CASE("line/thin/vertical", "")
{
  Bitmap bitmap(30, 30);
  bitmap.clear();
  Circle circle(7);
  bitmap.line(10, 10, 10, 21, true, circle);
  cerr << bitmap << endl;
}
