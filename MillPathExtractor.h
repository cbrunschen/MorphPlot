//
//  MillPathExtractor.h
//  MorphPlot
//
//  Created by Christian Brunschen on 08/02/2014.
//
//

#ifndef __MillPathExtractor_h__
#define __MillPathExtractor_h__

#include "PathExtractor.h"
#include "Workers.h"

namespace Extraction {
#if 0
}
#endif

class MillPathExtractor : public PathExtractor {

  typedef shared_ptr<Bitmap> BitmapRef;

public:
  BitmapRef outlineAndFill(BitmapRef remaining, int toolRadius, int extraInset, Chains &outlineChains, Chains &fillChains, Workers &workers) {
    Image<int> marks(remaining->width(), remaining->height());
    Circle toolCircle(toolRadius);

    if (steps()) {
      (*out_) << "    . drawing initial remainder" << endl;
      remaining->writePng(stepper_->makeName("remaining_before.png"));
    }

    int totalInset = toolRadius + extraInset;
    (*out_) << "  - Insetting by " << toolRadius << " + " << extraInset << " = " << totalInset << endl;
    BitmapRef inset(remaining->inset(totalInset, workers));

    if (steps()) {
      (*out_) << "    . drawing inset" << endl;
      inset->writePng(stepper_->makeName("inset.png"));
    }

    (*out_) << "  - Scanning inset boundaries" << endl;
    marks.clear();
    vector<Boundary> boundaries;
    inset->scanBoundaries(boundaries, marks, false);

    (*out_) << "  - Outsetting again to generate coverage" << endl;
    BitmapRef covered = inset->outset(toolRadius, workers);
    if (steps()) {
      (*out_) << "    . drawing outset" << endl;
      covered->writePng(stepper_->makeName("covered.png"));
    }

    (*out_) << "  - Clearing scanned boundary pixels" << endl;
    for (vector<Boundary>::iterator b = boundaries.begin(); b != boundaries.end(); ++b) {
      inset->set(*b, false);
    }

    // create the outset boundaries
    (*out_) << "  - Creating outset boundary pixel bitmap" << endl;
    BitmapRef outsetBoundaries = Bitmap::make(remaining->width(), remaining->height(), true);
    for (vector<Boundary>::iterator b = boundaries.begin(); b != boundaries.end(); ++b) {
      outsetBoundaries->set(*b, true);
    }
    outsetBoundaries = outsetBoundaries->outset(toolRadius, workers);

    (*out_) << "  - removing covered-by-boundary parts of inset" << endl;
    BitmapRef boundaryInsetReference = inset->clone();
    *inset -= *outsetBoundaries;
    inset = inset->reconstruct(boundaryInsetReference);

    if (steps()) {
      (*out_) << "    . drawing outset boundaries" << endl;
      outsetBoundaries->writePng(stepper_->makeName("outset_boundaries.png"));

      (*out_) << "    . drawing to-be-hatched" << endl;
      inset->writePng(stepper_->makeName("to_be_hatched.png"));
    }

    (*out_) << "  - Hatching" << endl;
    CanRetractWhenDeltaIsSetInReference canRetract(*outsetBoundaries, toolCircle);
    inset->hatch(fillChains, 0, toolRadius, 0, canRetract);
    (*out_) << "    . have " << fillChains.size() << " hatched chains" << endl;

    if (steps()) {
      (*out_) << "    . drawing hatched" << endl;
      Bitmap hatchedBitmap(remaining->width(), remaining->height());
      hatchedBitmap.copyRes(*remaining);

      hatchedBitmap.clear();
      hatchedBitmap.draw(fillChains, true, Circle(0));
      hatchedBitmap.writePng(stepper_->makeName("hatched.png"));
    }

    int minReconnect = 3 * toolRadius;
    (*out_) << "  - reconnecting Hatched with max distance 3*r = " << minReconnect << endl;
    fillChains.reconnect(minReconnect, inset->at());
    (*out_) << "    . have " << fillChains.size() << " recommected chains" << endl;

    if (steps()) {
      (*out_) << "    . drawing hatched output" << endl;
      ofstream out;
      out.open(stepper_->makeName("hatched.ps"));
      preparePS(out, remaining->width(), remaining->height());
      drawChainPixels(out, fillChains);
      out << endl << "0.7 setgray" << endl;
      strokeBetween(out, fillChains);
      out << endl << "0 setgray" << endl;
      strokeChains(out, fillChains);
      finishPS(out);
      out.close();
    }

    (*out_) << "  - Merging outline chains" << endl;
    // merge the outline chains
    for (vector<Boundary>::iterator b = boundaries.begin();
         b != boundaries.end();
         ++b) {
      outlineChains.moveChains(*b);
    }

    return covered;
  }

  BitmapRef outlineConcentric(BitmapRef remaining, int toolRadius, int extraInset, Chains &outlineChains, Chains &fillChains, Workers &workers) {
    Image<int> marks(remaining->width(), remaining->height());
    Circle toolCircle(toolRadius);

    if (steps()) {
      (*out_) << "    . drawing initial remainder" << endl;
      remaining->writePng(stepper_->makeName("remaining_before.png"));
    }

    (*out_) << "  - Calculating distance transform" << endl;
    shared_ptr< GreyImage<int> > dt = remaining->distanceTransform(false, workers);

    vector<Boundary> boundaries;

    for(int rInset = toolRadius + extraInset; ; rInset += toolRadius) {
      (*out_) << "  - Extracting inset at " << rInset << endl;
      shared_ptr<Bitmap> inset = dt->ge(rInset*rInset, workers);

      if (inset->isEmpty()) {
        (*out_) << "inset is empty, done!" << endl << flush;
        break;
      }

      (*out_) << "  - Scanning inset boundaries" << endl;
      marks.clear();
      inset->scanBoundaries(boundaries, marks, false);
    }

    // create the outset boundaries
    (*out_) << "  - Creating outset boundary pixel bitmap" << endl;
    BitmapRef boundariesBitmap = Bitmap::make(remaining->width(), remaining->height(), true);
    for (vector<Boundary>::iterator b = boundaries.begin(); b != boundaries.end(); ++b) {
      boundariesBitmap->set(*b, true);
    }

    if (steps()) {
      (*out_) << "    . drawing boundaries" << endl;
      boundariesBitmap->copyRes(*remaining);
      boundariesBitmap->writePng(stepper_->makeName("boundaries.png"));
    }

    (*out_) << "  - Creating coverage bitmap" << endl << flush;
    shared_ptr<Bitmap> covered = boundariesBitmap->outset(toolRadius, workers);

    (*out_) << "  - Merging outline chains" << endl;
    // merge the outline chains
    for (vector<Boundary>::iterator b = boundaries.begin();
         b != boundaries.end();
         ++b) {
      outlineChains.moveChains(*b);
    }

    return covered;
  }
};

#if 0
{
#endif
}

#endif  // __MillPathExtractor_h__
