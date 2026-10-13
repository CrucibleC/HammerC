/* Our own main(), as with hammerc.py --nomain. It runs the tests with a print
   function that stores every line, checks that each line is one complete line
   that fits in HAMMER_LINE_MAX, and then prints what it stored. This suite is
   built with -DHAMMER_LINE_MAX=64. */
#include <stdio.h>
#include <string.h>

#include "generated/hammer_tests.h"

#define MAX_LINES 64

static char lines[MAX_LINES][HAMMER_LINE_MAX];
static int line_count;
static int bad_lines; /* empty, too long, or not exactly one '\n' at the end */

static void store_line(const char *line)
{
    size_t length = strlen(line);
    if (length == 0 || length > (size_t)(HAMMER_LINE_MAX - 1) ||
        line[length - 1] != '\n' || memchr(line, '\n', length - 1) != NULL) {
        bad_lines++;
    }
    if (line_count < MAX_LINES) {
        snprintf(lines[line_count], sizeof lines[line_count], "%s", line);
    }
    line_count++;
}

int main(void)
{
    int failed = run_all_tests_with(store_line, HAMMER_NORMAL);

    printf("run_all_tests_with returned %d\n", failed);
    printf("lines: %d, bad lines: %d\n", line_count, bad_lines);
    printf("--- stored lines, with pointer values hidden ---\n");
    for (int i = 0; i < line_count && i < MAX_LINES; i++) {
        /* This suite only has string and pointer checks. A string value starts
           with a quote; anything else is a pointer, which changes every run. */
        int is_value = strncmp(lines[i], "     EXPECTED: ", 15) == 0 ||
                       strncmp(lines[i], "     ACTUAL:   ", 15) == 0;
        if (is_value && lines[i][15] != '"') {
            printf("%.15s<pointer>\n", lines[i]);
        } else {
            fputs(lines[i], stdout);
        }
    }
    return 0;
}
