/*
 *  plot_main.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 12/12/2010.
 *  Copyright 2010 Christian Brunschen. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <limits>
#include <cmath>
#include <vector>
#include <list>
#include <map>
#include <iomanip>
#include <cstdio>
#include <memory>

using namespace std;

#include "Primitives.h"
#include "Image.h"
#include "GreyImage.h"
#include "GreyImage_Impl.h"
#include "ColorImage.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"
#include "Chain.h"
#include "Line.h"
#include "Output.h"
#include "K3M.h"
#include "Progress.h"
#include "PlotterPathExtractor.h"

#undef D
#define D(x) do { (x); } while(0)

using namespace K3M;
using namespace Images;
using namespace std;
using namespace Extraction;

class Printer {
  ostream &out_;
public:
  Printer(ostream &out) : out_(out) { }
  void operator() (int x, int y) {
    out_ << "[" << x << " " << y << " 1 1] ";
  }
};

void verifyChains(Chains &chains) {
  cerr << "verifying chains:" << endl;
  for (Chains::iterator hc = chains.begin(); hc != chains.end(); ++hc) {
    if (hc->size() > 1) {
      Chain::iterator j = hc->begin();
      IPoint p = *j;
      for (++j; j != hc->end(); ++j) {
        if (!p.isNeighbour(*j)) {
          cerr << "not neighbours: " << p << " <-> " << *j << endl;
        }
        p = *j;
      }
    }
  }
}

int getInt(const string &s) {
  int value;
  stringstream ss(s);
  ss >> value;
  return value;
}

double getDouble(const string &s) {
  double value;
  stringstream ss(s);
  ss >> value;
  return value;
}

template<typename C> class Carousel {
  typedef Primitives::PenColor<C> PenColor;
  typedef Primitives::Pen<C> Pen;
  typedef list<Pen> Pens;

  map<PenColor, Pens> pensByColor_;
  list<PenColor> colors_;

public:
  typedef typename list<PenColor>::iterator iterator;
  typedef typename list<PenColor>::const_iterator const_iterator;

  typedef typename list<Pen>::iterator pen_iterator;
  typedef typename list<Pen>::const_iterator pen_const_iterator;

  Carousel() { }
  void addPen(const Pen &pen) {
    pensByColor_[pen.color()].push_back(pen);
    if (pensByColor_[pen.color()].size() == 1) {
      colors_.push_back(pen.color());
    }
  }

  iterator begin() { return colors_.begin(); }
  const_iterator begin() const { return colors_.begin(); }
  iterator end() { return colors_.end(); }
  const_iterator end() const { return colors_.end(); }

  list<Pen> &pensWithColor(const PenColor &color) {
    return pensByColor_[color];
  }

  const list<Pen> &pensWithColor(const PenColor &color) const {
    return pensByColor_[color];
  }

  pen_iterator begin(const PenColor &color) {
    return pensByColor_[color].begin();
  }
  pen_iterator end(const PenColor &color) {
    return pensByColor_[color].end();
  }

  pen_const_iterator begin(const PenColor &color) const {
    return pensByColor_[color].begin();
  }
  pen_const_iterator end(const PenColor &color) const {
    return pensByColor_[color].end();
  }

  const size_t size() const {
    return colors_.size();
  }
};

enum Rotation {
  None = 0, Left, Right, UpsideDown,
};

int main(int argc, char * const argv[]) {
  bool steps = false;
  string stepPrefix = "/tmp/steps";
  bool approximate = false;
  string approximationFilename;

  typedef uint8_t C;
  typedef ColorImage<C> ColorImage;
  typedef shared_ptr<ColorImage> ColorImageRef;
  typedef RGBPixel<C> RGBPixel;
  typedef GreyImage<C> GreyImage;
  typedef shared_ptr<GreyImage> GreyImageRef;
  typedef PenColor<C> PenColor;
  typedef Pen<C> Pen;
  typedef Carousel<C> Carousel;

  Carousel carousel;
  list< Output<Pen>* > outputs;
  int argn = 1;
  size_t equals = string::npos;
  Rotation rotation = None;
  int offsetX = 0, offsetY = 0;
  double scale = 1.0;
  bool dither = true;

  for (; argn < argc && argv[argn][0] == '-' && strlen(argv[argn]) > 1; argn++) {
    string arg(argv[argn]);
    D(cerr << "arg[" << argn << "] == '" << arg << "'" << endl << flush);
    string penSpec;
    if (0 == arg.find("-pen")) {
      Pen pen;
      if (4 == arg.find("=")) {
        penSpec = arg.substr(5);
      } else {
        ++argn;
        if (!(argn < argc)) {
          cerr << "missing argument to '-pen'" << endl << flush;
          exit(1);
        }
        penSpec = argv[argn];
      }
      size_t start = 0;
      while (start != string::npos) {
        D(cerr << "start=" << start << endl << flush);
        size_t comma = penSpec.find(',', start);
        string part;
        if (comma != string::npos) {
          part = penSpec.substr(start, comma - start);
          start = comma + 1;
        } else {
          part = penSpec.substr(start);
          start = string::npos;
        }
        D(cerr << "part='" << part << "'" << endl << flush);

        if ((equals = part.find('=')) == string::npos) {
          cerr << "pen specification part '" << part << "' missing '='" << endl << flush;
          exit(1);
        } else {
          string name = part.substr(0, equals);
          string value = part.substr(equals + 1);
          D(cerr << "name='" << name << "', value='" << value << "'" << endl << flush);
          switch(name[0]) {
            case 'c': // colour
              pen.color() = PenColor::parse(value);
              break;

            case 'r': // radius
              pen.r() = getInt(value);
              break;

            case 'm': // minimum size that can be drawn
              pen.rMin() = getInt(value);
              break;

            case 'i': // HPGL Index
              pen.hpglIndex() = getInt(value);
              break;

            case 'a': // hatch angle
              pen.hatchAngle() = getDouble(value);
              break;

            case 'p': // hatch phase
              pen.hatchPhase() = getDouble(value);
              break;
          }
        }
      }

      // now add this pen to the carousel
      carousel.addPen(pen);
    } else if (0 == arg.find("-rotate")) {
      string rotationSpec;
      if (7 == arg.find("=")) {
        rotationSpec = arg.substr(8);
      } else {
        ++argn;
        if (!(argn < argc)) {
          cerr << "missing argument to '-rotate'" << endl << flush;
          exit(1);
        }
        rotationSpec = argv[argn];
      }

      switch (rotationSpec[0]) {
        case 'l':
        case 'L':
          rotation = Left;
          break;
        case 'r':
        case 'R':
          rotation = Right;
          break;
        case 'u':
        case 'U':
          rotation = UpsideDown;
          break;
        default:
          cerr << "unrecognized rotation '" << arg << "'" << endl << flush;
          exit(1);
      }
    } else if (0 == arg.find("-offset")) {
      string offset;
      if (7 == arg.find("=")) {
        offset = arg.substr(8);
      } else {
        ++argn;
        if (!(argn < argc)) {
          cerr << "missing argument to '-offset'" << endl << flush;
          exit(1);
        }
        offset = argv[argn];
      }
      size_t delim = offset.find_first_of("x,");
      if (delim == string::npos) {
        // no delimiter - offset both x & y by the same amount
        offsetX = offsetY = getInt(offset);
      } else {
        offsetX = getInt(offset.substr(0, delim));
        offsetY = getInt(offset.substr(delim+1));
      }
    } else if (0 == arg.find("-steps")) {
      steps = true;
      if (6 == arg.find("=")) {
        stepPrefix = arg.substr(7);
      }
    } else if (0 == arg.find("-approx")) {
      approximate = true;
      if (7 == arg.find("=")) {
        approximationFilename = arg.substr(8);
      } else {
        approximationFilename = "approximation.png";
      }
    } else if (0 == arg.find("-nodither")) {
      dither = false;
    } else if (0 == arg.find("-out")) {
      size_t eqPos;
      string outputFile;
      if (string::npos != (eqPos = arg.find("="))) {
        outputFile = arg.substr(eqPos + 1);
        arg = arg.substr(0, eqPos);
      } else {
        ++argn;
        if (!(argn < argc)) {
          cerr << "missing argument to '" << arg << "'" << endl << flush;
          exit(1);
        }
        outputFile = argv[argn];
      }
      string outputType = arg.substr(4);
      if (0 == outputType.length()) {
        outputType = "ha"; // HPGL, absolute coordinates
      }
      std::transform(outputType.begin(), outputType.end(), outputType.begin(), ::tolower);
      outputs.push_back(makeOutput<Pen>(outputType, outputFile));
    } else if (0 == arg.find("-scale")) {
      string scaleString;
      if (6 == arg.find("=")) {
        scaleString = arg.substr(8);
      } else {
        ++argn;
        if (!(argn < argc)) {
          cerr << "missing argument to '-scale'" << endl << flush;
          exit(1);
        }
        scaleString = argv[argn];
      }
      scale = getDouble(scaleString);
    } else {
      cerr << "Unrecognized argument '" << arg << "'" << endl << flush;
      exit(1);
    }
  }

  string inputFile;

  if (argn == argc-1) {
    inputFile = argv[argn];
  } else if (argn == argc) {
    // default: is
    inputFile = "-";
  } else {
    cerr << "Extra arguments after input file '" << argv[argn] << "'!";
    cerr << endl << flush;
    exit(1);
  }


  // If there's no specified carousel, use a default one.
  if (carousel.size() == 0) {
    cerr << "No pens specified, using default carousel" << endl << flush;
    // populate the carousel with a default set of pens:
    // black, red, green blue, cyan, magenta, yellow, orange;
    // each at 0.35 mm diameter = 0.175 mm radius = 7 units radius
    Pen black(1.0, 1.0, 1.0, 3); black.hpglIndex() = 1; black.hatchAngle() = 1.0 / 25.0;
    Pen red(0.0, 1.0, 1.0, 3); red.hpglIndex() = 2; red.hatchAngle() = 4.0 / 25.0;
    Pen green(1.0, 0.0, 1.0, 3); green.hpglIndex() = 3; green.hatchAngle() = 7.0 / 25.0;
    Pen blue(1.0, 1.0, 0.0, 3); blue.hpglIndex() = 4; blue.hatchAngle() = 10.0 / 25.0;
    Pen orange(0.0, 0.5, 1.0, 3); orange.hpglIndex() = 8; orange.hatchAngle() = 13.0 / 25.0;
    Pen cyan(1.0, 0.0, 0.0, 3); cyan.hpglIndex() = 5; cyan.hatchAngle() = 16.0 / 25.0;
    Pen magenta(0.0, 1.0, 0.0, 3); magenta.hpglIndex() = 6; magenta.hatchAngle() = 19.0 / 25.0;
    Pen yellow(0.0, 0.0, 1.0, 3); yellow.hpglIndex() = 7; yellow.hatchAngle() = 22.0 / 25.0;

    // put the pens into the carousel in a suitable order
    carousel.addPen(black);
    carousel.addPen(orange);
    carousel.addPen(red);
    carousel.addPen(green);
    carousel.addPen(blue);
    carousel.addPen(cyan);
    carousel.addPen(magenta);
    carousel.addPen(yellow);
  }

  // if there's no output specified, use a default one.
  if (outputs.size() == 0) {
    cerr << "No output specified, writing HPGL to standard output" << endl << flush;
    outputs.push_back(new HPGLAbsoluteOutput<Pen>(cout));
  }

  FILE *f;
  if ("-" == inputFile) {
    cerr << "* Reading image from standard input" << endl << flush;
    f = stdin;
  } else {
    cerr << "* Reading image '" << inputFile << "'" << endl << flush;

    f = fopen(inputFile.c_str(), "r");
    if (f == NULL) {
      cerr << "unable to open input file '" << inputFile << "'!" << endl << flush;
      exit(1);
    }
  }
  ColorImageRef colored(ColorImage::readPng(f));
  if (scale != 1.0) {
    colored = scaleImage(colored, scale);
  }
  Approximator *approximator = NULL;

  if (approximate) {
    approximator = new Approximator(colored->width(), colored->height());
    approximator->approximation().copyRes(colored);
    RGBPixel whitePixel(1.0, 1.0, 1.0);
    approximator->approximation().setAll(whitePixel);
  }

  map<Pen, Chains> ditheredByPen;

  double dTheta = 1.0 / carousel.size();
  double theta = 0.1;

  PlotterPathExtractor extractor;
  extractor.setOut(&cerr);
  extractor.setDither(dither);
  Stepper *stepper;
  if (steps) {
    extractor.setStepper(stepper = new Stepper(stepPrefix));
  }
  if (approximate) {
    extractor.setApproximator(approximator);
  }

  int p = 0;
  for (Carousel::iterator c = carousel.begin(); c != carousel.end(); ++c) {
    const PenColor &color = *c;
    cerr << "- Separating out colour " << color << endl;
    GreyImageRef separated = colored->separateAndSubtract(color);
    if (steps) {
      cerr << "  . writing separation" << endl;
      separated->writePng(stepper->makeName("separation.png", color.unparse().c_str()));
    }

    if (approximate) {
      approximator->setPenColor(color);
    }

    list<Pen> &pens = carousel.pensWithColor(color);
    extractor.outlineHatchThinDither(separated, pens, ditheredByPen);

    ++p;
    theta += dTheta;
  }

  IMatrix transform = IMatrix::identity();

  cerr << "initial transform: " << transform << endl << flush;

  cerr << "offsetX = " << offsetX << ", offsetY = " << offsetY << endl << flush;
  if (offsetX != 0 || offsetY != 0) {
    transform = transform.concat(IMatrix::translate(offsetX, offsetY));
  }
  cerr << "after offset: " << transform << endl << flush;

  cerr << "rotation = " << rotation << endl << flush;
  if (rotation == Left) {
    transform = transform.concat(IMatrix::pageLeft(colored->width(), colored->height()));
  } else if (rotation == Right) {
    transform = transform.concat(IMatrix::pageRight(colored->width(), colored->height()));
  } else if (rotation == UpsideDown) {
    transform = transform.concat(IMatrix::pageUpsideDown(colored->width(), colored->height()));
  }
  cerr << "after rotation: " << transform << endl << flush;

  int maxX = numeric_limits<int>::min(), maxY = numeric_limits<int>::min();
  int minX = numeric_limits<int>::max(), minY = numeric_limits<int>::max();
  for (int x = 0; x < colored->width(); x += colored->width()-1) {
    for (int y = 0; y < colored->height(); y += colored->height()-1) {
      IPoint p(x, y);
      p.transform(transform);
      minX = min(p.x(), minX);
      maxX = max(p.x(), maxX);
      minY = min(p.y(), minY);
      maxY = max(p.y(), maxY);
    }
  }

  cerr << "minX = " << minX << ", minY = " << minY << ", maxX = " << maxX << ", maxY = " << maxY << endl << flush;

  // Finally, adjust for the top-to-bottom Y of our bitmaps
  // vs the bottom-to-top Y of our output devices
  IMatrix adjustment(1, 0, 0, -1, 0, colored->height());
  transform = transform.concat(adjustment);
  cerr << "after adjustment: " << transform << endl << flush;

  for (list< Output<Pen>* >::const_iterator outIter = outputs.begin();
       outIter != outputs.end(); ++outIter) {
    cerr << "Writing " << (**outIter) << endl << flush;
    (*outIter)->open();
    (*outIter)->beginPage(maxX + 1, maxY + 1);
  }

  for (map<Pen, Chains>::iterator di = ditheredByPen.begin(); di != ditheredByPen.end(); ++di) {
    const Pen &pen = di->first;
    Chains &dithered = di->second;

    if (transform != IMatrix::identity()) {
      cerr << "- applying transform " << transform << endl << flush;
      dithered.transform(transform);
    } else {
      cerr << "-  skipping identity transform " << transform << endl << flush;
    }

    for (list< Output<Pen>* >::const_iterator outIter = outputs.begin();
         outIter != outputs.end(); ++outIter) {
      (*outIter)->setPen(pen);
      (*outIter)->outputChains(dithered);
    }
  }

  for (list< Output<Pen>* >::const_iterator outIter = outputs.begin();
       outIter != outputs.end(); ++outIter) {
    (*outIter)->endPage();
    (*outIter)->close();
    delete (*outIter);
  }
  outputs.clear();

  if (approximate) {
    cerr << "    . drawing final approximation" << endl;
    approximator->approximation().writePng(approximationFilename.c_str());

    delete approximator;
    approximator = NULL;
  }
}

