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
template<typename T, bool shiftDown> extern shared_ptr< GreyImage<T> > readPng(FILE *fp, T background);

#if 0
{
#endif
}  // namespace GreyImages

template<typename C> class GreyImage : public Image<C> {
private:
  template <typename D> friend ostream &operator<<(ostream &out, const GreyImage<D> &i);

public:
  typedef GreyImage<C> Self;
  typedef shared_ptr<Self> GreyImageRef;
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

  template<typename Op> shared_ptr<Bitmap> where(Op &op, C value) const;

  shared_ptr<Bitmap> coverage() const;
  shared_ptr<Bitmap> coverage(int threads) const;
  shared_ptr<Bitmap> coverage(Workers &workers) const;

  double sumOfValues(const IPoint &p0, const list<IPoint> &points) const;

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
