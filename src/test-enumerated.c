/*
 * Test / Data Verification with Enumerated Types
 *
 * Use the dbus-typenum library to enumerate D-Bus types systematically. Then
 * verify we can deal with them correctly. All of them.
 */

#undef NDEBUG
#include <assert.h>
#include <c-stdaux.h>
#include <dbus-typenum.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

static void test_valid_type(const char *typestr, size_t n_typestr) {
        CDVarType *type = NULL;
        int r;

        /* verify @typestr is a valid type */

        r = c_dvar_type_new_from_signature(&type, typestr, n_typestr);
        c_assert(!r);
        c_assert(type->length == n_typestr);
        c_dvar_type_free(type);
}

static void test_enumerated_types(void) {
        DBusTypenum *e;
        char *str = NULL, *pos = NULL;
        size_t i, n_str = 0;
        char c;
        int r;

        /*
         * Enumerate valid types via DBusTypenum and check that all generated
         * types are accepted by c-dvar.
         */

        r = dbus_typenum_new(&e, 0);
        c_assert(!r);

        n_str = 2;
        str = malloc(n_str);
        c_assert(str);
        pos = str;

        for (i = 0; i < 1 << 16; ++i) {
                dbus_typenum_seed_u32(e, i);

                do {
                        c = dbus_typenum_step(e);
                        if (pos - str >= (ssize_t)n_str) {
                                str = realloc(str, (pos - str) * 2);
                                c_assert(str);
                                pos = str + n_str;
                                n_str *= 2;
                        }

                        *pos++ = c;
                } while (c);

                test_valid_type(str, pos - str - 1);
                pos = str;
                dbus_typenum_reset(e);
        }

        free(str);
        dbus_typenum_free(e);
}

int main(int argc, char **argv) {
        test_enumerated_types();
        return 0;
}
