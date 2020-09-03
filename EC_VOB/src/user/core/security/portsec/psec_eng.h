/* Module Name: PSEC_ENG.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the hardware independent interface of port security
 *         control functions to applications.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port security manipulation.
 *        ( 3.  The domain would not be handled by this module. )
 *         None.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *      2002/5/30    Arthur Wu   Create this file
 *
 *
 * Copyright(C)      Accton Corporation, 2002
 */
#ifndef PSEC_ENG_H
#define PSEC_ENG_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT
 */

/* MACRO DEFINITIONS
 */

/* TYPE DECLARATIONS
 */
typedef struct
{
    /* key */
    UI32_T vid;
    UI8_T  mac[6];

    UI32_T ifindex;
} PSEC_ENG_PortSecAddrEntry_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_ENG_Init (void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security status
 * INPUT    : ifindex : the logical port
 *            psec_status : VAL_portSecPortStatus_enabled
 *                          VAL_portSecPortStatus_disabled
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: It should call from UI config. So it updates AMTR port info, too.
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityStatus(UI32_T ifindex, UI32_T psec_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityStatus_Callback
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security status
 * INPUT    : ifindex : the logical port
 *            psec_status : VAL_portSecPortStatus_enabled
 *                          VAL_portSecPortStatus_disabled
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: It should call from netaccess group only. And, it doesn't update
 *            AMTR port info because it only tries to change port status and
 *            delete MAC addresses if necessary.
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityStatus_Callback(UI32_T ifindex, UI32_T psec_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityMaxMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security max mac count
 * INPUT    : ifindex   : the logical port
 *            mac_count : max-mac-count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE	    : None
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityMaxMacCount(UI32_T ifindex, UI32_T mac_count);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security action status
 * INPUT    : ifindex : the logical port
 *            action_status: VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_ENG_SetPortSecurityActionStatus( UI32_T ifindex, UI32_T  action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT	: port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_ENG_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_GetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->vid  -- vid
 *              port_sec_addr_entry->mac  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_GetPortSecAddrEntry(PSEC_ENG_PortSecAddrEntry_T *port_sec_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_GetNextPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified next addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->vid  -- vid
 *              port_sec_addr_entry->mac  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_GetNextPortSecAddrEntry(PSEC_ENG_PortSecAddrEntry_T *port_sec_addr_entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_SetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to create/update/remove port scure address
 *              entry.
 * INPUT    :   vid -- vid
 *              mac -- mac
 *              ifindex -- ifindex
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   ifindex = 0 => remove the entry
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecAddrEntry (   UI32_T vid,
                                        UI8_T *mac,
                                        UI32_T ifindex );

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_DeleteAllSecAddress
 *------------------------------------------------------------------------
 * FUNCTION: Delete all secured MAC address on the specified port
 * INPUT   : ifindex    -- ifindex
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_DeleteAllSecAddress(UI32_T ifindex);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_ConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
PSEC_ENG_ConvertSecuredAddressIntoManual(
    UI32_T ifindex
);

#endif /* End of PSEC_ENG_H */
