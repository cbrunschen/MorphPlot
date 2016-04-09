//
//  Image_Impl.h
//  MorphPlot
//
//  Created by Christian Brunschen on 22/02/2014.
//
//

#ifndef __Image_Impl_h__
#define __Image_Impl_h__

#include "Bitmap_Impl.h"

namespace Images {
#if 0
}
#endif

template<typename Pixel>
struct Range {
  typename Image<Pixel>::const_iterator begin;
  typename Image<Pixel>::const_iterator end;
  Bitmap::iterator dst;
  Pixel value;
};

template<typename Pixel>
struct ReductionRange {
  typename Image<Pixel>::const_iterator begin;
  typename Image<Pixel>::const_iterator end;
  Pixel value;
};

template<typename Pixel>
inline shared_ptr<Bitmap>
    Image<Pixel>::distribute(void (*func)(void *), Pixel value, Workers &workers) const {
  shared_ptr<Bitmap> result = make_shared<Bitmap>(width_, height_);
  int n = workers.n();
  Range<Pixel> *ranges = new Range<Pixel>[n];
  for (int i = 0; i < n; i++) {
    ranges[i].begin = this->row_iterator(i * height_ / n);
    ranges[i].end = this->row_iterator((i + 1) * height_ / n);
    ranges[i].dst = result->row_iterator(i * height_ / n);
    ranges[i].value = value;
  }

  workers.perform(func, &ranges[0]);

  delete [] ranges;
  return result;
}

#define IMPLEMENT_OP(op, name)                                                                 \
template<typename Pixel>                                                                       \
inline void Image<Pixel>::name(void *params) {                                                 \
  Range<Pixel> *p = static_cast<Range<Pixel> *>(params);                                       \
  typename Image<Pixel>::const_iterator src = p->begin;                                        \
  typename Image<Pixel>::const_iterator end = p->end;                                          \
  Bitmap::iterator dst = p->dst;                                                               \
  Pixel value = p->value;                                                                      \
                                                                                               \
  while (src != end) {                                                                         \
    *dst++ = *src++ op value;                                                                  \
  }                                                                                            \
}                                                                                              \
                                                                                               \
template<typename Pixel>                                                                       \
inline shared_ptr<Bitmap> Image<Pixel>::name(const Pixel &value) const {                       \
  shared_ptr<Bitmap> result = make_shared<Bitmap>(width_, height_);                            \
  Bitmap::iterator dst = result->begin();                                                      \
  Image<Pixel>::const_iterator src = this->begin();                                            \
  Image<Pixel>::const_iterator end = this->end();                                              \
  while (src != end) {                                                                         \
    *dst++ = (*src++ op value);                                                                \
  }                                                                                            \
  return result;                                                                               \
}                                                                                              \
                                                                                               \
template<typename Pixel>                                                                       \
inline shared_ptr<Bitmap> Image<Pixel>::operator op(const Pixel &value) const {                \
  return name(value);                                                                          \
}                                                                                              \
                                                                                               \
template<typename Pixel>                                                                       \
inline shared_ptr<Bitmap> Image<Pixel>::name(const Pixel &value, Workers &workers) const {     \
  if (workers.n() > 1) {                                                                       \
    return distribute(Image<Pixel>::name, value, workers);                                     \
  } else {                                                                                     \
    return name(value);                                                                        \
  }                                                                                            \
}                                                                                              \
                                                                                               \
template<typename Pixel>                                                                       \
inline shared_ptr<Bitmap> Image<Pixel>::operator op(Workers::Job<Pixel> &job) const {          \
  if (job.workers.n() > 1) {                                                                   \
    return distribute(Image<Pixel>::name, job.value, job.workers);                             \
  } else {                                                                                     \
    return name(job.value);                                                                    \
  }                                                                                            \
}                                                                                              \
                                                                                               \
template<typename Pixel>                                                                       \
inline shared_ptr<Bitmap> Image<Pixel>::name(const Pixel &value, int threads) const {          \
  if (threads > 1) {                                                                           \
    Workers workers(threads);                                                                  \
    return distribute(Image<Pixel>::name, value, workers);                                     \
  } else {                                                                                     \
    return name(value);                                                                        \
  }                                                                                            \
}

IMPLEMENT_OP(<, lt)
IMPLEMENT_OP(<=, le)
IMPLEMENT_OP(>, gt)
IMPLEMENT_OP(>=, ge)
IMPLEMENT_OP(==, eq)
IMPLEMENT_OP(!=, ne)

#undef IMPLEMENT_OP

#define IMPLEMENT_MINMAX(name, operator)                                                      \
template<typename Pixel>                                                                      \
inline void Image<Pixel>::name(void *params) {                                                \
  ReductionRange<Pixel> *p = static_cast<ReductionRange<Pixel> *>(params);                    \
  typename Image<Pixel>::const_iterator src = p->begin;                                       \
  typename Image<Pixel>::const_iterator end = p->end;                                         \
                                                                                              \
  Pixel value = *src++;                                                                       \
  while (src != end) {                                                                        \
    Pixel tmp = *src++;                                                                       \
    if (tmp operator value) value = tmp;                                                      \
  }                                                                                           \
  p->value = value;                                                                           \
}                                                                                             \
                                                                                              \
template<typename Pixel>                                                                      \
inline Pixel Image<Pixel>::name() const {                                                     \
  Image<Pixel>::const_iterator src = this->begin();                                           \
  Image<Pixel>::const_iterator end = this->end();                                             \
  Pixel value = *src++;                                                                       \
  while (src != end) {                                                                        \
    Pixel tmp = *src++;                                                                       \
    if (tmp operator value) value = tmp;                                                      \
  }                                                                                           \
  return value;                                                                               \
}                                                                                             \
                                                                                              \
template<typename Pixel>                                                                      \
inline Pixel Image<Pixel>::name(Workers &workers) const {                                     \
  int n = workers.n();                                                                        \
  if (n > 1) {                                                                                \
    ReductionRange<Pixel> *ranges = new ReductionRange<Pixel>[n];                             \
    for (int i = 0; i < n; i++) {                                                             \
      ranges[i].begin = this->row_iterator(i * height_ / n);                                  \
      ranges[i].end = this->row_iterator((i + 1) * height_ / n);                              \
    }                                                                                         \
                                                                                              \
    workers.perform(Image<Pixel>::name, &ranges[0]);                                          \
                                                                                              \
    Pixel value = ranges[0].value;                                                            \
    for (int i = 1; i < n; i++) {                                                             \
      Pixel tmp = ranges[i].value;                                                            \
      if (tmp operator value) value = tmp;                                                    \
    }                                                                                         \
                                                                                              \
    delete [] ranges;                                                                         \
    return value;                                                                             \
  } else {                                                                                    \
    return name();                                                                            \
  }                                                                                           \
}
																											
IMPLEMENT_MINMAX(min, <)																					
IMPLEMENT_MINMAX(max, >)

#undef IMPLEMENT_MINMAX

template<typename Pixel>
struct ConvolutionRange {
  const Image<Pixel> *src;
  Image<Pixel> *dst;
  int y0;
  int y1;
  
  const Image<Pixel> *ref;
  int xOffset;
  int yOffset;
};

template<typename Pixel> inline Pixel fit_func(Pixel prev, Pixel base, Pixel tool) {
  Pixel candidate = base - tool;
  return (candidate > prev) ? candidate : prev;
}

template<typename Pixel> inline Pixel affected_func(Pixel prev, Pixel base, Pixel tool) {
  Pixel candidate = base + tool;
  return (candidate < prev) ? candidate : prev;
}

#define IMPLEMENT_CONVOLUTION(name, func, initial)                                               \
template<typename Pixel>                                                                         \
inline void Image<Pixel>::name(void *params) {                                                   \
  ConvolutionRange<Pixel> *p = static_cast<ConvolutionRange<Pixel> *>(params);                   \
  const Image<Pixel> *src = p->src;                                                              \
  Image<Pixel> *dst = p->dst;                                                                    \
  const Image<Pixel> *ref = p->ref;                                                              \
                                                                                                 \
  int xOffset = p->xOffset;                                                                      \
  int yOffset = p->yOffset;                                                                      \
                                                                                                 \
  int y0 = p->y0;                                                                                \
  int y1 = p->y1;                                                                                \
  int w = src->width();                                                                          \
                                                                                                 \
  for (int y = y0; y < y1; y++) {                                                                \
    for (int x = 0; x < w; x++) {                                                                \
      Pixel value = (initial);                                                                   \
      for (int ry = 0; ry < ref->height(); ry++) {                                               \
        for (int rx = 0; rx < ref->width(); rx++) {                                              \
          value = func(value, src->at(x + rx - xOffset, y + ry - yOffset), ref->at(rx, ry));     \
        }                                                                                        \
      }                                                                                          \
      dst->set(x, y, value);                                                                     \
    }                                                                                            \
  }                                                                                              \
}                                                                                                \
                                                                                                 \
template<typename Pixel>                                                                         \
inline shared_ptr<Image<Pixel> > Image<Pixel>::name(shared_ptr<Image<Pixel> > ref) const {       \
  shared_ptr<Image<Pixel> > dst = make_shared<Image<Pixel> >(width_, height_, false);            \
                                                                                                 \
  int xOffset = (ref->width() - 1) / 2;                                                          \
  int yOffset = (ref->height() - 1) / 2;                                                         \
                                                                                                 \
  for (int y = 0; y < height(); y++) {                                                           \
    for (int x = 0; x < width(); x++) {                                                          \
      Pixel value = initial;                                                                     \
      for (int ry = 0; ry < ref->height(); ry++) {                                               \
        for (int rx = 0; rx < ref->width(); rx++) {                                              \
          value = func(value, at(x + rx - xOffset, y + ry - yOffset), ref->at(rx, ry));          \
        }                                                                                        \
      }                                                                                          \
      dst->set(x, y, value);                                                                     \
    }                                                                                            \
  }                                                                                              \
                                                                                                 \
  return dst;                                                                                    \
}                                                                                                \
                                                                                                 \
template<typename Pixel>                                                                         \
inline shared_ptr<Image<Pixel> >                                                                 \
    Image<Pixel>::name(shared_ptr<Image<Pixel> > ref, Workers &workers) const {                  \
  int n = workers.n();                                                                           \
  if (n > 1) {                                                                                   \
    shared_ptr<Image<Pixel> > dst = make_shared<Image<Pixel> >(width_, height_, false);          \
    int xOffset = (ref->width() - 1) / 2;                                                        \
    int yOffset = (ref->height() - 1) / 2;                                                       \
                                                                                                 \
    ConvolutionRange<Pixel> *ranges = new ConvolutionRange<Pixel>[n];                            \
                                                                                                 \
    for (int i = 0; i < n; i++) {                                                                \
      ranges[i].src = this;                                                                      \
      ranges[i].dst = dst.get();                                                                 \
                                                                                                 \
      ranges[i].y0 = ((i + 0) * height()) / n;                                                   \
      ranges[i].y1 = ((i + 1) * height()) / n;                                                   \
                                                                                                 \
      ranges[i].ref = ref.get();                                                                 \
      ranges[i].xOffset = xOffset;                                                               \
      ranges[i].yOffset = yOffset;                                                               \
    }                                                                                            \
                                                                                                 \
    workers.perform(name, &ranges[0]);                                                           \
                                                                                                 \
    delete [] ranges;                                                                            \
                                                                                                 \
    return dst;                                                                                  \
  } else {                                                                                       \
    return name(ref);                                                                            \
  }                                                                                              \
}                                                                                                \

IMPLEMENT_CONVOLUTION(fit, fit_func, numeric_limits<Pixel>::min())
IMPLEMENT_CONVOLUTION(affected, affected_func, numeric_limits<Pixel>::min())

#undef IMPLEMENT_CONVOLUTION

#if 0
{
#endif
}

#endif  // __Image_Impl_h__
