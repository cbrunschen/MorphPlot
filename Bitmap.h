/*
 *  Bitmap.h
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Bitmap_h__
#define __Bitmap_h__

#include "FloodFill.h"
#include "Primitives.h"
#include "Image.h"
#include "Chain.h"
#include "Line.h"
#include "Hatcher.h"
#include "FloodFill.h"
#include "Workers.h"
#include "OpenCLWorkers.h"

#include <list>
#include <vector>
#include <functional>
#include <set>

extern "C" {
#include <png.h>
}

namespace Images {
#if 0
}
#endif

template<typename C> class GreyImage;

#ifndef D
#define D(x)
#endif

using namespace std;
using namespace Primitives;
using namespace Neighbourhood;
using namespace Hatching;
using namespace Filling;

class Bitmap;

namespace Bitmaps {
  extern bool writePng(const Bitmap &image, FILE *fp);
  extern shared_ptr<Bitmap> readPng(FILE *fp);
}

class Bitmap : public Image<bool> {
public:
  typedef Bitmap Self;
  typedef shared_ptr<Self> BitmapRef;
  typedef Image<bool> Super;
  typedef Super::Row Row;

  Bitmap(int width = 0, int height = 0, bool doClear = true);

  Bitmap(const Bitmap &other);

  static BitmapRef make(int width = 0, int height = 0, bool doClear = true);

  static BitmapRef make(const Bitmap * const bitmap);

  static BitmapRef make(const Bitmap &bitmap);

  BitmapRef clone();

  istream &readData(istream &in);

  ostream &writeData(ostream &out);

  bool writePng(FILE *f);
  bool writePng(const char * const filename);
  bool writePng(const string &filename);

  static BitmapRef readPng(FILE *fp);
  static BitmapRef readPng(const char * const filename);
  static BitmapRef readPng(const string &filename);

  ostream &writeText(ostream &out);

  const bool isEmpty() const;

  template<bool background> void distanceTransformPass1(int x0, int x1, int *g) const;
  template<bool background> void distanceTransformPass2(int *g, int y0, int y1, int *result) const;
  template<bool background> void distanceTransform(int x0, int x1, int y0, int y1, int *g, int *result) const;

  shared_ptr< GreyImage<int> > distanceTransform(bool background = true, int threads = 1) const;
  shared_ptr< GreyImage<int> > distanceTransform(bool background, Workers &workers) const;
  shared_ptr< GreyImage<int> > distanceTransform(bool background, shared_ptr<Workers> workers) const;
  shared_ptr< GreyImage<int> > distanceTransform(bool background, Workers *workers) const;

  template<bool background> void featureTransformPass1(int x0, int x1, int *g, int *ys) const;
  template<bool background> void featureTransformPass2(int *g, int *ys, int y0, int y1, IPoint *result) const;
  template<bool background> void featureTransform(int x0, int x1, int y0, int y1, int *g, int *ys, IPoint *result) const;

  shared_ptr< Image<IPoint> > featureTransform(bool background = true, int threads = 1) const;
  shared_ptr< Image<IPoint> > featureTransform(bool background, Workers &workers) const;
  shared_ptr< Image<IPoint> > featureTransform(bool background, shared_ptr<Workers> workers) const;
  shared_ptr< Image<IPoint> > featureTransform(bool background, Workers *workers) const;

  template<bool background> shared_ptr< GreyImage<cl_uint> > clDistanceTransform(OpenCLWorkers &workers) const;
  template<bool background> shared_ptr< Image<cl_short2> > clFeatureTransform(OpenCLWorkers &workers) const;
  template<bool background> shared_ptr< Image<cl_short2> > clFeatureTransform_transposed(OpenCLWorkers &workers) const;

  BitmapRef inset(int r, int threads = 1) const;

  BitmapRef outset(int r, int threads = 1) const;

  BitmapRef close(int r, int threads = 1) const;

  BitmapRef open(int r, int threads = 1) const;

  BitmapRef inset(int r, Workers &workers) const;

  BitmapRef outset(int r, Workers &workers) const;

  BitmapRef close(int r, Workers &workers) const;

  BitmapRef open(int r, Workers &workers) const;

  BitmapRef inset(int r, shared_ptr<Workers> workers) const;
  
  BitmapRef outset(int r, shared_ptr<Workers> workers) const;
  
  BitmapRef close(int r, shared_ptr<Workers> workers) const;
  
  BitmapRef open(int r, shared_ptr<Workers> workers) const;

  BitmapRef inset(int r, Workers *workers) const;
  
  BitmapRef outset(int r, Workers *workers) const;
  
  BitmapRef close(int r, Workers *workers) const;
  
  BitmapRef open(int r, Workers *workers) const;

  BitmapRef inset_old(int r) const;

  BitmapRef inset_old(const Circle &c) const;

  BitmapRef outset_old(int r) const;

  BitmapRef outset_old(const Circle &c) const;

  BitmapRef close_old(const Circle &c) const;

  BitmapRef close_old(int r) const;

  BitmapRef open_old(const Circle &c) const;

  BitmapRef open_old(int r) const;

  // the 'old' thinning patterns
  static bool oldThinningPatterns[256];

  // the smoothing patterns
  static bool smoothingPatterns[256];
  static int neighbourCounts[256];
  static int transitions[256];

  // the neighbourhood value for a top-left square corner
  static int corners[4];

  // one iteration of thinning using the algorith of Ng, Zhou and Quek - implementation 2
  int thin(Bitmap &flags, bool ry, bool rx);

  // one iteration of thinning using the algorith of Ng, Zhou and Quek
  int thinOld(Bitmap &flags);

  int thin();

  void prune();

  void prune(int n);

  void clearThinConnected();

  struct IsMarkedForReconstruction;

  BitmapRef reconstruct(const Bitmap &reference);

  BitmapRef reconstruct(const BitmapRef &reference);

  BitmapRef operator+(const Bitmap &other) const;

  Bitmap &operator+=(const Bitmap &other);

  BitmapRef operator-(const Bitmap &other) const;

  Bitmap &operator-=(const Bitmap &other);

  // boundary scanning
  Direction nextDir(const IPoint &p, Direction dir, Turn delta);

  Direction nextDirCW(const IPoint &p, Direction dir);

  Direction nextDirCCW(const IPoint &p, Direction dir);

  void scanBoundary(Image<int> &marks, int mark, Boundary &boundary, const IPoint &start, Direction from, bool connect = false);

  void scanBoundaries(vector<Boundary> &result, Image<int> &marks, bool connect = false);

  vector<Boundary> scanBoundaries(bool connect = false);

  int neighbours(const IPoint &p) const;

  int countNeighbours(const IPoint &p) const;

  template<typename CanRetract> void hatch(Chains &chains, double angle, double period, double phase, CanRetract &canRetract);

  template<typename CanRetract> Chains hatch(double angle, double period, double phase, CanRetract &canRetract);

  void retract(const Bitmap &reference, const Circle &circle);

  void retract(const BitmapRef &reference, const Circle &circle);

  BitmapRef adjacent(const Bitmap &other);

  BitmapRef adjacent(const BitmapRef &other);
};

ostream &operator<<(ostream &out, const Bitmap &i);

#if 0
{
#endif
}  // namespace Images

#endif // __Bitmap_h__
