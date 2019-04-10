/*
 * Tests for String Validity
 */

#undef NDEBUG
#include <assert.h>
#include <c-stdaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

static void test_path(void) {
        static const char *valid[] = {
                "/",
                "/foo/bar",
                "/f/o/o/b/a/r",
                "/0",
                "/_",
                "/_0_f",
        };
        static const char *invalid[] = {
                "",
                "//",
                "f/",
                "\1",
                "\1/",
                "/f/",
                "/f//o",
        };
        size_t i;

        for (i = 0; i < sizeof(valid) / sizeof(*valid); ++i)
                c_assert(c_dvar_is_path(valid[i], strlen(valid[i])));

        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); ++i)
                c_assert(!c_dvar_is_path(invalid[i], strlen(invalid[i])));

        c_assert(!c_dvar_is_path("\0/", 2));
        c_assert(!c_dvar_is_path("/\0", 2));
        c_assert(!c_dvar_is_path("/foobar\0", 8));
        c_assert(!c_dvar_is_path("/\0foobar", 8));
}

static void test_signature(void) {
        static const char *valid[] = {
                "u",
                "uu",
                "(uu)",
                "(uu)(uu)",
                "ayayay",
                "aaaay",
                "a{yb}u",
                "ua{yb}",
        };
        static const char *invalid[] = {
                "$",
                "u()",
                "()u",
                "a{yb}{yb}",
        };
        size_t i;

        for (i = 0; i < sizeof(valid) / sizeof(*valid); ++i)
                c_assert(c_dvar_is_signature(valid[i], strlen(valid[i])));

        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); ++i)
                c_assert(!c_dvar_is_signature(invalid[i], strlen(invalid[i])));

        c_assert(!c_dvar_is_signature("\0u", 2));
        c_assert(!c_dvar_is_signature("u\0", 2));
        c_assert(!c_dvar_is_signature("(u\0)", 4));
        c_assert(!c_dvar_is_signature("(u\0u)", 5));
}

static void test_type(void) {
        static const char *valid[] = {
                "u",
                "v",
                "au",
                "a{yv}",
                "(ay(vv)a(bb)a{yb}(uu))",
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau",
                "((((((((((((((((((((((((((((((((u))))))))))))))))))))))))))))))))",
                "((((((((((((((((((((((((((((((((aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau))))))))))))))))))))))))))))))))",
                "a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(u))))))))))))))))))))))))))))))))",
        };
        static const char *invalid[] = {
                "A", /* invalid element */
                "$", /* invalid element */
                "{}", /* invalid pair */
                "{)", /* non-matching brackets */
                "(}", /* non-matching brackets */
                "{()y}", /* invalid pair */
                "{yyy}", /* invalid pair */
                "(yy{))", /* non-matching brackets */
                "(yy{}}", /* invalid pair */
                "(yy{)}", /* non-matching brackets */
                "(", /* unclosed container */
                ")", /* closing wrong container */
                "((", /* unclosed container */
                ")(", /* closing wrong container */
                "a", /* unclosed container */
                "aaa", /* unclosed container */
                "{aau}", /* invalid pair */
                "{vu}", /* invalid pair*/
                "(uu(u())uu{vu}uu)", /* invalid pair */
                "(uu(u())uu(vu}uu)", /* non-matching brackets */
                "(uu(u())uu{uu)uu)", /* non-matching brackets */
                "(uu(u())uuuuuuuu}", /* non-matching brackets */
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaau", /* depth exceeded */
                "(((((((((((((((((((((((((((((((((u)))))))))))))))))))))))))))))))))", /* depth exceeded */
                "a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(a(au))))))))))))))))))))))))))))))))", /* depth exceeded */
                "{yy}", /* pair not in array */
                "(ay{yy})", /* pair not in array */
                "()", /* empty tuple */
                "(ay())", /* empty tuple */
        };
        size_t i;

        for (i = 0; i < sizeof(valid) / sizeof(*valid); ++i)
                c_assert(c_dvar_is_type(valid[i], strlen(valid[i])));

        for (i = 0; i < sizeof(invalid) / sizeof(*invalid); ++i)
                c_assert(!c_dvar_is_type(invalid[i], strlen(invalid[i])));

        c_assert(!c_dvar_is_type("\0u", 2));
        c_assert(!c_dvar_is_type("u\0", 2));
        c_assert(!c_dvar_is_type("(u\0)", 4));
        c_assert(!c_dvar_is_type("(u\0u)", 5));
}

int main(int argc, char **argv) {
        test_path();
        test_signature();
        test_type();
        return 0;
}
