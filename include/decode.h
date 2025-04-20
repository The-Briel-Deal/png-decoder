#ifndef PNG_DECODE_H
#define PNG_DECODE_H

#include <stdbool.h>
#include <stdint.h>

enum color_type {
  COL_TYPE_GREYSCALE = 0,
  COL_TYPE_TRUECOLOR = 2,
  COL_TYPE_INDEXED = 3,
  COL_TYPE_GREYSCALE_ALPHA = 4,
  COL_TYPE_TRUECOLOR_ALPHA = 6,
};

struct image_header {
  uint32_t crc;
  uint32_t chunk_len;

  uint32_t width;
  uint32_t height;
  int8_t bit_depth;
  int8_t color_type;
  int8_t compression_method;
  int8_t filter_method;
  int8_t interlace_method;
};

//! Pulls IHDR out of png datastream.
//! See https://www.w3.org/TR/png/#11IHDR for more info.
bool read_image_header(uint8_t *data, struct image_header *image_header);
bool get_png_body(uint8_t *data);

bool raw_inflate_once(uint8_t *in, int in_size, uint8_t *out, int out_size);

#endif
