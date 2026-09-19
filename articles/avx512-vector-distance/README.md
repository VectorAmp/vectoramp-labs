# Two AVX-512 techniques for vector distance

This lab accompanies a draft VectorAmp Engineering article about two distinct
uses of SIMD in vector search:

1. dense squared-L2 distance using packed subtraction and fused multiply-add;
2. product-quantization asymmetric distance using byte widening and indexed gathers.

The code is an independently written educational implementation. It is **not**
the production SABLE implementation and does not reproduce SABLE's index,
graph, layouts, scheduling, search budgets, or tuning parameters.

## Mathematical definitions

For dense vectors, the squared-L2 distance is:

```text
d²(q, x) = Σᵢ (qᵢ - xᵢ)²
```

For a product-quantized candidate, asymmetric distance is:

```text
d_ADC(q, x̂) = Σₘ LUTₘ[codeₘ(x)]
```

This lab assumes 8-bit PQ codes, so each subquantizer table has 256 `f32`
entries. The layout is deliberately straightforward: consecutive table rows
with one row per subquantizer.

## Requirements

- x86-64 Linux
- CMake 3.20+
- GCC or Clang with AVX2 and AVX-512 intrinsic support
- an AVX2 or AVX-512 CPU to execute the corresponding kernels

The binary performs runtime feature checks. Unsupported optimized variants are
skipped by the benchmark and safely fall back to scalar behavior through the
public API.

## Build and test

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Run the benchmark

```bash
./scripts/run-benchmarks.sh
```

Pass an optional iteration count for a shorter or longer run:

```bash
./scripts/run-benchmarks.sh 5000000
```

The script records:

- raw CSV measurements under `results/`;
- CPU, operating-system, compiler, and CMake metadata;
- correctness test results before measurement.

## Interpreting results

The benchmark reports per-distance latency and throughput for scalar, AVX2,
and AVX-512 implementations across several vector dimensions and PQ sizes.

Treat these as kernel microbenchmarks, not end-to-end database claims. Results
depend on CPU generation, frequency policy, cache residency, compiler version,
virtualization, thermal state, and surrounding workload. AVX-512 gather does
not remove cache-miss latency; it is most effective when the distance tables
remain cache-resident.

## Inspect generated assembly

```bash
objdump -d -M intel --demangle build/libvector_distance.a | less
```

Useful instructions to locate include `vsubps`, `vfmadd231ps`, `vpmovzxbd`,
`vpaddd`, and `vgatherdps`.

