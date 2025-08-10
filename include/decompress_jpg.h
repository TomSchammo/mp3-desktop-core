
#ifndef DECOMPRESS_JPG_H
#define DECOMPRESS_JPG_H

#include "./img_processing.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool convert_jpeg_to_rgb888(const uint8_t *image_buffer, uint32_t size, Image *rgb888_image);

#endif // DECOMPRESS_JPG_H
