/*
 *  Hatcher.h
 *  Morph
 *
 *  Created by Christian Brunschen on 01/12/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Hatcher_h__
#define __Hatcher_h__

#include "Primitives.h"
#include "Line.h"
#include "Chain.h"
#include <list>
#include <algorithm>

namespace Hatching {
// to defeat Xcode's indenting
#if 0
}
#endif

using namespace std;
using namespace Chaining;
using namespace Primitives;

// |angle| is in pi-radians: useful range is [0 .. 1)
template <typename F> void hatch(int width, int height, double angle, double period, double phase, F &f) {
  // ensure that 0.0 <= angle < 1.0
  if (angle >= 1.0) {
    angle -= floor(angle);
  } else if (angle < 0.0) {
    angle += ceil(angle);
  }
  
  // translate from pi-radians to radians
  double theta = angle * M_PI;
  
  // get the bounding box of the rotated rectangle - extended by 1
  // in the x direction to handle rounding
  double llx, lly, lrx, lry, ulx, uly, urx, ury;
  rotate(llx, lly, 0, 0, theta);
  rotate(lrx, lry, width-1, 0, theta);
  rotate(ulx, uly, 0, height-1, theta);
  rotate(urx, ury, width-1, height-1, theta);
  
  // get the minimum and maximum extents of the bounding box
  double xmin = min(min(min(llx, lrx), urx), ulx) - 1;
  double xmax = max(max(max(llx, lrx), urx), ulx) + 1;
  double ymin = min(min(min(lly, lry), ury), uly);
  double ymax = max(max(max(lly, lry), ury), uly);
  
  // calculate how far below zero we need to start in order to cover every part
  // of the rotated rectangle
  int n = floor((phase - ymin) / period);
  
  for (double y = phase - n * period;
       y <= ymax;
       y += period) {
    double x0, x1, y0, y1;
    rotate(x0, y0, xmin, y, -theta);
    rotate(x1, y1, xmax, y, -theta);
    line(rint(x0), rint(y0), rint(x1), rint(y1), f);
  }
}

template <typename Include> struct Hatcher {
  Include include_;
  
  struct Extent : public Chain {
    int from_;
    int to_;
    Extent(int from) : Chain(), from_(from), to_(-1) { }
  };

  typedef list<Extent> Row;
  typedef typename Row::iterator RowIterator;
  
  typedef list<Row> Grid;
  typedef typename Grid::iterator GridIterator;

  struct GridCursor {
    GridIterator row;
    RowIterator extent;
    bool isFrom;
    const Point &point() {
      return isFrom ? extent->front() : extent->back();
    }
    double squareDistance(const Point &p) {
      return point().squareDistance(p);
    }
    double squareDistance(const GridCursor &c) {
      return point().squareDistance(c.point());
    }
  };
  
  Grid grid_;
  Row *row_;
  Extent *extent_;
  Point previous_;
  int along_;
  
  Hatcher(Include include) : include_(include), previous_(numeric_limits<int>::min(), -numeric_limits<int>::min()) { }
  
  void operator() (int x, int y) {
    Point p(x, y);
    if (!p.isNeighbour(previous_)) {
      // started a new row!
      grid_.push_back(Row());
      row_ = &(grid_.back());
      along_ = 0;
      extent_ = NULL;
    }
    previous_ = p;
    if (include_(p)) {
      if (!extent_) {
        row_->push_back(Extent(along_));
        extent_ = &(row_->back());
      }
      extent_->addPoint(p);
      extent_->to_ = along_;
    } else {
      if (extent_) {
        extent_ = NULL;
      }
    }
    ++along_;
  }
    
  bool start(GridIterator &r, RowIterator &e) {
    for (r = grid_.begin(); r != grid_.end(); r++) {
      e = r->begin();
      if (e != r->end()) {
        return true;
      }
    }
    return false;
  }
  
  static double minDistance(Point &p, const Extent &e, bool &isFrom) {
    double dFrom = p.squareDistance(e.front());
    double dTo = p.squareDistance(e.back());
    if (dFrom <= dTo) {
      isFrom = true;
      return dFrom;
    } else {
      isFrom = false;
      return dTo;
    }
  }
  
  double findNearestExtentInRow(Row &row, Point &p, /*out*/ RowIterator &eOut, bool &isFrom) {
    int dMin = std::numeric_limits<int>::max();
    for (RowIterator e = row.begin(); e != row.end(); ++e) {
      double dFrom = p.squareDistance(e->front());
      if (dFrom < dMin) {
        eOut = e;
        dMin = dFrom;
        isFrom = true;
      }
      double dTo = p.squareDistance(e->back());
      if (dTo < dMin) {
        eOut = e;
        dMin = dTo;
        isFrom = false;
      }
    }
    return dMin;
  }
  
  template<typename I> static void move(I &i, int delta) {
    while (delta > 0) {
      ++i;
      --delta;
    }
    while (delta < 0) {
      --i;
      ++delta;
    }
  }

  double findNearerExtent(GridIterator r0, int direction, Point &p, double period, double dMin, /*out*/ GridCursor &c) {
    GridIterator r = r0;
    move(r, direction);
    double dy = period;
    double d2y = dy*dy;
    while (r != grid_.end() && d2y <= dMin) {
      bool isFrom;
      RowIterator e;
      double d = findNearestExtentInRow(*r, p, e, isFrom);
      if (d < dMin) {
        c.row = r;
        c.extent = e;
        c.isFrom = isFrom;
        dMin = d;
      }
      move(r, direction);
      dy += period;
      d2y = dy * dy;
    }
    return dMin;
  }
  
  double findNearestExtent(GridIterator r, Point &p, double period, /*out*/ GridCursor &c) {
    bool isFrom;
    RowIterator e;
    double d = findNearestExtentInRow(*r, p, e, isFrom);
    if (e != r->end()) {
      c.row = r;
      c.extent = e;
    } else {
      c.row = grid_.end();
    }
    // search positive
    d = findNearerExtent(r, +1, p, period, d, c);
    // search negative
    d = findNearerExtent(r, -1, p, period, d, c);
    return d;
  }
  
  void addToChains(Chains &chains, bool front, GridCursor &c) {
    Chain &chain = front ? chains.prependChain() : chains.addChain();
    chain.splice(chain.begin(), *c.extent);
    if (!c.isFrom) {
      chain.reverse();
    }
    c.row->erase(c.extent);
    c.row = grid_.end();
  }
    
  template <typename CanRetract> void retractGrid(CanRetract &canRetract) {
    for (GridIterator r = grid_.begin(); r != grid_.end(); ++r) {
      RowIterator e = r->begin();
      while (e != r->end()) {
//        cerr << "Retracting " << *e << "; ";
        // retract front
        typename Extent::iterator i = e->begin();
        while (i != e->end()) {
          typename Extent::iterator ni = i; ++ni;
          int neighbours = ni == e->end() ? 0 : (1 << i->directionTo(*ni));
          if (canRetract(*i, neighbours)) {
            i = e->erase(i);
            i = ni;
          } else {
            break;
          }
        }
        
        // retract back
        typename Extent::reverse_iterator j = e->rbegin();
        while (j != e->rend()) {
          typename Extent::reverse_iterator nj = j; ++nj;
          int neighbours = nj == e->rend() ? 0 : (1 << j->directionTo(*nj));
          if (canRetract(*j, neighbours)) {
            j = nj;
          } else {
            break;
          }
        }
        for (i = j.base(); i != e->end();) {
          i = e->erase(i);
        }

        if (e->empty()) {
          // if we've retracted everything, remove this extent from its containing row
          e = r->erase(e);
//          cerr << "all retracted!" << endl;
        } else {
          // otherwise, simply step to the next extent
//          cerr << "remaining: " << *e << endl;
          ++e;
        }
      }
    }
  }
  
  template<typename CanRetract> void getChains(Chains &chains, double period, CanRetract &canRetract) {
    retractGrid(canRetract);
    int remaining = 0;
    for (GridIterator r = grid_.begin(); r != grid_.end(); ++r) {
      remaining += r->size();
    }
    if (remaining > 0) {
      GridIterator rFront, rBack;
      RowIterator e;
      start(rFront, e);
      rBack = rFront;
      Point pFront = e->front(), pBack = e->back();

      // insert the current extent as the first chain of the sequence of chains
      Chain &chain = chains.addChain();
      chain.splice(chain.begin(), *e);
      // and remove the extent from this row (as it's already been handled)
      rFront->erase(e);
      --remaining;

      // the cursors for finding the next candidate
      GridCursor cFront, cBack;
      cFront.row = cBack.row = grid_.end();

      double dFront, dBack;

      while (remaining > 0) {
        if (cFront.row == grid_.end()) {
          // find the nearest extent relative to the front
          dFront = findNearestExtent(rFront, pFront, period, cFront);
        }
        dFront = numeric_limits<double>::max();

        if (cBack.row == grid_.end()) {
          // find the nearest extent relative to the back
          dBack = findNearestExtent(rBack, pBack, period, cBack);
        }
        
        if (dFront < dBack) {
          rFront = cFront.row;
          pFront = cFront.isFrom ? cFront.extent->back() : cFront.extent->front();
          if (cBack.extent == cFront.extent) cBack.row = grid_.end();
          addToChains(chains, true, cFront);
        } else {
          rBack = cBack.row;
          pBack = cBack.isFrom ? cBack.extent->back() : cBack.extent->front();
          if (cFront.extent == cBack.extent) cFront.row = grid_.end();
          addToChains(chains, false, cBack);
        }
        --remaining;
      }
    }
  }
  
  int gridWidth() {
    int maxX = 0;
    for (GridIterator row = grid_.begin(); row != grid_.end(); ++row) {
      if (row->size() > 0) {
        if (row->back().to_ > maxX) {
          maxX = row->back().to_;
        }
      }
    }
    return maxX + 1;
  }
  
  int gridHeight() {
    return grid_.size();
  }
  
  template<typename B> void printGrid(B &b, int period) {
    int y = 0;
    for (GridIterator row = grid_.begin(); row != grid_.end(); ++row) {
      cerr << y << ": ";
      for (RowIterator extent = row->begin(); extent != row->end(); ++extent) {
        cerr << extent->from_ << "->" << extent->to_ << " ";
        for (int x = extent->from_; x <= extent->to_; ++x) {
          Point p(x, y);
          b.at(p) = true;
        }
      }
      cerr << endl;
      y += period;
    }
  }
  
  void verifyGrid() {
    for (GridIterator r = grid_.begin(); r != grid_.end(); ++r) {
      for (RowIterator e = r->begin(); e != r->end(); ++e) {
        if (e->size() > 1) {
          Chain::iterator j = e->begin();
          Point p = *j;
          for (++j; j != e->end(); ++j) {
            if (!p.isNeighbour(*j)) {
              cerr << "not neighbours: " << p << " <-> " << *j << endl;
            }
            p = *j;
          }
        }
      }
    }
  }
};

// to defeat Xcode's indenting
#if 0
{
#endif
}


#endif // __Hatcher_h__
