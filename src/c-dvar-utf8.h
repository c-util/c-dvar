#pragma once

/*
 * UTF-8 Validation Helper
 */

#include "c-dvar-private.h"

/**
 * c_dvar_utf8_verify() - verify that a scring is UTF-8 encoded
 * @strp:               pointer to string to verify
 * @lenp:               pointer to length of string
 *
 * Up to the first @lenp bytes of the string pointed to by @strp is
 * verified to be UTF-8 encoded, and @strp and @lenp are updated to
 * point to the first non-UTF-8 character or the first NULL of the
 * string, and the remaining number of bytes of the string,
 * respectively.
 */
static inline void c_dvar_utf8_verify(const char **strp, size_t *lenp) {
        unsigned char *str = (unsigned char *)*strp;
        size_t len = *lenp;

        /* See Unicode 9.0.0, Chapter 3, Section D92 */

        while (len > 0) {
                if (_unlikely_(*str == 0x00)) {
                        break;
                } else if (*str < 0x80) {
                        ++str;
                        --len;
                } else if (_unlikely_(*str < 0xC2)) {
                        break;
                } else if (*str < 0xE0) {
                        if (_unlikely_(len < 2))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;

                        str += 2;
                        len -= 2;
                } else if (*str < 0xE1) {
                        if (_unlikely_(len < 3))
                                break;
                        if (_unlikely_(*(str + 1) < 0xA0 || *(str + 1) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xED) {
                        if (_unlikely_(len < 3))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xEE) {
                        if (_unlikely_(len < 3))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x9F))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xF0) {
                        if (_unlikely_(len < 3))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;

                        str += 3;
                        len -= 3;
                } else if (*str < 0xF1) {
                        if (_unlikely_(len < 4))
                                break;
                        if (_unlikely_(*(str + 1) < 0x90 || *(str + 1) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else if (*str < 0xF4) {
                        if (_unlikely_(len < 4))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else if (*str < 0xF5) {
                        if (_unlikely_(len < 4))
                                break;
                        if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x8F))
                                break;
                        if (_unlikely_(*(str + 2) < 0x80 || *(str + 2) > 0xBF))
                                break;
                        if (_unlikely_(*(str + 3) < 0x80 || *(str + 3) > 0xBF))
                                break;

                        str += 4;
                        len -= 4;
                } else
                        break;
        }

        *strp = (char *)str;
        *lenp = len;
}
