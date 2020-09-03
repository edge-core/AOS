#include "cgi.h"

struct L_listnode *
L_listnode_lookup_key(struct L_list *list, void *val)
{
    struct L_listnode *node;

    if (!list->cmp) {
        return NULL;
    }

    for (node = list->head; node; L_NEXTNODE(node)) {
        if (list->cmp(val, L_GETDATA(node)) == 0)
            return node;
    }

    return NULL;
}

// TODO: llist_utils.c
void
L_list_delete_a_node(struct L_list *list, struct L_listnode *node)
{
    if (list->del)
        list->del(L_GETDATA(node));

    L_list_delete_node(list, node);
}