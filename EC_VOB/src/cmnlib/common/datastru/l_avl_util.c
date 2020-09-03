/* MODULE NAME: l_avl_util.c
 * PURPOSE:
 *   File contains all the functions needed for a avl tree implimentation
 * NOTES:
 *   None.
 *
 *
 * HISTORY:
 *    mm/dd/yy
 *    06/11/14 -- Wind, Create
 *
 * Copyright(C) Accton Corporation, 2014
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "l_avl_util.h"
#include "l_avl.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static L_AVL_TreeTables_T *L_AVL_UTIL_NewNewNode(L_AVL_Tree_T *avl_tree_p);
static void* L_AVL_UTIL_NewNewDataNode(L_AVL_Tree_T *avl_tree_p);
static void L_AVL_UTIL_NewFreeDataNode(L_AVL_Tree_T *avl_tree_p, void *item_p);
static void L_AVL_UTIL_NewFreeNode(L_AVL_Tree_T *avl_tree_p, L_AVL_TreeTables_T *root_p);
static L_AVL_TreeTables_T *L_AVL_UTIL_RemoveAndBalance(
    L_AVL_TreeTables_T *node_p,
    int direction,
    unsigned int *done_p);
static void **L_AVL_UTIL_FindEntry(L_AVL_Tree_T *avl_tree_p, void *item_p);
static void L_AVL_UTIL_Balance(
    L_AVL_TreeTables_T *new_node_p,
    L_AVL_TreeTables_T *node_p,
    L_AVL_TreeTables_T *base_node_p,
    L_AVL_Tree_T *avl_tree_p,
    int direction);
static void L_AVL_UTIL_SwapNodes(
    int direction,
    L_AVL_TreeTables_T *old_upper_p,
    L_AVL_TreeTables_T *old_lower_p);
static L_AVL_TreeTables_T *L_AVL_UTIL_RotateNodes(
    int other_direction,
    L_AVL_TreeTables_T *node1_p,
    L_AVL_TreeTables_T *node2_p);


/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: L_AVL_UTIL_CompareKey
 * PURPOSE: this is the comparision function which is used to compare two keys
 * INPUT:   item_p -- pointer to item being searched for
 *          data_item_p -- pointer to data item being searched with
 *          length_search_key -- length of the search key
 *          func -- compare function
 * OUTPUT:  None
 * RETURN:  L_AVL_UTIL_LESS_THAN, if item's key is less than DataItem's key
 *          L_AVL_UTIL_GREATER_THAN, 1 if item's key is greater than DataItem's key
 *          L_AVL_UTIL_EQUAL if item's key is equal to DataItem's key
 * NOTES:   None
 */
int L_AVL_UTIL_CompareKey(
    void *item_p,
    void *data_item_p,
    unsigned int length_search_key,
    L_AVL_Comparator_T func)
{
    int result;

    result = func(item_p, data_item_p, length_search_key);

    if (result < 0)
    {
        return L_AVL_UTIL_LESS_THAN;
    }
    else if (result > 0)
    {
        return L_AVL_UTIL_GREATER_THAN;
    }

    return L_AVL_UTIL_EQUAL;
}

/* FUNCTION NAME: L_AVL_UTIL_RemoveEntry
 * PURPOSE: Searches AVL tree TREE for an item matching ITEM.  If found, the
 *          item is removed from the tree and the actual item found is returned
 *          to the caller.  If no item matching ITEM exists in the tree,
 *          returns NULL.
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be deleted
 * OUTPUT:  None
 * RETURN:  pointer to the item if deleted
 *          NULL if item does not exist in the tree
 * NOTES:   None
 */
void *L_AVL_UTIL_RemoveEntry(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    unsigned int done = 0;
    int depth;
    char path_map_ar[L_AVL_UTIL_L7_MAX_DEPTH+2];
    L_AVL_TreeTables_T *path_ar[L_AVL_UTIL_L7_MAX_DEPTH+2];
    L_AVL_TreeTables_T **node_pp;
    L_AVL_TreeTables_T *node_p = avl_tree_p->root.link_ar[L_AVL_UTIL_L7_LEFT];

    if (node_p == NULL)
    {
        return NULL;
    }

    path_map_ar[L_AVL_UTIL_L7_LEFT] = L_AVL_UTIL_L7_LEFT;
    path_ar[L_AVL_UTIL_L7_LEFT] = &avl_tree_p->root;

    /* Find item to remove
     */
    for (depth = 1; depth < L_AVL_UTIL_L7_MAX_DEPTH; depth++)
    {
        int diff = L_AVL_UTIL_CompareKey(item_p, node_p->data_p,
                      avl_tree_p->length_search_key, avl_tree_p->compare);

        if (diff == L_AVL_UTIL_EQUAL)
        {
            break;
        }

        path_ar[depth] = node_p;

        if (diff == L_AVL_UTIL_GREATER_THAN)
        {
            node_p = node_p->link_ar[L_AVL_UTIL_L7_RIGHT];
            path_map_ar[depth] = L_AVL_UTIL_L7_RIGHT;
        }
        else if (diff == L_AVL_UTIL_LESS_THAN)
        {
            node_p = node_p->link_ar[L_AVL_UTIL_L7_LEFT];
            path_map_ar[depth] = L_AVL_UTIL_L7_LEFT;
        }

        if (node_p == NULL)
        {
            return NULL;
        }
    }

    item_p = node_p->data_p;
    node_pp = &path_ar[depth - 1]->link_ar[(int) path_map_ar[depth - 1]];

    if (node_p->link_ar[L_AVL_UTIL_L7_RIGHT] == NULL)
    {
        *node_pp = node_p->link_ar[L_AVL_UTIL_L7_LEFT];

        if (*node_pp)
        {
            (*node_pp)->balance = 0;
        }
    }
    else
    {
        L_AVL_TreeTables_T *temp_node_p = node_p->link_ar[L_AVL_UTIL_L7_RIGHT];

        if (temp_node_p->link_ar[L_AVL_UTIL_L7_LEFT] == NULL)
        {
            temp_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = node_p->link_ar[L_AVL_UTIL_L7_LEFT];
            temp_node_p->balance = node_p->balance;
            path_map_ar[depth] = 1;
            path_ar[depth++] = temp_node_p;
            *node_pp = temp_node_p;
        }
        else
        {
            L_AVL_TreeTables_T *node2_p = temp_node_p->link_ar[L_AVL_UTIL_L7_LEFT];
            int base_depth = depth++;

            path_map_ar[depth] = 0;
            path_ar[depth++] = temp_node_p;

            while (node2_p->link_ar[L_AVL_UTIL_L7_LEFT] != NULL)
            {
                temp_node_p = node2_p;
                node2_p = temp_node_p->link_ar[L_AVL_UTIL_L7_LEFT];
                path_map_ar[depth] = 0;
                path_ar[depth++] = temp_node_p;
            }

            path_map_ar[base_depth] = 1;
            path_ar[base_depth] = node2_p;
            node2_p->link_ar[L_AVL_UTIL_L7_LEFT] = node_p->link_ar[L_AVL_UTIL_L7_LEFT];
            temp_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = node2_p->link_ar[L_AVL_UTIL_L7_RIGHT];
            node2_p->link_ar[L_AVL_UTIL_L7_RIGHT] = node_p->link_ar[L_AVL_UTIL_L7_RIGHT];
            node2_p->balance = node_p->balance;
            *node_pp = node2_p;
        }
    }

    L_AVL_UTIL_NewFreeNode(avl_tree_p, node_p);

    while ((--depth) && (done == 0))
    {
      L_AVL_TreeTables_T *temp_node_p;
      temp_node_p = L_AVL_UTIL_RemoveAndBalance(path_ar[depth], path_map_ar[depth], &done);

      if (temp_node_p != NULL)
      {
          path_ar[depth-1]->link_ar[(int)path_map_ar[depth-1]] = temp_node_p;
      }
    }

    avl_tree_p->count--;
    L_AVL_UTIL_NewFreeDataNode(avl_tree_p, item_p);
    return(void *)item_p;
}

/* FUNCTION NAME: L_AVL_UTIL_AddEntry
 * PURPOSE: Inserts ITEM into TREE.  Returns NULL if the item was inserted,
 *          otherwise a pointer to the duplicate item
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be inserted
 * OUTPUT:  None
 * RETURN:  NULL if item was inserted
 *          void  pointer to duplicate item if duplicate exists
 *          void  pointer to item, if error
 * NOTES:   None
 */
void *L_AVL_UTIL_AddEntry(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    void **node_pp;
    void *new_entry_p;
    void *exact_entry_p;

    new_entry_p = L_AVL_UTIL_NewNewDataNode(avl_tree_p);

    if (new_entry_p == NULL)
    {
        /* check for the condition that the data heap is full
         * but the entry is already present in the tree
         */
        exact_entry_p = L_AVL_Search(avl_tree_p, item_p, L_AVL_EXACT);

        if (exact_entry_p != 0)
        {
            return exact_entry_p;
        }
        else
        {
            return item_p;
        }
    }

    memcpy(new_entry_p, item_p, avl_tree_p->length_data);

    node_pp = L_AVL_UTIL_FindEntry (avl_tree_p, new_entry_p);

    if (*node_pp != new_entry_p) /*entry not added*/
    {
        L_AVL_UTIL_NewFreeDataNode(avl_tree_p, new_entry_p);
        return(*node_pp);
    }
    else
    {
        return(NULL);
    }
}



/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: L_AVL_UTIL_NewNewNode
 * PURPOSE: Get space for a new node from the tree heap and updates the current
 *          table heap pointer in the avl tree structure
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 * OUTPUT:  None
 * RETURN:  L_AVL_TreeTables_T, pointer to new node
 * NOTES:   None
 */
static L_AVL_TreeTables_T *L_AVL_UTIL_NewNewNode(L_AVL_Tree_T *avl_tree_p)
{
    L_AVL_TreeTables_T* root_p;

    root_p = avl_tree_p->current_table_heap_p;

    if (root_p == NULL)
    {
      return NULL;
    }

    avl_tree_p->current_table_heap_p = root_p->link_ar[1];

    return root_p;
}

/* FUNCTION NAME: L_AVL_UTIL_NewNewDataNode
 * PURPOSE: Get space for a new node from the data heap and updates the current
 *          data heap pointer in the avl tree structure
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 * OUTPUT:  None
 * RETURN:  void pointer to new node
 * NOTES:   None
 */
static void* L_AVL_UTIL_NewNewDataNode(L_AVL_Tree_T *avl_tree_p)
{
    void *item_p;
    unsigned int offset_next;

    offset_next = avl_tree_p->offset_next;
    item_p = avl_tree_p->current_data_heap_p;

    if (item_p == NULL)
    {
        return NULL;
    }

    avl_tree_p->current_data_heap_p = (void *)(*((unsigned long *)((char *)item_p+offset_next)));
    return item_p;
}

/* FUNCTION NAME: L_AVL_UTIL_NewFreeDataNode
 * PURPOSE: Gives memory data node back to data heap and updates the current
 *          data heap pointer in the avl tree structure
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be freed
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void L_AVL_UTIL_NewFreeDataNode(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    unsigned int offset_next;

    offset_next = avl_tree_p->offset_next;
    bzero((unsigned char *)item_p, avl_tree_p->length_data);
    *((unsigned long *)((char *)item_p + offset_next)) = (unsigned long)(avl_tree_p->current_data_heap_p);
    avl_tree_p->current_data_heap_p = item_p;
}

/* FUNCTION NAME: L_AVL_UTIL_NewFreeNode
 * PURPOSE: Gives memory of node back to table heap and updates the current
 *          table heap pointer in the avl tree structure
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          root_p -- pointer to the item stored in the table heap
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void L_AVL_UTIL_NewFreeNode(L_AVL_Tree_T *avl_tree_p, L_AVL_TreeTables_T *root_p)
{
    bzero((unsigned char *)root_p, sizeof(L_AVL_TreeTables_T));
    root_p->link_ar[1]= avl_tree_p->current_table_heap_p;
    avl_tree_p->current_table_heap_p = root_p;
}

/* FUNCTION NAME: L_AVL_UTIL_RemoveAndBalance
 * PURPOSE: Balances the avl tree after a node has been removed.
 * INPUT:   node_p -- pointer to the node being acted upon
 *          direction -- direction to balance the tree
 *          done_p -- indication of whether balancing is complete
 * OUTPUT:  None
 * RETURN:  pointer to a node
 * NOTES:   None
 */
static L_AVL_TreeTables_T *L_AVL_UTIL_RemoveAndBalance(
    L_AVL_TreeTables_T *node_p,
    int direction,
    unsigned int *done_p)
{
    int other_direction, weight, other_weight;
    L_AVL_TreeTables_T *temp_node = NULL;

    other_direction = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_RIGHT : L_AVL_UTIL_L7_LEFT;
    weight = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_LEFT_WEIGHT : L_AVL_UTIL_L7_RIGHT_WEIGHT;
    other_weight = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_RIGHT_WEIGHT : L_AVL_UTIL_L7_LEFT_WEIGHT;
    direction = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_LEFT : L_AVL_UTIL_L7_RIGHT;

    if (node_p->balance == weight)
    {
        node_p->balance = 0;
    }
    else if (node_p->balance == 0)
    {
        node_p->balance = other_weight;
        *done_p = 1;
    }
    else
    {
        temp_node = node_p->link_ar[other_direction];

        if (temp_node == 0)
        {
            /* This should never happen.
             */
            *done_p = 1;
            return temp_node;
        }

        if ((temp_node->balance == 0) ||
            ((direction == L_AVL_UTIL_L7_RIGHT) && (temp_node == NULL)))
        {
            node_p->link_ar[other_direction] = temp_node->link_ar[direction];
            temp_node->link_ar[direction] = node_p;
            temp_node->balance = weight;
            *done_p = 1;
        }
        else if (temp_node->balance == other_weight)
        {
            L_AVL_UTIL_SwapNodes(other_direction, node_p, temp_node);
        }
        else
        {
            temp_node = L_AVL_UTIL_RotateNodes(other_direction, node_p, temp_node);
        }
    }
    return temp_node;
}

/* FUNCTION NAME: L_AVL_UTIL_FindEntry
 * PURPOSE: Search TREE for an item matching ITEM.  If found, returns a pointer
 *          to the address of the item.  If none is found, ITEM is inserted
 *          into the tree, and a pointer to the address of ITEM is returned.
 *          In either case, the pointer returned can be changed by the caller,
 *          or the returned data item can be directly edited, but the key data
 *          in the item must not be changed
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be found
 * OUTPUT:  None
 * RETURN:  pointer to the item if found or inserted
 * NOTES:   None
 */
static void **L_AVL_UTIL_FindEntry(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    int depth;
    L_AVL_TreeTables_T *new_node_p = 0, *node2_p;
    L_AVL_TreeTables_T *base_node_p = &avl_tree_p->root;
    L_AVL_TreeTables_T *node_p;

    node_p = node2_p = base_node_p->link_ar[L_AVL_UTIL_L7_LEFT];

    /* empty tree case
     */
    if (node_p == NULL)
    {
        avl_tree_p->count++;
        new_node_p = base_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = L_AVL_UTIL_NewNewNode(avl_tree_p);
        new_node_p->data_p = item_p;
        new_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = NULL;
        new_node_p->link_ar[L_AVL_UTIL_L7_RIGHT] = NULL;
        new_node_p->balance = L_AVL_UTIL_L7_LEFT;
        return((void *)(&new_node_p->data_p));
    }

    /* find match and return or create new node and break
     */
    for (depth = 0; depth < L_AVL_UTIL_L7_MAX_DEPTH; depth++)
    {
        int diff = L_AVL_UTIL_CompareKey(item_p, node2_p->data_p,
                    avl_tree_p->length_search_key, avl_tree_p->compare);

        /* Traverse down left side of tree.
         */
        if (diff == L_AVL_UTIL_LESS_THAN)
        {
            node2_p->balance_needed = 0;
            new_node_p = node2_p->link_ar[L_AVL_UTIL_L7_LEFT];

            if (new_node_p == NULL)
            {
                new_node_p = L_AVL_UTIL_NewNewNode(avl_tree_p);
                node2_p->link_ar[L_AVL_UTIL_L7_LEFT] = new_node_p;
                break;
            }
        }
        /* Traverse down right side of tree
         */
        else if (diff == L_AVL_UTIL_GREATER_THAN)
        {
            node2_p->balance_needed = 1;
            new_node_p = node2_p->link_ar[L_AVL_UTIL_L7_RIGHT];

            if (new_node_p == NULL)
            {
                new_node_p = L_AVL_UTIL_NewNewNode(avl_tree_p);
                node2_p->link_ar[L_AVL_UTIL_L7_RIGHT] = new_node_p;
                break;
            }
        }
        /* Found it
         */
        else
        {
            return((void *)(&node2_p->data_p));
        }

        if (new_node_p->balance != L_AVL_UTIL_L7_LEFT)
        {
            base_node_p = node2_p; /* shift nodes */
            node_p = new_node_p;
        }
        node2_p = new_node_p;
    }

    avl_tree_p->count++;
    new_node_p->data_p = item_p;
    new_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = NULL;
    new_node_p->link_ar[L_AVL_UTIL_L7_RIGHT] = NULL;
    new_node_p->balance = L_AVL_UTIL_L7_LEFT;

    L_AVL_UTIL_Balance(new_node_p, node_p, base_node_p, avl_tree_p, (int)node_p->balance_needed);

    return((void *)(&new_node_p->data_p));
}

/* FUNCTION NAME: L_AVL_UTIL_Balance
 * PURPOSE: This function will balance the added nodes.
 * INPUT:   new_node_p -- pointer to the new avl tree structure
 *          node_p -- pointer to avl tree structure
 *          base_node_p -- pointer to base avl tree structure
 *          avl_tree_p -- pointer to the avl tree structure
 *          direction -- balance direction left or right
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void L_AVL_UTIL_Balance(
    L_AVL_TreeTables_T *new_node_p,
    L_AVL_TreeTables_T *node_p,
    L_AVL_TreeTables_T *base_node_p,
    L_AVL_Tree_T *avl_tree_p,
    int direction)
{
    L_AVL_TreeTables_T *shift_node_p, *temp_node_p;
    int weight = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_LEFT_WEIGHT: L_AVL_UTIL_L7_RIGHT_WEIGHT;

    shift_node_p = node_p->link_ar[direction];

    while (shift_node_p != new_node_p)
    {
        shift_node_p->balance = (shift_node_p->balance_needed * 2 - 1);
        shift_node_p = shift_node_p->link_ar[(int) shift_node_p->balance_needed];
    }

    if (((weight == L_AVL_UTIL_L7_LEFT_WEIGHT) && (node_p->balance >= 0)) ||
        ((weight == L_AVL_UTIL_L7_RIGHT_WEIGHT) && (node_p->balance <= 0)))
    {
        node_p->balance += weight;
        return;
    }

    temp_node_p = node_p->link_ar[direction];

    if (temp_node_p->balance == weight)
    {
        shift_node_p = temp_node_p;
        L_AVL_UTIL_SwapNodes(direction, node_p, temp_node_p);
    }
    else
    {
        shift_node_p = L_AVL_UTIL_RotateNodes(direction, node_p, temp_node_p);
    }

    if (base_node_p != &avl_tree_p->root && node_p == base_node_p->link_ar[L_AVL_UTIL_L7_RIGHT])
    {
        base_node_p->link_ar[L_AVL_UTIL_L7_RIGHT] = shift_node_p;
    }
    else
    {
        base_node_p->link_ar[L_AVL_UTIL_L7_LEFT] = shift_node_p;
    }

    return;
}

/* FUNCTION NAME: L_AVL_UTIL_SwapNodes
 * PURPOSE: This function will swap the depth of the two imput nodes.
*           The node that is input as oldUpper will become the new lower
*           node, and oldLower will become the new Upper node.  This
*           function will cause a shift in the tree balance.  The
*           direction parameter indicates which direction to shift.
 * INPUT:   direction -- left or right
 *          old_upper_p -- upper node in table
 *          old_lower_p -- lower node in table
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void L_AVL_UTIL_SwapNodes(
    int direction,
    L_AVL_TreeTables_T *old_upper_p,
    L_AVL_TreeTables_T *old_lower_p)
{
    int other_direction = (direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_RIGHT: L_AVL_UTIL_L7_LEFT;

    old_upper_p->link_ar[direction] = old_lower_p->link_ar[other_direction];
    old_lower_p->link_ar[other_direction] = old_upper_p;
    old_upper_p->balance = 0;
    old_lower_p->balance = 0;
}

/* FUNCTION NAME: L_AVL_UTIL_RotateNodes
 * PURPOSE: This function will rotate the nodes to the left or right
 *          depending on the value of diretion.  This
 *          function will cause a shift in the tree balance.  The
 *          direction parameter indicates which direction to shift.
 * INPUT:   other_direction -- direction to rotate
 *          node1_p -- node in table
 *          node2_p -- node in table
 * OUTPUT:  None
 * RETURN:  pointer to rotated node.
 * NOTES:   None
 */
static L_AVL_TreeTables_T *L_AVL_UTIL_RotateNodes(
    int other_direction,
    L_AVL_TreeTables_T *node1_p,
    L_AVL_TreeTables_T *node2_p)
{
    L_AVL_TreeTables_T *node3_p;
    int direction = (other_direction == L_AVL_UTIL_L7_LEFT) ? L_AVL_UTIL_L7_RIGHT: L_AVL_UTIL_L7_LEFT;

    node3_p = node2_p->link_ar[direction];
    node2_p->link_ar[direction] = node3_p->link_ar[other_direction];
    node3_p->link_ar[other_direction] = node2_p;
    node1_p->link_ar[other_direction] = node3_p->link_ar[direction];
    node3_p->link_ar[direction] = node1_p;

    node1_p->balance = node2_p->balance = 0;

    if (direction == L_AVL_UTIL_L7_RIGHT)
    {
        if (node3_p->balance < 0)
        {
            node1_p->balance = 1;
        }
        else if (node3_p->balance > 0)
        {
            node2_p->balance = -1;
        }
    }
    else
    {
        if (node3_p->balance > 0)
        {
            node1_p ->balance = -1;
        }
        else if (node3_p->balance < 0)
        {
            node2_p->balance = 1;
        }
    }

    node3_p->balance = 0;
    return node3_p;
}
