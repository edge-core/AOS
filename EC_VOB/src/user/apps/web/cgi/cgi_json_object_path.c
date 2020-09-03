#include "cgi.h"

//#define JSON_NO_AUTO_CREATE   0x1000 // TODO: create-new-object-if-miss
//// TODO: new API, json_object_set_path ??
json_t* json_object_get_path(json_t *object, uint32_t flags, const char *first, ...)
{
    const char*str;
    va_list ap;

    int no_auto_create = flags & JSON_NO_AUTO_CREATE;

    json_t *parent = object;
    json_t *target;

    str = first;
    va_start(ap, first);

    do {
        //        printf("%s \n", str);

        if (!json_is_object(parent))
        {
            return NULL;
        }

        target = json_object_get(parent, str);
        if (target != NULL)
        {
            //            parent = target;
        }
        else
        {
            if (no_auto_create)
            {
                //                return NULL;
                goto end;
            }

            target = json_object();
            if (target == NULL)
            {
                //                return NULL;
                goto end;
            }

            if (json_object_set_new(parent, str, target) != 0)
            {
                //                return NULL;
                goto end;
            }
        }

        parent = target;

        str = va_arg(ap, const char*);
    } while (str != NULL);

end:
    va_end(ap);

    ASSERT((target != NULL && 0 < target->refcount) || target == NULL);
    //    ASSERT(0 < target->refcount);

    return target;
}