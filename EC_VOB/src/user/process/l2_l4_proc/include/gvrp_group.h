/*-----------------------------------------------------------------------------
 * MODULE NAME: GVRP_GROUP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    None.
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

#ifndef GVRP_GROUP_H
#define GVRP_GROUP_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate process resource for GVRP group.
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
void GVRP_GROUP_InitiateProcessResources(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Create inter-CSC relationships for GVRP group.
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
void GVRP_GROUP_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will spawn all threads in GVRP group.
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
void GVRP_GROUP_Create_All_Threads(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - GVRP_GROUP_MACTableDeleteByVIDnPortCallbackHandler
 *-----------------------------------------------------------------------------
 * PURPOSE : Notify other CSC when detecting a new neighbor.
 *
 * INPUT   : lport                --
 *           mac_addr             --
 *           network_addr_subtype --
 *           network_addr         --
 *           network_addr_len     --
 *           network_addr_ifindex --
 *           tel_exist            --
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void GVRP_GROUP_TelephoneDetectCallbackHandler(UI32_T lport,
                                               UI8_T *mac_addr,
                                               UI8_T network_addr_subtype,
                                               UI8_T *network_addr,
                                               UI8_T network_addr_len,
                                               UI32_T network_addr_ifindex,
                                               BOOL_T tel_exist);


#endif /* #ifndef GVRP_GROUP_H */
