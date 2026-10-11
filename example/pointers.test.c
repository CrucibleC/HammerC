#include <string.h>

#include "hammerc.h"

void test_strchr_finds_first_match(HammerCtx *t)
{
    const char *text = "hammer";
    CHECK_EQ_PTR(strchr(text, 'm'), &text[2]);
    CHECK_EQ_PTR(strchr(text, 'x'), NULL);
}

void test_array_end_pointer(HammerCtx *t)
{
    int numbers[4] = { 1, 2, 3, 4 };
    int *end = numbers + 4;
    REQUIRE(end - numbers == 4);
    CHECK_EQ_PTR(&numbers[3] + 1, end);
}

/* Fails on purpose so you can see what a failing REQUIRE prints.
   The test stops at the failed REQUIRE, so the CHECK after it never runs. */
void test_require_stops_on_purpose(HammerCtx *t)
{
    const char *text = "hammer";
    REQUIRE_EQ_PTR(strchr(text, 'x'), &text[0]);
    CHECK(text[0] == 'h');
}
