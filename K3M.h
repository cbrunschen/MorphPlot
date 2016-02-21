/*
 *  K3M.h
 *  Morph
 *
 *  Created by Christian Brunschen on 26/11/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __K3M_h__
#define __K3M_h__

#include <set>
#include "Primitives.h"
#include "Bitmap.h"

using namespace std;
using namespace Primitives;
using namespace Images;
using namespace Neighbourhood;

namespace K3M {
#if 0
}
#endif

extern bool A0[256];
extern bool A1[256];
extern bool A2[256];
extern bool A3[256];
extern bool A4[256];
extern bool A5[256];
extern bool A1pix[256];

template<typename B> int k3m_subiter(B &bitmap, set<IPoint> &candidates, set<IPoint> &nextCandidates, bool (&a)[256]) {
  list<IPoint> removed;
  for (set<IPoint>::iterator i = candidates.begin(); i != candidates.end(); ++i) {
    const IPoint &p = *i;
    if (bitmap.at(p)) {
      int n = bitmap.neighbours(p);
      if (a[n]) {
        bitmap.set(p, false);
        removed.push_back(p);
        for (int d = 0; d < DIRECTIONS; d++) {
          if (n & (1 << d)) {
            nextCandidates.insert(p.neighbour(d));
          }
        }
      }
    }
  }
  for (list<IPoint>::iterator i = removed.begin(); i != removed.end(); ++i) {
    candidates.erase(*i);
  }
  cerr << "  . subiteration removed " << removed.size() << endl;
  return static_cast<int>(removed.size());
}

template<typename B> int k3m_iter(B &bitmap, set<IPoint> &candidates, set<IPoint> &nextCandidates) {
  int removed = 0;
  removed += k3m_subiter(bitmap, candidates, nextCandidates, A1);
  removed += k3m_subiter(bitmap, candidates, nextCandidates, A2);
  removed += k3m_subiter(bitmap, candidates, nextCandidates, A3);
  removed += k3m_subiter(bitmap, candidates, nextCandidates, A4);
  removed += k3m_subiter(bitmap, candidates, nextCandidates, A5);
  cerr << " * full iteration removed " << removed << endl;
  return removed;
}

template<typename B> void k3m(B &bitmap) {  
  // First, find the initial set of candidates;
  set<IPoint> candidates;
  for (int y = 0; y < bitmap.height(); y++) {
    for (int x = 0; x < bitmap.width(); x++) {
      IPoint p(x, y);
      int n = bitmap.neighbours(p);
      if (A0[n]) {
        candidates.insert(p);
      }
    }
  }
  
  cerr << " * found " << candidates.size() << " border pixels" << endl;
  
#if DEBUG_THIN
  char filename[1024];
  int iterations = 0;
  sprintf(filename, "/tmp/thin_%03d.png", iterations);
  bitmap.writePng(filename);
#endif  
  
  // keep iterating until an iteration removes no points. Candidates for the next iteration 
  // are un-removed candidates from the previous iteration plus neighbours of removed 
  while (true) {
    cerr << " * considering " << candidates.size() << " candidate pixels" << endl;
    
    set<IPoint> nextCandidates;
    int removed = k3m_iter(bitmap, candidates, nextCandidates);
    
#if DEBUG_THIN
    sprintf(filename, "/tmp/thin_%03d.png", ++iterations);
    bitmap.writePng(filename);
#endif  
    
    if (removed > 0) {
      nextCandidates.insert(candidates.begin(), candidates.end());
      candidates.clear();
      for (set<IPoint>::iterator i = nextCandidates.begin(); i != nextCandidates.end(); ++i) {
        const IPoint p = *i;
        if (bitmap.get(p)) {
          int n = bitmap.neighbours(p);
          if (A0[n]) {
            candidates.insert(p);
          }
        }
      }
      nextCandidates.clear();
    } else {
      break;
    }
  }
  
  int removed = 0;
  // now run the 1-pixel pass
  for (int y = 0; y < bitmap.height(); y++) {
    for (int x = 0; x < bitmap.width(); x++) {
      IPoint p(x, y);
      int n = bitmap.neighbours(p);
      if (A1pix[n]) {
        bitmap.set(p, false);
        ++removed;
      }
    }
  }  
  cerr << " * removed " << removed << " for single-pixel thinness" << endl;
}

#if 0
{
#endif
}

#endif // __K3M_h__
