#ifndef IMG_PROCESSING_HPP
#define IMG_PROCESSING_HPP

#include <cstdint>

void downscale_area_average(uint8_t *src, uint32_t src_width, uint32_t src_height, uint8_t *dst,
                            uint32_t dst_width, uint32_t dst_height, int32_t channels);

#endif // IMG_PROCESSING_HPP
