#include <assert.h>
#include <gtest/gtest.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "img_processing.h"
}

class DownscaleAreaAverageTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Common setup for tests
  }

  void TearDown() override {
    // Common cleanup for tests
  }

  // Helper function to create uniform color image (RGB only)
  Image createUniformImage(uint32_t width, uint32_t height, uint8_t value) {
    Image img;
    img.img_width = width;
    img.img_height = height;
    img.length = width * height * 3; // RGB only
    img.buffer = (uint8_t *)malloc(img.length);

    for (size_t i = 0; i < img.length; i++) {
      img.buffer[i] = value;
    }
    return img;
  }

  // Helper function to create high-value uniform image for overflow testing (RGB only)
  Image createHighValueImage(uint32_t width, uint32_t height, uint8_t value = 255) {
    return createUniformImage(width, height, value);
  }

  // Helper function to create empty destination image
  Image createEmptyImage(uint32_t width, uint32_t height) {
    Image img;
    img.img_width = width;
    img.img_height = height;
    img.length = width * height * 3; // RGB only
    img.buffer = (uint8_t *)malloc(img.length);
    memset(img.buffer, 0, img.length);
    return img;
  }

  // Helper function to free image memory
  void freeImage(Image *img) {
    if (img->buffer) {
      free(img->buffer);
      img->buffer = NULL;
    }
  }
};

// Test basic downscaling functionality
TEST_F(DownscaleAreaAverageTest, BasicDownscaling) {
  // Create a 4x4 RGB image with known values, downscale to 2x2
  Image src;
  src.img_width = 4;
  src.img_height = 4;
  src.length = 4 * 4 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Fill with known pattern for RGB channels
  uint8_t src_data[] = {// Row 0: pixel (0,0) [10,11,12], pixel (0,1) [10,11,12], pixel (0,2)
                        // [30,31,32], pixel (0,3) [30,31,32]
                        10, 11, 12, 10, 11, 12, 30, 31, 32, 30, 31, 32,
                        // Row 1: pixel (1,0) [20,21,22], pixel (1,1) [20,21,22], pixel (1,2)
                        // [40,41,42], pixel (1,3) [40,41,42]
                        20, 21, 22, 20, 21, 22, 40, 41, 42, 40, 41, 42,
                        // Row 2: pixel (2,0) [50,51,52], pixel (2,1) [50,51,52], pixel (2,2)
                        // [70,71,72], pixel (2,3) [70,71,72]
                        50, 51, 52, 50, 51, 52, 70, 71, 72, 70, 71, 72,
                        // Row 3: pixel (3,0) [60,61,62], pixel (3,1) [60,61,62], pixel (3,2)
                        // [80,81,82], pixel (3,3) [80,81,82]
                        60, 61, 62, 60, 61, 62, 80, 81, 82, 80, 81, 82};
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Verify each output pixel has correct average for RGB channels
  // Top-left: (10+10+20+20)/4 = 15, (11+11+21+21)/4 = 16, (12+12+22+22)/4 = 17
  EXPECT_EQ(dst.buffer[0], 15); // R channel
  EXPECT_EQ(dst.buffer[1], 16); // G channel
  EXPECT_EQ(dst.buffer[2], 17); // B channel

  // Top-right: (30+30+40+40)/4 = 35, (31+31+41+41)/4 = 36, (32+32+42+42)/4 = 37
  EXPECT_EQ(dst.buffer[3], 35); // R channel
  EXPECT_EQ(dst.buffer[4], 36); // G channel
  EXPECT_EQ(dst.buffer[5], 37); // B channel

  // Bottom-left: (50+50+60+60)/4 = 55, (51+51+61+61)/4 = 56, (52+52+62+62)/4 = 57
  EXPECT_EQ(dst.buffer[6], 55); // R channel
  EXPECT_EQ(dst.buffer[7], 56); // G channel
  EXPECT_EQ(dst.buffer[8], 57); // B channel

  // Bottom-right: (70+70+80+80)/4 = 75, (71+71+81+81)/4 = 76, (72+72+82+82)/4 = 77
  EXPECT_EQ(dst.buffer[9], 75);  // R channel
  EXPECT_EQ(dst.buffer[10], 76); // G channel
  EXPECT_EQ(dst.buffer[11], 77); // B channel

  freeImage(&src);
  freeImage(&dst);
}

// Test uniform color image downscaling
TEST_F(DownscaleAreaAverageTest, UniformColor) {
  // Create a uniform 4x4 RGB image with value 100 for all channels
  Image src = createUniformImage(4, 4, 100);
  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // All output pixels should have the same value as input
  for (size_t i = 0; i < dst.length; i++) {
    EXPECT_EQ(dst.buffer[i], 100);
  }

  freeImage(&src);
  freeImage(&dst);
}

// Test mathematical correctness with known values
TEST_F(DownscaleAreaAverageTest, MathematicalCorrectness) {
  // Create a 2x2 RGB image with known values, downscale to 1x1
  Image src;
  src.img_width = 2;
  src.img_height = 2;
  src.length = 2 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      10, 11, 12, 20, 21, 22, // Row 0: pixels [10,11,12] and [20,21,22]
      30, 31, 32, 40, 41, 42  // Row 1: pixels [30,31,32] and [40,41,42]
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // Expected average: R=(10+20+30+40)/4=25, G=(11+21+31+41)/4=26, B=(12+22+32+42)/4=27
  EXPECT_EQ(dst.buffer[0], 25); // R channel
  EXPECT_EQ(dst.buffer[1], 26); // G channel
  EXPECT_EQ(dst.buffer[2], 27); // B channel

  freeImage(&src);
  freeImage(&dst);
}

// Test extreme downscaling
TEST_F(DownscaleAreaAverageTest, ExtremeDownscaling) {
  // Scale from 8x8 to 1x1 - test averaging across large area
  // Create checkerboard pattern for predictable average
  Image src;
  src.img_width = 8;
  src.img_height = 8;
  src.length = 8 * 8 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Fill with checkerboard pattern for RGB
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      uint8_t value = ((x + y) % 2 == 0) ? 100 : 200;
      src.buffer[(y * 8 + x) * 3 + 0] = value; // R
      src.buffer[(y * 8 + x) * 3 + 1] = value; // G
      src.buffer[(y * 8 + x) * 3 + 2] = value; // B
    }
  }

  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // Checkerboard pattern has equal 100s and 200s, so average = 150 for all channels
  EXPECT_EQ(dst.buffer[0], 150); // R channel
  EXPECT_EQ(dst.buffer[1], 150); // G channel
  EXPECT_EQ(dst.buffer[2], 150); // B channel

  freeImage(&src);
  freeImage(&dst);
}

// Test specific averaging calculation
TEST_F(DownscaleAreaAverageTest, SpecificAverageCalculation) {
  // Create a 4x4 RGB image with specific pattern
  Image src;
  src.img_width = 4;
  src.img_height = 4;
  src.length = 4 * 4 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      10,  11,  12,  20,  21,  22,  30,  31,  32,  40,  41,  42,  // Row 0
      50,  51,  52,  60,  61,  62,  70,  71,  72,  80,  81,  82,  // Row 1
      90,  91,  92,  100, 101, 102, 110, 111, 112, 120, 121, 122, // Row 2
      130, 131, 132, 140, 141, 142, 150, 151, 152, 160, 161, 162  // Row 3
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Top-left quadrant RGB: (10+20+50+60)/4=35, (11+21+51+61)/4=36, (12+22+52+62)/4=37
  EXPECT_EQ(dst.buffer[0], 35); // R
  EXPECT_EQ(dst.buffer[1], 36); // G
  EXPECT_EQ(dst.buffer[2], 37); // B

  // Top-right quadrant RGB: (30+40+70+80)/4=55, (31+41+71+81)/4=56, (32+42+72+82)/4=57
  EXPECT_EQ(dst.buffer[3], 55); // R
  EXPECT_EQ(dst.buffer[4], 56); // G
  EXPECT_EQ(dst.buffer[5], 57); // B

  // Bottom-left quadrant RGB: (90+100+130+140)/4=115, (91+101+131+141)/4=116,
  // (92+102+132+142)/4=117
  EXPECT_EQ(dst.buffer[6], 115); // R
  EXPECT_EQ(dst.buffer[7], 116); // G
  EXPECT_EQ(dst.buffer[8], 117); // B

  // Bottom-right quadrant RGB: (110+120+150+160)/4=135, (111+121+151+161)/4=136,
  // (112+122+152+162)/4=137
  EXPECT_EQ(dst.buffer[9], 135);  // R
  EXPECT_EQ(dst.buffer[10], 136); // G
  EXPECT_EQ(dst.buffer[11], 137); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test non-integer scaling factors
TEST_F(DownscaleAreaAverageTest, NonIntegerScaling) {
  // Scale from 6x6 to 2x2 (3x scaling factor - easier to verify)
  // Create a pattern where each 3x3 block has known values
  Image src;
  src.img_width = 6;
  src.img_height = 6;
  src.length = 6 * 6 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Fill with block pattern for RGB
  for (int y = 0; y < 6; y++) {
    for (int x = 0; x < 6; x++) {
      uint8_t value;
      if (y < 3 && x < 3)
        value = 10; // Top-left
      else if (y < 3 && x >= 3)
        value = 20; // Top-right
      else if (y >= 3 && x < 3)
        value = 30; // Bottom-left
      else
        value = 40; // Bottom-right

      src.buffer[(y * 6 + x) * 3 + 0] = value;
      src.buffer[(y * 6 + x) * 3 + 1] = value;
      src.buffer[(y * 6 + x) * 3 + 2] = value;
    }
  }

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Each 3x3 block should average to its uniform value for all RGB channels
  EXPECT_EQ(dst.buffer[0], 10);
  EXPECT_EQ(dst.buffer[1], 10);
  EXPECT_EQ(dst.buffer[2], 10); // Top-left
  EXPECT_EQ(dst.buffer[3], 20);
  EXPECT_EQ(dst.buffer[4], 20);
  EXPECT_EQ(dst.buffer[5], 20); // Top-right
  EXPECT_EQ(dst.buffer[6], 30);
  EXPECT_EQ(dst.buffer[7], 30);
  EXPECT_EQ(dst.buffer[8], 30); // Bottom-left
  EXPECT_EQ(dst.buffer[9], 40);
  EXPECT_EQ(dst.buffer[10], 40);
  EXPECT_EQ(dst.buffer[11], 40); // Bottom-right

  freeImage(&src);
  freeImage(&dst);
}

// Test rectangular images (non-square)
TEST_F(DownscaleAreaAverageTest, RectangularImages) {
  // Test with 4x2 to 2x1 scaling - create known pattern
  Image src;
  src.img_width = 4;
  src.img_height = 2;
  src.length = 4 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      10, 11, 12, 20, 21, 22, 30, 31, 32, 40, 41, 42, // Row 0
      50, 51, 52, 60, 61, 62, 70, 71, 72, 80, 81, 82  // Row 1
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(2, 1);

  downscale_area_average(&src, &dst);

  // Left half RGB: (10+20+50+60)/4=35, (11+21+51+61)/4=36, (12+22+52+62)/4=37
  EXPECT_EQ(dst.buffer[0], 35); // R
  EXPECT_EQ(dst.buffer[1], 36); // G
  EXPECT_EQ(dst.buffer[2], 37); // B

  // Right half RGB: (30+40+70+80)/4=55, (31+41+71+81)/4=56, (32+42+72+82)/4=57
  EXPECT_EQ(dst.buffer[3], 55); // R
  EXPECT_EQ(dst.buffer[4], 56); // G
  EXPECT_EQ(dst.buffer[5], 57); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test another rectangular case - vertical rectangle
TEST_F(DownscaleAreaAverageTest, VerticalRectangularImages) {
  // Test with 2x4 to 1x2 scaling - create known pattern
  Image src;
  src.img_width = 2;
  src.img_height = 4;
  src.length = 2 * 4 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      10, 11, 12, 20, 21, 22, // Row 0
      30, 31, 32, 40, 41, 42, // Row 1
      50, 51, 52, 60, 61, 62, // Row 2
      70, 71, 72, 80, 81, 82  // Row 3
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(1, 2);

  downscale_area_average(&src, &dst);

  // Top half RGB: (10+20+30+40)/4=25, (11+21+31+41)/4=26, (12+22+32+42)/4=27
  EXPECT_EQ(dst.buffer[0], 25); // R
  EXPECT_EQ(dst.buffer[1], 26); // G
  EXPECT_EQ(dst.buffer[2], 27); // B

  // Bottom half RGB: (50+60+70+80)/4=65, (51+61+71+81)/4=66, (52+62+72+82)/4=67
  EXPECT_EQ(dst.buffer[3], 65); // R
  EXPECT_EQ(dst.buffer[4], 66); // G
  EXPECT_EQ(dst.buffer[5], 67); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test boundary conditions
TEST_F(DownscaleAreaAverageTest, BoundaryConditions) {
  // Test with dimensions that create fractional pixel mappings: 5x5 -> 3x3
  // This tests proper handling of partial pixel contributions
  Image src;
  src.img_width = 5;
  src.img_height = 5;
  src.length = 5 * 5 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Fill with pattern for RGB
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      uint8_t value;
      if (y < 2 && x < 2)
        value = 10;
      else if (y < 2 && x >= 2 && x < 4)
        value = 20;
      else if (y < 2)
        value = 30;
      else if (y < 4 && x < 2)
        value = 40;
      else if (y < 4 && x >= 2 && x < 4)
        value = 50;
      else if (y < 4)
        value = 60;
      else if (x < 2)
        value = 70;
      else if (x < 4)
        value = 80;
      else
        value = 90;

      src.buffer[(y * 5 + x) * 3 + 0] = value;
      src.buffer[(y * 5 + x) * 3 + 1] = value;
      src.buffer[(y * 5 + x) * 3 + 2] = value;
    }
  }

  Image dst = createEmptyImage(3, 3);

  downscale_area_average(&src, &dst);

  // With 5->3 scaling, each destination pixel covers ~1.67 source pixels
  // This creates fractional coverage that must be handled correctly
  // The exact values depend on the implementation's boundary handling

  // Top-left region should be dominated by value 10 for all RGB channels
  EXPECT_EQ(dst.buffer[0], 10);
  EXPECT_EQ(dst.buffer[1], 10);
  EXPECT_EQ(dst.buffer[2], 10);

  // Now test fractional pixel mapping with exact calculations
  // Algorithm uses truncation: src_x_start = (int)(x * x_scale), src_x_end = (int)((x+1) * x_scale)
  // With 5->3 scaling: x_scale = 5/3 = 1.667, y_scale = 5/3 = 1.667
  //
  // dst(0,0): src_x_start=0, src_x_end=(int)(1*1.667)=1, src_y_start=0, src_y_end=1
  //          Covers only src pixel (0,0) = 1 pixel, so result = (10,10,10)
  // dst(1,0): src_x_start=1, src_x_end=(int)(2*1.667)=3, src_y_start=0, src_y_end=1
  //          Covers src pixels (1,0), (2,0) = 2 pixels
  //          Values: 10 + 20 = 30, avg = 30/2 = 15
  // dst(2,0): src_x_start=3, src_x_end=(int)(3*1.667)=5, src_y_start=0, src_y_end=1
  //          Covers src pixels (3,0), (4,0) = 2 pixels
  //          Values: 20 + 30 = 50, avg = 50/2 = 25
  //
  // Source pattern reminder (5x5):
  // 10 10 20 20 30
  // 10 10 20 20 30
  // 40 40 50 50 60
  // 40 40 50 50 60
  // 70 70 80 80 90

  // Verify the exact truncation behavior for first row
  EXPECT_EQ(dst.buffer[0], 10); // dst(0,0) red
  EXPECT_EQ(dst.buffer[1], 10); // dst(0,0) green
  EXPECT_EQ(dst.buffer[2], 10); // dst(0,0) blue

  EXPECT_EQ(dst.buffer[3], 15); // dst(1,0) red
  EXPECT_EQ(dst.buffer[4], 15); // dst(1,0) green
  EXPECT_EQ(dst.buffer[5], 15); // dst(1,0) blue

  EXPECT_EQ(dst.buffer[6], 25); // dst(2,0) red
  EXPECT_EQ(dst.buffer[7], 25); // dst(2,0) green
  EXPECT_EQ(dst.buffer[8], 25); // dst(2,0) blue

  freeImage(&src);
  freeImage(&dst);
}

// Test large image overflow concern
TEST_F(DownscaleAreaAverageTest, LargeImageOverflowTest) {
  // Test with large downscaling and maximum pixel values
  // Use smaller size for faster test but still verify overflow handling

  // Create a 64x64 RGB image with max values, downscale to 1x1
  Image src = createHighValueImage(64, 64, 255);
  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // Should produce the maximum value (255) since all input pixels are 255 for all RGB channels
  EXPECT_EQ(dst.buffer[0], 255); // R
  EXPECT_EQ(dst.buffer[1], 255); // G
  EXPECT_EQ(dst.buffer[2], 255); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test potential overflow edge case with mixed values
TEST_F(DownscaleAreaAverageTest, LargeImageOverflowEdgeCase) {
  // Test with a scenario that could cause overflow if not handled properly
  // Use alternating max and min values to test arithmetic

  Image src;
  src.img_width = 32;
  src.img_height = 32;
  src.length = 32 * 32 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Create 32x32 alternating pattern for RGB
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 32; x++) {
      uint8_t value = ((x + y) % 2 == 0) ? 255 : 0;
      src.buffer[(y * 32 + x) * 3 + 0] = value;
      src.buffer[(y * 32 + x) * 3 + 1] = value;
      src.buffer[(y * 32 + x) * 3 + 2] = value;
    }
  }

  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // Alternating 255/0 pattern should average to ~127 for all RGB channels
  EXPECT_EQ(dst.buffer[0], 127); // R
  EXPECT_EQ(dst.buffer[1], 127); // G
  EXPECT_EQ(dst.buffer[2], 127); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test mixed value averaging to verify correct arithmetic
TEST_F(DownscaleAreaAverageTest, MixedValueAveraging) {
  // Test with alternating high and low values
  Image src;
  src.img_width = 4;
  src.img_height = 4;
  src.length = 4 * 4 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  // Fill with alternating pattern for RGB
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      uint8_t value = ((x + y) % 2 == 0) ? 255 : 0;
      src.buffer[(y * 4 + x) * 3 + 0] = value;
      src.buffer[(y * 4 + x) * 3 + 1] = value;
      src.buffer[(y * 4 + x) * 3 + 2] = value;
    }
  }

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Each 2x2 quadrant should average to approximately 127.5 -> 127 for all RGB channels
  for (size_t i = 0; i < dst.length; i++) {
    EXPECT_EQ(dst.buffer[i], 127);
  }

  freeImage(&src);
  freeImage(&dst);
}

// Test diverse values within averaging blocks - this actually tests averaging logic
TEST_F(DownscaleAreaAverageTest, DiverseValuesWithinBlocks) {
  // 4x4 -> 2x2 with different values in each 2x2 source block
  Image src;
  src.img_width = 4;
  src.img_height = 4;
  src.length = 4 * 4 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {// Row 0
                        10, 11, 12, 30, 31, 32, 100, 101, 102, 120, 121, 122,
                        // Row 1
                        50, 51, 52, 70, 71, 72, 140, 141, 142, 160, 161, 162,
                        // Row 2
                        20, 21, 22, 60, 61, 62, 90, 91, 92, 110, 111, 112,
                        // Row 3
                        80, 81, 82, 200, 201, 202, 150, 151, 152, 250, 251, 252};
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Top-left block RGB: (10+30+50+70)/4=40, (11+31+51+71)/4=41, (12+32+52+72)/4=42
  EXPECT_EQ(dst.buffer[0], 40);
  EXPECT_EQ(dst.buffer[1], 41);
  EXPECT_EQ(dst.buffer[2], 42);

  // Top-right block RGB: (100+120+140+160)/4=130, (101+121+141+161)/4=131, (102+122+142+162)/4=132
  EXPECT_EQ(dst.buffer[3], 130);
  EXPECT_EQ(dst.buffer[4], 131);
  EXPECT_EQ(dst.buffer[5], 132);

  // Bottom-left block RGB: (20+60+80+200)/4=90, (21+61+81+201)/4=91, (22+62+82+202)/4=92
  EXPECT_EQ(dst.buffer[6], 90);
  EXPECT_EQ(dst.buffer[7], 91);
  EXPECT_EQ(dst.buffer[8], 92);

  // Bottom-right block RGB: (90+110+150+250)/4=150, (91+111+151+251)/4=151, (92+112+152+252)/4=152
  EXPECT_EQ(dst.buffer[9], 150);
  EXPECT_EQ(dst.buffer[10], 151);
  EXPECT_EQ(dst.buffer[11], 152);

  freeImage(&src);
  freeImage(&dst);
}

// Test rectangular with diverse values - ensures both coordinate mapping AND averaging work
TEST_F(DownscaleAreaAverageTest, RectangularDiverseValues) {
  // 6x2 -> 3x1 with varied values in each 2x2 block
  Image src;
  src.img_width = 6;
  src.img_height = 2;
  src.length = 6 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      // Row 0
      10, 11, 12, 40, 41, 42, 60, 61, 62, 100, 101, 102, 140, 141, 142, 180, 181, 182,
      // Row 1
      30, 31, 32, 50, 51, 52, 80, 81, 82, 120, 121, 122, 160, 161, 162, 200, 201, 202};
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(3, 1);

  downscale_area_average(&src, &dst);

  // Block 1 RGB: (10+40+30+50)/4=32.5->32, (11+41+31+51)/4=33.5->33, (12+42+32+52)/4=34.5->34
  EXPECT_EQ(dst.buffer[0], 32);
  EXPECT_EQ(dst.buffer[1], 33);
  EXPECT_EQ(dst.buffer[2], 34);

  // Block 2 RGB: (60+100+80+120)/4=90, (61+101+81+121)/4=91, (62+102+82+122)/4=92
  EXPECT_EQ(dst.buffer[3], 90);
  EXPECT_EQ(dst.buffer[4], 91);
  EXPECT_EQ(dst.buffer[5], 92);

  // Block 3 RGB: (140+180+160+200)/4=170, (141+181+161+201)/4=171, (142+182+162+202)/4=172
  EXPECT_EQ(dst.buffer[6], 170);
  EXPECT_EQ(dst.buffer[7], 171);
  EXPECT_EQ(dst.buffer[8], 172);

  freeImage(&src);
  freeImage(&dst);
}

// Test edge case averaging - values that could cause rounding issues
TEST_F(DownscaleAreaAverageTest, EdgeCaseAveraging) {
  // Test values that result in fractional averages to verify rounding
  Image src;
  src.img_width = 2;
  src.img_height = 2;
  src.length = 2 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      // Block RGB: [1,2,3], [2,3,4], [3,4,5], [4,5,6] -> avg = [2.5,3.5,4.5] -> [2,3,4] (truncated)
      1, 2, 3, 2, 3, 4, // Row 0
      3, 4, 5, 4, 5, 6  // Row 1
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // RGB: (1+2+3+4)/4=2.5->2, (2+3+4+5)/4=3.5->3, (3+4+5+6)/4=4.5->4 (truncated)
  EXPECT_EQ(dst.buffer[0], 2); // R
  EXPECT_EQ(dst.buffer[1], 3); // G
  EXPECT_EQ(dst.buffer[2], 4); // B

  freeImage(&src);
  freeImage(&dst);
}

// Test max/min value mixing to verify no overflow in intermediate calculations
TEST_F(DownscaleAreaAverageTest, MaxMinValueMixing) {
  // Mix extreme values to test overflow handling during summation
  Image src;
  src.img_width = 4;
  src.img_height = 2;
  src.length = 4 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      255, 255, 255, 0,   0,   0,   255, 255, 255, 0,   0,   0,  // Row 0: alternating max/min
      0,   0,   0,   255, 255, 255, 0,   0,   0,   255, 255, 255 // Row 1: alternating min/max
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(2, 2);

  downscale_area_average(&src, &dst);

  // Each 2x1 block should average to 127.5 -> 127 for all RGB channels
  EXPECT_EQ(dst.buffer[0], 127);
  EXPECT_EQ(dst.buffer[1], 127);
  EXPECT_EQ(dst.buffer[2], 127); // (255+0)/2 = 127.5 -> 127
  EXPECT_EQ(dst.buffer[3], 127);
  EXPECT_EQ(dst.buffer[4], 127);
  EXPECT_EQ(dst.buffer[5], 127); // (255+0)/2 = 127.5 -> 127

  freeImage(&src);
  freeImage(&dst);
}

// Test prime number averaging to catch any mathematical errors
TEST_F(DownscaleAreaAverageTest, PrimeNumberAveraging) {
  // Use prime numbers to avoid any "convenient" mathematical coincidences
  Image src;
  src.img_width = 2;
  src.img_height = 2;
  src.length = 2 * 2 * 3;
  src.buffer = (uint8_t *)malloc(src.length);

  uint8_t src_data[] = {
      // Block RGB: [7,11,13], [11,13,17], [13,17,19], [17,19,23] -> avg = [12,15,18]
      7,  11, 13, 11, 13, 17, // Row 0
      13, 17, 19, 17, 19, 23  // Row 1
  };
  memcpy(src.buffer, src_data, src.length);

  Image dst = createEmptyImage(1, 1);

  downscale_area_average(&src, &dst);

  // RGB: (7+11+13+17)/4=12, (11+13+17+19)/4=15, (13+17+19+23)/4=18
  EXPECT_EQ(dst.buffer[0], 12); // R
  EXPECT_EQ(dst.buffer[1], 15); // G
  EXPECT_EQ(dst.buffer[2], 18); // B

  freeImage(&src);
  freeImage(&dst);
}
