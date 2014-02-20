/*
 *  TestSeparation.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 12/11/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestSeparation.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Line.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "GreyImage.h"
#include "GreyImage_Impl.h"
#include "ColorImage.h"

using namespace Primitives;
using namespace Images;


TEST_CASE("reconstruct", "reconstruct: some should stay and some should go")
{
  typedef uint8_t Component;
  typedef ColorImage<Component> ColorImage;
  typedef GreyImage<Component> GreyImage;
  typedef RGBPixel<Component> RGBPixel;
  typedef Pen<Component> Pen;
  
  Pen redPen(0.0, 1.0, 1.0);
  Pen bluePen(1.0, 1.0, 0.0);
  
  shared_ptr<ColorImage> colorImage(new ColorImage(4, 4));
  for (int y = 0; y < 4; y++) {
    int vy = y | (y << 2) | (y << 4) | (y << 6);
    for (int x = 0; x < 4; x++) {
      int vx = x | (x << 2) | (x << 4) | (x << 6);
      colorImage->at(y, x) = RGBPixel((Component) (255 - vy), 255, (Component) (255 - vx));
    }
  }
  
  shared_ptr<GreyImage> redSeparation = colorImage->separateAndSubtract(redPen);
  shared_ptr<GreyImage> blueSeparation = colorImage->separateAndSubtract(bluePen);

  for (int y = 0; y < 4; y++) {
    int vy = y | (y << 2) | (y << 4) | (y << 6);
    for (int x = 0; x < 4; x++) {
      int vx = x | (x << 2) | (x << 4) | (x << 6);
      colorImage->at(y, x) = RGBPixel((Component) (255 - vy), 255, (Component) (255 - vx));
    }
  }
  
  cout << "Red:" << endl;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      cout << " " << (int)redSeparation->at(y, x);
    }
    cout << endl;
  }  

  cout << "Blue:" << endl;
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      cout << " " << (int)blueSeparation->at(y, x);
    }
    cout << endl;
  }
  
  cout << flush;
  
  return;
  
  for (int y = 0; y < 4; y++) {
    int vy = y | (y << 2) | (y << 4) | (y << 6);
    for (int x = 0; x < 4; x++) {
      int vx = x | (x << 2) | (x << 4) | (x << 6);
      RGBPixel &p = colorImage->at(y, x);
      REQUIRE(p.r() == 255);
      REQUIRE(p.g() == 255);
      REQUIRE(p.b() == 255);
      REQUIRE(redSeparation->at(y, x) == vy);
      REQUIRE(blueSeparation->at(y, x) == vx);
    }
  }  
}
