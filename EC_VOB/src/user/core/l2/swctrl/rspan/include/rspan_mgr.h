/*-------------------------------------------------------------------------
 * Module Name: rspan_mgr.h
 *-------------------------------------------------------------------------
 * PURPOSE: Definitions for the RSPAN
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    07/23/2007 - Tien Kuo, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-------------------------------------------------------------------------
 */

#ifndef _RSPAN_MGR_H
#define _RSPAN_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "rspan_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define RSPAN_MGR_MAX_SESSION_NUM                 SYS_ADPT_RSPAN_MAX_NBR_OF_SESSION
#define RSPAN_MGR_MAX_SRC_UPLINK_NUM              1

#define RSPAN_MGR_MSGBUF_TYPE_SIZE    sizeof(((RSPAN_MGR_IPCMsg_T *)0)->type)

/* DATA TYPE DECLARATIONS
 */

/*-------------------------------------------------------------------------
 * All structures provide CLI, Web, and SNMP to get specified information.
 *-------------------------------------------------------------------------
 */

/* For SNMP*/
typedef struct
{
    UI32_T rspanVlanId;
    UI32_T rspanVlanStatus;
}RSPAN_MGR_RspanVlan_T;

/*-------------------------------------------------------------------------
 * The following structures apply to RSPAN
 *-------------------------------------------------------------------------
 */

typedef struct
{
    union
    {
        UI32_T cmd;         /*cmd fnction id*/
        BOOL_T ret_bool;    /*respond bool return*/
        UI32_T ret_ui32;    /*respond ui32 return*/
        UI32_T ret_i32;     /*respond i32 return*/
    }type;

    union
    {
        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;

        struct
        {
                UI8_T session_id;
                RSPAN_OM_SessionEntry_T  rspan_entry;
        }rspan_sessionentry;

        struct
        {
            RSPAN_OM_SessionEntry_T entry;
            UI8_T field_id;
        }rspan_sessionentryfield;

        struct
        {
            RSPAN_MGR_RspanVlan_T entry;
            BOOL_T get_next;
        }rspan_vlanentry;

        struct
        {
                UI32_T u32_a1;
                UI32_T u32_a2;
        }u32a1_u32a2;

        struct
        {
                UI8_T u8_a1;
                UI8_T u8_a2;
                UI8_T u8_a3;
        }u8a1_u8a2_u8a3;

        struct
        {
                UI8_T u8_a1;
                UI8_T u8_a2;
        }u8a1_u8a2;

        struct
        {
                UI8_T u8_a1;
                UI8_T u8_a2;
                UI32_T u32_a3;
                UI8_T u8_a4;
        }u8a1_u8a2_u32a3_u8a4;

    }data;
   }RSPAN_MGR_IPCMsg_T;

enum
{
    RSPAN_MGR_IPC_ISRSPANUPLINKPORT,
    RSPAN_MGR_IPC_ISRSPANVLAN,
    RSPAN_MGR_IPC_ISRSPANMIRRORTOPORT,
    RSPAN_MGR_IPC_GETRSPANSESSIONENTRY,
    RSPAN_MGR_IPC_GETNEXTRSPANSESSIONENTRY,
    RSPAN_MGR_IPC_ISSESSIONENTRYCOMPLETED,
    RSPAN_MGR_IPC_SETSESSIONSOURCEINTERFACE,
    RSPAN_MGR_IPC_DELETESESSIONSOURCEINTERFACE,
    RSPAN_MGR_IPC_SETSESSIONDESTINATIONINTERFACE,
    RSPAN_MGR_IPC_DELETESESSIONDESTINATIONINTERFACE,
    RSPAN_MGR_IPC_SETSESSIONREMOTEVLAN,
    RSPAN_MGR_IPC_DELETESESSIONREMOTEVLAN,
    RSPAN_MGR_IPC_SETSESSIONREMOTEVLANDST,
    RSPAN_MGR_IPC_REMOVECLIWEBSESSIONID,
    RSPAN_MGR_IPC_CREATERSPANVLAN,
    RSPAN_MGR_IPC_DELETERSPANVLAN,
    RSPAN_MGR_IPC_SETRSPANVLANSTATUS,
    RSPAN_MGR_IPC_GETRSPANVLANENTRY,
    RSPAN_MGR_IPC_SETRSPANENTRYSTATUS,
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_Init
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the resource for RSPAN objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   This function is invoked in RSPAN_TASK_Init.
 *-------------------------------------------------------------------------
 */
void RSPAN_MGR_Init(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * FUNCTION : Enable RSPAN operation while in master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RSPAN_MGR_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the RSPAN operation while in slave mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RSPAN_MGR_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set operation mode into transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RSPAN_MGR_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the RSPAN operation while in transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void RSPAN_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the RSPAN operation mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : SYS_TYPE_STACKING_Master_MODE
 *            SYS_TYPE_STACKING_SLAVE_MODE
 *            SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T RSPAN_MGR_GetOperationMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_CreateRspanVlan
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
BOOL_T RSPAN_MGR_CreateRspanVlan( UI32_T vid );

/*------------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteRspanVlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This function returns true if the specific vlan that is RSPAN vlan
 *            is successfully deleted from the database.
 *            Otherwise, false is returned.
 * INPUT    : vid   -- the existed RSPAN vlan id
 * OUTPUT   : None
 * RETURN   : TRUE \ FALSE
 * NOTES    : If the specific vlan is not RSPAN vlan, return FALSE.
 *------------------------------------------------------------------------------*/
BOOL_T RSPAN_MGR_DeleteRspanVlan( UI32_T vid ) ;


/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanVlan
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
BOOL_T RSPAN_MGR_IsRspanVlan(UI32_T vid);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetSessionRemoteVlan
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
BOOL_T RSPAN_MGR_SetSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteSessionRemoteVlan
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
BOOL_T RSPAN_MGR_DeleteSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetSessionRemoteVlanDst
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
BOOL_T RSPAN_MGR_SetSessionRemoteVlanDst(UI8_T session_id, UI8_T dst);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetSessionSourceInterface
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
BOOL_T RSPAN_MGR_SetSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode ) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteSessionSourceInterface
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
BOOL_T RSPAN_MGR_DeleteSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetSessionDestinationInterface
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
BOOL_T RSPAN_MGR_SetSessionDestinationInterface(UI8_T session_id, UI8_T destination_port, UI8_T is_tagged);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteSessionDestinationInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN destination port (monitoring port).
 * INPUT    : session_id       -- RSPAN session ID.
 *            destination_port -- RSPAN destination function.
 *                                It's a source port, not a port list.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_DeleteSessionDestinationInterface(UI8_T session_id, UI8_T destination_port);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetRspanSessionEntry
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
BOOL_T RSPAN_MGR_GetRspanSessionEntry(UI8_T session_id, RSPAN_OM_SessionEntry_T *rspan_entry);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetNextRspanSessionEntry
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
BOOL_T RSPAN_MGR_GetNextRspanSessionEntry(UI8_T *session_id, RSPAN_OM_SessionEntry_T *rspan_entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to store the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : This API is for XOR and swctrl.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_SetRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SettingValidation
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are valid to set the database.
 * INPUT    : The pointer of the session structure.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is valid for setting the database.
 *            FALSE       -- The entry is not valid.
 * NOTES    : This API is for XOR and swctrl functions calling.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_SettingValidation ( RSPAN_OM_SessionEntry_T *is_rspan_entry_valid, UI8_T target_port ) ;

/*-------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetSessionEntryCounter
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the number of RSPAN entries from RSPAN database.
 * INPUT    : *session_cnt -- The total session numbers.
 * OUTPUT   : *session_cnt -- The total session numbers.
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_GetSessionEntryCounter(UI8_T *session_cnt) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteRspanSessionEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete RSPAN session entry in RSPAN_OM.
 * INPUT    : The whole data structure to delete the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : This API will be called by SWCTRL.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_DeleteRspanSessionEntry( RSPAN_OM_SessionEntry_T *rspan_entry ) ;

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
BOOL_T RSPAN_MGR_IsSessionEntryCompleted( UI8_T session_id );

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_RemoveCliWebSessionId
 *--------------------------------------------------------------------------
 * PURPOSE  : Remove a session id , all relative vlan membership and chip setting.
 * INPUT    : session_id : The specific session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The session id and relative setting are removed.
 *            FALSE       -- The session id and relative setting are not removed.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_RemoveCliWebSessionId( UI8_T session_id );

/* 802.1X and port security will need this. 2007/09/20 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_IsRspanUplinkPort(UI32_T ifindex) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanUplinkPortUsed
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port and ready to perform
 *                  RSPAN.
 *           FALSE: The ifindex is not a RSPAN mirrored port or not ready to
 *                  perform RSPAN.
 * NOTE    : This API is used for AMTR to check if it needs to enable/disable
 *           port learning of RSPAN uplink ports.
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_IsRspanUplinkPortUsed ( UI32_T ifindex ) ;

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanMirrorToPort
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
BOOL_T RSPAN_MGR_IsRspanMirrorToPort(UI32_T ifindex) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanMirroredPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN mirrored port
 *           FALSE: The ifindex is not a RSPAN mirrored port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_IsRspanMirroredPort(UI32_T ifindex) ;

#endif /*#if (SYS_CPNT_SYSCTRL_XOR == TRUE)*/

/* For SNMP - Src\SysInclude\MibConstants\leaf_es3626a.h */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetRspanVlanStatus
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
BOOL_T RSPAN_MGR_SetRspanVlanStatus(UI32_T vlan_id, UI32_T rspan_status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the status of the specific VLAN.
 * INPUT    : entry->rspanVlanId   -- The given VLAN id.
 * OUTPUT   : Information of the entry.
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : Need to fill the VLAN field when passing pointer.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_GetRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME -  RSPAN_MGR_GetNextRspanVlanEntry
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
BOOL_T RSPAN_MGR_GetNextRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry );

/*--------------------------------------------------------------------------
 * ROUTINE NAME -  RSPAN_MGR_SetRspanEntryStatus
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
BOOL_T  RSPAN_MGR_SetRspanEntryStatus( RSPAN_OM_SessionEntry_T *entry , UI8_T field_id ) ;

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_VlanActiveToSuspendCallback
 *--------------------------------------------------------------------------
 * PURPOSE  : Process callback when VLAN state is changed from active to
 *            suspended.
 * INPUT    : vid_ifindex - vlan ifindex whose state is changed
 *            vlan_status - vlan status which changes the state of the VLAN
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_MGR_VlanActiveToSuspendCallback(UI32_T vid_ifindex, UI32_T vlan_status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_VlanSuspendToActiveCallback
 *--------------------------------------------------------------------------
 * PURPOSE  : Process callback when VLAN state is changed from suspended to
 *            active.
 * INPUT    : vid_ifindex - vlan ifindex whose state is changed
 *            vlan_status - vlan status which changes the state of the VLAN
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void RSPAN_MGR_VlanSuspendToActiveCallback(UI32_T vid_ifindex, UI32_T vlan_status);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - RSPAN_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RSPAN MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif  /* _RSPAN_MGR_H */
