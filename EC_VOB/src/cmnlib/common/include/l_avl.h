/* MODULE NAME: l_avl.h
 * PURPOSE:
 *   File contains all the APIs, declared as external,
 *   needed for an avl tree implementation
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

#ifndef L_AVL_H
#define L_AVL_H

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define L_AVL_EXACT 1
#define L_AVL_NEXT  2


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* @purpose   Function pointer type for AVL tree comparators
 * @notes     This is the prototype that a custom AVL tree comparator
 *            must match.  The arguments 'a' and 'b' are pointers to
 *            space containing a key of the type used by the particular
 *            tree in question.  The 'l' argument is a length specifier
 *            which may or may not be used by the comparator (most
 *            custom comparators will know the length of the keys to
 *            be compared, but some may not).
 *
 *            The comparator function must accept these arguments, and
 *            return a determination of 'a' greater than 'b', 'a'
 *            less than 'b', or 'a' equal to 'b', using locally defined
 *            rules for greater than, less than, and equal to.  The only
 *            constraints on those rules are that greater than and less
 *            than must return a consistent result when given the same
 *            pair of keys in the same order, and equal to must uniquely
 *            identify a key in the domain of all possible keys in the
 *            tree.
 *
 *            NOTE: The use of 'size_t' in this context is specifically
 *                  sanctioned by the CRC since this prototype must
 *                  match the memcmp(3) prototype, and the size_t
 *                  usage here is not problematic.  Because the size
 *                  of a size_t varies widely, there are cases
 *                  in which the use of size_t can cause portability
 *                  concerns.  In particular, those concerns arize over
 *                  bitwise operations or operations that are aware of
 *                  the maximum value that a size_t can represent.
 *                  Neither of these concerns apply here.
 *
 *                  This declaration should not be seen as precedent
 *                  for general use of size_t parameters.
 */
typedef int (*L_AVL_Comparator_T)(const void *a_p, const void *b_p, size_t l);

/* L_AVL_TreeTables_S
 * @purpose     this is the generic tree table structure.
 *              all data structures need to have this defined
 * @notes       this stores all the information needed for the
 *              traversal of the tree with poiters to
 *              the data item and links to its subtrees
 */
typedef struct L_AVL_TreeTables_S
{
  signed char               balance;        /* Balance factor. */
  char                      balance_needed;  /* Used during insertion. */
  struct L_AVL_TreeTables_S *link_ar[2];       /* Subtrees. */
  void                      *data_p;          /* Pointer to data. */
} L_AVL_TreeTables_T;

/* @structures L_AVL_Tree_S
 * @purpose    this is the generic avl tree structure
 * @notes      contains all the information needed for the working
 *             of the avl tree
 */
typedef struct L_AVL_Tree_S
{
  unsigned int         type;         /* type of data to be stored */
  unsigned int         length_data;       /* length of data */
  unsigned int         offset_next;      /* offset to the 'next' member of the data structure*/
  unsigned int         length_search_key;  /* length of the serch key of the data structure*/
  L_AVL_TreeTables_T   *current_table_heap_p;   /* pointer to the current position of the table heap*/
  L_AVL_TreeTables_T   *initial_table_heap_p;   /* pointer to the first position of the table heap*/
  void                 *current_data_heap_p;  /* pointer to the current position of the data heap*/
  void                 *initial_data_heap_p;  /* pointer to the first position of the data heap*/
  L_AVL_TreeTables_T   root;              /* Tree root node. */
  unsigned int         count;      /* Number of nodes in the tree. */
  void                 *value_p;        /* Arbitary user data. */
  L_AVL_Comparator_T   compare;    /* Comparator func ptr (default: memcmp()) */

  /* The following pointers are only set if the memory was allocated   */
  /* by L_AVL_API_AllocAndCreateAvlTree and must be freed by avlDeleteAvlTree */
  L_AVL_TreeTables_T   *table_heap_p;   /* pointer to the table heap */
  void                 *data_heap_p;    /* pointer to the data heap*/
} L_AVL_Tree_T;



/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void *L_AVL_InsertEntry(L_AVL_Tree_T *avl_tree_p, void *item_p);

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
void *L_AVL_DeleteEntry(L_AVL_Tree_T *avl_tree_p, void *item_p);

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
void *L_AVL_Search(L_AVL_Tree_T *avl_tree_p, void *key_p, unsigned int flags);

/* FUNCTION NAME: L_AVL_GetTreeCount
 * PURPOSE: Obtains count of nodes in the tree
 * INPUT:   avl_tree_p -- pointer to the avl tree structure
 * OUTPUT:  None
 * RETURN:  count, count of items in the tree
 * NOTES:   None
 */
unsigned int L_AVL_GetTreeCount(L_AVL_Tree_T *avl_tree_p);

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
int L_AVL_CompareShort16(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareLong32(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareUShort16(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareULong32(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareULongLong64(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareIPAddr(const void *a_p, const void *b_p, size_t len);

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
int L_AVL_CompareIPNetAddr(const void *a_p, const void *b_p, size_t len);

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
    unsigned int length_search_key);

#endif /* End of L_AVL_H */
