#include "vectoramp_labs/kernels.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <immintrin.h>

namespace vectoramp::labs {
namespace {

#if defined(__GNUC__) || defined(__clang__)
#define VA_TARGET(features) __attribute__((target(features)))
#else
#define VA_TARGET(features)
#endif

VA_TARGET("avx2,fma")
float l2_squared_avx2(
    const float* lhs,
    const float* rhs,
    std::size_t dimensions) noexcept {
  __m256 accumulator = _mm256_setzero_ps();
  std::size_t i = 0;
  for (; i + 8 <= dimensions; i += 8) {
    const __m256 a = _mm256_loadu_ps(lhs + i);
    const __m256 b = _mm256_loadu_ps(rhs + i);
    const __m256 delta = _mm256_sub_ps(a, b);
    accumulator = _mm256_fmadd_ps(delta, delta, accumulator);
  }

  alignas(32) std::array<float, 8> lanes{};
  _mm256_store_ps(lanes.data(), accumulator);
  float sum = 0.0F;
  for (const float lane : lanes) {
    sum += lane;
  }
  for (; i < dimensions; ++i) {
    const float delta = lhs[i] - rhs[i];
    sum += delta * delta;
  }
  return sum;
}

VA_TARGET("avx512f,fma")
float l2_squared_avx512(
    const float* lhs,
    const float* rhs,
    std::size_t dimensions) noexcept {
  __m512 accumulator = _mm512_setzero_ps();
  std::size_t i = 0;
  for (; i + 16 <= dimensions; i += 16) {
    const __m512 a = _mm512_loadu_ps(lhs + i);
    const __m512 b = _mm512_loadu_ps(rhs + i);
    const __m512 delta = _mm512_sub_ps(a, b);
    accumulator = _mm512_fmadd_ps(delta, delta, accumulator);
  }

  float sum = _mm512_reduce_add_ps(accumulator);
  for (; i < dimensions; ++i) {
    const float delta = lhs[i] - rhs[i];
    sum += delta * delta;
  }
  return sum;
}

VA_TARGET("avx2")
float pq_adc_avx2(
    const std::uint8_t* codes,
    const float* distance_tables,
    std::size_t subquantizers) noexcept {
  __m256 accumulator = _mm256_setzero_ps();
  const __m256i row_stride = _mm256_setr_epi32(
      0 * 256, 1 * 256, 2 * 256, 3 * 256,
      4 * 256, 5 * 256, 6 * 256, 7 * 256);

  std::size_t m = 0;
  for (; m + 8 <= subquantizers; m += 8) {
    std::uint64_t packed_codes = 0;
    std::memcpy(&packed_codes, codes + m, sizeof(packed_codes));
    const __m128i bytes = _mm_cvtsi64_si128(static_cast<long long>(packed_codes));
    const __m256i code_indices = _mm256_cvtepu8_epi32(bytes);
    const __m256i row_base = _mm256_add_epi32(
        row_stride,
        _mm256_set1_epi32(static_cast<int>(m * 256)));
    const __m256i indices = _mm256_add_epi32(row_base, code_indices);
    const __m256 values = _mm256_i32gather_ps(distance_tables, indices, 4);
    accumulator = _mm256_add_ps(accumulator, values);
  }

  alignas(32) std::array<float, 8> lanes{};
  _mm256_store_ps(lanes.data(), accumulator);
  float sum = 0.0F;
  for (const float lane : lanes) {
    sum += lane;
  }
  for (; m < subquantizers; ++m) {
    sum += distance_tables[m * 256 + codes[m]];
  }
  return sum;
}

VA_TARGET("avx512f,avx512bw")
float pq_adc_avx512(
    const std::uint8_t* codes,
    const float* distance_tables,
    std::size_t subquantizers) noexcept {
  const __m512i row_stride = _mm512_setr_epi32(
      0 * 256, 1 * 256, 2 * 256, 3 * 256,
      4 * 256, 5 * 256, 6 * 256, 7 * 256,
      8 * 256, 9 * 256, 10 * 256, 11 * 256,
      12 * 256, 13 * 256, 14 * 256, 15 * 256);
  __m512 accumulator = _mm512_setzero_ps();

  std::size_t m = 0;
  for (; m + 16 <= subquantizers; m += 16) {
    const __m128i bytes = _mm_loadu_si128(
        reinterpret_cast<const __m128i*>(codes + m));
    const __m512i code_indices = _mm512_cvtepu8_epi32(bytes);
    const __m512i row_base = _mm512_add_epi32(
        row_stride,
        _mm512_set1_epi32(static_cast<int>(m * 256)));
    const __m512i indices = _mm512_add_epi32(row_base, code_indices);
    const __m512 values = _mm512_i32gather_ps(indices, distance_tables, 4);
    accumulator = _mm512_add_ps(accumulator, values);
  }

  float sum = _mm512_reduce_add_ps(accumulator);
  for (; m < subquantizers; ++m) {
    sum += distance_tables[m * 256 + codes[m]];
  }
  return sum;
}

}  // namespace

bool isa_supported(const Isa isa) noexcept {
#if defined(__x86_64__) || defined(_M_X64)
  switch (isa) {
    case Isa::scalar:
      return true;
    case Isa::avx2:
      return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
    case Isa::avx512:
      return __builtin_cpu_supports("avx512f") &&
             __builtin_cpu_supports("avx512bw");
  }
#endif
  return isa == Isa::scalar;
}

std::string_view isa_name(const Isa isa) noexcept {
  switch (isa) {
    case Isa::scalar:
      return "scalar";
    case Isa::avx2:
      return "avx2";
    case Isa::avx512:
      return "avx512";
  }
  return "unknown";
}

float l2_squared_scalar(
    const float* lhs,
    const float* rhs,
    const std::size_t dimensions) noexcept {
  float sum = 0.0F;
  for (std::size_t i = 0; i < dimensions; ++i) {
    const float delta = lhs[i] - rhs[i];
    sum += delta * delta;
  }
  return sum;
}

float l2_squared(
    const float* lhs,
    const float* rhs,
    const std::size_t dimensions,
    const Isa isa) noexcept {
  if (!isa_supported(isa)) {
    return l2_squared_scalar(lhs, rhs, dimensions);
  }
  switch (isa) {
    case Isa::scalar:
      return l2_squared_scalar(lhs, rhs, dimensions);
    case Isa::avx2:
      return l2_squared_avx2(lhs, rhs, dimensions);
    case Isa::avx512:
      return l2_squared_avx512(lhs, rhs, dimensions);
  }
  return l2_squared_scalar(lhs, rhs, dimensions);
}

float pq_adc_scalar(
    const std::uint8_t* codes,
    const float* distance_tables,
    const std::size_t subquantizers) noexcept {
  float sum = 0.0F;
  for (std::size_t m = 0; m < subquantizers; ++m) {
    sum += distance_tables[m * 256 + codes[m]];
  }
  return sum;
}

float pq_adc(
    const std::uint8_t* codes,
    const float* distance_tables,
    const std::size_t subquantizers,
    const Isa isa) noexcept {
  if (!isa_supported(isa)) {
    return pq_adc_scalar(codes, distance_tables, subquantizers);
  }
  switch (isa) {
    case Isa::scalar:
      return pq_adc_scalar(codes, distance_tables, subquantizers);
    case Isa::avx2:
      return pq_adc_avx2(codes, distance_tables, subquantizers);
    case Isa::avx512:
      return pq_adc_avx512(codes, distance_tables, subquantizers);
  }
  return pq_adc_scalar(codes, distance_tables, subquantizers);
}

}  // namespace vectoramp::labs

