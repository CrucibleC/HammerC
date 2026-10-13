/* Copyright 2026 CrucibleC
 * SPDX-License-Identifier: Apache-2.0
 */

/* hammerc.h - HammerC, a tiny test system for C (C99).
 *
 * Everything lives in this header. All functions are static inline, so every
 * file that includes it gets its own private copy and nothing has to be linked.
 *
 * Tests are plain functions with exactly this signature, on one line:
 *
 *     void test_<name>(HammerCtx *t)
 *
 * hammerc.py finds them and generates the table that hammer_run() walks.
 * The check macros below use the parameter named t.
 */
#ifndef HAMMERC_H
#define HAMMERC_H

/* The HammerC version this header belongs to. hammerc.py has the same number. */
#define HAMMERC_VERSION "1.0.0"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Receives one complete line of output, ending in '\n'. */
typedef void (*HammerPrintFn)(const char *line);

/* How much a run prints. Each level prints everything the one before it prints, and more. */
typedef enum {
    HAMMER_SILENT,  /* nothing: only the return value tells the result (--silent) */
    HAMMER_QUIET,   /* only the tests that failed, and the summary (--quiet) */
    HAMMER_NORMAL,  /* every test's RUN and N/M lines, failed checks, and the summary */
    HAMMER_VERBOSE  /* also every passed check, with the values of *_EQ_* checks (--verbose) */
} HammerOutput;

typedef struct {
    int checks;   /* number of checks evaluated */
    int failures; /* number of those that failed */
} HammerCounts;

/* One context lives for the whole run. hammer_run() resets test before each test
   and adds it into run afterwards. The check functions only touch test. */
typedef struct {
    HammerPrintFn print;
    HammerOutput  output;
    const char   *test_name;   /* test currently running */
    const char   *test_file;
    int           test_shown;  /* 1 once the current test's RUN line is printed */
    HammerCounts  test;        /* counts for the current test */
    HammerCounts  run;         /* counts for all tests finished so far */
    int           tests_run;
    int           tests_failed;
} HammerCtx;

typedef void (*HammerTestFn)(HammerCtx *t);

/* One row in the generated test table. */
typedef struct {
    const char  *name;
    const char  *file;
    HammerTestFn fn;
} HammerTest;

/* Longest line hammer_printf() builds, including the '\n'. Longer lines are cut.
   The buffer is on the stack; to change its size, build with -DHAMMER_LINE_MAX=<n>. */
#ifndef HAMMER_LINE_MAX
#define HAMMER_LINE_MAX 512
#endif

/* Default print function. Flushes so the last line before a crash is visible. */
static inline void hammer_print_stdout(const char *line)
{
    fputs(line, stdout);
    fflush(stdout);
}

/* Format one line and hand it to print. */
static inline void hammer_printf(HammerPrintFn print, const char *format, ...)
{
    char line[HAMMER_LINE_MAX];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(line, sizeof line, format, args);
    va_end(args);
    if (length >= (int)sizeof line) {
        line[sizeof line - 2] = '\n'; /* the line was cut: keep its line ending */
    }
    print(line);
}

/* Prints the current test's RUN line, unless it is printed already. NORMAL and
   VERBOSE print it before the test starts, so a crash shows which test it was.
   QUIET prints it just before the test's first failed check. */
static inline void hammer_show_test(HammerCtx *t)
{
    if (!t->test_shown) {
        hammer_printf(t->print, "RUN  %s (%s)\n", t->test_name, t->test_file);
        t->test_shown = 1;
    }
}

/* 1 if a check with this result is printed: a failed one from QUIET up,
   a passed one only in VERBOSE. */
static inline int hammer_shows_check(const HammerCtx *t, int passed)
{
    return t->output >= (passed ? HAMMER_VERBOSE : HAMMER_QUIET);
}

/* Counts one check and, if it is shown, prints its PASSED or FAILED line and
   its AT line. Returns passed. */
static inline int hammer_check(HammerCtx *t, int passed,
                               const char *check_text, const char *file, int line)
{
    t->test.checks++;
    if (!passed) {
        t->test.failures++;
    }
    if (hammer_shows_check(t, passed)) {
        hammer_show_test(t);
        hammer_printf(t->print, "     %s:   %s\n", passed ? "PASSED" : "FAILED", check_text);
        hammer_printf(t->print, "     AT:       %s:%d\n", file, line);
    }
    return passed;
}

/* The *_EQ_* checks: each returns 1 if the values are equal, 0 if not, and
   prints both values whenever the check itself is printed. */

static inline int hammer_check_eq_ptr(HammerCtx *t, const void *actual, const void *expected,
                                      const char *check_text, const char *file, int line)
{
    int passed = hammer_check(t, actual == expected, check_text, file, line);
    if (hammer_shows_check(t, passed)) {
        hammer_printf(t->print, "     EXPECTED: %p\n", (void *)expected);
        hammer_printf(t->print, "     ACTUAL:   %p\n", (void *)actual);
    }
    return passed;
}

static inline int hammer_check_eq_int(HammerCtx *t, long long actual, long long expected,
                                      const char *check_text, const char *file, int line)
{
    int passed = hammer_check(t, actual == expected, check_text, file, line);
    if (hammer_shows_check(t, passed)) {
        hammer_printf(t->print, "     EXPECTED: %lld\n", expected);
        hammer_printf(t->print, "     ACTUAL:   %lld\n", actual);
    }
    return passed;
}

static inline int hammer_check_eq_uint(HammerCtx *t, unsigned long long actual, unsigned long long expected,
                                       const char *check_text, const char *file, int line)
{
    int passed = hammer_check(t, actual == expected, check_text, file, line);
    if (hammer_shows_check(t, passed)) {
        hammer_printf(t->print, "     EXPECTED: %llu\n", expected);
        hammer_printf(t->print, "     ACTUAL:   %llu\n", actual);
    }
    return passed;
}

/* Two NULLs are equal; NULL and a string are not. */
static inline int hammer_check_eq_str(HammerCtx *t, const char *actual, const char *expected,
                                      const char *check_text, const char *file, int line)
{
    int equal;
    if (actual == NULL || expected == NULL) {
        equal = actual == expected;
    } else {
        equal = strcmp(actual, expected) == 0;
    }
    int passed = hammer_check(t, equal, check_text, file, line);
    if (hammer_shows_check(t, passed)) {
        /* A string is printed in quotes, NULL is printed as NULL. */
        if (expected) {
            hammer_printf(t->print, "     EXPECTED: \"%s\"\n", expected);
        } else {
            hammer_printf(t->print, "     EXPECTED: NULL\n");
        }
        if (actual) {
            hammer_printf(t->print, "     ACTUAL:   \"%s\"\n", actual);
        } else {
            hammer_printf(t->print, "     ACTUAL:   NULL\n");
        }
    }
    return passed;
}

/* Runs every test in the table and prints what output asks for.
   Returns the number of failed tests. */
static inline int hammer_run(const HammerTest *tests, int count, HammerPrintFn print, HammerOutput output)
{
    HammerCtx t = {0};
    t.print = print;
    t.output = output;

    for (int i = 0; i < count; i++) {
        t.test_name = tests[i].name;
        t.test_file = tests[i].file;
        t.test_shown = 0;
        t.test.checks = 0;
        t.test.failures = 0;

        if (output >= HAMMER_NORMAL) {
            hammer_show_test(&t);
        }
        tests[i].fn(&t);
        if (t.test_shown) { /* QUIET only shows a test once it has failed */
            hammer_printf(print, "     %d/%d checks passed\n",
                          t.test.checks - t.test.failures, t.test.checks);
            hammer_printf(print, "\n");
        }

        t.run.checks += t.test.checks;
        t.run.failures += t.test.failures;
        t.tests_run++;
        if (t.test.failures > 0) {
            t.tests_failed++;
        }
    }

    if (output >= HAMMER_QUIET) {
        hammer_printf(print, "TESTS:   %d\n", t.tests_run);
        hammer_printf(print, "SUCCESS: %d\n", t.tests_run - t.tests_failed);
        hammer_printf(print, "FAILED:  %d\n", t.tests_failed);
        hammer_printf(print, "\n");
        hammer_printf(print, "CHECKS:  %d\n", t.run.checks);
        hammer_printf(print, "SUCCESS: %d\n", t.run.checks - t.run.failures);
        hammer_printf(print, "FAILED:  %d\n", t.run.failures);
    }
    return t.tests_failed;
}

/* The body of a test runner's main(). Reads the command line, runs the tests
   and returns 0 if every test passed, 1 if any failed, 2 for a bad argument
   or an unknown test name. */
static inline int hammer_main(int argc, char **argv, const HammerTest *tests, int count)
{
    HammerOutput output = HAMMER_NORMAL; /* if several are given, the last one counts */
    const char *only = NULL;             /* name of the one test to run, or NULL for all */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            output = HAMMER_VERBOSE;
        } else if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            output = HAMMER_QUIET;
        } else if (strcmp(argv[i], "--silent") == 0 || strcmp(argv[i], "-s") == 0) {
            output = HAMMER_SILENT;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("usage: %s [--verbose | --quiet | --silent] [test_name]\n"
                   "  -v, --verbose  also print passed checks, with the values of *_EQ_* checks\n"
                   "  -q, --quiet    print only the tests that failed, and the summary\n"
                   "  -s, --silent   print nothing; the exit code tells the result\n"
                   "  test_name      run only this test, e.g. test_add_small_numbers\n"
                   "exit code: 0 if every test passed, 1 if any failed, 2 for a bad argument\n",
                   argv[0]);
            return 0;
        } else if (argv[i][0] != '-' && only == NULL) {
            only = argv[i];
        } else {
            fprintf(stderr, "%s: unknown argument '%s' (try --help)\n", argv[0], argv[i]);
            return 2;
        }
    }

    if (only == NULL) {
        return hammer_run(tests, count, hammer_print_stdout, output) == 0 ? 0 : 1;
    }
    /* Run a table of one row: the matching test. */
    for (int i = 0; i < count; i++) {
        if (strcmp(tests[i].name, only) == 0) {
            return hammer_run(&tests[i], 1, hammer_print_stdout, output) == 0 ? 0 : 1;
        }
    }
    fprintf(stderr, "%s: no test named '%s'\n", argv[0], only);
    return 2;
}

/* CHECK:   on failure, record it and keep going.
   REQUIRE: on failure, record it and return from the current function.
            Inside a helper that only leaves the helper, not the test.
   *_EQ_* compare two values and print both on failure:
     *_EQ_INT  as long long
     *_EQ_UINT as unsigned long long
     *_EQ_STR  with strcmp (NULL-safe)
     *_EQ_PTR  as const void * (object pointers, not function pointers)
   Each macro turns its arguments into the check's text at compile time:
   CHECK(a == b) passes the string "CHECK(a == b)". */

#define CHECK(expr) \
    do { hammer_check(t, (expr) ? 1 : 0, "CHECK(" #expr ")", __FILE__, __LINE__); } while (0)

#define REQUIRE(expr) \
    do { if (!hammer_check(t, (expr) ? 1 : 0, "REQUIRE(" #expr ")", __FILE__, __LINE__)) return; } while (0)

#define CHECK_EQ_INT(actual, expected) \
    do { hammer_check_eq_int(t, (actual), (expected), \
                             "CHECK_EQ_INT(" #actual ", " #expected ")", __FILE__, __LINE__); } while (0)

#define REQUIRE_EQ_INT(actual, expected) \
    do { if (!hammer_check_eq_int(t, (actual), (expected), \
                                  "REQUIRE_EQ_INT(" #actual ", " #expected ")", __FILE__, __LINE__)) return; } while (0)

#define CHECK_EQ_UINT(actual, expected) \
    do { hammer_check_eq_uint(t, (actual), (expected), \
                              "CHECK_EQ_UINT(" #actual ", " #expected ")", __FILE__, __LINE__); } while (0)

#define REQUIRE_EQ_UINT(actual, expected) \
    do { if (!hammer_check_eq_uint(t, (actual), (expected), \
                                   "REQUIRE_EQ_UINT(" #actual ", " #expected ")", __FILE__, __LINE__)) return; } while (0)

#define CHECK_EQ_STR(actual, expected) \
    do { hammer_check_eq_str(t, (actual), (expected), \
                             "CHECK_EQ_STR(" #actual ", " #expected ")", __FILE__, __LINE__); } while (0)

#define REQUIRE_EQ_STR(actual, expected) \
    do { if (!hammer_check_eq_str(t, (actual), (expected), \
                                  "REQUIRE_EQ_STR(" #actual ", " #expected ")", __FILE__, __LINE__)) return; } while (0)

#define CHECK_EQ_PTR(actual, expected) \
    do { hammer_check_eq_ptr(t, (actual), (expected), \
                             "CHECK_EQ_PTR(" #actual ", " #expected ")", __FILE__, __LINE__); } while (0)

#define REQUIRE_EQ_PTR(actual, expected) \
    do { if (!hammer_check_eq_ptr(t, (actual), (expected), \
                                  "REQUIRE_EQ_PTR(" #actual ", " #expected ")", __FILE__, __LINE__)) return; } while (0)

#endif
