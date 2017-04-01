/*
 * Context Management
 *
 * This file implements the generic management of the variant objects and other
 * library features.
 */

#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

/**
 * c_dvar_new() - XXX
 */
_public_ int c_dvar_new(CDVar **varp) {
        _cleanup_(c_dvar_freep) CDVar *var = NULL;

        var = malloc(sizeof(*var));
        if (!var)
                return -ENOMEM;

        var->data = NULL;
        var->n_data = 0;
        var->poison = 0;
        var->ro = false;
        var->big_endian = !!(__BYTE_ORDER == __BIG_ENDIAN);
        var->current = NULL;

        *varp = var;
        var = NULL;
        return 0;
}

/**
 * c_dvar_free() - XXX
 */
_public_ CDVar *c_dvar_free(CDVar *var) {
        if (!var)
                return NULL;

        c_dvar_reset(var);
        free(var);

        return NULL;
}

/**
 * c_dvar_reset() - reset variant to initial state
 * @var:                variant to reset
 *
 * This resets the given variant to its initial state. Any allocated data is
 * released. All pointers to external resources are cleared and any state is
 * reset to initial values.
 */
_public_ void c_dvar_reset(CDVar *var) {
        if (var->current) {
                for ( ; var->current > var->levels; --var->current)
                        if (var->current->allocated_parent_type)
                                free(var->current->parent_type);

                /* root-level type is always caller-owned */
                assert(!var->current->allocated_parent_type);
        }

        if (!var->ro)
                free(var->data);

        var->data = NULL;
        var->n_data = 0;
        var->poison = 0;
        var->ro = false;
        var->big_endian = !!(__BYTE_ORDER == __BIG_ENDIAN);
        var->current = NULL;
}

/**
 * c_dvar_is_big_endian() - XXX
 */
_public_ bool c_dvar_is_big_endian(CDVar *var) {
        return var->big_endian;
}

/**
 * c_dvar_get_poison() - XXX
 */
_public_ int c_dvar_get_poison(CDVar *var) {
        return var->poison;
}

/**
 * c_dvar_get_data() - XXX
 */
_public_ void c_dvar_get_data(CDVar *var, void **datap, size_t *n_datap) {
        if (datap)
                *datap = var->data;
        if (n_datap)
                *n_datap = var->n_data;
}

/**
 * c_dvar_get_root_type() - XXX
 */
_public_ const CDVarType *c_dvar_get_root_type(CDVar *var) {
        return var->current ? var->levels[0].parent_type : NULL;
}

/**
 * c_dvar_get_parent_type() - XXX
 */
_public_ const CDVarType *c_dvar_get_parent_type(CDVar *var) {
        return var->current ? var->current->parent_type : NULL;
}
