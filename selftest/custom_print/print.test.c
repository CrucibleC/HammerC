#include "hammerc.h"

void test_passing(HammerCtx *t)
{
    CHECK(1);
}

/* This suite is built with HAMMER_LINE_MAX=64, so these lines are cut. */
void test_long_line_is_cut(HammerCtx *t)
{
    CHECK_EQ_STR("this string is long enough to go past the 64 byte line limit", "short");
}

/* The addresses are hidden by main.c, because they change from run to run. */
void test_require_eq_ptr_fails_and_stops(HammerCtx *t)
{
    int a = 0;
    int b = 0;
    REQUIRE_EQ_PTR(&a, &b);
    CHECK(1); /* never runs */
}
