#include <stdio.h>
#include <string.h>

#include "hammerc.h"

/* A helper: returns the text after the last '.', or NULL if there is none. */
static const char *file_extension(const char *path)
{
    const char *dot = strrchr(path, '.');
    return dot ? dot + 1 : NULL;
}

void test_snprintf_formats_text(HammerCtx *t)
{
    char buffer[16];
    snprintf(buffer, sizeof buffer, "%d-%s", 7, "up");
    CHECK_EQ_STR(buffer, "7-up");
}

void test_file_extension(HammerCtx *t)
{
    CHECK_EQ_STR(file_extension("notes.txt"), "txt");
    CHECK_EQ_STR(file_extension("archive.tar.gz"), "gz");
    CHECK_EQ_STR(file_extension("README"), NULL);
}

void test_strtok_splits_on_comma(HammerCtx *t)
{
    char text[] = "red,green";
    REQUIRE_EQ_STR(strtok(text, ","), "red");
    CHECK_EQ_STR(strtok(NULL, ","), "green");
    CHECK_EQ_STR(strtok(NULL, ","), NULL);
}

/* Fails on purpose so you can see what CHECK_EQ_STR prints,
   for two different strings and for NULL against a string. */
void test_string_fails_on_purpose(HammerCtx *t)
{
    CHECK_EQ_STR(file_extension("photo.jpeg"), "jpg");
    CHECK_EQ_STR(file_extension("Makefile"), "mk");
}
