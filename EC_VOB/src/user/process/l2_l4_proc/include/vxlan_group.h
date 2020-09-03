/* MODULE NAME: vxlan_group.h
 * PURPOSE:
 *   The module implements the fucntionality of VXLAN group.
 * NOTES:
 *   None
 *
 * HISTORY:
 *    mm/dd/yy
 *    04/21/15 -- Kelly, Create
 *
 * Copyright(C) Accton Corporation, 2015
 */

#ifndef _VXLAN_GROUP_H
#define _VXLAN_GROUP_H


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
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VXLAN_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate process resource.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void VXLAN_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VXLAN_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: Create inter-CSC relationships.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void VXLAN_GROUP_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VXLAN_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will spawn all threads.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void VXLAN_GROUP_Create_All_Threads(void);


#endif /* #ifndef _VXLAN_GROUP_H */

