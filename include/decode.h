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

enum chunk_type {
  CHUNK_UNKNOWN = 0,

  CHUNK_IHDR = 1,
  CHUNK_PLTE = 2,
  CHUNK_IDAT = 3,
  CHUNK_IEND = 4,
};

struct png_chunk {
  uint32_t len;
  enum chunk_type type;
  const uint8_t *chunk_data;
  uint32_t crc;
};

struct png_chunk_list {
  struct png_chunk *chunks;
  int size;
  int capacity;
};

//! Pulls IHDR out of png datastream.
//! See https://www.w3.org/TR/png/#11IHDR for more info.
bool read_image_header(uint8_t *data, struct image_header *image_header);

bool png_chunk_list_init(struct png_chunk_list *chunks);
bool png_divide_into_chunks(const uint8_t *data, const int size,
                            struct png_chunk_list *chunks);
bool get_png_body(uint8_t *data);

bool raw_inflate_once(uint8_t *in, int in_size, uint8_t *out, int out_size);

#endif
