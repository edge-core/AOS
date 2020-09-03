#ifndef _STR_TO_INT_H_
#define _STR_TO_INT_H_

#include "sys_type.h"

#if __cplusplus
extern "C" {
#endif

BOOL_T StringToInteger(const char *str, long long *value);

#if __cplusplus
}
#endif

#endif /* _STR_TO_INT_H_ */
