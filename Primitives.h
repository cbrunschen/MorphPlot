/*
 *  Primitives.h
 *  Morph
 *
 *  Created by Christian Brunschen on 06/11/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __Primitives_h__
#define __Primitives_h__

#define nColorComponents 3

#include "FnvHash.h"

#include <stdint.h>
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <list>
#include <set>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#if DEBUG
#define D(x) do { x; } while(0)
#else // DEBUG
#define D(x)
#endif // DEBUG


#ifndef UINT8_MAX
#define UINT8_MAX (255)
#endif

using namespace std;

namespace Primitives {
#if 0
}
#endif

template<typename C, typename P> class PointHasher;

template <typename T> inline void writeToFile(const char * filename, const T &t) {
  ofstream s(filename);
  s << t;
  s.close();
}

template<typename C> inline double density(const C &comp, const C &pen) {
  double result = pen == 0 ? 2.0 : (double)comp / (double)pen;
  D(cerr << "density(" << (int)comp << "," << (int)pen << ") = " << result << endl << flush);
  return result;
}

namespace Neighbourhood {
  enum Direction {
    NW = 0,
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    DIRECTIONS,
  };

  enum Turn {
    CW = 1,
    CCW = DIRECTIONS - 1,
  };

  // offsets for pixels in the neighbourhood
  extern int xOffsets[DIRECTIONS+1];
  extern int yOffsets[DIRECTIONS+1];

  // lists those neighbourhoods that are considered single-connected
  extern bool singleConnected[];

  // lists those neighbourhoods that are thin-connected: one or two neighbours,
  // as might be found in a single-pixel-thin skeleton.
  extern bool thinConnected[];

  inline Direction turn(Direction dir, Turn delta) {
    return static_cast<Direction>((dir + delta) % DIRECTIONS);
  }
  inline Direction opposite(Direction dir) {
    return Direction((dir + DIRECTIONS/2) % DIRECTIONS);
  }
  inline Direction turnOrStop(Direction dir, int delta) {
    int d = dir + delta;
    return 0 <= d && d < DIRECTIONS ? static_cast<Direction>(d) : DIRECTIONS;
  }
  inline bool isSingleConnected(int n) {
    return singleConnected[n];
  }
  inline bool isDiagonal(const int &dir) {
    return (dir & 1) == 0;
  }
}

using namespace Neighbourhood;

template<typename C, typename M>
class Matrix {
  C a_, b_, c_, d_, tx_, ty_;

public:
  Matrix(C a = 1, C b = 0, C c = 0, C d = 1, C tx = 0, C ty = 0)
      : a_(a), b_(b), c_(c), d_(d), tx_(tx), ty_(ty) { }
  const C a() const { return a_; }
  const C b() const { return b_; }
  const C c() const { return c_; }
  const C d() const { return d_; }
  const C tx() const { return tx_; }
  const C ty() const { return ty_; }

  bool operator==(const M &m) const {
    return a_ == m.a_
        && b_ == m.b_
        && c_ == m.c_
        && d_ == m.d_
        && tx_ == m.tx_
        && ty_ == m.ty_;
  }
  bool operator!=(const M &m) const {
    return !operator==(m);
  }

  void transform(C &outX, C &outY, const C &x, const C &y) const {
    outX = x * a_ + y * c_ + tx_;
    outY = x * b_ + y * d_ + ty_;
  }

  M concat(const M &m) const {
    return M(m.a() * a() + m.b() * c(), m.a() * b() + m.b() * d(),
                  m.c() * a() + m.d() * c(), m.c() * b() + m.d() * d(),
                  m.tx() * a() + m.ty() * c() + tx(),
                  m.tx() * b() + m.ty() * d() + ty());
  }

  template<typename D, typename N> bool operator==(const Matrix<D, N> &m) const {
    return a_ == m.a_
    && b_ == m.b_
    && c_ == m.c_
    && d_ == m.d_
    && tx_ == m.tx_
    && ty_ == m.ty_;
  }
  template<typename D, typename N> bool operator!=(const Matrix<D, N> &m) const {
    return !operator==(m);
  }

  template<typename D> void transform(D &outX, D &outY, const D &x, const D &y) const {
    outX = x * a_ + y * c_ + tx_;
    outY = x * b_ + y * d_ + ty_;
  }

  template<typename D, typename N> M concat(const Matrix<D, M> &m) const {
    return IMatrix(m.a() * a() + m.b() * c(), m.a() * b() + m.b() * d(),
                  m.c() * a() + m.d() * c(), m.c() * b() + m.d() * d(),
                  m.tx() * a() + m.ty() * c() + tx(),
                  m.tx() * b() + m.ty() * d() + ty());
  }

  static M identity() { return M(1, 0, 0, 1, 0, 0); }
  static M rotateLeft() { return M(0, 1, -1, 0, 0, 0); }
  static M rotateRight() { return M(0, -1, 1, 0, 0, 0); }
  static M upsideDown() { return M(-1, 0, 0, -1, 0, 0); }
  static M flipX() { return M(-1, 0, 0, 1, 0, 0); }
  static M flipY() { return M(1, 0, 0, -1, 0, 0); }
  static M translate(int tx, int ty) { return M(1, 0, 0, 1, tx, ty); }

  static M flipXY() { return M(0, 1, 1, 0, 0, 0); }

  static M pageLeft(int width, int height) {
    return M(0, 1, -1, 0, height-1, 0);
  }
  static M pageRight(int width, int height) {
    return M(0, -1, 1, 0, 0, width-1);
  }
  static M pageUpsideDown(int width, int height) {
    return M(-1, 0, 0, -1, width-1, height-1);
  }
  static M flipLeftRight(int width, int height) {
    return M(-1, 0, 0, 1, width-1, 0);
  }
  static M flipTopBottom(int width, int height) {
    return M(1, 0, 0, -1, 0, height-1);
  }
};

class IMatrix : public Matrix<int, IMatrix> {
public:
  typedef Matrix<int, IMatrix> Super;
  IMatrix(int a = 1, int b = 0, int c = 0, int d = 1, int tx = 0, int ty = 0) : Super(a, b, c, d, tx, ty) { }
};

class FMatrix : public Matrix<double, FMatrix> {
public:
  typedef Matrix<double, FMatrix> Super;
  FMatrix(double a = 1, double b = 0, double c = 0, double d = 1, double tx = 0, double ty = 0) : Super(a, b, c, d, tx, ty) { }
};

template<typename C, typename M>
inline ostream &operator<<(ostream &out, const Matrix<C, M> &m) {
  out << "[ " << m.a() << " " << m.b() << " " << m.c() << " " << m.d() << " " << m.tx() << " " << m.ty() << " ]";
  return out;
}

template<typename C, typename P> class Point {
protected:
  C x_;
  C y_;
  friend ostream &operator<<(ostream &out, const Point);
  friend struct std::hash<Point>;
  friend class PointHasher<int, Point>;
public:
  Point() { }  // leave contents uninitialized.
  Point(C x, C y) : x_(x), y_(y) { }
  Point(const Point &p) : x_(p.x_), y_(p.y_) { }
  C &x() { return x_; }
  C &y() { return y_; }
  const C &x() const { return x_; }
  const C &y() const { return y_; }
  Point &operator=(const Point &p) {
    x_ = p.x_;
    y_ = p.y_;
    return *this;
  }
  const bool operator==(const Point &p) const {
    return p.x_ == x_ && p.y_ == y_;
  }
  const bool operator!=(const Point &p) const {
    return p.x_ != x_ || p.y_ != y_;
  }
  const bool operator<(const Point &p) const {
    if (x_ < p.x_) return true;
    if (p.x_ < x_) return false;
    return y_ < p.y_;
  }
  const bool operator>(const Point &p) const {
    if (x_ > p.x_) return true;
    if (p.x_ > x_) return false;
    return y_ > p.y_;
  }
  P operator+(const Point &p) const {
    return P(x_ + p.x_, y_ + p.y_);
  }
  P &operator+=(const Point &p) {
    x_ += p.x_;
    y_ += p.y_;
    return *this;
  }
  P operator-(const Point &p) const {
    return P(x_ - p.x_, y_ - p.y_);
  }
  P &operator-=(const Point &p) {
    x_ -= p.x_;
    y_ -= p.y_;
    return *this;
  }
  P offset(C dx, C dy) {
    return P(x_ + dx, y_ + dy);
  }
  P offset(C dx, C dy) const {
    return P(x_ + dx, y_ + dy);
  }

  template<typename D, typename Q> Point operator=(const Point<D, Q> &p) {
    x_ = (C) p.x_;
    y_ = (C) p.y_;
    return *this;
  }
  template<typename D, typename Q> const bool operator==(const Point<D, Q> &p) const {
    return p.x_ == x_ && p.y_ == y_;
  }
  template<typename D, typename Q> const bool operator!=(const Point<D, Q> &p) const {
    return p.x_ != x_ || p.y_ != y_;
  }
  template<typename D, typename Q> const bool operator<(const Point<D, Q> &p) const {
    if (x_ < p.x_) return true;
    if (p.x_ < x_) return false;
    return y_ < p.y_;
  }
  template<typename D, typename Q> const bool operator>(const Point<D, Q> &p) const {
    if (x_ > p.x_) return true;
    if (p.x_ > x_) return false;
    return y_ > p.y_;
  }
  template<typename D, typename Q> P operator+(const Point<D, Q> &p) const {
    return Point(x_ + p.x_, y_ + p.y_);
  }
  template<typename D, typename Q> P &operator+=(const Point<D, Q> &p) {
    x_ += p.x_;
    y_ += p.y_;
    return *this;
  }
  template<typename D, typename Q> P operator-(const Point<D, Q> &p) const {
    return Point(x_ - p.x_, y_ - p.y_);
  }
  template<typename D, typename Q> P &operator-=(const Point<D, Q> &p) {
    x_ -= p.x_;
    y_ -= p.y_;
    return *this;
  }
  template<typename D> P offset(D dx, D dy) {
    return P(x_ + dx, y_ + dy);
  }
  template<typename D> P offset(D dx, D dy) const {
    return P(x_ + dx, y_ + dy);
  }

  static int sign(C n) {
    return n == 0 ? 0 : n < 0 ? -1 : 1;
  }

  template<typename D, typename Q> const double squareDistance(const Point<D, Q> &p) const {
    double dx = p.x_ - x_;
    double dy = p.y_ - y_;
    return dx*dx + dy*dy;
  }
  const double squareDistance(const double x, const double y) const {
    double dx = x - x_;
    double dy = y - y_;
    return dx*dx + dy*dy;
  }
  template<typename D, typename Q> const double distance(const Point<D, Q> &p) const {
    return sqrt(squareDistance(p));
  }
  const double distance(const double x, const double y) const {
    return sqrt(squareDistance(x, y));
  }
  // calculates the squared distance from |this| to the segment (|p| to |q|)
  template<typename D, typename Q>
  const double squareDistance(const Point<D, Q> &p, const Point<D, Q> &q) const {
    double pqx = q.x_ - p.x_;
    double pqy = q.y_ - p.y_;
    double pqlen = sqrt(pqx*pqx + pqy+pqy);
    double pqnx = pqx / pqlen;
    double pqny = pqy / pqlen;
    double dx = x_ - p.x_;
    double dy = y_ - p.y_;
    double t = pqnx * dx + pqny * dy;
    if (t < 0.0) {
      return squareDistance(p);
    } else if (t <= pqlen) {
      double rx = p.x_ + t * pqnx;
      double ry = p.y_ + t * pqny;
      double drx = x_ - rx;
      double dry = y_ - ry;
      return drx*drx + dry*dry;
    } else {
      return squareDistance(q);
    }
  }
  // calculates the distance from |this| to the segment (|p| to |q|)
  template<typename D, typename Q> const double distance(const Point<D, Q> &p, const Point<D, Q> &q) const {
    return sqrt(squareDistance(p, q));
  }

};

class IPoint : public Point<int, IPoint> {
public:
  typedef Point<int, IPoint> Super;
  friend struct std::hash<IPoint>;

  IPoint(int x = 0, int y = 0) : Super(x, y) { }

  const IPoint neighbour(const int &dir) const {
    return IPoint(x_ + xOffsets[dir], y_ + yOffsets[dir]);
  }
  const bool isNeighbour(const IPoint &p) const {
    if (p == *this) {
      return false;
    } else {
      int dx = p.x_ - x_;
      int dy = p.y_ - y_;
      return -1 <= dx && dx <= 1 && -1 <= dy && dy <= 1;
    }
  }

  const int directionTo(const IPoint &p) const {
    int dx = sign(p.x_ - x_);
    int dy = sign(p.y_ - y_);
    if (dy < 0) {
      return Neighbourhood::N + dx;
    }
    if (dy > 0) {
      return Neighbourhood::S - dx;
    }
    if (dx < 0) return Neighbourhood::W;
    if (dx > 0) return Neighbourhood::E;
    return DIRECTIONS;
  }

  IPoint &transform(const IMatrix &m) {
    int x, y;
    m.transform(x, y, x_, y_);
    x_ = x;
    y_ = y;
    return *this;
  }

  IPoint transformed(const IMatrix &m) const {
    int x, y;
    m.transform(x, y, x_, y_);
    return IPoint(x, y);
  }
};

class FPoint : public Point<double, FPoint> {
public:
  typedef Point<double, FPoint> Super;
  friend struct std::hash<FPoint>;

  FPoint(double x = 0, double y = 0) : Super(x, y) { }

  FPoint &transform(const IMatrix &m) {
    double x, y;
    m.transform(x, y, x_, y_);
    x_ = x;
    y_ = y;
    return *this;
  }

  FPoint transformed(const IMatrix &m) const {
    double x, y;
    m.transform(x, y, x_, y_);
    return FPoint(x, y);
  }
};

template<typename C, typename P> class PointHasher : public unary_function<Point<C, P>, size_t> {
public:
  size_t operator() (const Point<C, P> &h) const {
    return continue_hash<int>::hash(h.x_, continue_hash<int>::hash(h.y_));
  }
};

template<typename C, typename P>
inline ostream &operator<<(ostream &out, const Point<C, P> &p) {
  out << "(" << p.x() << "," << p.y() << ")";
  return out;
}

inline ostream &operator<<(ostream &out, const IPoint &p) {
  out << "(" << p.x() << "," << p.y() << ")";
  return out;
}

struct Hex {
  static const char hexchars[];
  static int hex(char c) {
    const char *t = index(hexchars, tolower(c));
    if (t) return static_cast<int>(t-hexchars);
    return 0;
  }
};

template<typename F> struct PH {
  const F &f_;
  PH(const F &f) : f_(f) { }
};

template <typename F> inline ostream &operator<<(ostream &out, const PH<F> &ph) {
  ios_base::fmtflags flags(out.flags());
  out << hex << ph.f_;
  out.flags(flags);
  return out;
}

template <> inline ostream &operator<<(ostream &out, const PH<uint8_t> &ph) {
  ios_base::fmtflags flags(out.flags());
  out << hex << (int) ph.f_;
  out.flags(flags);
  return out;
}

struct PP : public PH<uintptr_t> {
  PP(void *p) : PH<uintptr_t>(uintptr_t(p)) { }
};

template <typename T> class Component {
public:
  static const T min() { return 0; }
  static const T max() { return 0; }
  static const int bits() { return 0; }
  static const T parse(const char *s) { return 0; }
  static const T parse(const string &s) { return 0; }
  static void unparse(string &c, const T &v) { }
  static ostream &unparse(ostream &out, const T &v) { return out; }
  static const T make(const double &f) {
    return f <= 0.0 ? min() :
           f >= 1.0 ? max() :
           static_cast<T>(min() + ((max() - min()) * f));
  }
  static const T make(const double &f, const T &c) {
    return f <= 0.0 ? min() :
           f >= 1.0 ? c :
           static_cast<T>(min() + ((c - min()) * f));
  }
  static const T inverse(const T &t) {
    return max() - (t - min());
  }
  static double fraction(const T &t) {
    return (double)(t - min()) / (double)(max() - min());
  }
  static ostream &print(ostream &out, const T &t) {
    return (out << t);
  }
};

template <> class Component<uint8_t> {
public:
  static const uint8_t min() { return 0; }
  static const uint8_t max() { return UINT8_MAX; }
  static const int bits() { return 8; }
  static const uint8_t parse(const char *s) {
    return 16 * Hex::hex(s[0]) + Hex::hex(s[1]);
  }
  static const uint8_t parse(const string &s) {
    return 16 * Hex::hex(s[0]) + Hex::hex(s[1]);
  }
  static void unparse(string &s, uint8_t v) {
    s += Hex::hexchars[(v >> 4) & 0xf];
    s += Hex::hexchars[(v     ) & 0xf];
  }
  static ostream &unparse(ostream &out, uint8_t v) {
    out << Hex::hexchars[(v >> 4) & 0xf] << Hex::hexchars[(v     ) & 0xf];
    return out;
  }
  static const uint8_t make(const double &f) {
    return f <= 0.0 ? 0 :
           f >= 1.0 ? UINT8_MAX :
           rint(UINT8_MAX * f);
  }
  static const uint8_t make(const double &f, const uint8_t &c) {
    return f <= 0.0 ? 0 :
           f >= 1.0 ? c :
           rint(c * f);
  }
  static const uint8_t inverse(const uint8_t &c) {
    return UINT8_MAX - c;
  }
  static double fraction(const uint8_t &t) {
    return (double)t / (double)UINT8_MAX;
  }
  static ostream &print(ostream &out, const uint8_t &t) {
    out << (int)t;
    return out;
  }
};

template <typename C> class PenColor {
  C c_, m_, y_;
public:
  PenColor(C c, C m, C y) : c_(c), m_(m), y_(y) { }
  PenColor(double c, double m, double y) :
  c_(Component<C>::make(c)),
  m_(Component<C>::make(m)),
  y_(Component<C>::make(y))
  { }

  C const c() const { return c_; }
  C const m() const { return m_; }
  C const y() const { return y_; }

  double cFraction() const { return Component<C>::fraction(c_); }
  double mFraction() const { return Component<C>::fraction(m_); }
  double yFraction() const { return Component<C>::fraction(y_); }

  string unparse() const {
    string s;
    Component<C>::unparse(s, c_);
    Component<C>::unparse(s, m_);
    Component<C>::unparse(s, y_);
    return s;
  }

  PenColor &operator=(const PenColor<C> &other) {
    c_ = other.c_;
    m_ = other.m_;
    y_ = other.y_;
    return *this;
  }

  static PenColor parse(const string &s) {
    C c = Component<C>::parse(s.substr(0, 2));
    C m = Component<C>::parse(s.substr(2, 2));
    C y = Component<C>::parse(s.substr(4, 2));
    return PenColor(c, m, y);
  }
  static PenColor parse(const char * const s) {
    return parse(string(s));
  }

  bool operator=(const PenColor<C> &other) const {
    return c_ == other.c_ &&
        m_ == other.m_ &&
        y_ == other.y_;
  }

  bool operator<(const PenColor<C> &other) const {
    if (c_ < other.c_) return true;
    if (other.c_ < c_) return false;

    if (m_ < other.m_) return true;
    if (other.m_ < m_) return false;

    return y_ < other.y_;
  }

  bool operator>(const PenColor<C> &other) const {
    if (c_ > other.c_) return true;
    if (other.c_ > c_) return false;

    if (m_ > other.m_) return true;
    if (other.m_ > m_) return false;

    return y_ > other.y_;
  }

  template<typename D> bool operator=(const PenColor<D> &other) const {
    return cFraction() == other.cFraction() &&
        mFraction() == other.mFraction() &&
        yFraction() == other.yFraction();
  }

  template<typename D> bool operator<(const PenColor<D> &other) const {
    if (cFraction() < other.cFraction()) return true;
    if (other.cFraction() < cFraction()) return false;

    if (mFraction() < other.mFraction()) return true;
    if (other.mFraction() < mFraction()) return false;

    return yFraction() < other.yFraction();
  }

  template<typename D> bool operator>(const PenColor<D> &other) const {
    if (cFraction() > other.cFraction()) return true;
    if (other.cFraction() > cFraction()) return false;

    if (mFraction() > other.mFraction()) return true;
    if (other.mFraction() > mFraction()) return false;

    return yFraction() > other.yFraction();
  }
};

class PenSpec {
protected:
  // radius
  int r_;

  // radius of minimum feature that this pen will draw
  int rMin_;

  // the HPGL index of this pen
  int hpglIndex_;

  // the angle (in piradians, counterclockwise from the X axis) when using this pen to hatch
  double hatchAngle_;

  double hatchPhase_;
public:
  PenSpec(int r = 5) : r_(r), rMin_(-1) { }

  int &r() { return r_; }
  int const r() const { return r_; }

  double &hatchAngle() { return hatchAngle_; }
  double const hatchAngle() const { return hatchAngle_; }

  double &hatchPhase() { return hatchPhase_; }
  double const hatchPhase() const { return hatchPhase_; }

  virtual string unparse() const {
    stringstream ss;
    ss << r_ << flush;
    return ss.str();
  }

  static PenSpec parse(const string &s) {
    int r = 5;
    stringstream ss(s);
    ss >> r;
    return PenSpec(r);
  }
  static PenSpec parse(const char * const s) {
    return parse(string(s));
  }

  const bool operator<(const PenSpec &other) const {
    if (r_ < other.r_) return true;
    if (other.r_ < r_) return false;
    return rMin_ < other.rMin_;
  }

  int &rMin() { return rMin_; }
  int const rMin() const { return rMin_; }

  int &hpglIndex() { return hpglIndex_; }
  int const hpglIndex() const { return hpglIndex_; }

};

template <typename C> class Pen : public PenSpec {
  typedef Primitives::PenColor<C> PenColor;
  PenColor color_;
public:
  Pen(C c = 0, C m = 0, C y = 0, int r = 5) : PenSpec(r), color_(c, m, y) { }
  Pen(double c, double m, double y, int r = 5) : PenSpec(r), color_(c, m, y) { }
  PenColor &color() { return color_; }
  PenColor const &color() const { return color_; }
  C const c() const { return color_.c(); }
  C const m() const { return color_.m(); }
  C const y() const { return color_.y(); }
  double cFraction() const { return Component<C>::fraction(color_.c()); }
  double mFraction() const { return Component<C>::fraction(color_.m()); }
  double yFraction() const { return Component<C>::fraction(color_.y()); }

  virtual string unparse() const {
    stringstream ss;
    ss << color_.unparse() << "," << r_ << flush;
    return ss.str();
  }

  static Pen parse(const string &s) {
    int r = 5;
    PenColor color = PenColor::parse(s);
    size_t comma = s.find(',');
    if (comma != string::npos) {
      stringstream ss(s.substr(comma+1));
      ss >> r;
    }
    return Pen(color, r);
  }
  static Pen parse(const char * const s) {
    return parse(string(s));
  }

  const bool operator<(const Pen<C> &other) const {
    if (color_ < other.color_) return true;
    if (other.color_ < color_) return false;
    return PenSpec::operator<(other);
  }

  int &rMin() { return rMin_; }
  int const rMin() const { return rMin_; }

  int &hpglIndex() { return hpglIndex_; }
  int const hpglIndex() const { return hpglIndex_; }

};

/*
template<typename C> inline ostream &operator<<(ostream &out, const PenColor<C> &color) {
  out << color.c() << "/" << color.m() << "/" << color.y();
  return out;
}
 */

inline ostream &operator<<(ostream &out, const PenColor<uint8_t> &color) {
  out << PH<uint8_t>(color.c()) << "/" << PH<uint8_t>(color.m()) << "/" << PH<uint8_t>(color.y());
  return out;
}

template <typename T> inline ostream &operator<<(ostream &out, const vector<T> &points) {
  out << "[ ";
  bool first = true;
  for (typename vector<T>::const_iterator i = points.begin(); i != points.end(); ++i) {
    if (first) first = false; else out << ", ";
    out << *i;
  }
  out << " ]";
  return out;
}

template <typename T> inline ostream &operator<<(ostream &out, const list<T> &points) {
  out << "[ ";
  bool first = true;
  for (typename list<T>::const_iterator i = points.begin(); i != points.end(); ++i) {
    if (first) first = false; else out << ", ";
    out << *i;
  }
  out << " ]";
  return out;
}

template <typename T> inline ostream &operator<<(ostream &out, const set<T> &points) {
  out << "[{ ";
  bool first = true;
  for (typename set<T>::const_iterator i = points.begin(); i != points.end(); ++i) {
    if (first) first = false; else out << ", ";
    out << *i;
  }
  out << " }]";
  return out;
}

#if 0
{
#endif
}

namespace std {
#if 0
}
#endif

using namespace Primitives;
template <typename C, typename P>
struct hash<Point<C, P>>
{
  typedef Point<C, P>         argument_type;
  typedef std::size_t   result_type;

  result_type operator()(const Point<C, P> &h) const {
    return continue_hash<int>::hash(h.x_, continue_hash<int>::hash(h.y_));
  }
};

template <>
struct hash<IPoint>
{
  typedef IPoint         argument_type;
  typedef std::size_t   result_type;

  result_type operator()(const IPoint &h) const {
    return continue_hash<int>::hash(h.x_, continue_hash<int>::hash(h.y_));
  }
};

#if 0
{
#endif
}

#endif // __Primitives_h__
