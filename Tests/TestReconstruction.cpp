/*
 *  TestReconstruction.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 01/05/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestReconstruction.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Line.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"

using namespace Primitives;
using namespace Images;

static void setAll(Bitmap &bitmap, const Circle &circle, const IPoint &p0) {
  for (list<IPoint>::const_iterator i = circle.points().begin();
       i != circle.points().end();
       ++i) {
    bitmap.at(p0 + *i) = true;
  }
}

TEST_CASE("reconstruct", "reconstruct: some should stay and some should go")
{
  shared_ptr<Bitmap> bitmap(new Bitmap(70, 70));

  Circle c9(9);
  Circle c7(7);
  Circle c5(5);

  bitmap->clear();

  setAll(*bitmap, c5, IPoint(10, 60));
  setAll(*bitmap, c9, IPoint(55, 15));
  setAll(*bitmap, c7, IPoint(9, 19));

  cerr << endl << "Original:" << endl << *bitmap << endl;
  shared_ptr<Bitmap> inset(bitmap->inset(7));
  cerr << endl << "Inset:" << endl << *inset << endl;
  shared_ptr<Bitmap> reconstructed(inset->reconstruct(bitmap));
  cerr << endl << "Reconstructed:" << endl << *reconstructed << endl;
}
