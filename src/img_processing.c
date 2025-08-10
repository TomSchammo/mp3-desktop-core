#include "../include/img_processing.h"
#include <assert.h>

#if __has_include(<arm_neon.h>)
#include "arm_neon.h"
#endif

void downscale_area_average(Image *src, Image *dst) {

  const float x_scale = ((float)(src->img_width)) / dst->img_width;
  const float y_scale = ((float)(src->img_height)) / dst->img_height;

  // downscaling only
  assert(x_scale > 1.0f);
  assert(y_scale > 1.0f);

  // for (size_t idx = 0; idx < dst_height * dst_width; idx++) {

  for (uint32_t y = 0; y < dst->img_height; y++) {
    for (uint32_t x = 0; x < dst->img_width; x++) {

      // const auto y = idx / dst_width;
      // const auto x = idx % dst_height;

      const uint32_t src_x_start = (uint32_t)(x * x_scale);
      const uint32_t src_y_start = (uint32_t)(y * y_scale);

      const uint32_t src_x_end = (uint32_t)((x + 1) * x_scale) < src->img_width
                                     ? (uint32_t)((x + 1) * x_scale)
                                     : src->img_width;
      const uint32_t src_y_end = (uint32_t)((y + 1) * y_scale) < src->img_height
                                     ? (uint32_t)((y + 1) * y_scale)
                                     : src->img_height;

      const uint32_t pixel_count = (src_x_end - src_x_start) * (src_y_end - src_y_start);

      for (int32_t c = 0; c < 3; c++) {

        uint32_t sum = 0;

        for (int32_t sy = src_y_start; sy < src_y_end; sy++) {
          for (int32_t sx = src_x_start; sx < src_x_end; sx++) {
            size_t src_idx = (sy * src->img_width + sx) * 3 + c;
            assert(src_idx < src->length);
            sum += src->buffer[src_idx];
          }
        }

        size_t dst_idx = (y * dst->img_width + x) * 3 + c;
        assert(dst_idx < dst->length);
        dst->buffer[dst_idx] = (uint8_t)(sum / pixel_count);
      }
    }
  }
}

void rgb888_to_rgb565_scalar(Image *src, Image *dst) {

  for (int i = 0; i < src->img_width * src->img_height; i++) {
    uint8_t r = src->buffer[i * 3 + 0];
    uint8_t g = src->buffer[i * 3 + 1];
    uint8_t b = src->buffer[i * 3 + 2];

    // Add half the lost precision before truncating
    uint16_t r5 = (r + 4) >> 3; // +4 is half of 8 (2^3)
    uint16_t g6 = (g + 2) >> 2; // +2 is half of 4 (2^2)
    uint16_t b5 = (b + 4) >> 3; // +4 is half of 8 (2^3)

    // Clamp to prevent overflow
    if (r5 > 31)
      r5 = 31;
    if (g6 > 63)
      g6 = 63;
    if (b5 > 31)
      b5 = 31;

    uint16_t value = (r5 << 11) | (g6 << 5) | b5;

    dst->buffer[i] = value;
  }
}

#if __has_include(<arm_neon.h>)

void rgb888_to_rgb565_neon(Image *src, Image *dst) {

  const uint16x8_t v_4 = vdupq_n_u16(4);
  const uint16x8_t v_2 = vdupq_n_u16(2);
  const uint16x8_t v_31 = vdupq_n_u16(31);
  const uint16x8_t v_63 = vdupq_n_u16(63);

  // expecting 200x200 image
  assert(src->img_width == 200);
  assert(src->img_height == 200);
  assert(dst->img_width == 200);
  assert(dst->img_height == 200);

  uint8x8x3_t v_rgb888;
  uint16x8_t v_rgb565;

  for (int i = 0; i < src->img_width * src->img_height; i += 8) {

    v_rgb888 = vld3_u8(src->buffer + (i * 3));

    uint16x8_t v_r = vmovl_u8(v_rgb888.val[0]);
    uint16x8_t v_g = vmovl_u8(v_rgb888.val[1]);
    uint16x8_t v_b = vmovl_u8(v_rgb888.val[2]);

    v_r = vaddq_u16(v_r, v_4);
    v_g = vaddq_u16(v_g, v_2);
    v_b = vaddq_u16(v_b, v_4);

    v_r = vshrq_n_u16(v_r, 3);
    v_g = vshrq_n_u16(v_g, 2);
    v_b = vshrq_n_u16(v_b, 3);

    v_r = vminq_u16(v_r, v_31);
    v_g = vminq_u16(v_g, v_63);
    v_b = vminq_u16(v_b, v_31);

    v_r = vshlq_n_u16(v_r, 11);
    v_g = vshlq_n_u16(v_g, 5);

    v_rgb565 = vorrq_u16(v_r, v_g);
    v_rgb565 = vorrq_u16(v_rgb565, v_b);

    vst1q_u16((uint16_t *)dst->buffer + i, v_rgb565);
  }
}

void rgb888_to_rgb565_neon_alt(Image *src, Image *dst) {

  const uint16x8_t v_4 = vdupq_n_u16(4);
  const uint16x8_t v_2 = vdupq_n_u16(2);
  const uint16x8_t v_31 = vdupq_n_u16(31);
  const uint16x8_t v_63 = vdupq_n_u16(63);

  // expecting 200x200 image
  assert(src->img_width == 200);
  assert(src->img_height == 200);
  assert(dst->img_width == 200);
  assert(dst->img_height == 200);

  uint8x16x3_t v_rgb888;
  uint16x8_t v_rgb565_high;
  uint16x8_t v_rgb565_low;

  for (int i = 0; i < src->img_width * src->img_height; i += 16) {

    v_rgb888 = vld3q_u8(src->buffer + (i * 3));

    uint16x8_t v_r8_high = vmovl_high_u8(v_rgb888.val[0]);
    uint16x8_t v_r8_low = vmovl_u8(vget_low_u8(v_rgb888.val[0]));

    uint16x8_t v_g8_high = vmovl_high_u8(v_rgb888.val[1]);
    uint16x8_t v_g8_low = vmovl_u8(vget_low_u8(v_rgb888.val[1]));

    uint16x8_t v_b8_high = vmovl_high_u8(v_rgb888.val[2]);
    uint16x8_t v_b8_low = vmovl_u8(vget_low_u8(v_rgb888.val[2]));

    v_r8_high = vaddq_u16(v_r8_high, v_4);
    v_g8_high = vaddq_u16(v_g8_high, v_2);
    v_b8_high = vaddq_u16(v_b8_high, v_4);

    v_r8_low = vaddq_u16(v_r8_low, v_4);
    v_g8_low = vaddq_u16(v_g8_low, v_2);
    v_b8_low = vaddq_u16(v_b8_low, v_4);

    v_r8_high = vshrq_n_u16(v_r8_high, 3);
    v_g8_high = vshrq_n_u16(v_g8_high, 2);
    v_b8_high = vshrq_n_u16(v_b8_high, 3);

    v_r8_low = vshrq_n_u16(v_r8_low, 3);
    v_g8_low = vshrq_n_u16(v_g8_low, 2);
    v_b8_low = vshrq_n_u16(v_b8_low, 3);

    v_r8_high = vminq_u16(v_r8_high, v_31);
    v_g8_high = vminq_u16(v_g8_high, v_63);
    v_b8_high = vminq_u16(v_b8_high, v_31);

    v_r8_low = vminq_u16(v_r8_low, v_31);
    v_g8_low = vminq_u16(v_g8_low, v_63);
    v_b8_low = vminq_u16(v_b8_low, v_31);

    v_r8_high = vshlq_n_u16(v_r8_high, 11);
    v_g8_high = vshlq_n_u16(v_g8_high, 5);

    v_r8_low = vshlq_n_u16(v_r8_low, 11);
    v_g8_low = vshlq_n_u16(v_g8_low, 5);

    v_rgb565_high = vorrq_u16(v_r8_high, v_g8_high);
    v_rgb565_high = vorrq_u16(v_rgb565_high, v_b8_high);

    v_rgb565_low = vorrq_u16(v_r8_low, v_g8_low);
    v_rgb565_low = vorrq_u16(v_rgb565_low, v_b8_low);

    vst1q_u16((uint16_t *)dst->buffer + i + 8, v_rgb565_high);
    vst1q_u16((uint16_t *)dst->buffer + i, v_rgb565_low);
  }
}
#endif

/*
void downscale_area_average_forward(uint8_t *src, uint32_t src_width, uint32_t src_height,
                                    uint8_t *dst, uint32_t dst_width, uint32_t dst_height,
                                    int32_t channels) {

  const float x_scale = ((float)(src_width)) / dst_width;
  const float y_scale = ((float)(src_height)) / dst_height;

  // float32x4_t x_scale_v = {x_scale, x_scale, x_scale, x_scale};
  // float32x4_t y_scale_v = {y_scale, y_scale, y_scale, y_scale};

  // downscaling only
  assert(x_scale > 1.0f);
  assert(y_scale > 1.0f);

  std::vector<int32_t> counts;
  counts.resize(dst_width * dst_height);

  std::vector<uint32_t> tmp_dst;
  tmp_dst.resize(dst_width * dst_height * channels);

  for (size_t idx = 0; idx < src_height * src_width; idx++) {
    const auto y = idx / src_width;
    const auto x = idx % src_height;

    const auto dst_x = static_cast<size_t>(x / x_scale);
    const auto dst_y = static_cast<size_t>(y / y_scale);

    counts[dst_y * dst_width + dst_x] += 1;

    for (int32_t c = 0; c < channels; c++) {
      tmp_dst[(dst_y * dst_width + dst_x) * channels + c] +=
          src[(y * src_width + x) * channels + c];
    }
  }

  for (size_t idx_pixel = 0; idx_pixel < dst_height * dst_width; idx_pixel++) {
    const auto y = idx_pixel / dst_width;
    const auto x = idx_pixel % dst_height;

    size_t idx = y * dst_width + x;
    for (uint8_t c = 0; c < channels; c++) {
      if (counts[idx] != 0)
        dst[(y * dst_width + x) * channels + c] =
            tmp_dst[(y * dst_width + x) * channels + c] / counts[idx];
    }
  }
}*/
