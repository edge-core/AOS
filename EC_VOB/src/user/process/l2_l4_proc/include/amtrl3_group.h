/* MODULE NAME:  amtrl3_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of amtrl3 group.
 *
 * NOTES:
 *
 * HISTORY
 *    1/22/2008 - djd, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

#ifndef _AMTRL3_GROUP_H_
#define _AMTRL3_GROUP_H_

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
 * ROUTINE NAME : AMTRL3_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in AMTRL3_Group.
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
 *    All threads in the same AMTRL3 group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void AMTRL3_GROUP_Create_All_Threads(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for AMTRL3 group.
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
void AMTRL3_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_GROUP__Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for AMTRL3 group.
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
void AMTRL3_GROUP_Create_InterCSC_Relation(void);

#endif
