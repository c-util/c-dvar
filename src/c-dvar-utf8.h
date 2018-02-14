#pragma once

/*
 * UTF-8 Validation Helper
 */

#include "c-dvar-private.h"

#define C_DVAR_CHAR_IS_UTF8_TAIL(_x)    (((_x) & 0xC0) == 0x80)

/**
 * c_dvar_utf8_verify() - verify that a string is UTF-8 encoded
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

        /* See Unicode 10.0.0, Chapter 3, Section D92 */

        while (len > 0) {
                if (*str < 128) {
                        if (_unlikely_(*str == 0x00))
                                goto out;
                        ++str;
                        --len;
                } else {
                        switch (*str) {
                        case 0xC2 ... 0xDF:
                                if (_unlikely_(len < 2))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 1))))
                                        goto out;

                                str += 2;
                                len -= 2;

                                break;
                        case 0xE0:
                                if (_unlikely_(len < 3))
                                        goto out;
                                if (_unlikely_(*(str + 1) < 0xA0 || *(str + 1) > 0xBF))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;

                                str += 3;
                                len -= 3;

                                break;
                        case 0xE1 ... 0xEC:
                                if (_unlikely_(len < 3))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 1))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;

                                str += 3;
                                len -= 3;

                                break;
                        case 0xED:
                                if (_unlikely_(len < 3))
                                        goto out;
                                if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x9F))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;

                                str += 3;
                                len -= 3;

                                break;
                        case 0xEE ... 0xEF:
                                if (_unlikely_(len < 3))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 1))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;

                                str += 3;
                                len -= 3;

                                break;
                        case 0xF0:
                                if (_unlikely_(len < 4))
                                        goto out;
                                if (_unlikely_(*(str + 1) < 0x90 || *(str + 1) > 0xBF))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 3))))
                                        goto out;

                                str += 4;
                                len -= 4;

                                break;
                        case 0xF1 ... 0xF3:
                                if (_unlikely_(len < 4))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 1))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 3))))
                                        goto out;

                                str += 4;
                                len -= 4;

                                break;
                        case 0xF4:
                                if (_unlikely_(len < 4))
                                        goto out;
                                if (_unlikely_(*(str + 1) < 0x80 || *(str + 1) > 0x8F))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 2))))
                                        goto out;
                                if (_unlikely_(!C_DVAR_CHAR_IS_UTF8_TAIL(*(str + 3))))
                                        goto out;

                                str += 4;
                                len -= 4;

                                break;
                        default:
                                goto out;
                        }
                }
        }

out:
        *strp = (char *)str;
        *lenp = len;
}
