#include "../include/img_processing.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

void downscale_area_average(uint8_t *src, uint32_t src_width, uint32_t src_height, uint8_t *dst,
                            uint32_t dst_width, uint32_t dst_height, int32_t channels) {

  const auto x_scale = static_cast<float>(src_width) / dst_width;
  const auto y_scale = static_cast<float>(src_height) / dst_height;

  // downscaling only
  assert(x_scale > 1.0f);
  assert(y_scale > 1.0f);

  for (uint32_t y = 0; y < dst_height; y++) {
    for (uint32_t x = 0; x < dst_width; x++) {

      const auto src_x_start = static_cast<uint32_t>(x * x_scale);
      const auto src_y_start = static_cast<uint32_t>(y * y_scale);

      const auto src_x_end = std::min(static_cast<uint32_t>((x + 1) * x_scale), src_width);
      const auto src_y_end = std::min(static_cast<uint32_t>((y + 1) * y_scale), src_height);

      const auto pixel_count = (src_x_end - src_x_start) * (src_y_end - src_y_start);

      for (int32_t c = 0; c < channels; c++) {

        uint32_t sum = 0;

        for (int32_t sy = src_y_start; sy < src_y_end; sy++) {
          for (int32_t sx = src_x_start; sx < src_x_end; sx++) {
            sum += src[(sy * src_width + sx) * channels + c];
          }
        }

        dst[(y * dst_width + x) * channels + c] = static_cast<uint8_t>(sum / pixel_count);
      }
    }
  }
}

void downscale_area_average_forward(uint8_t *src, uint32_t src_width, uint32_t src_height,
                                    uint8_t *dst, uint32_t dst_width, uint32_t dst_height,
                                    int32_t channels) {

  const auto x_scale = static_cast<float>(src_width) / dst_width;
  const auto y_scale = static_cast<float>(src_height) / dst_height;

  // float32x4_t x_scale_v = {x_scale, x_scale, x_scale, x_scale};
  // float32x4_t y_scale_v = {y_scale, y_scale, y_scale, y_scale};

  // downscaling only
  assert(x_scale > 1.0f);
  assert(y_scale > 1.0f);

  std::vector<int32_t> counts;
  counts.resize(dst_width * dst_height);

  std::vector<uint32_t> tmp_dst;
  tmp_dst.resize(dst_width * dst_height * channels);

  for (uint32_t y = 0; y < src_height; y++) {
    const auto dst_y = static_cast<uint32_t>(y / y_scale);

    for (uint32_t x = 0; x < src_width; x++) {
      const auto dst_x = static_cast<uint32_t>(x / x_scale);

      counts[dst_y * dst_width + dst_x] += 1;

      for (int32_t c = 0; c < channels; c++) {
        tmp_dst[(dst_y * dst_width + dst_x) * channels + c] +=
            src[(y * src_width + x) * channels + c];
      }
    }
  }

  for (uint32_t y = 0; y < dst_height; y++) {
    for (uint32_t x = 0; x < dst_width; x++) {

      for (int32_t c = 0; c < channels; c++) {
        size_t idx = y * dst_width + x;
        if (counts[idx] != 0)
          dst[(y * dst_width + x) * channels + c] =
              tmp_dst[(y * dst_width + x) * channels + c] / counts[idx];
      }
    }
  }
}
