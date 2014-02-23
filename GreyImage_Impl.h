//
//  GreyImage_Impl.h
//  MorphPlot
//
//  Created by Christian Brunschen on 09/02/2014.
//
//

#ifndef __GreyImage_Impl_h__
#define __GreyImage_Impl_h__

#include "GreyImage.h"

namespace Images {
#if 0
}
#endif

template<typename C>
inline GreyImage<C>::GreyImage(int width, int height, bool doClear)
: Image<C>(width, height, doClear) { }

template<typename C>
inline GreyImage<C>::GreyImage(const GreyImage &other) : Image<C>(other) { }

template<typename C>
inline GreyImage<C>::GreyImage(const GreyImage *other) : Image<C>(other) { }

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::make(int width, int height, bool doClear) {
  return make_shared< GreyImage<C> >(width, height, doClear);
}

template<typename C>
inline istream &GreyImage<C>::readData(istream &in) {
  in.read((char *)data_, width_ * height_ * sizeof(C));
  return in;
}

template<typename C>
inline ostream &GreyImage<C>::writeData(ostream &out) {
  out.write((char *)data_, width_ * height_ * sizeof(C));
  return out;
}

template<typename C>
inline bool GreyImage<C>::writePng(FILE *f) {
  return GreyImages::writePng(*this, f);
}

template<typename C>
inline bool GreyImage<C>::writePng(const char * const filename) {
  FILE *f = fopen(filename, "wb");
  if (f != NULL) {
    return writePng(f);
  }
  return false;
}

template<typename C>
inline bool GreyImage<C>::writePng(const string &filename) {
  return writePng(filename.c_str());
}

template<typename C>
inline C GreyImage<C>::defaultBackground() {
  return static_cast<C>(0);
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPng(FILE *fp) {
  return GreyImages::readPng<C, false>(fp, GreyImage<C>::defaultBackground());
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPngHeightmap(FILE *fp) {
  return GreyImages::readPng<C, true>(fp, GreyImage<C>::defaultBackground());
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPng(const char * const filename) {
  FILE *f = fopen(filename, "rb");
  if (f != NULL) {
    return readPng(f);
  }
  return make_shared< GreyImage<C> >(NULL);
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPngHeightmap(const char * const filename) {
  FILE *f = fopen(filename, "rb");
  if (f != NULL) {
    return readPngHeightmap(f);
  }
  return make_shared< GreyImage<C> >(NULL);
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPng(const string &filename) {
  return readPng(filename.c_str());
}

template<typename C>
inline shared_ptr< GreyImage<C> > GreyImage<C>::readPngHeightmap(const string &filename) {
  return readPngHeightmap(filename.c_str());
}

template<typename C>
inline shared_ptr<Bitmap> GreyImage<C>::coverage() const {
  return Image<C>::ne(Component<C>::min());
}

template<typename C>
inline shared_ptr<Bitmap> GreyImage<C>::coverage(int threads) const {
  return Image<C>::ne(Component<C>::min(), threads);
}

template<typename C>
inline shared_ptr<Bitmap> GreyImage<C>::coverage(Workers &workers) const {
  return Image<C>::ne(Component<C>::min(), workers);
}

template<typename C>
template<typename Op>
inline shared_ptr<Bitmap> GreyImage<C>::where(Op &op, C value) const {
  shared_ptr<Bitmap> result = make_shared<Bitmap>(width_, height_);
  Bitmap::iterator dst = result->begin();
  GreyImage<C>::const_iterator src = this->begin();
  GreyImage<C>::const_iterator end = this->end();
  while (src != end) {
    *dst++ = op(*src++, value);
  }
  return result;
}

template<typename C>
inline double GreyImage<C>::frand() {
  return (double) rand() / ((double) RAND_MAX);
}

template<typename C>
inline double GreyImage<C>::sumOfValues(const Point &p0, const list<Point> &points) const {
  double value = 0.0;
  for (list<Point>::const_iterator cp = points.begin(); cp != points.end(); ++cp) {
    value += Component<C>::fraction(GreyImage::get(p0 + *cp));
  }
  return value;
}

template<typename C>
template <typename T>
inline double GreyImage<C>::dither(Chains &results, const Chain &chain, const Circle &circle, const T &reference, double error) {
  Chain *result = NULL;
  int skip = 0;

  long nPoints = circle.points().size();
  double halfPoints = nPoints / 2.0;
  D(cerr << "n=" << nPoints << ", half=" << halfPoints << endl);

  Chain::const_iterator i = chain.begin();

  double value = sumOfValues(*i, circle.points());

  error += value;
  if (error > halfPoints) {
    double effect = nPoints - reference.sumOfValues(*i, circle.points());
    D(cerr << "[+] ");
    result = &(results.addChain());
    error -= effect;
    result->addPoint(*i);
    D(cerr << ", ###, effect = " << effect << ", new error=" << error << endl);
  } else {
    D(cerr << ", ___, new error=" << error << endl);
  }

  for (Chain::const_iterator j = i++; i != chain.end(); j = i++) {
    int dir = i->directionTo(*j);
    const list<Point> &delta = circle.getDeltaForDirection(dir);
    double value = sumOfValues(*i, delta);
    error += value;

    int dSize = static_cast<int>(delta.size());
    double threshold = dSize / 2.0;

    D(cerr << *i << ": value=" << value << ", error=" << error << ", threshold=" << threshold);
    if (skip > 0) {
      // we're forced-skipping after lifting the pen
      skip -= isDiagonal(dir) ? 7 : 5;
      D(cerr << ", _@_, new error=" << error << endl);
    } else if (error > threshold) {
      if (!result) {
        double effect = nPoints - reference.sumOfValues(*i, circle.points());
        D(cerr << "[+] ");
        result = &(results.addChain());
        error -= effect;
      } else {
        double effect = dSize - reference.sumOfValues(*i, delta);
        error -= effect;
      }
      result->addPoint(*i);
      D(cerr << ", ###, new error=" << error << endl);
    } else {
      if (result) {
        result = NULL;
        D(cerr << "[-] ");
        skip = 2 * 5 * (2 * circle.r() + 1);
      }
      D(cerr << ", ___, new error=" << error << endl);
    }
  }
  return error;
}

template<typename C>
template <typename T>
inline void GreyImage<C>::dither(Chains &results, const Chains &chains, const Circle &circle, const T &reference) {
  double error = 0.0;
  for (Chains::const_iterator i = chains.begin(); i != chains.end(); ++i) {
    error = dither(results, *i, circle, reference, error);
  }
}

class ConstantNothing {
public:
  template<typename A, typename B> double sumOfValues(const A &a, const B &b) const {
    return 0.0;
  }
};

template<typename C>
inline void GreyImage<C>::dither(Chains &results, const Chains &chains, const Circle &circle) {
  ConstantNothing cn;
  dither(results, chains, circle, cn);
}


template<>
inline uint8_t GreyImage<uint8_t>::defaultBackground() {
  return 0xff;
}

template<>
inline uint16_t GreyImage<uint16_t>::defaultBackground() {
  return 0x0000;
}

template <typename C>
inline ostream &operator<<(ostream &out, const GreyImage<C> &i) {
  int y, x;
  ios_base::fmtflags flags(out.flags());
  out << hex;
  for (y = 0; y < i.height(); y++) {
    for (x = 0; x < i.width(); x ++) {
      Component<C>::print(out, i.at(y, x));
    }
    out << endl;
  }
  out << flush;
  out.flags(flags);
  return out;
}

namespace GreyImages {
#if 0
}
#endif

template<typename T>
inline void readPng_setDepth(png_structp png_ptr) { }

template<>
inline void readPng_setDepth<uint8_t>(png_structp png_ptr) {
  // ensure samples are 8 bits
#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
  png_set_scale_16(png_ptr);
#else
  png_set_strip_16(png_ptr);
#endif
}

template<>
inline void readPng_setDepth<uint16_t>(png_structp png_ptr) {
  // expand samples to 16 bits
  png_set_expand_16(png_ptr);
}

#define PNG_BYTES_TO_CHECK 8
template<typename T, bool heightMap>
shared_ptr< GreyImage<T> > readPng(FILE *fp, T defaultBackground) {
  png_byte buf[PNG_BYTES_TO_CHECK];

  shared_ptr< GreyImage<T> > result(NULL);

  /* Read in some of the signature bytes */
  if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
    return result;
  }

  /* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
   Return nonzero (true) if they match */

  if (png_sig_cmp(buf, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
    return result;
  }

  png_structp png_ptr;
  png_infop info_ptr;
  png_uint_32 width, height, xRes, yRes;
  int bit_depth, color_type, interlace_type, resUnit;

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    return result;
  }

  /* Allocate/initialize the memory for image information.  REQUIRED. */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    return result;
  }

  if (setjmp(png_jmpbuf(png_ptr))) {
    fclose(fp);
    /* Free all of the memory associated with the png_ptr and info_ptr */
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    /* If we get here, we had a problem reading the file */
    result = nullptr;
    return result;
  }

  /* Set up the input control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  /* If we have already read some of the signature */
  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);

  png_read_info(png_ptr, info_ptr);

  png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
               &interlace_type, NULL, NULL);

  png_get_pHYs(png_ptr, info_ptr, &xRes, &yRes, &resUnit);

  // mask out the alpha
  color_type &= ~PNG_COLOR_MASK_ALPHA;

  // ensure that the data generated are expanded to the correct number of bits
  readPng_setDepth<T>(png_ptr);

  /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
   * byte into separate bytes (useful for paletted and grayscale images).
   */
  png_set_packing(png_ptr);

  if (color_type != PNG_COLOR_TYPE_GRAY) {
    // convert colors to gray
    png_set_rgb_to_gray(png_ptr, PNG_ERROR_ACTION_ERROR, PNG_RGB_TO_GRAY_DEFAULT, PNG_RGB_TO_GRAY_DEFAULT);
  } else if (bit_depth < 8) {
    // convert grays to 8 bits
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
  }

  // composite against a default background
  png_color_16 my_background = { 0, defaultBackground, defaultBackground, defaultBackground, defaultBackground };
  png_set_background(png_ptr, &my_background,
                     PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

  if (heightMap) {
    png_set_gamma(png_ptr, PNG_GAMMA_LINEAR, PNG_GAMMA_LINEAR);
  } else {
    int intent;
    if (png_get_sRGB(png_ptr, info_ptr, &intent)) {
      png_set_gamma(png_ptr, PNG_GAMMA_LINEAR, 0.45455);
    } else {
      double image_gamma;
      if (png_get_gAMA(png_ptr, info_ptr, &image_gamma)) {
        png_set_gamma(png_ptr, PNG_GAMMA_LINEAR, image_gamma);
      } else {
        png_set_gamma(png_ptr, PNG_GAMMA_LINEAR, 0.45455);
      }
    }
  }

  png_read_update_info(png_ptr, info_ptr);

  result = GreyImage<T>::make(width, height);
  result->setXRes(xRes);
  result->setYRes(yRes);
  result->setResUnit(resUnit);

  /* The easiest way to read the image: */
  png_bytep row_pointers[height];

  /* Clear the pointer array */
  for (int row = 0; row < height; row++) {
    row_pointers[row] = (png_bytep) &((*result)[row].data()[0]);
  }

  png_read_image(png_ptr, row_pointers);

  /* Read rest of file, and get additional chunks in info_ptr - REQUIRED */
  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  if (heightMap) {
    png_color_8p sig_bit;

    cerr << "bit_depth=" << bit_depth << endl << flush;
    int shift = 16 - bit_depth;

    if (png_get_sBIT(png_ptr, info_ptr, &sig_bit)) {
      cerr << "sbit: red=" << sig_bit->red << ", green=" << sig_bit->red << ", blue=" << sig_bit->gray << endl << flush;
      int bits = 0;
      if (sig_bit->red > bits)   bits = sig_bit->red;
      if (sig_bit->green > bits) bits = sig_bit->green;
      if (sig_bit->blue > bits)  bits = sig_bit->blue;
      if (sig_bit->gray > bits)  bits = sig_bit->gray;
      shift = 16 - bits;
    }
    cerr << "will shift all samples down by " << shift << " bits" << endl << flush;

    int zom = 0, znm = 0;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        uint16_t z = result->at(y, x);
        if (z > zom) zom = z;
        z = z >> shift;
        if (z > znm) znm = z;
        result->at(y, x) = z;
      }
    }
    cerr << "zMax should have shifted from " << zom << " to " << znm << endl << flush;
  }

  return result;
}

template<typename T>
bool writePng(const GreyImage<T> &image, FILE *fp) {
  png_structp png_ptr;
  png_infop info_ptr;

  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) {
    fclose(fp);
    return false;
  }

  /* Allocate/initialize the image information data.  REQUIRED */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) {
    fclose(fp);
    png_destroy_write_struct(&png_ptr,  NULL);
    return false;
  }

  /* Set error handling.  REQUIRED if you aren't supplying your own
   * error handling functions in the png_create_write_struct() call.
   */
  if (setjmp(png_jmpbuf(png_ptr)))
  {
    /* If we get here, we had a problem writing the file */
    fclose(fp);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return false;
  }

  /* Set up the output control if you are using standard C streams */
  png_init_io(png_ptr, fp);

  png_set_IHDR(png_ptr, info_ptr, image.width(), image.height(),
               8 * sizeof(T), PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

  /* Optional gamma chunk is strongly suggested if you have any guess
   * as to the correct gamma of the image.
   */
  png_set_gAMA(png_ptr, info_ptr, 1.0);

  png_set_pHYs(png_ptr, info_ptr, image.xRes(), image.yRes(), image.resUnit());

  /* Write the file header information.  REQUIRED */
  png_write_info(png_ptr, info_ptr);

  png_bytep row_pointers[image.height()];
  for (int row = 0; row < image.height(); row++) {
    row_pointers[row] = (png_bytep) image[row].data();
  }

  png_write_image(png_ptr, row_pointers);

  /* It is REQUIRED to call this to finish writing the rest of the file */
  png_write_end(png_ptr, info_ptr);

  /* Clean up after the write, and free any memory allocated */
  png_destroy_write_struct(&png_ptr, &info_ptr);

  /* Close the file */
  fclose(fp);

  return true;
}

#if 0
{
#endif
}  // namespace GreyImages

#if 0
{
#endif
}  // namespace Images

#endif  // __GreyImage_Impl_h__
