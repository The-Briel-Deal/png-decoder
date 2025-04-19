#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#include "decode.h"

#define BUF_SIZE 10240

#define FACTORIO_ICON_IMG_PATH "test_data/factorio-icon.png"

#define RUN_TEST(test)                                                         \
  do {                                                                         \
    printf("Running Test " #test "\n");                                        \
    test();                                                                    \
  } while (false)

static uint8_t *get_png_mmap(const char *path, uint8_t *buf, int size) {
  int factorio_icon = open(path, O_RDONLY);
  assert(factorio_icon != -1);
  uint8_t *factorio_icon_data =
      mmap(buf, size, PROT_READ, MAP_PRIVATE, factorio_icon, 0);
  close(factorio_icon);

  return factorio_icon_data;
}

static void close_png_mmap(uint8_t *buf, int size) {
  int e = munmap(buf, size);
  assert(e != -1);
}

void test_read_image_header() {
  uint8_t buf[BUF_SIZE];
  uint8_t *factorio_icon_data =
      get_png_mmap(FACTORIO_ICON_IMG_PATH, buf, BUF_SIZE);

  struct image_header image_header;
  read_image_header(factorio_icon_data, &image_header);
  close_png_mmap(factorio_icon_data, BUF_SIZE);

  assert(image_header.crc == 2859037150);
  assert(image_header.chunk_len == 13);
  assert(image_header.height == 64);
  assert(image_header.width == 64);
  assert(image_header.bit_depth == 8);
  assert(image_header.color_type == COL_TYPE_TRUECOLOR_ALPHA);
  // Only compression method 0 exists in the spec currently.
  assert(image_header.compression_method == 0);
  // Only filter method 0 exists currently.
  assert(image_header.filter_method == 0);
  assert(image_header.interlace_method == 0);
}

const char TEST_INFLATE_IN_FILE_1[] = "test_data/deflated_test123";
#define IN_BUF_SIZE 128
const char TEST_INFLATE_BODY_EXPECT_OUT_1[] = "test123";
#define OUT_SIZE_1 sizeof(TEST_INFLATE_BODY_EXPECT_OUT_1)

void test_inflate_body() {
  char result[OUT_SIZE_1];

  char input_text[IN_BUF_SIZE];
  int file = open(TEST_INFLATE_IN_FILE_1, O_RDONLY);

	struct stat stat_res;
	fstat(file, &stat_res);
	u_int32_t size = stat_res.st_size;

  uint8_t *deflated_memmap =
      mmap(input_text, IN_BUF_SIZE, PROT_READ, MAP_PRIVATE, file, 0);


  inflate_body((void*)deflated_memmap, size, (void *)result, OUT_SIZE_1);
  assert(memcmp(result, TEST_INFLATE_BODY_EXPECT_OUT_1, OUT_SIZE_1) == 0);
}

void run_png_decode_tests() {
  RUN_TEST(test_read_image_header);
  RUN_TEST(test_inflate_body);
}
