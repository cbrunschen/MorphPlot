//
//  Voronoi.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 20/03/2014.
//
//

#include <iostream>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include "Primitives.h"
#include "GreyImage.h"
#include "Progress.h"
#include "Bitmap_Impl.h"

using namespace std;
using namespace Images;

double frand() {
  return (double) rand() / ((double) RAND_MAX);
}

double frand2() {
  return (double) rand() / ((double) ((long)RAND_MAX + 1));
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

template<typename T>
inline static void addIfDifferent(const Point &a, const Point &b, T &result) {
  if (a != b) {
    result[a].insert(b);
    result[b].insert(a);
  }
}
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

template<typename T>
void getNeighbouringVoronoiCells(const Image<Point> &vd, T &result) {
  int w = vd.width();
  int h = vd.height();
  
  const Point *qx, *qy, *qxy;
  
  // left column
  qy = vd.data();
  for (int y = 1; y < h; y++) {
    const Point *p = qy + w;
    addIfDifferent(*p, *qy, result);
    qy = p;
  }
  
  // top row
  qx = vd.data();
  for (int x = 1; x < w; x++) {
    const Point *p = qx + 1;
    addIfDifferent(*p, *qx, result);
    qx = p;
  }
  
  // the rest of the image
  for (int y = 1; y < h; y++) {
    qxy = &vd.data()[(y-1) * w];
    qy = qxy + 1;
    qx = qxy + w;
    for (int x = 1; x < w; x++) {
      const Point *p = qx + 1;
      
      addIfDifferent(*p, *qx, result);
      addIfDifferent(*p, *qy, result);
      addIfDifferent(*p, *qxy, result);
      addIfDifferent(*qx, *qy, result);
      
      qx = p;
      ++qxy;
      ++qy;
    }
  }
}

struct CentroidCollector {
  int weightedX;
  int weightedY;
  int weight;
  CentroidCollector() : weightedX(0), weightedY(0), weight(0) { }
  void add(int x, int y, int w) {
    weightedX += x * w;
    weightedY += y * w;
    weight += w;
  }
  void add(const CentroidCollector &other) {
    weightedX += other.weightedX;
    weightedY += other.weightedY;
    weight += other.weight;
  }
  double x() { return weight > 0 ? (double) weightedX / (double) weight : 0; }
  double y() { return weight > 0 ? (double) weightedY / (double) weight : 0; }
  Point p() { return Point((int) x(), (int) y()); }
};

template<typename T>
void calculateCentroids(const Image<Point> &regions, const Image<uint8_t> &weights, T &result) {
  int w = regions.width();
  double xScale = (double)w / (double)weights.width();
  
  int h = regions.height();
  double yScale = (double)h / (double)weights.height();

  unordered_map<Point, CentroidCollector> collectors;
  
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      const Point &p = regions.at(y, x);
      int weight = 255 - weights.at((int) floor(y / yScale), (int) floor(x / xScale));
      collectors[p].add(x - p.x(), y - p.y(), weight);
    }
  }

  unordered_set<Point> empty;
  
  for (unordered_map<Point, CentroidCollector>::iterator i = collectors.begin(); i != collectors.end(); ++i) {
    const Point &p = i->first;
    CentroidCollector &c = i->second;
    if (c.weight == 0) {
      // empty cell; remove and put in some other random place.
      empty.insert(p);
    } else {
      result[p] = Point((int) round(p.x() + c.x()), (int) round(p.y() + c.y()));
    }
  }
  if (empty.size() > 0) {
    cerr << "-- found " << empty.size() << " empty voronoi regions, replacing with random." << endl << flush;
    unordered_set<Point> newSites;
    for (typename T::iterator i = result.begin(); i != result.end(); ++i) {
      newSites.insert(i->second);
    }
    for (unordered_set<Point>::iterator i = empty.begin(); i != empty.end(); ++i) {
      for (;;) {
        int x = (int) floor(frand2() * regions.width());
        int y = (int) floor(frand2() * regions.height());
        Point p(x, y);
        if (newSites.find(p) == newSites.end() && 256 * frand2() > weights.at(y / yScale, x / xScale)) {
          newSites.insert(p);
          result[*i] = p;
          break;
        }
      }
    }
  }
}

int main(int argc, char **argv) {
  typedef GreyImage<uint8_t> WeightMap;
  typedef shared_ptr<WeightMap> WeightMapRef;
  typedef shared_ptr<Bitmap> BitmapRef;
  typedef Image<Point> Voronoi;
  typedef shared_ptr<Voronoi> VoronoiRef;

  double scale = 1.0;
  int nStipples = 1000;
  
  bool steps = false;
  string stepPrefix = "/tmp/steps";
  Progress::Stepper *stepper = nullptr;

  int argn = 1;

  for (; argn < argc && argv[argn][0] == '-' && strlen(argv[argn]) > 1; argn++) {
    string arg(argv[argn]);
    bool hasSpec = false;
    D(cerr << "arg[" << argn << "] == '" << arg << "'" << endl << flush);
    string spec;
    if (checkArg(argc, argv, argn, "-scale", spec)) {
      scale = getDouble(spec);
    } else if (checkArg(argc, argv, argn, "-stipples", spec)) {
      nStipples = getInt(spec);
    } else if (checkArg(argc, argv, argn, "-steps", spec, hasSpec)) {
      steps = true;
      if (hasSpec) {
        stepPrefix = spec;
      }
      stepper = new Progress::Stepper(stepPrefix);
      stepper->setPen("");
    } else {
      cerr << "Unrecognized argument '" << arg << "'" << endl << flush;
      exit(1);
    }
  }
  
  string inputFile;
  
  if (argn == argc-1) {
    inputFile = argv[argn];
  } else if (argn == argc) {
    // default: standard input
    inputFile = "-";
  } else {
    cerr << "Extra arguments after input file '" << argv[argn] << "'!";
    cerr << endl << flush;
    exit(1);
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
  
  WeightMapRef weights = WeightMap::readPng(f);
  
  unordered_set<Point> stipples;
  while (stipples.size() < nStipples) {
    int x = (int) floor(frand2() * scale * weights->width());
    int y = (int) floor(frand2() * scale * weights->height());
    if (256 * frand2() > weights->at(y / scale, x / scale)) {
      stipples.insert(Point(x, y));
    }
  }
  
  unordered_map<Point, uint8_t> regionGreys;
  
  Workers workers(8);
  
  int i = 0;
  for (;;) {
    BitmapRef sites = Bitmap::make(weights->width() * scale, weights->height() * scale, true);
    for (unordered_set<Point>::iterator i = stipples.begin(); i != stipples.end(); ++i) {
      const Point &p = *i;
      sites->at(p) = true;
    }
    
    if (steps) {
      sites->writePng(stepper->makeName("sites.png"));
    }

    VoronoiRef voronoi = sites->featureTransform(true, workers);

    if (steps) {
      regionGreys.clear();
      int j = 0;
      for (unordered_set<Point>::iterator i = stipples.begin(); i != stipples.end(); ++i) {
        regionGreys[*i] = 5 + (j++ * 250 / nStipples);
      }
      
      shared_ptr< GreyImage<uint8_t> > regions = GreyImage<uint8_t>::make(sites->width(), sites->height(), true);
      for (int y = 0; y < regions->height(); y++) {
        for (int x = 0; x < regions->width(); x++) {
          regions->at(y, x) = regionGreys[voronoi->at(y, x)];
        }
      }
      regions->writePng(stepper->makeName("regions.png"));
    }
    
    unordered_map<Point, Point> centroidMovements;
    
    calculateCentroids(*voronoi, *weights, centroidMovements);
    
    int d2 = 0;
    int same = 0;
    int differ = 0;
    stipples.clear();
    for (unordered_map<Point, Point>::iterator i = centroidMovements.begin(); i != centroidMovements.end(); ++i) {
      const Point &p = i->first;
      const Point &q = i->second;
      
      if (p == q) {
        same++;
      } else {
        differ++;
        int dx = p.x() - q.x();
        int dy = p.y() - q.y();
        d2 += dx*dx + dy*dy;
      }
      
      stipples.insert(q);
    }
    
    cerr << "after " << i++ << " iterations: " << same << " same, " << differ << " differ, movement is " << d2 << endl << flush;
    if (d2 == 0 || differ == 0) {
      break;
    }
  }

  exit(0);
  return 0;
}
