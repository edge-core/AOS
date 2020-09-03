#ifndef _CGI_LINK_LIST_EX_H_
#define _CGI_LINK_LIST_EX_H_

#include "cgi.h"

#if __cplusplus
extern "C" {
#endif

struct L_listnode *
L_listnode_lookup_key(
    struct L_list *list,
    void *val
);

void
L_list_delete_a_node(
    struct L_list *list,
    struct L_listnode *node
);

#if __cplusplus
}
#endif

#endif /* _CGI_LINK_LIST_EX_H_ */
