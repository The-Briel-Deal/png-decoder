#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "decode.h"

#define WIDTH_OFFSET 4
#define HEIGHT_OFFSET 8
#define BIT_DEPTH_OFFSET 12
#define COLOR_TYPE_OFFSET 13
#define COMPRESSION_METHOD_OFFSET 14
#define FILTER_METHOD_OFFSET 15
#define INTERLACE_METHOD_OFFSET 16

static const int8_t GREYSCALE_ALLOWED_BIT_DEPTHS[] = {1, 2, 4, 8, 16};
static const int8_t TRUECOLOR_ALLOWED_BIT_DEPTHS[] = {8, 16};
static const int8_t INDEXED_ALLOWED_BIT_DEPTHS[] = {1, 2, 4, 8};
static const int8_t GREYSCALE_ALPHA_ALLOWED_BIT_DEPTHS[] = {8, 16};
static const int8_t TRUECOLOR_ALPHA_ALLOWED_BIT_DEPTHS[] = {8, 16};

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

static bool i8_in_list(const int8_t num, const int8_t *list, int size) {
  for (int i = 0; i < size; i++)
    if (num == list[i])
      return true;
  return false;
}

bool read_image_header(uint8_t *data, struct image_header *image_header) {
  int i = 0;
  while (!at_ihdr_label(data, i))
    i++;

  image_header->width = get_png_int(&data[i + WIDTH_OFFSET]);
  image_header->height = get_png_int(&data[i + HEIGHT_OFFSET]);
  image_header->bit_depth = data[i + BIT_DEPTH_OFFSET];
  image_header->color_type = data[i + COLOR_TYPE_OFFSET];

  switch (image_header->color_type) {
  case COL_TYPE_GREYSCALE:
    if (!i8_in_list(image_header->bit_depth, GREYSCALE_ALLOWED_BIT_DEPTHS,
                    sizeof(GREYSCALE_ALLOWED_BIT_DEPTHS) / sizeof(int8_t))) {
      fprintf(stderr, "Bit depth '%i' is invalid.\n", image_header->bit_depth);
      exit(EXIT_FAILURE);
    }
    break;
  case COL_TYPE_TRUECOLOR:
    if (!i8_in_list(image_header->bit_depth, TRUECOLOR_ALLOWED_BIT_DEPTHS,
                    sizeof(TRUECOLOR_ALLOWED_BIT_DEPTHS) / sizeof(int8_t))) {
      fprintf(stderr, "Bit depth '%i' is invalid.\n", image_header->bit_depth);
      exit(EXIT_FAILURE);
    }
    break;
  case COL_TYPE_INDEXED:
    if (!i8_in_list(image_header->bit_depth, INDEXED_ALLOWED_BIT_DEPTHS,
                    sizeof(INDEXED_ALLOWED_BIT_DEPTHS) / sizeof(int8_t))) {
      fprintf(stderr, "Bit depth '%i' is invalid.\n", image_header->bit_depth);
      exit(EXIT_FAILURE);
    }
    break;
  case COL_TYPE_GREYSCALE_ALPHA:
    if (!i8_in_list(image_header->bit_depth, GREYSCALE_ALPHA_ALLOWED_BIT_DEPTHS,
                    sizeof(GREYSCALE_ALPHA_ALLOWED_BIT_DEPTHS) /
                        sizeof(int8_t))) {
      fprintf(stderr, "Bit depth '%i' is invalid.\n", image_header->bit_depth);
      exit(EXIT_FAILURE);
    }
    break;
  case COL_TYPE_TRUECOLOR_ALPHA:
    if (!i8_in_list(image_header->bit_depth, TRUECOLOR_ALPHA_ALLOWED_BIT_DEPTHS,
                    sizeof(TRUECOLOR_ALPHA_ALLOWED_BIT_DEPTHS) /
                        sizeof(int8_t))) {
      fprintf(stderr, "Bit depth '%i' is invalid.\n", image_header->bit_depth);
      exit(EXIT_FAILURE);
    }
    break;
  default:
    fprintf(stderr, "Color type '%i' is invalid.\n", image_header->color_type);
    exit(EXIT_FAILURE);
  }

  return true;
}
