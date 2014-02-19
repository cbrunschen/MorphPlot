//
//  TestMeijer.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 09/02/2014.
//
//
#include <sys/time.h>

#include "Primitives.h"
#include "Circle.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "GreyImage.h"
#include "GreyImage_Impl.h"

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
#include <stdlib.h>

using namespace Primitives;
using namespace Images;
using namespace std;

double now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double)(tv.tv_sec % 86400) + ((double)tv.tv_usec / 1000000.0);
}

double frand() {
  return (double) rand() / ((double) RAND_MAX);
}

class PGI {
	Ref< GreyImage<int> > g_;
public:
	PGI(Ref< GreyImage<int> > g) : g_(g) { }	
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


int main(int argc, char **argv) {
  Ref<Bitmap> bitmap = Bitmap::make(40, 40, true);
  
  for (int y = 20; y < 21; y++) {
    for (int x = 20; x < 21; x++) {
      bitmap->at(y, x) = true;
    }
  }

  cout << "bitmap: " << endl << bitmap << endl << flush;

  Ref< GreyImage<int> > bg = bitmap->distanceTransform(true);
  cout << "Background:" << endl << PGI(bg) << endl << flush;  

  Ref< GreyImage<int> > fg = bitmap->distanceTransform(false);
  cout << "Foreground:" << endl << PGI(fg) << endl << flush;  

  Ref<Bitmap> inset = bitmap->inset(5);
  cout << "inset by 5:" << endl << inset << endl << flush;

  for (int r = 0; r < 10; r++) {
    Ref<Bitmap> outset = bitmap->outset(r);
    cout << "outset by " << r << ":" << endl << outset << endl << flush;
    cout << "difference should be " << r << ":" << endl << (*outset - *bitmap) << endl << flush;
  }
  
  for (int y = 2; y < 38; y++) {
    for (int x = 2; x < 38; x++) {
      bitmap->at(y, x) = true;
    }
  }
  
  for (int r = 0; r < 10; r++) {
    Ref<Bitmap> inset = bitmap->inset(r);
    cout << "inset by " << r << ":" << endl << inset << endl << flush;
    cout << "difference should be " << r << ":" << endl << (*bitmap - *inset) << endl << flush;
  }
  
  int W = 5000;
  int H = 5000;
  Ref<Bitmap> large = Bitmap::make(W, H, true);
  Bitmap::Set set = large->set(true);
  
  for (int a = 0; a < 200; a++) {
    int r = 50 * frand();
    Circle circle(r);
  
    int extents[r];
    Circle::makeExtents(r, extents);
    const list<Point> &deltaX = circle.getHorizontalDelta();
    const list<Point> &deltaXY = circle.getDiagonalDelta();
    const list<Point> &initial = circle.points();
    
    int x0 = (W-1) * frand();
    int x1 = (W-1) * frand();
    int y0 = (H-1) * frand();
    int y1 = (H-1) * frand();
    
    line(x0, y0, x1, y1, set, initial, deltaX, deltaXY);
  }
  
  large->writePng("/tmp/large.png");

#if 0
  cerr << "insetting old style ... " << flush;
  double t0 = now();
  Ref<Bitmap> insetOld = large->inset_old(40);
  double t1 = now();
  cerr << (t1 - t0) << endl << flush;
  insetOld = NULL;
  
  cerr << "insetting new style ... " << flush;
  double t2 = now();
  Ref<Bitmap> insetNew = large->inset(40);
  double t3 = now();
  cerr << (t3 - t2) << endl << flush;
  cerr << W << "x" << H << ": " << static_cast<int>((double)(W * H) / (t3 - t2)) << " pixels per second" << endl << flush;
#endif
  
  for (int w = W/5; w <= W; w += 2*W/5) {
    for (int h = H/6; h <= H; h += 2*H/5) {
      Ref<Bitmap> bm = scaleImage(large, (double)w / (double)W, (double)h / (double)H);
    
      for (int threads = 1; threads <= 8; threads *= 2) {
        double t2 = now();
        Ref<Bitmap> insetNew = large->inset(40, threads);
        double t3 = now();
        cerr << w << "x" << h << ", " << threads << " threads: " << static_cast<int>((double)(W * H) / (t3 - t2)) << " pixels per second" << endl << flush;
      }
    }
  }
}