#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  uint8_t *buffer;
  size_t length;
  size_t img_width;
  size_t img_height;
} Image;

void downscale_area_average(Image *src, Image *dst);
void rgb888_to_rgb565_scalar(Image *src, Image *dst);

#if __has_include(<arm_neon.h>)
void rgb888_to_rgb565_neon(Image *src, Image *dst);
#endif

/*
void downscale_area_average_forward(uint8_t *src, uint32_t src_width, uint32_t src_height,
                                    uint8_t *dst, uint32_t dst_width, uint32_t dst_height,
                                    int32_t channels);

*/

#endif // IMG_PROCESSING_H
