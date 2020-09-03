//
//  mm.h
//
//  Created by JunYing Yeh on 2014/4/22.
//
//

#ifndef cgi_demo_mm_h
#define cgi_demo_mm_h

enum
{
    MM_LOCK = 1,
    MM_UNLOCK = 2,
    MM_READ = 4,
    MM_WRITE = 8
};

enum
{
    MM_OPTION_SHOW_LEAKAGE = 1,
    MM_OPTION_SHOW_LEAKAGE_DETAIL = 2,
    MM_OPTION_ALL = (MM_OPTION_SHOW_LEAKAGE | MM_OPTION_SHOW_LEAKAGE_DETAIL)
};

typedef void (*mm_lock_t)(int flags);

#if __cplusplus
extern "C" {
#endif

void MM_set_lock_function(mm_lock_t cb);
void *MM_alloc(void *ptr, size_t size, int tid, const char *file, int line, void *);
void MM_free(void *ptr, int tid);
int MM_kill(int tid);
int MM_add_options(int options);
int MM_remove_options(int options);
int MM_toggle_options(int options);
int MM_get_options();

#if __cplusplus
}
#endif

#endif
