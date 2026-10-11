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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Receives one complete line of output, ending in '\n'. */
typedef void (*HammerPrintFn)(const char *line);

typedef struct {
    int checks;   /* number of checks evaluated */
    int failures; /* number of those that failed */
} HammerCounts;

/* One context lives for the whole run. hammer_run() resets test before each test
   and adds it into run afterwards. The check functions only touch test. */
typedef struct {
    HammerPrintFn print;
    int           show_passed; /* also print checks that pass (--verbose) */
    const char   *test_name;   /* test currently running */
    const char   *test_file;
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

/* Longest line hammer_printf() builds, including the '\n'. Longer lines are cut. */
#define HAMMER_LINE_MAX 512

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

/* Counts one check and prints its PASSED or FAILED line and its AT line.
   A passed check is only printed when show_passed is set. Returns passed. */
static inline int hammer_check(HammerCtx *t, int passed,
                               const char *check_text, const char *file, int line)
{
    t->test.checks++;
    if (!passed) {
        t->test.failures++;
    }
    if (!passed || t->show_passed) {
        hammer_printf(t->print, "     %s:   %s\n", passed ? "PASSED" : "FAILED", check_text);
        hammer_printf(t->print, "     AT:       %s:%d\n", file, line);
    }
    return passed;
}

/* Returns 1 if the pointers are equal, 0 if not (and prints both values). */
static inline int hammer_check_eq_ptr(HammerCtx *t, const void *actual, const void *expected,
                                      const char *check_text, const char *file, int line)
{
    if (hammer_check(t, actual == expected, check_text, file, line)) {
        return 1;
    }
    hammer_printf(t->print, "     EXPECTED: %p\n", (void *)expected);
    hammer_printf(t->print, "     ACTUAL:   %p\n", (void *)actual);
    return 0;
}

/* Returns 1 if the integers are equal, 0 if not (and prints both values). */
static inline int hammer_check_eq_int(HammerCtx *t, long long actual, long long expected,
                                      const char *check_text, const char *file, int line)
{
    if (hammer_check(t, actual == expected, check_text, file, line)) {
        return 1;
    }
    hammer_printf(t->print, "     EXPECTED: %lld\n", expected);
    hammer_printf(t->print, "     ACTUAL:   %lld\n", actual);
    return 0;
}

/* Returns 1 if the unsigned integers are equal, 0 if not (and prints both values). */
static inline int hammer_check_eq_uint(HammerCtx *t, unsigned long long actual, unsigned long long expected,
                                       const char *check_text, const char *file, int line)
{
    if (hammer_check(t, actual == expected, check_text, file, line)) {
        return 1;
    }
    hammer_printf(t->print, "     EXPECTED: %llu\n", expected);
    hammer_printf(t->print, "     ACTUAL:   %llu\n", actual);
    return 0;
}

/* Returns 1 if the strings are equal, 0 if not (and prints both values).
   Two NULLs are equal; NULL and a string are not. */
static inline int hammer_check_eq_str(HammerCtx *t, const char *actual, const char *expected,
                                      const char *check_text, const char *file, int line)
{
    int equal;
    if (actual == NULL || expected == NULL) {
        equal = actual == expected;
    } else {
        equal = strcmp(actual, expected) == 0;
    }
    if (hammer_check(t, equal, check_text, file, line)) {
        return 1;
    }
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
    return 0;
}

/* Runs every test in the table and prints a summary. Returns the number of failed tests.
   show_passed: 1 prints every check, 0 prints only failed checks. */
static inline int hammer_run(const HammerTest *tests, int count, HammerPrintFn print, int show_passed)
{
    HammerCtx t = {0};
    t.print = print;
    t.show_passed = show_passed;

    for (int i = 0; i < count; i++) {
        t.test_name = tests[i].name;
        t.test_file = tests[i].file;
        t.test.checks = 0;
        t.test.failures = 0;

        hammer_printf(print, "RUN  %s (%s)\n", t.test_name, t.test_file);
        tests[i].fn(&t);
        hammer_printf(print, "     %d/%d checks passed\n\n",
                      t.test.checks - t.test.failures, t.test.checks);

        t.run.checks += t.test.checks;
        t.run.failures += t.test.failures;
        t.tests_run++;
        if (t.test.failures > 0) {
            t.tests_failed++;
        }
    }

    hammer_printf(print, "TESTS:   %d\n", t.tests_run);
    hammer_printf(print, "SUCCESS: %d\n", t.tests_run - t.tests_failed);
    hammer_printf(print, "FAILED:  %d\n", t.tests_failed);
    hammer_printf(print, "\n");
    hammer_printf(print, "CHECKS:  %d\n", t.run.checks);
    hammer_printf(print, "SUCCESS: %d\n", t.run.checks - t.run.failures);
    hammer_printf(print, "FAILED:  %d\n", t.run.failures);
    return t.tests_failed;
}

/* The body of a test runner's main(). Reads the command line, runs the tests
   and returns 0 if every test passed, 1 if any failed, 2 for a bad argument. */
static inline int hammer_main(int argc, char **argv, const HammerTest *tests, int count)
{
    int show_passed = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-v") == 0) {
            show_passed = 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("usage: %s [--verbose]\n"
                   "  -v, --verbose  print every check, not only the failed ones\n", argv[0]);
            return 0;
        } else {
            fprintf(stderr, "%s: unknown argument '%s' (try --help)\n", argv[0], argv[i]);
            return 2;
        }
    }
    return hammer_run(tests, count, hammer_print_stdout, show_passed) == 0 ? 0 : 1;
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
