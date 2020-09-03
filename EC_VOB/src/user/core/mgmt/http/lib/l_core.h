//
//  l_core.h
//  http_ssl
//
//  Created by JunYing Yeh on 2014/5/1.
//
//

#ifndef lib_core_h
#define lib_core_h

enum
{
    L_SOCK
};

enum
{
    L_CORE_LOCK     = 1,
    L_CORE_UNLOCK   = 2,
    L_CORE_READ     = 4,
    L_CORE_WRITE    = 8
};

#define L_CORE_WRITE_LOCK(type)     L_CORE_write_lock(type, __FILE__, __LINE__)
#define L_CORE_WRITE_UNLOCK(type)   L_CORE_write_unlock(type, __FILE__, __LINE__)
#define L_CORE_READ_LOCK(type)      L_CORE_read_lock(type, __FILE__, __LINE__)
#define L_CORE_READ_UNLOCK(type)    L_CORE_read_unlock(type, __FILE__, __LINE__)

void L_CORE_set_locking_callback(void (*func)(int mode, int type,
                                              const char *file, int line));

void (*L_CORE_get_locking_callback(void))(int mode, int type,
                                          const char *file, int line);

void L_CORE_write_lock(int type, const char *file, int line);

void L_CORE_write_unlock(int type, const char *file, int line);

void L_CORE_read_lock(int type, const char *file, int line);

void L_CORE_read_unlock(int type, const char *file, int line);

#endif
