#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

#include "decode.h"

#define LEN_OFFSET -4
#define WIDTH_OFFSET 4
#define HEIGHT_OFFSET 8
#define BIT_DEPTH_OFFSET 12
#define COLOR_TYPE_OFFSET 13
#define COMPRESSION_METHOD_OFFSET 14
#define FILTER_METHOD_OFFSET 15
#define INTERLACE_METHOD_OFFSET 16
#define CRC_OFFSET 17

static const int8_t GREYSCALE_ALLOWED_BIT_DEPTHS[] = {1, 2, 4, 8, 16};
static const int8_t TRUECOLOR_ALLOWED_BIT_DEPTHS[] = {8, 16};
static const int8_t INDEXED_ALLOWED_BIT_DEPTHS[] = {1, 2, 4, 8};
static const int8_t GREYSCALE_ALPHA_ALLOWED_BIT_DEPTHS[] = {8, 16};
static const int8_t TRUECOLOR_ALPHA_ALLOWED_BIT_DEPTHS[] = {8, 16};

#define PNG_CHUNK_LIST_CAPACITY 16

// Reads Big-Endian png int to Little-Endian.
uint32_t get_png_int(const uint8_t *data) {
  uint32_t png_int = 0;

  png_int |= (data[0] << (3 * 8));
  png_int |= (data[1] << (2 * 8));
  png_int |= (data[2] << (1 * 8));
  png_int |= (data[3] << (0 * 8));

  return png_int;
}

bool png_chunk_list_init(struct png_chunk_list *chunks) {
  chunks->capacity = PNG_CHUNK_LIST_CAPACITY;
  chunks->size = 0;
  chunks->chunks = malloc(sizeof(struct png_chunk) * 16);

  return true;
}

bool png_divide_into_chunks(const uint8_t *data, const int size,
                            struct png_chunk_list *chunks) {
  assert(chunks->size == 0);
  int data_pos = 0;
  while (chunks->capacity > chunks->size && size > data_pos) {
    struct png_chunk *chunk = &chunks->chunks[chunks->size++];

    // Length
    chunk->len = get_png_int(&data[data_pos]);
    data_pos += 4;

    // Chunk Type
    const char *type = (char *)&data[data_pos];
    if (memcmp(type, "IHDR", 4) == 0) {
      chunk->type = CHUNK_IHDR;
    } else if (memcmp(type, "PLTE", 4) == 0) {
      chunk->type = CHUNK_PLTE;
    } else if (memcmp(type, "IDAT", 4) == 0) {
      chunk->type = CHUNK_IDAT;
    } else if (memcmp(type, "IEND", 4) == 0) {
      chunk->type = CHUNK_IEND;
    } else {
      chunk->type = CHUNK_UNKNOWN;
    }
    data_pos += 4;

    // Chunk Data
    if (chunk->len == 0) {
      chunk->chunk_data = NULL;
    } else {
      chunk->chunk_data = &data[data_pos];
    }
    data_pos += chunk->len;

    // CRC
    chunk->crc = get_png_int(&data[data_pos]);
		data_pos += 4;
  }

  return true;
}

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

  image_header->crc = get_png_int(&data[i + CRC_OFFSET]);
  image_header->chunk_len = get_png_int(&data[i + LEN_OFFSET]);
  image_header->width = get_png_int(&data[i + WIDTH_OFFSET]);
  assert(image_header->width != 0);
  image_header->height = get_png_int(&data[i + HEIGHT_OFFSET]);
  assert(image_header->height != 0);
  image_header->bit_depth = data[i + BIT_DEPTH_OFFSET];
  image_header->color_type = data[i + COLOR_TYPE_OFFSET];
  image_header->compression_method = data[i + COMPRESSION_METHOD_OFFSET];
  image_header->filter_method = data[i + FILTER_METHOD_OFFSET];
  image_header->interlace_method = data[i + INTERLACE_METHOD_OFFSET];

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

bool raw_inflate_once(uint8_t *in, int in_size, uint8_t *out, int out_size) {
  int ret;
  z_stream stream = {.zalloc = Z_NULL,
                     .zfree = Z_NULL,
                     .opaque = Z_NULL,
                     .avail_in = 0,
                     .next_in = Z_NULL};
  ret = inflateInit2(&stream, -15);
  assert(ret == Z_OK);

  stream.avail_in = in_size;
  stream.next_in = in;

  stream.avail_out = out_size;
  stream.next_out = out;

  ret = inflate(&stream, Z_FINISH);
  assert(ret == Z_STREAM_END);

  ret = inflateEnd(&stream);
  assert(ret == Z_OK);

  return true;
}

bool inflate_once(uint8_t *in, int in_size, uint8_t *out, int out_size) {
  int ret;
  z_stream stream = {.zalloc = Z_NULL,
                     .zfree = Z_NULL,
                     .opaque = Z_NULL,
                     .avail_in = 0,
                     .next_in = Z_NULL};
  ret = inflateInit(&stream);
  assert(ret == Z_OK);

  stream.avail_in = in_size;
  stream.next_in = in;

  stream.avail_out = out_size;
  stream.next_out = out;

  ret = inflate(&stream, Z_FINISH);
  assert(ret == Z_STREAM_END);

  ret = inflateEnd(&stream);
  assert(ret == Z_OK);

  return true;
}

// bool get_png_body(uint8_t *data) {}
