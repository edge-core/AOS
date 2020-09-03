/* MODULE NAME:  l4_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of l4 group.
 *
 * NOTES:
 *
 * HISTORY
 *    6/12/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef L4_GROUP_H
#define L4_GROUP_H

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
 * ROUTINE NAME : L4_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in L4_Group.
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
 *    All threads in the same L4 group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void L4_GROUP_Create_All_Threads(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L4_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for STA group.
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
void L4_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L4_GROUP__Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for STA group.
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
void L4_GROUP_Create_InterCSC_Relation(void);

//#define SYS_CPNT_L4 FALSE
#endif    /* End of L4_GROUP_H */


