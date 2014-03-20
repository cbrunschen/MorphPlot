//
//  PlotterPathExtractor.h
//  MorphPlot
//
//  Created by Christian Brunschen on 08/02/2014.
//
//

#ifndef __PlotterPathExtractor_h__
#define __PlotterPathExtractor_h__

#include "PathExtractor.h"

namespace Extraction {
#if 0
}
#endif

class PlotterPathExtractor : public PathExtractor {
  
  typedef shared_ptr<Bitmap> BitmapRef;
    
public:
  BitmapRef outlineAndHatch(Chains &outlineChains, Chains &hatchedChains, BitmapRef remaining, int penRadius, double hatchAngle, double hatchPhase, Circle &penCircle, Image<int> marks) {
    
    if (steps()) {
      (*out_) << "    . drawing initial remainder" << endl;
      remaining->writePng(stepper_->makeName("remaining_before.png"));
    }
    
    (*out_) << "  - Insetting by " << penRadius << endl;
    BitmapRef inset(remaining->inset(penRadius));
    
    if (steps()) {
      (*out_) << "    . drawing inset" << endl;
      inset->writePng(stepper_->makeName("inset.png"));
    }
    
    (*out_) << "  - removing thin parts of inset" << endl;
    BitmapRef thinInsetReference = inset->clone();
    inset->clearThinConnected();
    inset = inset->reconstruct(thinInsetReference);
    
    if (steps()) {
      (*out_) << "    . drawing inset minus thin parts" << endl;
      inset->writePng(stepper_->makeName("inset_without_thin.png"));
    }
    
    (*out_) << "  - Scanning inset boundaries" << endl;
    marks.clear();
    vector<Boundary> boundaries;
    inset->scanBoundaries(boundaries, marks, false);
    
    (*out_) << "  - Outsetting again to generate coverage" << endl;
    BitmapRef covered(inset->outset(penRadius));
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
      outsetBoundaries->draw(*b, true, penCircle);
    }
    
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
    CanRetractWhenDeltaIsSetInReference canRetract(*outsetBoundaries, penCircle);
    inset->hatch(hatchedChains, hatchAngle, 2 * penRadius, hatchPhase, canRetract);
    (*out_) << "    . have " << hatchedChains.size() << " hatched chains" << endl;
    
    if (steps()) {
      (*out_) << "    . drawing hatched" << endl;
      Bitmap hatchedBitmap(remaining->width(), remaining->height());
      hatchedBitmap.copyRes(*remaining);
      
      hatchedBitmap.clear();
      hatchedBitmap.draw(hatchedChains, true, Circle(0));
      hatchedBitmap.writePng(stepper_->makeName("hatched.png"));
    }
    
    int minReconnect = 3 * penRadius;
    (*out_) << "  - reconnecting Hatched with max distance 3*r = " << minReconnect << endl;
    hatchedChains.reconnect(minReconnect, inset->at());
    (*out_) << "    . have " << hatchedChains.size() << " recommected chains" << endl;
    
    if (steps()) {
      (*out_) << "    . drawing hatched output" << endl;
      ofstream out;
      out.open(stepper_->makeName("hatched.ps"));
      preparePS(out, remaining->width(), remaining->height());
      drawChainPixels(out, hatchedChains);
      out << endl << "0.7 setgray" << endl;
      strokeBetween(out, hatchedChains);
      out << endl << "0 setgray" << endl;
      strokeChains(out, hatchedChains);
      finishPS(out);
      out.close();
    }
    
    (*out_) << "  - Merging outline chains" << endl;
    // merge the outline chaind
    for (vector<Boundary>::iterator b = boundaries.begin();
         b != boundaries.end();
         ++b) {
      outlineChains.addChains(*b);
    }
    
    return covered;
  }
  
  void thinAndStroke(Chains &thinChains, BitmapRef remaining, BitmapRef alreadyCovered, int penRadius, int insetBy, Circle &penCircle, Image<int> &marks) {
    (*out_) << "  - getting remaining pixels that are adjacent to covered" << endl;
    BitmapRef adjacent(alreadyCovered->adjacent(remaining));
    
    if (steps()) {
      (*out_) << "    . drawing adjacent pixels" << endl;
      adjacent->writePng(stepper_->makeName("adjacent.png"));
    }
    
    D((*out_) << "  - scanning adjacent pixels" << endl);
    vector<Boundary> adjacents;
    marks.clear();
    adjacent->scanBoundaries(adjacents, marks, false);
    
    set<Point> adjacentCircleCenters;
    unordered_map<Chain*, set<Point> > circleCentersByChain;
    
    D((*out_) << "    . adjacent pixel chains:" << endl);
    for (vector<Boundary>::iterator b = adjacents.begin();
         b != adjacents.end();
         ++b) {
      for (Boundary::iterator c = b->begin(); c != b->end(); ++c) {
        Chain &chain = *c;
        set<Point> &current = circleCentersByChain[&chain];
        penCircle.findFilteredOffsets(c->begin(), c->end(), alreadyCovered->get(),
                                      inserter(current, current.begin()));
        copy(current.begin(), current.end(),
             inserter(adjacentCircleCenters, adjacentCircleCenters.begin()));
      }
    }
    
    Bitmap::Set setRemaining = remaining->set(true);
    D((*out_) << "- adding adjacent circles with centers: " << adjacentCircleCenters << endl);
    for (set<Point>::iterator pi = adjacentCircleCenters.begin();
         pi != adjacentCircleCenters.end();
         ++pi) {
      penCircle.setAt(*pi, setRemaining);
    }
    
    if (steps()) {
      (*out_) << "    . drawing remainder with added adjacent circles" << endl;
      remaining->writePng(stepper_->makeName("remainder_added_circles.png"));
    }
    
    (*out_) << "  - opening remainder by " << insetBy << endl;
    BitmapRef remainingOpened(remaining->open(insetBy));
    
    if (steps()) {
      (*out_) << "    . drawing opened remainder" << endl;
      remainingOpened->writePng(stepper_->makeName("remainder_opened.png"));
    }
    
    (*out_) << "  - thinning " << endl;
    remainingOpened->thin();
    
    if (steps()) {
      (*out_) << "    . drawing thinned" << endl;
      remainingOpened->writePng(stepper_->makeName("thinned.png"));
    }
    
    map<Point, Point> linesToDraw;
    set<Point> pointsToDraw;
    (*out_) << "  - finding lines to draw from adjacent thinned points to circle centers" << endl;
    for (unordered_map<Chain*, set<Point> >::iterator pairIter = circleCentersByChain.begin();
         pairIter != circleCentersByChain.end();
         ++pairIter) {
      Point a(-1, -1), b(-1, -1);
      int minD2 = numeric_limits<int>::max();
      set<Point> &centres = pairIter->second;
      Chain *chain = pairIter->first;
      list<Point> thickened;
      Chain::iterator i = chain->begin();
      Point g = *i;
      thickened.push_back(*i++);
      for (; i != chain->end(); ++i) {
        const Point &h = *i;
        int d = g.directionTo(h);
        if ((d % 2) == 0) {
          // diagonal; add the two fill-in points
          thickened.push_back(g.neighbour((d + 1) % 8));
          thickened.push_back(g.neighbour((d + 7) % 8));
        }
        thickened.push_back(h);
        g = h;
      }
      for (list<Point>::iterator i = thickened.begin(); i != thickened.end(); ++i) {
        const Point &p = *i;
        if (remainingOpened->at(p)) {
          pointsToDraw.insert(p);
          for (set<Point>::iterator j = centres.begin(); j != centres.end(); ++j) {
            const Point &q = *j;
            int dx = q.x() - p.x();
            int dy = q.y() - p.y();
            int d2 = dx * dx + dy * dy;
            if (d2 < minD2) {
              minD2 = d2;
              a = p;
              b = q;
            }
          }
        }
      }
      if (minD2 < numeric_limits<int>::max()) {
        linesToDraw[a] = b;
      }
    }
    
    Bitmap::Set unsetRemainingInset = remainingOpened->set(false);
    D((*out_) << "- removing adjacent circles with centers: " << adjacentCircleCenters << endl);
    for (set<Point>::iterator pi = adjacentCircleCenters.begin();
         pi != adjacentCircleCenters.end();
         ++pi) {
      penCircle.setAt(*pi, unsetRemainingInset);
    }
    
    if (steps()) {
      (*out_) << "    . drawing remainder with removed adjacent circles" << endl;
      remainingOpened->writePng(stepper_->makeName("thinned_removed_circles.png"));
    }
    
    (*out_) << "  - drawing lines from adjacent thinned points to circle centers" << endl;
    for (map<Point, Point>::iterator pairIter = linesToDraw.begin();
         pairIter != linesToDraw.end();
         ++pairIter) {
      const Point &p = pairIter->first;
      const Point &q = pairIter->second;
      line(p.y(), p.x(), q.y(), q.x(), remainingOpened->set(true));
    }
    for (set<Point>::iterator pointIter = pointsToDraw.begin();
         pointIter != pointsToDraw.end();
         ++pointIter) {
      remainingOpened->set(*pointIter, true);
    }
    
    if (steps()) {
      (*out_) << "    . drawing remainder with lines to circle centers added" << endl;
      remainingOpened->writePng(stepper_->makeName("thinned_circle_center_lines_added.png"));
    }
    
    (*out_) << "  - retracting thinned" << endl;
    remainingOpened->retract(remaining, penCircle);
    
    if (steps()) {
      (*out_) << "    . drawing retracted thinned" << endl;
      remainingOpened->writePng(stepper_->makeName("thinned_retracted.png"));
    }
    
    (*out_) << "  - scanning thinned & pruned skeletons" << endl;
    vector<Boundary> skeletons;
    marks.clear();
    remainingOpened->scanBoundaries(skeletons, marks, false);
    
    (*out_) << "  - add skeleton chains to existing set" << endl;
    for (vector<Boundary>::iterator b = skeletons.begin();
         b != skeletons.end();
         ++b) {
      thinChains.addChains(*b);
    }
  }
  
  template<typename C, typename Pens, typename DitheredByPen>
  void outlineHatchThinDither(const shared_ptr<GreyImage<C> > separated, const Pens &pens, DitheredByPen &ditheredByPen, double tolerance = 0.9) {
    (*out_) << "- Generating coverage" << endl;
    BitmapRef remaining(separated->coverage());
    BitmapRef covered(Bitmap::make(remaining->width(), remaining->height(), true));
    covered->copyRes(remaining);
    
    typename Pens::const_iterator cBegin = pens.begin();
    typename Pens::const_iterator i = cBegin;
    typename Pens::const_iterator cEnd = pens.end();
    
    typename Pens::const_iterator current;
    while (i != cEnd) {
      if (steps()) {
        stepper_->setPen(i->unparse());
      }
      
      Chains outlineChains, hatchChains, thinChains;
      
      (*out_) << "- Working on Pen " << i->unparse() << endl;
      current = i;
      int penRadius = i->r();
      Circle penCircle(penRadius);
      Image<int> marks(remaining->width(), remaining->height());
      
      BitmapRef workspace = remaining->clone();
      
      if (i != cBegin) {
        *workspace -= *(covered->inset(penRadius));
      }
      
      if (steps()) {
        (*out_) << "    . drawing workspace" << endl;
        workspace->writePng(stepper_->makeName("workspace.png"));
      }
      
      *covered += *(outlineAndHatch(outlineChains, hatchChains, workspace, penRadius, i->hatchAngle(), i->hatchPhase(), penCircle, marks));
      
      if (steps()) {
        (*out_) << "    . drawing workspace" << endl;
        workspace->writePng(stepper_->makeName("workspace.png"));
        (*out_) << "    . drawing covered" << endl;
        covered->writePng(stepper_->makeName("covered.png"));
      }

      ++i;
      int thinRadius = 0;
      if (i == cEnd) {
        // the current pen is the last pen with this colour; have it
        // handle anything that remains as skeletons
        (*out_) << "  + last pen this colour" << endl;
        int rMin = current->rMin();
        if (rMin > penRadius) rMin = penRadius;
        thinRadius = rMin < 0 ? (penRadius + 1) / 2 : rMin;
        
        *workspace -= *covered;
        
        thinAndStroke(thinChains, workspace, covered, penRadius, thinRadius, penCircle, marks);
      }
      
      if (steps()) {
        (*out_) << "    . creating bitmap of chain pixels" << endl;
        BitmapRef chainBitmap = Bitmap::make(remaining->width(), remaining->height(), true);
        chainBitmap->copyRes(remaining);
        
        chainBitmap->set(outlineChains, true);
        chainBitmap->set(hatchChains, true);
        chainBitmap->set(thinChains, true);
        chainBitmap->writePng(stepper_->makeName("chains.png"));
      }
      
      Chains dithered;
      (*out_) << "  - dithering" << endl;
      separated->dither(dithered, outlineChains, penCircle);
      
      if (i == cEnd && !thinChains.empty()) {
        // we also have thin stuff.
        Circle thinCircle(thinRadius);
        separated->dither(dithered, thinChains, thinCircle);
        
        BitmapRef combinedOutlineAndThin = Bitmap::make(remaining->width(), remaining->height(), true);
        combinedOutlineAndThin->copyRes(remaining);
        
        combinedOutlineAndThin->set(outlineChains, true);
        combinedOutlineAndThin->set(thinChains, true);
        
        vector<Boundary> rescanned;
        marks.clear();
        combinedOutlineAndThin->scanBoundaries(rescanned, marks, false);
        
        BitmapRef ditheredOnly = Bitmap::make(remaining->width(), remaining->height(), true);
        ditheredOnly->copyRes(remaining);
        
        ditheredOnly->set(dithered, true);
        dithered.clear();
        
        for (vector<Boundary>::iterator b = rescanned.begin(); b != rescanned.end(); ++b) {
          for (Boundary::iterator i = b->begin(); i != b->end(); ++i) {
            Chain *currentChain = NULL;
            for (Chain::iterator j = i->begin(); j != i->end(); ++j) {
              const Point &p = *j;
              if (ditheredOnly->at(p)) {
                if (currentChain == NULL) {
                  dithered.push_back(Chain());
                  currentChain = &(dithered.back());
                }
                currentChain->push_back(p);
              } else {
                currentChain = NULL;
              }
            }
          }
        }
      }
      separated->dither(dithered, hatchChains, penCircle);
      
      if (steps()) {
        (*out_) << "    . creating bitmap of dithered pixels" << endl;
        BitmapRef ditheredBitmap = Bitmap::make(remaining->width(), remaining->height(), true);
        ditheredBitmap->copyRes(remaining);
        
        ditheredBitmap->set(dithered, true);
        ditheredBitmap->writePng(stepper_->makeName("dithered.png"));
      }
      
      if (approximate()) {
        (*out_) << "    . creating dithered outset bitmap" << endl;
        BitmapRef ditheredOutsetBitmap = Bitmap::make(remaining->width(), remaining->height(), true);
        ditheredOutsetBitmap->copyRes(remaining);
        
        ditheredOutsetBitmap->set(dithered, true, penRadius);
        
        (*out_) << "    . adding to approximation" << endl;
        approximator()->addToApproximation(ditheredOutsetBitmap);
        
        if (steps()) {
          (*out_) << "    . drawing approximation so far" << endl;
          approximator()->approximation().writePng(stepper_->makeName("approx.png"));
        }
      }
      
      (*out_) << "  - simplifying with tolerance " << tolerance << endl;
      dithered.simplify(tolerance);
      
      if (steps()) {
        (*out_) << "  - drawing simplified PS" << endl;
        ofstream out;
        out.open(stepper_->makeName("finished.ps"));
        preparePS(out, separated->width(), separated->height());
        out << endl << "0.7 setgray" << endl;
        strokeBetween(out, dithered);
        out << endl << "0 setgray" << endl;
        strokeChains(out, dithered);
        finishPS(out);
        out.close();
      }
      
      (*out_) << "  - adding simplified to map" << endl;
      ditheredByPen[*current].splice(ditheredByPen[*current].end(), dithered);
    }
  }
};

#if 0
{
#endif
}

#endif  // __PlotterPathExtractor_h__
