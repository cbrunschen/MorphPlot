/*
 *  PathExtractor.h
 *  Morph
 *
 *  Created by Christian Brunschen on 27/11/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#ifndef __PathExtractor_h__
#define __PathExtractor_h__

#include "Primitives.h"
#include "Bitmap.h"
#include "GreyImage.h"
#include "Chain.h"
#include "Progress.h"
#include <iostream>
#include <list>
#include <map>

using namespace std;
using namespace Images;
using namespace Primitives;
using namespace Progress;

namespace Extraction {
#if 0
}
#endif

class PathExtractor {
protected:
  Stepper *stepper_;
  Approximator *approximator_;
  ostream *out_;

public:
  PathExtractor() : stepper_(NULL), approximator_(NULL), out_(NULL) {}
  
  void setStepper(Stepper *stepper) { stepper_ = stepper; }
  Stepper *stepper() { return stepper_; }
  bool steps() { return stepper_ != NULL; }
  
  void setApproximator(Approximator *approximator) { approximator_ = approximator; }
  Approximator *approximator() { return approximator_; }
  bool approximate() { return approximator_ != NULL; }
  
  void setOut(ostream *out) { out_ = out; }
  ostream *out() { return out_; }
};

class CanRetractWhenDeltaIsSetInReference {
  const Bitmap &reference_;
  const Circle &circle_;
public:
  CanRetractWhenDeltaIsSetInReference(const Bitmap &thinInsetReference, const Circle &circle)
  : reference_(thinInsetReference), circle_(circle) { }
  
  bool operator()(const IPoint &p, int neighbours) const {
    const list<IPoint> &delta = circle_.getDelta(neighbours);
    
    bool result = true;
    for (list<IPoint>::const_iterator i = delta.begin();
         result && i != delta.end();
         ++i) {
      IPoint q(p + *i);
      result = reference_.get(q);
    }
    return result;
  }
};


extern ostream &preparePS(ostream &a, int width, int height);

extern ostream &drawChainPixels(ostream &a, Chains &chains);

extern ostream &strokeChains(ostream &a, Chains chains);

extern ostream &strokeBetween(ostream &a, Chains chains);

extern void finishPS(ostream &a);

#if 0
{
#endif
}

#endif // __PathExtractor_h__

