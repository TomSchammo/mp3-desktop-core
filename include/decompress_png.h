#ifndef DECOMPRESS_PNG_H
#define DECOMPRESS_PNG_H

#include "img_processing.h"
#include "png.h"
#include "pngconf.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
  const uint8_t *data;
  png_size_t size;
  png_size_t offset;
} png_data;

void read_png_from_memory(png_structp png_ptr, png_bytep data, png_size_t num_bytes);
bool convert_png_to_rgb888(const uint8_t *image_buffer, uint32_t size, Image *rgb888_image);

#endif // DECOMPRESS_PNG_H
