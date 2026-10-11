#include <limits.h>
#include <stdlib.h>

#include "hammerc.h"

void test_abs_of_negative(HammerCtx *t)
{
    CHECK_EQ_INT(abs(-7), 7);
    CHECK_EQ_INT(abs(0), 0);
}

void test_strtol_parses_negative(HammerCtx *t)
{
    char *end = NULL;
    REQUIRE_EQ_INT(strtol("-42", &end, 10), -42);
    CHECK_EQ_INT(*end, '\0');
}

void test_unsigned_wraps_around(HammerCtx *t)
{
    unsigned int zero = 0;
    CHECK_EQ_UINT(zero - 1, UINT_MAX);
    CHECK_EQ_UINT(strtoull("18446744073709551615", NULL, 10), ULLONG_MAX);
}

/* Fails on purpose so you can see that CHECK_EQ_INT prints both values. */
void test_int_fails_on_purpose(HammerCtx *t)
{
    CHECK_EQ_INT(abs(-7), -7);
}
