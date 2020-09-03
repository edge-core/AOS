/* MODULE NAME:  ospf6_group.h
 * PURPOSE:
 *     This file provides APIs for implementations of OSPF csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    7/13/2009 - Steven.Gao , Created *    11/27/2008 - Lin.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

#ifndef OSPF6_GROUP_H
#define OSPF6_GROUP_H

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
 * FUNCTION NAME - OSPF6_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for OSPF6 group.
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
void OSPF6_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME:  OSPF_GROUP_Create_All_Threads
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
void OSPF6_GROUP_Create_All_Threads(void);
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - OSPF6_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for OSPF group.
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
void OSPF6_GROUP_Create_InterCSC_Relation(void);

#endif  /* End of OSPF6_GROUP_H */


