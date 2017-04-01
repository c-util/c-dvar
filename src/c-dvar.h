#pragma once

/*
 * D-Bus Variant Type-System
 *
 * This library implements the D-Bus type-system as a variant type, including
 * marshalling and demarshalling functionality. It is fully implemented in
 * ISO-C11 and has no external dependencies.
 *
 * This type-system strictly adheres to the D-Bus spec, including its
 * limitations in size and depth. Unlike the related GVariant type-system, it
 * is not suitable for use outside of D-Bus. Hence, this implementation does
 * not strive to be universally applicable, but solely meant to be used with
 * D-Bus IPC.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct CDVarType CDVarType;

/**
 * C_DVAR_TYPE_LENGTH_MAX - XXX
 */
#define C_DVAR_TYPE_LENGTH_MAX (255)

/**
 * C_DVAR_TYPE_DEPTH_MAX - XXX
 */
#define C_DVAR_TYPE_DEPTH_MAX (64)

enum {
        _C_DVAR_E_SUCCESS,

        /* type/signature parser */
        C_DVAR_E_OVERLONG_TYPE,
        C_DVAR_E_DEPTH_OVERFLOW,
        C_DVAR_E_INVALID_TYPE,
};

/**
 * struct CDVarType - XXX
 */
struct CDVarType {
        uint32_t size : 11;
        uint32_t alignment : 2;
        uint32_t element : 7;
        uint32_t length : 8;
        uint32_t basic : 1;
};

/* type handling */

int c_dvar_type_new_from_signature(CDVarType **typep, const char *signature, size_t n_signature);
CDVarType *c_dvar_type_free(CDVarType *type);

int c_dvar_type_compare_string(const CDVarType *subject, const char *object, size_t n_object);

/**
 * c_dvar_type_freep() - XXX
 */
static inline void c_dvar_type_freep(CDVarType **type) {
        if (*type)
                c_dvar_type_free(*type);
}

#ifdef __cplusplus
}
#endif
