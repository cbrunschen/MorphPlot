/*
 *  TestFilling.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 16/01/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestFilling.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "FloodFill.h"

using namespace Images;
using namespace Filling;

TEST_CASE("fill/4/simple", "simple 4-connected flood fill test")
{
  Bitmap bitmap(40, 40);
  Bitmap::Get isMarked = bitmap.get();
  Bitmap::Set mark = bitmap.set(true);

  bitmap.clear();
  bitmap.line( 4,  4,  4, 20, true);
  bitmap.line( 4, 20, 36, 20, true);
  bitmap.line(36, 20, 36, 36, true);
  bitmap.line(36, 36, 20, 36, true);
  bitmap.line(20, 36, 20,  4, true);
  bitmap.line(20,  4,  4,  4, true);

  cerr << "before fill-4:" << endl << bitmap << endl << flush;

  fill4(10, 10, 40, 40, isMarked, mark);

  cerr << "after fill-4:" << endl << bitmap << endl << flush;
}


TEST_CASE("fill/4/leak", "simple 4-connected flood fill test with a diaglonal leak possibility")
{
  Bitmap bitmap(40, 40);
  Bitmap::Get isMarked = bitmap.get();
  Bitmap::Set mark = bitmap.set(true);

  bitmap.clear();
  bitmap.line( 4,  4,  4, 20, true);
  bitmap.line( 4, 20, 36, 20, true);
  bitmap.line(36, 20, 36, 36, true);
  bitmap.line(36, 36, 20, 36, true);
  bitmap.line(20, 36, 20,  4, true);
  bitmap.line(20,  4,  4,  4, true);
  bitmap.set(20, 20, false);

  cerr << "before fill-4:" << endl << bitmap << endl << flush;

  fill4(10, 10, 40, 40, isMarked, mark);

  cerr << "after fill-4:" << endl << bitmap << endl << flush;
}

TEST_CASE("fill/8/leak", "simple 8-connected flood fill test with a leak between the two areas")
{
  Bitmap bitmap(40, 40);
  Bitmap::Get isMarked = bitmap.get();
  Bitmap::Set mark = bitmap.set(true);

  bitmap.clear();
  bitmap.line( 4,  4,  4, 20, true);
  bitmap.line( 4, 20, 36, 20, true);
  bitmap.line(36, 20, 36, 36, true);
  bitmap.line(36, 36, 20, 36, true);
  bitmap.line(20, 36, 20,  4, true);
  bitmap.line(20,  4,  4,  4, true);
  bitmap.set(20, 20, false);
  bitmap.set(19, 21, true);
  bitmap.set(21, 19, true);

  cerr << "before fill-8:" << endl << bitmap << endl << flush;

  fill8(10, 10, 40, 40, isMarked, mark);

  cerr << "after fill-8:" << endl << bitmap << endl << flush;
}

TEST_CASE("fill/8/sawtooth", "lots of things that need the 8-connectivity in all directions")
{
  Bitmap bitmap(9, 9);
  Bitmap::Get isMarked = bitmap.get();
  Bitmap::Set mark = bitmap.set(true);

  bitmap.clear();

  bitmap.line(0, 0, 0, 8, true);
  bitmap.line(0, 8, 8, 8, true);
  bitmap.line(8, 8, 8, 0, true);
  bitmap.line(8, 0, 0, 0, true);

  bitmap.line(3, 2, 3, 6, true);
  bitmap.line(2, 5, 6, 5, true);
  bitmap.line(5, 6, 5, 2, true);
  bitmap.line(6, 3, 2, 3, true);

  bitmap.set(2, 1, true);
  bitmap.set(4, 1, true);
  bitmap.set(6, 1, true);

  bitmap.set(2, 7, true);
  bitmap.set(4, 7, true);
  bitmap.set(6, 7, true);

  bitmap.set(1, 2, true);
  bitmap.set(1, 4, true);
  bitmap.set(1, 6, true);

  bitmap.set(7, 2, true);
  bitmap.set(7, 4, true);
  bitmap.set(7, 6, true);

  cerr << "before fill-8:" << endl << bitmap << endl << flush;

  fill8(1, 1, 10, 10, isMarked, mark);

  cerr << "after fill-8:" << endl << bitmap << endl << flush;
}

