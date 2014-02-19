/*
 *  TestSingleConnected.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 05/06/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "TestSingleConnected.h"

#define DEBUG 1
#include "catch.hpp"
#undef DEBUG

#include "Primitives.h"
#include "Bitmap.h"
#include "Bitmap_Impl.h"

#include <iterator>
#include <list>
#include <map>
#include <set>
#include <utility>
#include <iomanip>

#include <stdarg.h>

using namespace Primitives;
using namespace Images;
using namespace std;

TEST_CASE("neigbourhood/offsets", "Verify the x and y offsets")
{
  int nonzeroX = 0, nonzeroY = 0;
  for (int i = 0; i <= DIRECTIONS; i++) {
    if (xOffsets[i] != 0) nonzeroX++;
    if (yOffsets[i] != 0) nonzeroY++;
  }
  REQUIRE(nonzeroX == 6);
  REQUIRE(nonzeroY == 6);
}

TEST_CASE("neighbourhood/single connected", "generate single connected patterns")
{
  for (int n = 0; n < (1 << DIRECTIONS); n++) {
    map<int, set<int>*> connectedByDirection;
    set<set<int>*> sets;
    for (int dir = NW; dir < DIRECTIONS; dir++) {
      if (n & (1 << dir)) {
        // always check immediate predecessor
        int checkDir = (DIRECTIONS + dir - 1) % DIRECTIONS;
        set<int> *matching = NULL;
        if (n & (1 << checkDir)) {
          if (connectedByDirection.find(checkDir) != connectedByDirection.end()) {
            matching = connectedByDirection[checkDir];
          }
        }
          
        if (dir % 2 && (matching == NULL)) {
          // odd => horizontal or vertical; check diagonally as well
          checkDir = (DIRECTIONS + dir - 2) % DIRECTIONS;

          if (n & (1 << checkDir)) {
            if (connectedByDirection.find(checkDir) != connectedByDirection.end()) {
              matching = connectedByDirection[checkDir];
            }
          } 
        }
        
        if (matching == NULL) {
          matching = new set<int>();
          sets.insert(matching);
        }

        matching->insert(dir);
        connectedByDirection[dir] = matching;
      }
    }
    
    if ((n & (1 << NW | 1 << N)) && (n & 1 << W) && sets.size() > 1) {
      set<int>* existing = n & 1 << NW 
          ? connectedByDirection[NW] : connectedByDirection[N];
      set<int>* toMerge = connectedByDirection[W];
      connectedByDirection[W] = existing;
      existing->insert(toMerge->begin(), toMerge->end());
      sets.erase(toMerge);
      delete toMerge;
    }
    
    cout << "  /* " << setw(3) << n << " */ ";
    
    bool expected = sets.size() == 1;
    REQUIRE(Neighbourhood::isSingleConnected(n) == expected);

    if (expected) {
      // a single set of connected pixels
      cout << "true,  ";
    } else {
      cout << "false, ";
    }
    
    cout << "// ";
    bool first = true;
    for (set<set<int>*>::iterator ii = sets.begin(); ii != sets.end(); ++ii) {
      if (first) first = false; else cout << " ";
      for (set<int>::iterator jj = (*ii)->begin(); jj != (*ii)->end(); ++jj) {
        cout << *jj;
      }
    }
    cout << endl;
    
    for (set<set<int>*>::iterator ii = sets.begin(); ii != sets.end(); ++ii) {
      delete *ii;
    }
  }
}
