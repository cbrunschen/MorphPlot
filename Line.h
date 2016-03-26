/*
 *  Line.h
 *  Morph
 *
 *  Created by Christian Brunschen on 26/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Line_h__
#define __Line_h__

#include "Primitives.h"
#include "Circle.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

namespace Primitives {
#if 0
}
#endif

using namespace std;

template <typename F> void line(int x0, int y0, int x1, int y1, F &f) {
  IMatrix matrix = IMatrix::translate(x0, y0);
  int dx = x1 - x0;
  int dy = y1 - y0;
  if (dx < 0) {
    matrix = matrix.concat(IMatrix::flipX());
    dx = -dx;
  }
  if (dy < 0) {
    matrix = matrix.concat(IMatrix::flipY());
    dy = -dy;
  }
  if (dy > dx) {
    matrix = matrix.concat(IMatrix::flipXY());
    swap(dy, dx);
  }

  int error = dx / 2;
  int y = 0;
  for (int x = 0; x <= dx; x++) {
    int xx, yy;
    matrix.transform(xx, yy, x, y);
    f(xx, yy);
    error = error - dy;
    if (error < 0) {
      y += 1;
      error += dx;
    }
  }
}

template <typename F, typename D>
void drawAll(F &f, const IPoint &p, const D &points, const IMatrix &matrix) {
  for (typename D::const_iterator i = points.begin(); i != points.end(); ++i) {
    f(((*i) + p).transform(matrix));
  }
}

template <typename I, typename DX, typename DXY, typename F>
void line(int x0, int y0, int x1, int y1,
          F &f,
          const I &initial,
          const DX &deltaX,
          const DXY &deltaXY) {
  IMatrix matrix = IMatrix::translate(x0, y0);
  int dx = x1 - x0;
  int dy = y1 - y0;
  if (dx < 0) {
    matrix = matrix.concat(IMatrix::flipX());
    dx = -dx;
  }
  if (dy < 0) {
    matrix = matrix.concat(IMatrix::flipY());
    dy = -dy;
  }
  if (dy > dx) {
    matrix = matrix.concat(IMatrix::flipXY());
    swap(dy, dx);
  }

  drawAll(f, IPoint(0, 0), initial, matrix);

  int error = dx / 2;
  IPoint p(0, 0);
  for (p.x() = 1; p.x() <= dx; p.x()++) {
    error = error - dy;
    if (error < 0) {
      p.y()++;
      error += dx;
      drawAll(f, p, deltaXY, matrix);
    } else {
      drawAll(f, p, deltaX, matrix);
    }
  }
}

template <typename F>
void line(int x0, int y0, int x1, int y1,
          F &f,
          const Circle &circle) {
  line(x0, y0, x1, y1, f,
       circle.points(), circle.getHorizontalDelta(), circle.getDiagonalDelta());
}


inline void rotate(double &nx, double &ny, const double &x, const double &y, const double &theta) {
  double cosTheta = cos(theta);
  double sinTheta = sin(theta);
  nx = x * cosTheta - y * sinTheta;
  ny = x * sinTheta + y * cosTheta;
}

#if 0
{
#endif
}

#endif // __Line_h__
