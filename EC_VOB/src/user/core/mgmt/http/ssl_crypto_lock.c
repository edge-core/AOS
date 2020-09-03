#include <pthread.h>
#include "sys_bld.h"
#include "sysfun.h"
#include "ssl_crypto_lock.h"

#ifndef CRYPTO_LOCK
#define CRYPTO_LOCK         1
#endif

#ifndef CRYPTO_UNLOCK
#define CRYPTO_UNLOCK       2
#endif


#define CRYPTO_READ     4
#define CRYPTO_WRITE        8

static UI32_T ssl_crypto_dynlock_semid;

pthread_mutex_t     rwlock[30];

BOOL_T SSL_CRYPTO_LOCK_InitateSystemResource(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_OPENSSL, &ssl_crypto_dynlock_semid)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

void SSL_CRYPTO_LOCK_Create()
{
    UI32_T i;

    for (i = 0; i < sizeof(rwlock)/sizeof(rwlock[0]); i++)
    {
        pthread_mutex_init(&rwlock[i], NULL);
    }
}

void SSL_CRYPTO_LOCK_Lock(int mode, int type, const char *file, int line)
{
    int rc;

    if (type > sizeof(rwlock)/sizeof(rwlock[0]))
    {
        return;
    }

    if (mode & CRYPTO_LOCK)
    {
        if (mode & CRYPTO_READ)
        {
            rc = pthread_mutex_lock(&rwlock[type]);
        }
        else if (mode & CRYPTO_WRITE)
        {
            rc = pthread_mutex_lock(&rwlock[type]);
        }
    }
    else if (mode & CRYPTO_UNLOCK)
    {
        rc = pthread_mutex_unlock(&rwlock[type]);
    }
}