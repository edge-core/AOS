//
//  confdb_lock.c
//  confdb
//
//  Created by JunYing Yeh on 2014/11/7.
//
//
#include "pthread.h"
#include "confdb_private.h"

static int                  confdb_lock_init;
static pthread_rwlock_t     confdb_lock_rwlock[CONFDB_LOCK_NUM_OF_LOCK_TYPE];

void CONFDB_LOCK_Create()
{
    int i;

    if (confdb_lock_init)
    {
        return;
    }

    for (i = 0; i < _countof(confdb_lock_rwlock); i++)
    {
        pthread_rwlock_init(&confdb_lock_rwlock[i], NULL);
    }
}

void CONFDB_LOCK_Lock(int mode, int type, const char *file, int line)
{
    int rc;

    if (!confdb_lock_init)
    {
        return;
    }

    if (_countof(confdb_lock_rwlock) <= type)
    {
        return;
    }

    if (mode & CONFDB_LOCK_MODE_LOCK)
    {
        if (mode & CONFDB_LOCK_MODE_READ)
        {
            rc = pthread_rwlock_rdlock(&confdb_lock_rwlock[type]);
        }
        else if (mode & CONFDB_LOCK_MODE_WRITE)
        {
            rc = pthread_rwlock_wrlock(&confdb_lock_rwlock[type]);
        }
    }
    else if (mode & CONFDB_LOCK_MODE_UNLOCK)
    {
        rc = pthread_rwlock_unlock(&confdb_lock_rwlock[type]);
    }
}
