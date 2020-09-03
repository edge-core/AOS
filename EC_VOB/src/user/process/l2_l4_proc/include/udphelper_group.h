/* MODULE NAME:  udphelper_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of udphelper group.
 *
 * NOTES:
 *
 * HISTORY
 *    3/30/2009 - LinLi, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

#ifndef _UDPHELPER_GROUP_H_
#define _UDPHELPER_GROUP_H_

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
 * ROUTINE NAME : UDPHELPER_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in UDPHELPER_Group.
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
 *    All threads in the same UDPHELPER group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void UDPHELPER_GROUP_Create_All_Threads(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for UDPHELPER group.
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
void UDPHELPER_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - UDPHELPER_GROUP__Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for UDPHELPER group.
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
void UDPHELPER_GROUP_Create_InterCSC_Relation(void);

#endif

