/*
 *  GreyImage.h
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __GreyImage_h__
#define __GreyImage_h__

#include "Primitives.h"
#include "Image.h"
#include "Bitmap.h"
#include "Chain.h"
#include "Workers.h"

extern "C" {
#include <png.h>
}

#include <cmath>
#include <cstdlib>

namespace Images {
#if 0
}
#endif

using namespace Neighbourhood;

template<typename C> class GreyImage;

namespace GreyImages {
#if 0
}
#endif

template<typename T> extern bool writePng(const GreyImage<T> &image, FILE *fp);
template<typename T, bool shiftDown> extern Ref< GreyImage<T> > readPng(FILE *fp, T background);

#if 0
{
#endif
}  // namespace GreyImages

template<typename C> class GreyImage : public Image<C> {
private:
  template <typename D> friend ostream &operator<<(ostream &out, const GreyImage<D> &i);

public:
  typedef GreyImage<C> Self;
  typedef Ref<Self> GreyImageRef;
  typedef Image<C> Super;
  typedef typename Super::Row Row;
  typedef typename Super::iterator iterator;
  typedef typename Super::const_iterator const_iterator;
  using Super::width_;
  using Super::height_;
  using Super::data_;
  using Super::at;

  GreyImage(int width = 0, int height = 0, bool doClear = true);
  GreyImage(const GreyImage &other);
  GreyImage(const GreyImage *other);

  static GreyImageRef make(int width = 0, int height = 0, bool doClear = true);

  istream &readData(istream &in);

  ostream &writeData(ostream &out);

  bool writePng(FILE *f);
  bool writePng(const char * const filename);
  bool writePng(const string &filename);

  static C defaultBackground();

  static GreyImageRef readPng(FILE *fp);
  static GreyImageRef readPng(const char * const filename);
  static GreyImageRef readPng(const string &filename);

  static GreyImageRef readPngHeightmap(FILE *fp);
  static GreyImageRef readPngHeightmap(const char * const filename);
  static GreyImageRef readPngHeightmap(const string &filename);

  template<typename Op> Ref<Bitmap> where(Op &op, C value) const;

  Ref<Bitmap> coverage() const;
  Ref<Bitmap> coverage(int threads) const;
  Ref<Bitmap> coverage(Workers &workers) const;

#define DECLARE_OP(name)                                 \
  Ref<Bitmap> name(C value) const;			 \
  Ref<Bitmap> name(C value, int threads) const;		 \
  Ref<Bitmap> name(C value, Workers &workers) const

  DECLARE_OP(ge);
  DECLARE_OP(gt);
  DECLARE_OP(le);
  DECLARE_OP(lt);
  DECLARE_OP(eq);
  DECLARE_OP(ne);

#undef DECLARE_OP

  Ref<Bitmap> distribute(void (*func)(void *), C value, Workers &workers) const;

  C min() const;
  C min(Workers &workers) const;

  C max() const;
  C max(Workers &workers) const;

  struct Range {
    typename GreyImage<C>::const_iterator begin;
    typename GreyImage<C>::const_iterator end;
    Bitmap::iterator dst;
    C value;
  };

  struct ReductionRange {
    typename GreyImage<C>::const_iterator begin;
    typename GreyImage<C>::const_iterator end;
    C value;
  };

  static void ge(void *); // takes a Range
  static void gt(void *); // takes a Range
  static void le(void *); // takes a Range
  static void lt(void *); // takes a Range
  static void eq(void *); // takes a Range
  static void ne(void *); // takes a Range
  static void min(void *);  // takes a ReductionRange
  static void max(void *);  // takes a ReductionRange

  static double frand();

  double sumOfValues(const Point &p0, const list<Point> &points) const;

  template <typename T>
  double dither(Chains &results, const Chain &chain, const Circle &circle, const T &reference, double error);

  template <typename T>
  void dither(Chains &results, const Chains &chains, const Circle &circle, const T &reference);

  void dither(Chains &results, const Chains &chains, const Circle &circle);
};

#if 0
{
#endif
}  // namespace Images

#endif // __GreyImage_h__
