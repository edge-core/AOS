/*-----------------------------------------------------------------------------
 * MODULE NAME: CMGR_GROUP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The module implements the fucntionality of CMGR group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2010/11/08     --- Wakka Tu, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2010
 *-----------------------------------------------------------------------------
 */

#ifndef _CMGR_GROUP_H
#define _CMGR_GROUP_H


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
 * FUNCTION NAME - CMGR_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate process resource.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: Create inter-CSC relationships.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will spawn all threads.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void CMGR_GROUP_Create_All_Threads(void);


#endif /* #ifndef _CMGR_GROUP_H */

