#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H

#include "./image.h"

void scale_square_image(Image *src, Image *dst);
void downscale_area_average(Image *src, Image *dst);
void rgb888_to_rgb565_scalar(Image *src, Image *dst);

#if __has_include(<arm_neon.h>)
void rgb888_to_rgb565_neon(Image *src, Image *dst);
void rgb888_to_rgb565_neon_8vals(Image *src, Image *dst);
void rgb888_to_rgb565_neon_16_vals(Image *src, Image *dst);
#endif

/*
void downscale_area_average_forward(uint8_t *src, uint32_t src_width, uint32_t src_height,
                                    uint8_t *dst, uint32_t dst_width, uint32_t dst_height,
                                    int32_t channels);

*/

#endif // IMG_PROCESSING_H
