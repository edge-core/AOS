//
//  http_ring_buffer.h
//  http
//
//  Created by JunYing Yeh on 2014/7/23.
//
//

#ifndef _HTTP_HEADER_RING_BUFFER_H_
#define _HTTP_HEADER_RING_BUFFER_H_

#include "http_type.h"

#if __cplusplus
extern "C" {
#endif

struct HTTP_RING_BUFFER_DESC_ST
{
    size_t      capacity;       /* the number of elements that the container has
                                 * currently allocated space
                                 */
    size_t      element_size;

    size_t      nelements;      /* the number of elements */
    size_t      first_idx;
    size_t      last_idx;

    void        *buffer;
};

typedef struct HTTP_RING_BUFFER_DESC_ST HTTP_RING_BUFFER_DESC_T;

void http_ring_buffer_init(HTTP_RING_BUFFER_DESC_T *desc, size_t capacity, size_t element_size, void *buffer);
void * http_ring_buffer_new_element(HTTP_RING_BUFFER_DESC_T *desc);

#if __cplusplus
}
#endif

#endif /* _HTTP_HEADER_RING_BUFFER_H_ */
