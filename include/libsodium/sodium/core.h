
#ifndef sodium_core_H
#define sodium_core_H

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

SODIUM_EXPORT
int sodium_init(void)
            __attribute__ ((warn_unused_result));

/* ---- */

SODIUM_EXPORT
int sodium_set_misuse_handler(void (*handler)(void));

extern void _sodium_misuse(char const *file, int line);

#define sodium_misuse(...) _sodium_misuse(__FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif
