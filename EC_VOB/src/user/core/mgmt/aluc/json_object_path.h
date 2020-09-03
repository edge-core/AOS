#ifndef _CGI_JSON_OBJECT_PATH_H_
#define _CGI_JSON_OBJECT_PATH_H_

#include <stdint.h>
#include "jansson.h"

#if __cplusplus
extern "C" {
#endif

json_t*
json_object_path_get(
    const json_t *object,
    uint32_t flags,
    const char *first,
    ...
);

json_t*
json_object_path_get_2(
    json_t *object,
    int npath,
    const char **path
);

json_t*
json_object_path_new_object(
    json_t *object,
    uint32_t flags,
    const char *first,
    ...
);

int
json_object_path_set_new(
    json_t *object,
    int npath,
    const char **path,
    json_t *value
);

int
json_object_path_set(
    json_t *object,
    int npath,
    const char **path,
    json_t *value
);

int
json_object_path_set_1(
    json_t *object,
    const char *path1,
    json_t *value
);

int
json_object_path_set_2(
    json_t *object,
    const char *path1,
    const char *path2,
    json_t *value
);

int
json_object_path_set_3(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    json_t *value
);

int
json_object_path_set_4(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    const char *path4,
    json_t *value
);

int
json_object_path_set_new_1(
    json_t *object,
    const char *path1,
    json_t *value
);

int
json_object_path_set_new_2(
    json_t *object,
    const char *path1,
    const char *path2,
    json_t *value
);

int
json_object_path_set_new_3(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    json_t *value
);

int
json_object_path_set_new_4(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    const char *path4,
    json_t *value
);

#if __cplusplus
}
#endif

#endif /* _CGI_JSON_OBJECT_PATH_H_ */
