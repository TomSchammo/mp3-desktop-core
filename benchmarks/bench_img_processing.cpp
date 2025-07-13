#include "img_processing.hpp"
#include <benchmark/benchmark.h>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

// Platform-specific CPU affinity includes
#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <pthread.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#elif defined(__linux__)
#include <sched.h>
#include <unistd.h>
#endif

#include <thread>

// Check if optimizations are enabled
static void PrintBuildInfo() {
  std::cout << "Build Configuration: ";
#ifdef NDEBUG
  std::cout << "Release (Optimized)" << std::endl;
#else
  std::cout << "Debug (Unoptimized)" << std::endl;
#endif

#ifdef __OPTIMIZE__
  std::cout << "Compiler optimizations: Enabled" << std::endl;
#else
  std::cout << "Compiler optimizations: Disabled" << std::endl;
#endif

#ifdef __clang__
  std::cout << "Compiler: Clang " << __clang_major__ << "." << __clang_minor__ << std::endl;
#elif defined(__GNUC__)
  std::cout << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ << std::endl;
#endif
  std::cout << std::endl;
}

class BenchmarkFixture : public benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State &state) override {
    // Initialize random number generator for consistent results
    std::mt19937 gen(42); // Fixed seed for reproducible results
    std::uniform_int_distribution<> dis(0, 255);

    // Pre-generate test data for different sizes
    small_image = createRandomImage(64, 64, 3, gen, dis);
    medium_image = createRandomImage(256, 256, 3, gen, dis);
    large_image = createRandomImage(1024, 1024, 3, gen, dis);
    xlarge_image = createRandomImage(2048, 2048, 3, gen, dis);

    // Pre-allocate output buffers
    small_output.resize(32 * 32 * 3);
    medium_output.resize(128 * 128 * 3);
    large_output.resize(512 * 512 * 3);
    xlarge_output.resize(1024 * 1024 * 3);
  }

private:
  std::vector<uint8_t> createRandomImage(uint32_t width, uint32_t height, int32_t channels,
                                         std::mt19937 &gen, std::uniform_int_distribution<> &dis) {
    std::vector<uint8_t> image(width * height * channels);
    for (auto &pixel : image) {
      pixel = static_cast<uint8_t>(dis(gen));
    }
    return image;
  }

protected:
  std::vector<uint8_t> small_image;  // 64x64x3
  std::vector<uint8_t> medium_image; // 256x256x3
  std::vector<uint8_t> large_image;  // 1024x1024x3
  std::vector<uint8_t> xlarge_image; // 2048x2048x3

  std::vector<uint8_t> small_output;  // 32x32x3
  std::vector<uint8_t> medium_output; // 128x128x3
  std::vector<uint8_t> large_output;  // 512x512x3
  std::vector<uint8_t> xlarge_output; // 1024x1024x3
};

// Small image benchmarks (64x64 -> 32x32)
BENCHMARK_F(BenchmarkFixture, SmallImage_2x_Downscale)(benchmark::State &state) {
  for (auto _ : state) {
    downscale_area_average(small_image.data(), 64, 64, small_output.data(), 32, 32, 3);
  }
  state.SetItemsProcessed(state.iterations() * 64 * 64 * 3);
  state.SetBytesProcessed(state.iterations() * 64 * 64 * 3 * sizeof(uint8_t));
}

// Medium image benchmarks (256x256 -> 128x128)
BENCHMARK_F(BenchmarkFixture, MediumImage_2x_Downscale)(benchmark::State &state) {
  for (auto _ : state) {
    downscale_area_average(medium_image.data(), 256, 256, medium_output.data(), 128, 128, 3);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
  state.SetBytesProcessed(state.iterations() * 256 * 256 * 3 * sizeof(uint8_t));
}

// Large image benchmarks (1024x1024 -> 512x512)
BENCHMARK_F(BenchmarkFixture, LargeImage_2x_Downscale)(benchmark::State &state) {
  for (auto _ : state) {
    downscale_area_average(large_image.data(), 1024, 1024, large_output.data(), 512, 512, 3);
  }
  state.SetItemsProcessed(state.iterations() * 1024 * 1024 * 3);
  state.SetBytesProcessed(state.iterations() * 1024 * 1024 * 3 * sizeof(uint8_t));
}

// Extra large image benchmarks (2048x2048 -> 1024x1024)
BENCHMARK_F(BenchmarkFixture, XLargeImage_2x_Downscale)(benchmark::State &state) {
  for (auto _ : state) {
    downscale_area_average(xlarge_image.data(), 2048, 2048, xlarge_output.data(), 1024, 1024, 3);
  }
  state.SetItemsProcessed(state.iterations() * 2048 * 2048 * 3);
  state.SetBytesProcessed(state.iterations() * 2048 * 2048 * 3 * sizeof(uint8_t));
}

// Different scaling factors
BENCHMARK_F(BenchmarkFixture, MediumImage_4x_Downscale)(benchmark::State &state) {
  std::vector<uint8_t> output(64 * 64 * 3);
  for (auto _ : state) {
    downscale_area_average(medium_image.data(), 256, 256, output.data(), 64, 64, 3);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
}

BENCHMARK_F(BenchmarkFixture, MediumImage_8x_Downscale)(benchmark::State &state) {
  std::vector<uint8_t> output(32 * 32 * 3);
  for (auto _ : state) {
    downscale_area_average(medium_image.data(), 256, 256, output.data(), 32, 32, 3);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
}

// Different channel counts
BENCHMARK_F(BenchmarkFixture, MediumImage_SingleChannel)(benchmark::State &state) {
  std::vector<uint8_t> src_1ch(256 * 256 * 1);
  std::vector<uint8_t> dst_1ch(128 * 128 * 1);

  // Fill with test data
  std::iota(src_1ch.begin(), src_1ch.end(), 0);

  for (auto _ : state) {
    downscale_area_average(src_1ch.data(), 256, 256, dst_1ch.data(), 128, 128, 1);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 1);
}

BENCHMARK_F(BenchmarkFixture, MediumImage_FourChannels)(benchmark::State &state) {
  std::vector<uint8_t> src_4ch(256 * 256 * 4);
  std::vector<uint8_t> dst_4ch(128 * 128 * 4);

  // Fill with test data
  std::iota(src_4ch.begin(), src_4ch.end(), 0);

  for (auto _ : state) {
    downscale_area_average(src_4ch.data(), 256, 256, dst_4ch.data(), 128, 128, 4);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 4);
}

// Non-power-of-2 scaling
BENCHMARK_F(BenchmarkFixture, MediumImage_NonPowerOf2_Scaling)(benchmark::State &state) {
  std::vector<uint8_t> output(170 * 170 * 3); // 256->170 is approximately 1.5x downscale
  for (auto _ : state) {
    downscale_area_average(medium_image.data(), 256, 256, output.data(), 170, 170, 3);
  }
  state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
}

// Rectangular images
BENCHMARK_F(BenchmarkFixture, RectangularImage_Downscale)(benchmark::State &state) {
  std::vector<uint8_t> src_rect(512 * 256 * 3); // 512x256 (2:1 aspect ratio)
  std::vector<uint8_t> dst_rect(256 * 128 * 3); // 256x128 (2:1 aspect ratio)

  // Fill with test data
  std::iota(src_rect.begin(), src_rect.end(), 0);

  for (auto _ : state) {
    downscale_area_average(src_rect.data(), 512, 256, dst_rect.data(), 256, 128, 3);
  }
  state.SetItemsProcessed(state.iterations() * 512 * 256 * 3);
}

// Extreme downscaling (stress test)
BENCHMARK_F(BenchmarkFixture, ExtremeDownscale_1024_to_8)(benchmark::State &state) {
  std::vector<uint8_t> output(8 * 8 * 3);
  for (auto _ : state) {
    downscale_area_average(large_image.data(), 1024, 1024, output.data(), 8, 8, 3);
  }
  state.SetItemsProcessed(state.iterations() * 1024 * 1024 * 3);
}

// Memory access pattern benchmark (cache performance)
BENCHMARK_F(BenchmarkFixture, CachePerformance_Sequential)(benchmark::State &state) {
  // This benchmark tests how well the algorithm performs with different cache access patterns
  // by running multiple downscale operations in sequence
  std::vector<uint8_t> output1(128 * 128 * 3);
  std::vector<uint8_t> output2(128 * 128 * 3);
  std::vector<uint8_t> output3(128 * 128 * 3);

  for (auto _ : state) {
    downscale_area_average(medium_image.data(), 256, 256, output1.data(), 128, 128, 3);
    downscale_area_average(medium_image.data(), 256, 256, output2.data(), 128, 128, 3);
    downscale_area_average(medium_image.data(), 256, 256, output3.data(), 128, 128, 3);
  }
  state.SetItemsProcessed(state.iterations() * 3 * 256 * 256 * 3);
}

// Forward implementation benchmarks
// BENCHMARK_F(BenchmarkFixture, SmallImage_2x_Downscale_Forward)(benchmark::State &state) {
//   for (auto _ : state) {
//     downscale_area_average_forward(small_image.data(), 64, 64, small_output.data(), 32, 32, 3);
//   }
//   state.SetItemsProcessed(state.iterations() * 64 * 64 * 3);
//   state.SetBytesProcessed(state.iterations() * 64 * 64 * 3 * sizeof(uint8_t));
// }
//
// BENCHMARK_F(BenchmarkFixture, MediumImage_2x_Downscale_Forward)(benchmark::State &state) {
//   for (auto _ : state) {
//     downscale_area_average_forward(medium_image.data(), 256, 256, medium_output.data(), 128, 128,
//     3);
//   }
//   state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
//   state.SetBytesProcessed(state.iterations() * 256 * 256 * 3 * sizeof(uint8_t));
// }
//
// BENCHMARK_F(BenchmarkFixture, LargeImage_2x_Downscale_Forward)(benchmark::State &state) {
//   for (auto _ : state) {
//     downscale_area_average_forward(large_image.data(), 1024, 1024, large_output.data(), 512, 512,
//     3);
//   }
//   state.SetItemsProcessed(state.iterations() * 1024 * 1024 * 3);
//   state.SetBytesProcessed(state.iterations() * 1024 * 1024 * 3 * sizeof(uint8_t));
// }
//
// BENCHMARK_F(BenchmarkFixture, XLargeImage_2x_Downscale_Forward)(benchmark::State &state) {
//   for (auto _ : state) {
//     downscale_area_average_forward(xlarge_image.data(), 2048, 2048, xlarge_output.data(), 1024,
//     1024, 3);
//   }
//   state.SetItemsProcessed(state.iterations() * 2048 * 2048 * 3);
//   state.SetBytesProcessed(state.iterations() * 2048 * 2048 * 3 * sizeof(uint8_t));
// }
//
// BENCHMARK_F(BenchmarkFixture, MediumImage_4x_Downscale_Forward)(benchmark::State &state) {
//   std::vector<uint8_t> output(64 * 64 * 3);
//   for (auto _ : state) {
//     downscale_area_average_forward(medium_image.data(), 256, 256, output.data(), 64, 64, 3);
//   }
//   state.SetItemsProcessed(state.iterations() * 256 * 256 * 3);
// }
//
// BENCHMARK_F(BenchmarkFixture, MediumImage_SingleChannel_Forward)(benchmark::State &state) {
//   std::vector<uint8_t> src_1ch(256 * 256 * 1);
//   std::vector<uint8_t> dst_1ch(128 * 128 * 1);
//
//   // Fill with test data
//   std::iota(src_1ch.begin(), src_1ch.end(), 0);
//
//   for (auto _ : state) {
//     downscale_area_average_forward(src_1ch.data(), 256, 256, dst_1ch.data(), 128, 128, 1);
//   }
//   state.SetItemsProcessed(state.iterations() * 256 * 256 * 1);
// }

// Function to set thread affinity and priority on new thread
void set_thread_affinity_and_priority() {
#ifdef __APPLE__
  thread_port_t mach_thread = pthread_mach_thread_np(pthread_self());
  kern_return_t result;

  // Set high priority to avoid preemption
  thread_precedence_policy_data_t precedence_policy = {63}; // Max priority
  result = thread_policy_set(mach_thread, THREAD_PRECEDENCE_POLICY,
                             (thread_policy_t)&precedence_policy, THREAD_PRECEDENCE_POLICY_COUNT);

  if (result == KERN_SUCCESS) {
    std::cout << "Successfully set precedence" << std::endl;
  } else {
    std::cout << "Warning: Could not set precedence (error: " << result << ")" << std::endl;
  }

  // Disable interruptions for timing-critical sections
  thread_extended_policy_data_t extended_policy = {1}; // timeshare = false
  result = thread_policy_set(mach_thread, THREAD_EXTENDED_POLICY, (thread_policy_t)&extended_policy,
                             THREAD_EXTENDED_POLICY_COUNT);

  if (result == KERN_SUCCESS) {
    std::cout << "Successfully disabled timeshare" << std::endl;
  } else {
    std::cout << "Warning: Could not disable timeshare (error: " << result << ")" << std::endl;
  }
#elif defined(__linux__)
  // Linux-specific CPU affinity (for future reference)
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);

  if (sched_setaffinity(0, sizeof(cpuset), &cpuset) == 0) {
    std::cout << "Successfully set CPU affinity" << std::endl;
  } else {
    std::cout << "Warning: Could not set CPU affinity" << std::endl;
  }
#else
  std::cout << "CPU affinity not implemented for this platform" << std::endl;
#endif
}

// Function to run benchmarks in dedicated thread
void run_benchmarks_in_thread() {
  set_thread_affinity_and_priority();
  ::benchmark::RunSpecifiedBenchmarks();
}

// Register benchmark main with build info
int main(int argc, char **argv) {
  PrintBuildInfo();
  ::benchmark::Initialize(&argc, argv);

  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;

  std::cout << "Setting CPU affinity and priority in dedicated thread..." << std::endl;

  // Run benchmarks in a new thread where we can set affinity from the start
  std::thread benchmark_thread(run_benchmarks_in_thread);
  benchmark_thread.join();

  std::cout << std::endl;

  ::benchmark::Shutdown();
  return 0;
}
