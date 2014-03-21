/*
 *  Circle.h
 *  Morph
 *
 *  Created by Christian Brunschen on 14/08/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Circle_h__
#define __Circle_h__

#include "Primitives.h"

namespace Primitives {
#if 0
}
#endif

using namespace std;

class Circle {
  static list<Point> noPoints_;

  int r_;
  int area_;
  int *extents_;
  mutable list<Point> *points_;
  mutable vector<int> *directionsCw_;
  mutable vector<int> *directionsCcw_;
  mutable vector<Point> *offsetsCw_;
  mutable vector<Point> *offsetsCcw_;
  
  typedef unordered_map<int, list<Point> > Deltas;
  mutable Deltas deltas_;

public:
  static int makeExtents(int r, int extents[], double adjust = 0.0);
  
  template<typename I>
  static void makePoints(int r, int extents[], I i) {
    // centre point
    *i++ = Point(0, 0);
    // central cross
    for (int t = 1; t <= r; t++) {
      *i++ = Point(0, t);
      *i++ = Point(0, -t);
      *i++ = Point(t, 0);
      *i++ = Point(-t, 0);
    }
    // quadrants
    for (int t = 1; t <= r; t++) {
      for (int u = 1; u <= extents[t-1]; u++) {
        *i++ = Point(t, u);
        *i++ = Point(-t, u);
        *i++ = Point(t, -u);
        *i++ = Point(-t, -u);
      }
    }
  }
  
  static int getExtent(int r, int extents[], int y) {
    if (y < 0) y = -y;
    if (y > r) return 0;
    if (y == 0) return r;
    return extents[y - 1];
  }
    
  template<typename I>
  static void makeNeighbourhoodDelta(int r, int extents[], I i, const list<Point>& circlePoints, int neighbours) {
    // cerr << "making delta for neighbourhood " << PH<int>(neighbours) << ": ";
    unordered_set<Point> result;
    copy(circlePoints.begin(), circlePoints.end(), inserter(result, result.begin()));
    
    // cerr << "points without neighbours: ";
    // for (unordered_set<Point>::const_iterator ii = result.begin(); ii != result.end(); ++ii)
    //   cerr << *ii << ", ";
    // cerr << ";" << endl;
    
    for (int j = 0; j < DIRECTIONS; j++) {
      if (neighbours & (1 << j)) {
        // cerr << "removing direction " << j << ": ";
        int dx = xOffsets[j];
        int dy = yOffsets[j];
        for (list<Point>::const_iterator p = circlePoints.begin();
             p != circlePoints.end();
             ++p) {
          result.erase(p->offset(dx, dy));
        }
        
        // cerr << "remaining: ";
        // for (unordered_set<Point>::const_iterator ii = result.begin(); ii != result.end(); ++ii)
        //   cerr << *ii << ", ";
        // cerr << ";" << endl;
      }
    }
    copy(result.begin(), result.end(), i);
  }
  
public:
  Circle(int r)
  : r_(r), area_(0), extents_(new int[r]), points_(NULL), directionsCw_(NULL), directionsCcw_(NULL), offsetsCw_(NULL), offsetsCcw_(NULL)
  {
    int count = makeExtents(r_, extents_);
    area_ = 4 * (count + r) + 1;
  }
  
  virtual ~Circle() {
    delete extents_;
    if (points_) delete points_;
    if (directionsCw_) delete directionsCw_;
    if (directionsCcw_) delete directionsCcw_;
    if (offsetsCw_) delete offsetsCw_;
    if (offsetsCcw_) delete offsetsCcw_;
  }
  
  int r() const {
    return r_;
  }
  
  int area() const {
    return area_;
  }
  
  const int * const extents() const {
    return extents_;
  }
  
  const list<Point> &points() const {
    if (!points_) {
      points_ = new list<Point>;
      makePoints(r_, extents_, back_inserter(*points_));
    }
    return *points_;
  }
  
  const list<Point> &getDelta(int neighbours) const {
    Deltas::iterator i = deltas_.find(neighbours);
    if (i == deltas_.end()) {
      list<Point> &delta = deltas_[neighbours];
      makeNeighbourhoodDelta(r_, extents_, back_inserter(delta), points(), neighbours);
      return delta;              
    } else {
      return i->second;
    }
  }
  
  const list<Point> &getHorizontalDelta() const {
    return getDelta(1 << Neighbourhood::W);
  }
  
  const list<Point> &getDiagonalDelta() const {
    return getDelta(1 << Neighbourhood::NW);
  }
  
  const list<Point> &getDeltaForDirection(int dir) const {
    if (dir < 0 || DIRECTIONS <= dir) return noPoints_;
    return getDelta(1 << dir);
  }
  
  template<typename I, typename J> static void makeOffsets(I i, Point p, J j, const J &jEnd) {
    *i++ = p;
    while (j != jEnd) {
      p = p.neighbour(*j++);
      *i++ = p;
    }
  }
  
  void ensureDirectionsAndOffsets() const {
    if (!directionsCw_) {
      list<int> directionsCw;
      list<int> directionsCcw;
      
      // quadrant NE
      int x = r_, e, y;
      for (y = 1; y <= r_; y++) {
        e = getExtent(r_, extents_, y);
        D(cerr << "y=" << y << ", x=" << x << ", e=" << e << " => ");
        if (x > e) {
          while (x > e+1) {
            D(cerr << "W ");
            directionsCw.push_back(W);
            directionsCcw.push_back(W);
            x--;
          }          
          D(cerr << "NW ");
          directionsCw.push_back(NW);
          directionsCcw.push_back(SW);
          x--;
        } else {
          D(cerr << "N ");
          directionsCw.push_back(N);
          directionsCcw.push_back(S);
        }
        D(cerr << endl);
      }
      
      // top
      y--;
      D(cerr << "y=" << y << ", x=" << x << ", -e=" << -e << " => ");
      while (x > -e) {
        D(cerr << "W ");
        directionsCw.push_back(W);
        directionsCcw.push_back(W);
        x--;
      }
      D(cerr << endl);
      
      // Quadrant NW
      for (y--; y >= 0; y--) {
        e = getExtent(r_, extents_, y);
        D(cerr << "y=" << y << ", x=" << x << ", -e=" << -e << " => ");
        if (x > -e) {
          D(cerr << "SW ");
          directionsCw.push_back(SW);
          directionsCcw.push_back(NW);
          x--;
          while (x > -e) {
            D(cerr << "W ");
            directionsCw.push_back(W);
            directionsCcw.push_back(W);
            x--;
          }          
        } else {
          D(cerr << "S ");
          directionsCw.push_back(S);
          directionsCcw.push_back(N);
        }
        D(cerr << endl);
      }
      
      // quadrant SW
      for (; y >= -r_; y--) {
        e = getExtent(r_, extents_, y);
        D(cerr << "y=" << y << ", x=" << x << ", -e=" << -e << " => ");
        if (x < -e) {
          while (x < -(e+1)) {
            D(cerr << "E ");
            directionsCw.push_back(E);
            directionsCcw.push_back(E);
            x++;
          }          
          D(cerr << "SE ");
          directionsCw.push_back(SE);
          directionsCcw.push_back(NE);
          x++;
        } else {
          D(cerr << "S ");
          directionsCw.push_back(S);
          directionsCcw.push_back(N);
        }
        D(cerr << endl);
      }
      
      // bottom
      y++;
      D(cerr << "y=" << y << ", x=" << x << ", e=" << e << " => ");
      while (x < e) {
        D(cerr << "E ");
        directionsCw.push_back(E);
        directionsCcw.push_back(E);
        x++;
      }
      D(cerr << endl);
      
      // Quadrant SE
      for (y++; y <= 0; y++) {
        e = getExtent(r_, extents_, y);
        D(cerr << "y=" << y << ", x=" << x << ", e=" << e << " => ");
        if (x < e) {
          D(cerr << "NE ");
          directionsCw.push_back(NE);
          directionsCcw.push_back(SE);
          x++;
          while (x < e) {
            D(cerr << "E ");
            directionsCw.push_back(E);
            directionsCcw.push_back(E);
            x++;
          }
        } else {
          D(cerr << "N ");
          directionsCw.push_back(N);
          directionsCcw.push_back(S);
        }
        D(cerr << endl);
      }
      
      offsetsCw_ = new vector<Point>(directionsCw.size() + 1);
      makeOffsets(offsetsCw_->begin(), Point(r_, 0), directionsCw.begin(), directionsCw.end());
      
      offsetsCcw_ = new vector<Point>(directionsCw.size() + 1);
      makeOffsets(offsetsCcw_->begin(), Point(r_, 0), directionsCcw.begin(), directionsCcw.end());
      
#if DEBUG
      cerr << "CW Directions: " << directionsCw << endl;
      cerr << "CW Offsets: " << *offsetsCw_ << endl;
      cerr << "CCW Directions: " << directionsCcw << endl;
      cerr << "CCW Offsets: " << *offsetsCcw_ << endl << flush;
#endif
      
      directionsCw_ = new vector<int>();
      directionsCw_->reserve(directionsCw.size() * 2);
      copy(directionsCw.begin(), directionsCw.end(), back_inserter(*directionsCw_));
      copy(directionsCw.begin(), directionsCw.end(), back_inserter(*directionsCw_));
      
      directionsCcw_ = new vector<int>();
      directionsCcw_->reserve(directionsCw.size() * 2);
      copy(directionsCcw.begin(), directionsCcw.end(), back_inserter(*directionsCcw_));
      copy(directionsCcw.begin(), directionsCcw.end(), back_inserter(*directionsCcw_));
      
#if DEBUG
      cerr << "CW Directions: " << *directionsCw_ << endl;
      cerr << "CW Offsets: " << *offsetsCw_ << endl;
      cerr << "CCW Directions: " << *directionsCcw_ << endl;
      cerr << "CCW Offsets: " << *offsetsCcw_ << endl;
#endif
    }
  }
  
  template<typename I, typename J> static void makeDirections(J j, const J &jEnd, I i) {
    if (j == jEnd) return;
    
    Point p = *j++;
    while (j != jEnd) {
      Point q = *j++;
      *i++ = p.directionTo(q);
      p = q;
    }
  }
  
  template<typename Result, typename Source> void findOffsets(Source source, const Source &sourceEnd, Result result) {
    if (source == sourceEnd) return;
    
    vector<int> directions;
    makeDirections(source, sourceEnd, back_inserter(directions));
    
    ensureDirectionsAndOffsets();
    
    for (int i = 0; i <= offsetsCw_->size(); i++) {
      bool match = true;
      for (int j = 0; match && j < directions.size(); j++) {
        match = ((*directionsCw_)[i + j] == directions[j]);
      }
      if (match) {
        *result++ = *source - (*offsetsCw_)[i];
      }
      
      match = true;
      for (int j = 0; match && j < directions.size(); j++) {
        match = ((*directionsCcw_)[i + j] == directions[j]);
      }
      if (match) {
        *result++ = *source - (*offsetsCcw_)[i];
      }
    }
  }
  
  template<typename Source, typename Result, typename Check> void filterOffsets(Source source, const Source &sourceEnd, Check &check, Result result) {
    for (; source != sourceEnd; ++source) {
      Point p = *source;
      bool allOk = true;
      for (list<Point>::const_iterator q = points().begin(); allOk && q != points().end(); ++q) {
        allOk = check(p + *q);
      }
      if (allOk) {
        *result++ = p;
      }
    }
  }
  
  template<typename Result, typename Check> bool addFiltered(const Point &p, Check &check, Result &result) {
    bool allOk = true;
    for (list<Point>::const_iterator q = points().begin(); allOk && q != points().end(); ++q) {
      allOk = check(p + *q);
    }
    if (allOk) {
      *result++ = p;
      return true;
    }
    return false;
  }
  
  template<typename Dir, typename Result, typename Check> int _findFilteredOffsets(const vector<int> *directions, const vector<Point> *offsets, const Point &p, const Dir &dirStart, const Dir &dirEnd, Check &check, Result result) {
    int added = 0;
    for (int i = 0; i < offsets->size(); i++) {
      bool match = true;
      Dir dir = dirStart;
      for (int j = 0; match && dir != dirEnd; j++) {
        match = ((*directions)[i + j] == *dir++);
      }
      if (match && addFiltered(p - (*offsets)[i], check, result)) {
        ++added;
      }
    }
    return added;
  }
  
  template<typename Dir, typename Result, typename Check> int _findFilteredOffsets(const Point &p, const Dir &dirStart, const Dir &dirEnd, Check &check, Result result) {
    return _findFilteredOffsets(directionsCw_, offsetsCw_, p, dirStart, dirEnd, check, result)
        + _findFilteredOffsets(directionsCcw_, offsetsCcw_, p, dirStart, dirEnd, check, result);
  }
  
  template<typename Source, typename Result, typename Check> int findFilteredOffsets(Source source, const Source &sourceEnd, Check &check, Result result) {
    if (source == sourceEnd) return 0;
    
    vector<int> directions;
    makeDirections(source, sourceEnd, back_inserter(directions));
    
    ensureDirectionsAndOffsets();
    
    int added = _findFilteredOffsets(*source, directions.begin(), directions.end(), check, result);
    
    if (added == 0 && directions.size() > 0) {
      vector<int>::const_iterator dirEnd = directions.end();
      for (Source s = source; s != sourceEnd; ++s) {
        added += _findFilteredOffsets(*s, dirEnd, dirEnd, check, result);
      }
    }
    return added;
  }
  
  template<typename Set> void setAt(const Point &p, Set s) const {
    for (list<Point>::const_iterator i = points().begin(); i != points().end(); ++i) {
      s(p + *i);
    }
  }
  
  template<typename I, typename Set> void setAll(I i, const I &iEnd, Set s) const {
    for (; i != iEnd; ++i) {
      setAt(*i, s);
    }
  }
};

#if 0
{
#endif
}


#endif // __Circle_h__
