#pragma once

/*
 * Internal Definitions
 *
 * This header supplements the public header with all private symbols and
 * definitions.
 */

#include <c-stdaux.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "c-dvar.h"

typedef struct CDVarLevel CDVarLevel;

bool c_dvar_is_string(const char *string, size_t n_string);
bool c_dvar_is_signature(const char *string, size_t n_string);
bool c_dvar_is_type(const char *string, size_t n_string);

void c_dvar_rewind(CDVar *var);
int c_dvar_next_varg(CDVar *var, char c);
void c_dvar_push(CDVar *var);
void c_dvar_pop(CDVar *var);

uint16_t c_dvar_bswap16(CDVar *var, uint16_t v);
uint32_t c_dvar_bswap32(CDVar *var, uint32_t v);
uint64_t c_dvar_bswap64(CDVar *var, uint64_t v);
