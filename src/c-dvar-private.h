#pragma once

/*
 * Internal Definitions
 *
 * This header supplements the public header with all private symbols and
 * definitions.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "c-dvar.h"

typedef struct CDVarLevel CDVarLevel;

#define _cleanup_(_x) __attribute__((__cleanup__(_x)))
#define _likely_(_x) (__builtin_expect(!!(_x), 1))
#define _public_ __attribute__((__visibility__("default")))
#define _unlikely_(_x) (__builtin_expect(!!(_x), 0))

#define ALIGN_TO(_val, _alignment) ((_val + (_alignment) - 1) & ~((_alignment) - 1))

struct CDVarLevel {
        CDVarType *parent_type;
        CDVarType *i_type;
        uint8_t n_type;
        uint8_t container : 7;
        uint8_t allocated_parent_type : 1;
        size_t i_buffer;
        union {
                /* reader */
                size_t n_buffer;
                /* writer */
                size_t index;
        };
};

struct CDVar {
        uint8_t *data;
        size_t n_data;

        int poison;
        bool ro : 1;
        bool big_endian : 1;

        CDVarLevel *current;
        CDVarLevel levels[C_DVAR_TYPE_DEPTH_MAX + 1];
};

int c_dvar_next_varg(CDVar *var, char c);

static inline void c_dvar_push(CDVar *var) {
        ++var->current;

        var->current->parent_type = (var->current - 1)->parent_type;
        var->current->i_type = (var->current - 1)->i_type + 1;
        var->current->n_type = (var->current - 1)->n_type - 1;
        var->current->container = (var->current - 1)->i_type->element;
        var->current->allocated_parent_type = false;
        var->current->i_buffer = (var->current - 1)->i_buffer;

        if (var->ro)
                var->current->n_buffer = (var->current - 1)->n_buffer;
        else
                var->current->index = 0;
}

static inline void c_dvar_pop(CDVar *var) {
        size_t n;

        if (var->current->allocated_parent_type)
                free(var->current->parent_type);

        --var->current;

        n = (var->current + 1)->i_buffer - var->current->i_buffer;
        var->current->i_buffer += n;

        if (var->ro)
                var->current->n_buffer -= n;
}
