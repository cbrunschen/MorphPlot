//
//  Circle_Impl.h
//  MorphPlot
//
//  Created by Christian Brunschen on 18/02/2014.
//
//

#ifndef __Circle_Impl_h__
#define __Circle_Impl_h__

#include "Circle.h"

namespace Primitives {
#if 0
}
#endif

#if DEBUG
#define D(x) do { x; } while(0)
#else
#define D(x)
#endif

inline int Circle::makeExtents(int r, int extents[], double adjust) {
  int count = 0;
#if DEBUG
  int s = 2*r + 1;
  Bitmap element(s, s);
#define SETELEMENT(x, y) do {\
cerr << "setting(" << (y) << "," << (x) << ")" << endl;\
element.at(((x), (y))) = true;\
} while(0)
  SETELEMENT(r, r);
  for (int ddd = 1; ddd <= r; ddd++) {
    SETELEMENT(r    , r+ddd);
    SETELEMENT(r    , r-ddd);
    SETELEMENT(r+ddd, r    );
    SETELEMENT(r-ddd, r    );
  }
#endif

  if (adjust < 0.0) adjust = 0.0;
  int extent = r;
  double rAdjusted = r + adjust;
  double rSquared = rAdjusted * rAdjusted;
  for (int y = 1; y <= r; y++) {
    int ySquared = y*y;
    while (extent > 0 && extent*extent + ySquared > rSquared) {
      extent--;
    }
    extents[y-1] = extent;
    count += extent;
    D(cerr << "extent[" << y << "] = " << extents[y-1] << endl);
#if DEBUG
    for (int ddd = 1; ddd <= extent; ddd++) {
      SETELEMENT(r + y, r - ddd);
      SETELEMENT(r + y, r + ddd);
      SETELEMENT(r - y, r - ddd);
      SETELEMENT(r - y, r + ddd);
    }
#endif
  }
#if DEBUG
  cerr << "element with " << 4*(count + r) + 1 << ":" << endl;
  element.writeText(cerr);
#endif
  return count;
}

#undef D

#if 0
{
#endif
}

#endif  // __Circle_Impl_h__
