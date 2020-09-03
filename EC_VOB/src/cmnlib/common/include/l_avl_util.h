/* MODULE NAME: l_avl.h
 * PURPOSE:
 *   File contains defines needed for a avl tree implimentation
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

#ifndef L_AVL_UTIL_H
#define L_AVL_UTIL_H


/* INCLUDE FILE DECLARATIONS
 */
#include "l_avl.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define L_AVL_UTIL_LESS_THAN       -1
#define L_AVL_UTIL_EQUAL            0
#define L_AVL_UTIL_GREATER_THAN     1

#if defined(FUTURE_RELEASE) && FUTURE_RELEASE
/* defines for type of avl tree*/
 #define    MAC_ADDR            1
 #define    QVLAN_ADDR          2
 #define    MAC_FILTER          3
/* -----------------------------*/
#endif

#ifndef L_AVL_UTIL_L7_MAX_DEPTH
#define L_AVL_UTIL_L7_MAX_DEPTH    32
#endif

#define L_AVL_UTIL_L7_LEFT 0
#define L_AVL_UTIL_L7_RIGHT 1
#define L_AVL_UTIL_L7_LEFT_WEIGHT -1
#define L_AVL_UTIL_L7_RIGHT_WEIGHT 1


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: L_AVL_UTIL_CompareKey
 * PURPOSE: this is the comparision function which is used to compare two keys
 * INPUT:   item_p -- pointer to item being searched for
 *          data_item_p -- pointer to data item being searched with
 *          length_search_key -- length of the search key
 *          func -- compare function
 * OUTPUT:  None
 * RETURN:  L_AVL_LESS_THAN, if item's key is less than DataItem's key
 *          L_AVL_GREATER_THAN, 1 if item's key is greater than DataItem's key
 *          L_AVL_EQUAL if item's key is equal to DataItem's key
 * NOTES:   None
 */
int L_AVL_UTIL_CompareKey(
    void *item_p,
    void *data_item_p,
    unsigned int length_search_key,
    L_AVL_Comparator_T func);

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
void *L_AVL_UTIL_RemoveEntry(L_AVL_Tree_T *avl_tree_p, void *item_p);

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
void *L_AVL_UTIL_AddEntry(L_AVL_Tree_T *avl_tree_p, void *item_p);

#endif /* End of L_AVL_H */
