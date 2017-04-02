/*
 * Tests for Basic Operations
 *
 * A bunch of basic tests to verify the most simple setups and operations work.
 * Those tests should be fast and provide good coverage for initial testing.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

static void test_basic_serialization(void) {
        _cleanup_(c_dvar_type_freep) CDVarType *type = NULL, *type_q = NULL, *type_t = NULL;
        _cleanup_(c_dvar_freep) CDVar *var = NULL;
        const char *str1, *str2;
        size_t n_data;
        void *data;
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
        int r;

        /*
         * A very basic serialization and deserialization test that serves as
         * base-line for reader/writer operation testing.
         *
         * We simply allocate and build a variant of a fixed type, then read it
         * back and verify the data matches.
         */

        r = c_dvar_type_new_from_string(&type, "(yua{sv})");
        assert(!r);

        r = c_dvar_type_new_from_string(&type_q, "q");
        assert(!r);

        r = c_dvar_type_new_from_string(&type_t, "t");
        assert(!r);

        r = c_dvar_new(&var);
        assert(!r);

        /* write example data */

        c_dvar_begin_write(var, type);

        c_dvar_write(var,
                     "(yu[{s<q>}{s<t>}])",
                     UINT8_C(7),
                     UINT32_C(7),
                     "foo",
                     type_q,
                     UINT16_C(7),
                     "bar",
                     type_t,
                     UINT64_C(7));

        r = c_dvar_end_write(var, &data, &n_data);
        assert(!r);

        /* read back example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, data, n_data);

        c_dvar_read(var,
                    "(yu[{s<q>}{s<t>}])",
                    &u8,
                    &u32,
                    &str1,
                    NULL,
                    &u16,
                    &str2,
                    type_t,
                    &u64);
        assert(u8 == 7);
        assert(u32 == 7);
        assert(!strcmp(str1, "foo"));
        assert(u16 == 7);
        assert(!strcmp(str2, "bar"));
        assert(u64 == 7);

        r = c_dvar_end_read(var);
        assert(!r);

        /* skip example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, data, n_data);

        c_dvar_skip(var, "(yu[{sv}{s<t>}])", type_t);

        r = c_dvar_end_read(var);
        assert(!r);

        free(data);
}

int main(int argc, char **argv) {
        test_basic_serialization();
        return 0;
}
