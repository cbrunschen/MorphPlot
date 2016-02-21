//
//  TestDistanceTransform.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 09/02/2014.
//
//

#include "TestDistanceTransform.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Circle.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "GreyImage.h"
#include "GreyImage_Impl.h"

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
#include <random>

#include <stdarg.h>

using namespace Primitives;
using namespace Images;
using namespace std;

static default_random_engine rng;
static uniform_real_distribution<double> dist;
static inline double frand() { return dist(rng); }

class PGI {
	shared_ptr< GreyImage<int> > g_;
public:
	PGI(shared_ptr< GreyImage<int> > g) : g_(g) { }
	void print(ostream &out) const {
    for (int y = 0; y < g_->height(); y++) {
      for (int x = 0; x < g_->width(); x++) {
    	  out << setw(3) << right << g_->at(y, x) << " ";
      }
      out << endl;
    }
	}
};

inline ostream &operator<<(ostream &out, const PGI &pgi) {
	pgi.print(out);
	return out;
}

TEST_CASE("distanceTransform/square", "calculate the distance transform of a square")
{
  shared_ptr<Bitmap> bitmap = Bitmap::make(40, 40, true);
  
  for (int y = 9; y < 31; y++) {
    for (int x = 13; x < 27; x++) {
      bitmap->at(y, x) = true;
    }
  }
  
  cout << "bitmap: " << endl << bitmap << endl << flush;
  
  shared_ptr< GreyImage<int> > bg = bitmap->distanceTransform(true);
  cout << "Background:" << endl << PGI(bg) << endl << flush;
  
  shared_ptr< GreyImage<int> > fg = bitmap->distanceTransform(false);
  cout << "Foreground:" << endl << PGI(fg) << endl << flush;
  
  shared_ptr<Bitmap> inset = bitmap->inset(5);
  cout << "inset by 5:" << endl << inset << endl << flush;
  
  for (int r = 0; r < 10; r++) {
    shared_ptr<Bitmap> outset = bitmap->outset(r);
    cout << "outset by " << r << ":" << endl << *outset << endl << flush;
    cout << "difference should be " << r << ":" << endl << *(*outset - *bitmap) << endl << flush;
  }
  
  for (int y = 5; y < 35; y++) {
    for (int x = 7; x < 33; x++) {
      bitmap->at(y, x) = true;
    }
  }
  
  for (int r = 0; r < 10; r++) {
    shared_ptr<Bitmap> inset = bitmap->inset(r);
    cout << "inset by " << r << ":" << endl << *inset << endl << flush;
    cout << "difference should be " << r << ":" << endl << *(*bitmap - *inset) << endl << flush;
  }
}

TEST_CASE("distanceTransform/threads", "verify that single- and multi-threaded give the same result")
{
  int W = 2000;
  int H = 2000;
  shared_ptr<Bitmap> bitmap = Bitmap::make(W, H, true);
  Bitmap::Set set = bitmap->set(true);
  
  for (int a = 0; a < (W+H)/10; a++) {
    int r = (W/10) * frand();
    Circle circle(r);
    
    int extents[r];
    Circle::makeExtents(r, extents);
    const list<IPoint> &deltaX = circle.getHorizontalDelta();
    const list<IPoint> &deltaXY = circle.getDiagonalDelta();
    const list<IPoint> &initial = circle.points();
    
    int x0 = (W-1) * frand();
    int x1 = (W-1) * frand();
    int y0 = (H-1) * frand();
    int y1 = (H-1) * frand();
    
    line(x0, y0, x1, y1, set, initial, deltaX, deltaXY);
  }
  
  shared_ptr< GreyImage<int> > bg = bitmap->distanceTransform(true, 1);
  shared_ptr< GreyImage<int> > fg = bitmap->distanceTransform(false, 1);
  
  for (int threads = 2; threads <= 8; threads *= 2) {
    shared_ptr< GreyImage<int> > testBg = bitmap->distanceTransform(true, threads);
    REQUIRE(*testBg == *bg);
    
    shared_ptr< GreyImage<int> > testFg = bitmap->distanceTransform(false, threads);
    REQUIRE(*testFg == *fg);
  }
}

TEST_CASE("distanceTransform/workers", "verify that single- and multi-threaded give the same result")
{
  int W = 200;
  int H = 200;
  shared_ptr<Bitmap> bitmap = Bitmap::make(W, H, true);
  Bitmap::Set set = bitmap->set(true);
  
  for (int a = 0; a < (W+H)/10; a++) {
    int r = (W/10) * frand();
    Circle circle(r);
    
    int extents[r];
    Circle::makeExtents(r, extents);
    const list<IPoint> &deltaX = circle.getHorizontalDelta();
    const list<IPoint> &deltaXY = circle.getDiagonalDelta();
    const list<IPoint> &initial = circle.points();
    
    int x0 = (W-1) * frand();
    int x1 = (W-1) * frand();
    int y0 = (H-1) * frand();
    int y1 = (H-1) * frand();
    
    line(x0, y0, x1, y1, set, initial, deltaX, deltaXY);
  }
  
  shared_ptr< GreyImage<int> > bg = bitmap->distanceTransform(true, 1);
  shared_ptr< GreyImage<int> > fg = bitmap->distanceTransform(false, 1);
  
  for (int threads = 2; threads <= 8; threads *= 2) {
    Workers workers(threads);
    
    shared_ptr< GreyImage<int> > testBg = bitmap->distanceTransform(true, workers);
    REQUIRE(*testBg == *bg);
    
    shared_ptr< GreyImage<int> > testFg = bitmap->distanceTransform(false, workers);
    REQUIRE(*testFg == *fg);
  }
}
