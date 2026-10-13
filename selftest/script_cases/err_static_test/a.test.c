#include "hammerc.h"

/* A static test could never be run, so it is an error. */
static void test_static(HammerCtx *t) { (void)t; }

static inline void
test_static_inline(HammerCtx *t) { (void)t; }

/* A static helper whose name only starts with test_ is fine. */
static int test_helper(int a) { return a; }

void test_ok(HammerCtx *t) { (void)t; }
