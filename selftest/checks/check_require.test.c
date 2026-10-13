#include "hammerc.h"

/* A helper that uses REQUIRE: a failure there only leaves the helper. */
static void helper_with_failing_require(HammerCtx *t, int *reached_end)
{
    REQUIRE(1 == 2);
    *reached_end = 1;
}

void test_check_passes(HammerCtx *t)
{
    CHECK(1 + 1 == 2);
    CHECK(10 % 3 == 1); /* the check text is printed as-is, '%' included */
}

void test_check_fails_and_continues(HammerCtx *t)
{
    CHECK(1 + 1 == 3);
    CHECK(2 + 2 == 4); /* still runs and is counted */
}

void test_require_passes(HammerCtx *t)
{
    REQUIRE(1);
    CHECK(1);
}

void test_require_fails_and_stops(HammerCtx *t)
{
    REQUIRE(0);
    CHECK(1); /* never runs, so it is not counted */
}

void test_require_in_helper_only_leaves_helper(HammerCtx *t)
{
    int reached_end = 0;
    helper_with_failing_require(t, &reached_end);
    CHECK(reached_end == 0); /* the helper stopped... */
    CHECK(1);                /* ...but the test goes on */
}

void test_no_checks(HammerCtx *t)
{
    (void)t; /* a test without checks passes */
}
