/*
 *  Output.h
 *  Morph
 *
 *  Created by Christian Brunschen on 19/12/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Output_h__
#define __Output_h__

#include <iostream>
#include "Chain.h"
#include "ColorImage.h"

using namespace Primitives;
using namespace Chaining;
using namespace Images;
using namespace std;

template<typename Pen> class Output : public Counted {
protected:
  int width_;
  int height_;

public:
  Output() { }
  virtual ~Output() { }
  
  virtual void open() { }
  virtual void beginPage(int width, int height) {
    width_ = width;
    height_ = height;
  }
  virtual void setPen(const Pen &pen) = 0;
  virtual void outputChain(const Chain &chain) = 0;
  virtual void outputChains(const Chains &chains) {
    for (Chains::const_iterator i = chains.begin();
         i != chains.end();
         ++i) {
      outputChain(*i);
    }
  }
  virtual void endPage() { }
  virtual void close() { }
  virtual ostream &describe(ostream &out) const { return out << "<abstract output>"; } 
};

template <typename Pen>
ostream &operator<<(ostream &out, const Output<Pen> &output) {
  output.describe(out);
  return out;
}


template<typename Pen> class FileStreamOutput : public Output<Pen> {
  typedef Output<Pen> Base;
protected:
  const string filename_;
public:
  ostream *out_;
  FileStreamOutput(ostream &out) : Base(), filename_(""), out_(&out) { }
  FileStreamOutput(const char *filename) : Base(), filename_(filename), out_(NULL) { }
  FileStreamOutput(const string &filename) : Base(), filename_(filename), out_(NULL) { }
  virtual ~FileStreamOutput() { if (out_) { close(); } }
  
  virtual void open() {
    if (out_ == NULL) {
      if (filename_.length() > 0) {
        if ("-" == filename_) {
          out_ = &cout;
        } else {
          ofstream *fout;
          out_ = fout = new ofstream;
          fout->open(filename_.c_str());
        }
      }
    }
  }
  virtual void endPage() {
    *out_ << flush;
  }
  virtual void close() {
    *out_ << flush;
    if (filename_.length() > 0 && filename_ != "-") {
      static_cast<ofstream *>(out_)->close();
      delete out_;
      out_ = NULL;
    }
  }
  virtual ostream &describe(ostream &out) const { 
    return out << "<abstract output to '" << filename_ << "'>"; 
  }
};

template<typename Pen> class PostScriptOutput : public FileStreamOutput<Pen> {
  typedef FileStreamOutput<Pen> Base;
  using Base::out_;
protected:
  using Base::filename_;
public:
  PostScriptOutput(ostream &out) : Base(out) { }
  PostScriptOutput(const char *filename) : Base(filename) { }
  PostScriptOutput(const string &filename) : Base(filename) { }
  virtual ~PostScriptOutput() { }
  
  virtual void beginPage(int width, int height) {
    Base::beginPage(width, height);
    *out_ << "%!PS-Adobe-3.0" << endl;
    *out_ << "%%BoundingBox: -1 -1 " << width+1 << " " << height+1 << endl;
    *out_ << "%%Page: only 1" << endl;
    *out_ << "gsave initclip clippath pathbbox grestore" << endl;
    *out_ << "/ury exch def /urx exch def /lly exch def /llx exch def" << endl;
    *out_ << "/W urx llx sub def /H ury lly sub def" << endl;
    *out_ << "/w " << width << " def /h " << height << " def" << endl;
    *out_ << "/xs W w div def /ys H h div def" << endl;
    *out_ << "/s xs ys lt { xs } { ys } ifelse def" << endl;
    *out_ << "llx urx add 2 div lly ury add 2 div translate" << endl;
    *out_ << "s dup scale w 2 div neg h 2 div neg translate" << endl;
    *out_ << "1 setlinejoin 1 setlinecap" << endl;
  }

  virtual void setPen(const Pen &pen) {
    double r = 1.0 - pen.cFraction();
    double g = 1.0 - pen.mFraction();
    double b = 1.0 - pen.yFraction();
    double w = 2.0 * pen.r();
    *out_ << r << " " << g << " " << b << " setrgbcolor ";
    *out_ << w << " setlinewidth" << endl;
  }
  
  virtual void outputChain(const Chain &chain) {
    Chain::const_iterator k = chain.begin();
    *out_ << k->x() << " " << k->y() << " moveto ";
    ++k;
    if (k == chain.end()) {
      *out_ << "currentpoint lineto stroke" << endl;
    } else {
      for (; k != chain.end(); ++k) {
        *out_ << k->x() << " " << k->y() << " lineto ";
      }
      *out_ << "stroke" << endl;
    }
  }
  
  virtual void endPage() {
    *out_ << "showpage" << endl;
    Base::endPage();
  }

  virtual ostream &describe(ostream &out) const { 
    return out << "PostScript to '" << filename_ << "'"; 
  }
};

template<int base> struct HpEncode {
  int value_;
  HpEncode(int value) : value_(value) { }
};

typedef HpEncode<32> Hp32Encode;
typedef HpEncode<64> Hp64Encode;

template<int base> ostream &operator<<(ostream &out, const HpEncode<base> &h) {
  unsigned int v = h.value_ >= 0
      ? ((unsigned) h.value_) << 1
      : ((unsigned)-h.value_) << 1 | 0x1;
  while (v >= base) {
    out << (char)(63 + v % base);
    v = v / base;
  }
  out << (char)((base == 64 ? 191 : 95) + v);
  return out;
}

template<typename Pen> class HPGLOutput : public FileStreamOutput<Pen> {
  typedef FileStreamOutput<Pen> Base;
protected:
  using Base::out_;
  using Base::filename_;
  using Output<Pen>::width_;
  using Output<Pen>::height_;
  bool betweenPages_;
public:
  HPGLOutput(ostream &out) : Base(out), betweenPages_(true) { }
  HPGLOutput(const char *filename) : Base(filename), betweenPages_(true) { }
  HPGLOutput(const string &filename) : Base(filename), betweenPages_(true) { }

  virtual void setPen(const Pen &pen) {
    betweenPages_ = false;
    *out_ << "SP" << pen.hpglIndex() << ";" << endl;
  }

  virtual void outputChain(const Chain &chain) {
    betweenPages_ = false;
  }

  virtual void endPage() {
    betweenPages_ = true;
    *out_ << "PU;SP0;PG;" << endl;
  }
  
  virtual void close() {
    Base::close();
  }

  virtual ostream &describe(ostream &out) const { 
    return out << "<abstract HPGL to '" << filename_ << "'>"; 
  }
};

template<typename Pen> class HPGLAbsoluteOutput : public HPGLOutput<Pen> {
  typedef HPGLOutput<Pen> Base;
  using Base::out_;
protected:
  using Base::filename_;
public:
  HPGLAbsoluteOutput(ostream &out) : Base(out) { }
  HPGLAbsoluteOutput(const char *filename) : Base(filename) { }
  HPGLAbsoluteOutput(const string &filename) : Base(filename) { }
  virtual void outputChain(const Chain &chain) {
    Chain::const_iterator k = chain.begin();
    Point p = *k;
    *out_ << "PU " << p.x() << " " << p.y() << ";PD ";
    bool first = true;
    for (++k; k != chain.end(); ++k) {
      if (first) { first = false; } else { *out_ << " "; }
      p = *k;
      *out_ << p.x() << " " << p.y();
    }
    if (first) {
      k = chain.begin();
      *out_ << p.x() << " " << p.y();
    }
    *out_ << ";" << endl;
  }

  virtual ostream &describe(ostream &out) const { 
    return out << "HPGL (absolute coordinates) to '" << filename_ << "'"; 
  }
};

template<typename Pen> class HPGLRelativeOutput : public HPGLOutput<Pen> {
  typedef HPGLOutput<Pen> Base;
  using Base::out_;
protected:
  using Base::filename_;
  Point currentPoint_;
  bool haveCurrentPoint_;
public:
  HPGLRelativeOutput(ostream &out) : Base(out) { }
  HPGLRelativeOutput(const char *filename) : Base(filename) { }
  HPGLRelativeOutput(const string &filename) : Base(filename) { }
  virtual void outputChain(const Chain &chain) {
    if (chain.size() < 2) return;
    Chain::const_iterator k = chain.begin();
    Point p = *k;
    if (haveCurrentPoint_) {
      int dx = p.x() - currentPoint_.x();
      int dy = p.y() - currentPoint_.y();
      *out_ << "PU " << dx << " " << dy << ";PD ";
    } else {
      *out_ << "PA;PU " << p.x() << " " << p.y() << ";PR;PD ";
    }
    bool first = true;
    for (++k; k != chain.end(); ++k) {
      if (first) { first = false; } else { *out_ << " "; }
      Point q = *k;
      int dx = q.x() - p.x();
      int dy = q.y() - p.y();
      *out_ << dx << " " << dy;
      p = q;
    }
    if (first) {
      *out_ << "0 0";
    }
    *out_ << ";" << endl;
    currentPoint_ = p;
    haveCurrentPoint_ = true;
  }
  
  virtual void endPage() {
    Base::endPage();
    haveCurrentPoint_ = false;
  }  

  virtual ostream &describe(ostream &out) const { 
    return out << "HPGL (relative coordinates) to '" << filename_ << "'"; 
  }
};

template<typename Pen, int base> class HPGLEncodedOutput : public HPGLOutput<Pen> {
  typedef HPGLOutput<Pen> Base;
  using Base::out_;
  typedef ::HpEncode<base> HpEncode;
protected:
  using Base::filename_;
  Point currentPoint_;
  bool haveCurrentPoint_;
public:
  HPGLEncodedOutput(ostream &out) : Base(out) { }
  HPGLEncodedOutput(const char *filename) : Base(filename) { }
  HPGLEncodedOutput(const string &filename) : Base(filename) { }
  virtual void outputChain(const Chain &chain) {
    Chain::const_iterator k = chain.begin();
    // first, an absolute move to the beginning of the start of the chain
    Point p = *k;
    if (haveCurrentPoint_) {
      int dx = p.x() - currentPoint_.x();
      int dy = p.y() - currentPoint_.y();
      *out_ << (base == 32 ? "PE7<" : "PE<") << HpEncode(dx) << HpEncode(dy);
    } else {
      *out_ << (base == 32 ? "PE7<=" : "PE<=") << HpEncode(p.x()) << HpEncode(p.y());
    }
    bool first = true;
    for (++k; k != chain.end(); ++k) {
      // relative moves for the rest of the chain
      Point q = *k;
      int dx = q.x() - p.x();
      int dy = q.y() - p.y();
      *out_ << HpEncode(dx) << HpEncode(dy);
      p = q;
    }
    if (first) {
      // only a single point - a pen-down move with a delta of 0 to mark this point
      *out_ << HpEncode(0) << HpEncode(0);
    }
    *out_ << ";" << endl;
    currentPoint_ = p;
    haveCurrentPoint_ = true;
  }

  virtual void endPage() {
    Base::endPage();
    haveCurrentPoint_ = false;
  }  
  
  virtual ostream &describe(ostream &out) const { 
    return out << "HPGL (using Polyline Encoded) to '" << filename_ << "'"; 
  }
};

template<typename Tool> class RML1Output : public FileStreamOutput<Tool> {
  typedef FileStreamOutput<Tool> Base;
  using Base::out_;
  using Base::filename_;

  int x_;
  int y_;
  int z_;
  int v_;

  int vMoveXY_;
  int vMoveZ_;
  int vCutXY_;
  int vCutZ_;
  
  int zUp_;
  int zDown_;

public:
  void init() {
    x_ = y_ = z_ = 0;
    vMoveXY_ = vMoveZ_ = 15;
    vCutXY_ = 5;
    vCutZ_ = 1;
    zUp_ = 40;
    zDown_ = 0;
  }
  RML1Output(ostream &out) : Base(out) { init(); }
  RML1Output(const char *filename) : Base(filename) { init(); }
  RML1Output(const string &filename) : Base(filename) { init(); }
  
  
  virtual void setPen(const Tool &tool) {
    zUp_ = tool.zUp();
    zDown_ = tool.zDown();
  }

  virtual void open() {
    Base::open();
    (*out_) << ";;^IN;!MC0;V15.0;^PR;Z0,0,2420;^PA;!MC1;\n";
    x_ = 0;
    y_ = 0;
    z_ = 2420;
  }
  
  void setZUp(int zUp) {
    zUp_ = zUp;
  }

  void setZDown(int zDown) {
    zDown_ = zDown;
  }

  void setV(int v) {
    if (v != v_) {
      v_ = v;
      (*out_) << "V" << v << ";\n";
    }
  }

  void _moveTo(int x, int y, int z) {
    if (x != x_ || y != y_ || z != z_) {
      (*out_) << "Z" << x << "," << y << "," << z << ";\n";
    }
    x_ = x;
    y_ = y;
    z_ = z;
  }
  
  void moveTo(int x, int y, int z) {
    // If we're trying to move below zero, let's move up to be safe.
    if (z < 0) z = zUp_;
    if (x != x_ || y != y_) {
      // movement along the XY plane
      if (z != z_) {
        // movement in Z as well! Let's make a straight-up move first.
        setV(vMoveZ_);
        _moveTo(x_, y_, z);
      }
      setV(vMoveXY_);
      _moveTo(x, y, z);
    }
  }

  void cutTo(int x, int y, int z) {
    if (x != x_ || y != y_) {
      // cutting along the XY plane
      if (z >= z_) {
        // and movement level or up - consider this an XY move-or-cut for speed purposes
        if (z_ <= 0) {
          // at least partly at or below zero => cut.
          setV(vCutXY_);
        } else {
          // above zero => move.
          setV(vMoveXY_);
        }
      } else {
        // movement down as well - always considered a z cut
        setV(vCutZ_);
      }
    } else if (z < z_) {
      // cut straight down
      setV(vCutZ_);
    } else if (z > z_) {
      // move straight up
      setV(vMoveZ_);
    }
    _moveTo(x, y, z);
  }
  
  virtual void outputChain(const Chain &chain) {
    Chain::const_iterator k = chain.begin();
    Point p = *k;
    
    moveTo(p.x(), p.y(), zUp_);

    cutTo(p.x(), p.y(), zDown_);
    for (++k; k != chain.end(); ++k) {
      p = *k;
      cutTo(p.x(), p.y(), z_);
    }

    cutTo(p.x(), p.y(), zUp_);
  }

  virtual void close() {
    (*out_) << "!VZ15.0;!ZM0;!MC0;^IN;\n";
    Base::close();
  }
  
  virtual ostream &describe(ostream &out) const {
    return out << "<RML1 to '" << filename_ << "'>";
  }
};

template<typename Pen, typename C> class ColorImageOutput : public Output<Pen> {
  typedef Output<Pen> Base;
  typedef Images::ColorImage<C> ColorImage;

  ColorImage &image_;
  Bitmap &currentPenCoverage_;
public:
  ColorImageOutput(ColorImage &image) : image_(image), currentPenCoverage_(image.width(), image.height()) { }
  virtual void open() {
    Base::open();
    RGBPixel<C> whitePixel(1.0, 1.0, 1.0);
    image_.set(whitePixel);
  }
  virtual void outputChain(const Chain &chain) {
    // Chain::const_iterator k = chain.begin();
  }
};

#endif // __Output_h__
