#ifndef _CGI_REGEX_H_
#define _CGI_REGEX_H_

#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

pcre2_code *
re_compile(
    const char *pattern
);

BOOL_T
re_test(
    const char *pattern,
    const char *str
);

#if __cplusplus
}
#endif

#endif /* _CGI_REGEX_H_ */
