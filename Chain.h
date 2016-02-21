/*
 *  Chain.h
 *  Morph
 *
 *  Created by Christian Brunschen on 13/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Chain_h__
#define __Chain_h__

#include <list>
#include "Primitives.h"
#include "Line.h"

namespace Chaining {
#if 0
}
#endif

#ifdef D
#undef D
#endif

#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG 0

#if DEBUG
#define D(x) do { x; } while(0)
#else // DEBUG
#define D(x)
#endif // DEBUG


using namespace Primitives;

class Chain : public list<IPoint> {
public:
  struct PointAdder {
    Chain &chain_;
    PointAdder(Chain &chain) : chain_(chain) { }
    void operator()(int x, int y) {
      chain_.push_back(IPoint(x, y));
    }
    void operator()(const IPoint &p) const {
      chain_.push_back(p);
    }
  };

  PointAdder pointAdder() {
    return PointAdder(*this);
  }

  void addPoint(int x, int y) {
    push_back(IPoint(x, y));
  }

  void addPoint(const IPoint &p) {
    push_back(p);
  }

  void simplify(const iterator &first, const iterator &last, double tolerance) {
    const IPoint &p0 = *first;
    const IPoint &p1 = *last;
    D(cerr << "Simplifying from " << p0 << " to " << p1 << endl);
    long dx = p1.x() - p0.x();
    long dy = p1.y() - p0.y();
    long dxyyx = p0.x() * p1.y() - p0.y() * p1.x();
    long squareLength = dx*dx + dy*dy;
    double length = sqrt((double) squareLength);
    long maxArea = 0;
    iterator maxPointIter = first;
    iterator second = first; ++second;
    for (iterator iter = second; iter != last; ++iter) {
      const IPoint &p = *iter;
      long area = p.y() * dx - p.x() * dy + dxyyx;

      D(cerr << "@ " << p << ", area = " << area << endl);
      long absArea = area < 0 ? -area : area;
      if (absArea > maxArea) {
        D(cerr << "found absArea = " << absArea << endl);
        maxArea = absArea;
        maxPointIter = iter;
      }
    }
    D(cerr << "maxArea = " << maxArea << endl);
    if (maxArea != 0
        && ((double) maxArea / length) >= tolerance) {
      simplify(first, maxPointIter, tolerance);
      simplify(maxPointIter, last, tolerance);
    } else if (second != last) {
      erase(second, last);
    }
  }

  void simplify(double tolerance = 0.3) {
    if (front() == back()) {
      if (size() > 5) {
        iterator iter = begin();
        iterator last = end();
        --last;
        const IPoint &p0 = *iter;
        long maxSquareDistance = 0;
        iterator maxPointIter = iter;
        for (++iter; iter != last; ++iter) {
          const IPoint &p = *iter;
          long dx = p.x() - p0.x();
          long dy = p.y() - p0.y();
          long squareDistance = dx*dx + dy*dy;
          D(cerr << "@ " << p << ", squareDistance = " << squareDistance << endl);
          if (squareDistance > maxSquareDistance) {
            maxSquareDistance = squareDistance;
            maxPointIter = iter;
          }
        }
        D(cerr << "maxSquareDistance = " << maxSquareDistance << " at " << *maxPointIter << endl);
        simplify(begin(), maxPointIter, tolerance);
        simplify(maxPointIter, last, tolerance);
      } else {
        // <= 5 points & closed => it's a rectangle; don't try to simplify.
      }
    } else if (size() > 3) {
      iterator last = end();
      --last;
      simplify(begin(), last, tolerance);
    } else {
      // <= 3 points, don't simplify
    }
  }

  void transform(const IMatrix &m) {
    for (iterator i = begin(); i != end(); ++i) {
      i->transform(m);
    }
  }

  template<typename D> void transform(const IMatrix &m, D d) {
    for (iterator i = begin(); i != end(); ++i) {
      *d++ = i->transformed(m);
    }
  }

  Chain transformed(const IMatrix &m) const {
    Chain result;
    for (const_iterator i = begin(); i != end(); ++i) {
      result.push_back(i->transformed(m));
    }
    return result;
  }

  double minDistance(const Chain &next) {
    if (size() == 1) {
      if (next.size() == 1) {
        // just two points
        // cerr << " ++ checking distance from " << back() << " to " << next.front() << endl;
        return back().distance(next.front());
      } else {
        // one point vs a line segment
        const IPoint &p = back();
        Chain::const_iterator i = next.begin();
        const IPoint &q0 = *i++;
        const IPoint &q1 = *i;
        // cerr << " ++ checking distance from " << p << " to [" << q0 << "->" << q1 << "]" << endl;
        return p.distance(q0, q1);
      }
    } else {
      if (next.size() == 1) {
        // a line segment vs one point
        Chain::const_reverse_iterator i = rbegin();
        const IPoint &p0 = *i++;
        const IPoint &p1 = *i;
        const IPoint &q = next.front();
        // cerr << " ++ checking distance from [" << p0 << "->" << p1 << "] to " << q << endl;
        return q.distance(p0, p1);
      } else {
        // two line segments
        Chain::const_reverse_iterator i = rbegin();
        const IPoint &p0 = *i++;
        const IPoint &p1 = *i;
        Chain::const_iterator j = next.begin();
        const IPoint &q0 = *j++;
        const IPoint &q1 = *j;
        // cerr << " ++ checking distance from [" << p0 << "->" << p1 << "] to [" << q0 << "->" << q1 << "]" << endl;
        return min(p0.distance(q0, q1), q0.distance(p0, p1));
      }
    }
  }

  bool allPointsAreNeighbours() {
    if (size() <= 1) return true;
    const_iterator i = begin();
    const_iterator j = i;
    for (++j; j != end(); i = j++) {
      if (!j->isNeighbour(*i)) return false;
    }
    return true;
  }
};

class Chains : public list<Chain> {
public:
  Chain &addChain() {
    push_back(Chain());
    return back();
  }

  Chain &lastChain() {
    return back();
  }

  Chain &prependChain() {
    push_front(Chain());
    return front();
  }

  Chain &firstChain() {
    return front();
  }

  void moveChains(Chains &chains) {
    splice(end(), chains);
  }

  void addChains(Chains &chains) {
    for (Chains::iterator iter = chains.begin(); iter != chains.end(); ++iter) {
      Chain &src = *iter;
      Chain &dst = addChain();
      for (Chain::iterator cIter = src.begin(); cIter != src.end(); ++cIter) {
        dst.addPoint(*cIter);
      }
    }
  }

  void simplify(double tolerance = 0.3) {
    for (iterator iter = begin(); iter != end(); ++iter) {
      iter->simplify(tolerance);
    }
  }

  template<typename F> void reconnect(double maxDistance, F f) {
    iterator a = begin();
    iterator b = a; ++b;
    while (b != end()) {
      double d = a->minDistance(*b);
      if (d <= maxDistance) {
        // cerr << " ** distance " << d << " < " << maxDistance << ";" << endl;
        const IPoint &p = a->back();
        const IPoint &q = b->front();
        Chain connectingLine;
        Chain::PointAdder adder = connectingLine.pointAdder();
        line(p.x(), p.y(), q.x(), q.y(), adder);
        bool allPresent = true;
        for (Chain::iterator i = connectingLine.begin();
             allPresent && i != connectingLine.end();
             ++i) {
          allPresent = f(*i);
        }
        if (allPresent) {
          a->insert(a->end(), b->begin(), b->end());
          b = erase(b);
          continue;
        }
      }
      a = b;
      ++b;
    }
  }

  void transform(const IMatrix &m) {
    for (iterator iter = begin(); iter != end(); ++iter) {
      iter->transform(m);
    }
  }

  Chains transformed(const IMatrix &m) const {
    Chains result;
    for (const_iterator i = begin(); i != end(); ++i) {
      result.push_back(i->transformed(m));
    }
    return result;
  }
};

class Boundary : public Chains {
private:
  Boundary *parent_;
  list<Boundary *>children_;

public:
  Boundary() : parent_(NULL) { Chains(); }
  Boundary * &parent() { return parent_; }
  list<Boundary *> &children() { return children_; }
  void addChild(Boundary *child) {
    children_.push_back(child);
  }
  void close(bool connect = false) {
    if (size() == 1) {
      const IPoint &p = front().front();
      const IPoint &q = back().back();
      if (connect && p.isNeighbour(q)) {
        back().push_back(p);
      }
    } else if (size() > 1) {
      if (front().front().isNeighbour(back().back())) {
        front().splice(front().begin(), back());
        pop_back();
      } else if (front().front().isNeighbour(back().front())) {
        back().reverse();
        front().splice(front().begin(), back());
        pop_back();
      }

      iterator cur = begin(), next = begin();
      ++next;
      while (next != end()) {
        if (cur->front().isNeighbour(next->front())) {
          cur->reverse();
          next->splice(next->begin(), *cur);
          erase(cur);
        } else if (cur->front().isNeighbour(next->back())) {
          next->splice(next->end(), *cur);
          erase(cur);
        }
        cur = next;
        ++next;
      }
    }
  }
};

inline ostream &operator<<(ostream &out, const Chain &chain) {
  bool first = true;
  out << "[";
  for (Chain::const_iterator i = chain.begin();
       i != chain.end();
       ++i) {
    if (first) {
      first = false;
    } else {
      out << ", ";
    }
    out << *i;
  }
  out << "]";
  return out;
}

inline ostream &operator<<(ostream &out, const Chains &chains) {
  bool first = true;
  out << "[";
  for (Chains::const_iterator i = chains.begin();
       i != chains.end();
       ++i) {
    if (first) {
      first = false;
    } else {
      out << ", ";
    }
    out << *i;
  }
  out << "]";
  return out;
}

#undef D

#if 0
{
#endif
}

#endif // __Chain_h__
