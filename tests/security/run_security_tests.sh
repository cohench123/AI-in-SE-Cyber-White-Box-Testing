#!/usr/bin/env bash
# Security-focused black-box tests for the SimCity binary.
#   IV = input validation, FS = file / upload security, DP = data protection.
#
# The program is built with AddressSanitizer and UBSan so out-of-bounds access
# and undefined behavior are reported. Each case runs in a fresh directory with
# a 10 second timeout. A FAIL is a finding about the program, not a harness error.
#
# Usage:
#   tests/security/run_security_tests.sh               # all cases
#   FILTER=IV-0 tests/security/run_security_tests.sh  # only IV-01 .. IV-09

set -u
REPO="$(cd "$(dirname "$0")/../.." && pwd)"
WORK="$(mktemp -d)"
FILTER="${FILTER:-}"
trap 'rm -rf "$WORK"' EXIT

# SIMCITY_BIN lets a caller supply a prebuilt binary (e.g. an instrumented one for coverage).
if [ -n "${SIMCITY_BIN:-}" ]; then
    BIN="$SIMCITY_BIN"
else
    BIN="$WORK/SimCity"
    # -D_GLIBCXX_SANITIZE_VECTOR makes ASan flag reads and writes into the spare
    # capacity of std::vector. Without it, an out-of-bounds index that lands in
    # that spare capacity goes unreported (see tests/REVIEW.md, IV-12).
    g++ -std=c++17 -g -fsanitize=address,undefined -D_GLIBCXX_SANITIZE_VECTOR \
        "$REPO"/main.cpp "$REPO"/commercial.cpp "$REPO"/config.cpp "$REPO"/growth.cpp \
        "$REPO"/industrial.cpp "$REPO"/region.cpp "$REPO"/residential.cpp "$REPO"/goods.cpp \
        -o "$BIN" 2>"$WORK/build.log" || { cat "$WORK/build.log"; echo "build failed"; exit 2; }
fi

# ---------------------------------------------------------------------------
# Fixtures. Each runs inside a fresh directory and writes config.txt / region.csv.
# ---------------------------------------------------------------------------
STD_REGION='R,R,I,C\nT,#,R,C\nR,-,R,I\n'
LONG_NAME="$(head -c 5000 /dev/zero | tr '\0' a)"

std_config()      { printf 'Region Layout:region.csv\nTime Limit:3\nRefresh Rate:1\n' > config.txt; }
s_std()           { std_config; printf '%b' "$STD_REGION" > region.csv; }
s_crlf_config()   { printf 'Region Layout:region.csv\r\nTime Limit:3\r\nRefresh Rate:1\r\n' > config.txt; printf '%b' "$STD_REGION" > region.csv; }
s_refresh0()      { printf 'Region Layout:region.csv\nTime Limit:3\nRefresh Rate:0\n' > config.txt; printf '%b' "$STD_REGION" > region.csv; }
s_time_abc()      { printf 'Region Layout:region.csv\nTime Limit:abc\nRefresh Rate:1\n' > config.txt; printf '%b' "$STD_REGION" > region.csv; }
s_time_huge()     { printf 'Region Layout:region.csv\nTime Limit:99999999999\nRefresh Rate:1\n' > config.txt; printf '%b' "$STD_REGION" > region.csv; }
s_missing_region(){ printf 'Region Layout:nope.csv\nTime Limit:3\nRefresh Rate:1\n' > config.txt; }
s_empty_region()  { std_config; : > region.csv; }
s_ragged()        { std_config; printf 'R,I,C\nT\n' > region.csv; }
s_nul_region()    { std_config; printf 'R\0I,C\nR,I,C\n' > region.csv; }
s_big_region()    { std_config; awk 'BEGIN{for(i=0;i<300;i++){s=""; for(j=0;j<300;j++) s=s (j?",":"") "R"; print s}}' > region.csv; }
s_big_config()    { std_config; printf '%b' "$STD_REGION" > region.csv; head -c 1000000 /dev/zero | tr '\0' x >> config.txt; printf '\n' >> config.txt; }
s_traversal()     { printf 'Region Layout:../../../../etc/hostname\nTime Limit:3\nRefresh Rate:1\n' > config.txt; }
s_secret()        { std_config; printf 'Secret,Ibeta\nR,C\n' > region.csv; }

# ---------------------------------------------------------------------------
# Runner: run_case ID CATEGORY DESCRIPTION EXPECT_REGEX SETUP INPUT
# Optional env FORBID=regex: output matching it counts as a data exposure.
# ---------------------------------------------------------------------------
PASS=0
FAIL=0
# POLICY cases are design decisions, not defect checks. They are counted apart
# from the defect results so a policy gap does not look like a crash.
POL_PASS=0
POL_FAIL=0

run_case() {
    local id="$1" cat="$2" desc="$3" expect="$4" setup="$5" input="$6"
    if [ -n "$FILTER" ] && [[ "$id" != "$FILTER"* ]]; then return; fi

    local d out code size reason=""
    d="$(mktemp -d -p "$WORK")"
    ( cd "$d" && "$setup" )

    # Output goes to a file capped at ~10 MB so runaway output cannot exhaust memory.
    ( cd "$d" && ulimit -f 10240 && printf '%b' "$input" | timeout 10 "$BIN" > "$d/out.txt" 2>&1 )
    code=$?
    size="$(stat -c%s "$d/out.txt" 2>/dev/null || echo 0)"
    out="$(head -c 1000000 "$d/out.txt" 2>/dev/null)"

    if   [ "$size" -ge 9000000 ]; then reason="unbounded output (>9 MB, likely an infinite loop)"
    elif [ "$code" -eq 124 ]; then reason="hang: no termination within 10s"
    elif [ "$code" -ge 128 ]; then reason="killed by signal $((code - 128))"
    elif printf '%s' "$out" | grep -Eq 'AddressSanitizer|runtime error|terminate called'; then
        reason="memory error or uncaught exception"
    elif [ -n "$expect" ] && ! printf '%s' "$out" | grep -Eq "$expect"; then
        reason="expected output missing: $expect"
    elif [ -n "${FORBID:-}" ] && printf '%s' "$out" | grep -Eq "$FORBID"; then
        reason="forbidden content exposed: $FORBID"
    fi

    if [ "$cat" = POLICY ]; then
        if [ -z "$reason" ]; then
            POL_PASS=$((POL_PASS + 1)); echo "[PASS] $id [$cat] $desc"
        else
            POL_FAIL=$((POL_FAIL + 1)); echo "[FAIL] $id [$cat] $desc -- $reason"
        fi
    elif [ -z "$reason" ]; then
        PASS=$((PASS + 1)); echo "[PASS] $id [$cat] $desc"
    else
        FAIL=$((FAIL + 1)); echo "[FAIL] $id [$cat] $desc -- $reason"
    fi
}

# ---------------------------------------------------------------------------
# Input validation (IV)
# ---------------------------------------------------------------------------
run_case IV-01 IV "Invalid config filename rejected, then valid one accepted" \
    "Invalid filename" s_std "missing.txt\nconfig.txt\nEasy\n10\n"
run_case IV-02 IV "Invalid difficulty rejected, then valid one accepted" \
    "Invalid difficulty selection" s_std "config.txt\nInsane\nEasy\n10\n"
run_case IV-03 IV "Difficulty is case-sensitive" \
    "Invalid difficulty selection" s_std "config.txt\neasy\nEasy\n10\n"
run_case IV-04 IV "Non-numeric menu input terminates instead of looping" \
    "Invalid menu choice" s_std "config.txt\nEasy\nabc\n"
run_case IV-05 IV "Out-of-range menu choice (99) rejected" \
    "Invalid menu choice" s_std "config.txt\nEasy\n99\n10\n"
run_case IV-06 IV "Negative menu choice rejected" \
    "Invalid menu choice" s_std "config.txt\nEasy\n-1\n10\n"
run_case IV-07 IV "Add demand with index -1 rejected" \
    "Invalid" s_std "config.txt\nEasy\n5\n1\n-1\n10\n"
run_case IV-08 IV "Add demand with index 20 (one past end) rejected" \
    "Invalid" s_std "config.txt\nEasy\n5\n1\n20\n10\n"
run_case IV-09 IV "Add demand with index 19 (upper boundary) accepted" \
    "Added 1 demand level to Fabrics" s_std "config.txt\nEasy\n5\n1\n19\n10\n"
run_case IV-10 IV "Add demand with index 0 (lower boundary) accepted" \
    "Added 1 demand level to Toys" s_std "config.txt\nEasy\n5\n1\n0\n10\n"
run_case IV-11 IV "Remove demand with index -1 rejected" \
    "Invalid" s_std "config.txt\nEasy\n5\n2\n-1\n10\n"
run_case IV-12 IV "Remove demand with index 20 rejected" \
    "Invalid" s_std "config.txt\nEasy\n5\n2\n20\n10\n"
run_case IV-13 IV "Add/remove selector outside 1-2 rejected" \
    "Invalid choice to add or remove" s_std "config.txt\nEasy\n5\n3\n10\n"
run_case IV-14 IV "Remove demand from good with zero demand reports invalid good" \
    "Invalid good" s_std "config.txt\nEasy\n5\n2\n0\n10\n"
run_case IV-15 IV "Search with reversed corners rejected" \
    "Invalid input" s_std "config.txt\nEasy\n3\n1 1\n0 0\n10\n"
run_case IV-16 IV "Search with coordinates past region edge rejected" \
    "Invalid input" s_std "config.txt\nEasy\n3\n0 0\n99 99\n10\n"
run_case IV-17 IV "Search with negative coordinate rejected" \
    "Invalid input" s_std "config.txt\nEasy\n3\n-1 0\n0 0\n10\n"
run_case IV-18 IV "Search with non-numeric coordinates terminates" \
    "Invalid" s_std "config.txt\nEasy\n3\nx y\n10\n"
# The prompt text has no newline before the first output row, so the expectation
# matches the row's text ("R R ") anywhere on a line, not at line start.
run_case IV-19 IV "Search with valid corner (0,0)-(1,1) prints the top row of the block" \
    "R R $" s_std "config.txt\nEasy\n3\n0 0\n1 1\n10\n"
run_case IV-20 IV "Oversized (5000 char) config filename rejected" \
    "Invalid filename" s_std "$LONG_NAME\nconfig.txt\nEasy\n10\n"
run_case IV-21 IV "Refresh Rate 0 rejected or handled (no SIGFPE)" \
    "Invalid|[Ee]rror" s_refresh0 "config.txt\nEasy\n10\n"
run_case IV-22 IV "Non-numeric Time Limit rejected (no abort)" \
    "Invalid|[Ee]rror" s_time_abc "config.txt\nEasy\n10\n"
run_case IV-23 IV "Out-of-range Time Limit rejected (no abort)" \
    "Invalid|[Ee]rror" s_time_huge "config.txt\nEasy\n10\n"
run_case IV-24 IV "Config with CRLF line endings runs to completion" \
    "Ending Simulation|Final timestep" s_crlf_config "config.txt\nEasy\n10\n"
run_case IV-25 IV "Shell metacharacters in config filename rejected (not executed)" \
    "Invalid filename" s_std "touch_me;id\nconfig.txt\nEasy\n10\n"

# ---------------------------------------------------------------------------
# File and upload security (FS)
# ---------------------------------------------------------------------------
run_case FS-01 FS "Directory given as config filename rejected" \
    "Invalid filename" s_std ".\nconfig.txt\nEasy\n10\n"
run_case FS-02 FS "Missing region file reported as error" \
    "Invalid|[Ee]rror|not found" s_missing_region "config.txt\nEasy\n10\n"
run_case FS-03 FS "Empty region file reported as error" \
    "Invalid|[Ee]rror" s_empty_region "config.txt\nEasy\n10\n"
run_case FS-04 FS "Ragged region rows rejected or handled (no out-of-bounds)" \
    "Invalid|[Ee]rror|rectang" s_ragged "config.txt\nEasy\n10\n"
run_case FS-05 FS "Region file containing NUL bytes handled" \
    "Ending Simulation|Final timestep" s_nul_region "config.txt\nEasy\n10\n"
run_case FS-06 FS "Large (300x300) region file completes" \
    "Ending Simulation|Final timestep" s_big_region "config.txt\nEasy\n10\n"
run_case FS-07 FS "Config with 1 MB non-key line completes" \
    "Ending Simulation|Final timestep" s_big_config "config.txt\nEasy\n10\n"

# ---------------------------------------------------------------------------
# Data protection (DP)
# ---------------------------------------------------------------------------
FORBID='Secret|beta' run_case DP-01 DP "Region cells display only their first character (format check)" \
    "S I " s_secret "config.txt\nEasy\n10\n"
FORBID='SECRETPATH' run_case DP-02 DP "Invalid filename error does not echo the entered path" \
    "Invalid filename" s_std "SECRETPATH_123/nope.txt\nconfig.txt\nEasy\n10\n"
FORBID='AddressSanitizer|/home/|\.cpp:' run_case DP-03 DP "Normal error output reveals no build or source paths" \
    "Invalid menu choice" s_std "config.txt\nEasy\n99\n10\n"

run_case POL-01 POLICY "Region path outside the config directory is refused" \
    "outside|not allowed|[Ii]nvalid" s_traversal "config.txt\nEasy\n10\n"

echo
echo "$PASS passed, $FAIL failed (defect checks)"
echo "$POL_PASS passed, $POL_FAIL failed (policy checks, not counted above)"
exit $((FAIL > 0))
