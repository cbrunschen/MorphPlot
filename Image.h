/*
 *  Image.h
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Image_h__
#define __Image_h__

#include "Primitives.h"
#include "Chain.h"
#include "Counted.h"
#include "Line.h"
#include "Circle_Impl.h"

namespace Images {
#if 0
}
#endif

using namespace Chaining;

template <typename Pixel> class Image : public Counted {
protected:
  int width_;
  int height_;
  // resolutions, in x and y directions, in pixels per meter
  int xRes_;
  int yRes_;
  int resUnit_;
  Pixel *data_;
  
  template <typename P> friend ostream &operator<<(ostream &out, const Image<P> &i);
  
public:  
  typedef Image<Pixel> Self;
  typedef Ref< Image<Pixel> > ImageRef;

  class Row {
  protected:
    Pixel *data_;
  public:
    Row(Pixel *data) : data_(data) { }
    Pixel &operator[](int x) {
      return data_[x];
    }
    const Pixel &operator[](int x) const {
      return data_[x];
    }
    Pixel * const data() { return data_; }
    const Pixel * const data() const { return data_; }
  };
  
  Image(int width = 0, int height = 0, bool doClear = true) 
  : width_(width), height_(height), xRes_(0), yRes_(0), resUnit_(0), data_(new Pixel[width * height]) {
    if (doClear) {
      clear();
    }
  }
  virtual ~Image() {
    delete data_;
  }
  
  Image(const Image &other) : width_(other.width_), height_(other.height_),
  data_(new Pixel[other.width_ * other.height_])
  {    
    bcopy(other.data_, data_, width_ * height_ * sizeof(Pixel));
  }

  Image(const Image *other) : width_(other->width_), height_(other->height_),
  data_(new Pixel[other->width_ * other->height_])
  {
    bcopy(other->data_, data_, width_ * height_ * sizeof(Pixel));
  }
  
  static ImageRef make(int width = 0, int height = 0, bool doClear = true) {
    Image *image = new Image(width, height, doClear);
    return ImageRef(image);
  }
  
  void clear() {
    bzero(data_, width_ * height_ * sizeof(Pixel));
  }
  
  Image &operator=(const Image &other) {
    delete data_;
    width_ = other.width_;
    height_ = other.height_;
    data_ = new Pixel[width_ * height_];
    bcopy(other.data_, data_, width_ * height_ * sizeof(Pixel));
    return *this;
  }

  Image &operator=(const Image *other) {
    *this = *other;
  }
  
  int &width() { return width_; }
  const int width() const { return width_; }
  int &height() { return height_; }
  const int height() const { return height_; }
  const int length() const { return width_ * height_; }
  
  int &xRes() { return xRes_; }
  const int xRes() const { return xRes_; }
  void setXRes(int xRes) { xRes_ = xRes; }
  int &yRes() { return yRes_; }
  const int yRes() const { return yRes_; }
  void setYRes(int yRes) { yRes_ = yRes; }
  int &resUnit() { return resUnit_; }
  const int resUnit() const { return resUnit_; }
  void setResUnit(int resUnit) { resUnit_ = resUnit; }
  
  template<typename T> void copyRes(const T &other) {
    xRes_ = other.xRes();
    yRes_ = other.yRes();
    resUnit_ = other.resUnit();
  }

  template<typename T> void copyRes(const T *other) {
    copyRes(*other);
  }
  
  template<typename T> void copyRes(Ref< T > other) {
    copyRes(*other);
  }
    
  Row operator[](int y) {
    return Row(&data_[width_ * y]);
  }
  const Row operator[](int y) const {
    return Row(&data_[width_ * y]);
  }
  
  Pixel &at(int y, int x) {
#if DEBUG_AT
    if (y < 0 || height_ <= y || x < 0 || width_ <= x) {
      cerr << "!!: " << y << "," << x << " not in " << height_ << "x" << width_ << endl;
    }
#endif  // DEBUG_AT
    return data_[width_ * y + x];
  }
  const Pixel &at(int y, int x) const {
#if DEBUG_AT
    if (y < 0 || height_ <= y || x < 0 || width_ <= x) {
      cerr << "!!: " << y << "," << x << " not in " << height_ << "x" << width_ << endl;
    }
#endif  // DEBUG_AT
    return data_[width_ * y + x];
  }
  const Pixel get(int y, int x) {
    if (0 <= y && y < height_ && 0 <= x && x < width_) {
      return data_[width_ * y + x];
    } else {
      return 0;
    }
  }
  const Pixel get(int y, int x) const {
    if (0 <= y && y < height_ && 0 <= x && x < width_) {
      return data_[width_ * y + x];
    } else {
      return 0;
    }
  }
  const Pixel getVal(int y, int x) const {
    if (0 <= y && y < height_ && 0 <= x && x < width_) {
      return data_[width_ * y + x];
    } else {
      return 0;
    }
  }

  Pixel &at(const Point &p) {
    return at(p.y(), p.x());
  }
  const Pixel &at(const Point &p) const {
    return at(p.y(), p.x());
  }
  const Pixel get(const Point &p) {
    return get(p.y(), p.x());
  }
  const Pixel get(const Point &p) const {
    return get(p.y(), p.x());
  }
  
  typedef Pixel *iterator;
  typedef const Pixel *const_iterator;

  iterator begin() {
    return data_;
  }
  const_iterator begin() const {
    return data_;
  }
  iterator end() {
    return &data_[width_ * height_];
  }
  const_iterator end() const {
    return &data_[width_ * height_];
  }
  
  iterator iter_at(int row) {
    return &data_[row * width_];
  }
  const_iterator iter_at(int row) const {
    return &data_[row * width_];
  }

  void set(int y, int x, const Pixel &value) {
    if (0 <= x && x < width_ && 0 <= y && y < height_) {
      at(y, x) = value;
    }
  }

  void set(const Point &p, const Pixel &value) {
    set(p.y(), p.x(), value);
  }
  
  struct Get {
    const Image &image_;
    Get(const Image &image) : image_(image)  { }
    Pixel operator()(const Point &p) const { return image_.get(p); }
    Pixel operator()(const int y, const int x) const { return image_.get(y, x); }
  };
  
  struct At {
    const Image &image_;
    At(const Image &image) : image_(image)  { }
    Pixel operator()(const Point &p) const { return image_.at(p); }
    Pixel operator()(const int y, const int x) const { return image_.at(y, x); }
  };
  
  struct Set {
    Image &image_;
    const Pixel &value_;
    Set(Image &image, const Pixel &value) : image_(image), value_(value) { }
    void operator()(const Point &p) const { image_.set(p, value_); }
    void operator()(const int y, const int x) const { return image_.set(y, x, value_); }
  };
  
  const Get get() const {
    return Get(*this);
  }
  
  const At at() const {
    return At(*this);
  }
  
  const Set set(const Pixel &value) {
    return Set(*this, value);
  }
  
  // XY variants
  struct GetXY {
    const Image &image_;
    GetXY(const Image &image) : image_(image)  { }
    Pixel operator()(const Point &p) const { return image_.get(p); }
    Pixel operator()(const int x, const int y) const { return image_.get(y, x); }
  };
  
  struct AtXY {
    const Image &image_;
    AtXY(const Image &image) : image_(image)  { }
    Pixel operator()(const Point &p) const { return image_.at(p); }
    Pixel operator()(const int x, const int y) const { return image_.at(y, x); }
  };
  
  struct SetXY {
    Image &image_;
    const Pixel &value_;
    SetXY(Image &image, const Pixel &value) : image_(image), value_(value) { }
    void operator()(const Point &p) const { image_.set(p, value_); }
    void operator()(const int x, const int y) const { return image_.set(y, x, value_); }
  };
  
  const GetXY getXY() const {
    return GetXY(*this);
  }
  
  const AtXY atXY() const {
    return AtXY(*this);
  }
  
  const SetXY setXY(const Pixel &value) {
    return SetXY(*this, value);
  }
  
  int set(const Chain &chain, const Pixel &value) {
    int n = 0;
    for (Chain::const_iterator i = chain.begin(); i != chain.end(); ++i) {
      at(*i) = value;
      ++n;
    }
    return n;
  }
  
  int set(const Chains &chains, const Pixel &value) {
    int n = 0;
    for (Chains::const_iterator i = chains.begin(); i != chains.end(); ++i) {
      n += set(*i, value);
    }
    return n;
  }
  
  void set(const Chains &chains, const Pixel &value, int r, int const extents[]) {
    for (Chains::const_iterator i = chains.begin(); i != chains.end(); ++i) {
      for (Chain::const_iterator c = i->begin(); c != i->end(); ++c) {
        int x = c->x();
        int y = c->y();
        for (int dx = 0; dx <= r; ++dx) {
          set(y, x + dx, value);
          set(y, x - dx, value);
          set(y + dx, x, value);
          set(y - dx, x, value);
        }
        for (int dy = 1; dy <= r; ++dy) {
          int e = extents[dy - 1];
          for (int dx = 1; dx <= e; ++dx) {
            set(y + dy, x + dx, value);
            set(y + dy, x - dx, value);
            set(y - dy, x + dx, value);
            set(y - dy, x - dx, value);
          }
        }
      }
    }
  }
    
  void set(const Chains &chains, const Pixel &value, int r) {
    int extents[r];
    Circle::makeExtents(r, extents);
    set(chains, value, r, extents);
  }
  
  void set(const Chains &chains, const Pixel &value, const Circle &circle) {
    set(chains, value, circle.r(), circle.extents());
  }
  
  template <typename T> void set(const T &t, const Pixel &value) {
    for (typename T::const_iterator i = t.begin(); i != t.end(); ++i) {
      set(*i, value);
    }
  }
  
  template <typename I> void set(const Point &p, const Pixel &value, I i, const I &iEnd) {
    for (; i != iEnd; ++i) {
      set(p + *i, value);
    }
  }
  
  template <typename T> void set(const Point &p, const Pixel &value, const T &t) {
    set(p, value, t.points());
  }
  
  /*
  template<typename I, typename DX, typename DXY>
  void draw(const Chains &chains, const Pixel &value,
            const I &initial, const DX &deltaX, const DXY &deltaXY) {
    Set set(*this, value);
    for (Chains::const_iterator i = chains.begin(); i != chains.end(); ++i) {
      Chain::const_iterator c = i->begin();
      Point p = *c;
      Matrix matrix(Matrix::identity());
      drawAll(set, p, initial, matrix);
      for (++c; c != i->end(); ++c) {
        matrix = Matrix::translate(c->x(), c->y());
        int dx = c->x() - p.x();
        int dy = c->y() - p.y();
        if (dx < 0) {
          matrix = matrix.concat(Matrix::flipX());
        } else if (dx == 0) {
          matrix = matrix.concat(Matrix::rotateLeft());
        }
        if (dy < 0) {
          matrix = matrix.concat(Matrix::flipY());
        }
        
        p = *c;
        
        if (dx != 0 && dy != 0) {
          drawAll(set, Point(0, 0), deltaXY, matrix);          
        } else {
          drawAll(set, Point(0, 0), deltaX, matrix);          
        }
      }
    }
  }

  void draw(const Chains &chains, const Pixel &value, int r) {
    int extents[r];
    Circle::makeExtents(r, extents);
    list<Point> deltaX, deltaXY, initial;
    Circle::makePoints(r, extents, back_inserter(initial));
    Circle::makeHorizontalDelta(r, extents, back_inserter(deltaX));
    Circle::makeDiagonalDelta(r, extents, back_inserter(deltaXY));

    draw(chains, value, initial, deltaX, deltaXY);
  }
   */
  
  void draw(const Chains &chains, const Pixel &value, const Circle &circle) {
    Set set(*this, value);
    for (Chains::const_iterator i = chains.begin(); i != chains.end(); ++i) {
      circle.setAll(i->begin(), i->end(), set);
    }
  }
    
  void setAll(const Pixel &value) {
    for (int y = 0; y < height_; ++y) {
      for (int x = 0; x < width_; x++) {
        at(y, x) = value;
      }
    }
  }
  
  void line(const Point &p0, const Point &p1, const Pixel &value) {
    Primitives::line(p0.x(), p0.y(), p1.x(), p1.y(), setXY(value));
  }

  void line(int x0, int y0, int x1, int y1, const Pixel &value) {
    Primitives::line(x0, y0, x1, y1, setXY(value));
  }

  void line(const Point &p0, const Point &p1, const Pixel &value, const Circle &circle) {
    Primitives::line(p0.x(), p0.y(), p1.x(), p1.y(), setXY(value), circle);
  }
  
  void line(int x0, int y0, int x1, int y1, const Pixel &value, const Circle &circle) {
    Primitives::line(x0, y0, x1, y1, setXY(value), circle);
  }
  
  bool operator==(const Image<Pixel> &other) const {
    if (other.width_ != width_) return false;
    if (other.height_ != height_) return false;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        if (other.at(y, x) != at(y, x)) return false;
      }
    }
    return true;
  }
  
  bool operator!=(const Image<Pixel> &other) const {
    return !(*this == other);
  }
  
  template<typename F, typename R> Ref< Image<R> > apply(F &f) {
    Ref< Image<R> > result = Image<R>::make(width(), height(), false);
    for (int y = 0; y < height(); ++y) {
      for (int x = 0; x < width(); ++x) {
        result.at(y, x) = f(get(y, x));
      }
    }
  }
};

template <typename P> ostream &operator<<(ostream &out, const Image<P> &i) {
  int y, x;
  ios_base::fmtflags flags(out.flags());
  out << hex;
  for (y = 0; y < i.height(); y++) {
    for (x = 0; x < i.width(); x ++) {
      out << i.at(y, x) << " ";
    }
    out << endl;
  }
  out << flush;
  out.flags(flags);
  return out;
}

template<typename I> Ref<I> scaleImage(const I &src, double xScale, double yScale) {
  int w = src.width() * xScale;
  int h = src.height() * yScale;
  Ref<I> result(new I(w, h));
  for (int y = 0; y < h; y++) {
    typename I::Row sourceRow = (src)[y / yScale];
    typename I::Row resultRow = (*result)[y];
    for (int x = 0; x < w; x++) {
      resultRow[x] = sourceRow[x / xScale];
    }
  }
  result->setXRes(src.xRes() * xScale);
  result->setYRes(src.yRes() * yScale);
  result->setResUnit(src.resUnit());
  return result;
}

template<typename I> Ref<I> scaleImage(Ref<I> src, double xScale, double yScale) {
  return scaleImage(*src, xScale, yScale);
}

template<typename I> Ref<I> scaleImage(Ref<I> src, double scale) {
  return scaleImage(src, scale, scale);
}

#if 0
{
#endif
}

#endif // __Image_h__
