/*
 * Tests for Public API
 *
 * This test, unlikely the others, is linked against the real, distributed,
 * shared library. Its sole purpose is to test for symbol availability.
 */

#include <assert.h>
#include <endian.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"

static void test_api(void) {
        __attribute__((__cleanup__(c_dvar_type_freep))) CDVarType *type = NULL;
        __attribute__((__unused__)) __attribute__((__cleanup__(c_dvar_deinitp))) CDVar *varp = NULL;
        __attribute__((__cleanup__(c_dvar_deinit))) CDVar var = C_DVAR_INIT;
        __attribute__((__cleanup__(c_dvar_freep))) CDVar *heap_var = NULL;
        static const alignas(8) uint32_t u32 = 7;
        static const CDVarType t = {
                .size = 4,
                .alignment = 2,
                .element = 'u',
                .length = 1,
                .basic = 1,
        };
        uint32_t value;
        size_t n_data;
        void *data;
        int r;

        /* builtin */

        assert(c_dvar_type_y);
        assert(c_dvar_type_b);
        assert(c_dvar_type_n);
        assert(c_dvar_type_q);
        assert(c_dvar_type_i);
        assert(c_dvar_type_u);
        assert(c_dvar_type_x);
        assert(c_dvar_type_t);
        assert(c_dvar_type_h);
        assert(c_dvar_type_d);
        assert(c_dvar_type_s);
        assert(c_dvar_type_o);
        assert(c_dvar_type_g);
        assert(c_dvar_type_v);
        assert(c_dvar_type_unit);

        /* type handling */

        r = c_dvar_type_new_from_signature(&type, NULL, 0);
        assert(r != 0);

        r = c_dvar_type_new_from_string(&type, "");
        assert(r != 0);

        r = c_dvar_type_compare_string(NULL, NULL, 0);
        assert(!r);

        type = c_dvar_type_free(type);

        /* heap-allocated variant */
        r = c_dvar_new(&heap_var);
        assert(!r);

        heap_var = c_dvar_free(heap_var);

        /* variant management */

        c_dvar_init(&var);

        c_dvar_deinit(&var);
        assert(c_dvar_is_big_endian(&var) == (__BYTE_ORDER == __BIG_ENDIAN));
        assert(!c_dvar_get_poison(&var));
        c_dvar_get_data(&var, NULL, NULL);
        c_dvar_get_root_types(&var, NULL, NULL);
        c_dvar_get_parent_types(&var, NULL, NULL);

        c_dvar_deinit(&var);

        /* reader */

        c_dvar_init(&var);

        c_dvar_begin_read(&var, c_dvar_is_big_endian(&var), &t, 1, &u32, sizeof(u32));
        assert(c_dvar_more(&var));
        c_dvar_read(&var, "u", &value);
        c_dvar_skip(&var, "");
        r = c_dvar_end_read(&var);
        assert(r >= 0);
        assert(value == 7);

        c_dvar_deinit(&var);

        /* writer */

        c_dvar_init(&var);

        c_dvar_begin_write(&var, &t, 1);
        c_dvar_write(&var, "u", 0);
        r = c_dvar_end_write(&var, &data, &n_data);
        assert(r >= 0);
        assert(data);
        assert(n_data);
        free(data);

        c_dvar_deinit(&var);
}

int main(int argc, char **argv) {
        test_api();
        return 0;
}
