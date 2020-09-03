/* MODULE NAME:  netcfg_group.h
 * PURPOSE:
 *     This file provides APIs for implementations of NETCFG csc group.
 *
 * NOTES:
 *
 * HISTORY
 *    06/12/2007 - Charlie Chen , Created
 *    07/10/2008 - Max Chen     , Modified for porting NETCFG
 *
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef NETCFG_GROUP_H
#define NETCFG_GROUP_H

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


/* FUNCTION NAME:  NETCFG_GROUP_InitiateProcessResources
 * PURPOSE : Initiate process resource for NETCFG group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
void NETCFG_GROUP_InitiateProcessResources(void);

/* FUNCTION NAME:  NETCFG_GROUP_Create_InterCSC_Relation
 * PURPOSE : Create inter-CSC relationships for NETCFG group.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *
 */
void NETCFG_GROUP_Create_InterCSC_Relation(void);

/* FUNCTION NAME:  NETCFG_GROUP_Create_All_Threads
 * PURPOSE:
 *    This function will spawn all threads in NETCFG Group.
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
void NETCFG_GROUP_Create_All_Threads(void);



#endif    /* End of CSCGROUP2_H */

