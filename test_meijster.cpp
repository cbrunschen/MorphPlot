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

#include "OpenCLWorkers.h"

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
#include <stdlib.h>

using namespace Primitives;
using namespace Images;
using namespace std;

//double now() {
//  struct timeval tv;
//  gettimeofday(&tv, NULL);
//  return (double)(tv.tv_sec % 86400) + ((double)tv.tv_usec / 1000000.0);
//}

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

class PPI {
	shared_ptr< Image<IPoint> > g_;
public:
	PPI(shared_ptr< Image<IPoint> > g) : g_(g) { }
	void print(ostream &out) const {
    for (int y = 0; y < g_->height(); y++) {
      for (int x = 0; x < g_->width(); x++) {
    	  out << setw(3) << right << g_->at(y, x).x() << "," << setw(3) << left << g_->at(y, x).y() << " ";
      }
      out << endl;
    }
	}
};

class PCI {
	shared_ptr< Image<cl_short2> > g_;
public:
	PCI(shared_ptr< Image<cl_short2> > g) : g_(g) { }
	void print(ostream &out) const {
    for (int y = 0; y < g_->height(); y++) {
      for (int x = 0; x < g_->width(); x++) {
    	  out << setw(3) << right << g_->at(y, x).x << "," << setw(3) << left << g_->at(y, x).y << " ";
      }
      out << endl;
    }
	}
};


inline ostream &operator<<(ostream &out, const PGI &pgi) {
	pgi.print(out);
	return out;
}
inline ostream &operator<<(ostream &out, const PPI &ppi) {
	ppi.print(out);
	return out;
}
inline ostream &operator<<(ostream &out, const PCI &pci) {
	pci.print(out);
	return out;
}

int main(int argc, char **argv) {
#if 0
  shared_ptr<Bitmap> bitmap = Bitmap::make(40, 40, true);
  
  for (int y = 15; y < 26; y++) {
    for (int x = 0; x < 40; x++) {
      bitmap->at(y, x) = true;
    }
  }

  for (int y = 0; y < 40; y++) {
    for (int x = 15; x < 23; x++) {
      bitmap->at(y, x) = true;
    }
  }
  
  cout << "bitmap: " << endl << *bitmap << endl << flush;

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
  
  for (int y = 2; y < 38; y++) {
    for (int x = 2; x < 38; x++) {
      bitmap->at(y, x) = true;
    }
  }
  
  for (int r = 0; r < 10; r++) {
    shared_ptr<Bitmap> inset = bitmap->inset(r);
    cout << "inset by " << r << ":" << endl << *inset << endl << flush;
    cout << "difference should be " << r << ":" << endl << *(*bitmap - *inset) << endl << flush;
  }
#endif

#if 1
  int W = 80;
  int H = 300;
#else
  int W = 32;
  int H = 60;
#endif

  shared_ptr<Bitmap> large = Bitmap::make(W, H, true);
  Bitmap::Set set = large->set(true);
  
  rng.seed(10017);
  
#if 0
  for (int a = 0; a < 10; a++) {
    int r = 0.05 * frand() * sqrt(W*W+H*H);
    if (r == 0) r = 1;
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
#else 
  for (int a = 0; a < (W * H / 100); a++) {
    int x = (W-1) * frand();
    int y = (H-1) * frand();
    set(y, x);
  }
#endif
  
  large->writePng("/tmp/large.png");

#if 0
  cerr << "insetting old style ... " << flush;
  double t0 = now();
  shared_ptr<Bitmap> insetOld = large->inset_old(40);
  double t1 = now();
  cerr << (t1 - t0) << endl << flush;
  insetOld = NULL;
  
  cerr << "insetting new style ... " << flush;
  double t2 = now();
  shared_ptr<Bitmap> insetNew = large->inset(40);
  double t3 = now();
  cerr << (t3 - t2) << endl << flush;
  cerr << W << "x" << H << ": " << static_cast<int>((double)(W * H) / (t3 - t2)) << " pixels per second" << endl << flush;
  
  for (int w = W/5; w <= W; w += 2*W/5) {
    for (int h = H/5; h <= H; h += 2*H/5) {
      shared_ptr<Bitmap> bm = scaleImageTo(large, w, h);
      
      for (int threads = 1; threads <= 16; threads *= 2) {
        double t2 = now();
        shared_ptr<Bitmap> inset = bm->inset(40, threads);
        double t3 = now();
        cerr << "inset  " << w << "x" << h << ", " << threads << " threads: " << static_cast<int>((double)(w * h) / (t3 - t2)) << " pixels per second" << endl << flush;
        
        inset.reset();
        
        double t4 = now();
        shared_ptr<Bitmap> outset = bm->outset(40, threads);
        double t5 = now();
        cerr << "outset " << w << "x" << h << ", " << threads << " threads: " << static_cast<int>((double)(w * h) / (t5 - t4)) << " pixels per second" << endl << flush;
      }
    }
  }
#endif

#if 0
  shared_ptr< Image<IPoint> > ft = large->featureTransform(true, 8);
  
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      cout << setw(2) << right << ft->at(y, x).x() << "," << setw(2) << left << ft->at(y, x).y() << " ";
    }
    cout << endl << flush;
  }
#endif

  Workers workers(8);
  OpenCLWorkers clWorkers(2);
  int N = 1;
  
  shared_ptr< Image<cl_short2> > oclResult;
  shared_ptr< Image<IPoint> > cpuResult;

  cerr << "starting measurements" << endl << flush;
  double t0 = now();
  for (int i = 0; i < N; i++) {
    cerr << i << ", " << flush;
    oclResult = large->clFeatureTransform<true>(clWorkers);
  } while (0);
  double t1 = now();
  cerr << "OCL: " << (1000.0 * (t1 - t0)) << "ms" << endl << flush;
  cerr << PCI(oclResult) << endl << flush;

  double t2 = now();
  for (int i = 0; i < N; i++) {
    cerr << i << ", " << flush;
    cpuResult = large->featureTransform(true, workers);
  }
  double t3 = now();
  cerr << "CPU: " << (1000.0 * (t3 - t2)) << "ms" << endl << flush;
  cerr << PPI(cpuResult) << endl << flush;
  
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      if (oclResult->at(y, x).x != cpuResult->at(y, x).x() || oclResult->at(y, x).y != cpuResult->at(y, x).y()) {
        int dxCpu = cpuResult->at(y, x).x() - x;
        int dyCpu = cpuResult->at(y, x).y() - y;
        int dCpu = dxCpu*dxCpu + dyCpu*dyCpu;
        int dxOcl = oclResult->at(y, x).x - x;
        int dyOcl = oclResult->at(y, x).y - y;
        int dOcl = dxOcl*dxOcl + dyOcl*dyOcl;
        cerr << "differ at (" << x << "," << y << "): want (" <<  cpuResult->at(y, x).x() << "," << cpuResult->at(y, x).y() << ") @ " << dCpu << ", have (" << oclResult->at(y, x).x << "," << oclResult->at(y, x).y << ") @ " << dOcl << " ~= " << ((int16_t)dOcl) << endl << flush;
      }
    }
  }
  
//  for (int y = 0; y < H; y++) {
//    for (int x = 0; x < W; x++) {
//      cerr << setw(3) << right << oclResult->at(y, x).x << "," << setw(3) << left << oclResult->at(y, x).y << " ";
//    }
//    cerr << endl;
//    for (int x = 0; x < W; x++) {
//      if (x == oclResult->at(y, x).x && y == oclResult->at(y, x).y) {
//        cerr << "   *    ";
//      } else {
//        int dx = oclResult->at(y, x).x - cpuResult->at(y, x).x();
//        int dy = oclResult->at(y, x).y - cpuResult->at(y, x).y();
//        cerr << setw(3) << right << dx << "," << setw(3) << left << dy << " ";
//      }
//    }
//    cerr << endl;
//    for (int x = 0; x < W; x++) {
//      cerr << setw(3) << right << cpuResult->at(y, x).x() << "," << setw(3) << left << cpuResult->at(y, x).y() << " ";
//    }
//    cerr << endl << endl;
//  }
//  cerr << endl << flush;
}