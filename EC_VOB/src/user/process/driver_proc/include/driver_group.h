/* MODULE NAME:  driver_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of l2mux group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/19/2007 - KH shi, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef DRIVER_GROUP_H
#define DRIVER_GROUP_H

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
 * ROUTINE NAME : DRIVER_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in L2MUX_Group.
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
 *    All threads in the same L2MUX group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void DRIVER_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : DRIVER_GROUP_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  L2MUX Group.
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
void DRIVER_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - DRIVER_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for L2MUX Group.
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
void DRIVER_GROUP_Create_InterCSC_Relation(void);

#endif
