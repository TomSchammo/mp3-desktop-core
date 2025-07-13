#include "../include/img_processing.hpp"
#include <algorithm>
#include <cassert>

void downscale_area_average(uint8_t *src, uint32_t src_width, uint32_t src_height, uint8_t *dst,
                            uint32_t dst_width, uint32_t dst_height, int32_t channels) {

  // downscaling only
  assert(src_width > dst_width);
  assert(src_height > dst_height);

  auto x_scale = static_cast<float>(src_width) / dst_width;
  auto y_scale = static_cast<float>(src_height) / dst_height;

  for (uint32_t y = 0; y < dst_height; y++) {
    for (uint32_t x = 0; x < dst_width; x++) {

      auto src_x_start = static_cast<uint32_t>(x * x_scale);
      auto src_y_start = static_cast<uint32_t>(y * y_scale);

      auto src_x_end = std::min(static_cast<uint32_t>((x + 1) * x_scale), src_width);
      auto src_y_end = std::min(static_cast<uint32_t>((y + 1) * y_scale), src_height);

      for (int32_t c = 0; c < channels; c++) {

        uint32_t sum = 0;
        uint32_t count = 0;

        for (int32_t sy = src_y_start; sy < src_y_end; sy++) {
          for (int32_t sx = src_x_start; sx < src_x_end; sx++) {
            sum += src[(sy * src_width + sx) * channels + c];
            count++;
          }
        }

        dst[(y * dst_width + x) * channels + c] = static_cast<uint8_t>(sum / count);
      }
    }
  }
}
