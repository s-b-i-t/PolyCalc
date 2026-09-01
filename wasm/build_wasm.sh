#!/usr/bin/env bash

set -euo pipefail

script_directory="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_directory}/.." && pwd)"
output_directory="${project_root}/web/generated"

if ! command -v em++ >/dev/null 2>&1; then
    echo "em++ is not on PATH. Run: source ~/emsdk/emsdk_env.sh" >&2
    exit 1
fi

mkdir -p "${output_directory}"

em++ \
    -std=c++17 \
    -O2 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -Werror \
    -I"${project_root}/cpp_code" \
    -I"${project_root}/wasm" \
    "${project_root}/cpp_code/Polynomial.cpp" \
    "${project_root}/cpp_code/Complex.cpp" \
    "${project_root}/cpp_code/Rational.cpp" \
    "${project_root}/wasm/wasm_api.cpp" \
    --no-entry \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -sENVIRONMENT=web \
    -sFILESYSTEM=0 \
    -sEXPORTED_FUNCTIONS='["_polycalc_add","_polycalc_subtract","_polycalc_multiply","_polycalc_divmod","_polycalc_complex_add","_polycalc_complex_subtract","_polycalc_complex_multiply","_polycalc_complex_divmod","_polycalc_rational_add","_polycalc_rational_subtract","_polycalc_rational_multiply","_polycalc_rational_divide","_malloc","_free"]' \
    -sEXPORTED_RUNTIME_METHODS='["HEAPF64"]' \
    -o "${output_directory}/polycalc.mjs"

chmod 0644 \
    "${output_directory}/polycalc.mjs" \
    "${output_directory}/polycalc.wasm"
