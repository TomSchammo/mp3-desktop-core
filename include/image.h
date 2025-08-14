#ifndef MP3_IMAGE_H
#define MP3_IMAGE_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *buffer;
  size_t length;
  size_t img_width;
  size_t img_height;
} Image;

#endif // MP3_IMAGE_H
