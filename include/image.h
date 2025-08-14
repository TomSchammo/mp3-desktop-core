#ifndef MP3_IMAGE_H
#define MP3_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#define TARGET_IMG_WIDTH 200
#define TARGET_IMG_HEIGHT 200
#define TARGET_NUM_PIXELS (TARGET_IMG_WIDTH * TARGET_IMG_HEIGHT)
#define RGB565_BUFFER_SIZE (TARGET_NUM_PIXELS * 2)
#define RGB888_BUFFER_SIZE (TARGET_NUM_PIXELS * 3)

typedef struct {
  uint8_t *buffer;
  size_t length;
  size_t img_width;
  size_t img_height;
} Image;

#endif // MP3_IMAGE_H
