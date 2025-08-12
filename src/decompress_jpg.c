
#include "../include/decompress_jpg.h"

// NOTE: jpeg-turbo does not inlcude stdio
// clang-format off
#include <stdio.h>
#include <jpeglib.h>
// clang-format on

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

bool convert_jpeg_to_rgb888(const uint8_t *image_buffer, uint32_t size, Image *rgb888_image) {

  struct jpeg_decompress_struct info;
  struct jpeg_error_mgr err;

  info.err = jpeg_std_error(&err);

  jpeg_create_decompress(&info);

  jpeg_mem_src(&info, image_buffer, size);
  jpeg_read_header(&info, true);

  info.out_color_space = JCS_EXT_RGB;

  jpeg_start_decompress(&info);

  assert(info.output_components == 3);

  rgb888_image->img_width = info.output_width;
  rgb888_image->img_height = info.output_height;
  rgb888_image->length = rgb888_image->img_width * rgb888_image->img_height * 3;

  uint8_t *rgb888_buffer = malloc(rgb888_image->length);

  rgb888_image->buffer = rgb888_buffer;

  JSAMPROW row_pointer;
  int row_stride = info.output_width * 3;

  for (int y = 0; y < info.output_height; y++) {
    row_pointer = rgb888_buffer + (y * row_stride);
    jpeg_read_scanlines(&info, &row_pointer, 1);
  }

  jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  return true;
}
