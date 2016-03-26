/*
 *  Progress.h
 *  Morph
 *
 *  Created by Christian Brunschen on 27/11/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Progress_h__
#define __Progress_h__

#include <iostream>
#include <iomanip>
#include "Primitives.h"
#include "Bitmap.h"
#include "ColorImage.h"

using namespace std;
using namespace Primitives;
using namespace Images;

namespace Progress {
#if 0
}
#endif

class Stepper {
  int step_;
  string prefix_;
  string pen_;
public:
  Stepper(const string &prefix, const string &pen = "no pen", int step = 1) : step_(step), prefix_(prefix), pen_(pen) { }
  void setPen(const char *s) { pen_ = s; }
  void setPen(const string &s) { pen_ = s; }
  void clearPen() { pen_ = "no_pen"; }
  const string &pen() { return pen_; }
  void clearStep() { step_ = 1; }
  int step() { return step_; }
  const string makeName(const char *s, const char *pen = NULL) {
    ostringstream os;
    os << prefix_ << "_";
    os << setw(3) << setfill('0') << step_++ << "_";
    if (pen) {
      os << pen << "_";
    } else {
      os << pen_ << "_";
    }
    os << s;
    os << flush;
    return os.str();
  }
};

class Approximator {
  ColorImage<uint8_t> approximation_;
  double cFraction, mFraction, yFraction;
public:
  Approximator(int w, int h) : approximation_(w, h, true) { }
  template<typename C> void setPenColor(const PenColor<C> &color) {
    cFraction = color.cFraction();
    mFraction = color.mFraction();
    yFraction = color.yFraction();
  }
  void addToApproximation(shared_ptr<Bitmap> covered) {
    for (int y = 0; y < covered->height(); y++) {
      for (int x = 0; x < covered->width(); x++)  {
        if (covered->at(x, y)) {
          RGBPixel<uint8_t> &p = approximation_.at(x, y);
          p.rFraction() -= cFraction;
          p.gFraction() -= mFraction;
          p.bFraction() -= yFraction;
          // approximation_.at(x, y) = p;
        }
      }
    }
  }
  ColorImage<uint8_t> &approximation() { return approximation_; }
};

#if 0
{
#endif
}

#endif // __Progress_h__
