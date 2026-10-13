#include <stddef.h>

#include "hammerc.h"

void test_eq_str_passes(HammerCtx *t)
{
    char built[4] = { 'a', 'b', 'c', '\0' };
    CHECK_EQ_STR(built, "abc"); /* compares the characters, not the pointers */
    CHECK_EQ_STR("", "");
    CHECK_EQ_STR(NULL, NULL);   /* two NULLs are equal */
}

void test_eq_str_fails_prints_values(HammerCtx *t)
{
    CHECK_EQ_STR("abc", "abd");
    CHECK_EQ_STR(NULL, "abc");
    CHECK_EQ_STR("abc", NULL);
    CHECK_EQ_STR("100%", "%d"); /* a '%' in a value is printed as-is */
}

void test_require_eq_str_stops(HammerCtx *t)
{
    REQUIRE_EQ_STR("a", "b");
    CHECK(1); /* never runs */
}

/* Failing pointer checks print addresses, which change from run to run,
   so they are tested in custom_print/, where the addresses are hidden. */
void test_eq_ptr_passes(HammerCtx *t)
{
    int numbers[3] = { 0, 0, 0 };
    CHECK_EQ_PTR(&numbers[1], numbers + 1);
    CHECK_EQ_PTR(NULL, NULL);
    REQUIRE_EQ_PTR(&numbers[0], numbers);
    CHECK(1); /* runs, because the REQUIRE passed */
}
