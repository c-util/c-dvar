/*
 * Tests for Public API
 *
 * This test, unlikely the others, is linked against the real, distributed,
 * shared library. Its sole purpose is to test for symbol availability.
 */

#include <assert.h>
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"

static void test_api(void) {
        __attribute__((__cleanup__(c_dvar_type_freep))) CDVarType *type = NULL;
        __attribute__((__cleanup__(c_dvar_freep))) CDVar *var = NULL;
        int r;

        /* type handling */

        r = c_dvar_type_new_from_signature(&type, NULL, 0);
        assert(r != 0);

        r = c_dvar_type_compare_string(NULL, NULL, 0);
        assert(!r);

        type = c_dvar_type_free(type);

        /* variant management */

        r = c_dvar_new(&var);
        assert(!r);

        c_dvar_reset(var);
        assert(c_dvar_is_big_endian(var) == (__BYTE_ORDER == __BIG_ENDIAN));
        assert(!c_dvar_get_poison(var));
        c_dvar_get_data(var, NULL, NULL);
        assert(!c_dvar_get_root_type(var));
        assert(!c_dvar_get_parent_type(var));

        var = c_dvar_free(var);
}

int main(int argc, char **argv) {
        test_api();
        return 0;
}
