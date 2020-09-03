#ifndef _CGI_PATH_2_REGEX_H_
#define _CGI_PATH_2_REGEX_H_

#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

char *
path2Regexp(
    const char *path,
    BOOL_T strict
);

#if __cplusplus
}
#endif

#endif /* _CGI_PATH_2_REGEX_H_ */
