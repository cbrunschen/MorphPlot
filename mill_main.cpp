/*
 *  mill_main.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 8/2/2014.
 *  Copyright 2014 Christian Brunschen. All rights reserved.
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
#include "MillPathExtractor.h"

#undef D
#define D(x) do { (x); } while(0)

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

enum Rotation {
  None = 0, Left, Right, UpsideDown,
};

static bool checkArg(int argc, const char * const * const argv, int &argn, const string &s, string &param) {
  string arg(argv[argn]);
  if (0 == arg.find(s)) {
    int l = static_cast<int>(s.length());
    if (l == arg.find("=")) {
      param = arg.substr(l+1);
    } else {
      ++argn;
      if (!(argn < argc)) {
        cerr << "missing argument to '" << s << "'" << endl << flush;
        exit(1);
      }
      param = argv[argn];
    }
    return true;
  } else {
    return false;
  }
}

static bool checkArg(int argc, const char * const * const argv, int &argn, const string &s, string &param, bool &hasParam) {
  string arg(argv[argn]);
  if (0 == arg.find(s)) {
    int l = static_cast<int>(s.length());
    if (l == arg.find("=")) {
      hasParam = true;
      param = arg.substr(l+1);
    } else {
      hasParam = false;
    }
    return true;
  } else {
    return false;
  }
}

template<typename H>
static shared_ptr<GreyImage<uint16_t> > makeShape(int r, const H &h) {
  int wh = 2 * r + 1;
  shared_ptr<GreyImage<uint16_t> > shape = GreyImage<uint16_t>::make(wh, wh, false);
  for (int y = 0; y < wh; y++) {
    int dy = r - y;
    for (int x = 0; x < wh; x++) {
      int dx = r - x;
      double d = sqrt(dx*dx + dy*dy);
      shape->set(x, y, h(d));
    }
  }
  return shape;
}

struct FlatHeight {
  int r_;
  FlatHeight(int r) : r_(r) { }
  const uint16_t operator()(double d) const {
    return d <= r_ ? 0 : numeric_limits<uint16_t>::max();
  }
};

struct ConeHeight {
  int r0_;
  int r1_;
  double incline_;
  ConeHeight(int r0, int r1, double angle) : r0_(r0), r1_(r1), incline_(atan(angle / 2)) { }
  const uint16_t operator()(double d) const {
    if (d <= r0_) {
      return 0;
    } else if (r1_ < d) {
      return numeric_limits<uint16_t>::max();
    } else {
      return (d - r0_) * incline_;
    }
  }
};

struct HemisphereHeight {
  int r_;
  int r2_;
  HemisphereHeight(int r) : r_(r), r2_(r * r) { }
  const uint16_t operator()(double d) const {
    if (d > r_) {
      return numeric_limits<uint16_t>::max();
    } else {
      return r_ - sqrt(r2_ - d * d);
    }
  }
};

static shared_ptr<GreyImage<uint16_t> > makeFlatShape(int r) {
  return makeShape<FlatHeight>(r, FlatHeight(r));
}

static shared_ptr<GreyImage<uint16_t> > makeConeShape(int r0, int r1, double angle) {
  return makeShape<ConeHeight>(r1, ConeHeight(r0, r1, angle));
}

static shared_ptr<GreyImage<uint16_t> > makeHemisphereShape(int r) {
  return makeShape<HemisphereHeight>(r, HemisphereHeight(r));
}

class Tool {
public:
  typedef uint16_t H;
  typedef GreyImage<H> HeightMap;
  typedef shared_ptr<HeightMap> HeightMapRef;

private:
  // The shape of the tool, as a height map, for purposes of fitting the tool shape insite the height map
  HeightMapRef shape_;
  // the "radius" of the tool for offsetting and for drawing the path
  int r_;
  int zUp_;
  int zDown_;
  int zBottom_;
public:
  Tool(int r) : shape_(NULL), r_(r), zUp_(40), zDown_(0), zBottom_(-256) {}
  Tool(int r, HeightMapRef shape) : shape_(shape), r_(r), zUp_(40), zDown_(0), zBottom_(-256) {}

  const HeightMapRef shape() const { return shape_; }
  void setShape(HeightMapRef shape) { shape_ = shape; }

  void setR(int r) { r_ = r; }
  const int r() const { return r_; }

  const int hpglIndex() const { return 1; }
  const double zLevel() const { return (double)(zDown_) / (double)(zBottom_); }
  const double cFraction() const { return zLevel(); }
  const double mFraction() const { return zLevel(); }
  const double yFraction() const { return zLevel(); }

  const int zUp() const { return zUp_; }
  void setZUp(int z) { zUp_ = z; }
  const int zDown() const { return zDown_; }
  void setZDown(int z) { zDown_ = z; }
  const int zBottom() const { return zBottom_; }
  void setZBottom(int z) { zBottom_ = z; }
};

int main(int argc, char * const argv[]) {
  typedef uint16_t H;
  typedef GreyImage<H> HeightMap;
  typedef shared_ptr<HeightMap> HeightMapRef;
  typedef shared_ptr<Bitmap> BitmapRef;

  bool steps = false;
  string stepPrefix = "/tmp/steps";
  bool approximate = false;
  string approximationFilename;
  list< Output<Tool>* > outputs;

  int argn = 1;
  size_t equals = string::npos;
  Rotation rotation = None;
  int offsetX = 0, offsetY = 0;
  double scale = 1.0;
  Tool tool(40); // 1mm radius = 2mm tool.
  int threads = 4;  // run reasonably parallel if we can.

  int extraInset = 0;  // 0.2mm extra inset in the roughing pass
  int dz = 1;  // how far apart the layers are

  bool concentric = false;

  for (; argn < argc && argv[argn][0] == '-' && strlen(argv[argn]) > 1; argn++) {
    string arg(argv[argn]);
    bool hasSpec = false;
    D(cerr << "arg[" << argn << "] == '" << arg << "'" << endl << flush);
    string spec;
    if (checkArg(argc, argv, argn, "-tool", spec)) {
      size_t start = 0;
      while (start != string::npos) {
        D(cerr << "start=" << start << endl << flush);
        size_t comma = spec.find(',', start);
        string part;
        if (comma != string::npos) {
          part = spec.substr(start, comma - start);
          start = comma + 1;
        } else {
          part = spec.substr(start);
          start = string::npos;
        }
        D(cerr << "part='" << part << "'" << endl << flush);

        if ((equals = part.find('=')) == string::npos) {
          cerr << "tool specification part '" << part << "' missing '='" << endl << flush;
          exit(1);
        } else {
          string name = part.substr(0, equals);
          string value = part.substr(equals + 1);
          D(cerr << "name='" << name << "', value='" << value << "'" << endl << flush);
          switch(name[0]) {
            case 'r': // radius
              tool.setR(getInt(value));
              break;
          }
        }
      }
    } else if (checkArg(argc, argv, argn, "-rotate", spec)) {
      switch (spec[0]) {
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
    } else if (checkArg(argc, argv, argn, "-offset", spec)) {
      size_t delim = spec.find_first_of("x,");
      if (delim == string::npos) {
        // no delimiter - offset both x & y by the same amount
        offsetX = offsetY = getInt(spec);
      } else {
        offsetX = getInt(spec.substr(0, delim));
        offsetY = getInt(spec.substr(delim+1));
      }
    } else if (checkArg(argc, argv, argn, "-steps", spec, hasSpec)) {
      steps = true;
      if (hasSpec) {
        stepPrefix = spec;
      }
    } else if (checkArg(argc, argv, argn, "-approx", spec, hasSpec)) {
      approximate = true;
      if (hasSpec) {
        approximationFilename = arg.substr(8);
      } else {
        approximationFilename = "approximation.png";
      }
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
        outputType = "rml";
      }
      std::transform(outputType.begin(), outputType.end(), outputType.begin(), ::tolower);
      outputs.push_back(make3DOutput<Tool>(outputType, outputFile));
    } else if (checkArg(argc, argv, argn, "-scale", spec)) {
      scale = getDouble(spec);
    } else if (checkArg(argc, argv, argn, "-dz", spec)) {
      dz = getInt(spec);
    } else if (checkArg(argc, argv, argn, "-inset", spec)) {
      extraInset = getInt(spec);
    } else if (checkArg(argc, argv, argn, "-threads", spec)) {
      threads = getInt(spec);
    } else if (checkArg(argc, argv, argn, "-concentric", spec, hasSpec)) {
      if (hasSpec) {
        std::transform(spec.begin(), spec.end(), spec.begin(), ::tolower);
        concentric = (spec == "true") || (spec == "yes");
      } else {
        concentric = true;
      }
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

  // if there's no output specified, use a default one.
  if (outputs.size() == 0) {
    cerr << "No output specified, writing RML1 to standard output" << endl << flush;
    outputs.push_back(new RML1Output<Tool>(cout));
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

  HeightMapRef heightMap = HeightMap::readPngHeightmap(f);
  
  cerr << "running with " << threads << " threads" << endl << flush;
  Workers workers(threads);
  
  HeightMapRef toolShape = makeConeShape(4, 40, M_PI / 6.0);
  
  BitmapRef gt = heightMap->gt(16, workers);
  
  shared_ptr<Image<uint16_t> > fit = heightMap->fit(toolShape, workers);

  if (scale != 1.0) {
    heightMap = scaleImage(heightMap, scale);
  }
  // Approximator *approximator = NULL;

  if (approximate) {
  }

  list<Chains> outlineChains, fillChains;
  MillPathExtractor extractor;
  extractor.setOut(&cerr);
  Stepper *stepper = nullptr;

  if (steps) {
    extractor.setStepper(stepper = new Stepper(stepPrefix));
  }

  if (approximate) {
    // extractor.setApproximator(approximator);
    // approximator->setPenColor(PenColor<C>(1.0, 1.0, 1.0));
  }

  H minZ = heightMap->min(workers);
  H maxZ = heightMap->max(workers);

  for (int z = maxZ; z >= minZ; z -= dz) {
    if (stepper) {
      ostringstream s;
      s << "z=" << setw(4) << setfill('0') << z << flush;
      stepper->setPen(s.str());
    }
    cerr << "layer at z = " << z << endl;
    BitmapRef bitmap = heightMap->lt(z, workers);
    outlineChains.push_back(Chains());
    Chains &outline = outlineChains.back();
    fillChains.push_back(Chains());
    Chains &fill = fillChains.back();
    if (concentric) {
      extractor.outlineConcentric(bitmap, tool.r(), extraInset, outline, fill, workers);
    } else {
      extractor.outlineAndFill(bitmap, tool.r(), extraInset, outline, fill, workers);
    }
    outline.simplify(1.5);
    fill.simplify(1.5);
  }

  int defaultOffset = 1 + tool.r();

  IMatrix transform = IMatrix::translate(-defaultOffset, -defaultOffset);

  cerr << "initial transform: " << transform << endl << flush;

  cerr << "offsetX = " << offsetX << ", offsetY = " << offsetY << endl << flush;
  if (offsetX != 0 || offsetY != 0) {
    transform = transform.concat(IMatrix::translate(offsetX, offsetY));
  }
  cerr << "after offset: " << transform << endl << flush;

  cerr << "rotation = " << rotation << endl << flush;
  if (rotation == Left) {
    transform = transform.concat(IMatrix::pageLeft(heightMap->width(), heightMap->height()));
  } else if (rotation == Right) {
    transform = transform.concat(IMatrix::pageRight(heightMap->width(), heightMap->height()));
  } else if (rotation == UpsideDown) {
    transform = transform.concat(IMatrix::pageUpsideDown(heightMap->width(), heightMap->height()));
  }
  cerr << "after rotation: " << transform << endl << flush;

  int maxX = numeric_limits<int>::min(), maxY = numeric_limits<int>::min();
  int minX = numeric_limits<int>::max(), minY = numeric_limits<int>::max();
  for (int x = 0; x < heightMap->width(); x += heightMap->width()-1) {
    for (int y = 0; y < heightMap->height(); y += heightMap->height()-1) {
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
  IMatrix adjustment(1, 0, 0, -1, 0, heightMap->height());
  transform = transform.concat(adjustment);
  cerr << "after adjustment: " << transform << endl << flush;

  for (list< Output<Tool>* >::const_iterator outIter = outputs.begin();
       outIter != outputs.end(); ++outIter) {
    cerr << "Writing " << (**outIter) << endl << flush;
    (*outIter)->open();
    (*outIter)->beginPage(maxX + 1, maxY + 1);
  }

  list<Chains>::iterator outlineIter = outlineChains.begin();
  list<Chains>::iterator fillIter = fillChains.begin();

  tool.setZBottom(minZ - maxZ);
  for (int z = maxZ; z >= minZ; z -= dz) {
    Chains &outline = *outlineIter++;
    Chains &fill = *fillIter++;

    tool.setZDown(z - maxZ);

    if (transform != IMatrix::identity()) {
      cerr << "- applying transform " << transform << endl << flush;
      outline.transform(transform);
      fill.transform(transform);
    } else {
      cerr << "-  skipping identity transform " << transform << endl << flush;
    }

    for (list< Output<Tool>* >::const_iterator outIter = outputs.begin();
         outIter != outputs.end(); ++outIter) {
      (*outIter)->setPen(tool);
      (*outIter)->outputChains(outline);
      (*outIter)->outputChains(fill);
    }
  }

  for (list< Output<Tool>* >::const_iterator outIter = outputs.begin();
       outIter != outputs.end(); ++outIter) {
    (*outIter)->endPage();
    (*outIter)->close();
    delete (*outIter);
  }
  outputs.clear();

  if (approximate) {
//    cerr << "    . drawing final approximation" << endl;
//    approximator->approximation().writePng(approximationFilename.c_str());

//    delete approximator;
//    approximator = NULL;
  }
}

