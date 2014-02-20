/*
 *  ColorImage.h
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __ColorImage_h__
#define __ColorImage_h__

#include "Image.h"
#include "GreyImage.h"

extern "C" {
#include <png.h>
}

namespace Images {
#if 0
}
#endif

template<typename C> class RGBPixel {
  C data_[3];
public:
  template <typename D> friend ostream &operator<<(ostream &out, const RGBPixel<D> &p);
  template <typename D> struct FractionForComponent {
    D &comp_;
    FractionForComponent(D &comp) : comp_(comp) { }
    operator double() const {
      return Component<D>::fraction(comp_);
    }
    double operator=(const double &v) {
      comp_ = Component<D>::make(v);
      return v;
    }
    double operator+=(const double &v) {
      comp_ = Component<D>::make(Component<D>::fraction(comp_) + v);
      return v;
    }
    double operator-=(const double &v) {
      comp_ = Component<D>::make(Component<D>::fraction(comp_) - v);
      return v;
    }
  };
  RGBPixel(const C &r = 0, const C &g = 0, const C &b = 0) {
    data_[0] = r;
    data_[1] = g;
    data_[2] = b;
  }
  RGBPixel(const double &r, const double &g, const double &b) {
    data_[0] = Component<C>::make(r);
    data_[1] = Component<C>::make(g);
    data_[2] = Component<C>::make(b);
  }
  C *data() { return data_; }
  const C *data() const { return data_; } 
  C &r() { return data_[0]; }
  C &g() { return data_[1]; }
  C &b() { return data_[2]; }
  const C &r() const { return data_[0]; }
  const C &g() const { return data_[1]; }
  const C &b() const { return data_[2]; }
  const double rFraction() const { return Component<C>::fraction(r()); }
  const double gFraction() const { return Component<C>::fraction(g()); }
  const double bFraction() const { return Component<C>::fraction(b()); }
  FractionForComponent<C> rFraction() { return FractionForComponent<C>(r()); }
  FractionForComponent<C> gFraction() { return FractionForComponent<C>(g()); }
  FractionForComponent<C> bFraction() { return FractionForComponent<C>(b()); }
};

template <typename C> class ColorImage;

namespace ColorImages {
  extern bool writePng(const ColorImage<uint8_t> &image, FILE *fp);
  extern shared_ptr< ColorImage<uint8_t> > readPng(FILE *fp);
}

template <typename C> class ColorImage : public Image<RGBPixel<C> > {
private:
  template<typename D> friend ostream &operator<<(ostream &out, const ColorImage<D> &i);
  
  template<typename D> friend bool writePng(const ColorImage<D> &image, FILE *fp);
  
public:
  typedef Images::RGBPixel<C> RGBPixel;
  typedef Image<RGBPixel> Super;
  typedef ColorImage<C> Self;
  typedef typename Super::Row Row;
  typedef shared_ptr<Self> ColorImageRef;
  using Super::width_;
  using Super::height_;
  using Super::data_;
  using Super::at;
  
  ColorImage(int width = 0, int height = 0, bool doClear = true) : Super(width, height, doClear) { }
  ColorImage(const ColorImage<C> &other) : Super(other) { }
  ColorImage(const GreyImage<C> &gi) : Super(gi.width(), gi.height(), false)
  {
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        p->r() = p->g() = p->b() = gi.at(y, x);
      }
    }
  }
  
  static ColorImageRef make(int width = 0, int height = 0, bool doClear = true) {
    return make_shared<ColorImage>(width, height, doClear);
  }
  
  static ColorImageRef make(const ColorImage * const image) {
    return make_shared<ColorImage>(*image);
  }

  static ColorImageRef make(const ColorImage &image) {
    return make_shared<ColorImage>(image);
  }
  
  ColorImageRef clone() {
    return make(this);
  }
  
  istream &readData(istream &in) {
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        in.read((char *)(p++)->data(), nColorComponents * sizeof(C));
      }
    }
    return in;
  }

  bool writePng(FILE *f) {
    return ColorImages::writePng(*this, f);
  }
  
  bool writePng(const string &filename) {
    FILE *f = fopen(filename.c_str(), "w");
    if (f != NULL) {
      return writePng(f);
    }
    return false;
  }

  static ColorImageRef readPng(FILE *fp) {
    return ColorImages::readPng(fp);
  }

  static ColorImageRef readPng(const char * const filename) {
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
      return readPng(f);
    }
    return ColorImageRef(NULL);
  }
    
  template <typename Pens, typename Targets> void separate(Pens &pens, Targets &targets) {
    // cerr << "separating into " << n << " components" << endl << flush;
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        C cy = Component<C>::inverse(p->r());
        C ma = Component<C>::inverse(p->g());
        C ye = Component<C>::inverse(p->b());
        ++p;
        D(cerr << "@" << y << "," << x << ":" << (int)cy << "/" << (int)ma << "/" << (int)ye << endl << flush);
        
        typename Targets::iterator ti = targets.begin();
        typename Pens::iterator pi = pens.begin();
        while (ti != targets.end() && pi != pens.end()) {
          typename Pens::reference pen = *pi;
          typename Targets::reference target = *ti;
          double d = min(min(min(2.0, density(cy, pen.c())), density(ma, pen.m())), density(ye, pen.y()));
          D(cerr << "density=" << d << " => ");
          if (0.0 < d && d <= 1.0) {
            *target = Component<C>::make(d);
            cy -= Component<C>::make(d, pen.c());
            ma -= Component<C>::make(d, pen.m());
            ye -= Component<C>::make(d, pen.y());
          } else {
            *target = 0;
          }
          D(cerr << "using " << (int)*target << ", remaining:" << (int)cy << "/" << (int)ma << "/" << (int)ye << endl << flush);
          ++target;
          ++pi;
          ++ti;
        }
      }
    }
  }
  
  template <typename Pen> shared_ptr< GreyImage<C> > separateRemainder(const Pen &pen) {
    shared_ptr< GreyImage<C> > result = make_shared< GreyImage<C> >(width_, height_);
    result->copyRes(this);

    D(cerr << "pen has c=" << (int)pen.c() << ",m=" << (int)pen.m() << "y=" << (int)pen.y() << ": ");
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        RGBPixel &p = at(y, x);
        uint8_t cy = Component<C>::inverse(p.r());
        uint8_t ma = Component<C>::inverse(p.g());
        uint8_t ye = Component<C>::inverse(p.b());
        D(cerr << "pixel has c=" << (int)cy << ",m=" << (int)ma << ",y=" << (int)ye << ": ");
        double d = min(min(min(2.0, density(cy, pen.c())), density(ma, pen.m())), density(ye, pen.y()));
        D(cerr << "density=" << d <<endl);
        if (0.0 < d && d <= 1.0) {
          result->at(y, x) = Component<C>::make(d);
        } else {
          result->at(y, x) = Component<C>::make(0);
        }
      }
    }
    return result;
  }
  
  template <typename Pen> shared_ptr< GreyImage<C> > separateAndSubtract(const Pen &pen) {
    shared_ptr< GreyImage<C> > result = make_shared< GreyImage<C> >(width_, height_);
    result->copyRes(*this);
    
    D(cerr << "pen has c=" << (int)pen.c() << ",m=" << (int)pen.m() << "y=" << (int)pen.y() << ": ");
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        RGBPixel &p = at(y, x);
        uint8_t cy = Component<C>::inverse(p.r());
        uint8_t ma = Component<C>::inverse(p.g());
        uint8_t ye = Component<C>::inverse(p.b());
        D(cerr << "pixel has c=" << (int)cy << ",m=" << (int)ma << ",y=" << (int)ye << ": ");
        double d = min(min(min(2.0, density(cy, pen.c())), density(ma, pen.m())), density(ye, pen.y()));
        D(cerr << "density=" << d <<endl);
        if (0.0 < d && d <= 1.0) {
          result->at(y, x) = Component<C>::make(d);
          cy -= Component<C>::make(d, pen.c());
          ma -= Component<C>::make(d, pen.m());
          ye -= Component<C>::make(d, pen.y());
          p.r() = Component<C>::inverse(cy);
          p.g() = Component<C>::inverse(ma);
          p.b() = Component<C>::inverse(ye);
        } else {
          result->at(y, x) = Component<C>::make(0);
        }
      }
    }
    return result;
  }
  
  GreyImage<C> r() {
    GreyImage<C> result(width_, height_);
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        result.at(y, x) = (p++)->r();
      }
    }
    return result;
  }
  
  GreyImage<C> g() {
    GreyImage<C> result(width_, height_);
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        result.at(y, x) = (p++)->g();
      }
    }
    return result;
  }
  
  GreyImage<C> b() {
    GreyImage<C> result(width_, height_);
    RGBPixel *p = data_;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        result.at(y, x) = (p++)->b();
      }
    }
    return result;
  }
};

template <typename D> ostream &operator<<(ostream &out, const RGBPixel<D> &p) {
  out << p.r() << "/" << p.g() << "/" << p.b();
  return out;
}

template <typename C> ostream &operator<<(ostream &out, const ColorImage<C> &i) {
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


#if 0
{
#endif
}

#endif // __ColorImage_h__
