/*
 * Context Management
 *
 * This file implements the generic management of the variant objects and other
 * library features.
 */

#include <assert.h>
#include <c-stdaux.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "c-dvar.h"
#include "c-dvar-private.h"
#include "c-dvar-type.h"

static_assert(sizeof(CDVarType) == 4, "Unexpected padding in CDVarType");

_c_public_ const CDVarType c_dvar_type_y[] = { C_DVAR_T_INIT(C_DVAR_T_y) };
_c_public_ const CDVarType c_dvar_type_b[] = { C_DVAR_T_INIT(C_DVAR_T_b) };
_c_public_ const CDVarType c_dvar_type_n[] = { C_DVAR_T_INIT(C_DVAR_T_n) };
_c_public_ const CDVarType c_dvar_type_q[] = { C_DVAR_T_INIT(C_DVAR_T_q) };
_c_public_ const CDVarType c_dvar_type_i[] = { C_DVAR_T_INIT(C_DVAR_T_i) };
_c_public_ const CDVarType c_dvar_type_u[] = { C_DVAR_T_INIT(C_DVAR_T_u) };
_c_public_ const CDVarType c_dvar_type_x[] = { C_DVAR_T_INIT(C_DVAR_T_x) };
_c_public_ const CDVarType c_dvar_type_t[] = { C_DVAR_T_INIT(C_DVAR_T_t) };
_c_public_ const CDVarType c_dvar_type_h[] = { C_DVAR_T_INIT(C_DVAR_T_h) };
_c_public_ const CDVarType c_dvar_type_d[] = { C_DVAR_T_INIT(C_DVAR_T_d) };
_c_public_ const CDVarType c_dvar_type_s[] = { C_DVAR_T_INIT(C_DVAR_T_s) };
_c_public_ const CDVarType c_dvar_type_o[] = { C_DVAR_T_INIT(C_DVAR_T_o) };
_c_public_ const CDVarType c_dvar_type_g[] = { C_DVAR_T_INIT(C_DVAR_T_g) };
_c_public_ const CDVarType c_dvar_type_v[] = { C_DVAR_T_INIT(C_DVAR_T_v) };
_c_public_ const CDVarType c_dvar_type_unit[] = { C_DVAR_T_INIT(C_DVAR_T_TUPLE0) };

static const CDVarType *c_dvar_type_builtins[256] = {
        ['y'] = c_dvar_type_y,
        ['b'] = c_dvar_type_b,
        ['n'] = c_dvar_type_n,
        ['q'] = c_dvar_type_q,
        ['i'] = c_dvar_type_i,
        ['u'] = c_dvar_type_u,
        ['x'] = c_dvar_type_x,
        ['t'] = c_dvar_type_t,
        ['h'] = c_dvar_type_h,
        ['d'] = c_dvar_type_d,
        ['s'] = c_dvar_type_s,
        ['o'] = c_dvar_type_o,
        ['g'] = c_dvar_type_g,
        ['v'] = c_dvar_type_v,
};

/**
 * c_dvar_type_new_from_signature() - parse D-Bus type from type signature
 * @typep:              output argument to store allocated type array
 * @signature:          type signature
 * @n_signature:        length of type signature
 *
 * This parses the D-Bus type signature @signature of length @n_signature (it
 * does not require 0-termination) and stores the first fully parsed type in
 * @typep.
 *
 * On success, this fills in a type array in @typep. If @typep is non-NULL, it
 * must point to a pre-allocated array of the correct length (at most
 * @n_signature elements). If @typep is NULL, it is dynamically allocated and
 * the caller is responsible to free it via c_dvar_type_free() once done.
 *
 * In case @typep is non-NULL and pre-allocated by the caller, this function
 * might modify it even if it ends up returning an error code. Hence, the
 * contents of the type array are undefined on failure.
 *
 * This function parses the type signature from the beginning until it fully
 * parsed a single complete type. The remaining bytes of the signature are not
 * looked at. The length of the parsed signature can be retrieved at
 * typep->length on success.
 *
 * Return: <0 on fatal failure,
 *         0 on success,
 *         C_DVAR_E_OVERLONG_TYPE if type exceeds length limits,
 *         C_DVAR_E_DEPTH_OVERFLOW if type exceeds depth limits,
 *         C_DVAR_E_INVALID_TYPE if element constellation is not valid.
 */
_c_public_ int c_dvar_type_new_from_signature(CDVarType **typep, const char *signature, size_t n_signature) {
        _c_cleanup_(c_dvar_type_freep) CDVarType *allocated_type = NULL;
        CDVarType *type, *stack[C_DVAR_TYPE_DEPTH_MAX], *container, *this;
        size_t i, i_container, n_type, depth, depth_tuple;
        const CDVarType *builtin;
        char c;

        /*
         * As a first step, figure out how long the next type in @signature is.
         * This requires iterating the entire type, counting opening/closing
         * brackets. We do this up-front, with only limited type validation.
         * Knowing the final type-size allows pre-allocating @type.
         *
         * Note that empty signatures will be rejected by this. The caller is
         * responsible to check for empty signatures, otherwise you might end
         * up in infinite loops.
         */

        n_type = 0;
        depth = 0;

        do {
                if (_c_unlikely_(n_type >= n_signature || n_type >= C_DVAR_TYPE_LENGTH_MAX))
                        return C_DVAR_E_OVERLONG_TYPE;

                c = signature[n_type++];
                switch (c) {
                case '(':
                case '{':
                        ++depth;
                        break;
                case ')':
                case '}':
                        if (!depth--)
                                return C_DVAR_E_DEPTH_OVERFLOW;
                        break;
                }
        } while (c == 'a' || depth);

        /*
         * Now that we know the type length, we pre-allocate @type. Iterate the
         * type again and parse all required information into the type array.
         *
         * While parsing, @this points at the current position in @type and
         * needs to be filled in. @container points at the parent container of
         * @this (or NULL if root level). @i_container is the offset of
         * @container compared to @type (0 if NULL).
         */

        if (*typep) {
                type = *typep;
        } else {
                allocated_type = malloc(n_type * sizeof(*allocated_type));
                if (!allocated_type)
                        return -ENOMEM;

                type = allocated_type;
        }

        depth = 0;
        depth_tuple = 0;
        container = NULL;

        for (i = 0; i < n_type; ++i) {
                c = signature[i];
                this = type + i;
                builtin = c_dvar_type_builtins[(uint8_t)c];
                i_container = container ? (container - type) : 0;

                /*
                 * In case our surrounding container is a DICT, we need to make
                 * sure that the _first_ following type is basic, and there are
                 * exactly 2 types following.
                 */
                if (container && container->element == '{') {
                        if (i < i_container + 2) {
                                /* first type must be basic */
                                if (_c_unlikely_(!builtin || !builtin->basic))
                                        return C_DVAR_E_INVALID_TYPE;
                        } else if (i == i_container + 2) {
                                /* there must be a second type */
                                if (_c_unlikely_(c == '}'))
                                        return C_DVAR_E_INVALID_TYPE;
                        } else if (i > i_container + 2) {
                                /* DICT is closed after second type */
                                if (_c_unlikely_(c != '}'))
                                        return C_DVAR_E_INVALID_TYPE;
                        }
                }

                switch (c) {
                case '(':
                case '{':
                        ++depth_tuple;
                        /* fallthrough */
                case 'a':
                        ++depth;

                        /*
                         * D-Bus has very clear depth restrictions. Total depth
                         * must not exceed 64 (DEPTH_MAX), combined tuple depth
                         * must not exceed 32, nor can array depth exceed 32.
                         * We verify all three here, even though our
                         * implementation would work with a unified limit.
                         */
                        if (_c_unlikely_(depth > C_DVAR_TYPE_DEPTH_MAX))
                                return C_DVAR_E_DEPTH_OVERFLOW;
                        if (_c_unlikely_(depth_tuple > C_DVAR_TYPE_DEPTH_MAX / 2))
                                return C_DVAR_E_DEPTH_OVERFLOW;
                        if (_c_unlikely_(depth - depth_tuple > C_DVAR_TYPE_DEPTH_MAX / 2))
                                return C_DVAR_E_DEPTH_OVERFLOW;

                        this->size = 0;
                        this->alignment = 2 + (c != 'a');
                        this->element = c;
                        this->length = 1 + (c != 'a');
                        this->basic = 0;

                        /*
                         * We opened a new container type, so continue with the
                         * next character. Skip handling terminal types below.
                         */
                        stack[depth - 1] = this;
                        container = this;
                        continue;

                case '}':
                case ')':
                        if (_c_unlikely_(!container || container->element != ((c == '}') ? '{' : '(')))
                                return C_DVAR_E_INVALID_TYPE;

                        this->size = 0;
                        this->alignment = 0;
                        this->element = c;
                        this->length = 1;
                        this->basic = 0;

                        /*
                         * We closed a container. This is a terminal type,
                         * which will be handled below. Make sure to update
                         * our state accordingly.
                         */
                        this = container;
                        container = --depth ? stack[depth - 1] : NULL;
                        --depth_tuple;
                        break;

                default:
                        if (_c_unlikely_(!builtin))
                                return C_DVAR_E_INVALID_TYPE;

                        /*
                         * This a a builtin type. It is always a terminal type,
                         * and will be handled below. Simply copy the builtin
                         * information into the type array;
                         */
                        *this = *builtin;
                        break;
                }

                /*
                 * At this point we handle terminal types. That means, we know
                 * that a full type was finished here. If the parent container
                 * is an array, it has to be completed here. If the parent
                 * container is a tuple, we need to account for the type.
                 */

                while (container && container->element == 'a') {
                        container->length += this->length;

                        this = container;
                        container = --depth ? stack[depth - 1] : NULL;
                }

                if (container) {
                        if (this->size && (this == container + 1 || container->size)) {
                                container->size = c_align_to((size_t)container->size, 1 << this->alignment);
                                container->size += this->size;
                        } else {
                                container->size = 0;
                        }

                        container->length += this->length;
                }

                /*
                 * If, after handling a terminal type, we are back at the root
                 * level, the type must be complete. Hence, return the
                 * information to the caller.
                 */

                if (!container) {
                        *typep = type;
                        allocated_type = NULL;
                        return 0;
                }
        }

        return C_DVAR_E_INVALID_TYPE;
}

/**
 * c_dvar_type_free() - free type object
 * @type:               type to free, or NULL
 *
 * This deallocates the memory for @type, previously allocated by one of the
 * constructors.
 *
 * If @type is NULL, this is a no-op.
 *
 * Return: NULL is returned.
 */
_c_public_ CDVarType *c_dvar_type_free(CDVarType *type) {
        free(type);
        return NULL;
}

/**
 * c_dvar_type_compare_string() - compare type to string representation
 * @subject:            type to compare, or NULL
 * @object:             string representation to compare against
 * @n_object:           length of @object
 *
 * This compares @subject to the string representation of a valid type, given
 * as @object (with length @n_object). This returns -1, 0, or 1, if @subject
 * orders before, equals, or orders after @object, respectively.
 *
 * Note that no validation of either input is done. This means, if the
 * arguments do not equal, then the string representation might be an invalid
 * type.
 *
 * Note that @subject can be NULL, even though this would not represent a valid
 * type.
 *
 * Return: -1, 0, or 1, if @subject orders before, equals, or orders after
 *         @object, respectively.
 */
_c_public_ int c_dvar_type_compare_string(const CDVarType *subject, const char *object, size_t n_object) {
        static const CDVarType null = {};
        int diff;

        subject = subject ?: &null;

        if (subject->length != n_object)
                return (subject->length > n_object) ? 1 : -1;

        while (n_object-- > 0) {
                diff = subject++->element - *object++;
                if (diff)
                        return (diff > 0) ? 1 : -1 ;
        }

        return 0;
}
