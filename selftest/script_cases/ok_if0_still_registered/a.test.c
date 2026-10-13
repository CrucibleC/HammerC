#include "hammerc.h"

/* Known limitation: hammerc.py ignores #if, so a test inside #if 0 is still
   put in the table, and the build then fails with an undefined reference.
   This case pins down the documented behaviour. */
#if 0
void test_inside_if0(HammerCtx *t) { (void)t; }
#endif

void test_outside(HammerCtx *t) { (void)t; }
