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

#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct CDVar CDVar;
typedef struct CDVarLevel CDVarLevel;
typedef struct CDVarType CDVarType;

/**
 * C_DVAR_TYPE_LENGTH_MAX - Maximum length of a type signature
 *
 * Every valid D-Bus Type has a string representation, called its type
 * signature. The maximum length of such a signature of a single complete type
 * is defined by the D-Bus specification to be 255. The original limit only
 * affects the maximum length of a signature of concatenated types. This,
 * however, implies that a single type has the same limit.
 *
 * No signature of any type can ever exceed this length. There is no intention
 * to every support longer signatures. If bigger types are needed, you better
 * use a non-deprecated serialization like GVariant.
 */
#define C_DVAR_TYPE_LENGTH_MAX (255)

/**
 * C_DVAR_TYPE_DEPTH_MAX - Maximum depth of a type signature
 *
 * Similar to C_DVAR_TYPE_LENGTH_MAX, this limits the complexity of any valid
 * type signature. This limits the maximum depth of containers to 64. It is
 * defined by the D-Bus specification and enforced by this implementation. The
 * specification further restricts the depth among the different container
 * types. See the specification for details.
 *
 * No signature of any type can ever exceed this depth. There is no intention
 * to every support deeper signatures. If needed, you better use a
 * non-deprecated serialization like GVariant.
 */
#define C_DVAR_TYPE_DEPTH_MAX (64)

enum {
        _C_DVAR_E_SUCCESS,

        /* type/signature parser */
        C_DVAR_E_OVERLONG_TYPE,
        C_DVAR_E_DEPTH_OVERFLOW,
        C_DVAR_E_INVALID_TYPE,

        /* variant parser */
        C_DVAR_E_CORRUPT_DATA,
        C_DVAR_E_OUT_OF_BOUNDS,
        C_DVAR_E_TYPE_MISMATCH,
};

/**
 * struct CDVarType - D-Bus Type Information
 * @size:               size in bytes required for the serialization, 0 if dynamic
 * @alignment:          required alignment, given as power of 2
 * @element:            character code of this entry
 * @length:             length of the full type, given as number of entries
 * @basic:              whether this is a basic type
 *
 * Every valid D-Bus Type can be parsed into an array of CDVarType objects,
 * containing detailed information about the type. The length of a parsed
 * CDVarType array is the same as the length of the same type signature. That
 * is, each character code in the type signature is parsed into a CDVarType
 * object, based on its position in the signature.
 *
 * A CDVarType array contains recursive type-information for all sub-types of a
 * valid type signature. For instance, the type array of '{sv}' is found
 * unmodified in the type array of '(ua{sv})' at offset 3.
 */
struct CDVarType {
        uint32_t size : 11;
        uint32_t alignment : 2;
        uint32_t element : 7;
        uint32_t length : 8;
        uint32_t basic : 1;
        uint32_t __padding : 3;
};

/**
 * struct CDVarLevel - D-Bus Variant Level information
 * @parent_types:               type information of the parent signature
 * @i_type:                     current type position in parent signature
 * @n_parent_types:             number of single complete types in @parent_types
 * @n_type:                     remaining length after @i_type
 * @container:                  cached parent container element
 * @allocated_parent_types:     whether @parent_types is owned and allocated
 * @i_buffer:                   current data position
 * @n_buffer:                   remaining length after @i_buffer
 * @index:                      cached container-dependent index
 */
struct CDVarLevel {
        CDVarType *parent_types;
        CDVarType *i_type;
        uint8_t n_parent_types;
        uint8_t n_type;
        uint8_t container : 7;
        uint8_t allocated_parent_types : 1;
        size_t i_buffer;
        union {
                /* reader */
                size_t n_buffer;
                /* writer */
                size_t index;
        };
};

/**
 * struct CDVar - D-Bus Variant
 * @data:               data buffer to parse or write
 * @n_data:             length of @data in bytes
 * @poison:             current object poison error code, or 0
 * @n_root_type:        cached total signature length of the root type
 * @ro:                 object is read-only
 * @big_endian:         data is provided as big-endian
 * @current:            current level position
 * @levels:             container levels
 */
struct CDVar {
        uint8_t *data;
        size_t n_data;

        int poison;
        uint8_t n_root_type;
        bool ro : 1;
        bool big_endian : 1;

        CDVarLevel *current;
        CDVarLevel levels[C_DVAR_TYPE_DEPTH_MAX + 1];
};

#define C_DVAR_INIT { .big_endian = !!(__BYTE_ORDER == __BIG_ENDIAN) }

/* builtin */

#define C_DVAR_TYPE_y (1, 0, 'y', 1, 1)
#define C_DVAR_TYPE_b (4, 2, 'b', 1, 1)
#define C_DVAR_TYPE_n (2, 1, 'n', 1, 1)
#define C_DVAR_TYPE_q (2, 1, 'q', 1, 1)
#define C_DVAR_TYPE_i (4, 2, 'i', 1, 1)
#define C_DVAR_TYPE_u (4, 2, 'u', 1, 1)
#define C_DVAR_TYPE_x (8, 3, 'x', 1, 1)
#define C_DVAR_TYPE_t (8, 3, 't', 1, 1)
#define C_DVAR_TYPE_h (4, 2, 'h', 1, 1)
#define C_DVAR_TYPE_d (8, 3, 'd', 1, 1)
#define C_DVAR_TYPE_s (0, 2, 's', 1, 1)
#define C_DVAR_TYPE_o (0, 2, 'o', 1, 1)
#define C_DVAR_TYPE_g (0, 0, 'g', 1, 1)
#define C_DVAR_TYPE_v (0, 0, 'v', 1, 0)

extern const CDVarType c_dvar_type_y[];
extern const CDVarType c_dvar_type_b[];
extern const CDVarType c_dvar_type_n[];
extern const CDVarType c_dvar_type_q[];
extern const CDVarType c_dvar_type_i[];
extern const CDVarType c_dvar_type_u[];
extern const CDVarType c_dvar_type_x[];
extern const CDVarType c_dvar_type_t[];
extern const CDVarType c_dvar_type_h[];
extern const CDVarType c_dvar_type_d[];
extern const CDVarType c_dvar_type_s[];
extern const CDVarType c_dvar_type_o[];
extern const CDVarType c_dvar_type_g[];
extern const CDVarType c_dvar_type_v[];
extern const CDVarType c_dvar_type_unit[];

/* type handling */

int c_dvar_type_new_from_signature(CDVarType **typep, const char *signature, size_t n_signature);
CDVarType *c_dvar_type_free(CDVarType *type);

int c_dvar_type_compare_string(const CDVarType *subject, const char *object, size_t n_object);

/* variant management */

int c_dvar_new(CDVar **varp);
CDVar *c_dvar_free(CDVar *var);
void c_dvar_init(CDVar *var);
void c_dvar_deinit(CDVar *var);

bool c_dvar_is_big_endian(CDVar *var);
int c_dvar_get_poison(CDVar *var);
void c_dvar_get_data(CDVar *var, void **datap, size_t *n_datap);
void c_dvar_get_root_types(CDVar *var, const CDVarType **typesp, size_t *n_typesp);
void c_dvar_get_parent_types(CDVar *var, const CDVarType **typesp, size_t *n_typesp);

void c_dvar_begin_read(CDVar *var, bool big_endian, const CDVarType *types, size_t n_types, const void *data, size_t n_data);
bool c_dvar_more(CDVar *var);
int c_dvar_vread(CDVar *var, const char *format, va_list args);
int c_dvar_vskip(CDVar *var, const char *format, va_list args);
int c_dvar_end_read(CDVar *var);

bool c_dvar_is_path(const char *string, size_t n_string);

void c_dvar_begin_write(CDVar *var, bool big_endian, const CDVarType *types, size_t n_types);
int c_dvar_vwrite(CDVar *var, const char *format, va_list args);
int c_dvar_end_write(CDVar *var, void **datap, size_t *n_datap);

/* inline helpers */

/**
 * c_dvar_type_new_from_string() - allocate new type information from string
 * @typep:              output argument for newly allocated object
 * @str:                string representation of the type
 *
 * This is similar to c_dvar_type_new_from_signature(), but it fails if @str is
 * not a single complete type. In case @str contains more than a single
 * complete type, C_DVAR_E_INVALID_TYPE is returned.
 *
 * Return: 0 on success, negative error code on fatal failure, positive error
 *         code on parser errors.
 */
static inline int c_dvar_type_new_from_string(CDVarType **typep, const char *str) {
        CDVarType *type = *typep ? : NULL;
        size_t n;
        int r;

        n = strlen(str);
        r = c_dvar_type_new_from_signature(&type, str, n);
        if (r)
                return r;

        if (n != type->length) {
                if (!*typep)
                        c_dvar_type_free(type);
                return C_DVAR_E_INVALID_TYPE;
        }

        *typep = type;
        return 0;

}

/**
 * c_dvar_type_freep() - free type information
 * @type:               type information to free
 *
 * This is the cleanup-helper for c_dvar_type_free().
 */
static inline void c_dvar_type_freep(CDVarType **type) {
        if (*type)
                c_dvar_type_free(*type);
}

/**
 * c_dvar_freep() - free variant
 * @var:                variant to free
 *
 * This is the cleanup-helper for c_dvar_free().
 */
static inline void c_dvar_freep(CDVar **var) {
        if (*var)
                c_dvar_free(*var);
}

/**
 * c_dvar_deinitp() - reset variant
 * @var:                variant to reset
 *
 * This is the cleanup-helper for c_dvar_deinit().
 */
static inline void c_dvar_deinitp(CDVar **var) {
        if (*var)
                c_dvar_deinit(*var);
}

/**
 * c_dvar_read() - read data from variant
 * @var:                variant to operate on
 * @format:             format string
 *
 * This is the va_arg-based equivalent of c_dvar_vread(). See its documentation
 * for details.
 *
 * Return: 0 on success, negative error code on fatal errors, positive error
 *         code on parser failure.
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
 * c_dvar_skip() - skip over data from variant
 * @var:                variant to operate on
 * @format:             format string
 *
 * This is the va_arg-based equivalent of c_dvar_vskip(). See its documentation
 * for details.
 *
 * Return: 0 on success, negative error code on fatal errors, positive error
 *         code on parser failure.
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
 * c_dvar_write() - write data to variant
 * @var:                variant to operate on
 * @format:             format string
 *
 * This is the va_arg-based equivalent of c_dvar_vwrite(). See its
 * documentation for details.
 *
 * Return: 0 on success, negative error code on fatal errors, positive error
 *         code on builder failure.
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
