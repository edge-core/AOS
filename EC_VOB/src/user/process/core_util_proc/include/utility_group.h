/* MODULE NAME:  UTILITY_GROUP.h
 * PURPOSE:
 *     Files for EH group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/07/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef UTILITY_GROUP_H
#define UTILITY_GROUP_H

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
 * FUNCTION NAME - UTILITY_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for UTILITY_Group.
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
void UTILITY_GROUP_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will spawn all threads in EH GROUP.
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
 *
 *------------------------------------------------------------------------------
 */
void UTILITY_GROUP_Create_All_Threads(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_InitiateProcessResource
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
void UTILITY_GROUP_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : UTILITY_GROUP_ProvisionComplete
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will call all CSC in UTILITY_GROUP inform that provision complete
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
 *
 *------------------------------------------------------------------------------
 */
void UTILITY_GROUP_ProvisionComplete(void);

#endif    /* End of UTILITY_GROUP_H */

