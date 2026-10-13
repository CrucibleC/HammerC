#include "hammerc.h"

/* Every non-static function must have the exact test signature. */
int helper(int a) { return a; }

void test_wrong_parameter_name(HammerCtx *ctx) { (void)ctx; }

int test_returns_int(HammerCtx *t) { (void)t; return 0; }

__declspec(noinline) int declspec_helper(void) { return 0; }

int main(void) { return 0; }

void test_ok(HammerCtx *t) { (void)t; }
