/*-----------------------------------------------------------------------------
 * Module Name: xstp_uty.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Definitions for the XSTP utility
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/20/2001 - Allen Cheng, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef _XSTP_UTY_H
#define _XSTP_UTY_H

#include "sys_cpnt.h"
#include "xstp_om.h"

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
typedef struct XSTP_UTY_BpduGuardPortList_S
{
    UI32_T                              lport;
    struct XSTP_UTY_BpduGuardPortList_S *next;
} XSTP_UTY_BpduGuardPortList_T;
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetChangeStatePortListForbidden
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will set the flag which controls whether the
 *            XSTP_UTY_ChangeStatePortList is allowed to be added new
 *            element.
 * INPUT    : flag    -- TRUE:disallowed, FALSE:allowed
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetChangeStatePortListForbidden(BOOL_T flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RetrieveChangeStateLportList
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will retrieve the lport list which enters/leaves
 *            the forwarding state. Then the XSTP_UTY_ChangeStatePortList
 *            is cleared.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : XSTP_UTY_ChangeStatePortList
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
XSTP_TYPE_LportList_T    *XSTP_UTY_RetrieveChangeStateLportList(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NotifyLportEnterForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will record the lport number which enters the
 *            the forwarding state.
 * INPUT    : xstid             -- index of the spanning tree
 *            lport             -- lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_NotifyLportEnterForwarding(UI32_T xstid, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NotifyLportLeaveForwarding
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will record the lport number which leaves the
 *            forwarding state.
 * INPUT    : sta_index         -- index of the spanning tree
 *            lport             -- lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_NotifyLportLeaveForwarding(UI32_T xstid, UI32_T lport);


/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1W */
#ifdef  XSTP_TYPE_PROTOCOL_RSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Cist
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE only for CIST state machines, i.e. FALSE for MSTI state
 *            machine instances.
 * INPUT    : om_ptr        -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.2, IEEE Std 802.1s(D13)-2002.
 *            TRUE returned for RSTP
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_Cist(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearReselectBridge
 * ------------------------------------------------------------------------
 * PURPOSE  : Clear reselect bridge
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.1, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_ClearReselectBridge(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  : Disable forwarding
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.2, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.3, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.4, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.5, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Flush
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.6, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_Flush(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewTcWhile
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   :
 * RETURN   : tc_while  -- topology change timer
 * NOTE     : Ref to the description in 17.19.7, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewTcWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewEdgeDelayWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Return the edge delay upon a port's link type.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : edge_delay_while  -- edge delay timer
 * RETURN   : None
 * NOTE     : Ref to the description in 17.20.4 IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewEdgeDelayWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvBpdu
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   :
 * NOTE     : Ref to the description in 17.19.8, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_RcvBpdu(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposed
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.9, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RecordProposed(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSyncBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.10, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSyncBridge(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetReRootBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.11, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetReRootBridge(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSelectedBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.12, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSelectedBridge(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcFlags
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.13, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcFlags(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcPropBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.14, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcPropBridge(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.15, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxRstp
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.16, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxRstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxTcn
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.17, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxTcn(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtBpduVersion
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.18, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtBpduVersion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhile
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.19, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRcvdInfoWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRoleDisabledBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.20, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRoleDisabledBridge(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesBridge
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 17.19.21, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRolesBridge(XSTP_OM_InstanceData_T *om_ptr);

/* ===================================================================== */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer(UI16_T *timer);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer32
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer32(UI32_T *timer);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReselectForAnyPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if the reselect variable of any port is set
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE if the reselect variable of any port is TRUE, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReselectForAnyPort(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_AllSyncedForOthers
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports except the specified one are synced
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports other than the specified one are synced,
 *            else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_AllSyncedForOthers(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReRootedForOthers
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports except the specified one are synced
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports other than the specified one are synced,
 *            else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReRootedForOthers(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_BetterOrSameInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE if the received CIST priority vector is better
 *               than or the same as (13.10) the CIST port priority vector.
 *            2. returns TRUE if the MSTI priority vector is better than or
 *               the same as (13.11) the MSTI port priority vector.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.26.1, 13.26.2,
 *            IEEE Std 802.1s(D14.1)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_BetterOrSameInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

#endif /* XSTP_TYPE_PROTOCOL_RSTP */

/* ===================================================================== */
/* ===================================================================== */
/* IEEE 802.1S */
#ifdef  XSTP_TYPE_PROTOCOL_MSTP

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_AllSynced
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if all ports are synced for the given tree.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE if all ports are synced for the given tree,
 *            else FALSE
 * NOTE     : Ref to the description in 13.25.1, IEEE Std 802.1Q-2005
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_AllSynced(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Cist
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE only for CIST state machines, i.e. FALSE for MSTI state
 *            machine instances.
 * INPUT    : om_ptr        -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.2, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_Cist(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_CistRootPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the CIST role for the given Port is RootPort.
 *            Return FALSE otherwise.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.25.3, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_CistRootPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_CistDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the CIST role for the given Port is
 *            DesignatedPort. Return FALSE otherwise.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.25.4, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_CistDesignatedPort(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiRootPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the role for any MSTI for the given Port is
 *            root port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.5, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiRootPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiDesignatedPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the role for any MSTI for the given Port is
 *            designated port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.6, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiDesignatedPort(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvdAnyMsg
 * ------------------------------------------------------------------------
 * PURPOSE  : return TRUE for a given Port if rcvd_msg is TRUE for the CIST
 *            or any MSTI for that Port, else FALSE.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.7, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RcvdAnyMsg(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvdXstInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE for a given Port if and only if rcvd_msg is TRUE
 *               for the CIST for that port.
 *            2. return TRUE for a given port and MSTI if and only if rcvd_msg
 *               is FALSE for the CIST for that port and rcvd_msg is TRUE for
 *               the MSTI for that port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.8, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.25.9, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RcvdXstInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReRootedForOthers
 * ------------------------------------------------------------------------
 * PURPOSE  : TRUE if the rr_while timer is clear for all ports for the
 *            given tree other than the given port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            this_lport    -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.10, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReRooted(XSTP_OM_InstanceData_T *om_ptr, UI32_T this_lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtXstInfo
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE for a given port if and only if updt_info is
 *               TRUE for the CIST for that port.
 *            2. return TRUE for a given Port and MSTI if and only if
 *               updt_info is TRUE for the MSTI for that port or either
 *               updt_info or selected are TRUE for the CIST for that port.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.25.11, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.25.12, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_UpdtXstInfo(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxTcn
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.a, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.17, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxTcn(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  : Disable forwarding
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.b, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.2, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DisableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  : Disable learning
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.c, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.3, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_DisableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableForwarding
 * ------------------------------------------------------------------------
 * PURPOSE  : Enable forwarding
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.d, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.4, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableForwarding(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_EnableLearning
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.e, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.5, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_EnableLearning(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_Flush
 * ------------------------------------------------------------------------
 * PURPOSE  : remove all Dynamic Filtering Entries in the Filtering
 *            Database that contain information learned on this port,
 *            unless this Port is an edge Port.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.f, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_Flush(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtBpduVersion
 * ------------------------------------------------------------------------
 * PURPOSE  : Set rcvd_stp TRUE if the BPDU received is a version 0 or
 *            version 1 PDU, either a TCN or a Config BPDU. It sets rcvd_rstp
 *            TRUE if the received BPDU is an RST BPDU and (ForceVersion >= 2).
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.g, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 17.19.18, IEEE Std 802.1w-2001
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtBpduVersion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_BetterOrSameInfoXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. return TRUE if the received CIST priority vector is better
 *               than or the same as (13.10) the CIST port priority vector.
 *            2. returns TRUE if the MSTI priority vector is better than or
 *               the same as (13.11) the MSTI port priority vector.
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.26.1, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.2, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_BetterOrSameInfoXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearAllRcvdMsgs
 * ------------------------------------------------------------------------
 * PURPOSE  : Clears rcvd_msg for the CIST and all MSTIs, for all Ports.
 * INPUT    : om_ptr        -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.3, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_ClearAllRcvdMsgs();

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ClearReselectTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Clear reselect for the tree for all ports of the bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.4, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_ClearReselectTree(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FromSameRegion
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : Ref to the description in 13.26.5, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_FromSameRegion(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FromSameRegionAndUpdateReselect
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if rcvd_rstp is TRUE, and the received BPDU
 *            conveys an MST Configuration Identifier that matches that
 *            held for the Bridge. Return FALSE otherwise.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : TRUE/FALSE
 * RETURN   : None
 * NOTE     : 1. Ref to the description in 13.26.5, IEEE Std 802.1s(D14.1)-2002
 *            2. If rcvd_internal is changed,
 *               (1)Set "reselect" flag, and
 *               (2)Reset "selected" flag.
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_FromSameRegionAndUpdateReselect(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewTcWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Set the value of tc_while
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : tc_while  -- topology change timer
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.6, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewTcWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_NewEdgeDelayWhile
 * ------------------------------------------------------------------------
 * PURPOSE  : Return the edge delay upon a port's link type.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport
 * OUTPUT   : edge_delay_while  -- edge delay timer
 * RETURN   : None
 * NOTE     : Ref to the description in 17.20.4 IEEE Std 802.1D-2004
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_NewEdgeDelayWhile(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RcvInfoXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. rcvInfoCist()
 *            2. rcvInfoMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : rcv_info
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_SUPERIOR_DESIGNATED_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_REPEATED_DESIGNATED_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_ROOT_INFO
 *              -- XSTP_ENGINE_PORTVAR_RCVD_INFO_OTHER_INFO
 * NOTE     : Ref to the description in 13.26.7, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.8, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_RcvInfoXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordAgreementXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordAgreementCist()
 *            2. recordAgreementCist()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.9, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.10, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordAgreementXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordMasteredXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordMasteredCist()
 *            2. recordMasteredMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.11, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.12, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordMasteredXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RecordProposalXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. recordProposalCist
 *            2. recordProposalMsti
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.13, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.14, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_RecordProposalXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetRcvdMsgs
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.15, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void  XSTP_UTY_SetRcvdMsgs(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetReRootTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Set re_root TRUE for this tree for all ports of the bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.16, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetReRootTree(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSelectedTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets selected TRUE for this tree for all Ports of the Bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.17, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSelectedTree(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetSyncTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets sync TRUE for this tree for all Ports of the Bridge.
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.18, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetSyncTree(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcFlags
 * ------------------------------------------------------------------------
 * PURPOSE  :
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.19, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcFlags(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetTcPropTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Sets tc_prop TRUE for the given tree for all ports
 *            except the Port.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- lport (1..max)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.20, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_SetTcPropTree(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxConfig
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits a Configuration BPDU.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.21, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxConfig(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_TxMstp
 * ------------------------------------------------------------------------
 * PURPOSE  : Transmits an MST BPDU
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.22, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_TxMstp(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRcvdInfoWhileXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. updtRcvdInfoWhileCist()
 *            2. updtRcvdInfoWhileMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.23, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.24, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRcvdInfoWhileXst(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRolesXst
 * ------------------------------------------------------------------------
 * PURPOSE  : 1. updtRolesCist()
 *            2. updtRolesMsti()
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.25, IEEE Std 802.1s(D13)-2002
 *            Ref to the description in 13.26.26, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRolesXst(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_UpdtRoleDisabledTree
 * ------------------------------------------------------------------------
 * PURPOSE  : Set selected_role to DisabledPort for all ports of the bridge
 *            for a given tree
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 13.26.27, IEEE Std 802.1s(D13)-2002
 *-------------------------------------------------------------------------
 */
void    XSTP_UTY_UpdtRoleDisabledTree(XSTP_OM_InstanceData_T *om_ptr);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetHelloTime
 * ------------------------------------------------------------------------
 * PURPOSE  : Get hello_time value of CIST port_times.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : hello time.
 * NOTE     : According to the definition in 17.16.3 for RSTP, the HelloTime
 *            is the Bridge Hello Time component of Bridge Times.
 *            In the implementation we use the Hello Time component of
 *            CIST Port Times in order to have the consistent behavior with
 *            802.1d (STP).
 *            In 802.1d (STP), the Hello Time is dominated by the Bridge
 *            Hello Time of the Root Bridge via received BPDU from the root.
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_GetHelloTime(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetFwdDelay
 * ------------------------------------------------------------------------
 * PURPOSE  : Get forward_delay value of CIST port_times.
 * INPUT    : om_ptr    -- om pointer for this instance
 *            lport     -- specified lport
 * OUTPUT   : None
 * RETURN   : forward_delay.
 * NOTE     : According to the definition in 17.16.2 for RSTP, the FwdDelay
 *            is the Bridge Forward Delay component of Bridge Times.
 *            In the implementation we use the Forward Delay component of
 *            CIST Port Times in order to have the consistent behavior with
 *            802.1d (STP).
 *            In 802.1d (STP), the Forward Delay is dominated by the Bridge
 *            Forward Delay of the Root Bridge via received BPDU from the root.
 *-------------------------------------------------------------------------
 */
UI16_T  XSTP_UTY_GetFwdDelay(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer(UI16_T *timer);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DecreaseTimer32
 * ------------------------------------------------------------------------
 * PURPOSE  : Decrease the specified timer
 * INPUT    : timer     -- value of timer
 * OUTPUT   : None
 * RETURN   : TRUE if the timer expires, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_DecreaseTimer32(UI32_T *timer);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_ReselectForAnyPort
 * ------------------------------------------------------------------------
 * PURPOSE  : Check if the reselect variable of any port is set
 * INPUT    : om_ptr    -- om pointer for this instance
 * OUTPUT   : None
 * RETURN   : TRUE if the reselect variable of any port is TRUE, else FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_ReselectForAnyPort(XSTP_OM_InstanceData_T *om_ptr);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_MstiTcWhileIsZero
 * ------------------------------------------------------------------------
 * PURPOSE  : Return TRUE if the tc_while is equal to zero for any MSTI for
 *            the given Port. Return FALSE otherwise.
 * INPUT    : lport         -- specified lport
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_MstiTcWhileIsZero(UI32_T lport);

#endif /* XSTP_TYPE_PROTOCOL_MSTP */

/* ===================================================================== */
/* ===================================================================== */
/* Common utilities used by both RSTP and MSTP */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetMstEnableStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the XSTP_UTY_MstEnable flag
 * INPUT    : state         -- TRUE/FALSE
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the protocol version is less than MSTP's, XSTP_UTY_MstEnable
 *            is always set to FALSE.
 */
void    XSTP_UTY_SetMstEnableStatus(BOOL_T state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetPortState
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the spanning port state to SWCTRL
 * INPUT    : om_ptr        -- om pointer for this instance
 *            lport         -- lport
 *            state         -- stp port state
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : If the XSTP_UTY_MstEnable flag is set then the SWCTRL is invoked
 *            per vlan for the specified port.
 */
BOOL_T  XSTP_UTY_SetPortState(XSTP_OM_InstanceData_T *om_ptr, UI32_T lport, UI32_T state);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_FloodingBpdu
 *-------------------------------------------------------------------------
 * PURPOSE  : Flooding this packet to all the ports other than the receiver itself
 * INPUT    : bpdu_msg_ptr  -- BPDU message pointer
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
void    XSTP_UTY_FloodingBpdu(UI32_T lport, XSTP_TYPE_MSG_T *bpdu_msg_ptr, BOOL_T is_tagged, UI32_T global_status, UI32_T rcv_port_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_RootBridge
 * ------------------------------------------------------------------------
 * FUNCTION : Check whether the Bridge is the Root Bridge.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE if the Bridge is the Root Bridge, else FALSE.
 * NOTE     : If the Designated Root parameter held by the Bridge is the
 *            same as the Bridge ID of the Bridge, then conclude the
 *            Bridge to be the Root Bridge.
 * ------------------------------------------------------------------------
 */
BOOL_T  XSTP_UTY_RootBridge(XSTP_OM_InstanceData_T *om_ptr);

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_SetBpduGuardRecoverPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the port into a port list for BPDU guard auto recovery.
 * INPUT    :   lport -- lport number
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_SetBpduGuardRecoverPortList(UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_GetBpduGuardRecoverPortList
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the port list for BPDU guard auto recovery.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   XSTP_UTY_BpduGuardPortList_T*
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
XSTP_UTY_BpduGuardPortList_T *XSTP_UTY_GetBpduGuardRecoverPortList(void);
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */

#if (SYS_CPNT_DEBUG == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPringBPDU
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the BPDU content
 * INPUT    :   lport   -- the port send or receive the BPDU
 *              src_addr -- the source mac address
 *              is_tagged -- the BPDU is tagged BPDU or not
 *              pkt_length--the packet length
 *              pdu     -- the PDU content
 *              is_tx   -- identify print send BPDU or recevied BPDU
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPringBPDU(UI32_T lport, UI8_T *src_addr, BOOL_T is_tagged, UI8_T pkt_length, UI8_T *pdu, BOOL_T is_tx);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPrintRoot
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the root change or this now is root.
 * INPUT    :   *om_ptr     -- the port information pointer
 *              is_root_bridge-- identify this switch is root or not
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPrintRoot(XSTP_OM_InstanceData_T *om_ptr , BOOL_T is_root_bridge);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - XSTP_UTY_DebugPrintEvents
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function print the spanning tree event.
 * INPUT    :   lport   -- the port send or receive the BPDU
 * OUTPUT   :   None.
 * RETURN   :   None.
 * NOTES    :   now only print the TCN
 * ------------------------------------------------------------------------
 */
void XSTP_UTY_DebugPrintEvents(UI32_T lport);
#endif /* #if (SYS_CPNT_DEBUG == TRUE) */

#endif /* _XSTP_UTY_H */
