#include "../include/id3_parsing.h"
#include "../include/decompress_jpg.h"
#include "../include/decompress_png.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool get_image_data(uint8_t *frame_buffer, uint32_t frame_size, uint8_t *rgb565_buffer) {
  uint8_t text_encoding = frame_buffer[0];
  uint32_t offset = 1;

  ImageType image_type = OTHER;
  const char *mime_type = (const char *)(&frame_buffer[offset]);

  regex_t regex_jpg;
  int regr_jpg = regcomp(&regex_jpg, "(image\/)?((jpe?g)|(JPE?G))", 0);

  regex_t regex_png;
  int regr_png = regcomp(&regex_jpg, "(image\/)?((png)|(PNG))", 0);

  if (regr_jpg) {
    fprintf(stderr, "Could not compile JPEG regex\n");
    return false;
  }

  if (regr_png) {
    fprintf(stderr, "Could not compile PNG regex\n");
    return false;
  }

  regr_jpg = regexec(&regex_jpg, mime_type, 0, NULL, 0);
  regr_png = regexec(&regex_png, mime_type, 0, NULL, 0);

  if (!regr_jpg)
    image_type = JPEG;
  else if (!regr_png)
    image_type = PNG;
  else if (strcmp(mime_type, "-->") == 0)
    image_type = LINK;

  regfree(&regex_jpg);
  regfree(&regex_png);

  // skip mime type
  while (offset < frame_size && frame_buffer[offset] != 0) {
    offset++;
  }

  // include null terminator
  offset++;

  // skip picture type (1 byte)
  offset += 1;

  // skip description (terminator depends on text encoding)
  if (text_encoding == 0 || text_encoding == 3) {
    // ISO-8859-1 or UTF-8: NUL-terminated (single 0x00)
    while (offset < frame_size && frame_buffer[offset] != 0) {
      offset++;
    }
    // skip terminator
    if (offset < frame_size) {
      offset++;
    }
  } else {
    // UTF-16 with BOM (1) or UTF-16BE (2): terminated by 0x00 0x00
    while (offset + 1 < frame_size &&
           !(frame_buffer[offset] == 0 && frame_buffer[offset + 1] == 0)) {
      // advance by 2 bytes (UTF-16 code unit)
      offset += 2;
    }
    // skip 2-byte terminator if present
    if (offset + 1 < frame_size) {
      offset += 2;
    }
  }

  uint32_t image_data_size = frame_size - offset;
  uint8_t *image_buffer = frame_buffer + offset;

  Image rgb888_image = {.img_width = 0, .img_height = 0, .buffer = NULL, .length = 0};

  if (image_type == LINK) {
    printf("image data is a link, fetching data...\n");
    printf("To be implemented!\n");
    return false;

  } else if (image_type == JPEG) {

    if (!convert_jpeg_to_rgb888(image_buffer, image_data_size, &rgb888_image)) {
      // TODO error handling
      return false;
    }

  } else if (image_type == PNG) {
    if (!convert_png_to_rgb888(image_buffer, image_data_size, &rgb888_image)) {
      // TODO error handling
      return false;
    }

  } else {
    printf("MIME Type: '%s' is not supported!\n", mime_type);
    return false;
  }

  if (rgb888_image.length != 0) {

    Image rgb888_downscaled = {.img_height = TARGET_IMG_HEIGHT,
                               .img_width = TARGET_IMG_WIDTH,
                               .length = RGB888_BUFFER_SIZE};
    uint8_t *downscaled_buffer = malloc(rgb888_downscaled.length);
    rgb888_downscaled.buffer = downscaled_buffer;

    scale_square_image(&rgb888_image, &rgb888_downscaled);

    free(rgb888_image.buffer);

    Image rgb565_image = {
        .img_height = TARGET_IMG_HEIGHT,
        .img_width = TARGET_IMG_WIDTH,
        .length = RGB565_BUFFER_SIZE,
        .buffer = rgb565_buffer,
    };

#if __has_include(<arm_neon.h>)
    rgb888_to_rgb565_neon(&rgb888_downscaled, &rgb565_image);
#else
    rgb888_to_rgb565_scalar(&rgb888_downscaled, &rgb585_image);
#endif

    return true;
  }

  return false;
}
