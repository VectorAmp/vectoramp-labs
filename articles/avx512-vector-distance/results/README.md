# Benchmark results

## Publication reference run

The current reference result is the Intel Ice Lake run captured on 2026-09-19:

- `benchmark-intel-xeon-8375c-20260919-runs.csv`: all 180 timed cases from ten repetitions;
- `benchmark-intel-xeon-8375c-20260919-summary.csv`: median, minimum, maximum, mean, standard deviation, and median throughput;
- `environment-intel-xeon-8375c-20260919.txt`: host, CPU flags, topology, toolchain, binary hashes, and disassembly evidence.

### Method

- AWS account: VectorAmp development
- Instance type: `r6id.2xlarge`
- CPU: Intel Xeon Platinum 8375C, Ice Lake, 2.90 GHz base
- Logical CPUs: 8
- Benchmark affinity: logical CPU 3
- Sibling logical CPU: 7
- Repetitions: 10
- Operations per case per repetition: 3,000,000
- Working set: deterministic synthetic data, warmed before timing
- Compiler: GCC 11.4.0, `-O3`, C++20
- Benchmark commit: `210e632479664bdabaa9397d548a2955b990c19b`

The benchmark skips unsupported instruction-set variants. AVX-512 rows were
emitted only after runtime detection confirmed `avx512f` and `avx512bw`.
Correctness tests for scalar, AVX2, and AVX-512 passed on the same host before
measurement. Captured disassembly includes ZMM packed FMA and ZMM gather
instructions.

### Selected medians

| Kernel | Size | Scalar | AVX2 | AVX-512 | AVX-512 vs scalar |
| --- | ---: | ---: | ---: | ---: | ---: |
| Squared L2 | 256 dimensions | 244.475 ns | 30.262 ns | 29.755 ns | 8.22x |
| Squared L2 | 768 dimensions | 854.179 ns | 105.429 ns | 104.569 ns | 8.17x |
| Squared L2 | 1,536 dimensions | 1,766.655 ns | 213.609 ns | 206.214 ns | 8.57x |
| PQ ADC | 16 subquantizers | 9.640 ns | 7.138 ns | 5.505 ns | 1.75x |
| PQ ADC | 32 subquantizers | 20.974 ns | 10.976 ns | 10.070 ns | 2.08x |
| PQ ADC | 64 subquantizers | 49.922 ns | 19.823 ns | 19.109 ns | 2.61x |

### Limitations

This was a process-pinned run on a shared Kubernetes development node. The node
was nearly idle immediately before measurement, and run-to-run variance was
low, but sibling CPU 7 was not isolated from the scheduler. The container did
not expose a `perf` binary, so this result does not include hardware performance
counters. Treat the numbers as reproducible kernel measurements, not a claim
about complete VectorAmp query latency.

## Earlier exploratory run

The timestamped `benchmark-20260919T182813Z.csv` and matching environment file
are retained as an exploratory AMD EPYC VM run. They are not the publication
reference result.
