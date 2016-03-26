#include <iostream>
#include <fstream>
#include <stdint.h>
#include <limits>
#include <cmath>
#include <vector>
#include <list>

using namespace std;

#define FAKE_DATA 0

#include "Primitives.h"
#include "Image.h"
#include "GreyImage.h"
#include "ColorImage.h"
#include "Bitmap.h"
#include "Chain.h"
#include "Line.h"

using namespace Images;

class Printer {
  ostream &out_;
public:
  Printer(ostream &out) : out_(out) { }
  void operator() (int x, int y) {
    out_ << "[" << x << " " << y << " 1 1] ";
  }
};

ostream &preparePS(ostream &out, int width, int height) {
  out << "%!PS-Adobe-3.0" << endl;
  out << "%%BoundingBox: 0 0 " << width << " " << height << endl;
  out << "%%Page: only 1" << endl;
  out << "0 " << height << " translate 1 -1 scale 0.1 setlinewidth 1 setlinejoin 1 setlinecap" << endl;
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

void verifyChains(Chains &chains) {
  cerr << "verifying chains:" << endl;
  for (Chains::iterator hc = chains.begin(); hc != chains.end(); ++hc) {
    if (hc->size() > 1) {
      Chain::iterator j = hc->begin();
      Point p = *j;
      for (++j; j != hc->end(); ++j) {
        if (!p.isNeighbour(*j)) {
          cerr << "not neighbours: " << p << " <-> " << *j << endl;
        }
        p = *j;
      }
    }
  }
}

int main(int argc, char * const argv[]) {
  pm_init(argv[0], 0);
  
  typedef uint8_t C;
  typedef ColorImage<C> ColorImage;
  typedef RGBPixel<C> RGBPixel;
  typedef GreyImage<C> GreyImage;
  typedef Pen<C> Pen;

  cerr << "* Reading image '" << argv[1] << "'" << endl;
  
  ColorImage colored = ColorImage::readPpm(argv[1]);
#if APPROX
  ColorImage approx(colored.width(), colored.height());
  RGBPixel whitePixel(1.0, 1.0, 1.0);
  approx.set(whitePixel);
#endif // APPROX
  
  Pen cyan(1.0, 0.0, 0.0, 20);  
  Pen magenta(0.0, 1.0, 0.0, 20);
  Pen yellow(0.0, 0.0, 1.0, 20);
  Pen black(1.0, 1.0, 1.0, 20);
  Pen blackFine(1.0, 1.0, 1.0, 7);
  Pen red(0.0, 1.0, 1.0, 20);
  Pen green(1.0, 0.0, 1.0, 20);
  Pen blue(1.0, 1.0, 0.0, 20);

  typedef list<Pen> Pens;
  typedef list<Pens> Carousel;
  Carousel carousel;
  Pens blackPens; blackPens.push_back(black); blackPens.push_back(blackFine); carousel.push_back(blackPens);
  Pens redPens; redPens.push_back(red); carousel.push_back(redPens);
  Pens greenPens; greenPens.push_back(green); carousel.push_back(greenPens);
  Pens bluePens; bluePens.push_back(blue); carousel.push_back(bluePens);
  Pens cyanPens; cyanPens.push_back(cyan); carousel.push_back(cyanPens);
  Pens magentaPens; magentaPens.push_back(magenta); carousel.push_back(magentaPens);
  Pens yellowPens; yellowPens.push_back(yellow); carousel.push_back(yellowPens);

  double dTheta = 1.0 / carousel.size();
  double theta = dTheta / 3.0;
  
  int p = 0;
  for (Carousel::iterator c = carousel.begin(); c != carousel.end(); ++c) {
    Pens::iterator i = c->begin();
    if (i == c->end()) continue;
    const PenColor<C> &color = i->color();
    cerr << "- Separating out colour " << color << endl;
    GreyImage separated = colored.separateAndSubtract(color);
    cerr << "- Generating coverage" << endl;
    Bitmap remaining = separated.coverage();

    while (i != c->end()) {
      cerr << "- Working on Pen " << i->unparse() << endl;
      Pens::iterator current = i;
      ofstream out;

      string prefix = "/tmp/pen_";
      prefix += i->unparse();
      prefix += "_";
      
      int penRadius = i->r();
      double phase = p * penRadius / carousel.size();

      remaining.writePbm(prefix + "remaining_before.pbm");
      cerr << "  - Insetting by " << penRadius << endl;
      Bitmap inset(remaining.inset(penRadius));
      inset.writePbm(prefix + "inset.pbm");
      
      Chains mergedChains;

      cerr << "  - Scanning inset boundaries" << endl;
      Image<int> marks(inset.width(), inset.height());
      vector<Boundary> boundaries;
      inset.scanBoundaries(boundaries, marks, false);
      
      cerr << "  - Outsetting again to generate coverage" << endl;
      Bitmap covered(inset.outset(penRadius));
      covered.writePbm(prefix + "covered.pbm");
      
      remaining -= covered;
      remaining.writePbm(prefix + "remaining_after.pbm");
      
      cerr << "  - Clearing scanned boundary pipxels" << endl;
      for (vector<Boundary>::iterator b = boundaries.begin(); b != boundaries.end(); ++b) {
        inset.set(*b, false);
      }

      Chains hatchedChains;
      cerr << "  - Hatching" << endl;
      inset.hatch(hatchedChains, theta, 2 * penRadius, phase);

      
      out.open((prefix + "hatched.ps").c_str());
      preparePS(out, remaining.width(), remaining.height());
      drawChainPixels(out, hatchedChains);
      out << endl << "0.7 setgray" << endl;
      strokeBetween(out, hatchedChains);
      out << endl << "0 setgray" << endl;
      strokeChains(out, hatchedChains);
      finishPS(out);
      out.close();

      cerr << "  - Merging chains" << endl;
      // merge everything
      for (vector<Boundary>::iterator b = boundaries.begin();
           b != boundaries.end();
           ++b) {
        mergedChains.addChains(*b);
      }
            
      mergedChains.addChains(hatchedChains);
      
      ++i;
      if (i == c->end()) {
        // the current pen is the last pen with this colour; have it
        // handle anything that remains as skeletons
        cerr << "  + last pen this colour" << endl;
        int insetBy = (penRadius + 1) / 2;
        int pruneBy = (penRadius + 1) / 2 - 1;
        cerr << "  - insetting remainder by " << insetBy << endl;
        remaining.inset(insetBy);
        cerr << "  - thinning " << endl;
        remaining.thin();
        remaining.writePbm(prefix + "thinned.pbm");
        cerr << "  - pruning by " << pruneBy << endl;
        remaining.prune(pruneBy);
        remaining.writePbm(prefix + "pruned.pbm");
        
        vector<Boundary> skeletons;
        marks.clear();
        remaining.scanBoundaries(skeletons, marks, false);

        for (vector<Boundary>::iterator b = skeletons.begin();
             b != skeletons.end();
             ++b) {
          mergedChains.addChains(*b);
        }
      }
      
      Chains dithered;
      cerr << "  - dithering" << endl;
      separated.dither(dithered, mergedChains, penRadius);

#if APPROX
      cerr << "  - creating dithered outset:" << endl;
      cerr << "    . creating bitmap of dithered pixels" << endl;
      Bitmap ditheredBitmap(remaining.width(), remaining.height(), true);
      ditheredBitmap.set(dithered, true, penRadius);

      cerr << "    . adding to approximation" << endl;
      for (int y = 0; y < covered.height(); y++) {
        for (int x = 0; x < covered.width(); x++)  {
          if (ditheredBitmap.at(x, y)) {
            RGBPixel &p = approx.at(x, y);
            p.r() = Component<C>::make(Component<C>::fraction(p.r()) - Component<C>::fraction(current->c()));
            p.g() = Component<C>::make(Component<C>::fraction(p.g()) - Component<C>::fraction(current->m()));
            p.b() = Component<C>::make(Component<C>::fraction(p.b()) - Component<C>::fraction(current->y()));
          }
        }
      }
      approx.writePpm((prefix + "approx.ppm").c_str());
#endif
  
      cerr << "  - simplifying" << endl;
      dithered.simplify(0.9);
      
      string name(prefix);
      name.append("finished.ps");
      out.open(name.c_str());
      preparePS(out, separated.width(), separated.height());
      strokeChains(out, dithered);
      finishPS(out);
      out.close();
    }
    
    ++p;
  }

#if APPROX
  approx.writePpm("/tmp/approx.ppm");
#endif // APPROX
}

int separate(int argc, char * const argv[]) {
#if FAKE_DATA
  int N = 10;
  int width = N * N;
  int height = N;
  ColorImage colored(width, height);
  for (int y = 0; y < colored.height(); y++) {
    for (int x = 0; x < colored.width(); x++) {
      int block = x / N;
      int column = x % N;
      colored[y][x].r() = UINT8_MAX * (double)block / N;
      colored[y][x].g() = UINT8_MAX * (double)column / N;
      colored[y][x].b() = UINT8_MAX * (double)y / N;
    }
  }
#else
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  ColorImage<uint8_t> colored(width, height);
  ifstream in(argv[3]);
  for (int y = 0; y < colored.height(); y++) {
    for (int x = 0; x < colored.width(); x++) {
      uint8_t bytes[3];
      in.read((char *)bytes, 3);
      colored[y][x].r() = (uint8_t)bytes[0];
      colored[y][x].g() = (uint8_t)bytes[1];
      colored[y][x].b() = (uint8_t)bytes[2];
    }
  }
  in.close();
#endif

  writeToFile("/tmp/colored.ppm", colored);
  
  GreyImage<uint8_t> rImage(colored.width(), colored.height());
  GreyImage<uint8_t> gImage(colored.width(), colored.height());
  GreyImage<uint8_t> bImage(colored.width(), colored.height());
  for (int y = 0; y < colored.height(); y++) {
    for (int x = 0; x < colored.width(); x++) {
      rImage[y][x] = colored[y][x].r();
      gImage[y][x] = colored[y][x].g();
      bImage[y][x] = colored[y][x].b();
    }
  }
  writeToFile("/tmp/chan_r.pgm", rImage);
  writeToFile("/tmp/chan_g.pgm", gImage);
  writeToFile("/tmp/chan_b.pgm", bImage);
  
  Pen<uint8_t> cyan(1.0, 0.0, 0.0);
  GreyImage<uint8_t> cyanImage(colored.width(), colored.height());

  Pen<uint8_t> magenta(0.0, 1.0, 0.0);
  GreyImage<uint8_t> magentaImage(colored.width(), colored.height());
  
  Pen<uint8_t> yellow(0.0, 0.0, 1.0);
  GreyImage<uint8_t> yellowImage(colored.width(), colored.height());
  
  Pen<uint8_t> black(1.0, 1.0, 1.0);
  GreyImage<uint8_t> blackImage(colored.width(), colored.height());
  
  Pen<uint8_t> red(0.0, 1.0, 1.0);
  GreyImage<uint8_t> redImage(colored.width(), colored.height());
  
  Pen<uint8_t> green(1.0, 0.0, 1.0);
  GreyImage<uint8_t> greenImage(colored.width(), colored.height());
  
  Pen<uint8_t> blue(1.0, 1.0, 0.0);
  GreyImage<uint8_t> blueImage(colored.width(), colored.height());
  
  Pen<uint8_t> grey(0.5, 0.5, 0.5);
  GreyImage<uint8_t> greyImage(colored.width(), colored.height());
  
  vector<Pen<uint8_t> > pens;
  vector<uint8_t *>targets;
  
  pens.push_back(grey); targets.push_back(greyImage.begin());
  pens.push_back(black); targets.push_back(blackImage.begin());
  pens.push_back(red); targets.push_back(redImage.begin());
  pens.push_back(green); targets.push_back(greenImage.begin());
  pens.push_back(blue); targets.push_back(blueImage.begin());
  pens.push_back(cyan); targets.push_back(cyanImage.begin());
  pens.push_back(magenta); targets.push_back(magentaImage.begin());
  pens.push_back(yellow); targets.push_back(yellowImage.begin());

  colored.separate(pens, targets);
  
  writeToFile("/tmp/sep_red.pgm", redImage);
  writeToFile("/tmp/sep_green.pgm", greenImage);
  writeToFile("/tmp/sep_blue.pgm", blueImage);
  writeToFile("/tmp/sep_cyan.pgm", cyanImage);
  writeToFile("/tmp/sep_magenta.pgm", magentaImage);
  writeToFile("/tmp/sep_yellow.pgm", yellowImage);
  writeToFile("/tmp/sep_grey.pgm", greyImage);
  writeToFile("/tmp/sep_black.pgm", blackImage);
  
  return 0;
}
