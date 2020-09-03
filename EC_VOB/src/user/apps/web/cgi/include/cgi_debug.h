#ifndef CGI_DEBUG_H
#define CGI_DEBUG_H 1

#include <stdio.h>
#include "sys_type.h"

#if __cplusplus
extern "C" {
#endif

typedef enum
{
    CGI_DEBUG_FILE    = 0x00000001,
    CGI_DEBUG_CACHE   = 0x00000002,
    CGI_DEBUG_LIBRARY = 0x00000004,
} CGI_DEBUG_FLAG;

#if (1400 <= _MSC_VER /* VC2005 */)
#define CGI_DEBUG_LOG(fmt, ...)                     \
    {                                               \
        if (IS_THIS_DEBUG_ON())                     \
        {                                           \
            CGI_DEBUG_PRINT_STR(fmt, __VA_ARGS__);  \
        }                                           \
    }
#else
#define CGI_DEBUG_LOG(fmt, args...)                 \
    {                                               \
        if (IS_THIS_DEBUG_ON())                     \
        {                                           \
            CGI_DEBUG_PRINT_STR(fmt, ##args);       \
        }                                           \
    }
#endif /* _MSC_VER */

#if (1400 <= _MSC_VER /* VC2005 */)
#define CGI_DEBUG_PRINT_STR(fmt, ...)               \
    {                                               \
        printf("%s(%d)\r\n  ",                      \
               __FUNCTION__, __LINE__);             \
        printf(fmt, __VA_ARGS__);                   \
        printf("\r\n");                             \
        fflush(stdout);                             \
    }
#else
#define CGI_DEBUG_PRINT_STR(fmt, args...)           \
    {                                               \
        printf("%s(%d)\r\n  ",                      \
               __FUNCTION__, __LINE__);             \
        printf(fmt, ##args);                        \
        printf("\r\n");                             \
        fflush(stdout);                             \
    }
#endif

UI32_T CGI_DEBUG_GetFlag();

UI32_T CGI_DEBUG_SetFlag(UI32_T flag);

UI32_T CGI_DEBUG_ToggleFlag(CGI_DEBUG_FLAG flag);

UI32_T CGI_DEBUG_CleanFlag();

#if __cplusplus
}
#endif

#endif /* CGI_DEBUG_H */

