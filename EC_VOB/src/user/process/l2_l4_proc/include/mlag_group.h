/* =============================================================================
 * MODULE NAME : MLAG_GROUP.H
 * PURPOSE     : Provide declarations for MLAG CSC group functions.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_GROUP_H
#define MLAG_GROUP_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resources for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME - MLAG_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relations for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_Create_InterCSC_Relation(void);

/* FUNCTION NAME - MLAG_GROUP_Create_All_Threads
 * PURPOSE : Spawn all threads for the CSC group.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void MLAG_GROUP_Create_All_Threads(void);

#endif /* End of MLAG_GROUP_H */
