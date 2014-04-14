//
//  Bitmap_Impl.h
//  MorphPlot
//
//  Created by Christian Brunschen on 09/02/2014.
//
//

#ifndef __Bitmap_Impl_h__
#define __Bitmap_Impl_h__

#include "Bitmap.h"
#include "Image_Impl.h"
#include "GreyImage.h"
#include "GreyImage_Impl.h"

#include "Workers.h"

#include <memory>
#include <algorithm>
#include <iomanip>

namespace Images {
#if 0
}
#endif

inline Bitmap::Bitmap(const int width, const int height, const bool doClear)
: Super(width, height, doClear) { }

inline Bitmap::Bitmap(const Bitmap &other) : Super(other) { }

inline shared_ptr<Bitmap> Bitmap::make(int width, int height, bool doClear) {
  return make_shared<Bitmap>(width, height, doClear);
}

inline shared_ptr<Bitmap> Bitmap::make(const Bitmap * const bitmap) {
  return make_shared<Bitmap>(*bitmap);
}

inline shared_ptr<Bitmap> Bitmap::make(const Bitmap &bitmap) {
  return make_shared<Bitmap>(bitmap);
}

inline shared_ptr<Bitmap> Bitmap::clone() {
  return make(this);
}

inline istream &Bitmap::readData(istream &in) {
  int y, x, xx;
  for (y = 0; y < height_; y++) {
    for (x = 0; x < width_; x += 8) {
      uint8_t b;
      in.read((char *)&b, 1);
      uint8_t mask = 0x80;
      for (xx = x; xx < x+8 && xx < width_; xx++) {
        at(y, xx) = ((b & mask) != 0);
        mask >>= 1;
      }
    }
  }
  return in;
}

inline ostream &Bitmap::writeData(ostream &out) {
  int y, x, xx;
  for (y = 0; y < height_; y++) {
    for (x = 0; x < width_; x += 8) {
      uint8_t b = 0;
      uint8_t mask = 0x80;
      for (xx = x; xx < x+8 && xx < width_; xx++) {
        if (at(y, xx)) {
          b |= mask;
        }
        mask >>= 1;
      }
      out.write((const char *)&b, 1);
    }
  }
  return out;
}

inline bool Bitmap::writePng(FILE *f) {
  return Bitmaps::writePng(*this, f);
}

inline bool Bitmap::writePng(const char * const filename) {
  FILE *f = fopen(filename, "w");
  if (f != NULL) {
    return writePng(f);
  }
  return false;
}

inline bool Bitmap::writePng(const string &filename) {
  return writePng(filename.c_str());
}

inline shared_ptr<Bitmap> Bitmap::readPng(FILE *fp) {
  return Bitmaps::readPng(fp);
}

inline shared_ptr<Bitmap> Bitmap::readPng(const char * const filename) {
  FILE *f = fopen(filename, "r");
  if (f != NULL) {
    return readPng(f);
  }
  return shared_ptr<Bitmap>(nullptr);
}

inline shared_ptr<Bitmap> Bitmap::readPng(const string &filename) {
  return readPng(filename.c_str());
}

inline ostream &Bitmap::writeText(ostream &out) {
  int y, x;
  for (y = 0; y < height_; y++) {
    for (x = 0; x < width_; x++) {
      out << (data_[y*width_ + x] ? '#' : '.');
    }
    out << endl;
  }
  out << flush;
  return out;
}

inline const bool Bitmap::isEmpty() const {
  bool *dataEnd = data_ + (width_ * height_);
  for (bool *b = data_; b != dataEnd; b++) {
    if (*b) return false;
  }
  return true;
}

inline static int distanceFromColumn(int column, int x, int *g) {
  int gi2 = g[column];
  int dx = (x - column);
  return dx*dx + gi2;
}

inline static int intersection(int i, int gi2, int u, int gu2) {
  return (u*u - i*i + gu2 - gi2) / (2 * (u - i));
}

inline static int meijsterSeparation_euclidean(int i, int u, int *g) {
  return intersection(i, g[i], u, g[u]);
}

template<bool background>
inline void Bitmap::distanceTransformPass1(int x0, int x1, int *g) const {
  int infinity = width_ + height_ + 1;
  infinity *= infinity;
  int stride = width_;
  
  for (int x = x0; x < x1; x++) {
    // scan 1:
    bool *src = &data_[x];
    bool *srcEnd = src + height_ * stride;
    int *p = &g[x];
    int distance, difference;
    
    // - start with the primitive value
    if (*src == background) {
      distance = *p = 0;
      difference = 1;
    } else if (background) {
      distance = *p = infinity;
      difference = 1;
    } else {
      distance = *p = 1;
      difference = 3;
    }
    
    // - calculate the next one from the previous one(s)
    for (p += stride, src += stride; src < srcEnd; p += stride, src += stride) {
      if (*src == background) {
        distance = *p = 0;
        difference = 1;
        // scan backwards
        int rDistance = 1;
        int rDifference = 3;
        for (int *q = p - stride; q >= g && *q > rDistance; q -= stride) {
          *q = rDistance;
          rDistance += rDifference;
          rDifference += 2;
        }
      } else {
        if (distance == infinity) {
          *p = infinity;
        } else {
          distance = *p = distance + difference;
          difference += 2;
        }
      }
    }
    
    if (!background) {
      // scan backwards from the far edge
      int rDistance = 1;
      int rDifference = 3;
      for (int *q = p - stride; q >= g && *q > rDistance; q -= stride) {
        *q = rDistance;
        rDistance += rDifference;
        rDifference += 2;
      }
    }
  }
}

template<bool background>
inline void Bitmap::distanceTransformPass2(int *g, int y0, int y1, int *result) const {
  int dominantColumn[width_ + 2];
  int dominanceStarts[width_ + 2];
  int *gs, *gr;
  
  if (!background) {
    gs = new int[width_ + 2];
    gr = &gs[1];
    gr[-1] = gr[width_] = background ? width_ + height_ + 1 : 0;
  }
  
  int u0 = background ? 1 : 0;
  int u1 = background ? width_ : width_ + 1;
  
  for (int y = y0; y < y1; y++) {
    int *dst = &result[y * width_];
    if (background) {
      gr = &g[y * width_];
    } else {
      int *src = &g[y * width_];
      std::copy(src, src + width_, gr);
    }
    int q = 0;
    dominanceStarts[0] = 0;
    dominantColumn[0] = background ? 0 : -1;
    
    // scan 3
    for (int u = u0; u < u1; u++) {
      while (q >= 0 && distanceFromColumn(dominantColumn[q], dominanceStarts[q], gr) > distanceFromColumn(u, dominanceStarts[q], gr)) {
        q--;
      }
      
      if (q < 0) {
        q = 0;
        dominantColumn[0] = u;
      } else {
        int w = 1 + meijsterSeparation_euclidean(dominantColumn[q], u, gr);
        if (w < width_) {
          q++;
          dominantColumn[q] = u;
          dominanceStarts[q] = w;
        }
      }
    }
    
    // scan 4
    for (int u = width_ - 1; u >= 0; u--) {
      dst[u] = distanceFromColumn(dominantColumn[q], u, gr);
      if (u == dominanceStarts[q]) {
        q--;
      }
    }
  }
}

template<bool background>
inline void Bitmap::distanceTransform(int x0, int x1, int y0, int y1, int *g, int *result) const {
  distanceTransformPass1<background>(x0, x1, g);
  distanceTransformPass2<background>(g, y0, y1, result);
}

struct distance_transform_params {
  const Bitmap *bitmap;
  int x0, x1, y0, y1;
  int *g;
  int *result;
  Workers::Barrier *barrier;
};

extern void distance_transform_thread_background(void *params);
extern void distance_transform_thread_foreground(void *params);

inline shared_ptr< GreyImage<int> > Bitmap::distanceTransform(bool background, Workers &workers) const {
  int threads = workers.n();
  shared_ptr< GreyImage<int> > g = GreyImage<int>::make(width_, height_, false);
  shared_ptr< GreyImage<int> > result = GreyImage<int>::make(width_, height_, false);
  distance_transform_params *dt_params = new distance_transform_params[threads];
  Workers::Barrier barrier;
  
  workers.initBarrier(&barrier);
  
  for (int i = 0; i < threads; i++) {
    dt_params[i].bitmap = this;
    dt_params[i].x0 = ((width_ * i) / threads);
    dt_params[i].x1 = ((width_ * (i + 1)) / threads);
    dt_params[i].y0 = ((height_ * i) / threads);
    dt_params[i].y1 = ((height_ * (i + 1)) / threads);
    dt_params[i].g = g->data();
    dt_params[i].result = result->data();
    dt_params[i].barrier = &barrier;
  }

  if (background) {
    workers.perform<distance_transform_params>(distance_transform_thread_background, &dt_params[0]);
  } else {
    workers.perform<distance_transform_params>(distance_transform_thread_foreground, &dt_params[0]);
  }
  
  workers.destroyBarrier(&barrier);
  
  return result;
}

inline shared_ptr< GreyImage<int> > Bitmap::distanceTransform(bool background, int threads) const {
  if (threads > 1) {
    Workers workers(threads);
    return distanceTransform(background, workers);
  } else {
    shared_ptr< GreyImage<int> > g = GreyImage<int>::make(width_, height_, false);
    shared_ptr< GreyImage<int> > result = GreyImage<int>::make(width_, height_, false);
    if (background) {
      distanceTransformPass1<true>(0, width_, g->data());
      distanceTransformPass2<true>(g->data(), 0, height_, result->data());
    } else {
      distanceTransformPass1<false>(0, width_, g->data());
      distanceTransformPass2<false>(g->data(), 0, height_, result->data());
    }
    return result;
  }
}

template<bool background>
inline void Bitmap::featureTransformPass1(int x0, int x1, int *g, int *ys) const {
  int infinity = width_ + height_ + 1;
  infinity *= infinity;
  int stride = width_;
  
  for (int x = x0; x < x1; x++) {
    // scan 1:
    bool *src = &data_[x];
    bool *srcEnd = src + height_ * stride;
    int *p = &g[x];
    int *yp = &ys[x];
    int y, ny;
    int distance, difference;
    
    // - start with the primitive value
    if (*src == background) {
      distance = *p = 0;
      difference = 1;
      *yp = ny = 0;
    } else if (background) {
      distance = *p = infinity;
      difference = 1;
      *yp = ny = 0-height_;
    } else {
      distance = *p = 1;
      difference = 3;
      *yp = ny = -1;
    }
    
    // - calculate the next one from the previous one(s)
    for (y = 1, p += stride, src += stride, yp += stride; src < srcEnd; ++y, p += stride, src += stride, yp += stride) {
      if (*src == background) {
        distance = *p = 0;
        difference = 1;
        *yp = ny = y;
        // scan backwards
        int rDistance = 1;
        int rDifference = 3;
        for (int *q = p - stride, *yq = yp - stride; q >= g && *q > rDistance; q -= stride, yq -= stride) {
          *q = rDistance;
          *yq = ny;
          rDistance += rDifference;
          rDifference += 2;
        }
      } else {
        if (distance == infinity) {
          *p = infinity;
        } else {
          distance = *p = distance + difference;
          difference += 2;
        }
        *yp = ny;
      }
    }
    
    if (!background) {
      // scan backwards from the far edge
      int rDistance = 1;
      int rDifference = 3;
      for (int *q = p - stride, *yq = yp - stride; q >= g && *q > rDistance; q -= stride, yq -= stride) {
        *q = rDistance;
        *yq = height_;
        rDistance += rDifference;
        rDifference += 2;
      }
    }
  }
}

template<bool background>
inline void Bitmap::featureTransformPass2(int *g, int *ys, int y0, int y1, Point *result) const {
  int dominantColumn[width_ + 2];
  int dominanceStarts[width_ + 2];
  int dominantY[width_ + 2];
  int *gs, *gr;
  
  if (!background) {
    gs = new int[width_ + 2];
    gr = &gs[1];
    gr[-1] = gr[width_] = background ? width_ + height_ + 1 : 0;
  }
  
  int u0 = background ? 1 : 0;
  int u1 = background ? width_ : width_ + 1;
  
  for (int y = y0; y < y1; y++) {
    int *yr = &ys[y * width_];
    Point *dst = &result[y * width_];
    if (background) {
      gr = &g[y * width_];
    } else {
      int *src = &g[y * width_];
      std::copy(src, src + width_, gr);
    }
    int q = 0;
    dominanceStarts[0] = 0;
    dominantColumn[0] = background ? 0 : -1;
    dominantY[0] = background ? *yr : 0;
    
    // scan 3
    for (int u = u0; u < u1; u++) {
      while (q >= 0 && distanceFromColumn(dominantColumn[q], dominanceStarts[q], gr) > distanceFromColumn(u, dominanceStarts[q], gr)) {
        q--;
      }
      
      if (q < 0) {
        q = 0;
        dominantColumn[0] = u;
        dominantY[0] = yr[u];
      } else {
        int w = 1 + meijsterSeparation_euclidean(dominantColumn[q], u, gr);
        if (w < width_) {
          q++;
          dominantColumn[q] = u;
          dominanceStarts[q] = w;
          dominantY[q] = yr[u];
        }
      }
    }
    
    // scan 4
    for (int u = width_ - 1; u >= 0; u--) {
      dst[u] = Point(dominantColumn[q], dominantY[q]);
      
      if (u == dominanceStarts[q]) {
        q--;
      }
    }
  }
}

template<bool background>
inline void Bitmap::featureTransform(int x0, int x1, int y0, int y1, int *g, int *ys, Point *result) const {
  featureTransformPass1<background>(x0, x1, g, ys);
  featureTransformPass2<background>(g, ys, y0, y1, result);
}

struct feature_transform_params {
  const Bitmap *bitmap;
  int x0, x1, y0, y1;
  int *g;
  int *ys;
  Point *result;
  Workers::Barrier *barrier;
};

extern void feature_transform_thread_background(void *params);
extern void feature_transform_thread_foreground(void *params);

inline shared_ptr< Image<Point> > Bitmap::featureTransform(bool background, Workers &workers) const {
  int threads = workers.n();
  shared_ptr< GreyImage<int> > g = GreyImage<int>::make(width_, height_, false);
  shared_ptr< GreyImage<int> > ys = GreyImage<int>::make(width_, height_, false);
  shared_ptr< Image<Point> > result = Image<Point>::make(width_, height_, false);
  feature_transform_params *ft_params = new feature_transform_params[threads];
  Workers::Barrier barrier;
  
  workers.initBarrier(&barrier);
  
  for (int i = 0; i < threads; i++) {
    ft_params[i].bitmap = this;
    ft_params[i].x0 = ((width_ * i) / threads);
    ft_params[i].x1 = ((width_ * (i + 1)) / threads);
    ft_params[i].y0 = ((height_ * i) / threads);
    ft_params[i].y1 = ((height_ * (i + 1)) / threads);
    ft_params[i].g = g->data();
    ft_params[i].ys = ys->data();
    ft_params[i].result = result->data();
    ft_params[i].barrier = &barrier;
  }
  
  if (background) {
    workers.perform<feature_transform_params>(feature_transform_thread_background, &ft_params[0]);
  } else {
    workers.perform<feature_transform_params>(feature_transform_thread_foreground, &ft_params[0]);
  }
  
  workers.destroyBarrier(&barrier);
  
  return result;
}

inline shared_ptr< Image<Point> > Bitmap::featureTransform(bool background, int threads) const {
  if (threads > 1) {
    Workers workers(threads);
    return featureTransform(background, workers);
  } else {
    shared_ptr< GreyImage<int> > g = GreyImage<int>::make(width_, height_, false);
    shared_ptr< GreyImage<int> > ys = GreyImage<int>::make(width_, height_, false);
    shared_ptr< Image<Point> > result = Image<Point>::make(width_, height_, false);
    if (background) {
      featureTransformPass1<true>(0, width_, g->data(), ys->data());
      featureTransformPass2<true>(g->data(), ys->data(), 0, height_, result->data());
    } else {
      featureTransformPass1<false>(0, width_, g->data(), ys->data());
      featureTransformPass2<false>(g->data(), ys->data(), 0, height_, result->data());
    }
    return result;
  }
}

inline shared_ptr<Bitmap> Bitmap::inset_old(int r) const {
  if (r <= 0) {
    return make_shared<Bitmap>(*this);
  }
  return inset_old(Circle(r));
}

inline shared_ptr<Bitmap> Bitmap::inset_old(const Circle &c) const {
  int r = c.r();
  int twoR = r << 1;
  int s = twoR + 1;
  const int *extents = c.extents();
  int sufficient = s*s;
  int needed = c.area();
  
  int columnCounts[width_];
  memset(columnCounts, 0, sizeof(columnCounts));
  
  // sum the column counts of pixels in the rows early rows
  for (int y = 0; y < twoR; y++) {
    for (int x = 0; x < width_; x++) {
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      if (at(y, x)) {
        columnCounts[x]++;
      }
      D(cerr << columnCounts[x] << endl);
    }
  }
  
  shared_ptr<Bitmap> result = make(width_, height_, true);
  int cy = r;
  for (int y = twoR; y < height_; y++) {
    int areaCount = 0;
    
    for (int x = 0; x < twoR; x++) {
      // just update the column & area counts
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      if (at(y, x)) {
        columnCounts[x]++;
      }
      D(cerr << columnCounts[x] << endl);
      D(cerr << "area:" << areaCount << " -> ");
      areaCount += columnCounts[x];
      D(cerr << areaCount << endl);
    }
    
    int cx = r;
    // for the rest of the row we have enough coverage
    for (int x = twoR; x < width_; x++) {
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      // update the column & area counts
      if (at(y, x)) {
        columnCounts[x]++;
      }
      D(cerr << columnCounts[x] << endl);
      D(cerr << "area:" << areaCount << " -> ");
      areaCount += columnCounts[x];
      D(cerr << areaCount << endl);
      
#if DEBUG
      cerr << "(" << y << "," << x << ")/(" << cy << "," << cx << "):" << areaCount << endl;
      for (int t = s-1; t >= 0; t--) {
        cerr << columnCounts[x-t];
      }
      cerr << endl;
      for (int tdy = -r; tdy <= r; tdy++) {
        for (int tdx = -r; tdx <= r; tdx++) {
          cerr << (at(cy+tdy, cx+tdx) ? '#' : '.');
        }
        cerr << endl;
      }
#endif
      if (areaCount >= sufficient) {
        // we have enough set pixels to say 'yes'
        D(cerr << "completely covered!" << endl);
        result->at(cy, cx) = true;
      } else if (areaCount < needed) {
        // we have enough unset pixels to say 'no'
        D(cerr << "not enough set pixels!" << endl);
        result->at(cy, cx) = false;
      } else {
        D(cerr << "have to check:" << endl);
        // start by checking the extreme points
        bool covered =
        at(cy,   cx  ) &&
        at(cy-r, cx  ) &&
        at(cy+r, cx  ) &&
        at(cy,   cx-r) &&
        at(cy,   cx+r);
        D(cerr << "- extremes:" << covered << endl);
        // check the rest of the 'cross'
        for (int d = 1; covered && d <= r; d++) {
          covered = covered &&
          at(cy+d, cx  ) &&
          at(cy-d, cx  ) &&
          at(cy,   cx+d) &&
          at(cy,   cx-d);
          D(cerr << "- cross,d=" << d << ":" << covered << endl);
        }
        D(cerr << "- cross:" << covered << endl);
        
        // check the rest of the circle
        for (int d = 1; covered && d <= r; d++) {
          int extent = extents[d-1];
          for (int dd = 1; dd <= extent; dd++) {
            covered = covered &&
            at(cy+d, cx+dd) &&
            at(cy-d, cx+dd) &&
            at(cy+d, cx-dd) &&
            at(cy-d, cx-dd);
            D(cerr << "- circle,d=" << d << ",dd=" << dd << ":" << covered << endl);
          }
        }
        
        D(cerr << "- overall:" << covered << endl);
        result->at(cy, cx) = covered;
      }
      
      D(cerr << "area:" << areaCount << " -> ");
      areaCount -= columnCounts[x-twoR];
      D(cerr << areaCount << endl);
      D(cerr << "col[" << x-twoR << "]:" << columnCounts[x-twoR] << " -> ");
      if (at(y-twoR, x-twoR)) {
        columnCounts[x-twoR]--;
      }
      D(cerr << columnCounts[x-twoR] << endl);
      
      cx++;
    }
    // update the column counts for the remaining columns
    for (int x = width_ - twoR; x < width_; x++) {
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      if (at(y-twoR, x)) {
        columnCounts[x]--;
      }
      D(cerr << columnCounts[x] << endl);
    }
    cy++;
  }
  return result;
}

inline shared_ptr<Bitmap> Bitmap::outset_old(int r) const {
  if (r <= 0) {
    return make_shared<Bitmap>(*this);
  }
  return outset_old(Circle(r));
}

inline shared_ptr<Bitmap> Bitmap::outset_old(const Circle &c) const {
  int r = c.r();
  int twoR = r << 1;
  int s = twoR + 1;
  int circle = c.area();
  const int *extents = c.extents();
  int outside = s*s - circle;
  
  int columnCounts[width_];
  memset(columnCounts, 0, sizeof(columnCounts));
  
  // sum the column counts of pixels in the early rows
  for (int y = 0; y < r; y++) {
    for (int x = 0; x < width_; x++) {
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      if (get(y, x)) {
        columnCounts[x]++;
      }
      D(cerr << columnCounts[x] << endl);
    }
  }
  
  shared_ptr<Bitmap> result = make(width_, height_);
  for (int y = r; y < height_+r; y++) {
    D(cerr << "*** Row " << y << endl);
    int cy = y - r;
    int areaCount = 0;
    
    for (int x = 0; x < r; x++) {
      // just update the column & area counts
      D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
      if (get(y, x)) {
        columnCounts[x]++;
      }
      D(cerr << columnCounts[x] << endl);
      D(cerr << "area:" << areaCount << " -> ");
      areaCount += columnCounts[x];
      D(cerr << areaCount << endl);
    }
    
    // for the rest of the row we have enough coverage
    for (int x = r; x < width_+r; x++) {
      int cx = x - r;
      
      if (y < height_ && x < width_) {
        D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
        if (at(y, x)) {
          columnCounts[x]++;
        }
        D(cerr << columnCounts[x] << endl);
      } else {
        D(cerr << "# (" << y << "," << x << ") outside, columnCount unchanged" << endl);
      }
      if (x < width_) {
        D(cerr << "area:" << areaCount << " -> ");
        areaCount += columnCounts[x];
        D(cerr << areaCount << endl);
      } else {
        D(cerr << "# (" << y << "," << x << ") outside, areaCount unchanged" << endl);
      }
      
#if DEBUG
      cerr << "(" << y << "," << x << ")/(" << cy << "," << cx << "):" << areaCount << endl;
      for (int t = s-1; t >= 0; t--) {
        cerr << ((0 <= (x-t) && (x-t) < width_) ? columnCounts[x-t] : 0) << "  ";
      }
      cerr << endl;
      for (int tdy = -r; tdy <= r; tdy++) {
        for (int tdx = -r; tdx <= r; tdx++) {
          cerr << (get(cy+tdy, cx+tdx) ? '#' : '.') << "  ";
        }
        cerr << endl;
      }
#endif
      
      if (areaCount == 0) {
        // we have enough un-set pixels to say 'no'
        D(cerr << "completely uncovered!" << endl);
        result->at(cy, cx) = false;
      } else if (areaCount > outside) {
        // we have enough set pixels to say 'yes'
        D(cerr << "enough set pixels!" << endl);
        result->at(cy, cx) = true;
      } else {
        D(cerr << "have to check:" << endl);
        // start by checking the extreme points
        bool covered =
        get(cy,   cx  ) ||
        get(cy-r, cx  ) ||
        get(cy+r, cx  ) ||
        get(cy,   cx-r) ||
        get(cy,   cx+r);
        D(cerr << "- extremes:" << covered << endl);
        // check the rest of the 'cross'
        for (int d = 1; !covered && d <= r; d++) {
          covered = covered ||
          get(cy+d, cx  ) ||
          get(cy-d, cx  ) ||
          get(cy,   cx+d) ||
          get(cy,   cx-d);
          D(cerr << "- cross,d=" << d << ":" << covered << endl);
        }
        D(cerr << "- cross:" << covered << endl);
        
        // check the rest of the circle
        for (int d = 1; !covered && d <= r; d++) {
          int extent = extents[d-1];
          for (int dd = 1; !covered && dd <= extent; dd++) {
            covered = covered ||
            get(cy+d, cx+dd) ||
            get(cy-d, cx+dd) ||
            get(cy+d, cx-dd) ||
            get(cy-d, cx-dd);
            D(cerr << "- circle,d=" << d << ",dd=" << dd << ":" << covered << endl);
          }
        }
        
        D(cerr << "- overall:" << covered << endl);
        result->at(cy, cx) = covered;
      }
      
      if (x - twoR >= 0) {
        D(cerr << "area:" << areaCount << " -> ");
        areaCount -= columnCounts[x-twoR];
        D(cerr << areaCount << endl);
        if (y - twoR >= 0) {
          D(cerr << "col[" << x-twoR << "]:" << columnCounts[x-twoR] << " -> ");
          if (at(y-twoR, x-twoR)) {
            columnCounts[x-twoR]--;
          }
          D(cerr << columnCounts[x-twoR] << endl);
        }
      }
    }
    if (y - twoR >= 0) {
      // update the column counts for the remaining columns
      for (int x = width_ - r; x < width_; x++) {
        D(cerr << "col[" << x << "]:" << columnCounts[x] << " -> ");
        if (at(y - twoR, x)) {
          columnCounts[x]--;
        }
        D(cerr << columnCounts[x] << endl);
      }
    }
  }
  return result;
}

inline shared_ptr<Bitmap> Bitmap::inset(int r, int threads) const {
  return distanceTransform(false, threads)->gt(r*r, threads);
}

inline shared_ptr<Bitmap> Bitmap::outset(int r, int threads) const {
  return distanceTransform(true, threads)->le(r*r, threads);
}

inline shared_ptr<Bitmap> Bitmap::close(int r, int threads) const {
  return outset(r, threads)->inset(r, threads);
}

inline shared_ptr<Bitmap> Bitmap::open(int r, int threads) const {
  return inset(r, threads)->outset(r, threads);
}

inline shared_ptr<Bitmap> Bitmap::inset(int r, Workers &workers) const {
  return distanceTransform(false, workers)->gt(r*r, workers);
}

inline shared_ptr<Bitmap> Bitmap::outset(int r, Workers &workers) const {
  return distanceTransform(true, workers)->le(r*r, workers);
}

inline shared_ptr<Bitmap> Bitmap::close(int r, Workers &workers) const {
  return outset(r, workers)->inset(r, workers);
}

inline shared_ptr<Bitmap> Bitmap::open(int r, Workers &workers) const {
  return inset(r, workers)->outset(r, workers);
}

inline shared_ptr<Bitmap> Bitmap::close_old(const Circle &c) const {
  return outset_old(c)->inset_old(c);
}

inline shared_ptr<Bitmap> Bitmap::close_old(int r) const {
  if (r <= 0) {
    return make_shared<Bitmap>(*this);
  }
  Circle c(r);
  return outset_old(c)->inset_old(c);
}

inline shared_ptr<Bitmap> Bitmap::open_old(const Circle &c) const {
  return inset_old(c)->outset_old(c);
}

inline shared_ptr<Bitmap> Bitmap::open_old(int r) const {
  if (r <= 0) {
    return make_shared<Bitmap>(*this);
  }
  Circle c(r);
  return inset_old(c)->outset_old(c);
}


// one iteration of thinning using the algorith of Ng, Zhou and Quek - implementation 2
inline int Bitmap::thin(Bitmap &flags, bool ry, bool rx) {
  int count = 0;
  flags.clear();
  int y0 = ry ? height_ - 1 : 0;
  int dy = ry ? -1 : 1;
  int y1 = ry ? -1 : height_;
  int x0 = rx ? width_ - 1 : 0;
  int dx = rx ? -1 : 1;
  int x1 = rx ? -1 : width_;
  int corner = corners[(ry ? 2 : 0) + (rx ? 1 : 0)];
  
  for (int y = y0; y != y1; y += dy) {
    for (int x = x0; x != x1; x += dx) {
      Point p(x, y);
      D(cerr << "(" << y << "," << x << "): ");
      if (at(y, x)) {
#if DEBUG
        cerr << endl;
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            cerr << get(y + dy, x + dx);
          }
          cerr << " - ";
          for (int dx = -1; dx <= 1; dx++) {
            cerr << flags.get(y + dy, x + dx);
          }
          cerr << " -> ";
          for (int dx = -1; dx <= 1; dx++) {
            cerr << (get(y + dy, x + dx) && !flags.get(y + dy, x + dx));
          }
          cerr << endl;
        }
#endif
        // calculate the previous & current neighbourhoods,
        // and the number of zero-to-one transitions.
        int prevNeighbours = neighbours(p);
        if (prevNeighbours == corner) {
          D(cerr << "corner, skipping" << endl);
          continue;
        }
        
        int flagNeighbours = flags.neighbours(p);
        int currentNeighbours = prevNeighbours & ~flagNeighbours;
        
#if DEBUG
        for (int i = 0; i < 8; i++) {
          cerr << (((prevNeighbours & (1 << i)) != 0) ? '*' : '.');
        }
        cerr << " - ";
        for (int i = 0; i < 8; i++) {
          cerr << (((flagNeighbours & (1 << i)) != 0) ? '*' : '.');
        }
        cerr << " -> ";
        for (int i = 0; i < 8; i++) {
          cerr << (((currentNeighbours & (1 << i)) != 0) ? '*' : '.');
        }
#endif
        
        int pn = neighbourCounts[prevNeighbours];
        int cn = neighbourCounts[currentNeighbours];
        int trans = transitions[currentNeighbours];
        
        D(cerr << ", pn=" << pn << ", cn=" << cn << ", trans=" << trans << " : ");
        if (pn < 8 && 1 < cn && cn < 6) {
          if (trans == 1) {
            flags.at(y, x) = true;
            count++;
            D(cerr << "trans == 1 -> remove");
          } else if (smoothingPatterns[currentNeighbours]) {
            flags.at(y, x) = true;
            count++;
            D(cerr << "smoothingPattern = 1 -> remove");
          } else {
            D(cerr << "keep");
          }
        } else {
          D(cerr << "keep");
        }
        D(cerr << endl << flush);
      } else {
        D(cerr << "pixel already unset" << endl << flush);
      }
    }
  }
  return count;
}

// one iteration of thinning using the algorith of Ng, Zhou and Quek
inline int Bitmap::thinOld(Bitmap &flags) {
  int count = 0;
  flags.clear();
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      D(cerr << "(" << y << "," << x << "): ");
      if (at(y, x)) {
#if DEBUG
        cerr << endl;
        for (int dy = -1; dy <= 1; dy++) {
          for (int dx = -1; dx <= 1; dx++) {
            cerr << get(y + dy, x + dx);
          }
          cerr << " - ";
          for (int dx = -1; dx <= 1; dx++) {
            cerr << flags.get(y + dy, x + dx);
          }
          cerr << " -> ";
          for (int dx = -1; dx <= 1; dx++) {
            cerr << (get(y + dy, x + dx) && !flags.get(y + dy, x + dx));
          }
          cerr << endl;
        }
#endif
        // calculate the previous & current neighbourhoods,
        // and the number of zero-to-one transitions.
        int pn = 0, cn = 0, trans = 0, pTrans = 0;
        uint8_t neighbours = 0;
        uint8_t consecutiveWhite = 0;
        uint8_t maxConsecutiveWhite = 0;
        for (int r = 0; r < 8; r++) {
          bool imagePixel = get(y + yOffsets[r], x + xOffsets[r]);
          bool flagPixel = flags.get(y + yOffsets[r], x + xOffsets[r]);
          neighbours <<= 1;
          
          if (imagePixel) {
            if (consecutiveWhite > maxConsecutiveWhite) {
              maxConsecutiveWhite = consecutiveWhite;
            }
            consecutiveWhite = 0;
            pn++;
            if (!flagPixel) {
              neighbours++;
              cn++;
            }
          } else {
            consecutiveWhite++;
          }
          
          int nextR = (r+1) % 8;
          bool nextImagePixel = get(y + yOffsets[nextR], x + xOffsets[nextR]);
          if (imagePixel && !nextImagePixel) {
            pTrans++;
          }
          bool nextFlagPixel = flags.get(y + yOffsets[nextR], x + xOffsets[nextR]);
          if ((imagePixel && !flagPixel) && !(nextImagePixel && !nextFlagPixel)) {
            trans++;
          }
        }
        for (int r = 0; r < 5; r++) {
          if (get(y + yOffsets[r], x + xOffsets[r])) {
            if (consecutiveWhite > maxConsecutiveWhite) {
              maxConsecutiveWhite = consecutiveWhite;
            }
            consecutiveWhite = 0;
          } else {
            consecutiveWhite++;
          }
        }
        if (consecutiveWhite > maxConsecutiveWhite) {
          maxConsecutiveWhite = consecutiveWhite;
        }
        
#if DEBUG
        cerr << "neighbours = ";
        for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
          cerr << ((neighbours & mask) != 0 ? '1' : '0');
        }
        cerr << ", pn=" << pn << ", cn=" << cn << ", trans=" << trans << ", maxConW=" << (int)maxConsecutiveWhite << " => ";
#endif
        if (pn < 8 && 1 < cn && cn < 6) {
          if (pTrans == 1 && maxConsecutiveWhite > 4) {
            flags.at(y, x) = true;
            count++;
            D(cerr << "remove");
          } else if (trans == 1 || oldThinningPatterns[neighbours]) {
            flags.at(y, x) = true;
            count++;
            D(cerr << "remove");
#if DEBUG
          } else {
            cerr << "keep";
#endif
          }
#if DEBUG
        } else {
          cerr << "keep";
#endif
        }
#if DEBUG
        cerr << endl;
      } else {
        cerr << "pixel already unset" << endl << flush;
#endif
      }
    }
  }
  return count;
}

inline int Bitmap::thin() {
  Bitmap flags(width_, height_);
  int changed;
  int iterations = 0;
  bool rx = false, ry = true;
#if DEBUG_THIN
  char filename[1024];
  sprintf(filename, "/tmp/thin_%03d.png", iterations);
  writePng(filename);
#endif
  //    while ((changed = thinOld(flags)) > 0) {
  while ((changed = thin(flags, ry, rx)) > 0) {
    (*this) -= flags;
    iterations++;
#if DEBUG_THIN
    sprintf(filename, "/tmp/thin_%03d.png", iterations);
    writePng(filename);
#endif
    rx = !rx;
    if (!rx) ry = !ry;
  }
  return iterations;
}

inline void Bitmap::prune() {
  list<Point> pruned;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      if (at(y, x)) {
        int neighbours = 0;
        for (int dir = 0; dir < DIRECTIONS && neighbours < 2; dir++) {
          if (get(y + yOffsets[dir], x + xOffsets[dir])) {
            neighbours++;
          }
        }
        if (neighbours < 2) {
          pruned.push_back(Point(x, y));
        }
      }
    }
  }
  for (list<Point>::iterator i = pruned.begin(); i != pruned.end(); ++i) {
    at(*i) = false;
  }
}

inline void Bitmap::prune(int n) {
  for (int i = 0; i < n; i++) {
    prune();
  }
}

inline void Bitmap::clearThinConnected() {
  for (int y = 0; y < height(); ++y) {
    for (int x = 0; x < width(); ++x) {
      Point p(x, y);
      if (at(p) && thinConnected[neighbours(p)]) {
        set(p, false);
      }
    }
  }
}

struct Bitmap::IsMarkedForReconstruction {
  const Bitmap &result_;
  const Bitmap &reference_;
  IsMarkedForReconstruction(const Bitmap &result, const Bitmap &reference)
  : result_(result), reference_(reference) { }
  bool operator()(int y, int x) {
    return result_.get(y, x) || !reference_.get(y, x);
  }
};

inline shared_ptr<Bitmap> Bitmap::reconstruct(const Bitmap &reference) {
  shared_ptr<Bitmap> result = make(width_, height_, true);
  
  Set setResult = result->set(true);
  IsMarkedForReconstruction isMarked(*result, reference);
  
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      if (at(y, x) && !result->at(y, x)) {
        D(cerr << "*** flood-filling from " << Point(x, y) << endl);
        Filling::fill8(y, x, height_, width_, isMarked, setResult);
      }
    }
  }
  return result;
}

inline shared_ptr<Bitmap> Bitmap::reconstruct(const shared_ptr<Bitmap> &reference) {
  return reconstruct(*reference);
}

inline shared_ptr<Bitmap> Bitmap::operator+(const Bitmap &other) const {
  int w = std::max(width_, other.width_);
  int h = std::max(height_, other.height_);
  shared_ptr<Bitmap> result = make(w, h, false);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      result->at(y, x) = get(y, x) || other.get(y, x);
    }
  }
  return result;
}

inline Bitmap &Bitmap::operator+=(const Bitmap &other) {
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      at(y, x) = at(y, x) || other.get(y, x);
    }
  }
  return *this;
}

inline shared_ptr<Bitmap> Bitmap::operator-(const Bitmap &other) const {
  int w = std::max(width_, other.width_);
  int h = std::max(height_, other.height_);
  shared_ptr<Bitmap> result = make(w, h, false);
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      result->at(y, x) = get(y, x) && !other.get(y, x);
    }
  }
  return result;
}

inline Bitmap &Bitmap::operator-=(const Bitmap &other) {
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      at(y, x) = at(y, x) && !other.get(y, x);
    }
  }
  return *this;
}

// boundary scanning
inline Direction Bitmap::nextDir(const Point &p, Direction dir, Turn delta) {
  for (Direction d = turn(dir, delta);
       d != dir;
       d = turn(d, delta)) {
    if (get(p.neighbour(d))) {
      return d;
    }
  }
  if (get(p.neighbour(dir))) {
    return dir;
  }
  return DIRECTIONS;
}

inline Direction Bitmap::nextDirCW(const Point &p, Direction dir) {
  return nextDir(p, dir, CW);
}

inline Direction Bitmap::nextDirCCW(const Point &p, Direction dir) {
  return nextDir(p, dir, CCW);
}

inline void Bitmap::scanBoundary(Image<int> &marks, int mark, Boundary &boundary, const Point &start, Direction from, bool connect) {
  Direction preDir = nextDirCCW(start, from);
  Point preStart(start.neighbour(preDir));
  
  // we arrived here from |from|, outside; so that's a good starting direction, as well
  // as a good direction for the initial 'preceding' point, since we know that will
  // never accidentally match any other useful preceding point.
  Point current(start);
  Point preceding(current.neighbour(from));
  Point prepreceding(preceding);
  Direction dir = opposite(from);
  while (!(current == start && preceding == preStart)) {
    if (marks.get(current) == 0) {
      // we're starting at an unmarked pixel
      Chain &chain = boundary.addChain();
      
      // if our preceding pixel is part of the same boundary, add it, to make sure
      // everything is connected
      if (connect && marks.at(preceding) == mark) {
        chain.addPoint(preceding);
      }
      
      // collect all the unmarked boundary pixels until we encounter the end, or an
      // already-marked boundary pixel
      while (!(current == start && preceding == preStart)) {
        if (marks.get(current) == 0) {
          chain.addPoint(current);
          marks.at(current) = mark;
          dir = nextDirCW(current, opposite(dir));
          prepreceding = preceding;
          preceding = current;
          current = current.neighbour(dir);
        } else {
          if (connect && marks.at(current) == mark && current != prepreceding) {
            chain.addPoint(current);
          }
          break;
        }
      }
    }
    
    // skip past any already-marked boundary pixels
    while (!(current == start && preceding == preStart) && marks.at(current) != 0) {
      dir = nextDirCW(current, opposite(dir));
      preceding = current;
      current = current.neighbour(dir);
    }
  }
  boundary.close(connect);
}

inline void Bitmap::scanBoundaries(vector<Boundary> &result, Image<int> &marks, bool connect) {
  marks.clear();
  for (int y = 0; y < height(); y++) {
    int inside = 0;
    for (int x = 0; x < width(); x++) {
      Point p(x, y);
      if (at(p)) {
        int current = marks.at(p);
        if (current == 0) {
          // found an unmarked set pixel; check it for unset neighbours N,S,E,W
          for (Direction from = N; from != DIRECTIONS; from = turnOrStop(from, 2)) {
            if (!get(p.neighbour(from))) {
              // found an exposed, unmarked edge; scan it!
              result.push_back(Boundary());
              Boundary &boundary = result.back();
              int mark = static_cast<int>(result.size());
              if (inside == 0) {
                // check for any neighbouring marked pixels, from which this is a child branch
                for (Direction d = NW;
                     inside == 0 && d != DIRECTIONS;
                     d = turnOrStop(d, 1)) {
                  inside = marks.get(p.neighbour(d));
                }
              }
              scanBoundary(marks, mark, boundary, p, from, connect);
              if (inside != 0) {
                Boundary &parent = result[inside - 1];
                parent.addChild(&boundary);
                boundary.parent() = &parent;
              }
              // and since we've now scanned this pixel, we break out of the loop.
              break;
            }
          }
        } else {
          // found a marked pixel (must be a boundary pixel since only those are marked)
          inside = current;
        }
      } else {
        // outside; note this.
        inside = 0;
      }
    }
  }
}

inline vector<Boundary> Bitmap::scanBoundaries(bool connect) {
  vector<Boundary> result;
  Image<int> marks(width(), height());
  
  scanBoundaries(result, marks, connect);
  
  return result;
}

inline int Bitmap::neighbours(const Point &p) const {
  int n = 0;
  int mask = 1;
  for (int i = 0; i < DIRECTIONS; i++) {
    if (get(p.neighbour(i))) n |= mask;
    mask <<= 1;
  }
  return n;
}

inline int Bitmap::countNeighbours(const Point &p) const {
  int n = 0;
  for (int i = 0; i < DIRECTIONS; i++) {
    if (get(p.neighbour(i))) n++;
  }
  return n;
}

template<typename CanRetract>
inline void Bitmap::hatch(Chains &chains, double angle, double period, double phase, CanRetract &canRetract) {
  Hatcher<Get> hatcher(Get(*this));
  Hatching::hatch(width(), height(), angle, period, phase, hatcher);
  hatcher.getChains(chains, period, canRetract);
}

template<typename CanRetract>
inline Chains Bitmap::hatch(double angle, double period, double phase, CanRetract &canRetract) {
  Chains chains;
  hatch(chains, angle, period, phase, canRetract);
  return chains;
}

inline void Bitmap::retract(const Bitmap &reference, const Circle &circle) {
  // find all extreme points - those that have exactly a single neighbour
  std::set<Point> setA, setB;
  std::set<Point> *a = &setA, *b = &setB;
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      Point p(x, y);
      if (at(p) && countNeighbours(p) == 1) {
        a->insert(p);
      }
    }
  }
  
  int didRetract = 0;
  int iterations = 0;
  do {
    D(cerr << " Bitmap::retract: " << a->size() << " extreme points to retract in iteration " << iterations << endl);
    ++iterations;
    didRetract = 0;
    for (std::set<Point>::iterator i = a->begin(); i != a->end(); ++i) {
      int n = neighbours(*i);
      bool canRetract = (n == 0) || isSingleConnected(n);
      if (canRetract) {
        const list<Point> &delta = circle.getDelta(n);
        for (list<Point>::const_iterator j = delta.begin();
             canRetract && j != delta.end();
             ++j) {
          canRetract = !reference.get(*i + *j);
        }
      }
      if (canRetract) {
        didRetract++;
        set(*i, false);
        for (int d = NW; d < DIRECTIONS; d++) {
          Point q(i->neighbour(d));
          if (get(q)) {
            int qn = neighbours(q);
            if (qn == 0 || isSingleConnected(qn)) {
              b->insert(q);
            }
          }
        }
      } else {
        b->insert(*i);
      }
    }
    D(cerr << " Bitmap::retract: retracted " << didRetract << " points" << endl);
    std::set<Point> *tmp = a;
    a = b;
    b = tmp;
    b->clear();
  } while (didRetract > 0);
  D(cerr << "Bitmap::retract: done after " << iterations << " iterations" << endl);
}

inline void Bitmap::retract(const shared_ptr<Bitmap> &reference, const Circle &circle) {
  retract(*reference, circle);
}

inline shared_ptr<Bitmap> Bitmap::adjacent(const Bitmap &other) {
  shared_ptr<Bitmap> result = make(width_, height_);
  for (int y = 0; y < height_; y++) {
    for (int x = 0; x < width_; x++) {
      result->at(y, x) = at(y, x) &&
      !(get(y, x-1) &&
        get(y, x+1) &&
        get(y-1, x) &&
        get(y+1, x)) &&
      (other.get(y, x-1) ||
       other.get(y, x+1) ||
       other.get(y-1, x) ||
       other.get(y+1, x));
    }
  }
  return result;
}

inline shared_ptr<Bitmap> Bitmap::adjacent(const shared_ptr<Bitmap> &other) {
  return adjacent(*other);
}

inline
ostream &operator<<(ostream &out, const Bitmap &i) {
  int y, x;
  for (y = 0; y < i.height(); y++) {
    for (x = 0; x < i.width(); x++) {
      out << (i.at(y,x) ? '#' : '.');
    }
    out << endl;
  }
  out << flush;
  return out;
}

#if 0
{
#endif
}  // namespace Images


#endif  // __Bitmap_Impl_h__
