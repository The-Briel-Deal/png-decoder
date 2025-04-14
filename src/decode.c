#include "decode.h"
#include <stdio.h>

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

bool read_image_header(uint8_t *data, struct image_header *image_header) {
  int i = 0;
  while (!at_ihdr_label(data, i))
    i++;

  printf("Found Image Header at byte %i", i);

  return true;
}
