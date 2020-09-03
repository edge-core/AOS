/* MODULE NAME:  sysmgmt_group.h
 * PURPOSE:
 *     This file is a demonstration code for implementations of sysmgmt group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/18/2007 - Echo Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef SYSMGMT_GROUP_H
#define SYSMGMT_GROUP_H

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
 * ROUTINE NAME : SYS_MGMT_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in AUTH_PROTOCOL_Group.
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
 *    All threads in the same SYSMGMT group will join the same thread group.
 *------------------------------------------------------------------------------
 */
void SYS_MGMT_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_GROUP_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will init  CSC Group.
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
void SYS_MGMT_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SYS_MGMT_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for SWCTRL Group.
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
void SYS_MGMT_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_COMM_GetClusterGroupTGHandle
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Get the thread group handler for SYSMGMT Group.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
L_THREADGRP_Handle_T SYS_MGMT_PROC_COMM_GetClusterGroupTGHandle(void);


/*maggie liu for RADIUS authentication ansync*/
void SYS_MGMT_GROUP_HandleRadiusAuthenResult(I32_T result, I32_T privilege, UI32_T cookie);

#endif    /* End of SYSMGMT_GROUP_H */


