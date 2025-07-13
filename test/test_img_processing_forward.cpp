#include "img_processing.hpp"
#include <algorithm>
#include <cmath>
#include <gtest/gtest.h>
#include <limits>
#include <vector>

class DownscaleAreaAverageForwardTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for tests
  }

  void TearDown() override {
    // Common cleanup for tests
  }

  // Helper function to create a test image with known pattern
  std::vector<uint8_t> createTestImage(uint32_t width, uint32_t height, int32_t channels) {
    std::vector<uint8_t> image(width * height * channels);
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        for (int32_t c = 0; c < channels; c++) {
          // Create a predictable pattern: value increases with position
          image[(y * width + x) * channels + c] = (x + y + c * 10) % 256;
        }
      }
    }
    return image;
  }

  // Helper function to create uniform color image
  std::vector<uint8_t> createUniformImage(uint32_t width, uint32_t height, int32_t channels,
                                          uint8_t value) {
    std::vector<uint8_t> image(width * height * channels, value);
    return image;
  }

  // Helper function to create gradient image
  std::vector<uint8_t> createGradientImage(uint32_t width, uint32_t height, int32_t channels) {
    std::vector<uint8_t> image(width * height * channels);
    for (uint32_t y = 0; y < height; y++) {
      for (uint32_t x = 0; x < width; x++) {
        for (int32_t c = 0; c < channels; c++) {
          // Create gradient based on position
          int value = (x * 255 / (width - 1)) + (y * 255 / (height - 1));
          image[(y * width + x) * channels + c] = std::min(255, value / 2);
        }
      }
    }
    return image;
  }

  // Helper function to create high-value uniform image for overflow testing
  std::vector<uint8_t> createHighValueImage(uint32_t width, uint32_t height, int32_t channels,
                                            uint8_t value = 255) {
    std::vector<uint8_t> image(width * height * channels, value);
    return image;
  }
};

// Test basic downscaling functionality
TEST_F(DownscaleAreaAverageForwardTest, BasicDownscaling) {
  // Create a 4x4 image with 3 channels, downscale to 2x2
  auto src = createTestImage(4, 4, 3);
  std::vector<uint8_t> dst(2 * 2 * 3);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 3);

  // Verify the output has the expected size
  EXPECT_EQ(dst.size(), 2 * 2 * 3);

  // The result should be valid (reasonable values)
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test uniform color image downscaling
TEST_F(DownscaleAreaAverageForwardTest, UniformColor) {
  // Create a uniform 4x4 image with value 100 for all channels
  auto src = createUniformImage(4, 4, 3, 100);
  std::vector<uint8_t> dst(2 * 2 * 3);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 3);

  // All output pixels should have the same value as input
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_EQ(dst[i], 100);
  }
}

// Test mathematical correctness with known values
TEST_F(DownscaleAreaAverageForwardTest, MathematicalCorrectness) {
  // Create a 2x2 image with known values for 1 channel, downscale to 1x1
  std::vector<uint8_t> src = {
      10, 20, // Row 0
      30, 40  // Row 1
  };
  std::vector<uint8_t> dst(1 * 1 * 1);

  downscale_area_average_forward(src.data(), 2, 2, dst.data(), 1, 1, 1);

  // Expected average: (10 + 20 + 30 + 40) / 4 = 25
  EXPECT_EQ(dst[0], 25);
}

// Test different channel counts
TEST_F(DownscaleAreaAverageForwardTest, SingleChannel) {
  auto src = createTestImage(4, 4, 1);
  std::vector<uint8_t> dst(2 * 2 * 1);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 1);

  // Should not crash and produce valid output
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

TEST_F(DownscaleAreaAverageForwardTest, FourChannels) {
  auto src = createTestImage(4, 4, 4);
  std::vector<uint8_t> dst(2 * 2 * 4);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 4);

  // Should not crash and produce valid output
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test extreme downscaling
TEST_F(DownscaleAreaAverageForwardTest, ExtremeDownscaling) {
  // Scale from 8x8 to 1x1
  auto src = createTestImage(8, 8, 3);
  std::vector<uint8_t> dst(1 * 1 * 3);

  downscale_area_average_forward(src.data(), 8, 8, dst.data(), 1, 1, 3);

  // Should produce valid output
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test specific averaging calculation
TEST_F(DownscaleAreaAverageForwardTest, SpecificAverageCalculation) {
  // Create a 4x4 image with specific pattern for 1 channel
  std::vector<uint8_t> src = {
      10,  20,  30,  40,  // Row 0
      50,  60,  70,  80,  // Row 1
      90,  100, 110, 120, // Row 2
      130, 140, 150, 160  // Row 3
  };
  std::vector<uint8_t> dst(2 * 2 * 1);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 1);

  // Top-left quadrant: (10 + 20 + 50 + 60) / 4 = 35
  EXPECT_EQ(dst[0], 35);

  // Top-right quadrant: (30 + 40 + 70 + 80) / 4 = 55
  EXPECT_EQ(dst[1], 55);

  // Bottom-left quadrant: (90 + 100 + 130 + 140) / 4 = 115
  EXPECT_EQ(dst[2], 115);

  // Bottom-right quadrant: (110 + 120 + 150 + 160) / 4 = 135
  EXPECT_EQ(dst[3], 135);
}

// Test non-integer scaling factors
TEST_F(DownscaleAreaAverageForwardTest, NonIntegerScaling) {
  // Scale from 3x3 to 2x2 (1.5x scaling factor)
  auto src = createTestImage(3, 3, 3);
  std::vector<uint8_t> dst(2 * 2 * 3);

  downscale_area_average_forward(src.data(), 3, 3, dst.data(), 2, 2, 3);

  // Should produce valid output without crashes
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test rectangular images (non-square)
TEST_F(DownscaleAreaAverageForwardTest, RectangularImages) {
  // Test with 4x2 to 2x1 scaling
  auto src = createTestImage(4, 2, 3);
  std::vector<uint8_t> dst(2 * 1 * 3);

  downscale_area_average_forward(src.data(), 4, 2, dst.data(), 2, 1, 3);

  // Should produce valid output
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test boundary conditions
TEST_F(DownscaleAreaAverageForwardTest, BoundaryConditions) {
  // Test with dimensions that might cause boundary issues
  auto src = createTestImage(5, 5, 3);
  std::vector<uint8_t> dst(3 * 3 * 3);

  downscale_area_average_forward(src.data(), 5, 5, dst.data(), 3, 3, 3);

  // Should handle boundary conditions properly
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_GE(dst[i], 0);
    EXPECT_LE(dst[i], 255);
  }
}

// Test large image overflow concern
TEST_F(DownscaleAreaAverageForwardTest, LargeImageOverflowTest) {
  // Test with worst-case scenario for overflow
  // Large downscaling with maximum pixel values (255)
  // This tests the uint32_t sum can handle large accumulation

  // Create a 256x256 image with max values, downscale to 1x1
  // This would sum 256*256*255 = 16,711,680 which is well within uint32_t range
  auto src = createHighValueImage(256, 256, 1, 255);
  std::vector<uint8_t> dst(1 * 1 * 1);

  downscale_area_average_forward(src.data(), 256, 256, dst.data(), 1, 1, 1);

  // Should produce the maximum value (255) since all input pixels are 255
  EXPECT_EQ(dst[0], 255);
}

// Test potential overflow edge case
TEST_F(DownscaleAreaAverageForwardTest, LargeImageOverflowEdgeCase) {
  // Test with a scenario that approaches uint32_t limits
  // 512x512 image with max values would sum to 512*512*255 = 66,846,720
  // Still within uint32_t range (4,294,967,295) but getting closer

  auto src = createHighValueImage(512, 512, 1, 255);
  std::vector<uint8_t> dst(1 * 1 * 1);

  downscale_area_average_forward(src.data(), 512, 512, dst.data(), 1, 1, 1);

  // Should still produce correct result
  EXPECT_EQ(dst[0], 255);
}

// Test mixed value averaging to verify correct arithmetic
TEST_F(DownscaleAreaAverageForwardTest, MixedValueAveraging) {
  // Test with alternating high and low values
  std::vector<uint8_t> src = {
      255, 0,   255, 0,   // Row 0
      0,   255, 0,   255, // Row 1
      255, 0,   255, 0,   // Row 2
      0,   255, 0,   255  // Row 3
  };
  std::vector<uint8_t> dst(2 * 2 * 1);

  downscale_area_average_forward(src.data(), 4, 4, dst.data(), 2, 2, 1);

  // Each 2x2 quadrant should average to approximately 127.5 -> 127
  for (size_t i = 0; i < dst.size(); i++) {
    EXPECT_EQ(dst[i], 127);
  }
}
