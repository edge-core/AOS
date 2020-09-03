//
//  http_list.h
//  http
//
//  Created by JunYing Yeh on 2014/7/15.
//
//

#ifndef _HTTP_HEADER_LIST_H_
#define _HTTP_HEADER_LIST_H_

#include "http_type.h"

typedef struct
{
    HTTP_SHM_POINTER_T  list_ptr;

    size_t              counter;

    void * (*ptr2instance)(HTTP_SHM_POINTER_T);
    HTTP_SHM_POINTER_T (*instance2ptr)(void *);
    void (*dbg_validate_instance)(void *);
} HTTP_LIST_SHM_T;

typedef struct
{
    HTTP_INSTANCE_T     list_ptr;

    size_t              counter;

    void * (*ptr2instance)(HTTP_SHM_POINTER_T);
    HTTP_SHM_POINTER_T (*instance2ptr)(void *);
    void (*dbg_validate_instance)(HTTP_INSTANCE_T *);
} HTTP_LIST_T;

#if __cplusplus
extern "C" {
#endif

/**----------------------------------------------------------------------
 * Initialize list
 *
 * @param  list_ctx         list
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_Init(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_INSTANCE_TYPE_T type,
    void * (*ptr2instance)(HTTP_SHM_POINTER_T),
    HTTP_SHM_POINTER_T (*instance2ptr)(void *),
    void (*dbg_validate_instance)(void *)
);

/**----------------------------------------------------------------------
 * Inserts node after the specified node
 *
 * @param  list_ctx         list
 * @param  new_ptr          inserts node after this node
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertAfter(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T node_ptr,
    HTTP_SHM_POINTER_T new_node_ptr
);

/**----------------------------------------------------------------------
 * Inserts node before the specified node
 *
 * @param  list_ctx         list
 * @param  new_ptr          inserts node before this node
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertBefore(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T node_ptr,
    HTTP_SHM_POINTER_T new_node_ptr
);

/**----------------------------------------------------------------------
 * Inserts node before current first node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertBeginning(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T new_node_ptr
);

/**----------------------------------------------------------------------
 * Append node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertEnd(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T new_node_ptr
);

/**----------------------------------------------------------------------
 * Remove node
 *
 * @param  list_ctx         list
 * @param  node_ptr         removed node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_Remove(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T node_ptr
);

/**----------------------------------------------------------------------
 * Initialize list
 *
 * @param  list_ctx         list
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_Init(
    HTTP_LIST_T *list_ctx,
    void (*dbg_validate_instance)(HTTP_INSTANCE_T *)
);

/**----------------------------------------------------------------------
 * Inserts node after the specified node
 *
 * @param  list_ctx         list
 * @param  new_ptr          inserts node after this node
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertAfter(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T node,
    HTTP_INSTANCE_PTR_T new_node
);

/**----------------------------------------------------------------------
 * Inserts node before the specified node
 *
 * @param  list_ctx         list
 * @param  new_ptr          inserts node before this node
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertBefore(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T node,
    HTTP_INSTANCE_PTR_T new_node
);

/**----------------------------------------------------------------------
 * Inserts node before current first node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertBeginning(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T new_node
);

/**----------------------------------------------------------------------
 * Append node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertEnd(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T new_node
);

/**----------------------------------------------------------------------
 * Remove node
 *
 * @param  list_ctx         list
 * @param  node_ptr         removed node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_Remove(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T node
);

#if __cplusplus
}
#endif

#endif
