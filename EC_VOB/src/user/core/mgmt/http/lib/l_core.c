//
//  l_core.c
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#include "l_config.h"
#include "l_core.h"

static void (*locking_callback)(int mode, int type, const char *file, int line);

void L_CORE_set_locking_callback(void (*func)(int mode, int type,
                                         const char *file, int line))
{
    locking_callback = func;
}

void (*L_CORE_get_locking_callback(void))(int mode, int type,
                                          const char *file, int line)
{
    return locking_callback;
}

void L_CORE_write_lock(int type, const char *file, int line)
{
    if (locking_callback)
    {
        locking_callback(L_CORE_LOCK | L_CORE_WRITE, type, file, line);
    }
}

void L_CORE_write_unlock(int type, const char *file, int line)
{
    if (locking_callback)
    {
        locking_callback(L_CORE_UNLOCK | L_CORE_WRITE, type, file, line);
    }
}

void L_CORE_read_lock(int type, const char *file, int line)
{
    if (locking_callback)
    {
        locking_callback(L_CORE_LOCK | L_CORE_READ, type, file, line);
    }
}

void L_CORE_read_unlock(int type, const char *file, int line)
{
    if (locking_callback)
    {
        locking_callback(L_CORE_LOCK | L_CORE_READ, type, file, line);
    }
}