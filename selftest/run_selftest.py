# Copyright 2026 CrucibleC
# SPDX-License-Identifier: Apache-2.0

"""run_selftest.py - tests HammerC itself: hammerc.py and hammerc.h.

Usage: python selftest/run_selftest.py [--cc gcc|clang|cl] [--update]

There are two kinds of tests. Each one produces text that is compared with
an expected-output file:

1. script_cases/<case>/ holds .test.c files for hammerc.py to read. The
   script's exit code and output, and the tests it put in the table, must
   match <case>/expected.txt.

2. checks/ and custom_print/ are test suites written with HammerC. Each is
   generated with hammerc.py, compiled, and run with the arguments listed in
   SUITES below. Each run must match <suite>/expected/<run>.txt.

It also checks that hammerc.h, hammerc.py and the README title give the same
version number.

Before comparing, the runner's own path is replaced with <runner> and every
backslash with '/', so the same expected files work on Linux and Windows.

--update writes the actual output into the expected files instead of
comparing. Read the diff (git diff selftest) before committing it.
"""

import argparse
import difflib
import glob
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
HAMMERC_PY = os.path.join(ROOT, "hammerc.py")
BUILD_DIR = os.path.join(HERE, "build")
EXE = ".exe" if os.name == "nt" else ""

# One row of the generated test table: { "name", "file", name },
TABLE_ROW = re.compile(r'^    \{ "(\w+)", "(.*)", \w+ \},$', re.MULTILINE)

SUITES = [
    {
        "name": "checks",
        # Generated twice: the second run must report both files as unchanged.
        "generate": [[], []],
        "main": "checks/generated/hammer_main.c",
        "defines": [],
        "runs": [
            ("all", []),
            ("verbose_one_test", ["--verbose", "test_check_passes"]),
            ("verbose_passed_int_values", ["--verbose", "test_eq_int_passes"]),
            ("verbose_passed_str_values", ["-v", "test_eq_str_passes"]),
            ("verbose_failed_values_once", ["-v", "test_eq_int_fails_prints_values"]),
            ("quiet", ["--quiet"]),
            ("quiet_one_passing_test", ["-q", "test_check_passes"]),
            ("silent", ["--silent"]),
            ("silent_one_passing_test", ["-s", "test_check_passes"]),
            ("last_level_wins", ["--silent", "--quiet", "test_check_fails_and_continues"]),
            ("one_failing_test", ["test_check_fails_and_continues"]),
            ("unknown_test", ["test_does_not_exist"]),
            ("two_test_names", ["test_check_passes", "test_no_checks"]),
            ("unknown_flag", ["--bogus"]),
            ("help", ["--help"]),
        ],
    },
    {
        "name": "custom_print",
        # Without --nomain first, so the --nomain run must remove that hammer_main.c.
        "generate": [[], ["--nomain"]],
        "main": "custom_print/main.c",
        "defines": ["HAMMER_LINE_MAX=64"],
        "runs": [
            ("all", []),
        ],
    },
]


def run(command):
    """Run command in the selftest folder. Returns its exit code and output (stdout, then stderr)."""
    result = subprocess.run(command, cwd=HERE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            universal_newlines=True)
    return f"exit: {result.returncode}\n{result.stdout}{result.stderr}"


def normalize(text, runner=None):
    """Make output the same on every platform: <runner> for the runner's path, '/' for '\\'."""
    if runner:
        text = text.replace(runner, "<runner>")
    return text.replace("\\", "/")


def compare(expected_path, actual, update):
    """Compare actual with the expected file (or write it with --update). Returns True if they match."""
    shown = os.path.relpath(expected_path, ROOT).replace("\\", "/")
    if update:
        with open(expected_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(actual)
        print(f"updated  {shown}")
        return True

    expected = ""
    if os.path.exists(expected_path):
        with open(expected_path, encoding="utf-8") as f:
            expected = f.read()
    if actual == expected:
        print(f"ok       {shown}")
        return True
    print(f"FAILED   {shown}")
    diff = difflib.unified_diff(expected.splitlines(True), actual.splitlines(True),
                                "expected", "actual")
    sys.stdout.writelines("         " + line for line in diff)
    print()
    return False


def compile_command(cc, sources, defines, exe):
    """The command line that compiles sources into exe, for gcc/clang or for MSVC's cl."""
    if os.path.basename(cc).lower() in ("cl", "cl.exe"):
        return ([cc, "/nologo", "/std:c11", "/W4", "/WX", "/I" + ROOT]
                + ["/D" + define for define in defines]
                + sources + ["/Fe" + exe, "/Fo" + BUILD_DIR + os.sep])
    return ([cc, "-std=c99", "-Wall", "-Wextra", "-pedantic", "-Werror", "-I" + ROOT]
            + ["-D" + define for define in defines]
            + sources + ["-o", exe])


def test_script_case(case, update):
    """Run hammerc.py on script_cases/<case> and compare with its expected.txt."""
    folder = f"script_cases/{case}"
    shutil.rmtree(os.path.join(HERE, folder, "generated"), ignore_errors=True)
    actual = run([sys.executable, HAMMERC_PY, folder])

    header = os.path.join(HERE, folder, "generated", "hammer_tests.h")
    if os.path.exists(header):
        with open(header, encoding="utf-8") as f:
            for name, file in TABLE_ROW.findall(f.read()):
                actual += f"registered: {name} ({file})\n"
    return compare(os.path.join(HERE, folder, "expected.txt"), normalize(actual), update)


def test_suite(suite, cc, update):
    """Generate, compile and run one test suite, comparing each step with its expected file."""
    name = suite["name"]
    expected_dir = os.path.join(HERE, name, "expected")
    os.makedirs(expected_dir, exist_ok=True)
    shutil.rmtree(os.path.join(HERE, name, "generated"), ignore_errors=True)

    generate = "".join(run([sys.executable, HAMMERC_PY, name] + args) for args in suite["generate"])
    passed = compare(os.path.join(expected_dir, "generate.txt"), normalize(generate), update)

    # Relative paths with '/', so __FILE__ (and every AT: line) is the same on every platform.
    tests = glob.glob(os.path.join(HERE, name, "**", "*.test.c"), recursive=True)
    sources = sorted(os.path.relpath(path, HERE).replace("\\", "/") for path in tests)
    sources.append(suite["main"])
    os.makedirs(BUILD_DIR, exist_ok=True)
    exe = os.path.join(BUILD_DIR, name + EXE)
    command = compile_command(cc, sources, suite["defines"], exe)
    print("$ " + " ".join(command), flush=True)  # before the compiler's own output
    if subprocess.run(command, cwd=HERE).returncode != 0:
        print(f"FAILED   {name}: compiling failed")
        return False

    for run_name, args in suite["runs"]:
        actual = normalize(run([exe] + args), runner=exe)
        passed &= compare(os.path.join(expected_dir, run_name + ".txt"), actual, update)
    return passed


def test_versions_match():
    """The version in hammerc.h, hammerc.py and the README title must be the same."""
    patterns = {
        "hammerc.h": r'#define HAMMERC_VERSION "([^"]+)"',
        "hammerc.py": r'^VERSION = "([^"]+)"',
        "README.md": r"^# HammerC v(\S+)",
    }
    versions = {}
    for file, pattern in patterns.items():
        with open(os.path.join(ROOT, file), encoding="utf-8") as f:
            match = re.search(pattern, f.read(), re.MULTILINE)
        versions[file] = match.group(1) if match else "(not found)"
    if len(set(versions.values())) == 1:
        print(f"ok       versions match ({versions['hammerc.h']})")
        return True
    print("FAILED   versions differ: " + ", ".join(f"{file} {v}" for file, v in versions.items()))
    return False


def main():
    parser = argparse.ArgumentParser(description="Test HammerC itself.")
    parser.add_argument("--cc", default="gcc", help="C compiler: gcc, clang or cl (default: gcc)")
    parser.add_argument("--update", action="store_true",
                        help="write the actual output into the expected files instead of comparing")
    args = parser.parse_args()

    failed = []
    if not test_versions_match():
        failed.append("versions")
    for case in sorted(os.listdir(os.path.join(HERE, "script_cases"))):
        if not test_script_case(case, args.update):
            failed.append(f"script_cases/{case}")
    for suite in SUITES:
        if not test_suite(suite, args.cc, args.update):
            failed.append(suite["name"])

    print()
    if failed:
        print(f"selftest FAILED: {', '.join(failed)}")
        return 1
    print("selftest passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
