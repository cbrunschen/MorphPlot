//
//  Voronoi.cpp
//  MorphPlot
//
//  Created by Christian Brunschen on 20/03/2014.
//
//

#include <sys/time.h>

#include <iostream>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <functional>
#include <utility>

#include "Primitives.h"
#include "GreyImage.h"
#include "Progress.h"
#include "Bitmap_Impl.h"

using namespace std;
using namespace Images;

static random_device rd;
static default_random_engine rng(rd());
static uniform_real_distribution<double> dist;
static inline double frand() { return dist(rng); }

double now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (double)(tv.tv_sec % 86400) + ((double)tv.tv_usec / 1000000.0);
}

int getInt(const string &s) {
  int value;
  stringstream ss(s);
  ss >> value;
  return value;
}

unsigned int getUnsignedInt(const string &s) {
  unsigned int value;
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

template<typename NBS>
struct GetNeighboursParams {
  int y0;
  int y1;
  int w;
  const Image<Point> *regions;
  NBS neighboursBySite;
};

template<typename NBS>
inline static void addIfDifferent(const Point &a, const Point &b, NBS &result) {
  if (a != b) {
    result[a].insert(b);
    result[b].insert(a);
  }
}

template<typename NBS>
void getNeighbours_workerFunction(void *v) {
  GetNeighboursParams<NBS> *params = static_cast<GetNeighboursParams<NBS> *>(v);
 
  const Point *qx, *qy, *qxy;

  if (params->y0 == 0) {
    // This is the top chunk; it has to deal with the top row, and thus also the top pixel in the left column, differently.
    
    // top row
    qx = params->regions->data();
    for (int x = 1; x < params->w; x++) {
      const Point *p = qx + 1;
      addIfDifferent(*p, *qx, params->neighboursBySite);
      qx = p;
    }

    // left column, skipping the top pixel
    qy = params->regions->data();
    for (int y = 1; y < params->y1; y++) {
      const Point *p = qy + params->w;
      addIfDifferent(*p, *qy, params->neighboursBySite);
      qy = p;
    }
    
    // the rest of the image
    for (int y = 1; y < params->y1; y++) {
      qxy = &params->regions->data()[(y-1) * params->w];
      qy = qxy + 1;
      qx = qxy + params->w;
      for (int x = 1; x < params->w; x++) {
        const Point *p = qx + 1;
        
        addIfDifferent(*p, *qx, params->neighboursBySite);
        addIfDifferent(*p, *qy, params->neighboursBySite);
        addIfDifferent(*p, *qxy, params->neighboursBySite);
        addIfDifferent(*qx, *qy, params->neighboursBySite);
        
        qx = p;
        ++qxy;
        ++qy;
      }
    }
  } else {
    // left column, including the top pixel
    qy = &params->regions->data()[(params->y0 - 1) * params->w];
    for (int y = params->y0; y < params->y1; y++) {
      const Point *p = qy + params->w;
      addIfDifferent(*p, *qy, params->neighboursBySite);
      qy = p;
    }
    
    // the rest of the image, including the top row
    for (int y = params->y0; y < params->y1; y++) {
      qxy = &params->regions->data()[(y-1) * params->w];
      qy = qxy + 1;
      qx = qxy + params->w;
      for (int x = 1; x < params->w; x++) {
        const Point *p = qx + 1;
        
        addIfDifferent(*p, *qx, params->neighboursBySite);
        addIfDifferent(*p, *qy, params->neighboursBySite);
        addIfDifferent(*p, *qxy, params->neighboursBySite);
        addIfDifferent(*qx, *qy, params->neighboursBySite);
        
        qx = p;
        ++qxy;
        ++qy;
      }
    }
  }
}

template<typename NBS>
void getNeighbours_single(const Image<Point> &vd, NBS &neighboursBySite) {
  int w = vd.width();
  int h = vd.height();
  
  const Point *qx, *qy, *qxy;
  
  // left column
  qy = vd.data();
  for (int y = 1; y < h; y++) {
    const Point *p = qy + w;
    addIfDifferent(*p, *qy, neighboursBySite);
    qy = p;
  }
  
  // top row
  qx = vd.data();
  for (int x = 1; x < w; x++) {
    const Point *p = qx + 1;
    addIfDifferent(*p, *qx, neighboursBySite);
    qx = p;
  }
  
  // the rest of the image
  for (int y = 1; y < h; y++) {
    qxy = &vd.data()[(y-1) * w];
    qy = qxy + 1;
    qx = qxy + w;
    for (int x = 1; x < w; x++) {
      const Point *p = qx + 1;
      
      addIfDifferent(*p, *qx, neighboursBySite);
      addIfDifferent(*p, *qy, neighboursBySite);
      addIfDifferent(*p, *qxy, neighboursBySite);
      addIfDifferent(*qx, *qy, neighboursBySite);
      
      qx = p;
      ++qxy;
      ++qy;
    }
  }
}

template<typename NBS>
void getNeighbours(const Image<Point> &vd, NBS &neighboursBySite, Workers &workers) {
  int threads = workers.n();
  
  if (threads == 1) {
    getNeighbours_single(vd, neighboursBySite);
  } else {
    int w = vd.width();
    int h = vd.height();
    
    GetNeighboursParams<NBS> *params = new GetNeighboursParams<NBS>[threads];
    int top = 0;
    for (int i = 0; i < threads; i++) {
      int bottom = ((i + 1) * h) / threads;
      params[i].y0 = top;
      params[i].y1 = bottom;
      params[i].w = w;
      params[i].regions = &vd;
      top = bottom;
    }
    
    workers.perform(getNeighbours_workerFunction<NBS>, params);
    
    for (int i = 0; i < threads; i++) {
      for (auto j = params[i].neighboursBySite.begin(); j != params[i].neighboursBySite.end(); j++) {
        neighboursBySite[j->first].insert(j->second.begin(), j->second.end());
      }
    }
    
    delete [] params;
  }
}

template<typename NBS>
void getNeighbours(const Image<Point> &vd, NBS &neighboursBySite, int threads = 1) {
  if (threads == 1) {
    getNeighbours_single(vd, neighboursBySite);
  } else {
    Workers workers(threads);
    getNeighbours(vd, neighboursBySite, workers);
  }
}

struct WeightedRegion {
  int weightedX;
  int weightedY;
  int weight;
  WeightedRegion() : weightedX(0), weightedY(0), weight(0) { }
  void add(int x, int y, int w) {
    weightedX += x * w;
    weightedY += y * w;
    weight += w;
  }
  void add(const WeightedRegion &other) {
    weightedX += other.weightedX;
    weightedY += other.weightedY;
    weight += other.weight;
  }
  double x() { return weight > 0 ? (double) weightedX / (double) weight : 0; }
  double y() { return weight > 0 ? (double) weightedY / (double) weight : 0; }
  Point p() { return Point((int) x(), (int) y()); }
};

typedef uint8_t Weight;
typedef GreyImage<Weight> WeightMap;
typedef shared_ptr<WeightMap> WeightMapRef;
typedef shared_ptr<Bitmap> BitmapRef;
typedef Image<Point> Voronoi;
typedef shared_ptr<Voronoi> VoronoiRef;

int WEIGHT_MAX = numeric_limits<Weight>::max();

template<typename WRBS>
struct CentroidCalculationParams {
  int y0;
  int y1;
  int w;
  double xScale;
  double yScale;
  const Image<Point> *regions;
  const Image<uint8_t> *weights;
  WRBS weightedRegionsBySite;
};

template<typename WRBS>
void calculateCentroid_workerFunction(void *v) {
  CentroidCalculationParams<WRBS> *params = static_cast<CentroidCalculationParams<WRBS> *>(v);
  for (int y = params->y0; y < params->y1; y++) {
    for (int x = 0; x < params->w; x++) {
      const Point &p = params->regions->at(y, x);
      int weight = WEIGHT_MAX - params->weights->at((int) floor(y / params->yScale), (int) floor(x / params->xScale));
      params->weightedRegionsBySite[p].add(x - p.x(), y - p.y(), weight);
    }
  }
}

template<typename WRBS>
inline void calculateCentroids_single(const Image<Point> &regions, const Image<uint8_t> &weights, WRBS &weightedRegionsBySite) {
  int w = regions.width();
  double xScale = (double)w / (double)weights.width();
  
  int h = regions.height();
  double yScale = (double)h / (double)weights.height();

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      const Point &p = regions.at(y, x);
      int weight = WEIGHT_MAX - weights.at((int) floor(y / yScale), (int) floor(x / xScale));
      weightedRegionsBySite[p].add(x - p.x(), y - p.y(), weight);
    }
  }
}

template<typename WRBS>
void calculateCentroids(const Image<Point> &regions, const Image<uint8_t> &weights, WRBS &weightedRegionsBySite, Workers &workers) {
  int threads = workers.n();
  
  if (threads == 1) {
    calculateCentroids_single(regions, weights, weightedRegionsBySite);
  } else {
    int w = regions.width();
    double xScale = (double)w / (double)weights.width();
    
    int h = regions.height();
    double yScale = (double)h / (double)weights.height();

    CentroidCalculationParams<WRBS> *calcParams = new CentroidCalculationParams<WRBS>[threads];
    int top = 0;
    for (int i = 0; i < threads; i++) {
      int bottom = ((i + 1) * h) / threads;
      calcParams[i].y0 = top;
      calcParams[i].y1 = bottom;
      calcParams[i].w = w;
      calcParams[i].xScale = xScale;
      calcParams[i].yScale = yScale;
      calcParams[i].regions = &regions;
      calcParams[i].weights = &weights;
      top = bottom;
    }
    
    workers.perform(calculateCentroid_workerFunction<WRBS>, calcParams);
    
    for (int i = 0; i < threads; i++) {
      WRBS &wrbs = calcParams[i].weightedRegionsBySite;
      for (auto j = wrbs.begin(); j != wrbs.end(); j++) {
        weightedRegionsBySite[j->first].add(j->second);
      }
    }
   
    delete [] calcParams;
  }
}

template<typename WRBS>
void calculateCentroids(const Image<Point> &regions, const Image<uint8_t> &weights,
                        WRBS &weightedRegionsBySite, int threads) {
  if (threads == 1) {
    calculateCentroids_single(regions, weights, weightedRegionsBySite);
  } else {
    Workers workers(threads);
    calculateCentroids(regions, weights, weightedRegionsBySite, workers);
  }
}

template<typename V> void rotateLeft(V &v, size_t offset, size_t len, long amount) {
  amount %= len;
  if (amount < 0) {
    amount = len + amount;
  }
  for (int m = 0, count = 0; count < len; m++) {
    auto t = v[offset + m];
    long i, j;
    for (i = m, j = m + amount; j != m; i = j, j = (j + amount) % len, count++) {
      v[offset + i] = v[offset + j];
    }
    v[offset + i] = t;
    count++;
  }
}

template<typename V> void reverse(V &v, size_t offset, size_t len) {
  size_t last = offset + len - 1;
  size_t n = len / 2;
  for (size_t i = 0; i < n; i++) {
    auto temp = v[offset + i];
    v[offset + i] = v[last - i];
    v[last - i] = temp;
  }
}

template<typename V> void rotateRight(V &v, int offset, int len, int amount) {
  rotateLeft(v, offset, len, len-amount);
}

template<typename P>
struct EuclideanDistance {
  double operator()(const P &p, const P &q) {
    return p.distance(q);
  }
};

template<typename P, typename V, typename D = EuclideanDistance<P> >
void twoOpt(V &v) {
  D d;
  size_t n = v.size();
  for (int i = 0;; i++) {
    int aMin = -1, bMin = -1;
    double bestImprovement = -numeric_limits<double>::infinity();
    for (int a = 0; a < n - 4; a++) {
      int sa = a + 1;
      for (int b = a + 2; b < n - 2; b++) {
        int sb = b + 1;
        double dBefore = d(v[a], v[sa]) + d(v[b], v[sb]);
        double dAfter = d(v[a], v[b]) + d(v[sb], v[sa]);
        double improvement = dBefore - dAfter;
        if (improvement > bestImprovement) {
          aMin = a;
          bMin = b;
          bestImprovement = improvement;
        }
      }
    }
    
    if (bestImprovement > 0.0) {
      reverse(v, aMin+1, bMin - aMin);
      cerr << "2-opt iteration " << i << ": improvement by " << bestImprovement << " at " << aMin << "," << bMin << endl << flush;
    } else {
      cerr << "2-opt found no further improvement, done" << endl << flush;
      break;
    }
  }
}

template<typename P, typename V, typename D = EuclideanDistance<P> >
void randomTwoOpt(V &v) {
  D d;
  int n = static_cast<int>(v.size());
  int unimproved = 0;
  double unreportedImprovement = 0.0;
  int unreportedSwaps = 0;
  uniform_int_distribution<int> dist(0, n-4);

  for (int i = 0; ; i++) {
    int a = dist(rng);
    int t = dist(rng);
    int b = (a + 2 + t) % n;
    if (b < a) {
      swap(a, b);
    }
    int sa = a+1;
    int sb = b+1;
    
    double dBefore = d(v[a], v[sa]) + d(v[b], v[sb]);
    double dAfter = d(v[a], v[b]) + d(v[sb], v[sa]);
    double improvement = dBefore - dAfter;
    
    if (improvement > 0.0) {
      unreportedImprovement += improvement;
      ++unreportedSwaps;
      reverse(v, sa, b - a);
      unimproved = 0;
    } else {
      ++unimproved;
    }
    
    if (i % 1000000 == 0) {
      cerr << "2-opt iteration " << i << ": improvement by " << unreportedImprovement << " in " << unreportedSwaps << " swaps" << endl << flush;
      unreportedImprovement = 0;
      unreportedSwaps = 0;
    }
    if (unimproved > n*n) {
      cerr << "2-opt iteration " << i << ": no more improvement detected afer " << unimproved << " iterations, done." << endl << flush;
      break;
    }
  }
}

template<typename P, typename V, typename D = EuclideanDistance<P> >
void continuousTwoOpt(V &v) {
  D d;
  size_t n = v.size();
  for (int i = 0;; i++) {
    double totalImprovement = 0.0;
    int swaps = 0;
#if 0
    {
      cerr << "2-opt, tour=[  ";
      double total = 0.0;
      Point p;
      for (auto j = v.begin(); j != v.end(); ++j) {
        if (j == v.begin()) p = *j; else {
          double d = p.distance(*j);
          cerr << "  +" << d << "+  ";
          total += d; p = *j;
        }
        cerr << j->x() << "," << j->y();
      }
      cerr << "], length " << total << endl << flush;
    }
#endif
    
    for (int a = 0; a < n - 2; a++) {
      int sa = a + 1;
      for (int b = a + 2; b < n; b++) {
        int sb = (b + 1) % n;
        double dBefore = d(v[a], v[sa]) + d(v[b], v[sb]);
        double dAfter = d(v[a], v[b]) + d(v[sb], v[sa]);
        double improvement = dBefore - dAfter;
        // cerr << "* (" << a << "," << b << "," << c << "): " << dBefore << " -> " << dAfter << endl << flush;
        if (improvement > 0.0) {
          totalImprovement += improvement;
          ++swaps;
          reverse(v, sa, b - a);
        }
      }
    }
    
    if (totalImprovement > 0.0) {
      cerr << "continuous 2-opt iteration " << i << ": improvement by " << totalImprovement << " in " << swaps << " swaps" << endl << flush;
    } else {
      cerr << "continuous 2-opt found no further improvement, done" << endl << flush;
      break;
    }
  }
}

template<typename P, typename V, typename N, typename D = EuclideanDistance<P> >
void threeOpt(V &v, N &neighbours) {
  D d;
  size_t n = v.size();
  for (int i = 0;; i++) {
    int aMin = -1, bMin = -1, cMin = -1;
    double bestImprovement = -numeric_limits<double>::infinity();
    
    {
      cerr << "3-opt, tour=[  ";
      double total = 0.0;
      Point p;
      for (auto j = v.begin(); j != v.end(); ++j) {
        if (j == v.begin()) p = *j; else {
          double d = p.distance(*j);
          cerr << "  +" << d << "+  ";
          total += d; p = *j;
        }
        cerr << j->x() << "," << j->y();
      }
      cerr << "], length " << total << endl << flush;
    }
    
    for (int a = 0; a < n-3; a++) {
      int sa = a+1;
      auto &na = neighbours[v[a]];
      for (int b = sa; b < n-2; b++) {
        int sb = b+1;
//        if (na.find(v[sb]) == na.end()) {
//          cerr << "* (" << a << "," << b << "," << "): not a neighbour, skipping" << endl << flush;
//          continue;
//        }
        auto &nb = neighbours[v[b]];
        for (int c = sb; c < n-1; c++) {
          int sc = c+1;
          auto &nc = neighbours[v[c]];
//          if (nb.find(v[sc]) == nb.end() || nc.find(v[sa]) == nc.end()) {
//            cerr << "* (" << a << "," << b << "," << c << "): not a neighbour, skipping" << endl << flush;
//            continue;
//          }

          double dAA = d(v[a], v[sa]);
          double dBB = d(v[b], v[sb]);
          double dCC = d(v[c], v[sc]);
          double dAB = d(v[a], v[sb]);
          double dCA = d(v[c], v[sa]);
          double dBC = d(v[b], v[sc]);
          
          double dBefore = dAA + dBB + dCC;
          double dAfter = dAB + dCA + dBC;
          double improvement = dBefore - dAfter;
          
//          cerr << "* (" << a << "," << b << "," << c << "): " << dBefore << " -> " << dAfter << endl << flush;
          if (improvement > bestImprovement) {
            aMin = a;
            bMin = b;
            cMin = c;
            bestImprovement = improvement;
            cerr << "  - have new best improvement " << bestImprovement << " at (" << a << "," << b << "," << c << "):" << endl;
            cerr << "  " << dAA << " + " << dBB << " + " << dCC << " = " << dBefore << "  => " ;
            cerr << "  " << dAB << " + " << dCA << " + " << dBC << " = " << dAfter << endl << flush;
          }
        }
      }
    }
    
    if (bestImprovement > 0.0) {
      cerr << "3-opt iteration " << i << ": improvement by " << bestImprovement << " at " << aMin << "," << bMin << "," << cMin << endl << flush;
      rotateLeft(v, aMin+1, cMin - aMin, bMin - aMin);
    } else {
      cerr << "3-opt found no further improvement, done" << endl << flush;
      break;
    }
  }
}

template<typename P, typename V, typename N, typename D = EuclideanDistance<P> >
void randomThreeOpt(V &v, N &neighbours) {
  D d;
  int n = static_cast<int>(v.size());
  int unimproved = 0;
  double unreportedImprovement = 0.0;
  int unreportedSwaps = 0;

  for (int i = 0;; i++) {
#if 0
    {
      cerr << "3-opt, tour=[  ";
      double total = 0.0;
      Point p;
      for (auto j = v.begin(); j != v.end(); ++j) {
        if (j == v.begin()) p = *j; else {
          double d = p.distance(*j);
          cerr << "  +" << d << "+  ";
          total += d; p = *j;
        }
        cerr << j->x() << "," << j->y();
      }
      cerr << "], length " << total << endl << flush;
    }
#endif
    
    uniform_int_distribution<int> dist(0, n-6);
    
    int x = dist(rng);
    int y = dist(rng);
    int z = dist(rng);
    
    if (y < x) {
      swap(x, y);
    }
    if (z < y) {
      swap(y, z);
    }
    if (y < x) {
      swap(x, y);
    }
    
    int a = x, sa = a+1;
    int b = y + 2, sb = b + 1;
    int c = z + 4, sc = c + 1;
    
    double dAA = d(v[a], v[sa]);
    double dBB = d(v[b], v[sb]);
    double dCC = d(v[c], v[sc]);
    double dAB = d(v[a], v[sb]);
    double dCA = d(v[c], v[sa]);
    double dBC = d(v[b], v[sc]);

    double dBefore = dAA + dBB + dCC;
    double dAfter = dAB + dCA + dBC;
    double improvement = dBefore - dAfter;
    
    if (improvement > 0.0) {
      unreportedImprovement += improvement;
      ++unreportedSwaps;
      rotateLeft(v, sa, c - a, b - a);
      unimproved = 0;
    } else {
      ++unimproved;
    }
    
    if (i % 1000000 == 0) {
      cerr << "3-opt iteration " << i << ": improvement by " << unreportedImprovement << " in " << unreportedSwaps << " swaps" << endl << flush;
      unreportedImprovement = 0;
      unreportedSwaps = 0;
    }
    if (unimproved > n*n) {
      cerr << "3-opt iteration " << i << ": no more improvement detected afer " << unimproved << " iterations, done." << endl << flush;
      break;
    }
  }
}

template<typename T, typename I>
void addRandomStipples(T &stipples, int nStipples, const I &weights, double scale) {
  int w = scale * weights.width();
  int h = scale * weights.height();

  uniform_int_distribution<int> randX(0, w-1);
  uniform_int_distribution<int> randY(0, h-1);
  uniform_int_distribution<int> randWeight(0, WEIGHT_MAX);
  
  while (stipples.size() < nStipples) {
    int x = randX(rng);
    int y = randY(rng);
    int weight = randWeight(rng);
    if (weight >= weights.at(y / scale, x / scale)) {
      stipples.insert(Point(x, y));
    }
  }
}

void preamble(ostream &out, int width, int height) {
  double pScale = min(595.0 / width, 841.0 / height);
  int w = ceil(width * pScale);
  int h = ceil(height * pScale);
  out << "%!PS-Adobe-3.0 EPSF-3.0" << endl;
  out << "%%BoundingBox: 0 0 " << w << " " << h << endl;
  out << "0 " << h << " translate " << pScale << " -" << pScale << " scale " << endl;
}

void postamble(ostream &out) {
  out << "showpage" << endl << flush;
}

template<typename WRBS>
void fillScaledStipples(ostream &out, WRBS &weightedRegionsBySite, double rMin = 0.0) {
  for (auto i = weightedRegionsBySite.begin(); i != weightedRegionsBySite.end(); ++i) {
    const Point &p = i->first;
    WeightedRegion &c = i->second;
    double r = sqrt(c.weight / (WEIGHT_MAX * M_PI));
    if (r >= rMin) {
      out << p.x() << " " << p.y() << " " << r << " 0 360 arc fill" << endl;
    }
  }
}

template<typename WRBS>
void strokeScaledStipples(ostream &out, WRBS &weightedRegionsBySite, double lineWidth = 1.0) {
  out << lineWidth << " setlinewidth" << endl;
  
  for (auto i = weightedRegionsBySite.begin(); i != weightedRegionsBySite.end(); ++i) {
    const Point &p = i->first;
    WeightedRegion &c = i->second;
    double r = sqrt(c.weight / (WEIGHT_MAX * M_PI));
    if (r >= lineWidth) {
      int nCircles = ceil(r / lineWidth);
      for (int c = 0; c < nCircles; c++) {
        double drawR = r - (2 * c + 1) * (lineWidth / 2.0);
        if (drawR > 0) {
          out << p.x() << " " << p.y() << " " << drawR << " 0 360 arc " << endl;
        } else {
          out << p.x() << " " << p.y() << " moveto 0 0 rlineto " << endl;
        }
      }
      out << "stroke" << endl;
    }
  }
}

template<typename V>
void strokeTour(ostream &out, V &tour, double lineWidth = 1.0) {
  out << lineWidth << " setlinewidth" << endl;
  
  for (auto i = tour.begin(); i != tour.end(); i++) {
    out << i->x() << " " << i->y();
    out << (i == tour.begin() ? " moveto" : " lineto") << endl;
  }
  out << "stroke" << endl;
}


template<typename V, typename N>
void nearestNeighbourTour(V &tour, N &neighboursBySite) {
  unordered_set<Point> remaining;
  auto siteIterator = neighboursBySite.begin();
  Point p = (siteIterator++)->first;
  
  for (; siteIterator != neighboursBySite.end(); ++siteIterator) {
    remaining.insert(siteIterator->first);
  }
  
  tour.push_back(p);
  
  while (remaining.size() > 0) {
    Point q;
    double distance = numeric_limits<double>::infinity();
    unordered_set<Point> candidates = neighboursBySite[p];
    while (isinf(distance)) {
      for (auto i = candidates.begin(); i != candidates.end(); ++i) {
        if (remaining.find(*i) != remaining.end()) {
          double d = p.distance(*i);
          if (d < distance) {
            q = *i;
            distance = d;
          }
        }
      }
      if (isinf(distance)) {
        unordered_set<Point> newCandidates;
        for (auto i = candidates.begin(); i != candidates.end(); ++i) {
          auto &ns = neighboursBySite[*i];
          newCandidates.insert(ns.begin(), ns.end());
        }
        candidates = newCandidates;
      }
    }

    p = q;
    tour.push_back(p);
    remaining.erase(p);
  }
}

template<typename V>
void rotateToShortest(V &tour) {
  long jMax = -1;
  double dMax = 0;
  long i = tour.size() - 1;
  long j = 0;
  while (j < tour.size()) {
    double d = tour[i].distance(tour[j]);
    if (d > dMax) {
      dMax = d;
      jMax = j;
    }
    i = j;
    ++j;
  }
  rotateLeft(tour, 0, tour.size(), jMax);
}

int main(int argc, char **argv) {
  double scale = 1.0;
  int nStipples = 1000;
  double rMin = 0.0;
  int minWeight = 0;
  int threads = 8;
  
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
    } else if (checkArg(argc, argv, argn, "-threads", spec)) {
      threads = getInt(spec);
    } else if (checkArg(argc, argv, argn, "-seed", spec)) {
      rng.seed(getUnsignedInt(spec));
    } else if (checkArg(argc, argv, argn, "-rMin", spec)) {
      rMin = getDouble(spec);
      minWeight = floor(WEIGHT_MAX * M_PI * rMin * rMin);
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
  addRandomStipples(stipples, nStipples, *weights, scale);
  
  unordered_map<Point, uint8_t> regionGreys;
  VoronoiRef voronoi;
  unordered_map<Point, WeightedRegion> weightedRegionsBySite;
  BitmapRef sites = Bitmap::make(weights->width() * scale, weights->height() * scale, true);

  Workers workers(threads);
  
  int i = 0;
  double t0 = now();
  for (;;) {
    sites->clear();
    for (auto i = stipples.begin(); i != stipples.end(); ++i) {
      const Point &p = *i;
      sites->at(p) = true;
    }
    
    if (steps) {
      sites->writePng(stepper->makeName("sites.png"));
    }

    voronoi = sites->featureTransform(true, workers);

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
    
    weightedRegionsBySite.clear();
    calculateCentroids(*voronoi, *weights, weightedRegionsBySite, workers);
    
    int same = 0;
    int differ = 0;

    stipples.clear();
    
    for (unordered_map<Point, WeightedRegion>::iterator i = weightedRegionsBySite.begin();
         i != weightedRegionsBySite.end(); ++i) {
      const Point &p = i->first;
      WeightedRegion &c = i->second;
      if (c.weight > 0) {
        int x = round(p.x() + c.x());
        int y = round(p.y() + c.y());
        Point q(x, y);
        if (q == p) {
          ++same;
        } else {
          ++differ;
        }
        stipples.insert(q);
      } else {
        ++differ;
      }
    }
    if (stipples.size() < nStipples) {
      cerr << "-- remove " << (nStipples - stipples.size()) << " empty voronoi regions, adding replacements." << endl << flush;
      addRandomStipples(stipples, nStipples, *weights, scale);
    }
    
    cerr << "after " << i++ << " iterations: " << same << " same, " << differ << " differ" << endl << flush;
    if (differ == 0) {
      break;
    }
  }
  double t1 = now();
  cerr << "* done after " << i << " iterations in " << (t1 - t0) << " seconds with " << threads << " threads" << endl << flush;

  unordered_map<Point, unordered_set<Point> > neighboursBySite;

  getNeighbours(*voronoi, neighboursBySite, workers);
  vector<Point> tour;
  
  // start with a nearest-neighbour tour
  nearestNeighbourTour(tour, neighboursBySite);
  // now improve it using 2-opt
  continuousTwoOpt<Point>(tour);

  // find the adjacent sites with the greatest distance between them, and make those the start and end respectively
  rotateToShortest(tour);

  cerr << "Tour has " << tour.size() << " points" << endl << flush;

  preamble(cout, sites->width(), sites->height());
  fillScaledStipples(cout, weightedRegionsBySite);
  postamble(cout);
  
  // strokeScaledStipples(cout, weightedRegionsBySite);
  
  preamble(cout, sites->width(), sites->height());
  strokeTour(cout, tour);
  cout << "gsave 0 1 0 setrgbcolor " << tour.front().x() << " " << tour.front().y() << " translate -5 -5 10 10 rectfill grestore" << endl << flush;
  cout << "gsave 1 0 0 setrgbcolor " << tour.back().x() << " " << tour.back().y() << " translate -5 -5 10 10 rectfill grestore" << endl << flush;
  postamble(cout);

  exit(0);
  return 0;
}
