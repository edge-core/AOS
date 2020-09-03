#include "json_object_path.h"

#ifndef _countof
#define _countof(_array) (sizeof(_array) / sizeof(*_array))
#endif

json_t*
json_object_path_get( // <------- no use any more !
    const json_t *object,
    uint32_t flags,
    const char *first,
    ...)
{
    const char*str;
    va_list ap;

    const json_t *parent = object;
    json_t *target;

    str = first;
    va_start(ap, first);

    do {
        if (!json_is_object(parent))
        {
            target = NULL;
            goto end;
        }

        target = json_object_get(parent, str);
        if (target != NULL)
        {
        }

        parent = target;

        str = va_arg(ap, const char*);
    } while (str != NULL);

end:
    va_end(ap);

    return target;
}

json_t*
json_object_path_new_object( // <------- no use any more !
    json_t *object,
    uint32_t flags,
    const char *first,
    ...)
{
    const char*str;
    va_list ap;

    json_t *parent = object;
    json_t *target;

    str = first;
    va_start(ap, first);

    do {
        if (!json_is_object(parent))
        {
            target = NULL;
            goto end;
        }

        target = json_object_get(parent, str);
        if (target != NULL)
        {
        }
        else
        {
            target = json_object();
            if (target == NULL)
            {
                goto end;
            }

            if (json_object_set_new(parent, str, target) != 0)
            {
                target = NULL;
                goto end;
            }
        }

        parent = target;

        str = va_arg(ap, const char*);
    } while (str != NULL);

end:
    va_end(ap);

    if (target && json_typeof(target) != JSON_OBJECT)
    {
        target = NULL;
    }

    return target;
}

json_t*
json_object_path_get_2(
    json_t *object,
    int npath,
    const char **path)
{
    int i;
    json_t *target;
    const char *key;

    i = 0;

    if (2 <= npath) {
        do {
            key = path[i];

            if (!key) {
                return NULL;
            }

            if (!json_is_object(object)) {
                return NULL;
            }

            target = json_object_get(object, key);

            if (target == NULL) {
                return NULL;
            }

            object = target;

        } while (++i < npath - 1);
    }

    if (1 <= npath) {
        key = path[i];

        if (key) {
            return json_object_get(object, key);
        }
    }

    return NULL;
}

int
json_object_path_set_new(
    json_t *object,
    int npath,
    const char **path,
    json_t *value)
{
    int i;
    json_t *target;
    const char *key;

    i = 0;

    if (2 <= npath) {
        do {
            key = path[i];

            if (!key) {
                return -1;
            }

            if (!json_is_object(object)) {
                return -1;
            }

            target = json_object_get(object, key);

            if (target == NULL) {
                target = json_object();
                if (target == NULL) {
                    return -1;
                }

                json_object_set_new(object, key, target);
            }

            object = target;

        } while (++i < npath - 1);
    }

    if (1 <= npath) {
        key = path[i];

        if (key) {
            return json_object_set_new(object, key, value);
        }
    }

    return -1;
}

int
json_object_path_set(
    json_t *object,
    int npath,
    const char **path,
    json_t *value)
{
    int rc;

    json_incref(value);
    rc = json_object_path_set_new(object, npath, path, value);

    if (rc != 0) {
        json_decref(value);
    }

    return rc;
}

int
json_object_path_set_1(
    json_t *object,
    const char *path1,
    json_t *value)
{
    const char *_path[1];

    _path[0] = path1;

    return json_object_path_set(object, _countof(_path), _path, value);
}

int
json_object_path_set_2(
    json_t *object,
    const char *path1,
    const char *path2,
    json_t *value)
{
    const char *_path[2];

    _path[0] = path1;
    _path[1] = path2;

    return json_object_path_set(object, _countof(_path), _path, value);
}

int
json_object_path_set_3(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    json_t *value)
{
    const char *_path[3];

    _path[0] = path1;
    _path[1] = path2;
    _path[2] = path3;

    return json_object_path_set(object, _countof(_path), _path, value);
}

int
json_object_path_set_4(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    const char *path4,
    json_t *value)
{
    const char *_path[4];

    _path[0] = path1;
    _path[1] = path2;
    _path[2] = path3;
    _path[3] = path4;

    return json_object_path_set(object, _countof(_path), _path, value);
}

int
json_object_path_set_new_1(
    json_t *object,
    const char *path1,
    json_t *value)
{
    const char *_path[1];

    _path[0] = path1;

    return json_object_path_set_new(object, _countof(_path), _path, value);
}

int
json_object_path_set_new_2(
    json_t *object,
    const char *path1,
    const char *path2,
    json_t *value)
{
    const char *_path[2];

    _path[0] = path1;
    _path[1] = path2;

    return json_object_path_set_new(object, _countof(_path), _path, value);
}

int
json_object_path_set_new_3(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    json_t *value)
{
    const char *_path[3];

    _path[0] = path1;
    _path[1] = path2;
    _path[2] = path3;

    return json_object_path_set_new(object, _countof(_path), _path, value);
}

int
json_object_path_set_new_4(
    json_t *object,
    const char *path1,
    const char *path2,
    const char *path3,
    const char *path4,
    json_t *value)
{
    const char *_path[4];

    _path[0] = path1;
    _path[1] = path2;
    _path[2] = path3;
    _path[3] = path4;

    return json_object_path_set_new(object, _countof(_path), _path, value);
}


