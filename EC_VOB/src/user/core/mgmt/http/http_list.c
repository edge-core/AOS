//
//  http_list.c
//  http
//
//  Created by JunYing Yeh on 2014/7/15.
//
//

#include "http_loc.h"

#define HTTP_LIST_PTR2INSTANCE(ptr)                 (HTTP_INSTANCE_T*)list_ctx->ptr2instance(ptr)
#define HTTP_LIST_INSTANCE2PTR(in)                  list_ctx->instance2ptr(in)

#define HTTP_LIST_VALIDATE_INSTANCE(in)             \
    do {                                            \
        if (list_ctx->dbg_validate_instance) {      \
            list_ctx->dbg_validate_instance(in);    \
        }                                           \
    } while(0)

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
    void (*dbg_validate_instance)(void *))
{
    ASSERT(list_ctx != NULL);

    memset(list_ctx, 0, sizeof(*list_ctx));

    list_ctx->list_ptr.type = type;

    list_ctx->ptr2instance = ptr2instance;
    list_ctx->instance2ptr = instance2ptr;
    list_ctx->dbg_validate_instance = dbg_validate_instance;
}

/**----------------------------------------------------------------------
 * Initialize list
 *
 * @param  list_ctx         list
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_Init(
    HTTP_LIST_T *list_ctx,
    void (*dbg_validate_instance)(HTTP_INSTANCE_T *))
{
    ASSERT(list_ctx != NULL);

    memset(list_ctx, 0, sizeof(*list_ctx));

    list_ctx->ptr2instance = NULL;
    list_ctx->instance2ptr = NULL;
    list_ctx->dbg_validate_instance = dbg_validate_instance;
}

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
    HTTP_SHM_POINTER_T new_node_ptr)
{
    HTTP_INSTANCE_T *list;
    HTTP_INSTANCE_T *node;
    HTTP_INSTANCE_T *new_node;

    ASSERT(NULL != list_ctx);
    ASSERT(HTTP_NIL != list_ctx->list_ptr.type);
    ASSERT(HTTP_NIL != node_ptr.type);
    ASSERT(HTTP_NIL != new_node_ptr.type);

    list = HTTP_LIST_PTR2INSTANCE(list_ctx->list_ptr);
    node = HTTP_LIST_PTR2INSTANCE(node_ptr);
    new_node = HTTP_LIST_PTR2INSTANCE(new_node_ptr);

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    new_node->links.prev.ptr = node_ptr;
    new_node->links.next = node->links.next;

    if (HTTP_NIL == node->links.next.ptr.type)
    {
        list->links.last_node.ptr = new_node_ptr;
    }
    else
    {
        HTTP_INSTANCE_T *next = HTTP_LIST_PTR2INSTANCE(node_ptr);

        HTTP_LIST_VALIDATE_INSTANCE(next);

        next->links.prev.ptr = new_node_ptr;
    }

    node->links.next.ptr = new_node_ptr;
    new_node->links.parent.ptr = list_ctx->list_ptr;

    list_ctx->counter ++;
}

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
    HTTP_INSTANCE_PTR_T new_node)
{
    HTTP_INSTANCE_PTR_T list;

    ASSERT(NULL != list_ctx);

    list = &list_ctx->list_ptr;

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    new_node->links.prev.in = node;
    new_node->links.next = node->links.next;

    if (HTTP_NIL == node->links.next.ptr.type)
    {
        list->links.last_node.in = new_node;
    }
    else
    {
        HTTP_INSTANCE_PTR_T next = node;

        HTTP_LIST_VALIDATE_INSTANCE(next);

        next->links.prev.in = new_node;
    }

    node->links.next.in = new_node;
    new_node->links.parent.in = &list_ctx->list_ptr;

    list_ctx->counter ++;
}

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
    HTTP_SHM_POINTER_T new_node_ptr)
{
    HTTP_INSTANCE_T *list;
    HTTP_INSTANCE_T *node;
    HTTP_INSTANCE_T *new_node;

    ASSERT(NULL != list_ctx);
    ASSERT(HTTP_NIL != list_ctx->list_ptr.type);
    ASSERT(HTTP_NIL != node_ptr.type);
    ASSERT(HTTP_NIL != new_node_ptr.type);

    list = HTTP_LIST_PTR2INSTANCE(list_ctx->list_ptr);
    node = HTTP_LIST_PTR2INSTANCE(node_ptr);
    new_node = HTTP_LIST_PTR2INSTANCE(new_node_ptr);

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    new_node->links.prev = node->links.prev;
    new_node->links.next.ptr = node_ptr;

    if (HTTP_NIL == node->links.prev.ptr.type)
    {
        list->links.first_node.ptr = new_node_ptr;
    }
    else
    {
        HTTP_INSTANCE_T *prev = HTTP_LIST_PTR2INSTANCE(node->links.prev.ptr);

        HTTP_LIST_VALIDATE_INSTANCE(prev);
        prev->links.next.ptr = new_node_ptr;
    }

    node->links.prev.ptr = new_node_ptr;
    new_node->links.parent.ptr = list_ctx->list_ptr;

    list_ctx->counter ++;
}

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
    HTTP_INSTANCE_PTR_T new_node)
{
    HTTP_INSTANCE_PTR_T list;

    ASSERT(NULL != list_ctx);

    list = &list_ctx->list_ptr;

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    new_node->links.prev = node->links.prev;
    new_node->links.next.in = node;

    if (NULL == node->links.prev.in)
    {
        list->links.first_node.in = new_node;
    }
    else
    {
        HTTP_INSTANCE_PTR_T prev = node->links.prev.in;

        HTTP_LIST_VALIDATE_INSTANCE(prev);
        prev->links.next.in = new_node;
    }

    node->links.prev.in = new_node;
    new_node->links.parent.in = &list_ctx->list_ptr;

    list_ctx->counter ++;
}

/**----------------------------------------------------------------------
 * Inserts node before current first node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertBeginning(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T new_node_ptr)
{
    HTTP_INSTANCE_T *list;
    HTTP_INSTANCE_T *new_node;

    ASSERT(NULL != list_ctx);
    ASSERT(HTTP_NIL != list_ctx->list_ptr.type);
    ASSERT(HTTP_NIL != new_node_ptr.type);

    list = HTTP_LIST_PTR2INSTANCE(list_ctx->list_ptr);
    new_node = HTTP_LIST_PTR2INSTANCE(new_node_ptr);

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    if (HTTP_NIL == list->links.first_node.ptr.type)
    {
        list->links.first_node.ptr = new_node_ptr;
        list->links.last_node.ptr  = new_node_ptr;
        new_node->links.parent.ptr = list_ctx->list_ptr;

        list_ctx->counter ++;

        memset(&new_node->links.prev, 0, sizeof(new_node->links.prev));
        memset(&new_node->links.next, 0, sizeof(new_node->links.next));
    }
    else
    {
        HTTP_LIST_SHM_InsertBefore(list_ctx, list->links.first_node.ptr, new_node_ptr);
    }
}

/**----------------------------------------------------------------------
 * Inserts node before current first node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertBeginning(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T new_node)
{
    HTTP_INSTANCE_PTR_T list;

    ASSERT(NULL != list_ctx);

    list = &list_ctx->list_ptr;

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    if (NULL == list->links.first_node.in)
    {
        list->links.first_node.in = new_node;
        list->links.last_node.in  = new_node;
        new_node->links.parent.in = &list_ctx->list_ptr;

        list_ctx->counter ++;

        memset(&new_node->links.prev, 0, sizeof(new_node->links.prev));
        memset(&new_node->links.next, 0, sizeof(new_node->links.next));
    }
    else
    {
        HTTP_LIST_InsertBefore(list_ctx, list->links.first_node.in, new_node);
    }
}

/**----------------------------------------------------------------------
 * Append node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_InsertEnd(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T new_node_ptr)
{
    HTTP_INSTANCE_T *list;
    HTTP_INSTANCE_T *new_node;

    ASSERT(NULL != list_ctx);
    ASSERT(HTTP_NIL != list_ctx->list_ptr.type);
    ASSERT(HTTP_NIL != new_node_ptr.type);

    list = HTTP_LIST_PTR2INSTANCE(list_ctx->list_ptr);
    new_node = HTTP_LIST_PTR2INSTANCE(new_node_ptr);

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    if (HTTP_NIL == list->links.last_node.ptr.type)
    {
        HTTP_LIST_SHM_InsertBeginning(list_ctx, new_node_ptr);
    }
    else
    {
        HTTP_LIST_SHM_InsertAfter(list_ctx, list->links.last_node.ptr, new_node_ptr);
    }
}

/**----------------------------------------------------------------------
 * Append node
 *
 * @param  list_ctx         list
 * @param  new_node_ptr     new node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_InsertEnd(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T new_node)
{
    HTTP_INSTANCE_PTR_T list;

    ASSERT(NULL != list_ctx);

    list = &list_ctx->list_ptr;

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(new_node);

    if (NULL == list->links.last_node.in)
    {
        HTTP_LIST_InsertBeginning(list_ctx, new_node);
    }
    else
    {
        HTTP_LIST_InsertAfter(list_ctx, list->links.last_node.in, new_node);
    }
}

/**----------------------------------------------------------------------
 * Remove node
 *
 * @param  list_ctx         list
 * @param  node_ptr         removed node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_SHM_Remove(
    HTTP_LIST_SHM_T *list_ctx,
    HTTP_SHM_POINTER_T node_ptr)
{
    HTTP_INSTANCE_T *list;
    HTTP_INSTANCE_T *node;

    ASSERT(NULL != list_ctx);
    ASSERT(HTTP_NIL != list_ctx->list_ptr.type);
    ASSERT(HTTP_NIL != node_ptr.type);

    list = HTTP_LIST_PTR2INSTANCE(list_ctx->list_ptr);
    node = HTTP_LIST_PTR2INSTANCE(node_ptr);

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);

    if (HTTP_NIL == node->links.prev.ptr.type)
    {
        list->links.first_node = node->links.next;
    }
    else
    {
        HTTP_INSTANCE_T *prev = HTTP_LIST_PTR2INSTANCE(node->links.prev.ptr);

        HTTP_LIST_VALIDATE_INSTANCE(prev);
        prev->links.next = node->links.next;
    }

    if (HTTP_NIL == node->links.next.ptr.type)
    {
        list->links.last_node = node->links.prev;
    }
    else
    {
        HTTP_INSTANCE_T *next = HTTP_LIST_PTR2INSTANCE(node->links.next.ptr);

        HTTP_LIST_VALIDATE_INSTANCE(next);
        next->links.prev = node->links.prev;
    }

    node->links.parent.ptr.type = HTTP_NIL;

    list_ctx->counter --;

    memset(&node->links.prev, 0, sizeof(node->links.prev));
    memset(&node->links.next, 0, sizeof(node->links.next));
}

/**----------------------------------------------------------------------
 * Remove node
 *
 * @param  list_ctx         list
 * @param  node_ptr         removed node
 * ---------------------------------------------------------------------- */
void
HTTP_LIST_Remove(
    HTTP_LIST_T *list_ctx,
    HTTP_INSTANCE_PTR_T node)
{
    HTTP_INSTANCE_PTR_T list;

    ASSERT(NULL != list_ctx);

    list = &list_ctx->list_ptr;

    HTTP_LIST_VALIDATE_INSTANCE(list);
    HTTP_LIST_VALIDATE_INSTANCE(node);

    if (NULL == node->links.prev.in)
    {
        list->links.first_node = node->links.next;
    }
    else
    {
        HTTP_INSTANCE_PTR_T prev = node->links.prev.in;

        HTTP_LIST_VALIDATE_INSTANCE(prev);
        prev->links.next = node->links.next;
    }

    if (NULL == node->links.next.in)
    {
        list->links.last_node = node->links.prev;
    }
    else
    {
        HTTP_INSTANCE_T *next = node->links.next.in;

        HTTP_LIST_VALIDATE_INSTANCE(next);
        next->links.prev = node->links.prev;
    }

    node->links.parent.in = NULL;

    list_ctx->counter --;

    memset(&node->links.prev, 0, sizeof(node->links.prev));
    memset(&node->links.next, 0, sizeof(node->links.next));
}

