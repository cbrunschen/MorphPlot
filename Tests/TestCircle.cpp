/*
 *  TestCircle.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 05/06/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestCircle.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Circle.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"

#include "Count.h"

#include <iterator>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <iomanip>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>

#include <stdarg.h>

using namespace Primitives;
using namespace Images;
using namespace std;

static void setAll(Bitmap &bitmap, const Circle &circle, const IPoint &p0) {
  for (list<IPoint>::const_iterator i = circle.points().begin(); 
       i != circle.points().end();
       ++i) {
    bitmap.at(p0 + *i) = true;
  }
}

void verifyFindOffsets(int r) {
  Circle circle(r);
  Bitmap bitmap(4*r + 16, 4*r + 16, true);
  IPoint center(2*r + 8 - 2, 2*r + 8 + 3);
  setAll(bitmap, circle, center);
  vector<Boundary> boundaries(bitmap.scanBoundaries(false));
  REQUIRE(boundaries.size() == 1);
  Boundary &boundary = *boundaries.begin();
  REQUIRE(boundary.size() == 1);
  Chain &chain = *boundary.begin();
  
  circle.ensureDirectionsAndOffsets();

  int length = static_cast<int>(chain.size());
  vector<IPoint> points, rpoints;
  points.reserve(2 * length);
  rpoints.reserve(2 * length);
  copy(chain.begin(), chain.end(), back_inserter(points));
  copy(chain.begin(), chain.end(), back_inserter(points));
  copy(chain.rbegin(), chain.rend(), back_inserter(rpoints));
  copy(chain.rbegin(), chain.rend(), back_inserter(rpoints));
  
  for (int l = length; l >= 1; l--) {
    for (int offset = 0; offset < length; offset++) {
      list<IPoint> candidates;
      vector<IPoint>::iterator a = points.begin() + offset;
      vector<IPoint>::iterator b = a + l;
      
      circle.findOffsets(a, b, back_inserter(candidates));
      REQUIRE(find(candidates.begin(), candidates.end(), center) != candidates.end());
      
      candidates.clear();
      a = rpoints.begin() + offset;
      b = a + l;

      circle.findOffsets(a, b, back_inserter(candidates));
      REQUIRE(find(candidates.begin(), candidates.end(), center) != candidates.end());
    }
  }
}

TEST_CASE("offsets/find", "Find the offsets")
{
  for (int r = 1; r < 20; r+= 4) {
    SECTION(count("r=", r), count("testing at radius ", r)) {
      verifyFindOffsets(r);
    }
  }
}

void verifyFilterOffsets(int r) {
  Circle circle(r);
  Bitmap bitmap(4*r + 16, 4*r + 16, true);
  IPoint center(2*r + 8 - 2, 2*r + 8 + 3);
  setAll(bitmap, circle, center);
  vector<Boundary> boundaries(bitmap.scanBoundaries(false));
  REQUIRE(boundaries.size() == 1);
  Boundary &boundary = *boundaries.begin();
  REQUIRE(boundary.size() == 1);
  Chain &chain = *boundary.begin();

  for (Chain::const_iterator i = chain.begin(); i != chain.end();) {
    Chain::const_iterator j = i;
    ++j;
    
    set<IPoint> candidates, filtered;
    circle.findOffsets(i, j, inserter(candidates, candidates.begin()));
    
    circle.filterOffsets(candidates.begin(), candidates.end(), bitmap.get(), inserter(filtered, filtered.begin()));
//    cerr << "From " << *i << ": " << filtered.size() << " filtered: " << filtered << endl;
    REQUIRE(filtered.size() == 1);
    REQUIRE(*filtered.begin() == center);
    
    i = j;
  }
}

TEST_CASE("offsets/filter", "filter the offsets")
{
  for (int r = 1; r < 20; r++) {
    SECTION(count("r=", r), count("testing at radius ", r)) {
      verifyFilterOffsets(r);
    }
  }
}

TEST_CASE("offsets/fromHorizontalTInset", "testing a theory")
{
  int r = 7;
  shared_ptr<Bitmap> bitmap(new Bitmap(51, 51, true));
  
  Circle circle(r);
  
  bitmap->line(IPoint(7, 10), IPoint(42, 10), true, circle);
  bitmap->line(IPoint(7, 20), IPoint(42, 20), true, circle);
  
  bitmap->line(IPoint(19, 25), IPoint(19, 49), true);
  bitmap->line(IPoint(20, 25), IPoint(20, 49), true);
    
  cerr << *bitmap << endl;

  shared_ptr<Bitmap> inset(bitmap->inset(r));

  cerr << *inset << endl;
  
  shared_ptr<Bitmap> outset(inset->outset(r));
  
  cerr << *outset << endl;

  shared_ptr<Bitmap> remainder(*bitmap - *outset);

  cerr << *remainder << endl;
  
  shared_ptr<Bitmap> adjacent(outset->adjacent(remainder));

  cerr << *adjacent << endl;
  
  vector<Boundary> adjacents;
  shared_ptr< GreyImage<int> > marks = make_shared< GreyImage<int> >(adjacent->width(), adjacent->height());
  adjacent->scanBoundaries(adjacents, *marks, false);
  
  cerr << "  - adjacent pixel chains:" << endl;
  for (vector<Boundary>::iterator b = adjacents.begin();
       b != adjacents.end();
       ++b) {
    if (b->size() != 1) {
      cerr << "adjacent boundary with " << b->size() << " chains!!" << endl;
    }
    cerr << *b << endl;
  }
  cerr << flush;
  
  return;
  
  set<IPoint> candidates, filtered;
  circle.findOffsets(adjacents.begin()->begin()->begin(), adjacents.begin()->begin()->end(), inserter(candidates, candidates.begin()));
  
  cerr << candidates.size() << " candidates: " << candidates << endl;
  
  circle.filterOffsets(candidates.begin(), candidates.end(), outset->getXY(), inserter(filtered, filtered.begin()));

  cerr << filtered.size() << " filtered: " << filtered << endl;
  
  REQUIRE(filtered.size() == 0);
}

TEST_CASE("offsets/fromAlmostHorizontalTInset", "testing a theory")
{
  return;
  
  int r = 7;
  shared_ptr<Bitmap> bitmap(new Bitmap(51, 51, true));
  
  Circle circle(r);
  
  bitmap->line(IPoint(7, 10), IPoint(42, 14), true, circle);
  bitmap->line(IPoint(7, 20), IPoint(42, 24), true, circle);
  
  bitmap->line(IPoint(18, 26), IPoint(16, 49), true);
  bitmap->line(IPoint(19, 26), IPoint(17, 49), true);
  bitmap->line(IPoint(20, 26), IPoint(18, 49), true);
  bitmap->line(IPoint(21, 26), IPoint(19, 49), true);
  
  cerr << *bitmap << endl;
  
  shared_ptr<Bitmap> inset(bitmap->inset(r));
  
  cerr << *inset << endl;
  
  shared_ptr<Bitmap> outset(inset->outset(r));
  
  cerr << *outset << endl;
  
  shared_ptr<Bitmap> remainder(*bitmap - *outset);
  
  cerr << *remainder << endl;
  
  shared_ptr<Bitmap> adjacent(outset->adjacent(remainder));
  
  cerr << *adjacent << endl;
  
  vector<Boundary> adjacents;
  shared_ptr< Image<int> > marks = make_shared< Image<int> >(adjacent->width(), adjacent->height());
  marks->clear();
  adjacent->scanBoundaries(adjacents, *marks, false);
  
  cerr << "  - adjacent pixel chains:" << endl;
  for (vector<Boundary>::iterator b = adjacents.begin();
       b != adjacents.end();
       ++b) {
    if (b->size() != 1) {
      cerr << "adjacent boundary with " << b->size() << " chains!!" << endl;
    }
    cerr << *b << endl;
  }
  
  set<IPoint> candidates, filtered;
  circle.findOffsets(adjacents.begin()->begin()->begin(), adjacents.begin()->begin()->end(), inserter(candidates, candidates.begin()));
  
  cerr << candidates.size() << " candidates: " << candidates << endl;
  
  circle.filterOffsets(candidates.begin(), candidates.end(), outset->getXY(), inserter(filtered, filtered.begin()));
  
  cerr << filtered.size() << " filtered: " << filtered << endl;
  
  REQUIRE(filtered.size() == 1);
}

TEST_CASE("offsets/from45DegreeTInset", "testing a theory")
{
  return;
  
  int r = 17;
  int width = 4*r + 2 + 10;
  int w2 = width / 2;
  shared_ptr<Bitmap> bitmap(new Bitmap(width, width, true));
  
  Circle circle(r);
  
  bitmap->line(IPoint(r, r), IPoint(3*r + 1 + 10, 3*r + 1 + 10), true, circle);
  bitmap->line(IPoint(2*r, r), IPoint(3*r + 1 + 10, 2*r + 1 + 10), true, circle);
  
  bitmap->line(IPoint(w2 - 1, w2), IPoint(6 - 1, width - 6), true);
  bitmap->line(IPoint(w2 - 1, w2 - 1), IPoint(6 - 1, width - 6 - 1), true);
  bitmap->line(IPoint(w2, w2), IPoint(6, width - 6), true);
  bitmap->line(IPoint(w2 + 1, w2), IPoint(6 + 1, width - 6), true);
  bitmap->line(IPoint(w2 + 1, w2 + 1), IPoint(6 + 1, width - 6 + 1), true);
  
  cerr << *bitmap << endl;
  
  shared_ptr<Bitmap> inset(bitmap->inset(r));
  
  cerr << *inset << endl;
  
  shared_ptr<Bitmap> outset(inset->outset(r));
  
  cerr << *outset << endl;
  
  shared_ptr<Bitmap> remainder(*bitmap - *outset);
  
  cerr << *remainder << endl;
  
  shared_ptr<Bitmap> adjacent(outset->adjacent(remainder));
  
  cerr << adjacent << endl;
  
  vector<Boundary> adjacents;
  shared_ptr< Image<int> > marks = make_shared< Image<int> >(adjacent->width(), adjacent->height());
  marks->clear();
  adjacent->scanBoundaries(adjacents, *marks, false);
  
  cerr << "  - adjacent pixel chains:" << endl;
  for (vector<Boundary>::iterator b = adjacents.begin();
       b != adjacents.end();
       ++b) {
    if (b->size() != 1) {
      cerr << "adjacent boundary with " << b->size() << " chains!!" << endl;
    }
    cerr << *b << endl;
  }
  
  set<IPoint> candidates, filtered;
  circle.findOffsets(adjacents.begin()->begin()->begin(), adjacents.begin()->begin()->end(), inserter(candidates, candidates.begin()));
  
  cerr << candidates.size() << " candidates: " << candidates << endl;
  
  circle.filterOffsets(candidates.begin(), candidates.end(), outset->getXY(), inserter(filtered, filtered.begin()));
  
  cerr << filtered.size() << " filtered: " << filtered << endl;
  
  REQUIRE(filtered.size() == 0);
}
