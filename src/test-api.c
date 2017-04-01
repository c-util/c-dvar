/*
 * Tests for Public API
 *
 * This test, unlikely the others, is linked against the real, distributed,
 * shared library. Its sole purpose is to test for symbol availability.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"

static void test_api(void) {
        __attribute__((__cleanup__(c_dvar_type_freep))) CDVarType *type = NULL;
        int r;

        r = c_dvar_type_new_from_signature(&type, NULL, 0);
        assert(r != 0);

        r = c_dvar_type_compare_string(NULL, NULL, 0);
        assert(!r);

        type = c_dvar_type_free(type);
}

int main(int argc, char **argv) {
        test_api();
        return 0;
}
