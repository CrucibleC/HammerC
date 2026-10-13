#include "hammerc.h"

/* Text that only looks like a test must be ignored:
   void test_in_block_comment(HammerCtx *t) { } */
// void test_in_line_comment(HammerCtx *t) { }
// a backslash at the end of a line comment continues it onto the next line \
void test_in_continued_comment(HammerCtx *t) { }
static const char *text = "void test_in_string(HammerCtx *t) { }";
static const char brace = '{';
#define BLOCK { int x; }
#define MAKE_TEST(name) \
    void test_in_macro(HammerCtx *t) { }

/* Structs, enums and initializers are not functions. */
struct point { int x, y; };
enum color { RED, GREEN };
static const int numbers[] = { 1, 2, 3 };

static int helper(int a) { return a; }

static struct point make_point(void)
{
    struct point p = { 1, 2 };
    return p;
}

/* The return type has a struct body, so a '}' comes before the name. */
static struct pair { int a, b; } make_pair(void)
{
    struct pair p = { 1, 2 };
    return p;
}

/* Attributes are ignored when looking for static. */
__attribute__((unused)) static int attribute_first(void) { return 0; }
static int __attribute__((noinline)) attribute_after_static(void) { return 0; }

void test_first(HammerCtx *t) { (void)t; }

void test_brace_on_next_line(HammerCtx *t)
{
    (void)t;
}

void test_last(HammerCtx *t) { (void)t; }
