/*
 * Reader
 *
 * XXX
 */

#include <assert.h>
#include <c-stdaux.h>
#include <c-utf8.h>
#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

static int c_dvar_read_data(CDVar *var, int alignment, const void **datap, size_t n_data) {
        size_t i, align;

        align = c_align_to(var->current->i_buffer, 1 << alignment) - var->current->i_buffer;

        if (_c_unlikely_(var->current->n_buffer < align + n_data))
                return C_DVAR_E_OUT_OF_BOUNDS;

        /*
         * Verify alignment bytes are 0. Needed for compatibility with
         * dbus-daemon.
         */
        for (i = 0; i < align; ++i)
                if (_c_unlikely_(var->data[var->current->i_buffer + i]))
                        return C_DVAR_E_CORRUPT_DATA;

        if (datap)
                *datap = var->data + var->current->i_buffer + align;

        var->current->i_buffer += align + n_data;
        var->current->n_buffer -= align + n_data;
        return 0;
}

static int c_dvar_read_u8(CDVar *var, uint8_t *datap) {
        const void *p;
        int r;

        r = c_dvar_read_data(var, 0, &p, sizeof(*datap));
        if (_c_likely_(r >= 0))
                *datap = *(const uint8_t *)p;

        return r;
}

static int c_dvar_read_u16(CDVar *var, uint16_t *datap) {
        const void *p;
        int r;

        r = c_dvar_read_data(var, 1, &p, sizeof(*datap));
        if (_c_likely_(r >= 0))
                *datap = c_dvar_bswap16(var, *(const uint16_t *)p);

        return r;
}

static int c_dvar_read_u32(CDVar *var, uint32_t *datap) {
        const void *p;
        int r;

        r = c_dvar_read_data(var, 2, &p, sizeof(*datap));
        if (_c_likely_(r >= 0))
                *datap = c_dvar_bswap32(var, *(const uint32_t *)p);

        return r;
}

static int c_dvar_read_u64(CDVar *var, uint64_t *datap) {
        const void *p;
        int r;

        r = c_dvar_read_data(var, 3, &p, sizeof(*datap));
        if (_c_likely_(r >= 0))
                *datap = c_dvar_bswap64(var, *(const uint64_t *)p);

        return r;
}

static int c_dvar_dummy_vread(CDVar *var, const char *format, va_list args) {
        void *p;
        char c;

        while ((c = *format++)) {
                switch (c) {
                case '[':
                case '(':
                case '{':
                case ']':
                case '>':
                case ')':
                case '}':
                        /* no @args required */
                        break;

                case '<':
                        p = va_arg(args, const char **);
                        /* unused *input* argument */
                        break;

                case 'y':
                        p = va_arg(args, uint8_t *);
                        if (p)
                                *(uint8_t *)p = 0;
                        break;

                case 'b':
                        p = va_arg(args, bool *);
                        if (p)
                                *(bool *)p = false;
                        break;

                case 'n':
                case 'q':
                        p = va_arg(args, uint16_t *);
                        if (p)
                                *(uint16_t *)p = 0;
                        break;

                case 'i':
                case 'h':
                case 'u':
                        p = va_arg(args, uint32_t *);
                        if (p)
                                *(uint32_t *)p = 0;
                        break;

                case 'x':
                case 't':
                        p = va_arg(args, uint64_t *);
                        if (p)
                                *(uint64_t *)p = 0;
                        break;

                case 'd':
                        p = va_arg(args, double *);
                        if (p)
                                *(double *)p = 0;
                        break;

                case 's':
                case 'g':
                        p = va_arg(args, const char **);
                        if (p)
                                *(const char **)p = "";
                        break;

                case 'o':
                        p = va_arg(args, const char **);
                        if (p)
                                *(const char **)p = "/";
                        break;

                case 'a':
                case 'v':
                default:
                        /*
                         * Invalid format-codes imply invalid @args, no way
                         * to recover meaningfully.
                         */
                        assert(0);
                        break;
                }
        }

        return -ENOTRECOVERABLE;
}

static int c_dvar_try_vread(CDVar *var, const char *format, va_list args) {
        CDVarType *type;
        const char *str;
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
        size_t i, n;
        void *p;
        char c;
        int r;

        while ((c = *format++)) {
                r = c_dvar_next_varg(var, c);
                if (r)
                        goto error;

                switch (c) {
                case '[':
                        /* read array size */
                        r = c_dvar_read_u32(var, &u32);
                        if (r)
                                goto error;

                        /* align to child-alignment */
                        r = c_dvar_read_data(var, (var->current->i_type + 1)->alignment, NULL, 0);
                        if (r)
                                goto error;

                        /* check space (alignment and size are not counted) */
                        if (u32 > var->current->n_buffer) {
                                r = C_DVAR_E_OUT_OF_BOUNDS;
                                goto error;
                        }

                        c_dvar_push(var);
                        var->current->n_buffer = u32;
                        continue; /* do not advance type iterator */

                case '<':
                        r = c_dvar_read_u8(var, &u8);
                        if (r)
                                goto error;

                        n = u8;
                        r = c_dvar_read_data(var, 0, (const void **)&str, n);
                        if (r)
                                goto error;

                        r = c_dvar_read_u8(var, &u8);
                        if (r)
                                goto error;

                        if (u8 || !c_dvar_is_type(str, n)) {
                                r = C_DVAR_E_CORRUPT_DATA;
                                goto error;
                        }

                        type = p = (CDVarType *)va_arg(args, const CDVarType *);
                        if (!type) {
                                r = c_dvar_type_new_from_signature(&type, str, n);
                                if (r > 0 || (!r && type->length != n))
                                        r = C_DVAR_E_CORRUPT_DATA;
                        } else if (type->length != n) {
                                r = C_DVAR_E_TYPE_MISMATCH;
                        } else {
                                /* verify @type matches @str */
                                r = 0;
                                for (i = 0; i < n; ++i) {
                                        if (type[i].element != str[i]) {
                                                r = C_DVAR_E_TYPE_MISMATCH;
                                                break;
                                        }
                                }
                        }

                        if (r) {
                                if (type != p)
                                        c_dvar_type_free(type);

                                /*
                                 * We fetched va_arg() of the current format
                                 * character. Increment @format, so the
                                 * error-path does the right thing.
                                 */
                                ++format;
                                goto error;
                        }

                        c_dvar_push(var);
                        var->current->parent_types = type;
                        var->current->n_parent_types = 1;
                        var->current->i_type = type;
                        var->current->n_type = type->length;
                        var->current->allocated_parent_types = (type != p);
                        continue; /* do not advance type iterator */

                case '(':
                case '{':
                        /* align to 8 bytes */
                        r = c_dvar_read_data(var, 3, NULL, 0);
                        if (r)
                                goto error;

                        c_dvar_push(var);
                        --var->current->n_type; /* truncate trailing bracket */
                        continue; /* do not advance type iterator */

                case ']':
                        /* trailing padding is not allowed */
                        if (var->current->n_buffer) {
                                r = C_DVAR_E_CORRUPT_DATA;
                                goto error;
                        }

                        c_dvar_pop(var);
                        break;

                case '>':
                case ')':
                case '}':
                        c_dvar_pop(var);
                        break;

                case 'y':
                        r = c_dvar_read_u8(var, &u8);
                        if (r)
                                goto error;

                        p = va_arg(args, uint8_t *);
                        if (p)
                                *(uint8_t *)p = u8;

                        break;

                case 'b':
                        r = c_dvar_read_u32(var, &u32);
                        if (r)
                                goto error;
                        if (u32 != 0 && u32 != 1) {
                                r = C_DVAR_E_CORRUPT_DATA;
                                goto error;
                        }

                        p = va_arg(args, bool *);
                        if (p)
                                *(bool *)p = u32;

                        break;

                case 'n':
                case 'q':
                        r = c_dvar_read_u16(var, &u16);
                        if (r)
                                goto error;

                        p = va_arg(args, uint16_t *);
                        if (p)
                                *(uint16_t *)p = u16;

                        break;

                case 'i':
                case 'h':
                case 'u':
                        r = c_dvar_read_u32(var, &u32);
                        if (r)
                                goto error;

                        p = va_arg(args, uint32_t *);
                        if (p)
                                *(uint32_t *)p = u32;

                        break;

                case 'x':
                case 't':
                case 'd':
                        r = c_dvar_read_u64(var, &u64);
                        if (r)
                                goto error;

                        p = va_arg(args, uint64_t *);
                        if (p)
                                *(uint64_t *)p = u64;

                        break;

                case 's':
                case 'o':
                case 'g':
                        if (c == 'g') {
                                r = c_dvar_read_u8(var, &u8);
                                if (r)
                                        goto error;

                                u32 = u8;
                        } else {
                                r = c_dvar_read_u32(var, &u32);
                                if (r)
                                        goto error;
                        }

                        r = c_dvar_read_data(var, 0, (const void **)&str, u32);
                        if (r)
                                goto error;

                        r = c_dvar_read_u8(var, &u8);
                        if (r)
                                goto error;

                        if (u8 ||
                            (c == 's' && !c_dvar_is_string(str, u32)) ||
                            (c == 'o' && !c_dvar_is_path(str, u32)) ||
                            (c == 'g' && !c_dvar_is_signature(str, u32))) {
                                r = C_DVAR_E_CORRUPT_DATA;
                                goto error;
                        }

                        p = va_arg(args, const char **);
                        if (p)
                                *(const char **)p = str;

                        break;

                default:
                        r = -ENOTRECOVERABLE;
                        goto error;
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

error:
        c_dvar_dummy_vread(var, format - 1, args);
        return r;
}

static int c_dvar_ff(CDVar *var) {
        size_t t, depth = 0;
        char c;
        int r;

        do {
                if (var->current->n_type && (var->current->container != 'a' || c_dvar_more(var))) {
                        c = var->current->i_type->element;
                } else if (depth > 0) {
                        switch (var->current->container) {
                        case 'a':
                                c = ']';
                                break;
                        case 'v':
                                c = '>';
                                break;
                        case '(':
                                c = ')';
                                break;
                        case '{':
                                c = '}';
                                break;
                        default:
                                return -ENOTRECOVERABLE;
                        }
                } else {
                        /*
                         * You cannot fast-forward if you are at the end of a
                         * compound type, or past the last element of an array.
                         * There is nothing to skip.
                         */
                        return -ENOTRECOVERABLE;
                }

                switch (c) {
                case 'a':
                        ++depth;
                        c = '[';
                        break;

                case 'v':
                        ++depth;
                        c = '<';
                        break;

                case '[':
                case '<':
                case '(':
                case '{':
                        ++depth;
                        break;

                case ']':
                case '>':
                case ')':
                case '}':
                        assert(depth > 0);
                        --depth;
                        break;
                case 'y':
                case 'n':
                case 'q':
                case 'i':
                case 'h':
                case 'u':
                case 'x':
                case 't':
                case 'd':
                        /*
                         * If we are skipping an entire array with a fixed size
                         * member, we can jump over all members in one go. Note
                         * that this only works if the member does not need any
                         * validation (this excludes 'b' and any non-basic
                         * type), because callers rely on this function to
                         * check for content validity.
                         */
                        if (depth > 0 && var->current->container == 'a') {
                                t = var->current->n_buffer % var->current->i_type->size;
                                var->current->i_buffer += var->current->n_buffer - t;
                                var->current->n_buffer = t;

                                c = ']';
                                --depth;
                        }
                        break;
                }

                r = c_dvar_read(var, (char [2]){ c, 0 }, NULL);
                if (r)
                        return r;
        } while (depth);

        return 0;
}

static int c_dvar_try_vskip(CDVar *var, const char *format, va_list args) {
        void *p;
        char c;
        int r;

        /*
         * This simply iterates over the format-string @format, but passes NULL
         * to the reader so its value is skipped (but still validated).
         * Additionally, it supports '*' in the format-string, in which case it
         * skips an entire type. This is done by simply skipping whatever is
         * found in the variant (and, again, validating it). This is done
         * recursively, since the D-Bus serialization does not support random
         * access.
         */

        while ((c = *format++)) {
                p = NULL;

                switch (c) {
                case '*':
                        r = c_dvar_ff(var);
                        if (r)
                                return r;

                        break;

                case '<':
                        p = (void *)va_arg(args, const CDVarType *);
                        /* fallthrough */
                default:
                        r = c_dvar_read(var, (char [2]){ c, 0 }, p);
                        if (r)
                                return r;

                        break;
                }
        }

        return 0;
}

/**
 * c_dvar_begin_read() - XXX
 */
_c_public_ void c_dvar_begin_read(CDVar *var, bool big_endian, const CDVarType *types, size_t n_types, const void *data, size_t n_data) {
        size_t i;

        /*
         * D-Bus types are generally composable, unlike its default
         * serialization, which is position dependent. Its required padding
         * changes depending on its initial alignment. Hence, you must pass in
         * 64-bit aligned data, otherwise you will not get a canonical
         * representation.
         *
         * There are exceptions, where you could successfully do composition.
         * However, almost all those exceptions are statically sized types, so
         * you're better off using a hard-coded structure type, anyway. You
         * need deep understanding of the serialization to realize the
         * composition would work, so there is no point in using a CDVar object
         * at all.
         *
         * XXX: We could support a `shifted' reader, that supports parsing
         *      sub-variants. So far we didn't see a need for it, though.
         */
        assert(data == (void *)c_align_to((unsigned long)data, 8));

        c_dvar_deinit(var);

        var->data = (void *)data;
        var->n_data = n_data;
        var->ro = true;
        var->big_endian = big_endian;

        var->current = var->levels;
        var->current->parent_types = (CDVarType *)types;
        var->current->n_parent_types = n_types;
        var->current->i_type = (CDVarType *)types;
        var->current->n_type = 0;
        var->current->container = 0;
        var->current->allocated_parent_types = false;
        var->current->i_buffer = 0;
        var->current->n_buffer = var->n_data;

        for (i = 0; i < n_types; ++i) {
                assert(var->n_root_type + types->length >= types->length);
                var->n_root_type += types->length;
                types += types->length;
        }

        var->current->n_type = var->n_root_type;
}

/**
 * c_dvar_more() - XXX
 */
_c_public_ bool c_dvar_more(CDVar *var) {
        return !var->poison && var->ro && var->current && var->current->n_buffer;
}

/**
 * c_dvar_vread() - XXX
 */
_c_public_ int c_dvar_vread(CDVar *var, const char *format, va_list args) {
        assert(var->ro);
        assert(var->current);

        if (_c_unlikely_(var->poison))
                c_dvar_dummy_vread(var, format, args);
        else
                var->poison = c_dvar_try_vread(var, format, args);

        return var->poison;
}

/**
 * c_dvar_vskip() - XXX
 */
_c_public_ int c_dvar_vskip(CDVar *var, const char *format, va_list args) {
        assert(var->ro);
        assert(var->current);

        if (_c_unlikely_(var->poison))
                return var->poison;

        return var->poison = c_dvar_try_vskip(var, format, args);
}

/**
 * c_dvar_end_read() - XXX
 */
_c_public_ int c_dvar_end_read(CDVar *var) {
        int r;

        assert(var->ro);
        assert(var->current);

        if (_c_unlikely_(var->poison))
                r = var->poison;
        else if (_c_unlikely_(var->current != var->levels || var->current->n_type))
                r = -ENOTRECOVERABLE;
        else if (_c_unlikely_(var->current->n_buffer))
                r = C_DVAR_E_CORRUPT_DATA;
        else
                r = 0;

        c_dvar_rewind(var);
        var->current->i_type = var->current->parent_types;
        var->current->n_type = var->n_root_type;
        var->current->i_buffer = 0;
        var->current->n_buffer = var->n_data;

        return r;
}

/**
 * c_dvar_is_path() - XXX
 */
_c_public_ bool c_dvar_is_path(const char *str, size_t len) {
        bool slash = true;
        size_t i;

        if (_c_unlikely_(len == 0 || *str != '/'))
                return false;

        for (i = 1; i < len; ++i) {
                if (str[i] == '/') {
                        if (_c_unlikely_(slash))
                                return false;

                        slash = true;
                } else if (_c_unlikely_(!((str[i] >= 'A' && str[i] <= 'Z') ||
                                          (str[i] >= 'a' && str[i] <= 'z') ||
                                          (str[i] >= '0' && str[i] <= '9') ||
                                          (str[i] == '_')))) {
                        return false;
                } else {
                        slash = false;
                }
        }

        return !slash || len == 1;
}

bool c_dvar_is_string(const char *str, size_t len) {
        c_utf8_verify(&str, &len);

        return (*str == '\0' && len == 0);
}

static const char *c_dvar_verify_type(const char *string, size_t n_string) {
        size_t i, depth = 0, n_tuple = 0;
        uint64_t tuples = 0, arrays = 0;
        char c, container = 0;

        static_assert(C_DVAR_TYPE_DEPTH_MAX <= 64, "Invalid maximum type depth");

        for (i = 0; i < n_string; ++i) {
                c = string[i];

                if (i >= 255)
                        return NULL;

                if (container == '{') {
                        if (string[i - 1] == '{') {
                                /* first type must be basic */
                                if (_c_unlikely_(!(c == 'y' || c == 'b' || c == 'n' || c == 'q' ||
                                                   c == 'i' || c == 'u' || c == 'x' || c == 't' ||
                                                   c == 'h' || c == 'd' || c == 's' || c == 'o' ||
                                                   c == 'g')))
                                        return NULL;
                        } else if (string[i - 2] == '{') {
                                /* there must be a second type */
                                if (_c_unlikely_(c == '}'))
                                        return NULL;
                        } else {
                                /* DICT is closed after second type */
                                if (_c_unlikely_(c != '}'))
                                        return NULL;
                        }
                }

                switch (c) {
                case '{':
                        /* dicts are not allowed outside arrays */
                        if (container != 'a')
                                return NULL;

                        /* fallthrough */
                case '(':
                        ++n_tuple;

                        /* fallthrough */
                case 'a':
                        ++depth;
                        if (_c_unlikely_(depth > C_DVAR_TYPE_DEPTH_MAX ||
                                         n_tuple > C_DVAR_TYPE_DEPTH_MAX / 2 ||
                                         depth - n_tuple > C_DVAR_TYPE_DEPTH_MAX / 2))
                                return NULL;

                        if (c == '{')
                                tuples |= UINT64_C(1) << (depth - 1);
                        else if (c == 'a')
                                arrays |= UINT64_C(1) << (depth - 1);

                        container = c;
                        continue;

                case '}':
                case ')':
                        /* closing braces must match opening ones */
                        if (_c_unlikely_(container != ((c == '}') ? '{' : '(')))
                                return NULL;
                        /* empty structs are not allowed */
                        if (_c_unlikely_(string[i - 1] == '('))
                                return NULL;

                        --n_tuple;
                        tuples &= ~(UINT64_C(1) << --depth);

                        if (depth == 0)
                                container = 0;
                        else if (arrays & (UINT64_C(1) << (depth - 1)))
                                container = 'a';
                        else if (tuples & (UINT64_C(1) << (depth - 1)))
                                container = '{';
                        else
                                container = '(';

                        break;

                case 'y':
                case 'b':
                case 'n':
                case 'q':
                case 'i':
                case 'u':
                case 'x':
                case 't':
                case 'h':
                case 'd':
                case 's':
                case 'o':
                case 'g':
                case 'v':
                        break;

                default:
                        return NULL;
                }

                while (container == 'a') {
                        arrays &= ~(UINT64_C(1) << --depth);

                        if (depth == 0)
                                container = 0;
                        else if (arrays & (UINT64_C(1) << (depth - 1)))
                                container = 'a';
                        else if (tuples & (UINT64_C(1) << (depth - 1)))
                                container = '{';
                        else
                                container = '(';
                }

                if (!container)
                        return string + i + 1;
        }

        return NULL;
}

bool c_dvar_is_signature(const char *string, size_t n_string) {
        const char *next;

        if (_c_unlikely_(n_string > 255))
                return false;

        while (n_string) {
                next = c_dvar_verify_type(string, n_string);
                if (!next)
                        return false;

                n_string -= next - string;
                string = next;
        }

        return true;
}

bool c_dvar_is_type(const char *string, size_t n_string) {
        return string + n_string == c_dvar_verify_type(string, n_string);
}
