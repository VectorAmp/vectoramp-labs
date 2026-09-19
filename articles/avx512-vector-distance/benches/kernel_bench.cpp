#include "vectoramp_labs/kernels.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string_view>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

template <typename Operation>
double measure_ns_per_operation(Operation&& operation, const std::size_t iterations) {
  volatile float checksum = 0.0F;
  for (std::size_t i = 0; i < iterations / 20 + 1; ++i) {
    checksum = checksum + operation(i);
  }

  const auto started = Clock::now();
  for (std::size_t i = 0; i < iterations; ++i) {
    checksum = checksum + operation(i);
  }
  const auto elapsed = Clock::now() - started;

  if (checksum == -1.0F) {
    std::cerr << "unreachable checksum: " << checksum << '\n';
  }
  return std::chrono::duration<double, std::nano>(elapsed).count() /
         static_cast<double>(iterations);
}

void emit_row(
    const std::string_view kernel,
    const vectoramp::labs::Isa isa,
    const std::size_t problem_size,
    const std::size_t iterations,
    const double nanoseconds) {
  std::cout << kernel << ',' << vectoramp::labs::isa_name(isa) << ','
            << problem_size << ',' << iterations << ',' << std::fixed
            << std::setprecision(3) << nanoseconds << ','
            << std::setprecision(2) << (1.0e9 / nanoseconds) << '\n';
}

}  // namespace

int main(int argc, char** argv) {
  using vectoramp::labs::Isa;

  const std::size_t iterations = argc > 1
      ? static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10))
      : 1'000'000;

  std::mt19937 generator(42);
  std::uniform_real_distribution<float> floats(-1.0F, 1.0F);
  std::uniform_int_distribution<int> bytes(0, 255);

  std::cout << "kernel,isa,problem_size,iterations,ns_per_operation,operations_per_second\n";

  for (const std::size_t dimensions : {256U, 768U, 1536U}) {
    constexpr std::size_t corpus_size = 1024;
    std::vector<float> query(dimensions);
    std::vector<float> corpus(corpus_size * dimensions);
    for (auto& value : query) {
      value = floats(generator);
    }
    for (auto& value : corpus) {
      value = floats(generator);
    }

    for (const Isa isa : {Isa::scalar, Isa::avx2, Isa::avx512}) {
      if (!vectoramp::labs::isa_supported(isa)) {
        continue;
      }
      const double ns = measure_ns_per_operation(
          [&](const std::size_t i) {
            const float* candidate = corpus.data() +
                (i % corpus_size) * dimensions;
            return vectoramp::labs::l2_squared(
                query.data(), candidate, dimensions, isa);
          },
          iterations);
      emit_row("l2_squared", isa, dimensions, iterations, ns);
    }
  }

  for (const std::size_t subquantizers : {16U, 32U, 64U}) {
    constexpr std::size_t corpus_size = 8192;
    std::vector<std::uint8_t> codes(corpus_size * subquantizers);
    std::vector<float> tables(subquantizers * 256);
    for (auto& code : codes) {
      code = static_cast<std::uint8_t>(bytes(generator));
    }
    for (auto& value : tables) {
      value = std::abs(floats(generator));
    }

    for (const Isa isa : {Isa::scalar, Isa::avx2, Isa::avx512}) {
      if (!vectoramp::labs::isa_supported(isa)) {
        continue;
      }
      const double ns = measure_ns_per_operation(
          [&](const std::size_t i) {
            const std::uint8_t* candidate = codes.data() +
                (i % corpus_size) * subquantizers;
            return vectoramp::labs::pq_adc(
                candidate, tables.data(), subquantizers, isa);
          },
          iterations);
      emit_row("pq_adc", isa, subquantizers, iterations, ns);
    }
  }
  return 0;
}

