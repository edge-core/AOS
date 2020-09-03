/* MODULE NAME:  l2mcast_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mcast group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/12/2007 - Wakka Tu, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef L2MCAST_GROUP_H
#define L2MCAST_GROUP_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init CSC Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *------------------------------------------------------------------------------
 */
void L2MCAST_GROUP_InitiateProcessResource(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2MCAST_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for L2MCAST Group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void L2MCAST_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MCAST_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in L2MCAST group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    All threads in the same L2MCAST group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void L2MCAST_GROUP_Create_All_Threads(void);

#endif /* L2MCAST_GROUP_H */


