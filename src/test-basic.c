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
#include "c-dvar-type.h"

static void test_basic_serialization(bool big_endian) {
        _cleanup_(c_dvar_type_freep) CDVarType *type = NULL;
        _cleanup_(c_dvar_freep) CDVar *var = NULL;
        const char *str1, *str2;
        size_t n_data;
        void *data;
        uint64_t u64 = 0;
        uint32_t u32 = 0;
        uint16_t u16 = 0;
        uint8_t u8 = 0;
        double d = 0.0;
        int r;

        /*
         * A very basic serialization and deserialization test that serves as
         * base-line for reader/writer operation testing.
         *
         * We simply allocate and build a variant of a fixed type, then read it
         * back and verify the data matches.
         */

        r = c_dvar_type_new_from_string(&type, "(yua{sv}d)");
        assert(!r);

        r = c_dvar_new(&var);
        assert(!r);

        /* write example data */

        c_dvar_begin_write(var, big_endian, type, 1);

        c_dvar_write(var,
                     "(yu[{s<q>}{s<t>}]d)",
                     UINT8_C(7),
                     UINT32_C(7),
                     "foo",
                     c_dvar_type_q,
                     UINT16_C(7),
                     "bar",
                     c_dvar_type_t,
                     UINT64_C(7),
                     7.0);

        r = c_dvar_end_write(var, &data, &n_data);
        assert(!r);

        /* read back example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_read(var,
                    "(yu[{s<q>}{s<t>}]d)",
                    &u8,
                    &u32,
                    &str1,
                    NULL,
                    &u16,
                    &str2,
                    c_dvar_type_t,
                    &u64,
                    &d);
        assert(u8 == 7);
        assert(u32 == 7);
        assert(!strcmp(str1, "foo"));
        assert(u16 == 7);
        assert(!strcmp(str2, "bar"));
        assert(u64 == 7);
        assert(d > 6.0 && d < 8.0);

        r = c_dvar_end_read(var);
        assert(!r);

        /* skip example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "(yu[{s*}{s<t>}]d)", c_dvar_type_t);

        r = c_dvar_end_read(var);
        assert(!r);

        /* skip example data again */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "*");

        r = c_dvar_end_read(var);
        assert(!r);

        free(data);
}

static void test_dbus_message(void) {
        static const CDVarType type[] = {
                C_DVAR_T_INIT(
                        /* (yyyyuua(yv)(st)) */
                        C_DVAR_T_TUPLE8(
                                C_DVAR_T_y,
                                C_DVAR_T_y,
                                C_DVAR_T_y,
                                C_DVAR_T_y,
                                C_DVAR_T_u,
                                C_DVAR_T_u,
                                C_DVAR_T_ARRAY(
                                        C_DVAR_T_TUPLE2(
                                                C_DVAR_T_y,
                                                C_DVAR_T_v
                                        )
                                ),
                                C_DVAR_T_TUPLE2(
                                        C_DVAR_T_s,
                                        C_DVAR_T_t
                                )
                        )
                )
        };
        _cleanup_(c_dvar_freep) CDVar *var = NULL;
        size_t n_data;
        void *data;
        int r;

        /*
         * Simple test that marshals a basic dbus-message type with a fixed
         * field-array and body.
         */

        r = c_dvar_new(&var);
        assert(!r);

        c_dvar_begin_write(var, (__BYTE_ORDER == __BIG_ENDIAN), type, 1);

        c_dvar_write(var, "(yyyyuu[", 0, 0, 0, 0, 0, 0);
        c_dvar_write(var, "(y<u>)", 0, c_dvar_type_u, 0);
        c_dvar_write(var, "(y<y>)", 0, c_dvar_type_y, 0);
        c_dvar_write(var, "](st))", "", 0);

        r = c_dvar_end_write(var, &data, &n_data);
        assert(!r);

        free(data);
}

static void test_dbus_body(void) {
        static const CDVarType types[] = {
                /* sss */
                C_DVAR_T_INIT(C_DVAR_T_s),
                C_DVAR_T_INIT(C_DVAR_T_s),
                C_DVAR_T_INIT(C_DVAR_T_s),
        };
        _cleanup_(c_dvar_freep) CDVar *var = NULL;
        const char *str;
        size_t n_data;
        void *data;
        int r;

        /*
         * Simple test that marshals a basic dbus-message body with 3 strings.
         */

        r = c_dvar_new(&var);
        assert(!r);

        c_dvar_begin_write(var, (__BYTE_ORDER == __BIG_ENDIAN), types, 3);
        c_dvar_write(var, "sss", "fo", "ob", "ar");
        r = c_dvar_end_write(var, &data, &n_data);
        assert(!r);

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), types, 3, data, n_data);
        c_dvar_read(var, "s", &str);
        assert(!strcmp(str, "fo"));
        c_dvar_read(var, "s", &str);
        assert(!strcmp(str, "ob"));
        c_dvar_read(var, "s", &str);
        assert(!strcmp(str, "ar"));
        r = c_dvar_end_read(var);
        assert(!r);

        free(data);
}

int main(int argc, char **argv) {
        test_basic_serialization(true);
        test_basic_serialization(false);
        test_dbus_message();
        test_dbus_body();
        return 0;
}
