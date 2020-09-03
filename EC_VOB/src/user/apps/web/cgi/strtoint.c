#include <stdlib.h>

#ifndef LONG_MAX
#include <limits.h>
#endif

#include "strtoint.h"

#if defined(_MSC_VER)
#define strtoll  _strtoi64
#endif

BOOL_T StringToInteger(const char *str, long long *value)
{
#if defined(_MSC_VER)
    long long LL_MAX = _I64_MAX;
    long long LL_MIN = _I64_MIN;
#else
    long long LL_MAX = LONG_MAX;
    long long LL_MIN = LONG_MIN;
#endif

    char *end;

    *value = strtoll(str, &end, 10);

    if ((end == NULL) || (end != NULL && *end != '\0'))
    {
        return FALSE;
    }

    if (*value == LL_MAX ||
        *value == LL_MIN)
    {
        return FALSE;
    }

    return TRUE;
}
