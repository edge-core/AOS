//
//  http_ring_buffer.c
//  http
//
//  Created by JunYing Yeh on 2014/7/23.
//
//

#include "http_loc.h"

#if (HTTP_CFG_RING_BUFFER_DEBUG == 1)
static void http_ring_buffer_dbg_check(HTTP_RING_BUFFER_DESC_T *desc);
#else
#define http_ring_buffer_dbg_check(d)
#endif /* HTTP_CFG_RING_BUFFER_DEBUG */

void http_ring_buffer_init(HTTP_RING_BUFFER_DESC_T *desc, size_t capacity, size_t element_size, void *buffer)
{
    memset(desc, 0, sizeof(*desc));

    desc->capacity      = capacity;
    desc->element_size  = element_size;
    desc->buffer        = buffer;
}

void * http_ring_buffer_new_element(HTTP_RING_BUFFER_DESC_T *desc)
{
    void *elem = NULL;

    ASSERT(desc != NULL);

    elem = ((char *)desc->buffer + (desc->last_idx * desc->element_size));

    if (desc->nelements == desc->capacity)
    {
        ASSERT(desc->first_idx == desc->last_idx);

        desc->first_idx = (desc->first_idx + 1) % desc->capacity;
    }
    else
    {
        desc->nelements ++;
    }

    desc->last_idx = (desc->last_idx + 1) % desc->capacity;

    http_ring_buffer_dbg_check(desc);

    return elem;
}

#if (HTTP_CFG_RING_BUFFER_DEBUG == 1)
static void http_ring_buffer_dbg_check(HTTP_RING_BUFFER_DESC_T *desc)
{
    ASSERT(desc->first_idx < desc->capacity);
    ASSERT(desc->last_idx < desc->capacity);

    if (desc->first_idx == desc->last_idx)
    {
        ASSERT(desc->nelements == 0 || desc->nelements == desc->capacity);
    }
    else
    {
        ASSERT(desc->nelements < desc->capacity);
    }
}
#endif /* HTTP_CFG_RING_BUFFER_DEBUG */
