# Benchmark results

## Publication reference run

The current reference result is the tuned Intel Ice Lake run captured on 2026-09-19:

- `benchmark-intel-xeon-8375c-20260919-tuned-runs.csv`: all 180 timed cases from ten repetitions;
- `benchmark-intel-xeon-8375c-20260919-tuned-summary.csv`: median, minimum, maximum, mean, standard deviation, and median throughput;
- `environment-intel-xeon-8375c-20260919-tuned.txt`: host, CPU flags, topology, toolchain, binary hashes, and disassembly evidence.

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
- Dense L2 accumulation: four independent accumulators in scalar, AVX2, and AVX-512
- Scalar vectorization: explicitly disabled for the scalar reference function

The benchmark skips unsupported instruction-set variants. AVX-512 rows were
emitted only after runtime detection confirmed `avx512f` and `avx512bw`.
Correctness tests for scalar, AVX2, and AVX-512 passed on the same host before
measurement. Captured disassembly includes ZMM packed FMA and ZMM gather
instructions.

### Selected medians

| Kernel | Size | Scalar | AVX2 | AVX-512 | AVX-512 vs scalar |
| --- | ---: | ---: | ---: | ---: | ---: |
| Squared L2 | 256 dimensions | 113.609 ns | 21.038 ns | 30.958 ns | 3.67x |
| Squared L2 | 768 dimensions | 335.018 ns | 88.241 ns | 108.308 ns | 3.09x |
| Squared L2 | 1,536 dimensions | 669.547 ns | 176.892 ns | 217.820 ns | 3.07x |
| PQ ADC | 16 subquantizers | 10.613 ns | 7.155 ns | 5.481 ns | 1.94x |
| PQ ADC | 32 subquantizers | 20.731 ns | 10.901 ns | 10.056 ns | 2.06x |
| PQ ADC | 64 subquantizers | 49.759 ns | 19.803 ns | 19.073 ns | 2.61x |

### Limitations

This was a process-pinned run on a shared Kubernetes development node. The node
was nearly idle immediately before measurement, and run-to-run variance was
low, but sibling CPU 7 was not isolated from the scheduler. The container did
not expose a `perf` binary, so this result does not include hardware performance
counters. AVX2 beat AVX-512 in the tuned dense-L2 cases on this host, while
AVX-512 retained a small advantage for PQ ADC. Treat the numbers as
reproducible kernel measurements, not a claim about complete VectorAmp query
latency or a universal ranking of vector widths.

## Earlier exploratory run

The untuned Intel files without `-tuned` preserve the original single-
accumulator run. The timestamped `benchmark-20260919T182813Z.csv` and matching
environment file preserve the exploratory AMD EPYC VM run. Neither is the
publication reference result.
