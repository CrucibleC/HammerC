#include "hammerc.h"
#include "calc.h"

/* A helper: static, so hammerc.py does not treat it as a test. */
static int is_between(int value, int low, int high)
{
    return value >= low && value <= high;
}

void test_add_small_numbers(HammerCtx *t)
{
    CHECK(calc_add(2, 2) == 4);
    CHECK(calc_add(-3, 3) == 0);
}

void test_clamp_stays_in_range(HammerCtx *t)
{
    for (int value = -10; value <= 10; value++) {
        CHECK(is_between(calc_clamp(value, -5, 5), -5, 5));
    }
}

/* Fails on purpose so you can see what a failing CHECK prints.
   The test keeps going after the failure. */
void test_add_fails_on_purpose(HammerCtx *t)
{
    CHECK(calc_add(2, 2) == 5);
    CHECK(calc_add(1, 1) == 2);
}
