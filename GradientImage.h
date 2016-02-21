//
//  GradientImage.hpp
//  MorphPlot
//
//  Created by Christian Brunschen on 21/02/2016.
//  Copyright 2016 Christian Brunschen. All rights reserved.
//
//

#ifndef __GradientImage_h__
#define __GradientImage_h__

#include "Image.h"

namespace Images {
#if 0
}
#endif

namespace Gradient {
#if 0
}
#endif

struct XY {
public:
  double x_;
  double y_;
  
  XY(double x = 0.0, double y = 0.0) : x_(x), y_(y) { }
  double &x() { return x_; }
  const double x() const { return x_; }
  void setX(double x) { x_ = x; }
  
  double &y() { return y_; }
  const double y() const { return y_; }
  void setY(double y) { y_ = y; }
};

#if 0
{
#endif
}  // namespace Gradient

class GradientImage : public Image<Gradient::XY> {
};

#if 0
{
#endif
}  // namespace Images

#endif /* __GradientImage_h__ */
