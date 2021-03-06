//
//  TestGreyImage.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 15/02/2014.
//
//

#include "TestGreyImage.h"

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

static inline int randInt(int max) {
  return max * frand();
}

static inline void verifySame(const Bitmap &a, const Bitmap &b) {
  for (int r = 0; r < N; r++) {
    for (int c = 0; c < N; c++) {
      if (a.at(c, r) != b.at(c, r)) {
        cout << "difference at (" << r << "," << c << ")" << endl << flush;
      }
    }
  }
}

#ifdef T
#undef T
#endif
#define T(op) do {                                                                                       \
  double t0 = now();                                                                                     \
  shared_ptr<Bitmap> seq = image.op(cutoff);                                                             \
  double t1 = now();                                                                                     \
  shared_ptr<Bitmap> par = image.op(cutoff, workers);                                                    \
  double t2 = now();                                                                                     \
  double t01 = t1 - t0;                                                                                  \
  double t12 = t2 - t1;                                                                                  \
  double speedup = t01 / t12;                                                                            \
  cout << #op << " sequential: " << t01 << ", " << N << " parallel: " << t12                             \
       << ", speedup: " << speedup << endl << flush;                                                     \
  verifySame(*seq, *par);                                                                                \
} while (0)

TEST_CASE("GreyImage/parallels", "Test serial vs parallel performance of greater-than-or-equal")
{
  int S = 2000;
  int max = 200;
  GreyImage<int> image(S, S);

  for (GreyImage<int>::iterator i = image.begin(); i != image.end(); ++i) {
    *i = randInt(max);
  }

  int cutoff = max / 2;

  for (int N = 2; N <= 8; N *= 2) {
    Workers workers(N);

    T(ge);
    T(le);
    T(gt);
    T(lt);
    T(eq);
    T(ne);
  }
}

#undef T
