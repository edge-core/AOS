#ifndef SSL_CRYPTO_LOCK
#define SSL_CRYPTO_LOCK

#include "sys_type.h"

#if __cplusplus
extern "C" {
#endif

BOOL_T SSL_CRYPTO_LOCK_InitateSystemResource();
void SSL_CRYPTO_LOCK_Create();
void SSL_CRYPTO_LOCK_Lock(int mode, int type, const char *file, int line);

#if __cplusplus
}
#endif

#endif /* SSL_CRYPTO_LOCK */