#!/usr/bin/env bash
set -euo pipefail

lab_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_dir="${lab_dir}/build"
results_dir="${lab_dir}/results"

cmake -S "${lab_dir}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_dir}" --parallel
ctest --test-dir "${build_dir}" --output-on-failure

mkdir -p "${results_dir}"
timestamp="$(date -u +%Y%m%dT%H%M%SZ)"
result_file="${results_dir}/benchmark-${timestamp}.csv"
metadata_file="${results_dir}/environment-${timestamp}.txt"

{
  uname -a
  lscpu
  c++ --version
  cmake --version
} > "${metadata_file}"

"${build_dir}/kernel_bench" "${1:-1000000}" | tee "${result_file}"

echo "Results: ${result_file}"
echo "Environment: ${metadata_file}"

