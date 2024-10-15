/*
 * Tests for Basic Operations
 *
 * A bunch of basic tests to verify the most simple setups and operations work.
 * Those tests should be fast and provide good coverage for initial testing.
 */

#undef NDEBUG
#include <assert.h>
#include <c-stdaux.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"
#include "c-dvar-type.h"

static void test_basic_serialization(bool big_endian) {
        _c_cleanup_(c_dvar_type_freep) CDVarType *type = NULL;
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
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
        c_assert(!r);

        r = c_dvar_new(&var);
        c_assert(!r);

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
        c_assert(!r);

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
        c_assert(u8 == 7);
        c_assert(u32 == 7);
        c_assert(!strcmp(str1, "foo"));
        c_assert(u16 == 7);
        c_assert(!strcmp(str2, "bar"));
        c_assert(u64 == 7);
        c_assert(d > 6.0 && d < 8.0);

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "(yu[{s*}{s<t>}]d)", c_dvar_type_t);

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip example data again */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "*");

        r = c_dvar_end_read(var);
        c_assert(!r);

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
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
        size_t n_data;
        void *data;
        int r;

        /*
         * Simple test that marshals a basic dbus-message type with a fixed
         * field-array and body.
         */

        r = c_dvar_new(&var);
        c_assert(!r);

        c_dvar_begin_write(var, (__BYTE_ORDER == __BIG_ENDIAN), type, 1);

        c_dvar_write(var, "(yyyyuu[", 0, 0, 0, 0, 0, 0);
        c_dvar_write(var, "(y<u>)", 0, c_dvar_type_u, 0);
        c_dvar_write(var, "(y<y>)", 0, c_dvar_type_y, 0);
        c_dvar_write(var, "](st))", "", UINT64_C(0));

        r = c_dvar_end_write(var, &data, &n_data);
        c_assert(!r);

        free(data);
}

static void test_dbus_body(void) {
        static const CDVarType types[] = {
                /* sss */
                C_DVAR_T_INIT(C_DVAR_T_s),
                C_DVAR_T_INIT(C_DVAR_T_s),
                C_DVAR_T_INIT(C_DVAR_T_s),
        };
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
        const char *str;
        size_t n_data;
        void *data;
        int r;

        /*
         * Simple test that marshals a basic dbus-message body with 3 strings.
         */

        r = c_dvar_new(&var);
        c_assert(!r);

        c_dvar_begin_write(var, (__BYTE_ORDER == __BIG_ENDIAN), types, 3);
        c_dvar_write(var, "sss", "fo", "ob", "ar");
        r = c_dvar_end_write(var, &data, &n_data);
        c_assert(!r);

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), types, 3, data, n_data);
        c_dvar_read(var, "s", &str);
        c_assert(!strcmp(str, "fo"));
        c_dvar_read(var, "s", &str);
        c_assert(!strcmp(str, "ob"));
        c_dvar_read(var, "s", &str);
        c_assert(!strcmp(str, "ar"));
        r = c_dvar_end_read(var);
        c_assert(!r);

        free(data);
}

static void test_skip(void) {
        _c_cleanup_(c_dvar_type_freep) CDVarType *type = NULL;
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
        size_t n_data;
        void *data;
        uint64_t t0, t1;
        int r;

        /*
         * A very basic serialization and deserialization test that serves as
         * base-line for reader/writer operation testing.
         *
         * We simply allocate and build a variant of a fixed type, then read it
         * back and verify the data matches.
         */

        r = c_dvar_type_new_from_string(&type, "at");
        c_assert(!r);

        r = c_dvar_new(&var);
        c_assert(!r);

        /* write example data */

        c_dvar_begin_write(var, (__BYTE_ORDER == __BIG_ENDIAN), type, 1);

        c_dvar_write(var, "[tt]", UINT64_C(7), UINT64_C(127));

        r = c_dvar_end_write(var, &data, &n_data);
        c_assert(!r);

        /* read back example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_read(var, "[tt]", &t0, &t1);
        c_assert(t0 == 7);
        c_assert(t1 == 127);

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip example data */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "*");

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip single array member */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "[*t]");

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip both array members */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "[**]");

        r = c_dvar_end_read(var);
        c_assert(!r);

        /* skip past last member */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "[tt*");
        c_assert(c_dvar_get_poison(var) == -ENOTRECOVERABLE);

        /* skip past array */

        c_dvar_begin_read(var, c_dvar_is_big_endian(var), type, 1, data, n_data);

        c_dvar_skip(var, "[tt]*");
        c_assert(c_dvar_get_poison(var) == -ENOTRECOVERABLE);

        c_dvar_end_read(var);
        free(data);
}

static void test_sample0(void) {
        static const CDVarType types[] = {
                /* gs */
                C_DVAR_T_INIT(C_DVAR_T_g),
                C_DVAR_T_INIT(C_DVAR_T_s),
        };
        static const alignas(8) uint8_t data[] = {
                0, 0,
        };
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
        int r;

        /*
         * This sample reads "gs" from a short buffer. It was reported on
         * dbus-broker and used to trigger out-of-bound reads. Verify this is
         * now correctly handled as OUT_OF_BOUND error.
         */

        r = c_dvar_new(&var);
        c_assert(!r);

        c_dvar_begin_read(var, true, types, 2, data, sizeof(data));
        c_dvar_read(var, "gs", NULL, NULL);
        r = c_dvar_end_read(var);
        c_assert(r == C_DVAR_E_OUT_OF_BOUNDS);
}

static void test_sample1(void) {
        static const CDVarType types[] = {
                C_DVAR_T_INIT(
                        /* (yyyyuua(yv)()) */
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
                                C_DVAR_T_TUPLE0
                        )
                ),
        };
        static const alignas(8) uint8_t data[] = {
                0x6c, 0x30, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x21, 0x3a, 0x32, 0x2a, 0xa6, 0x01, 0x00, 0x00,
                0x1b, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76,
                0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0xf7, 0x76, 0x00, 0x6e, 0x74, 0x6e, 0x6e, 0x6e, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x28, 0x61, 0x61, 0x61, 0x0e, 0x93, 0x61,
                0x61, 0x61, 0x67, 0x67, 0x67, 0x6e, 0x6e, 0x6e, 0x6e, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00,
                0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01, 0x76, 0x00, 0x01,
                0x76, 0x00, 0x01, 0x76, 0x00, 0x75, 0x4b, 0x00,
        };
        _c_cleanup_(c_dvar_freep) CDVar *var = NULL;
        int r;

        /*
         * This sample reads a dbus message from a proper buffer. It was
         * reported on dbus-broker and used to trigger ENOTRECOVERABLE due to
         * nested variant-types exceeding the maximum dbus type length.
         * Verify this is now correctly handled as DEPTH_OVERFLOW.
         */

        r = c_dvar_new(&var);
        c_assert(!r);

        c_dvar_begin_read(var, false, types, 1, data, sizeof(data));
        c_dvar_skip(var, "(yyyyuu*())");
        r = c_dvar_end_read(var);
        c_assert(r == C_DVAR_E_DEPTH_OVERFLOW);
}

int main(int argc, char **argv) {
        test_basic_serialization(true);
        test_basic_serialization(false);
        test_dbus_message();
        test_dbus_body();
        test_skip();
        test_sample0();
        test_sample1();
        return 0;
}
