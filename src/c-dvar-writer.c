/*
 * Writer
 *
 * XXX
 */

#include <assert.h>
#include <c-stdaux.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

static int c_dvar_write_data(CDVar *var, int alignment, const void *data, size_t n_data) {
        size_t n, align;
        int shift;
        void *p;

        align = c_align_to(var->current->i_buffer, 1 << alignment) - var->current->i_buffer;

        if (_c_unlikely_(var->n_data - var->current->i_buffer < align + n_data)) {
                n = var->current->i_buffer + align + n_data;
                if (n <= 4096) {
                        /* use page-size as base */
                        n = 4096;
                } else {
                        /* align to next power of 2 */
                        shift = sizeof(unsigned long long) * 8;
                        shift -= __builtin_clzll(n - 1);
                        n = (unsigned long long)1 << shift;
                }

                p = realloc(var->data, n);
                if (!p)
                        return -ENOMEM;

                var->data = p;
                var->n_data = n;
        }

        memset(var->data + var->current->i_buffer, 0, align);
        if (data)
                memcpy(var->data + var->current->i_buffer + align, data, n_data);

        var->current->i_buffer += align + n_data;
        return 0;
}

static int c_dvar_write_u8(CDVar *var, uint8_t v) {
        return c_dvar_write_data(var, 0, &v, sizeof(v));
}

static int c_dvar_write_u16(CDVar *var, uint16_t v) {
        v = c_dvar_bswap16(var, v);
        return c_dvar_write_data(var, 1, &v, sizeof(v));
}

static int c_dvar_write_u32(CDVar *var, uint32_t v) {
        v = c_dvar_bswap32(var, v);
        return c_dvar_write_data(var, 2, &v, sizeof(v));
}

static int c_dvar_write_u64(CDVar *var, uint64_t v) {
        v = c_dvar_bswap64(var, v);
        return c_dvar_write_data(var, 3, &v, sizeof(v));
}

static int c_dvar_try_vwrite(CDVar *var, const char *format, va_list args) {
        const CDVarType *type;
        const char *str;
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
        double fp;
        size_t n;
        char c;
        int r;

        while ((c = *format++)) {
                r = c_dvar_next_varg(var, c);
                if (r)
                        return r;

                switch (c) {
                case '[':
                        /* write and remember placeholder for array size */
                        r = c_dvar_write_u32(var, 0);
                        if (r)
                                return r;

                        var->current->index = var->current->i_buffer - 4;

                        /* all arrays contain alignment to enclosed type */
                        r = c_dvar_write_data(var, (var->current->i_type + 1)->alignment, NULL, 0);
                        if (r)
                                return r;

                        c_dvar_push(var);
                        continue; /* do not advance type iterator */

                case '<':
                        type = va_arg(args, const CDVarType *);

                        r = c_dvar_write_u8(var, type->length);
                        if (r)
                                return r;

                        r = c_dvar_write_data(var, 0, NULL, type->length + 1);
                        if (r)
                                return r;

                        for (n = 0; n < type->length; ++n)
                                var->data[var->current->i_buffer - type->length - 1 + n] = type[n].element;
                        var->data[var->current->i_buffer - 1] = 0;

                        c_dvar_push(var);
                        var->current->parent_types = (CDVarType *)type;
                        var->current->n_parent_types = 1;
                        var->current->i_type = (CDVarType *)type;
                        var->current->n_type = type->length;
                        continue; /* do not advance type iterator */

                case '(':
                case '{':
                        /* align to 64-bit */
                        r = c_dvar_write_data(var, 3, NULL, 0);
                        if (r)
                                return r;

                        c_dvar_push(var);
                        --var->current->n_type; /* truncate trailing bracket */
                        continue; /* do not advance type iterator */

                case ']':
                        /* compute array size */
                        u32 = c_dvar_bswap32(var, var->current->i_buffer - (var->current - 1)->i_buffer);
                        /* write previously written placeholder */
                        *(uint32_t *)&var->data[(var->current - 1)->index] = u32;

                        c_dvar_pop(var);
                        break;

                case '>':
                case ')':
                case '}':
                        c_dvar_pop(var);
                        break;

                case 'y':
                        u8 = va_arg(args, int);
                        r = c_dvar_write_u8(var, u8);
                        if (r)
                                return r;

                        break;

                case 'b':
                        u32 = va_arg(args, int);
                        r = c_dvar_write_u32(var, !!u32);
                        if (r)
                                return r;

                        break;

                case 'n':
                case 'q':
                        u16 = va_arg(args, int);
                        r = c_dvar_write_u16(var, u16);
                        if (r)
                                return r;

                        break;

                case 'i':
                case 'h':
                case 'u':
                        u32 = va_arg(args, uint32_t);
                        r = c_dvar_write_u32(var, u32);
                        if (r)
                                return r;

                        break;

                case 'x':
                case 't':
                        u64 = va_arg(args, uint64_t);
                        r = c_dvar_write_u64(var, u64);
                        if (r)
                                return r;

                        break;

                case 'd': {
                        static_assert(sizeof(double) == sizeof(uint64_t),
                                      "Unsupported size of 'double'");

                        /*
                         * va_arg(double) may use floating point registers,
                         * so must be explicitly retrieved as double. We then
                         * copy into u64 to avoid aliasing restrictions.
                         */
                        fp = va_arg(args, double);
                        memcpy(&u64, &fp, sizeof(fp));

                        r = c_dvar_write_u64(var, u64);
                        if (r)
                                return r;

                        break;
                }

                case 's':
                case 'o':
                        str = va_arg(args, const char *);
                        n = strlen(str);
                        if (_c_unlikely_(n > UINT32_MAX))
                                return -ENOTRECOVERABLE;

                        r = c_dvar_write_u32(var, n);
                        if (r)
                                return r;

                        r = c_dvar_write_data(var, 0, str, n + 1);
                        if (r)
                                return r;

                        break;

                case 'g':
                        str = va_arg(args, const char *);
                        n = strlen(str);
                        if (_c_unlikely_(n > UINT8_MAX))
                                return -ENOTRECOVERABLE;

                        r = c_dvar_write_u8(var, n);
                        if (r)
                                return r;

                        r = c_dvar_write_data(var, 0, str, n + 1);
                        if (r)
                                return r;

                        break;

                default:
                        return -ENOTRECOVERABLE;
                }

                /*
                 * At this point we handled a terminal type. We must advance
                 * the type-iterator so we can continue with the next type. In
                 * case of arrays, this does not happen, though, since there we
                 * continously write the same type, until explicitly terminated
                 * by the caller.
                 */
                if (var->current->container != 'a') {
                        var->current->n_type -= var->current->i_type->length;
                        var->current->i_type += var->current->i_type->length;
                }
        }

        return 0;
}

/**
 * c_dvar_begin_write() - XXX
 */
_c_public_ void c_dvar_begin_write(CDVar *var, bool big_endian, const CDVarType *types, size_t n_types) {
        size_t i;

        c_dvar_deinit(var);

        var->big_endian = big_endian;

        var->current = var->levels;
        var->current->parent_types = (CDVarType *)types;
        var->current->n_parent_types = n_types;
        var->current->i_type = (CDVarType *)types;
        var->current->n_type = 0;
        var->current->container = 0;
        var->current->allocated_parent_types = false;
        var->current->i_buffer = 0;
        var->current->index = 0;

        for (i = 0; i < n_types; ++i) {
                c_assert(var->n_root_type + types->length >= types->length);
                var->n_root_type += types->length;
                types += types->length;
        }

        var->current->n_type = var->n_root_type;
}

/**
 * c_dvar_vwrite() - XXX
 */
_c_public_ int c_dvar_vwrite(CDVar *var, const char *format, va_list args) {
        c_assert(!var->ro);
        c_assert(var->current);

        if (_c_unlikely_(var->poison))
                return var->poison;

        return var->poison = c_dvar_try_vwrite(var, format, args);
}

/**
 * c_dvar_end_write() - XXX
 */
_c_public_ int c_dvar_end_write(CDVar *var, void **datap, size_t *n_datap) {
        int r;

        c_assert(!var->ro);
        c_assert(var->current);

        if (_c_unlikely_(var->poison)) {
                free(var->data);
                r = var->poison;
        } else if (_c_unlikely_(var->current != var->levels || var->current->n_type)) {
                free(var->data);
                r = C_DVAR_E_CORRUPT_DATA;
        } else {
                *datap = var->data;
                *n_datap = var->current->i_buffer;
                r = 0;
        }

        var->data = NULL;
        var->n_data = 0;

        c_dvar_rewind(var);
        var->current->i_type = var->current->parent_types;
        var->current->n_type = var->n_root_type;
        var->current->i_buffer = 0;
        var->current->index = 0;

        return r;
}
