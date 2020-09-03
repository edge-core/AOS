/*-----------------------------------------------------------------------------
 * FILE NAME: RSPAN_PMGR.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file declares the APIs for RSPAN MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2009/07/29     --- Joeanne, Create
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */

#ifndef RSPAN_PMGR_H
#define RSPAN_PMGR_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "sys_cpnt.h"
#include "rspan_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : RSPAN_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for RSPAN_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_InitiateProcessResource(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN uplink port
 *           FALSE: The ifindex is not a RSPAN uplink port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_IsRspanUplinkPort(UI32_T ifindex);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_IsRspanVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will find the specific RSPAN VLAN entry from RSPAN database.
 *            This function is exported for other CSCs.
 * INPUT    : vid         -- The specific vlan id
 * OUTPUT   : None
 * RETURN   : TRUE        -- RSPAN VLAN is found
 *            FALSE       -- RSPAN VLAN isn't found
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_IsRspanVlan(UI32_T vid);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_IsRspanMirrorToPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN
 *           destination port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN destination port
 *           FALSE: The ifindex is not a RSPAN destination port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_IsRspanMirrorToPort(UI32_T ifindex);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_GetRspanSessionEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN entry of the specific session from RSPAN database.
 * INPUT    : *session_id  -- The specific session id.
 *            *rspan_entry -- The RSPAN entry pointer.
 * OUTPUT   : *rspan_entry -- The whole data structure with the specific entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_GetRspanSessionEntry(UI8_T session_id, RSPAN_OM_SessionEntry_T *rspan_entry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_GetNextRspanSessionEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the next available RSPAN entry of the specific session from
 *            RSPAN database.
 * INPUT    : *session_id  -- The session id to get next RSPAN entry.
 *            *rspan_entry -- The RSPAN entry pointer.
 * OUTPUT   : *session_id  -- The next session id.
 *            *rspan_entry -- The whole data structure with the current entry.
 * RETURN   : TRUE         -- The configuration is set successfully.
 *            FALSE        -- The configuration isn't set successfully.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_GetNextRspanSessionEntry(UI8_T *session_id, RSPAN_OM_SessionEntry_T *rspan_entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsSessionEntryCompleted
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are ready to set the chip.
 * INPUT    : session_id  -- The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is ready for setting the chip.
 *            FALSE       -- The entry is not completed.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_IsSessionEntryCompleted( UI8_T session_id );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_SetSessionSourceInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Configure RSPAN source port (monitored port) and the direction
 *            of traffic to monitor per session.
 * INPUT    : session_id  -- RSPAN session ID.
 *            source_port -- RSPAN source port to monitor. It's a source port,
 *                           not a port list.
 *            mode        -- VAL_mirrorType_rx   1L
 *                           VAL_mirrorType_tx   2L
 *                           VAL_mirrorType_both 3L
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_SetSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_DeleteSessionSourceInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN source port (monitored port) and the direction
 *            of traffic to monitor per session.
 * INPUT    : session_id  -- RSPAN session ID.
 *            source_port -- RSPAN source port to monitor. It's a source port,
 *                           not a port list.
 *            mode        -- VAL_mirrorType_rx   1L
 *                           VAL_mirrorType_tx   2L
 *                           VAL_mirrorType_both 3L
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_DeleteSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_SetSessionDestinationInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Configure RSPAN destination port (monitoring port).
 * INPUT    : session_id       -- RSPAN session ID.
 *            destination_port -- RSPAN destination function.
 *                                It's a source port, not a port list.
 *            is_tagged        -- VAL_rspanDstPortTag_untagged 1L
 *                                VAL_rspanDstPortTag_tagged   2L
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_SetSessionDestinationInterface(UI8_T session_id, UI8_T destination_port, UI8_T is_tagged);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_DeleteSessionDestinationInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN destination port (monitoring port).
 * INPUT    : session_id       -- RSPAN session ID.
 *            destination_port -- RSPAN destination function.
 *                                It's a source port, not a port list.
 *            is_tagged        -- VAL_rspanDstPortTag_untagged 1L
 *                                VAL_rspanDstPortTag_tagged   2L
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_DeleteSessionDestinationInterface( UI8_T session_id, UI8_T destination_port );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_SetSessionRemoteVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Configure RSPAN remote VLAN and the uplink port(s), which will
 *            flood RSPAN VLAN traffic.
 * INPUT    : session_id  -- RSPAN session ID.
 *            switch_role -- VAL_rspanSwitchRole_source       1L
 *                           VAL_rspanSwitchRole_intermediate 2L
 *                           VAL_rspanSwitchRole_destination  3L
 *            remote_vid  -- RSPAN VLAN to carry monitored traffic to the
 *                           destination port.
 *            uplink      -- The port will flood RSPAN VLAN traffic.
 *                           It's a source port, not a port list.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : Remote VLAN ID must be created in VLAN database before setting
 *            this configuration.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_SetSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_DeleteSessionRemoteVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Configure RSPAN remote VLAN and the uplink port(s), which will
 *            flood RSPAN VLAN traffic.
 * INPUT    : session_id  -- RSPAN session ID.
 *            switch_role -- VAL_rspanSwitchRole_source       1L
 *                           VAL_rspanSwitchRole_intermediate 2L
 *                           VAL_rspanSwitchRole_destination  3L
 *            remote_vid  -- RSPAN VLAN to carry monitored traffic to the
 *                           destination port.
 *            uplink      -- The port will flood RSPAN VLAN traffic.
 *                           It's a source port, not a port list.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_DeleteSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_SetSessionRemoteVlanDst
 *--------------------------------------------------------------------------
 * PURPOSE  : Configure RSPAN remote VLAN dst port.
 * INPUT    : session_id  -- RSPAN session ID.
 *            dst         -- dst port, must be one of uplink port
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_SetSessionRemoteVlanDst(UI8_T session_id, UI8_T dst);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_RemoveCliWebSessionId
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id , all relative vlan membership and chip setting.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id and relative setting are removed.
 *            FALSE       -- The session id and relative setting are not removed.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_RemoveCliWebSessionId( UI8_T session_id );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_CreateRspanVlan
 *--------------------------------------------------------------------------
 * PURPOSE  : Create a RSPAN VLAN.
 * INPUT    : vid         -- The new created VLAN id
 * OUTPUT   : None
 * RETURN   : TRUE        -- RSPAN VLAN is created successfully
 *            FALSE       -- RSPAN VLAN isn't created successfully
 * NOTES    : If the new vlan is not normal vlan which is created already,
 *            return FALSE.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_CreateRspanVlan( UI32_T vid );

/*------------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_DeleteRspanVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is RSPAN vlan
 *            is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed RSPAN vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : If the specific vlan is not RSPAN vlan, return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T RSPAN_PMGR_DeleteRspanVlan( UI32_T vid ) ;

/* For SNMP - Src\SysInclude\MibConstants\leaf_es3626a.h */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_SetRspanVlanStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Create/Delete RSPAN VLAN through SNMP.
 * INPUT    : vlan_id       -- The VLAN id for creating or deleting.
 *            rspan_status  -- VAL_vlanStaticExtRspanStatus_destroy   1L
 *                             VAL_vlanStaticExtRspanStatus_vlan      2L
 *                             VAL_vlanStaticExtRspanStatus_rspanVlan 3L
 * OUTPUT   : None
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : VAL_vlanStaticExtRspanStatus_vlan can't be used here.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_SetRspanVlanStatus(UI32_T vlan_id, UI32_T rspan_status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_PMGR_GetRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the status of the specific VLAN.
 * INPUT    : entry->rspanVlanId   -- The given VLAN id.
 * OUTPUT   : Information of the entry.
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : Need to fill the VLAN field when passing pointer.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_GetRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME -  RSPAN_PMGR_GetNextRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next available RSPAN VLAN entry and its status.
 * INPUT    : entry->rspanVlanId  -- The given VLAN id to get next entry in
 *                                   the Current VLAN table.
 * OUTPUT   : Information of the entry.
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : Need to fill the VLAN field when passing pointer.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_PMGR_GetNextRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry );

/*--------------------------------------------------------------------------
 * ROUTINE NAME -  RSPAN_PMGR_SetRspanEntryStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Create/Delete RSPAN Entry through SNMP.
 * INPUT    : *entry -- The structure of RSPAN data.
 *            field_id -- Which field is going to modify/delete.
 * OUTPUT   : None
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : Types of field_id
 *            RSPANSRCTXPORTS     2
 *            RSPANSRCRXPORTS     3
 *            RSPANDSTPORTINDEX   4
 *            RSPANDSTPORTTAG     5
 *            RSPANSWITCHROLE     6
 *            RSPANREMOTEPORTS    7
 *            RSPANREMOTEVLANID   8
 *            RSPANSTATUS         10
 *--------------------------------------------------------------------------
 */
BOOL_T  RSPAN_PMGR_SetRspanEntryStatus( RSPAN_OM_SessionEntry_T *entry , UI8_T field_id ) ;

#endif
