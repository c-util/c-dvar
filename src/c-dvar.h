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

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
}
#endif
