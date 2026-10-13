#include <limits.h>

#include "hammerc.h"

void test_eq_int_passes(HammerCtx *t)
{
    CHECK_EQ_INT(-5, -5);
    CHECK_EQ_INT(LLONG_MIN, LLONG_MIN);
    CHECK_EQ_INT('A', 65);
}

void test_eq_int_fails_prints_values(HammerCtx *t)
{
    CHECK_EQ_INT(-1, 1);
    CHECK_EQ_INT(LLONG_MAX, LLONG_MIN);
}

void test_require_eq_int_stops(HammerCtx *t)
{
    REQUIRE_EQ_INT(2, 3);
    CHECK(1); /* never runs */
}

void test_eq_uint_passes(HammerCtx *t)
{
    CHECK_EQ_UINT(ULLONG_MAX, ULLONG_MAX);
    CHECK_EQ_UINT(0u, 0u);
}

void test_eq_uint_fails_prints_values(HammerCtx *t)
{
    CHECK_EQ_UINT(ULLONG_MAX, 0u);
}

void test_require_eq_uint_stops(HammerCtx *t)
{
    REQUIRE_EQ_UINT(1u, 2u);
    CHECK(1); /* never runs */
}

/* Each argument is evaluated exactly once. */
void test_arguments_evaluated_once(HammerCtx *t)
{
    int calls = 0;
    CHECK_EQ_INT(++calls, 1);
    CHECK_EQ_INT(calls, 1);
}
