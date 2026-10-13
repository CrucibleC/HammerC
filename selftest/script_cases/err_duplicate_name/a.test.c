#include "hammerc.h"

/* Test names must be unique within a file and across files. */
void test_same(HammerCtx *t) { (void)t; }

void test_twice_in_this_file(HammerCtx *t) { (void)t; }

void test_twice_in_this_file(HammerCtx *t) { (void)t; }
