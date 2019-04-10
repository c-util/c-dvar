#pragma once

/*
 * D-Bus Variant Compile-time Type Compilation
 *
 * This header provides pre-processor macros to compile types into a CDVarType
 * array at compile-time. That is, it performs the same task as
 * c_dvar_type_new_from_string(), but at compile time. Furthermore, it can be
 * used to access all kinds of type information, as long as the type-definition
 * is compile-time constant.
 *
 * Unfortunately, it is not possible to tokenize string literals via the
 * pre-processor. Hence, type definitions must be given explicit, rather than
 * as type-string. For instance, the type "(yqut)" can be created via:
 *
 *     C_DVAR_T_TUPLE4(
 *             C_DVAR_T_y,
 *             C_DVAR_T_q,
 *             C_DVAR_T_u,
 *             C_DVAR_T_t
 *     )
 *
 * For "better" readability, this file uses a bunch of internal macros. Those
 * are prefixed with C_DVAR_TI*_. Do not use them directly! All macros that are
 * API are exposed with the prefix C_DVAR_T_*().
 *
 * We represent a type as a pre-processor tuple, containing the following
 * elements:
 *
 *     1) fixed-size type
 *     2) alignment
 *     3) type string
 *     4) type length
 *     5) basic type
 *     6) array initializer
 *
 * IOW, the tuple contains all the fields of the CDVarType structure, plus an
 * initializer to create such a type. Note that the 'element' field is replaced
 * with the type-string, since that is more useful.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <c-dvar.h>

/*
 * Strip paranthesis from a tuple.
 *
 * Usage:
 *         C_DVAR_TI_EXPAND((A, B, C, D, E, F)) -> A, B, C, D, E, F
 */
#define C_DVAR_TI_EXPAND(_x) C_DVAR_TII_EXPAND _x
#define C_DVAR_TII_EXPAND(...) __VA_ARGS__

/*
 * Evaluate and then concatenate two symbols.
 *
 * Usage:
 *         C_DVAR_TI_CONCATENATE(FOOBAR_, __COUNTER__) -> FOOBAR_5
 */
#define C_DVAR_TI_CONCATENATE(_a, _b) C_DVAR_TII_CONCATENATE(_a, _b)
#define C_DVAR_TII_CONCATENATE(_a, _b) _a ## _b

/*
 * Select a specific tuple member. We do not assert on the tuple size, but
 * rather allow any finite tuple to be used.
 *
 * Usage:
 *         C_DVAR_TI_SELECT(3, (A, B, C, D, E, F)) -> C
 */
#define C_DVAR_TI_SELECT_1(_1, ...) _1
#define C_DVAR_TI_SELECT_2(_1, _2, ...) _2
#define C_DVAR_TI_SELECT_3(_1, _2, _3, ...) _3
#define C_DVAR_TI_SELECT_4(_1, _2, _3, _4, ...) _4
#define C_DVAR_TI_SELECT_5(_1, _2, _3, _4, _5, ...) _5
#define C_DVAR_TI_SELECT_6(_1, _2, _3, _4, _5, _6, ...) _6
#define C_DVAR_TI_SELECT(_which, _set) C_DVAR_TI_CONCATENATE(C_DVAR_TI_SELECT_, _which) _set

/*
 * Create a pre-processor type for builtins.
 *
 * Usage:
 *         C_DVAR_TI_BUILTIN(u) -> Type-tuple for builtin type 'u'
 */
#define C_DVAR_TI_BUILTIN(_char) C_DVAR_TII_BUILTIN(C_DVAR_TI_CONCATENATE(C_DVAR_TYPE_, _char), _char)
#define C_DVAR_TII_BUILTIN(_set, _char) (                               \
                C_DVAR_TI_SELECT(1, _set),                              \
                C_DVAR_TI_SELECT(2, _set),                              \
                # _char,                                                \
                C_DVAR_TI_SELECT(4, _set),                              \
                C_DVAR_TI_SELECT(5, _set),                              \
                ({                                                      \
                        .size           = C_DVAR_TI_SELECT(1, _set),    \
                        .alignment      = C_DVAR_TI_SELECT(2, _set),    \
                        .element        = C_DVAR_TI_SELECT(3, _set),    \
                        .length         = C_DVAR_TI_SELECT(4, _set),    \
                        .basic          = C_DVAR_TI_SELECT(5, _set),    \
                })                                                      \
        )

/*
 * Create array-type given its embedded type.
 *
 * Usage:
 *         C_DVAR_TI_ARRAY(C_DVAR_T_u) -> Type: au
 */
#define C_DVAR_TI_ARRAY(_type) (                                                        \
                0,                                                                      \
                2,                                                                      \
                "a" C_DVAR_TI_SELECT(3, _type),                                         \
                C_DVAR_TI_SELECT(4, _type) + 1,                                         \
                0,                                                                      \
                (                                                                       \
                        {                                                               \
                                .size           = 0,                                    \
                                .alignment      = 2,                                    \
                                .element        = 'a',                                  \
                                .length         = C_DVAR_TI_SELECT(4, _type) + 1,       \
                                .basic          = 0,                                    \
                        },                                                              \
                        C_DVAR_TI_EXPAND(C_DVAR_TI_SELECT(6, _type))                    \
                )                                                                       \
        )

/*
 * Calculate tuple-size, suitable as callback for fold().
 *
 * Usage:
 *         C_DVAR_TI_TUPLE_SIZE(7, C_DVAR_T_u) -> 12
 */
#define C_DVAR_TI_TUPLE_SIZE(_size, _next) (                                            \
                (                                                                       \
                        (                                                               \
                                (                                                       \
                                        (_size) +                                       \
                                        (1 << C_DVAR_TI_SELECT(2, _next)) -             \
                                        1                                               \
                                ) &                                                     \
                                ~(                                                      \
                                        (1 << C_DVAR_TI_SELECT(2, _next)) -             \
                                        1                                               \
                                )                                                       \
                        ) +                                                             \
                        C_DVAR_TI_SELECT(1, _next)                                      \
                ) *                                                                     \
                !!(_size) *                                                             \
                !!C_DVAR_TI_SELECT(1, _next)                                            \
        )

/*
 * Append another type to a tuple.
 *
 * Usage:
 *         C_DVAR_TI_TUPLE_APPEND(MY_TYPE, C_DVAR_T_u)
 */
#define C_DVAR_TI_TUPLE_APPEND(_set, _next) (                                           \
                C_DVAR_TI_TUPLE_SIZE(                                                   \
                        C_DVAR_TI_SELECT(1, _set),                                      \
                        _next                                                           \
                ),                                                                      \
                0,                                                                      \
                C_DVAR_TI_SELECT(3, _set) C_DVAR_TI_SELECT(3, _next),                   \
                C_DVAR_TI_SELECT(4, _set) + C_DVAR_TI_SELECT(4, _next),                 \
                0,                                                                      \
                (                                                                       \
                        C_DVAR_TI_EXPAND(C_DVAR_TI_SELECT(6, _set)),                    \
                        C_DVAR_TI_EXPAND(C_DVAR_TI_SELECT(6, _next))                    \
                )                                                                       \
        )

/*
 * Closure for a dynamic tuple. Any tuple can be created with nested
 * C_DVAR_TI_TUPLE_APPEND. However, those calls must be wrapped in a
 * C_DVAR_TI_TUPLE invocation to add tuple closure state.
 *
 * Usage:
 *         C_DVAR_TI_TUPLE(
 *                 C_DVAR_TI_TUPLE_APPEND(
 *                         C_DVAR_TI_TUPLE_APPEND(
 *                                 C_DVAR_T_y,
 *                                 C_DVAR_T_q
 *                         ),
 *                         C_DVAR_T_u
 *                 ),
 *                 '(', "(", ')', ")"
 *         ) -> Type: (yqu)
 */
#define C_DVAR_TI_TUPLE(_set, _open1, _open2, _close1, _close2) (                       \
                C_DVAR_TI_SELECT(1, _set),                                              \
                3,                                                                      \
                _open2 C_DVAR_TI_SELECT(3, _set) _close2,                               \
                C_DVAR_TI_SELECT(4, _set) + 2,                                          \
                0,                                                                      \
                (                                                                       \
                        {                                                               \
                                .size           = C_DVAR_TI_SELECT(1, _set),            \
                                .alignment      = 3,                                    \
                                .element        = _open1,                               \
                                .length         = C_DVAR_TI_SELECT(4, _set) + 2,        \
                                .basic          = 0,                                    \
                        },                                                              \
                        C_DVAR_TI_EXPAND(C_DVAR_TI_SELECT(6, _set)),                    \
                        {                                                               \
                                .size           = 0,                                    \
                                .alignment      = 0,                                    \
                                .element        = _close1,                              \
                                .length         = 1,                                    \
                                .basic          = 0,                                    \
                        }                                                               \
                )                                                                       \
        )

/**
 * C_DVAR_T_SIZE(): Return fixed size of a type
 * C_DVAR_T_ALIGNMENT(): Return alignment of a type
 * C_DVAR_T_SIGNATURE(): Return signature of a type
 * C_DVAR_T_LENGTH(): Return length of a type
 * C_DVAR_T_BASIC(): Return whether a type is basic
 * C_DVAR_T_INIT(): Return CDVarType initializer of a type
 */
#define C_DVAR_T_SIZE(_type)            C_DVAR_TI_SELECT(1, _type)
#define C_DVAR_T_ALIGNMENT(_type)       C_DVAR_TI_SELECT(2, _type)
#define C_DVAR_T_SIGNATURE(_type)       C_DVAR_TI_SELECT(3, _type)
#define C_DVAR_T_LENGTH(_type)          C_DVAR_TI_SELECT(4, _type)
#define C_DVAR_T_BASIC(_type)           C_DVAR_TI_SELECT(5, _type)
#define C_DVAR_T_INIT(_type)            C_DVAR_TI_EXPAND(C_DVAR_TI_SELECT(6, _type))

/**
 * C_DVAR_T_?: Builtin types
 *
 * This macro exists for all single-character builtin types, and other common
 * compound types. It evaluates to their pre-processor representation. Use the
 * type-accessors to get access to specific fields.
 */
#define C_DVAR_T_y C_DVAR_TI_BUILTIN(y)
#define C_DVAR_T_b C_DVAR_TI_BUILTIN(b)
#define C_DVAR_T_n C_DVAR_TI_BUILTIN(n)
#define C_DVAR_T_q C_DVAR_TI_BUILTIN(q)
#define C_DVAR_T_i C_DVAR_TI_BUILTIN(i)
#define C_DVAR_T_u C_DVAR_TI_BUILTIN(u)
#define C_DVAR_T_x C_DVAR_TI_BUILTIN(x)
#define C_DVAR_T_t C_DVAR_TI_BUILTIN(t)
#define C_DVAR_T_h C_DVAR_TI_BUILTIN(h)
#define C_DVAR_T_d C_DVAR_TI_BUILTIN(d)
#define C_DVAR_T_s C_DVAR_TI_BUILTIN(s)
#define C_DVAR_T_o C_DVAR_TI_BUILTIN(o)
#define C_DVAR_T_g C_DVAR_TI_BUILTIN(g)
#define C_DVAR_T_v C_DVAR_TI_BUILTIN(v)

/**
 * C_DVAR_T_ARRAY(): XXX
 */
#define C_DVAR_T_ARRAY(_type) \
        C_DVAR_TI_ARRAY(_type)

/**
 * C_DVAR_T_PAIR(): XXX
 */
#define C_DVAR_T_PAIR(_key, _value) \
        C_DVAR_TI_TUPLE(C_DVAR_TI_TUPLE_APPEND(_key, _value), '{',"{", '}', "}")

/**
 * C_DVAR_T_TUPLE(): XXX
 */
#define C_DVAR_T_TUPLE(_type) \
        C_DVAR_TI_TUPLE(_type, '(', "(", ')', ")")

/**
 * C_DVAR_T_TUPLE_APPEND(): XXX
 */
#define C_DVAR_T_TUPLE_APPEND(_set, _next) \
        C_DVAR_TI_TUPLE_APPEND(_set, _next)

/**
 * C_DVAR_T_TUPLE0: XXX
 */
#define C_DVAR_T_TUPLE0 ( \
                0,                                                                      \
                3,                                                                      \
                "()",                                                                   \
                2,                                                                      \
                0,                                                                      \
                (                                                                       \
                        {                                                               \
                                .size           = 0,                                    \
                                .alignment      = 3,                                    \
                                .element        = '(',                                  \
                                .length         = 2,                                    \
                                .basic          = 0,                                    \
                        },                                                              \
                        {                                                               \
                                .size           = 0,                                    \
                                .alignment      = 0,                                    \
                                .element        = ')',                                  \
                                .length         = 1,                                    \
                                .basic          = 0,                                    \
                        }                                                               \
                )                                                                       \
        )

/**
 * C_DVAR_T_TUPLE1(): XXX
 */
#define C_DVAR_T_TUPLE1(_type1) \
        C_DVAR_T_TUPLE(_type1)

/**
 * C_DVAR_T_TUPLE2(): XXX
 */
#define C_DVAR_T_TUPLE2(_type1, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE3(): XXX
 */
#define C_DVAR_T_TUPLE3(_type1, _type2, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE4(): XXX
 */
#define C_DVAR_T_TUPLE4(_type1, _type2, _type3, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE5(): XXX
 */
#define C_DVAR_T_TUPLE5(_type1, _type2, _type3, _type4, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE6(): XXX
 */
#define C_DVAR_T_TUPLE6(_type1, _type2, _type3, _type4, _type5, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE7(): XXX
 */
#define C_DVAR_T_TUPLE7(_type1, _type2, _type3, _type4, _type5, _type6, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE8(): XXX
 */
#define C_DVAR_T_TUPLE8(_type1, _type2, _type3, _type4, _type5, _type6, _type7, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE9(): XXX
 */
#define C_DVAR_T_TUPLE9(_type1, _type2, _type3, _type4, _type5, _type6, _type7, _type8, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE10(): XXX
 */
#define C_DVAR_T_TUPLE10(_type1, _type2, _type3, _type4, _type5, _type6, _type7, _type8, _type9, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE11(): XXX
 */
#define C_DVAR_T_TUPLE11(_type1, _type2, _type3, _type4, _type5, _type6, _type7, _type8, _type9, _type10, _type_l) \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE12(): XXX
 */
#define C_DVAR_T_TUPLE12(               \
                _type1,  _type2,  _type3,  _type4,      \
                _type5,  _type6,  _type7,  _type8,      \
                _type9,  _type10, _type11, _type_l      \
        )                               \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type11),       \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE13(): XXX
 */
#define C_DVAR_T_TUPLE13(               \
                _type1,  _type2,  _type3,  _type4,      \
                _type5,  _type6,  _type7,  _type8,      \
                _type9,  _type10, _type11, _type12,     \
                _type_l                                 \
        )                               \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type11),       \
                        _type12),       \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE14(): XXX
 */
#define C_DVAR_T_TUPLE14(               \
                _type1,  _type2,  _type3,  _type4,      \
                _type5,  _type6,  _type7,  _type8,      \
                _type9,  _type10, _type11, _type12,     \
                _type13, _type_l                        \
        )                               \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type11),       \
                        _type12),       \
                        _type13),       \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE15(): XXX
 */
#define C_DVAR_T_TUPLE15(               \
                _type1,  _type2,  _type3,  _type4,      \
                _type5,  _type6,  _type7,  _type8,      \
                _type9,  _type10, _type11, _type12,     \
                _type13, _type14, _type_l               \
        )                               \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type11),       \
                        _type12),       \
                        _type13),       \
                        _type14),       \
                        _type_l)        \
        )

/**
 * C_DVAR_T_TUPLE16(): XXX
 */
#define C_DVAR_T_TUPLE16(               \
                _type1,  _type2,  _type3,  _type4,      \
                _type5,  _type6,  _type7,  _type8,      \
                _type9,  _type10, _type11, _type12,     \
                _type13, _type14, _type15, _type_l      \
        )                               \
        C_DVAR_T_TUPLE(                 \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                C_DVAR_T_TUPLE_APPEND(  \
                        _type1,         \
                        _type2),        \
                        _type3),        \
                        _type4),        \
                        _type5),        \
                        _type6),        \
                        _type7),        \
                        _type8),        \
                        _type9),        \
                        _type10),       \
                        _type11),       \
                        _type12),       \
                        _type13),       \
                        _type14),       \
                        _type15),       \
                        _type_l)        \
        )

#ifdef __cplusplus
}
#endif
