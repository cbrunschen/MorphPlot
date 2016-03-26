/*
 *  ColorImage.cpp
 *  MorphPlot
 *
 *  Created by Christian Brunschen on 09/09/2012.
 *  Copyright 2012 Christian Brunschen. All rights reserved.
 *
 */

#include "ColorImage.h"

namespace Images {
#if 0
}
#endif

namespace ColorImages {
#if 0
}
#endif

#define PNG_BYTES_TO_CHECK 8
shared_ptr< ColorImage<uint8_t> > readPng(FILE *fp) {
  png_byte buf[PNG_BYTES_TO_CHECK];

  shared_ptr< ColorImage<uint8_t> > result(NULL);

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

#ifdef PNG_READ_SCALE_16_TO_8_SUPPORTED
  png_set_scale_16(png_ptr);
#else
  png_set_strip_16(png_ptr);
#endif

  /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
   * byte into separate bytes (useful for paletted and grayscale images).
   */
  png_set_packing(png_ptr);

  if (color_type == PNG_COLOR_TYPE_GRAY) {
    if (bit_depth < 8) {
      // convert grays to 8 bits
      png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    // convert colors to gray
    png_set_gray_to_rgb(png_ptr);
  }

  /* Expand paletted colors into true RGB triplets */
  if (color_type == PNG_COLOR_TYPE_PALETTE) {
    png_set_palette_to_rgb(png_ptr);
  }

  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
    png_set_tRNS_to_alpha(png_ptr);
  }

  // composite against a default white background
  png_color_16 my_background = { 0, 0xff, 0xff, 0xff, 0xff };
  png_set_background(png_ptr, &my_background,
                     PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);

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

  png_read_update_info(png_ptr, info_ptr);

  result = ColorImage<uint8_t>::make(width, height);
  result->setXRes(xRes);
  result->setYRes(yRes);
  result->setResUnit(resUnit);

  /* The easiest way to read the image: */
  png_bytep row_pointers[height];

  /* Clear the pointer array */
  for (int row = 0; row < height; row++) {
    row_pointers[row] = (png_bytep) (*result)[row].data();
  }

  png_read_image(png_ptr, row_pointers);

  /* Read rest of file, and get additional chunks in info_ptr - REQUIRED */
  png_read_end(png_ptr, info_ptr);
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  fclose(fp);

  return result;
}

bool writePng(const ColorImage<uint8_t> &image, FILE *fp) {
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
               8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
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
}

#if 0
{
#endif
}
