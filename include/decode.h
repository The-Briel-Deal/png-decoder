#ifndef PNG_DECODE_H
#define PNG_DECODE_H

#include <stdbool.h>
#include <stdint.h>

struct image_header {
  uint32_t width;
  uint32_t height;
};

//! Pulls IHDR out of png datastream.
//! See https://www.w3.org/TR/png/#11IHDR for more info.
bool read_image_header(uint8_t *data, struct image_header *image_header);

#endif
