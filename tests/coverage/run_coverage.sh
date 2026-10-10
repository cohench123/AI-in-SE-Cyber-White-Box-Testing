#!/usr/bin/env bash
# Measure code coverage for SimCity using GCC's gcov.
#
# 1. Builds an instrumented CLI binary (with sanitizers) and an instrumented
#    unit-test binary, each in its own directory under build/cov/.
# 2. Runs the unit tests and the black-box security suite.
# 3. Merges the gcov data and writes build/cov/report.txt.
#
# Usage (from anywhere):   tests/coverage/run_coverage.sh
# Or:                      make coverage
#
# Coverage is only written when a process exits normally. Crashing, aborting, or
# timed-out runs record no data, so error-path coverage is undercounted.

set -u
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
OUT="$REPO/build/cov"
FL="-std=c++17 -O0 -g --coverage"

rm -rf "$OUT"
mkdir -p "$OUT/cli" "$OUT/unit"

# Project modules: every .cpp in the repo root. main.cpp is excluded from the unit binary.
MODULES=()
for f in "$REPO"/*.cpp; do MODULES+=("$(basename "$f" .cpp)"); done

echo "== Building instrumented CLI binary"
(
    cd "$OUT/cli" || exit 1
    for m in "${MODULES[@]}"; do
        g++ $FL -fsanitize=address,undefined -c "$REPO/$m.cpp" -o "$m.o" || exit 1
    done
    g++ $FL -fsanitize=address,undefined ./*.o -o SimCity_cov
) || { echo "CLI build failed"; exit 2; }

echo "== Building instrumented unit-test binary"
(
    cd "$OUT/unit" || exit 1
    for m in "${MODULES[@]}"; do
        [ "$m" = "main" ] && continue
        g++ $FL -c "$REPO/$m.cpp" -o "$m.o" || exit 1
    done
    g++ $FL -c "$REPO/tests/test_main.cpp" -o test_main.o || exit 1
    g++ $FL -c "$REPO/tests/test_config_region.cpp" -o test_config_region.o || exit 1
    g++ $FL ./*.o -o unit_tests
) || { echo "unit-test build failed"; exit 2; }

echo "== Running unit tests"
(cd "$OUT/unit" && ./unit_tests > "$OUT/unit.log" 2>&1)
tail -1 "$OUT/unit.log"

echo "== Running security suite (this takes a few minutes)"
(cd "$REPO" && SIMCITY_BIN="$OUT/cli/SimCity_cov" tests/security/run_security_tests.sh > "$OUT/security.log" 2>&1)
tail -1 "$OUT/security.log"

echo "== Merging coverage"
python3 "$REPO/tests/coverage/merge_gcov.py" "$REPO" "$OUT/cli" "$OUT/unit" | tee "$OUT/report.txt"

echo
echo "Report written to $OUT/report.txt"
