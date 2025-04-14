#include "decode.h"
#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>

#define BUF_SIZE 10240
// This only exists for quick tests, this will be a lib without entrypoint.
int main() {
  int factorio_icon = open("test_data/factorio-icon.png", O_RDONLY);
  assert(factorio_icon != -1);
  uint8_t buf[BUF_SIZE];
  uint8_t *factorio_icon_data =
      mmap(buf, BUF_SIZE, PROT_READ, MAP_PRIVATE, factorio_icon, 0);
  struct image_header image_header;
  read_image_header(factorio_icon_data, &image_header);

  return 0;
}
