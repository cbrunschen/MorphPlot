/*
 *  PathExtractor.cpp
 *  Morph
 *
 *  Created by Christian Brunschen on 27/11/2011.
 *  Copyright 2011 Christian Brunschen. All rights reserved.
 *
 */

#include "PathExtractor.h"

namespace Extraction {
#if 0
}
#endif

ostream &preparePS(ostream &out, int width, int height) {
  out << "%!PS-Adobe-3.0" << endl;
  out << "%%BoundingBox: 0 0 " << width << " " << height << endl;
  out << "%%Page: only 1" << endl;
  out << "gsave initclip clippath pathbbox grestore" << endl;
  out << "/ury exch def /urx exch def /lly exch def /llx exch def" << endl;
  out << "/W urx llx sub def /H ury lly sub def" << endl;
  out << "/w " << width << " def /h " << height << " def" << endl;
  out << "/xs W w div def /ys H h div def" << endl;
  out << "/s xs ys lt { xs } { ys } ifelse def" << endl;
  out << "llx urx add 2 div lly ury add 2 div translate" << endl;
  out << "s dup scale w 2 div neg h 2 div translate 1 -1 scale" << endl;
  out << "1 setlinejoin 1 setlinecap" << endl;
  return out;
}

ostream &drawChainPixels(ostream &out, Chains &chains) {
  out << "[" << endl;
  int n = 200;
  for (Chains::iterator j = chains.begin();
       j != chains.end();
       ++j) {
    for (Chain::iterator k = j->begin(); k != j->end(); ++k) {
      out << k->x() << " " << k->y() << " 1 1 " ;
      if (--n == 0) {
        out << " ] dup 0.8 setgray rectfill 0.3 setgray rectstroke [ " << endl;
        n = 200;
      }
    }
  }
  out << " ] dup 0.8 setgray rectfill 0.3 setgray rectstroke" << endl;
  out << "0 setgray 0.5 0.5 translate 0.2 setlinewidth" << endl;
  return out;
}

ostream &strokeChains(ostream &out, Chains chains) {
  for (Chains::iterator j = chains.begin();
       j != chains.end();
       ++j) {
    Chain::iterator k = j->begin();
    out << k->x() << " " << k->y() << " moveto currentpoint lineto ";
    out << "gsave currentpoint translate newpath -0.3 -0.3 0.6 0.6 rectfill grestore ";
    for (++k; k != j->end(); ++k) {
      out << k->x() << " " << k->y() << " lineto ";
    }
    out << "stroke" << endl << flush;
  }
  return out;
}

ostream &strokeBetween(ostream &out, Chains chains) {
  Chains::iterator j = chains.begin();
  Point &prev = j->back();
  for (++j;
       j != chains.end();
       ++j) {
    out << prev.x() << " " << prev.y() << " moveto ";
    out << j->front().x() << " " << j->front().y() << " lineto ";
    prev = j->back();
  }
  out << "stroke" << endl << flush;
  return out;
}

void finishPS(ostream &out) {
  out << endl << "showpage" << endl;
}

#if 0
{
#endif
}


