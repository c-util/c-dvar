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

typedef struct CDVar CDVar;
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

/* variant management */

int c_dvar_new(CDVar **varp);
CDVar *c_dvar_free(CDVar *var);
void c_dvar_reset(CDVar *var);

bool c_dvar_is_big_endian(CDVar *var);
int c_dvar_get_poison(CDVar *var);
void c_dvar_get_data(CDVar *var, void **datap, size_t *n_datap);
const CDVarType *c_dvar_get_root_type(CDVar *var);
const CDVarType *c_dvar_get_parent_type(CDVar *var);

void c_dvar_begin_read(CDVar *var, bool big_endian, const CDVarType *type, const void *data, size_t n_data);
bool c_dvar_more(CDVar *var);
int c_dvar_vread(CDVar *var, const char *format, va_list args);
int c_dvar_vskip(CDVar *var, const char *format, va_list args);
int c_dvar_end_read(CDVar *var);

void c_dvar_begin_write(CDVar *var, const CDVarType *type);
int c_dvar_vwrite(CDVar *var, const char *format, va_list args);
int c_dvar_end_write(CDVar *var, void **datap, size_t *n_datap);

/**
 * c_dvar_type_freep() - XXX
 */
static inline void c_dvar_type_freep(CDVarType **type) {
        if (*type)
                c_dvar_type_free(*type);
}

/**
 * c_dvar_freep() - XXX
 */
static inline void c_dvar_freep(CDVar **var) {
        if (*var)
                c_dvar_free(*var);
}

/**
 * c_dvar_read() - XXX
 */
static inline int c_dvar_read(CDVar *var, const char *format, ...) {
        va_list args;
        int r;

        va_start(args, format);
        r = c_dvar_vread(var, format, args);
        va_end(args);
        return r;
}

/**
 * c_dvar_skip() - XXX
 */
static inline int c_dvar_skip(CDVar *var, const char *format, ...) {
        va_list args;
        int r;

        va_start(args, format);
        r = c_dvar_vskip(var, format, args);
        va_end(args);
        return r;
}

/**
 * c_dvar_write() - XXX
 */
static inline int c_dvar_write(CDVar *var, const char *format, ...) {
        va_list args;
        int r;

        va_start(args, format);
        r = c_dvar_vwrite(var, format, args);
        va_end(args);
        return r;
}

#ifdef __cplusplus
}
#endif
