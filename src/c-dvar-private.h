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

bool c_dvar_is_string(const char *str, size_t len);
bool c_dvar_is_path(const char *str, size_t len);
bool c_dvar_is_signature(const char *string, size_t n_string);
bool c_dvar_is_type(const char *string, size_t n_string);

void c_dvar_rewind(CDVar *var);
int c_dvar_next_varg(CDVar *var, char c);
void c_dvar_push(CDVar *var);
void c_dvar_pop(CDVar *var);

uint16_t c_dvar_bswap16(CDVar *var, uint16_t v);
uint32_t c_dvar_bswap32(CDVar *var, uint32_t v);
uint64_t c_dvar_bswap64(CDVar *var, uint64_t v);
