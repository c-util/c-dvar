/*
 * Common Reader/Writer Helpers
 *
 * XXX
 */

#include <assert.h>
#include <byteswap.h>
#include <c-stdaux.h>
#include <endian.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include "c-dvar.h"
#include "c-dvar-private.h"

uint16_t c_dvar_bswap16(CDVar *var, uint16_t v) {
        return _c_likely_(!!var->big_endian == !!(__BYTE_ORDER == __BIG_ENDIAN)) ? v : bswap_16(v);
}

uint32_t c_dvar_bswap32(CDVar *var, uint32_t v) {
        return _c_likely_(!!var->big_endian == !!(__BYTE_ORDER == __BIG_ENDIAN)) ? v : bswap_32(v);
}

uint64_t c_dvar_bswap64(CDVar *var, uint64_t v) {
        return _c_likely_(!!var->big_endian == !!(__BYTE_ORDER == __BIG_ENDIAN)) ? v : bswap_64(v);
}
