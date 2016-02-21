/*
 *  TestBoundaries.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 23/01/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestBoundaries.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "Chain.h"

#include <iterator>
#include <list>
#include <set>
#include <utility>

#include <stdarg.h>

using namespace Primitives;
using namespace Chaining;
using namespace Images;
using namespace std;

static void verifyChain(const Chain &chain, int n, ...) {
  va_list ap;
  va_start(ap, n);
  Chain::const_iterator ci = chain.begin();
  for (int i = 0; i < n; i++) {
    REQUIRE(ci != chain.end());
    int x = va_arg(ap, int);
    int y = va_arg(ap, int);
    REQUIRE(ci->x() == x);
    REQUIRE(ci->y() == y);
    ++ci;
  }
  REQUIRE(ci == chain.end());
  va_end(ap);
}

TEST_CASE("boundary/square", "scan the boundary of a square")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  for (int y = 10; y < 30; y++) {
    for (int x = 10; x < 30; x++) {
      bitmap.at(y, x) = true;
    }
  }
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(10, 10), Neighbourhood::W, true);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 5, 10,10, 29,10, 29,29, 10,29, 10,10);
}

TEST_CASE("boundary/horizontal line", "scan the boundary of a horizontal line")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 10, 30, 10, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(10, 10), Neighbourhood::W, true);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 2, 10,10, 30,10);
}

TEST_CASE("boundary/vertical lineboundary/", "scan the boundary of a vertical line")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 10, 10, 30, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(10, 10), Neighbourhood::W, true);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 2, 10,10, 10, 30);
}

TEST_CASE("boundary/diagonal line", "scan the boundary of a diagonal line")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 10, 30, 30, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(10, 10), Neighbourhood::W, true);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 2, 10,10, 30, 30);
}

TEST_CASE("boundary/diagonal line, middle start", "scan the boundary of a diagonal line")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 10, 30, 30, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(20, 20), Neighbourhood::W, false);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 2, 30,30, 10, 10);
}

TEST_CASE("boundary/inverted V", "scan the boundary of an inverted V")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 20, 20, 10, true);
  bitmap.line(20, 10, 30, 20, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(20, 10), Neighbourhood::W, false);
  result.simplify(0.5);
  REQUIRE(result.size() == 1);
  verifyChain(result.firstChain(), 3, 10,20, 20,10, 30,20);
}

TEST_CASE("boundary/slanted F", "scan the boundary of a slanted F")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10, 20, 20, 10, true);
  bitmap.line(20, 10, 30, 20, true);
  bitmap.line(15, 15, 20, 20, true);
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  Boundary result;
  bitmap.scanBoundary(marks, 1, result, IPoint(20, 10), Neighbourhood::W, false);
  result.simplify(0.5);
  REQUIRE(result.size() == 2);

  Boundary::iterator i = result.begin();
  REQUIRE(i != result.end());
  verifyChain(*i, 4, 30,20, 20,10, 15,15, 20,20);
  ++i;
  verifyChain(*i, 2, 14,16, 10,20);
  ++i;
  REQUIRE(i == result.end());
}

// more than one boundary

ostream &operator<<(ostream &out, const vector<Boundary> &v) {
  for (vector<Boundary>::const_iterator i = v.begin(); i != v.end(); ++i) {
    out << *i << endl;
  }
  return out;
}

TEST_CASE("boundary/two nested squares", "two boundaries, child and parent")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  for (int d = 5; d <= 10; d++) {
    int p = 20 + d;
    int m = 20 - d;
    bitmap.line(m,m, m,p, true);
    bitmap.line(m,p, p,p, true);
    bitmap.line(p,p, p,m, true);
    bitmap.line(p,m, m,m, true);
  }
  cerr << bitmap << endl;
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  vector<Boundary> result;
  bitmap.scanBoundaries(result, marks, true);
  for (vector<Boundary>::iterator i = result.begin(); i != result.end(); ++i) {
    i->simplify(0.5);
    cerr << "Boundary @ " << uintptr_t(&(*i)) << ", parent " << uintptr_t(i->parent()) << ": " << *i << endl;
  }
}

TEST_CASE("boundary/D shape", "two boundaries, child and parent")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10,10, 10,30, true);
  for (int d = 5; d <= 10; d++) {
    int p = 20 + d;
    int m = 20 - d;
    bitmap.line(10,p, p,p, true);
    bitmap.line(p,p, p,m, true);
    bitmap.line(p,m, 10,m, true);
  }
  cerr << bitmap << endl;
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  vector<Boundary> result;
  bitmap.scanBoundaries(result, marks, true);
  for (vector<Boundary>::iterator i = result.begin(); i != result.end(); ++i) {
    i->simplify(0.5);
    cerr << "Boundary @ " << uintptr_t(&(*i)) << ", parent " << uintptr_t(i->parent()) << ": " << *i << endl;
  }
}

TEST_CASE("boundary/D shape/thin", "two boundaries, child and parent")
{
  Bitmap bitmap(40, 40);
  bitmap.clear();
  bitmap.line(10,10, 10,30, true);
  bitmap.line(10,10, 30,10, true);
  bitmap.line(10,30, 30,30, true);
  for (int x = 25; x <= 30; x++) {
    bitmap.line(x,10, x,30, true);
  }
  cerr << bitmap << endl;
  
  Image<int> marks(bitmap.width(), bitmap.height());
  marks.clear();
  vector<Boundary> result;
  bitmap.scanBoundaries(result, marks, true);
  for (vector<Boundary>::iterator i = result.begin(); i != result.end(); ++i) {
    i->simplify(0.5);
    cerr << "Boundary @ " << uintptr_t(&(*i)) << ", parent " << uintptr_t(i->parent()) << ": " << *i << endl;
  }
}
