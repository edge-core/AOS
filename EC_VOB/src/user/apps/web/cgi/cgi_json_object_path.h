#ifndef _CGI_JSON_OBJECT_PATH_H_
#define _CGI_JSON_OBJECT_PATH_H_

#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

#define JSON_NO_AUTO_CREATE   0x1000 // TODO: create-new-object-if-miss
json_t*
json_object_get_path(
    json_t *object,
    uint32_t flags,
    const char *first, ...
);

#if __cplusplus
}
#endif

#endif /* _CGI_JSON_OBJECT_PATH_H_ */
