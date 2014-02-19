/*
 *  FloodFill.h
 *  Morph
 *
 *  Created by Christian Brunschen on 16/01/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __FloodFill_h__
#define __FloodFill_h__

#include <list>

namespace Filling {
#if 0
}
#endif

using namespace std;

namespace Private {
#if 0
}
#endif

/*
 * Filled horizontal segment of scanline y for left<=x<=right.
 * Parent segment was on line y-dy.  dy=1 or -1
 */
struct StackEntry {
  int y;
  int left;
  int right;
  int dy;
  StackEntry(int yy, int ll, int rr, int dd) : y(yy), left(ll), right(rr), dy(dd) { }
};

struct Stack : public list<StackEntry> {
  void push(const int &y, const int &left, const int &right, const int &dy) {
    push_back(StackEntry(y, left, right, dy));
  }
  void pop(int &y, int &left, int &right, int &dy) {
    const StackEntry &e = back();
    y = e.y + (dy = e.dy);
    left = e.left;
    right = e.right;
    pop_back();
  }
};

template <typename IsMarked, typename Mark, int extend>
void fill(int y, int x, int H, int W, IsMarked isMarked, Mark mark) {
  Stack stack;
  int l, x1, x2, dy;
  if (x < 0 || x >= W || y < 0 || y >= H || isMarked(y, x)) {
    return;
  }
  
  stack.push(y, x, x, 1); /* needed in some cases */
  stack.push(y+1, x, x, -1);  /* seed segment (popped 1st) */
  
  while (!stack.empty()) {
    /* pop segment off stack and fill a neighboring scan line */
    stack.pop(y, x1, x2, dy);
    
    if (y < 0 || y >= H) {
      continue;
    }
    
    /*
     * segment of scan line y-dy for x1<=x<=x2 was previously filled,
     * now explore adjacent pixels in scan line y
     */
    for (x = x1 - extend; x >= 0 && !isMarked(y, x); x--) {
      mark(y, x);
    }
    if (x >= x1 - extend) {
      goto skip;
    }
    l = x + 1;
    if (l < x1) {
      stack.push(y, l, x1 - 1, -dy);  /* leak on left? */
    }
    x = x1 - extend + 1;
    do {
      for (; x < W && !isMarked(y, x); x++) {
        mark(y, x);
      }
      stack.push(y, l, x - 1, dy);
      if (x > x2 + 1) {
        stack.push(y, x2 + 1, x - 1, -dy);  /* leak on right? */
      }
    skip:
      for (x++; x <= x2 + extend && isMarked(y, x); x++)
        ;
      l = x;
    } while (x <= x2 + extend);
  }
}

#if 0
{
#endif
}

template <typename IsMarked, typename Mark>
void fill4(int y, int x, int H, int W, IsMarked isMarked, Mark mark) {
  Filling::Private::fill<IsMarked, Mark, 0>(y, x, H, W, isMarked, mark);
}

template <typename IsMarked, typename Mark>
void fill8(int y, int x, int H, int W, IsMarked isMarked, Mark mark) {
  Filling::Private::fill<IsMarked, Mark, 1>(y, x, H, W, isMarked, mark);
}

#if 0
{
#endif
}

#endif // __FloodFill_h__
