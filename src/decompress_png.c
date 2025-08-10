
#include "../include/decompress_png.h"
#include "png.h"
#include <assert.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_png_from_memory(png_structp png_ptr, png_bytep data, png_size_t num_bytes) {
  png_data *input_data = (png_data *)png_get_io_ptr(png_ptr);

  if (num_bytes > input_data->size - input_data->offset)
    png_error(png_ptr, "read beyond end of buffer");

  memcpy(data, input_data->data + input_data->offset, num_bytes);
  input_data->offset += num_bytes;
}

bool convert_png_to_rgb888(const uint8_t *image_buffer, uint32_t size, Image *rgb888_image) {

  png_byte image_header[8];
  memcpy(image_header, image_buffer, 8);

  // check for png signature
  if (png_sig_cmp((png_const_bytep)image_header, 0, 8) == 0) {

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {

      // TODO
      return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      // TODO
      return false;
    }

    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      // TODO
      return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
      // TODO
      return false;
    }

    png_data data = {image_buffer, size, 0};
    png_set_read_fn(png_ptr, &data, &read_png_from_memory);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);

    rgb888_image->length = 3 * width * height;
    rgb888_image->img_width = width;
    rgb888_image->img_height = height;

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (bit_depth < 8) {
      png_set_expand(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
      png_set_palette_to_rgb(png_ptr);
    }

    if (color_type & PNG_COLOR_MASK_ALPHA) {
      png_set_strip_alpha(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
      png_set_gray_to_rgb(png_ptr);
    }

    if (bit_depth == 16) {
      png_set_scale_16(png_ptr);
    }
    png_read_update_info(png_ptr, info_ptr);

    uint8_t *rgb888_buffer = malloc(rgb888_image->length);
    rgb888_image->buffer = rgb888_buffer;

    png_bytep *row_pointers = malloc(height * sizeof(png_bytep));

    for (int y = 0; y < height; y++) {
      row_pointers[y] = rgb888_buffer + (y * width * 3);
    }

    png_read_image(png_ptr, row_pointers);
    free(row_pointers);

    return true;

  } else {
    // TODO does not contain png signature!
    printf("Could not find PNG signature depsite MIME type 'image/png'!\n");
    return false;
  }
}
