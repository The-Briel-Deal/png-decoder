#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
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

u_int32_t file_size(int fd) {
  struct stat stat_res;
  fstat(fd, &stat_res);
  return stat_res.st_size;
}

void assert_inflated_file_eq(char *filename, char *expect_inflated) {
  char result[BUF_SIZE];
  char compressed_input[BUF_SIZE];

  int file = open(filename, O_RDONLY);

  uint8_t *deflated_memmap =
      mmap(compressed_input, BUF_SIZE, PROT_READ, MAP_PRIVATE, file, 0);

  raw_inflate_once((void *)deflated_memmap, file_size(file), (void *)result,
                   BUF_SIZE);
  assert(memcmp(result, expect_inflated, strlen(expect_inflated)) == 0);
}

void test_raw_inflate() {
  assert_inflated_file_eq("test_data/deflated_text1", "test123");
}

// void test_inflate_png_body() {
//   uint8_t *factorio_icon_data =
//       get_png_mmap(FACTORIO_ICON_IMG_PATH, buf, BUF_SIZE);
// 	get_png_body(factorio_icon_data);
// 
// }

void run_png_decode_tests() {
  RUN_TEST(test_read_image_header);
  RUN_TEST(test_raw_inflate);
}
