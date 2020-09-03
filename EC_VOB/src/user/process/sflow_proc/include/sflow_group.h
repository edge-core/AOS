/* MODULE NAME:  sflow_group.h
 * PURPOSE:
 *     This file provides APIs for implementations of SFLOW csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    09/08/2009 - Nelson Dai     , Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

#ifndef SFLOW_GROUP_H
#define SFLOW_GROUP_H

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
 * FUNCTION NAME - SFLOW_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for SFLOW group.
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
void SFLOW_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME:  SFLOW_GROUP_Create_All_Threads
 * PURPOSE:
 *    This function will spawn all threads in CSCGroup.
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
 *    All threads in the same CSC group will join the same thread group.
 *
 */
void SFLOW_GROUP_Create_All_Threads(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    Create inter-CSC relationships for WEBGROUP.
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
 *    None.
 *-----------------------------------------------------------------------------
 */
void SFLOW_GROUP_Create_InterCSC_Relation(void);

#endif  /* End of SFLOW_GROUP_H */

