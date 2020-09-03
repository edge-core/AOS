/* MODULE NAME: l_avl.c
 * PURPOSE:
 *   File contains all the api functions for a generic avl tree
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
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
static void L_AVL_CreateAvlTree(
    L_AVL_Tree_T *tree_p,
    L_AVL_TreeTables_T *tree_heap_p,
    void *data_heap_p,
    unsigned int max_entries,
    unsigned int data_length,
    unsigned int type,
    unsigned int length_search_key);
static L_AVL_Comparator_T L_AVL_SetAvlTreeComparator(
    L_AVL_Tree_T *tree_p,
    L_AVL_Comparator_T c);
#if 0 /* unused now. comment out to avoid compilation error */
static int L_AVL_DeleteAvlTree(L_AVL_Tree_T *tree_p);
#endif
static void L_AVL_PurgeAvlTree(L_AVL_Tree_T *avl_tree_p, unsigned int max_entries);


/* STATIC VARIABLE DEFINITIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: L_AVL_InsertEntry
 * PURPOSE: Inserts ITEM into TREE.  Returns NULL if the item was inserted,
 *          otherwise a pointer to the duplicate item
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be inserted
 * OUTPUT:  None
 * RETURN:  0, if item was inserted
 *          void, pointer to a duplicate item, if duplicate exists
 *          void, pointer to item, if error
 * NOTES:   None
 */
void *L_AVL_InsertEntry(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    void * rc_p;

    rc_p = L_AVL_UTIL_AddEntry(avl_tree_p, item_p);

    return(rc_p);
}

/* FUNCTION NAME: L_AVL_DeleteEntry
 * PURPOSE: Searches AVL tree for an item matching ITEM.  If found, the
 *          item is removed from the tree and the actual item found is returned
 *          to the caller.  If no item matching ITEM exists in the tree, returns NULL.
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          item_p -- pointer to item to be inserted
 * OUTPUT:  None
 * RETURN:  void *, pointer to the item if deleted
 *          0, if item does not exist in the tree
 * NOTES:   None
 */
void *L_AVL_DeleteEntry(L_AVL_Tree_T *avl_tree_p, void *item_p)
{
    void * rc_p;

    rc_p = L_AVL_UTIL_RemoveEntry(avl_tree_p, item_p);

    return(rc_p);
}

/* FUNCTION NAME: L_AVL_Search
 * PURPOSE: Searches AVL tree for exact or get next match
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          key_p -- pointer to key
 *          flags -- search flags, AVL_EXACT or AVL_NEXT
 * OUTPUT:  None
 * RETURN:  void *, pointer to the item if matched
 *          0, if item does not exist in the tree
 * NOTES:   None
 */
void *L_AVL_Search(L_AVL_Tree_T *avl_tree_p, void *key_p, unsigned int flags)
{
    int  diff;
    unsigned int  found_equal;
    L_AVL_TreeTables_T *ptr, *saved_node_p;

    ptr = avl_tree_p->root.link_ar[L_AVL_UTIL_L7_LEFT];
    saved_node_p = 0;
    found_equal = 0;

    while (ptr != NULL)
    {
        diff = L_AVL_UTIL_CompareKey(key_p, ptr->data_p,
                    avl_tree_p->length_search_key,
                    avl_tree_p->compare);

        if (diff == L_AVL_UTIL_EQUAL)
        {
            if (flags & L_AVL_EXACT)
            {
                saved_node_p = ptr;
                found_equal = 1;
                break;
            }
            else
            {
                ptr=ptr->link_ar[L_AVL_UTIL_L7_RIGHT];
            }
        }
        else if (diff == L_AVL_UTIL_LESS_THAN)  /*goto left subtree*/
        {
            saved_node_p=ptr;
            ptr=ptr->link_ar[L_AVL_UTIL_L7_LEFT];
        }
        else  /*goto right subtree*/
        {
            ptr=ptr->link_ar[L_AVL_UTIL_L7_RIGHT];
        }
    }   /* ptr */

    if ((found_equal == 1) ||
        ((flags & L_AVL_NEXT) && saved_node_p != 0)) /* if found or doing a get next */
    {
        return saved_node_p->data_p;
    }

    return 0;
}

/* FUNCTION NAME: L_AVL_GetTreeCount
 * PURPOSE: Obtains count of nodes in the tree
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 * OUTPUT:  None
 * RETURN:  count, count of items in the tree
 * NOTES:   None
 */
unsigned int L_AVL_GetTreeCount(L_AVL_Tree_T *avl_tree_p)
{
    return(avl_tree_p->count);
}

/* FUNCTION NAME: L_AVL_CompareShort16
 * PURPOSE: Compare short (16 bit signed keys) and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareShort16(const void *a_p, const void *b_p, size_t len)
{
    int16_t va = *(int16_t *)a_p;
    int16_t vb = *(int16_t *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareLong32
 * PURPOSE: Compare long (32 bit signed keys) and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareLong32(const void *a_p, const void *b_p, size_t len)
{
    int32_t va = *(int32_t *)a_p;
    int32_t vb = *(int32_t *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareUShort16
 * PURPOSE: Compare ushort (16 bit unsigned keys) and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareUShort16(const void *a_p, const void *b_p, size_t len)
{
    int16_t va = *(int16_t *)a_p;
    int16_t vb = *(int16_t *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareULong32
 * PURPOSE: Compare ulong (32 bit unsigned keys) and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareULong32(const void *a_p, const void *b_p, size_t len)
{
    uint32_t va = *(uint32_t *)a_p;
    uint32_t vb = *(uint32_t *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareULongLong64
 * PURPOSE: Compare ulonglong (64 bit unsigned keys) and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareULongLong64(const void *a_p, const void *b_p, size_t len)
{
    uint64_t va = *(uint64_t *)a_p;
    uint64_t vb = *(uint64_t *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareIPAddr
 * PURPOSE: Compare IP Address keys and indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareIPAddr(const void *a_p, const void *b_p, size_t len)
{
    unsigned int va = *(unsigned int *)a_p;
    unsigned int vb = *(unsigned int *)b_p;

    if (va == vb)
    {
        return 0;
    }

    if (va > vb)
    {
        return 1;
    }

    return -1;
}

/* FUNCTION NAME: L_AVL_CompareIPNetAddr
 * PURPOSE: Compare IP Network Address (Address and Mask) keys and
 *          indicate relationship
 * INPUT:   a_p -- Pointer to the first key
 *          b_p -- Pointer to the second key
 *          len -- Length of the key (unused)
 * OUTPUT:  None
 * RETURN:  0, if the keys are equal
 *          1, if key 'a' is greater than key 'b'
 *          -1, if key 'a' is less than key 'b'
 * NOTES:   None
 */
int L_AVL_CompareIPNetAddr(const void *a_p, const void *b_p, size_t len)
{
    unsigned int addr_a = *(unsigned int *)a_p;
    unsigned int mask_a = *(((unsigned int *)a_p) + 1);
    unsigned int addr_b = *(unsigned int *)b_p;
    unsigned int mask_b = *(((unsigned int *)b_p) + 1);

    if (addr_a < addr_b)
    {
        return -1;
    }

    if (addr_a > addr_b)
    {
        return 1;
    }

    if (mask_a < mask_b)
    {
        return -1;
    }

    if (mask_a > mask_b)
    {
        return 1;
    }

    return 0;
}

/* FUNCTION NAME: L_AVL_AllocAndCreateAvlTree
 * PURPOSE: Creates the generic avl tree structure and initialize
 * INPUT:   avl_tree_p -- pointer to the tree structure
 *          max_entries -- maximum number of entries in the tree
 *          data_length -- length of a data entry
 *          type -- type of data
 *          compare_fun -- pointer to the comparator function
 *                         if NULL is used, the default comparator (memcmp()) will be used.
 *          length_search_key -- length of the search key
 * OUTPUT:  None
 * RETURN:  0, if avl tree initialization was successful
 *          -1, if not successful
 * NOTES:   RESTRICTIONS:
 *          1. First field in the data structure (of data heap) must be the key
 *          2. Last field in the data structure (of data heap) must be a void pointer type
 */
int L_AVL_AllocAndCreateAvlTree(
    L_AVL_Tree_T *avl_tree_p,
    unsigned int max_entries,
    unsigned int data_length,
    unsigned int type,
    L_AVL_Comparator_T compare_fun,
    unsigned int length_search_key)
{
    L_AVL_TreeTables_T *avl_tree_table_p;
    void  *avl_tree_data_p;

    /* Allocate the AVL Tree Tables
     */
    avl_tree_table_p = malloc(sizeof(L_AVL_TreeTables_T) * max_entries);

    if (avl_tree_table_p == 0)
    {
        return -1;
    }

    /* Allocate the AVL data heap
     */
    avl_tree_data_p = malloc(data_length * max_entries);

    if (avl_tree_data_p == 0)
    {
        free(avl_tree_table_p);
        return -1;
    }

    L_AVL_CreateAvlTree(avl_tree_p, avl_tree_table_p, avl_tree_data_p,
            max_entries, data_length, type, length_search_key);

    (void)L_AVL_SetAvlTreeComparator(avl_tree_p, compare_fun);
    avl_tree_p->table_heap_p = avl_tree_table_p;
    avl_tree_p->data_heap_p = avl_tree_data_p;

    return 0;
}



/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: L_AVL_CreateAvlTree
 * PURPOSE: Creates the generic avl tree structure and initialize
 * INPUT:   tree_p -- pointer to the tree structure
 *          tree_heap_p -- pointer to the tree heap
 *          data_heap_p -- pointer to the data heap
 *          max_entries -- maximum number of entries in the tree
 *          data_length -- length of a data entry
 *          type -- type of data
 *          length_search_key -- length of the search key
 * OUTPUT:  None
 * RETURN:  L_AVL_Tree_T   pointer to the generic avl tree structure if successful
 *          0     if not successful
 * NOTES:   RESTRICTIONS:
 *          1. First field in the data structure (of data heap) must be the key
 *          2. Last field in the data structure (of data heap) must be a void pointer type
 */
static void L_AVL_CreateAvlTree(
    L_AVL_Tree_T *tree_p,
    L_AVL_TreeTables_T *tree_heap_p,
    void *data_heap_p,
    unsigned int max_entries,
    unsigned int data_length,
    unsigned int type,
    unsigned int length_search_key)
{
    bzero((char *)&tree_p->root, sizeof(L_AVL_TreeTables_T));
    bzero((char *)tree_heap_p, max_entries*sizeof(L_AVL_TreeTables_T));
    bzero((char *)data_heap_p, max_entries*data_length);

    tree_p->type = type;
    tree_p->length_data = data_length;
    tree_p->offset_next = data_length - sizeof(void *);
    tree_p->length_search_key = length_search_key;

    tree_p->initial_table_heap_p = tree_heap_p;
    tree_p->initial_data_heap_p = data_heap_p;
    tree_p->compare = memcmp;
    tree_p->table_heap_p = 0;
    tree_p->data_heap_p = 0;

    L_AVL_PurgeAvlTree(tree_p, max_entries);
}

/* FUNCTION NAME: L_AVL_SetAvlTreeComparator
 * PURPOSE: Set the comparator function for an AVL tree
 * INPUT:   tree_p -- pointer to the tree structure
 *          compare -- pointer to the new comparator function
 *                     if NULL is used, the default comparator (memcmp()) will be used.
 * OUTPUT:  None
 * RETURN:  pointer to the previous comparator (this function does not fail)
 * NOTES:   The default comparator in a generic avl tree is memcmp()
 *          There are also some canned comparators supplied (these are
 *          declared below).  If the user wants to provide a new type
 *          specific comparator it should have the same signature as memcmp():
 *
 *              int bar(void *a, void *b, size_t key_len);
 *
 *          and it should have the following integer return values:
 *
 *              >0 (I generally use 1) if a > b
 *              0  if a == b
 *              <0 if a < b
 *
 *          The algorithm for the comparison and the definitions
 *          of <, >, and == belong entirely to the comparator. The
 *          only requirements are that < and > are consistent, and
 *          == ensures uniqueness within the tree.
 */
static L_AVL_Comparator_T L_AVL_SetAvlTreeComparator(
    L_AVL_Tree_T *tree_p,
    L_AVL_Comparator_T c)
{
    L_AVL_Comparator_T ret = tree_p->compare;

    if (c != 0)
    {
        /* The caller supplied a non-NULL pointer, so use the supplied
         * comparator function.
         */
        tree_p->compare = c;
    }
    else
    {
        /* If the caller supplied a NULL pointer, then restore the default.
         */
        tree_p->compare = memcmp;
    }
    return ret;
}

#if 0 /* unused now. comment out to avoid compilation error */
/* FUNCTION NAME: L_AVL_DeleteAvlTree
 * PURPOSE: Deletes an avl tree structure
 * INPUT:   tree_p -- pointer to the tree structure
 * OUTPUT:  None
 * RETURN:  0, -1, -2
 * NOTES:   None
 */
static int L_AVL_DeleteAvlTree(L_AVL_Tree_T *tree_p)
{
    if (tree_p == 0)
    {
        return -2;
    }

    if (tree_p->table_heap_p != 0)
    {
        free(tree_p->table_heap_p);
    }

    if (tree_p->data_heap_p != 0)
    {
        free(tree_p->data_heap_p);
    }

    return 0;
}
#endif /* unused now. comment out to avoid compilation error */

/* FUNCTION NAME: L_AVL_PurgeAvlTree
 * PURPOSE: Deletes an avl tree structure
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 *          max_entries -- max number of entries in the structure
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void L_AVL_PurgeAvlTree(L_AVL_Tree_T *avl_tree_p, unsigned int max_entries)
{
    unsigned int       i;
    unsigned int       offset_next, length_data;
    L_AVL_TreeTables_T *tree_heap_p;
    void            *data_heap_p;

    offset_next = avl_tree_p->offset_next;
    length_data = avl_tree_p->length_data;

    avl_tree_p->root.link_ar[L_AVL_UTIL_L7_LEFT] = 0;
    avl_tree_p->root.link_ar[L_AVL_UTIL_L7_RIGHT] = 0;
    avl_tree_p->count = 0;
    avl_tree_p->value_p = 0;

    tree_heap_p = avl_tree_p->initial_table_heap_p;
    data_heap_p = avl_tree_p->initial_data_heap_p;

    avl_tree_p->current_table_heap_p = tree_heap_p;
    avl_tree_p->current_data_heap_p = data_heap_p;

    for (i = 0; i < max_entries; i++)
    {
        tree_heap_p[i].link_ar[L_AVL_UTIL_L7_LEFT] = 0;
        tree_heap_p[i].link_ar[L_AVL_UTIL_L7_RIGHT] = &tree_heap_p[i+1];

        *((unsigned long*)((char*)data_heap_p + offset_next)) = (unsigned long)((char*)data_heap_p + length_data);
        data_heap_p = (char*)data_heap_p + length_data;
    }

    tree_heap_p = avl_tree_p->initial_table_heap_p;
    data_heap_p = avl_tree_p->initial_data_heap_p;

    if (i > 0)  /* Added this check to make sure there is no array bound violation */
    {
        tree_heap_p[i-1].link_ar[L_AVL_UTIL_L7_RIGHT] = 0;
        *(unsigned int *)((char*)data_heap_p + ((i-1)*length_data) + offset_next) = 0;
    }
}

