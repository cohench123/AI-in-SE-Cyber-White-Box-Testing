#!/usr/bin/env python3
"""Merge gcov coverage from one or more instrumented build directories.

Usage:
    merge_gcov.py REPO_ROOT OBJDIR [OBJDIR ...]

Each OBJDIR holds the .gcno/.gcda files of one instrumented build (compiled with
--coverage). Results are restricted to the project's own .cpp files in REPO_ROOT,
so standard library headers and test code are not counted.

Definitions:
  lines     - statement coverage, as gcov reports it ("Lines executed").
  branches  - a branch is covered when it was "taken at least once". This is NOT
              gcov's "Branches executed", which counts branches on lines that ran.
  functions - a function is covered when it was entered at least once.

A line, branch, or function counts as covered if ANY OBJDIR covered it, so the
union of several test runs is reported as well as each run on its own.

Requires gcov (GCC) and Python 3. Does not need gcovr.
"""
import glob
import gzip
import json
import os
import subprocess
import sys
import tempfile


def project_sources(repo):
    return sorted(os.path.basename(p) for p in glob.glob(os.path.join(repo, "*.cpp")))


def collect(repo, objdir):
    """Return {'lines': {...}, 'branches': {...}, 'functions': {...}} for one build dir."""
    sources = project_sources(repo)
    keep = set(sources)
    lines, branches, functions = {}, {}, {}

    with tempfile.TemporaryDirectory() as tmp:
        for src in sources:
            stem = src[: -len(".cpp")]
            if not os.path.exists(os.path.join(objdir, stem + ".gcda")):
                continue  # module never ran in this build (e.g. main.cpp in the unit binary)
            subprocess.run(
                ["gcov", "-j", "-b", "-o", objdir, os.path.join(repo, src)],
                cwd=tmp, capture_output=True,
            )
            # gcov names its JSON after the source stem: config.cpp -> config.gcov.json.gz
            report = os.path.join(tmp, stem + ".gcov.json.gz")
            if not os.path.exists(report):
                continue
            with gzip.open(report) as fh:
                data = json.load(fh)
            os.remove(report)

            for fe in data["files"]:
                fname = os.path.basename(fe["file"])
                if fname not in keep:
                    continue
                for ln in fe["lines"]:
                    k = (fname, ln["line_number"])
                    lines[k] = max(lines.get(k, 0), ln["count"])
                    fn = ln.get("function_name", "")
                    for i, br in enumerate(ln.get("branches", [])):
                        kb = (fname, ln["line_number"], fn, i)
                        branches[kb] = max(branches.get(kb, 0), br["count"])
                for fn in fe.get("functions", []):
                    kf = (fname, fn["name"], fn["start_line"])
                    functions[kf] = max(functions.get(kf, 0), fn["execution_count"])

    return {"lines": lines, "branches": branches, "functions": functions}


def merge(parts):
    out = {"lines": {}, "branches": {}, "functions": {}}
    for part in parts:
        for kind in out:
            for k, v in part[kind].items():
                out[kind][k] = max(out[kind].get(k, 0), v)
    return out


def totals(data):
    result = {}
    for kind in ("lines", "branches", "functions"):
        total = len(data[kind])
        covered = sum(1 for v in data[kind].values() if v > 0)
        result[kind] = (covered, total)
    return result


def print_totals(label, data):
    print(f"== {label}")
    for kind, (c, t) in totals(data).items():
        pct = 100.0 * c / t if t else 0.0
        print(f"  {kind:10s} {c:5d}/{t:<5d} {pct:6.2f}%")


def print_per_file(data):
    per = {}
    for kind in ("lines", "branches", "functions"):
        for k, v in data[kind].items():
            f = k[0]
            entry = per.setdefault(f, {kd: [0, 0] for kd in ("lines", "branches", "functions")})
            entry[kind][1] += 1
            if v > 0:
                entry[kind][0] += 1
    print(f"  {'file':18s} {'lines':>12s} {'branches':>14s} {'functions':>12s}")
    for f in sorted(per):
        e = per[f]
        print(f"  {f:18s} {e['lines'][0]:5d}/{e['lines'][1]:<5d}"
              f" {e['branches'][0]:6d}/{e['branches'][1]:<6d}"
              f" {e['functions'][0]:5d}/{e['functions'][1]:<5d}")


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    repo = os.path.abspath(argv[1])
    objdirs = [os.path.abspath(d) for d in argv[2:]]

    parts = {}
    for d in objdirs:
        parts[d] = collect(repo, d)

    if all(not any(parts[d][k] for k in parts[d]) for d in objdirs):
        print("error: no gcov data found in any OBJDIR (were the programs run?)",
              file=sys.stderr)
        return 1

    for d in objdirs:
        print_totals(f"{os.path.basename(d)} only", parts[d])
    union = merge(parts.values())
    print_totals("union (all runs)", union)
    print()
    print("Per file (union of all runs):")
    print_per_file(union)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
