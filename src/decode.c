#include <stdint.h>

#include "decode.h"

#define WIDTH_OFFSET 4
#define HEIGHT_OFFSET 8

static bool at_ihdr_label(uint8_t *data, int i) {
  if (data[i] != 'I')
    return false;
  if (data[i + 1] != 'H')
    return false;
  if (data[i + 2] != 'D')
    return false;
  if (data[i + 3] != 'R')
    return false;
  return true;
}

// Reads Big-Endian png int to Little-Endian.
uint32_t get_png_int(uint8_t *data) {
  uint32_t png_int = 0;

  png_int |= (data[0] << (3 * 8));
  png_int |= (data[1] << (2 * 8));
  png_int |= (data[2] << (1 * 8));
  png_int |= (data[3] << (0 * 8));

  return png_int;
}

bool read_image_header(uint8_t *data, struct image_header *image_header) {
  int i = 0;
  while (!at_ihdr_label(data, i))
    i++;

  image_header->width = get_png_int(&data[i + WIDTH_OFFSET]);
  image_header->height = get_png_int(&data[i + HEIGHT_OFFSET]);

  return true;
}

