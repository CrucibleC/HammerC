[Part of The Crucible C Project.](https://github.com/CrucibleC/CrucibleC)

# HammerC V1.0.0

A tiny test system for C. It has one header and one Python script, and every part of it is meant to be read.

- `hammerc.h` holds the types, the check macros and the runner. All functions are `static inline`, so there is nothing to link.
- `hammerc.py` finds your tests and generates the table that runs them, plus an optional `main`.

The generated code contains only declarations and a table. All the logic is in `hammerc.h`, in ordinary functions you can read and step through in a debugger.

## Quick start

```
python hammerc.py example
gcc -std=c99 -Wall -Wextra -I. example/*.test.c example/eq/*.test.c example/calc.c example/generated/hammer_main.c -o example_tests
./example_tests
./example_tests --verbose
```

The `-I.` puts the folder containing `hammerc.h` on the include path. The example has four tests that fail on purpose, so you can see what failures look like. `--verbose` also prints every check that passes.

## Writing tests

Put tests in files ending in `.test.c`:

```c
#include "hammerc.h"
#include "calc.h"

static int is_between(int value, int low, int high)   /* helper: static */
{
    return value >= low && value <= high;
}

void test_clamp_stays_in_range(HammerCtx *t)          /* test */
{
    for (int value = -10; value <= 10; value++) {
        CHECK(is_between(calc_clamp(value, -5, 5), -5, 5));
    }
}
```

The rules, which `hammerc.py` enforces:

- **A test** is a non-static function whose whole signature is on one line, in exactly the form `void test_<name>(HammerCtx *t)`. The opening `{` can be on the same line or the next.
- **A helper** must be `static`.
- **Test names** must be unique across all files, because tests are global functions.

The parameter must be named `t`, because the check macros use it.

## Checks

| Macro | On failure |
|---|---|
| `CHECK(expr)` | records the failure and continues |
| `REQUIRE(expr)` | records the failure and returns from the current function |
| `CHECK_EQ_INT(actual, expected)` | compares as `long long`, prints both values, and continues |
| `REQUIRE_EQ_INT(actual, expected)` | compares as `long long`, prints both values, and returns |
| `CHECK_EQ_UINT(actual, expected)` | compares as `unsigned long long`, prints both values, and continues |
| `REQUIRE_EQ_UINT(actual, expected)` | compares as `unsigned long long`, prints both values, and returns |
| `CHECK_EQ_STR(actual, expected)` | compares with `strcmp`, prints both strings, and continues |
| `REQUIRE_EQ_STR(actual, expected)` | compares with `strcmp`, prints both strings, and returns |
| `CHECK_EQ_PTR(actual, expected)` | compares as `const void *`, prints both pointer values, and continues |
| `REQUIRE_EQ_PTR(actual, expected)` | compares as `const void *`, prints both pointer values, and returns |

Each macro only captures `__FILE__`, `__LINE__` and the check's source text, then calls one `hammer_check*` function. For example:

```c
#define CHECK(expr) \
    do { hammer_check(t, (expr) ? 1 : 0, "CHECK(" #expr ")", __FILE__, __LINE__); } while (0)
```

`"CHECK(" #expr ")"` is joined into one string at compile time, so `CHECK(a == b)` passes the text `"CHECK(a == b)"`. The `*_EQ_*` macros do the same with both arguments.

Things to know:

- **`REQUIRE` returns from the function it is written in.** Inside a helper, it only leaves the helper and the test keeps running. In helpers, use `CHECK` or return a result the test can `REQUIRE`.
- **`CHECK` and `REQUIRE` print the expression text, not its values.** The `*_EQ_*` macros also print both values.
- **The `*_EQ_*` macros convert both arguments to one type before comparing.** Pick the macro that matches your values. A negative number passed to `*_EQ_UINT` becomes a huge unsigned value, for example.
- **`*_EQ_STR` handles NULL.** Two NULLs are equal, and NULL never equals a string. A failure prints strings in quotes and NULL as `NULL`.
- **`*_EQ_PTR` work with object pointers only.** Function pointers don't portably convert to `void *`.

## Output

```
RUN  test_add_small_numbers (calc.test.c)
     2/2 checks passed

RUN  test_add_fails_on_purpose (calc.test.c)
     FAILED:   CHECK(calc_add(2, 2) == 5)
     AT:       example/calc.test.c:27
     1/2 checks passed

RUN  test_string_fails_on_purpose (eq/strings.test.c)
     FAILED:   CHECK_EQ_STR(file_extension("photo.jpeg"), "jpg")
     AT:       example/eq/strings.test.c:39
     EXPECTED: "jpg"
     ACTUAL:   "jpeg"
     FAILED:   CHECK_EQ_STR(file_extension("Makefile"), "mk")
     AT:       example/eq/strings.test.c:40
     EXPECTED: "mk"
     ACTUAL:   NULL
     0/2 checks passed

RUN  test_require_stops_on_purpose (pointers.test.c)
     FAILED:   REQUIRE_EQ_PTR(strchr(text, 'x'), &text[0])
     AT:       example/pointers.test.c:25
     EXPECTED: 00007FF7DCEE54CE
     ACTUAL:   0000000000000000
     0/1 checks passed

TESTS:   14
SUCCESS: 10
FAILED:  4

CHECKS:  46
SUCCESS: 41
FAILED:  5
```

- **Passing checks print nothing,** unless the runner is started with `--verbose` (see below).
- **Each failure is printed under its test's `RUN` line.** `FAILED:` shows the check as written in the source, and `AT:` shows `file:line`, which editors and terminals make clickable. The file is the path the compiler was given.
- **The `*_EQ_*` checks also print `EXPECTED:` and `ACTUAL:`** with both values.
- **A `RUN` line is printed before each test,** and the default print function flushes after every line. If a test crashes, the last `RUN` line names it.
- **After each test, a line shows how many of its checks passed,** followed by a blank line. Checks skipped because a `REQUIRE` stopped the test are not counted.
- **The summary comes last,** with test and check totals. The runner's exit code is 0 if every test passed and 1 otherwise.

### Showing passing checks

```
./example_tests --verbose
```

`--verbose` (or `-v`) also prints every check that passes, with the same `AT:` line as a failure:

```
RUN  test_add_small_numbers (calc.test.c)
     PASSED:   CHECK(calc_add(2, 2) == 4)
     AT:       example/calc.test.c:12
     PASSED:   CHECK(calc_add(-3, 3) == 0)
     AT:       example/calc.test.c:13
     2/2 checks passed
```

`--help` (or `-h`) prints the usage. Any other argument prints an error and the runner exits with 2. The command line is read by `hammer_main` in `hammerc.h`, which the generated `main` calls.

## How a run works

`HammerCtx` is created once for the whole run:

```c
typedef struct {
    HammerPrintFn print;
    int           show_passed; /* also print checks that pass (--verbose) */
    const char   *test_name;   /* test currently running */
    const char   *test_file;
    HammerCounts  test;        /* checks and failures for the current test */
    HammerCounts  run;         /* checks and failures for all finished tests */
    int           tests_run;
    int           tests_failed;
} HammerCtx;
```

`hammer_run` goes through the generated table. For each test it sets the name and file, zeroes `test`, prints the `RUN` line and calls the test. Afterwards it prints how many of the test's checks passed, adds `test` into `run` and updates `tests_run` and `tests_failed`. The check functions only touch `test`, so `hammer_run` is the one place where the totals change.

## Printing

```c
typedef void (*HammerPrintFn)(const char *line);
```

The print function is called once per complete line, and the line ends in `\n`. HammerC formats every line itself, with `vsnprintf` into a buffer of `HAMMER_LINE_MAX` (512) bytes. To send output somewhere else, such as a UART, a log file or a buffer, write a function with this signature and pass it to `run_all_tests_with`. Its second argument is `show_passed`: 1 prints every check, like `--verbose`, and 0 prints only failures.

```c
#include "generated/hammer_tests.h"

static void print_to_uart(const char *line)
{
    uart_write_string(line);
}

int main(void)
{
    return run_all_tests_with(print_to_uart, 0) == 0 ? 0 : 1;
}
```

`run_all_tests()` is the same as `run_all_tests_with(hammer_print_stdout, 0)`. `hammer_print_stdout` writes to stdout and flushes.

## hammerc.py

```
python hammerc.py <dir> [--nomain]
```

- **Finds tests** in every `*.test.c` file under `<dir>`, including subfolders and skipping `<dir>/generated/`. Files are sorted by path, and tests keep their order within each file.
- **Checks the rules** above. Problems are printed as `file:line: error: ...`, nothing is generated, and the script exits with 1.
- **Writes `<dir>/generated/`:**
  - `hammer_tests.h` has declarations of all tests, `HAMMER_TEST_COUNT`, the `hammer_tests` table, `run_all_tests_with(print, show_passed)` and `run_all_tests()`.
  - `hammer_main.c` has a `main` that passes `argc` and `argv` to `hammer_main`. It returns 0 if every test passed, 1 if any failed, and 2 for an unknown argument.
- **Rewrites a file only when its content changes,** so your build doesn't redo work. It prints whether each file was `written` or `unchanged`.
- **`--nomain` skips `hammer_main.c`.** Use it when you write your own `main`. If a `hammer_main.c` is left from an earlier run, it is deleted so it can't clash with yours.

Include `hammer_tests.h` in exactly one file, the one with `main`. Its contents are `static`, so including it elsewhere would only make unused copies.

## How hammerc.py finds functions

The script doesn't parse C. It works in three simple steps, all in `hammerc.py`:

1. **Blank out** comments, string and character literals, and preprocessor lines. They are replaced with spaces, so line numbers stay correct.
2. **Track brace depth.** Every `{` at file level whose preceding text ends in `)` and contains no `=` is a function definition. Structs, enums and initializers are skipped.
3. **Classify** each function by its preceding text: it contains `static` (helper), it matches the test signature (test), or it is an error.

Functions created by macros are invisible to the script. Write tests out in full.

## Requirements

- A C99 compiler.
- Python 3.6 or newer, with no packages needed.

## License

HammerC is licensed under the [Apache License 2.0](LICENSE). Copyright 2026 CrucibleC.
