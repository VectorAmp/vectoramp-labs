#include "vectoramp_labs/kernels.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <vector>

namespace {

bool approximately_equal(const float lhs, const float rhs) {
  const float scale = std::max({1.0F, std::abs(lhs), std::abs(rhs)});
  return std::abs(lhs - rhs) <= 2.0e-5F * scale;
}

int fail(const char* message) {
  std::cerr << "FAILED: " << message << '\n';
  return 1;
}

}  // namespace

int main() {
  using vectoramp::labs::Isa;

  std::mt19937 generator(42);
  std::uniform_real_distribution<float> floats(-1.0F, 1.0F);
  std::uniform_int_distribution<int> bytes(0, 255);

  for (const std::size_t dimensions : {1U, 7U, 16U, 31U, 256U, 769U, 1536U}) {
    std::vector<float> lhs(dimensions);
    std::vector<float> rhs(dimensions);
    for (std::size_t i = 0; i < dimensions; ++i) {
      lhs[i] = floats(generator);
      rhs[i] = floats(generator);
    }
    const float expected = vectoramp::labs::l2_squared_scalar(
        lhs.data(), rhs.data(), dimensions);
    for (const Isa isa : {Isa::scalar, Isa::avx2, Isa::avx512}) {
      const float actual = vectoramp::labs::l2_squared(
          lhs.data(), rhs.data(), dimensions, isa);
      if (!approximately_equal(expected, actual)) {
        return fail("L2 kernel diverged from scalar reference");
      }
    }
  }

  for (const std::size_t subquantizers : {1U, 7U, 16U, 23U, 32U, 64U}) {
    std::vector<std::uint8_t> codes(subquantizers);
    std::vector<float> tables(subquantizers * 256);
    for (auto& code : codes) {
      code = static_cast<std::uint8_t>(bytes(generator));
    }
    for (auto& value : tables) {
      value = std::abs(floats(generator));
    }
    const float expected = vectoramp::labs::pq_adc_scalar(
        codes.data(), tables.data(), subquantizers);
    for (const Isa isa : {Isa::scalar, Isa::avx2, Isa::avx512}) {
      const float actual = vectoramp::labs::pq_adc(
          codes.data(), tables.data(), subquantizers, isa);
      if (!approximately_equal(expected, actual)) {
        return fail("PQ ADC kernel diverged from scalar reference");
      }
    }
  }

  std::cout << "All scalar, AVX2, and AVX-512 correctness checks passed.\n";
  return 0;
}

