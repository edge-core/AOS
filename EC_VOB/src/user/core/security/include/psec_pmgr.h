/*-----------------------------------------------------------------------------
 * Module   : psec_pmgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Provide APIs to access port security control functions.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 06/29/2007 - Wakka Tu, Create
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef	PSEC_PMGR_H
#define	PSEC_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "psec_mgr.h"

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_PMGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT	: port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityActionActive
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port state
 * INPUT    : ifindex : the logical port
 * OUTPUT	: action_active
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityActionActive( UI32_T ifindex, UI32_T  *action_active);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLastIntrusionMac
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion mac
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac addrress
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLastIntrusionMac( UI32_T ifindex, UI8_T  *mac);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLastIntrusionTime
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion time
 * INPUT    : ifindex : the logical port
 * OUTPUT   : last intrusion time
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLastIntrusionTime( UI32_T ifindex, UI32_T  *seconds);


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the action status of port security
 * INPUT    : ifindex : the logical port
 * OUTPUT	: action_status
 * RETURN   : TRUE/FALSE
 * NOTE		: none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityActionStatus( UI32_T ifindex, UI32_T  *action_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE		: None
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityStatus( UI32_T ifindex,
                                                                  UI32_T *port_security_status);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action status
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: action_status
 * RETURN   : TRUE/FALSE
 * NOTE		: none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityActionStatus( UI32_T ifindex,
                                                                        UI32_T *action_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityMacCount
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityMacCount(UI32_T ifindex, UI32_T mac_count);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityMacCountOperation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityMacCountOperation(UI32_T ifindex, UI32_T mac_count);

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_PMGR_GetRunningPortSecurityMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security max mac count
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT	: mac_count
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTE		: none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_PMGR_GetRunningPortSecurityMacCount( UI32_T ifindex,
                                                                    UI32_T *mac_count);

/*---------------------------------------------------------------------- */
/* ( PortSecurityMgt 1 )--VS2524 */
/*
 *      INDEX       { portSecPortIndex }
 *      portSecPortEntry ::= SEQUENCE
 *      {
 *          portSecPortIndex      INTEGER,
            portSecPortStatus     INTEGER
 *      }
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetNextPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetNextPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityActionActive
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security port state
 * INPUT   : ifindex                - interface index
 *           action_active         - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityActionActive (UI32_T ifindex, UI32_T action_active);


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityStatus (UI32_T ifindex, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityStatusOperation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityStatusOperation(UI32_T ifindex, UI32_T portsec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityActionStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security action status
 * INPUT   : ifindex                - interface index
 *           action_status - VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityActionStatus (UI32_T ifindex, UI32_T action_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_SetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security learning status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_SetPortSecurityLearningStatus (UI32_T ifindex, UI32_T learning_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_GetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Get the port security learning status
 * INPUT   : ifindex                - interface index
 * OUTPUT  : learning_status        - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_PMGR_GetPortSecurityLearningStatus (UI32_T ifindex, UI32_T *learning_status);

/* Function - PSEC_PMGR_GetPortSecurityMacCount
 * Purpose  - This function will get port auto learning mac counts
 * Input    - ifindex        -- which port to
 * Output   - mac_count      -- mac learning count
 * Return  : TRUE: Successfully, FALSE: If not available
 * Note     -
 */
BOOL_T PSEC_PMGR_GetPortSecurityMacCount( UI32_T ifindex, UI32_T * mac_count );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_PMGR_GetMinNbrOfMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that max-mac-count can be set to.
 * INPUT    : none
 * OUTPUT   : min_number
 * RETURN   : none
 * NOTES    : for 3com CLI & WEB
 * ---------------------------------------------------------------------
 */
void PSEC_PMGR_GetMinNbrOfMaxMacCount(UI32_T *min_number);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_PMGR_ConvertSecuredAddressIntoManual
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
PSEC_PMGR_ConvertSecuredAddressIntoManual(
    UI32_T ifindex
);

#endif /* PSEC_PMGR_H */
