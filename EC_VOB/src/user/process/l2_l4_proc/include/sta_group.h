/*-----------------------------------------------------------------------------
 * MODULE NAME: STA_GROUP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This module provides the APIs of STA group.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/06/23     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef STA_GROUP_H
#define STA_GROUP_H


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
 * FUNCTION NAME - STA_GROUP_InitiateProcessResources
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
void STA_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Create_InterCSC_Relation
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
void STA_GROUP_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - STA_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in STA group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : All threads in the same CSC group will join the same thread group.
 *-----------------------------------------------------------------------------
 */
void STA_GROUP_Create_All_Threads(void);


#endif /* #ifndef STA_GROUP_H */
