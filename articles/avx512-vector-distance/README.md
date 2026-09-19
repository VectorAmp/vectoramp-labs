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

On Linux, pass a logical CPU as the second argument to pin the process:

```bash
./scripts/run-benchmarks.sh 5000000 3
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

## Reference Intel run

The publication reference run used an AWS `r6id.2xlarge` backed by an Intel
Xeon Platinum 8375C (Ice Lake). The process was pinned to logical CPU 3, the
correctness suite passed on the host, and every case was measured ten times at
3,000,000 operations per run.

Selected median results:

| Kernel | Problem size | Scalar | AVX2 | AVX-512 | AVX-512 vs scalar |
| --- | ---: | ---: | ---: | ---: | ---: |
| Squared L2 | 1,536 dimensions | 669.547 ns | 176.892 ns | 217.820 ns | 3.07x |
| PQ ADC | 64 subquantizers | 49.759 ns | 19.803 ns | 19.073 ns | 2.61x |

The raw repeats, aggregate statistics, host metadata, binary hashes, and
disassembly evidence are under [`results/`](results/). These are kernel
microbenchmarks on a shared development node, not end-to-end database results.
The dense implementations use four independent accumulators. On this host,
AVX2 beat AVX-512 for tuned dense L2, while AVX-512 remained slightly faster
for PQ ADC. Wider is a workload-dependent choice, not an automatic win.

## Inspect generated assembly

```bash
objdump -d -M intel --demangle build/libvector_distance.a | less
```

Useful instructions to locate include `vsubps`, `vfmadd231ps`, `vpmovzxbd`,
`vpaddd`, and `vgatherdps`.
