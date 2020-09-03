/* MODULE NAME:  netcfg_pmgr_main.h
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_MGR_MAIN and NETCFG_OM_MAIN service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    01/29/2008 - Vai Wang, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */
 
#ifndef NETCFG_PMGR_MAIN_H
#define NETCFG_PMGR_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "netcfg_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : NETCFG_PMGR_MAIN_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource used in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_PMGR_XXX, it should initiate 
 *    the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_PMGR_MAIN_InitiateProcessResource(void);

/*---------------------------------------
 *  System Wise Configuration
 *---------------------------------------
 */

/* FUNCTION NAME - NETCFG_PMGR_MAIN_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 *
 */
void NETCFG_PMGR_MAIN_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* FUNCTION NAME - NETCFG_PMGR_MAIN_HandleHotRemoval
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * 
 */
void NETCFG_PMGR_MAIN_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);


/* FUNCTION NAME : NETCFG_PMGR_MAIN_IsEmbeddedUdpPort
 * PURPOSE:
 *      Check the udp-port is used in protocol engine or not.
 *
 * INPUT:
 *      udp_port -- the udp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_PMGR_MAIN_IsEmbeddedUdpPort(UI32_T udp_port);


/* FUNCTION NAME : NETCFG_PMGR_MAIN_IsEmbeddedTcpPort
 * PURPOSE:
 *      Check the tcp-port is used in protocol engine or not.
 *
 * INPUT:
 *      tcp_port -- the tcp-port to be checked.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    -- this port is used in protocl engine, eg. 520.
 *      FALSE   -- this port is available for sokcet used,
 *                 but possibly already used in socket engine
 *
 * NOTES:
 *      None.
 */
BOOL_T NETCFG_PMGR_MAIN_IsEmbeddedTcpPort(UI32_T tcp_port);


#endif /* NETCFG_PMGR_MAIN_H */

