/*-------------------------------------------------------------------------
 * Module Name: rspan_mgr.c
 *-------------------------------------------------------------------------
 * PURPOSE: Definitions for the RSPAN
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    07/27/2007 - Tien Kuo, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rspan_mgr.h"
#include "amtr_mgr.h"
#include "1x_om.h"
#include "sysfun.h"
#include "stktplg_mgr.h"
#include "leaf_2674q.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "swctrl.h"
#include "sys_dflt.h"
#include "sys_type.h"
#include "backdoor_mgr.h"
#include "swdrv.h"
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
#include "sysctrl_xor_mgr.h"
#endif
#include "stktplg_pom.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define RSPAN_IFINDEX_TO_UNIT(ifindex)        (((UI32_T)((ifindex - 1) / SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)) + 1)
#define RSPAN_IFINDEX_TO_PORT(ifindex)        (ifindex - (RSPAN_IFINDEX_TO_UNIT(ifindex) - 1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void RSPAN_MGR_BackdoorMain(void) ;
static BOOL_T RSPAN_MGR_ReadyToSetMirrorChip(RSPAN_OM_SessionEntry_T is_rspan_entry_valid);
static BOOL_T RSPAN_MGR_ReadyToDelMirrorChip(RSPAN_OM_SessionEntry_T is_rspan_entry_valid, RSPAN_OM_SessionEntry_T compared_entry);
static BOOL_T RSPAN_MGR_DisableMacLearning(UI8_T uplink);
static BOOL_T RSPAN_MGR_EnableMacLearning(UI8_T uplink);
static BOOL_T RSPAN_MGR_SetSessionVlanMembership(RSPAN_OM_SessionEntry_T is_rspan_entry_valid);
static BOOL_T RSPAN_MGR_DeleteSessionVlanMembership(RSPAN_OM_SessionEntry_T is_rspan_entry_valid);
static BOOL_T RSPAN_MGR_RemoveSnmpSessionProcess(RSPAN_OM_SessionEntry_T compared_rspan_entry);
static BOOL_T RSPAN_MGR_ResaveSnmpSessionProcess(RSPAN_OM_SessionEntry_T compared_rspan_entry);
static BOOL_T RSPAN_MGR_ValidateSnmpSessionProcess(UI8_T session_id, UI8_T *port_bit, UI8_T switch_role, UI8_T port_list_type);
static BOOL_T RSPAN_MGR_ValidateSnmpSwitchRole(RSPAN_OM_SessionEntry_T entry, UI8_T field_id);
static BOOL_T RSPAN_MGR_IsSrcUplinkCountValid(UI8_T *port_bit);
static BOOL_T RSPAN_MGR_ValidateSnmpSessionCommonProcess(UI8_T session_id, I16_T targetPort, UI8_T switch_role, UI8_T port_list_type);

/* EXPORTED SUBPROGRAM BODIES
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
void RSPAN_MGR_Init(void)
{
#if (RSPAN_BACK_DOOR == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("rspan",
        SYS_BLD_SWCTRL_GROUP_IPCMSGQ_KEY, RSPAN_MGR_BackdoorMain);
#endif
}

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
void RSPAN_MGR_EnterMasterMode(void)
{
    RSPAN_OM_EnterMasterMode();
    return;
}

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
void RSPAN_MGR_EnterSlaveMode(void)
{
    RSPAN_OM_EnterSlaveMode();
    return;
}

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
void RSPAN_MGR_SetTransitionMode(void)
{
    RSPAN_OM_SetTransitionMode();
    return;
}

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
void RSPAN_MGR_EnterTransitionMode(void)
{
    RSPAN_OM_EnterTransitionMode();
    RSPAN_OM_InitDataBase();
    return;
}

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
SYS_TYPE_Stacking_Mode_T RSPAN_MGR_GetOperationMode(void)
{
    return RSPAN_OM_GetOperatingMode();
}

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
BOOL_T RSPAN_MGR_CreateRspanVlan(UI32_T vid)
{
    if (RSPAN_OM_GetOperatingMode()!= SYS_TYPE_STACKING_MASTER_MODE)
    {
    	return FALSE;
    }

    if (VLAN_MGR_CreateRspanVlan(vid, VAL_dot1qVlanStatus_permanent) != TRUE)
    {
        return FALSE;
    }

    RSPAN_OM_SetRspanVlanEntry(vid) ;
    return TRUE;
}

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
BOOL_T RSPAN_MGR_DeleteRspanVlan(UI32_T vid)
{
    RSPAN_OM_SessionEntry_T is_src_role;
    UI8_T cnt = 0;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* If the rspan session role is source, meaning this remote vid is still used; then return False first. */
    for (cnt = 1; cnt <= RSPAN_MGR_MAX_SESSION_NUM; cnt ++)
    {
        memset(&is_src_role, 0, sizeof (RSPAN_OM_SessionEntry_T));

        if (RSPAN_OM_GetRspanSessionEntry(cnt, &is_src_role))
        {
            /* As long as this rspan vlan is configured in session, it can not be removed */
            if (is_src_role.remote_vid == vid)
            {
                return FALSE;
            }
        }
    }

    if (VLAN_MGR_DeleteRspanVlan(vid, VAL_dot1qVlanStatus_permanent) != TRUE)
    {
        return FALSE;
    }

    RSPAN_OM_DeleteRspanVlanEntry(vid);
    return TRUE;
}

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
BOOL_T RSPAN_MGR_SetSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid ;
    RSPAN_OM_SessionEntry_T rspan_entry_infor ;
    UI8_T compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,};
    UI8_T ret = -1 ; /* For debugging. */

#if (SYS_CPNT_SYSCTRL_XOR == FALSE)
#if (SYS_CPNT_PORT_SECURITY == TRUE) || (SYS_CPNT_DOT1X == TRUE)
    UI32_T port_security_enabled_by_who = 0;
#endif
#endif

    VLAN_OM_VlanPortEntry_T  vlan_port_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    	return FALSE;
    }

    if (session_id > RSPAN_MGR_MAX_SESSION_NUM || session_id == 0)
    {
        return FALSE;
    }

    if (switch_role < VAL_rspanSwitchRole_source || 
        switch_role > VAL_rspanSwitchRole_destination)
    {
        return FALSE;
    }

    if (remote_vid == 0 || remote_vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return FALSE;
    }

    if (uplink == 0)
    {
        return FALSE;
    }

    if (VLAN_OM_GetVlanPortEntryByIfindex(uplink, &vlan_port_entry) == FALSE)
    {
        return FALSE;
    }

    if (vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
    {
        return FALSE;
    }

    if (RSPAN_OM_IsRspanVlan(remote_vid) != TRUE) /* Remote VID isn't valid */
    {
        return FALSE;
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_GetSemaphore();

    if (SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry(uplink, LEAF_rspanRemotePorts) == FALSE)
    {
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return FALSE;
    }

#else
    /* Validate the setting port is a trunk(member) or user port. */
    if (SWCTRL_RspanSettingValidation(uplink) == FALSE)
    {
        return FALSE;
    }

#if (SYS_CPNT_PORT_SECURITY == TRUE || SYS_CPNT_DOT1X == TRUE)
    if (TRUE == SWCTRL_IsSecurityPort((UI32_T)uplink, &port_security_enabled_by_who))
    {
        if (port_security_enabled_by_who == SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X || 
            port_security_enabled_by_who == SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC)
        {
            printf("Set RSPAN uplink port error: [%s] is enabled.\n",
                (port_security_enabled_by_who == SWCTRL_PORT_SECURITY_ENABLED_BY_DOT1X)?("802.1X"):("Port security"));
            return FALSE;
        }
    }
#endif /* #if ((SYS_CPNT_PORT_SECURITY == TRUE) || (SYS_CPNT_DOT1X == TRUE)) */
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));
    is_rspan_entry_valid.session_id = session_id;
    is_rspan_entry_valid.switch_role = switch_role;
    is_rspan_entry_valid.remote_vid = remote_vid;
    is_rspan_entry_valid.uplink[0] = uplink;/* To indicate this is a uplink port */

    if (RSPAN_OM_SettingValidation(&is_rspan_entry_valid, uplink) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    /* Get previous record in is_rspan_entry_valid structure data. */
    if (RSPAN_OM_GetRspanSessionEntry(session_id, &rspan_entry_infor))
    {
        if (rspan_entry_infor.switch_role == 0) /* There is no switch role configuration in this session */
        {
            if (switch_role != VAL_rspanSwitchRole_destination && rspan_entry_infor.dst)
            {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                return FALSE;
            }

            if (switch_role != VAL_rspanSwitchRole_source)
            {
                if (!(! memcmp(compared, rspan_entry_infor.src_tx, sizeof(rspan_entry_infor.src_tx))
                    || ! memcmp(compared, rspan_entry_infor.src_rx, sizeof(rspan_entry_infor.src_rx))))
                {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                    SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                    return FALSE;
                }
            }
        }
        else /* There is a switch role configuration in this session */
        {
            /* Users need to delete the previous setting with "no" form first. */
            if (switch_role != rspan_entry_infor.switch_role)
            {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                return FALSE;
            }
        }

        is_rspan_entry_valid.dst = rspan_entry_infor.dst ;
        is_rspan_entry_valid.is_tagged = rspan_entry_infor.is_tagged ;
        memcpy(is_rspan_entry_valid.src_tx, rspan_entry_infor.src_tx, sizeof(rspan_entry_infor.src_tx));
        memcpy(is_rspan_entry_valid.src_rx, rspan_entry_infor.src_rx, sizeof(rspan_entry_infor.src_rx));
        is_rspan_entry_valid.uplink[0] = 0 ;
        memcpy(is_rspan_entry_valid.uplink, rspan_entry_infor.uplink, sizeof(rspan_entry_infor.uplink));

        /* Put the newest data in the structure. */
        RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.uplink, uplink);
    }
    else
    {
        is_rspan_entry_valid.uplink[0] = 0 ;
        /* Put the newest data in the structure. */
        RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.uplink, uplink);
    }

    /* Save entry into RSPAN OM */
    if (switch_role == VAL_rspanSwitchRole_source)
    {
        if (RSPAN_OM_IncreaseSrcUplinkCounter(session_id) == FALSE)
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }
    }

    if (RSPAN_OM_SetRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    /* Check if config. needs to set chip. */
    if (RSPAN_OM_IsSessionEntryCompleted(&is_rspan_entry_valid))
    {
        if (RSPAN_MGR_SetSessionVlanMembership(is_rspan_entry_valid) == FALSE)
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_source)
        {
            if (RSPAN_MGR_ReadyToSetMirrorChip(is_rspan_entry_valid) == FALSE)
            {
                return FALSE;
            }
        }

        /* Every RSPAN role needs to disable mac-learning at uplink ports */
        ret = RSPAN_MGR_DisableMacLearning(uplink);

        /* update the usage of uplinnk ports for AMTR */
        if (ret == TRUE)
            RSPAN_OM_SetRspanUplinkPortUsage(session_id, TRUE);
    }

    /* Every uplink port needs to increase maximum frame size */
    SWCTRL_ModifyMaxFrameSizeForRspan(uplink, TRUE);

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

	return TRUE;
}

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
BOOL_T RSPAN_MGR_DeleteSessionRemoteVlan(UI8_T session_id, UI8_T switch_role, UI32_T remote_vid, UI8_T uplink)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid ;
    RSPAN_OM_SessionEntry_T compared_entry ;
    UI8_T   ret = -1 ;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (switch_role < VAL_rspanSwitchRole_source || 
        switch_role > VAL_rspanSwitchRole_destination)
    {
        return FALSE;
    }

    if (remote_vid == 0 || remote_vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return FALSE;
    }

    if (uplink == 0)
    {
        return FALSE;
    }

    if (RSPAN_OM_IsRspanVlan(remote_vid) == FALSE) /* Remote VID is invalid */
    {
        return FALSE;
    }

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));

    is_rspan_entry_valid.session_id = session_id;
    is_rspan_entry_valid.switch_role = switch_role;
    is_rspan_entry_valid.remote_vid = remote_vid;
    /* For deleting usage. */
    is_rspan_entry_valid.uplink[0] = uplink;

    /* Check if config. needs to set chip. */
    memset(&compared_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

    if (RSPAN_OM_GetRspanSessionEntry(session_id, &compared_entry))
    {
        if (RSPAN_OM_IS_MEMBER(compared_entry.uplink, is_rspan_entry_valid.uplink[0]))
        {
            /* Check if there is no src uplink existing and update the src uplink counter for adding a
             * new src uplink port next time.
             */
            if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_source)
            {
                if (RSPAN_OM_DecreaseSrcUplinkCounter(session_id) == FALSE)
                {
                    return FALSE;
                }
            }

            if (RSPAN_OM_IsSessionEntryCompleted(&is_rspan_entry_valid) == TRUE)
            {
                RSPAN_OM_SessionEntry_T deal_vlan_entry;

                memset(&deal_vlan_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));
                RSPAN_OM_ADD_MEMBER(deal_vlan_entry.uplink, uplink);
                deal_vlan_entry.remote_vid = is_rspan_entry_valid.remote_vid;

                if (RSPAN_MGR_DeleteSessionVlanMembership(deal_vlan_entry) == FALSE)
                {
                    return FALSE;
                }

                if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_source)
                    RSPAN_MGR_ReadyToDelMirrorChip(is_rspan_entry_valid, compared_entry);
            }
        }
        else
        {
            return FALSE;
        }
    }

    if (RSPAN_OM_DeleteRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
        return FALSE;
    }

    /* Check if needing to remove all vlan membership */
    memset(&compared_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

    if (RSPAN_OM_GetRspanSessionEntry(session_id, &compared_entry))
    {
        if (RSPAN_OM_IsSessionEntryCompleted(&compared_entry) == FALSE)
        {
            compared_entry.remote_vid = remote_vid;
            compared_entry.switch_role = switch_role;

            if (RSPAN_MGR_DeleteSessionVlanMembership(compared_entry) == FALSE)
            {
                /*SYSFUN_Debug_Printf("\r\nRSPAN_OM_GetRspanSessionEntry is failed.");*/
            }
        }
    }

    if (RSPAN_OM_RemoveSessionId(session_id) == FALSE)
    {
        /*SYSFUN_Debug_Printf("\r\nRSPAN_OM_RemoveSessionId is failed.");*/
    }

    /* Re-enable Mac Learning on this port again because there is no uplink relation existing. */
    ret = RSPAN_MGR_EnableMacLearning(uplink);

    /* When delete a uplink port, needs to reduce maximum frame size */
    SWCTRL_ModifyMaxFrameSizeForRspan(uplink, FALSE);

    /* update the usage of uplinnk ports for AMTR */
    if (ret == TRUE)
        RSPAN_OM_SetRspanUplinkPortUsage(session_id, FALSE);

	return TRUE;
}

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
BOOL_T RSPAN_MGR_SetSessionRemoteVlanDst(UI8_T session_id, UI8_T dst)
{
    RSPAN_OM_SessionEntry_T rspan_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (dst == 0)
    {
        return FALSE;
    }

    memset(&rspan_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

    if (RSPAN_OM_GetRspanSessionEntry(session_id, &rspan_entry) == FALSE)
    {
        return FALSE;
    }

    /*
    if (RSPAN_OM_IS_MEMBER(rspan_entry.uplink, dst))
    {printf("%s, %d. dst:%d.\n", __FUNCTION__, __LINE__, dst);
        return FALSE;
    }
    */

    rspan_entry.dst_in = dst;

    if (RSPAN_OM_SetRspanSessionEntry(&rspan_entry) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T RSPAN_MGR_SetSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid;
    RSPAN_OM_SessionEntry_T rspan_entry_infor;
    I8_T    ret = -1;
    I16_T    i, j, uplink;
    VLAN_OM_VlanPortEntry_T  vlan_port_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (mode < VAL_mirrorType_rx || mode > VAL_mirrorType_both)
    {
        return FALSE;
    }

    if (source_port == 0)
    {
    	return FALSE;
    }

    if (VLAN_OM_GetVlanPortEntryByIfindex(source_port, &vlan_port_entry) == FALSE)
    {
        return FALSE;
    }

    if (vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
    {
        return FALSE;
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_GetSemaphore();
    if (SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry(source_port, LEAF_rspanSrcTxPorts) == FALSE)
    {
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return FALSE;
    }
#else
    /* Validate the setting port is a trunk(member) or user port */
    if (SWCTRL_RspanSettingValidation(source_port) == FALSE )
    {
        return FALSE;
    }
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));
    is_rspan_entry_valid.session_id = session_id;

    /* to indicate this is src port. */
    if (mode == VAL_mirrorType_tx || mode == VAL_mirrorType_both)
        is_rspan_entry_valid.src_tx[0] = source_port;

    if (mode == VAL_mirrorType_rx || mode == VAL_mirrorType_both)
        is_rspan_entry_valid.src_rx[0] = source_port;

    if (RSPAN_OM_SettingValidation(&is_rspan_entry_valid, source_port) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    /* Get previous record in is_rspan_entry_valid structure data */
    if (RSPAN_OM_GetRspanSessionEntry(session_id, &rspan_entry_infor))
    {
        if (rspan_entry_infor.dst || rspan_entry_infor.is_tagged)
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        if (rspan_entry_infor.switch_role && 
            (rspan_entry_infor.switch_role == VAL_rspanSwitchRole_intermediate ||
             rspan_entry_infor.switch_role == VAL_rspanSwitchRole_destination))
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        is_rspan_entry_valid.switch_role = rspan_entry_infor.switch_role;
        is_rspan_entry_valid.dst = rspan_entry_infor.dst;
        is_rspan_entry_valid.is_tagged = rspan_entry_infor.is_tagged;
        is_rspan_entry_valid.remote_vid = rspan_entry_infor.remote_vid;
        memcpy(is_rspan_entry_valid.uplink, rspan_entry_infor.uplink, sizeof(rspan_entry_infor.uplink));
        memcpy(is_rspan_entry_valid.src_tx, rspan_entry_infor.src_tx, sizeof(rspan_entry_infor.src_tx));
        memcpy(is_rspan_entry_valid.src_rx, rspan_entry_infor.src_rx, sizeof(rspan_entry_infor.src_rx));

        /* Put the newest data in the structure */
        if (mode == VAL_mirrorType_both)
        {
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_tx, source_port);
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_rx, source_port);
        }
        else if (mode == VAL_mirrorType_rx)
        {
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_rx, source_port);
            RSPAN_OM_DEL_MEMBER(is_rspan_entry_valid.src_tx, source_port);
        }
        else
        {
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_tx, source_port);
            RSPAN_OM_DEL_MEMBER(is_rspan_entry_valid.src_rx, source_port);
        }
    }
    else
    {
        memset(is_rspan_entry_valid.src_tx, 0, sizeof(rspan_entry_infor.src_tx));
        memset(is_rspan_entry_valid.src_rx, 0, sizeof(rspan_entry_infor.src_rx));

        /* Put the newest data in the structure */
        if (mode == VAL_mirrorType_tx || mode == VAL_mirrorType_both)
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_tx, source_port);

        if (mode == VAL_mirrorType_rx || mode == VAL_mirrorType_both)
            RSPAN_OM_ADD_MEMBER(is_rspan_entry_valid.src_rx, source_port);
    }

    if (RSPAN_OM_SetRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    /* Check if config. needs to set chip */
    if (RSPAN_OM_IsSessionEntryCompleted( &is_rspan_entry_valid))
    {
        if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_source)
        {
            if (RSPAN_MGR_ReadyToSetMirrorChip(is_rspan_entry_valid) == FALSE)
            {
                return FALSE;
            }
        }

        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
        {
            for (j = 7; j >= 0; j--)
            {
                if ((is_rspan_entry_valid.uplink[i]&(0x1<<j)) != 0)
                {
                    uplink = (i * 8) + (8 - j);

                    /* Any RSPAN role needs to disable mac-learning in uplink ports */
                    ret = RSPAN_MGR_DisableMacLearning(uplink);

                    /* update the usage of uplinnk ports for AMTR */
                    if (ret == TRUE)
                        RSPAN_OM_SetRspanUplinkPortUsage(session_id, TRUE);
                }
            }
        }
    }

	return TRUE;
}

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
BOOL_T RSPAN_MGR_DeleteSessionSourceInterface(UI8_T session_id, UI8_T source_port, UI8_T mode)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid;
	RSPAN_OM_SessionEntry_T compared_entry;
    UI8_T   ret = -1 ; /* For debugging. */
    BOOL_T  is_completed = FALSE;
    I16_T    i, j, uplink;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }
   
    if (mode < VAL_mirrorType_rx || mode > VAL_mirrorType_both)
    {
        return FALSE;
    }

    if (source_port == 0)
    {
        return FALSE;
    }

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));
    is_rspan_entry_valid.session_id = session_id;

    /* to indicate this is src port */
    if (mode == VAL_mirrorType_tx || mode == VAL_mirrorType_both)
        is_rspan_entry_valid.src_tx[0] = source_port;

    if (mode == VAL_mirrorType_rx || mode == VAL_mirrorType_both)
        is_rspan_entry_valid.src_rx[0] = source_port;

    /* Check if config. needs to set chip. for deleting */
    if (RSPAN_OM_IsSessionEntryCompleted(&is_rspan_entry_valid))
    {
        memset(&compared_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

        if (RSPAN_OM_GetRspanSessionEntry(session_id, &compared_entry))
        {
            if (RSPAN_OM_IS_MEMBER(compared_entry.src_tx, source_port) ||
                RSPAN_OM_IS_MEMBER(compared_entry.src_rx, source_port))
            {
                RSPAN_MGR_ReadyToDelMirrorChip(is_rspan_entry_valid, compared_entry);
            }
		}
    }

    if (RSPAN_OM_DeleteRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
        return FALSE;
    }

    if (RSPAN_OM_RemoveSessionId(session_id) == FALSE)
    {
        /* Check if config. needs to set chip. for adding. This is only for source interface implementmentation.
         * test steps: (1) tx: 1/1,3,5 (2) add rx: 1/1,3,5 (3) delete tx:1/1,3,5 (4) result should be rx:1/1,3,5
         */
        if (RSPAN_OM_GetRspanSessionEntry(session_id, &is_rspan_entry_valid))
        {
            if (RSPAN_OM_IsSessionEntryCompleted(&is_rspan_entry_valid))
            {
                if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_source)
                {
                    if (RSPAN_MGR_ReadyToSetMirrorChip(is_rspan_entry_valid) == FALSE)
                    {
                        return FALSE;
                    }
                    is_completed = TRUE;
                }
            }

            /* Need to re-set the uplink setting for Mac-Learning issue, according to new setting of source ports */
            for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
            {
                for (j = 7; j >= 0; j--)
                {
                    if ((is_rspan_entry_valid.uplink[i]&(0x1 << j)) != 0)
                    {
                        uplink = (i * 8) + (8 - j);

                        if (is_completed == TRUE)
                        {
                            /* Every RSPAN role needs to disable mac-learning in uplink ports */
                            ret = RSPAN_MGR_DisableMacLearning(uplink);

                            /* update the usage of uplinnk ports for AMTR */
                            if (ret == TRUE)
                                RSPAN_OM_SetRspanUplinkPortUsage(session_id, TRUE);
                        }
                        else
                        {
                            /* Re-enable Mac Learning on this port again because there is no uplink relation existing */
                            ret = RSPAN_MGR_EnableMacLearning(uplink);

                            /* update the usage of uplinnk ports for AMTR. 2007/10/18. */
                            if (ret == TRUE)
                                RSPAN_OM_SetRspanUplinkPortUsage(session_id, FALSE);
                        }
                    }
                }
            }
        }
    }

	return TRUE;
}

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
BOOL_T RSPAN_MGR_SetSessionDestinationInterface(UI8_T session_id, UI8_T destination_port, UI8_T is_tagged)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid;
    RSPAN_OM_SessionEntry_T rspan_entry_infor;
    UI8_T compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,};
    I16_T i, j, uplink;
    I8_T ret = -1;
    VLAN_OM_VlanPortEntry_T vlan_port_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (is_tagged < VAL_rspanDstPortTag_untagged || is_tagged > VAL_rspanDstPortTag_tagged)
    {
        return FALSE;
    }

    if (destination_port == 0)
    {
        return FALSE;
    }

    if (VLAN_OM_GetVlanPortEntryByIfindex(destination_port, &vlan_port_entry) == FALSE)
    {
        return FALSE;
    }

    if (vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access)
    {
        return FALSE;
    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_GetSemaphore();
    if (SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry(destination_port, LEAF_rspanDstPort) == FALSE)
    {
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
        return FALSE;
    }
#else  /* SYS_CPNT_SYSCTRL_XOR == FALSE */
    /* Validate the setting port is a trunk(member) or user port. */
    if (SWCTRL_RspanSettingValidation(destination_port) == FALSE)
    {
        return FALSE;
    }
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));
    is_rspan_entry_valid.session_id = session_id;
    is_rspan_entry_valid.dst = destination_port;
    is_rspan_entry_valid.is_tagged = is_tagged;

    if (RSPAN_OM_SettingValidation(&is_rspan_entry_valid, destination_port) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    /* Get previous record in is_rspan_entry_valid structure data */
    if (RSPAN_OM_GetRspanSessionEntry(session_id, &rspan_entry_infor))
    {
        if (rspan_entry_infor.dst && (rspan_entry_infor.dst != is_rspan_entry_valid.dst))
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        if (rspan_entry_infor.switch_role && 
            (rspan_entry_infor.switch_role == VAL_rspanSwitchRole_intermediate ||
             rspan_entry_infor.switch_role == VAL_rspanSwitchRole_source))
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        if (!(! memcmp(compared, rspan_entry_infor.src_tx, sizeof(rspan_entry_infor.src_tx))
            || ! memcmp(compared, rspan_entry_infor.src_rx, sizeof(rspan_entry_infor.src_rx))))
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        is_rspan_entry_valid.switch_role = rspan_entry_infor.switch_role ;
        memcpy(is_rspan_entry_valid.src_tx, rspan_entry_infor.src_tx, sizeof(rspan_entry_infor.src_tx));
        memcpy(is_rspan_entry_valid.src_rx, rspan_entry_infor.src_rx, sizeof(rspan_entry_infor.src_rx));
        is_rspan_entry_valid.remote_vid = rspan_entry_infor.remote_vid ;
        memcpy(is_rspan_entry_valid.uplink, rspan_entry_infor.uplink, sizeof(rspan_entry_infor.uplink));
    }

    if (RSPAN_OM_SetRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
        return FALSE;
    }

    /* Check if session configuration pair is set completely */
    if (RSPAN_OM_IsSessionEntryCompleted(&is_rspan_entry_valid))
    {
        if (RSPAN_MGR_SetSessionVlanMembership(is_rspan_entry_valid) == FALSE)
        {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
            SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
            return FALSE;
        }

        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
        {
            for (j = 7; j >= 0; j--)
            {
                if ((is_rspan_entry_valid.uplink[i]&(0x1<<j)) != 0)
                {
                    uplink = (i * 8) + (8 - j);

                    /* Every RSPAN role needs to disable mac-learning in uplink ports */
                    ret = RSPAN_MGR_DisableMacLearning(uplink);

                    /* update the usage of uplinnk ports for AMTR */
                    if (ret == TRUE)
                        RSPAN_OM_SetRspanUplinkPortUsage(session_id, TRUE);
                }
            }
        }
    }
    /* Every destination port needs to increase maximum frame size */
    SWCTRL_ModifyMaxFrameSizeForRspan(destination_port, TRUE);

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteSessionDestinationInterface
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
BOOL_T RSPAN_MGR_DeleteSessionDestinationInterface(UI8_T session_id, UI8_T destination_port)
{
    RSPAN_OM_SessionEntry_T is_rspan_entry_valid, compared_entry;
    I16_T i, j, uplink;
    BOOL_T isDeletionNeeded = FALSE;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    	return FALSE;
    }

    if ( session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (destination_port == 0)
    {
        return FALSE;
    }

    /* Delete the vlan membership when session is competed */
    memset(&compared_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

    if (RSPAN_OM_GetRspanSessionEntry(session_id, &compared_entry) &&
        RSPAN_OM_IsSessionEntryCompleted(&compared_entry))
    {
        isDeletionNeeded = TRUE;
    }

    memset(&is_rspan_entry_valid, 0, sizeof(RSPAN_OM_SessionEntry_T));
    is_rspan_entry_valid.session_id = session_id;
    is_rspan_entry_valid.dst = destination_port;

    if (RSPAN_OM_DeleteRspanSessionEntry(&is_rspan_entry_valid) == FALSE)
    {
        return FALSE;
    }
    else
    {
        if (isDeletionNeeded)
        {
            RSPAN_OM_SessionEntry_T deal_vlan_entry;
            memset(&deal_vlan_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));
            deal_vlan_entry.dst = destination_port;
            deal_vlan_entry.is_tagged = compared_entry.is_tagged;
            deal_vlan_entry.remote_vid = compared_entry.remote_vid;

            if (RSPAN_MGR_DeleteSessionVlanMembership(deal_vlan_entry) == FALSE)
            {
                return FALSE;
            }

            /* Check if needing to remove all vlan membership */
            memset(&compared_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

            if (RSPAN_OM_GetRspanSessionEntry(session_id, &compared_entry))
            {
                if (RSPAN_OM_IsSessionEntryCompleted(&compared_entry) == FALSE)
                {
                    if (RSPAN_MGR_DeleteSessionVlanMembership(compared_entry) == FALSE)
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    if (RSPAN_OM_RemoveSessionId(session_id) == FALSE)
    {
        UI8_T ret = -1;
        
    	for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
        {
            for (j = 7; j >= 0; j--)
            {
                if ((is_rspan_entry_valid.uplink[i]&(0x1<<j)) != 0)
                {
                    uplink = (i * 8) + (8 - j);

                    /* Re-enable Mac Learning on this port again because there is no uplink relation existing. */
                    ret = RSPAN_MGR_EnableMacLearning(uplink);

                    /* update the usage of uplinnk ports for AMTR. 2007/10/18. */
                    if (ret == TRUE)
                        RSPAN_OM_SetRspanUplinkPortUsage(session_id, FALSE);
                }
            }
        }
    }
    /* When delete a destination port,needs to reduce maximum frame size */
    SWCTRL_ModifyMaxFrameSizeForRspan(destination_port,FALSE);

	return TRUE;
}

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
BOOL_T RSPAN_MGR_GetRspanSessionEntry(UI8_T session_id, RSPAN_OM_SessionEntry_T *rspan_entry)
{
    if (session_id == 0 || session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    if (NULL != rspan_entry)
    {
        if (RSPAN_OM_GetRspanSessionEntry(session_id, rspan_entry) == FALSE)
        {
            return FALSE;
        }

        rspan_entry->snmpEntryStatus = VAL_rspanStatus_valid;
        return TRUE;
    }

    return FALSE;
}

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
BOOL_T RSPAN_MGR_GetNextRspanSessionEntry(UI8_T *session_id, RSPAN_OM_SessionEntry_T *rspan_entry)
{
    UI8_T session_incre_cnt = *session_id;

    if (NULL != rspan_entry)
    {
        if (session_incre_cnt == 0) /* when the CLI passes 0 at the first time, meaning CLI needs all session data. */
            session_incre_cnt = 1;
        else
            session_incre_cnt ++;

        while ((session_incre_cnt <= RSPAN_MGR_MAX_SESSION_NUM) &&
               (RSPAN_OM_GetRspanSessionEntry(session_incre_cnt, rspan_entry) == FALSE))
        {
            session_incre_cnt ++;
        }

        if (session_incre_cnt > RSPAN_MGR_MAX_SESSION_NUM)
        {
            return FALSE;
        }

        *session_id = session_incre_cnt;
        rspan_entry->snmpEntryStatus = VAL_rspanStatus_valid;
        return TRUE;
    }

    return FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SettingValidation
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if items of a session entry are valid to set the database.
 * INPUT    : The pointer of the session structure.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The entry is valid for setting the database.
 *            FALSE       -- The entry is not valid.
 * NOTES    : This API is for swctrl functions calling.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_SettingValidation(RSPAN_OM_SessionEntry_T *is_rspan_entry_valid, UI8_T target_port)
{
    if (RSPAN_OM_SettingValidation(is_rspan_entry_valid, target_port) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T RSPAN_MGR_IsRspanVlan(UI32_T vid)
{
    if (RSPAN_OM_IsRspanVlan(vid) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T RSPAN_MGR_SetRspanSessionEntry(RSPAN_OM_SessionEntry_T *rspan_entry)
{
    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (RSPAN_OM_SetRspanSessionEntry(rspan_entry) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T RSPAN_MGR_GetSessionEntryCounter(UI8_T *session_cnt)
{
    RSPAN_OM_GetSessionEntryCounter(session_cnt);
    return TRUE;
}

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
BOOL_T RSPAN_MGR_DeleteRspanSessionEntry(RSPAN_OM_SessionEntry_T *rspan_entry)
{
    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (RSPAN_OM_DeleteRspanSessionEntry(rspan_entry) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

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
BOOL_T RSPAN_MGR_IsSessionEntryCompleted(UI8_T session_id)
{
    RSPAN_OM_SessionEntry_T rspan_entry ;

    if (!RSPAN_OM_GetRspanSessionEntry(session_id, &rspan_entry))
    {
        return FALSE;
    }

    if (RSPAN_OM_IsSessionEntryCompleted(&rspan_entry) == TRUE)
        return TRUE;
    else
        return FALSE;
}

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
BOOL_T RSPAN_MGR_RemoveCliWebSessionId(UI8_T session_id)
{
    RSPAN_OM_SessionEntry_T rspan_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    rspan_entry.session_id = session_id;

    if (RSPAN_OM_GetRspanSessionEntry(rspan_entry.session_id, &rspan_entry) == FALSE)
    {
        return FALSE;
    }

    if (RSPAN_MGR_RemoveSnmpSessionProcess(rspan_entry) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

/* 802.1X and port security will need this. 2007/09/20 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_IsRspanUplinkPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will test whether the ifindex is a RSPAN mirrored port.
 * INPUT   : ifindex -- this interface index
 * OUTPUT  : None
 * RETURN  : TRUE : The ifindex is a RSPAN uplink port
 *           FALSE: The ifindex is not a RSPAN uplink port
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_IsRspanUplinkPort(UI32_T ifindex)
{
    return RSPAN_OM_IsRspanUplinkPort(ifindex);
}

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
BOOL_T RSPAN_MGR_IsRspanUplinkPortUsed(UI32_T ifindex)
{
    return RSPAN_OM_IsRspanUplinkPortUsed(ifindex);
}

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
BOOL_T RSPAN_MGR_IsRspanMirrorToPort(UI32_T ifindex)
{
    return RSPAN_OM_IsRspanMirrorToPort(ifindex);
}

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
BOOL_T RSPAN_MGR_IsRspanMirroredPort(UI32_T ifindex)
{
    return RSPAN_OM_IsRspanMirroredPort(ifindex);
}
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
BOOL_T RSPAN_MGR_SetRspanVlanStatus(UI32_T vlan_id, UI32_T rspan_status)
{
    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
    {
        if (VLAN_OM_IsVlanExisted(vlan_id))  /* vlan id exists */
        {
            return FALSE;
        }

        if (VLAN_MGR_CreateRspanVlan(vlan_id, VAL_dot1qVlanStatus_permanent)== FALSE)
        {
            return FALSE;
        }

        /* Set this vlan id as active */
        VLAN_MGR_SetDot1qVlanStaticRowStatus(vlan_id, VAL_dot1qVlanStaticRowStatus_active);
    }
    else if (rspan_status == VAL_vlanStaticExtRspanStatus_destroy)
    {
        if (VLAN_MGR_DeleteRspanVlan(vlan_id, VAL_dot1qVlanStatus_permanent)== FALSE)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    if (rspan_status == VAL_vlanStaticExtRspanStatus_rspanVlan)
        RSPAN_OM_SetRspanVlanEntry(vlan_id);

    if (rspan_status == VAL_vlanStaticExtRspanStatus_destroy)
        RSPAN_OM_DeleteRspanVlanEntry(vlan_id);

    return TRUE;

}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_GetRspanVlanEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the status of the specific VLAN.
 * INPUT    : entry->rspanVlanId   -- The given VLAN id.
 * OUTPUT   : Information of the entry.
 * RETURN   : TRUE          -- The configuration is set successfully.
 *            FALSE         -- The configuration isn't set successfully.
 * NOTES    : Need to fill the VLAN field when passing pointer.
 *            This API will display all static vlans, including normal.
 *--------------------------------------------------------------------------
 */
BOOL_T RSPAN_MGR_GetRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry)
{
    VLAN_MGR_Dot1qVlanStaticEntry_T vlan_static_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (VLAN_OM_GetDot1qVlanStaticEntry(entry->rspanVlanId, &vlan_static_entry) == FALSE)
    {
        return FALSE;
    }

    if (RSPAN_MGR_IsRspanVlan(entry->rspanVlanId))
        entry->rspanVlanStatus = VAL_vlanStaticExtRspanStatus_rspanVlan;
    else
        entry->rspanVlanStatus = VAL_vlanStaticExtRspanStatus_vlan;

    return TRUE;
}

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
BOOL_T RSPAN_MGR_GetNextRspanVlanEntry(RSPAN_MGR_RspanVlan_T *entry)
{
    VLAN_MGR_Dot1qVlanStaticEntry_T vlan_static_entry;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (VLAN_OM_GetNextDot1qVlanStaticEntry( &(entry->rspanVlanId), &vlan_static_entry) == FALSE)
    {
        return FALSE;
    }

    if (RSPAN_MGR_IsRspanVlan(entry->rspanVlanId))
        entry->rspanVlanStatus = VAL_vlanStaticExtRspanStatus_rspanVlan;
    else
        entry->rspanVlanStatus = VAL_vlanStaticExtRspanStatus_vlan;

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME -  RSPAN_MGR_SetRspanEntryStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Create/Delete RSPAN Entry through SNMP.
 * INPUT    : The structure of RSPAN data.
 *            Which field is going to modify/delete.
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
BOOL_T  RSPAN_MGR_SetRspanEntryStatus(RSPAN_OM_SessionEntry_T *entry , UI8_T field_id)
{
    RSPAN_OM_SessionEntry_T compared_rspan_entry, removed_rspan_entry;
    BOOL_T isCreated = FALSE;
    BOOL_T ret = TRUE;

    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (entry->session_id == 0 || entry->session_id > RSPAN_MGR_MAX_SESSION_NUM)
    {
        return FALSE;
    }

    memset(&compared_rspan_entry, 0, sizeof(RSPAN_OM_SessionEntry_T));

    /* Check this session is created or not. 
     * If yes, then it can allow to do destroying or editing 
     */
    if (!(isCreated = RSPAN_OM_GetRspanSessionEntry(entry->session_id, &compared_rspan_entry)))
    {
        /*SYSFUN_Debug_Printf("\r\n RSPAN_MGR_SetRspanEntryStatus: There is no data to get for this session id.\r\n");*/
    }

    if (entry-> snmpEntryStatus == VAL_rspanStatus_valid && field_id == LEAF_rspanStatus) /* Create a new session */
    {
        if (isCreated == FALSE)
        {
            compared_rspan_entry.session_id = entry->session_id; /* create a new session id for rspan */

            /* all fields for this new session id are default values. */
            if (RSPAN_OM_SetRspanSessionEntry(&compared_rspan_entry) == FALSE)
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else if ( entry-> snmpEntryStatus == VAL_rspanStatus_invalid && field_id == LEAF_rspanStatus ) /* Destory a session. */
    {
        if (isCreated == TRUE)
        {
            /* remove vlan membership, chip setting and OM data */
            if (RSPAN_MGR_RemoveSnmpSessionProcess(compared_rspan_entry) == FALSE)
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }
    else /* Edit the special item of specific session id. */
    {
        if (isCreated == FALSE)
        {
            return FALSE;
        }

        /* For removing data later */
        memcpy(&removed_rspan_entry, &compared_rspan_entry, sizeof(RSPAN_OM_SessionEntry_T));

        switch (field_id)
        {
            case LEAF_rspanSrcTxPorts:
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_GetSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                if (RSPAN_MGR_ValidateSnmpSessionProcess(entry->session_id, entry->src_tx, VAL_rspanSwitchRole_source, LEAF_rspanSrcTxPorts))
                    memcpy(compared_rspan_entry.src_tx, entry->src_tx, sizeof(compared_rspan_entry.src_tx));
                else
                    ret = FALSE;
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif  /* SYS_CPNT_SYSCTRL_XOR == FALSE */
                break;

            case LEAF_rspanSrcRxPorts:
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_GetSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

                if (RSPAN_MGR_ValidateSnmpSessionProcess ( entry->session_id, entry->src_rx , VAL_rspanSwitchRole_source, LEAF_rspanSrcRxPorts))
                    memcpy(compared_rspan_entry.src_rx, entry->src_rx, sizeof(compared_rspan_entry.src_rx));
                else
                    ret = FALSE;
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif  /* SYS_CPNT_SYSCTRL_XOR == FALSE */
                break;

            case LEAF_rspanDstPort:
            {
                UI32_T my_unit_id;

                if (entry->dst == 0)
                {
                    compared_rspan_entry.dst = entry->dst;
                    break;
                }

                STKTPLG_POM_GetMyUnitID(&my_unit_id);
                if (entry->dst <= (SWCTRL_UIGetUnitPortNumber(my_unit_id)) && entry->dst >= 0)
                {
                    if (RSPAN_MGR_ValidateSnmpSessionProcess(entry->session_id, &(entry->dst), VAL_rspanSwitchRole_destination, LEAF_rspanDstPort))
                        compared_rspan_entry.dst = entry->dst;
                }
                else
                {
                    ret = FALSE ;
                }
                break;
            }

            case LEAF_rspanDstPortTag:
                if (entry->is_tagged == VAL_rspanDstPortTag_untagged ||
                    entry->is_tagged == VAL_rspanDstPortTag_tagged ||
                    entry->is_tagged == VAL_rspanDstPortTag_none)
                {
                    if (entry->is_tagged == VAL_rspanDstPortTag_none)
                        compared_rspan_entry.is_tagged = 0; /* default value for tag */
                    else
                        compared_rspan_entry.is_tagged = entry->is_tagged;
                }
                else
                    ret = FALSE ;
                break;

            case LEAF_rspanSwitchRole:
                if (entry->switch_role == VAL_rspanSwitchRole_source ||
                    entry->switch_role == VAL_rspanSwitchRole_intermediate ||
                    entry->switch_role == VAL_rspanSwitchRole_destination ||
                    entry->switch_role == VAL_rspanSwitchRole_none)
                {
                    if (entry->switch_role == VAL_rspanSwitchRole_none)
                        compared_rspan_entry.switch_role = 0; /* default value for switch role */
                    else
                        compared_rspan_entry.switch_role = entry->switch_role;
                }
                else
                    ret = FALSE ;
                break;

            case LEAF_rspanRemotePorts :
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_GetSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                if (RSPAN_MGR_ValidateSnmpSessionProcess(entry->session_id, entry->uplink, entry->switch_role, LEAF_rspanRemotePorts))
                    memcpy(compared_rspan_entry.uplink, entry->uplink, sizeof(compared_rspan_entry.uplink));
                else
                    ret = FALSE;
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif  /* SYS_CPNT_SYSCTRL_XOR == FALSE */
                break;

            case LEAF_rspanRemoteVlanId :
                if (entry->remote_vid > 0 && entry->remote_vid <= SYS_ADPT_MAX_VLAN_ID
                    && RSPAN_OM_IsRspanVlan(entry->remote_vid)) /* Make sure RSPAN VLAN is already created. */
                {
                    compared_rspan_entry.remote_vid = entry->remote_vid;
                }
                else
                    ret = FALSE;
                break;

            default:
                printf ("This field id is unknown. field_id=[%d]/n", field_id);
                break;
        }

        if (ret == TRUE)
        {
            /* Check if the current structure set is reasonable for the specific switch role before saving into database */
            if (RSPAN_MGR_ValidateSnmpSwitchRole(compared_rspan_entry, field_id))
            {
                /* remove vlan membership, chip setting and OM data */
                if (RSPAN_MGR_RemoveSnmpSessionProcess(removed_rspan_entry))
                {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                    SYSCTRL_XOR_MGR_GetSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                    /* Re-create this session id, all data ,and vlan membership, chip setting, mac learning if needed. */
                    if (RSPAN_MGR_ResaveSnmpSessionProcess(compared_rspan_entry) == FALSE)
                    {
#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                        SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                        return FALSE;
                    }

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
                    SYSCTRL_XOR_MGR_ReleaseSemaphore();
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */
                }
                else
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return FALSE;
        }
    }

    return TRUE;

}

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
void RSPAN_MGR_VlanActiveToSuspendCallback(UI32_T vid_ifindex, UI32_T vlan_status)
{
    RSPAN_OM_SessionEntry_T session_entry;
    SYS_TYPE_Uport_T user_port;
    UI32_T vid;
    BOOL_T found;
    int i, j;
    UI32_T ifindex;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    found = FALSE;

    for (i = 0; i < RSPAN_MGR_MAX_SESSION_NUM; i++)
    {
        if (RSPAN_OM_GetRspanSessionEntry(i + 1, &session_entry) == TRUE)
        {
            if (session_entry.remote_vid == vid)
            {
                found = TRUE;
                break;
            }
        }
    }

    if (found)
    {
        if (session_entry.switch_role == VAL_rspanSwitchRole_source)
        {
            for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
            {
                for (j = 7; j >= 0; j--)
                {
                    if ((session_entry.src_tx[i] & (1 << j)) ||
                        (session_entry.src_rx[i] & (1 << j)))
                    {
                        /* deal with source port */
                        ifindex = i * 8 + 8 - j;
                        user_port.unit = RSPAN_IFINDEX_TO_UNIT(ifindex);
                        user_port.port = RSPAN_IFINDEX_TO_PORT(ifindex);

                        if (SWDRV_DeletePortMirroring(user_port, user_port) == FALSE)
                        {
                            return;
                        }
                    }
                    else if (session_entry.uplink[i] & (1 << j))
                    {
                        /* deal with uplink port */
                        ifindex = i * 8 + 8 - j;
                        user_port.unit = RSPAN_IFINDEX_TO_UNIT(ifindex);
                        user_port.port = RSPAN_IFINDEX_TO_PORT(ifindex);

                        if (SWDRV_SetRspanVlanTag(user_port, 0, 0) == FALSE)
                        {
                            return;
                        }
                    }
                }
            }
        }
    }
}

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
void RSPAN_MGR_VlanSuspendToActiveCallback(UI32_T vid_ifindex, UI32_T vlan_status)
{
    RSPAN_OM_SessionEntry_T session_entry;
    SYS_TYPE_Uport_T user_port, rx_to_port, tx_to_port;
    UI32_T vid;
    BOOL_T found;
    int i, j;
    UI32_T source_port, uplink_port;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    found = FALSE;

    for (i = 0; i < RSPAN_MGR_MAX_SESSION_NUM; i++)
    {
        if (RSPAN_OM_GetRspanSessionEntry(i + 1, &session_entry) == TRUE)
        {
            if (session_entry.remote_vid == vid)
            {
                found = TRUE;
                break;
            }
        }
    }

    if (found)
    {
        if (session_entry.switch_role == VAL_rspanSwitchRole_source)
        {
            /* deal with uplink port */
            uplink_port = 0;

            for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
            {
                if (session_entry.uplink[i] == 0)
                {
                    continue;
                }

                for (j = 7; j >= 0; j--)
                {
                    if (session_entry.uplink[i] & (1 << j))
                    {
                        uplink_port = i * 8 + (8 - j);
                        user_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_port);
                        user_port.port = RSPAN_IFINDEX_TO_PORT(uplink_port);

                        if (SWDRV_SetRspanVlanTag(user_port, 0x8100, vid) == FALSE)
                        {
                            return;
                        }
                        break;
                    }
                }

                if (uplink_port)
                {
                    break;
                }
            }

            /* deal with source port */
            for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
            {
                for (j = 7; j >= 0; j--)
                {
                    if ((session_entry.src_tx[i] & (1 << j)) &&
                        (session_entry.src_rx[i] & (1 << j)))
                    {
                        if (uplink_port)
                        {
                            tx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_port);
                            tx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_port);
                            rx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_port);
                            rx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_port);
                        }
                        else
                        {
                            tx_to_port.unit = tx_to_port.port =
                            rx_to_port.unit = rx_to_port.port = 0;
                        }
                    }
                    else if (session_entry.src_tx[i] & (1 << j))
                    {
                        if (uplink_port)
                        {
                            tx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_port);
                            tx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_port);
                        }
                        else
                        {
                            tx_to_port.unit = tx_to_port.port = 0;
                        }

                        rx_to_port.unit = rx_to_port.port = 0;
                    }
                    else if (session_entry.src_rx[i] & (1 << j))
                    {
                        tx_to_port.unit = tx_to_port.port = 0;

                        if (uplink_port)
                        {
                            rx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_port);
                            rx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_port);
                        }
                        else
                        {
                            rx_to_port.unit = rx_to_port.port = 0;
                        }
                    }
                    else
                    {
                        continue;
                    }

                    source_port = i * 8 + (8 - j);
                    user_port.unit = RSPAN_IFINDEX_TO_UNIT(source_port);
                    user_port.port = RSPAN_IFINDEX_TO_PORT(source_port);

                    if (SWDRV_SetPortMirroring(user_port, rx_to_port, tx_to_port) == FALSE)
                    {
                        return;
                    }
                }
            }
        }
    }
}

/* Re-create this session id, all data ,and vlan membership, chip setting, mac learning if needed. */
static BOOL_T RSPAN_MGR_ResaveSnmpSessionProcess(RSPAN_OM_SessionEntry_T entry)
{
    I8_T ret = -1;
    I16_T i, j, targetPort;
    UI8_T compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,};

    if (RSPAN_OM_SetRspanSessionEntry(&entry) == FALSE)
    {
        return FALSE;
    }

    if (memcmp(compared, entry.uplink, sizeof(entry.uplink)) != 0)
    {
        RSPAN_OM_IncreaseSrcUplinkCounter(entry.session_id);
    }

    /* Check if config. needs to set chip */
    if (RSPAN_OM_IsSessionEntryCompleted(&entry))
    {
        if (RSPAN_MGR_SetSessionVlanMembership(entry) == FALSE)
        {
            return FALSE;
        }

        if (entry.switch_role == VAL_rspanSwitchRole_source)
        {
            if (RSPAN_MGR_ReadyToSetMirrorChip(entry) == FALSE)
            {
                return FALSE;
            }
        }

    	for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
        {
            for (j = 7; j >= 0; j --)
            {
                if ((entry.uplink[i] & (0x1<<j)) != 0)
                {
                    targetPort = (i * 8) + (8 - j);

                    /* Every RSPAN role needs to disable mac-learning in uplink ports. */
                    ret = RSPAN_MGR_DisableMacLearning(targetPort);
                    /* Every uplink port needs to increase maximum frame size */
                    SWCTRL_ModifyMaxFrameSizeForRspan(targetPort, TRUE);
                }
            }
        }

        /* update the usage of uplinnk ports for AMTR. 2007/10/18. */
        if (ret == TRUE)
            RSPAN_OM_SetRspanUplinkPortUsage(entry.session_id, TRUE);
    }

    if (entry.switch_role == VAL_rspanSwitchRole_destination)
    {
        SWCTRL_ModifyMaxFrameSizeForRspan(entry.dst, TRUE);
    }

    return TRUE;

}

/* Check if the current structure set is reasonable for the specific switch role before saving it into database */
static BOOL_T RSPAN_MGR_ValidateSnmpSwitchRole(RSPAN_OM_SessionEntry_T entry, UI8_T field_id)
{
    UI8_T compared[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0,};

    /* Rule 1 : src tx/rx field can't set if switch role isn't src or uplink port more than one or dst fields are set */
    if (( field_id == LEAF_rspanSrcTxPorts || field_id == LEAF_rspanSrcRxPorts )
        && ((entry.switch_role != VAL_rspanSwitchRole_source && entry.switch_role != 0)
            || RSPAN_MGR_IsSrcUplinkCountValid(entry.uplink) == FALSE
            || entry.dst != 0 || entry.is_tagged != 0))
    {
        if (memcmp(compared, entry.src_tx, sizeof(entry.src_tx)) != 0 ||
            memcmp(compared, entry.src_tx, sizeof(entry.src_rx)) != 0)
        {
            return FALSE;
        }
    }

    /* Rule 2 : dst port or tag field can't set if switch role isn't dst or src tx/rx isn't zero */
    if ((field_id == LEAF_rspanDstPort || field_id == LEAF_rspanDstPortTag)
        && ((entry.switch_role != VAL_rspanSwitchRole_destination && entry.switch_role != 0)
            || memcmp(compared, entry.src_tx, sizeof(entry.src_tx)) != 0
            || memcmp(compared, entry.src_tx, sizeof(entry.src_rx)) != 0 ))
    {
        if (entry.dst != 0 || entry.is_tagged != 0)
        {
            return FALSE;
        }
    }

    /* Rule 3 : specific role value can't set if there are some incompatible field values existed. */
    if (field_id == LEAF_rspanSwitchRole)
    {
        if (entry.switch_role != 0)
        {
            if (entry.switch_role == VAL_rspanSwitchRole_source)
            {
                if (entry.dst != 0 || entry.is_tagged != 0 ||
                    RSPAN_MGR_IsSrcUplinkCountValid(entry.uplink) == FALSE)
                {
                    return FALSE;
                }
            }

            if (entry.switch_role == VAL_rspanSwitchRole_intermediate)
            {
                if (memcmp(compared, entry.src_tx, sizeof(entry.src_tx)) != 0 ||
                    memcmp (compared, entry.src_tx, sizeof(entry.src_rx)) != 0)
                    return FALSE;

                if (entry.dst != 0 || entry.is_tagged != 0)
                    return FALSE ;
            }

            if (entry.switch_role == VAL_rspanSwitchRole_destination)
            {
                if (memcmp (compared, entry.src_tx, sizeof(entry.src_tx)) != 0 ||
                    memcmp (compared, entry.src_tx, sizeof(entry.src_rx)) != 0 )
                    return FALSE;
            }
        }
    }

    /* Rule 4 :  uplink can't set if its role is source with one uplink set already */
    if (field_id == LEAF_rspanRemotePorts && memcmp(compared, entry.uplink, sizeof(entry.uplink)) != 0)
    {
        if (entry.switch_role == VAL_rspanSwitchRole_source ||
            memcmp(compared, entry.src_tx, sizeof(entry.src_tx)) != 0 ||
            memcmp(compared, entry.src_tx, sizeof(entry.src_rx)) != 0)
        {
            if (RSPAN_MGR_IsSrcUplinkCountValid(entry.uplink) == FALSE)
                return FALSE;
        }
    }

    /* Rule 5 :  dst tag field can't set if its role isn't dst or source related fields are set */
    if (field_id == LEAF_rspanDstPortTag && entry.is_tagged != 0)
    {
        if ((entry.switch_role != VAL_rspanSwitchRole_destination && entry.switch_role != 0) ||
            memcmp(compared, entry.src_tx, sizeof(entry.src_tx)) != 0 ||
            memcmp(compared, entry.src_tx, sizeof(entry.src_rx)) != 0 )
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* Check if the number of uplink ports is valid when switch role is source */
static BOOL_T RSPAN_MGR_IsSrcUplinkCountValid(UI8_T *port_bit)
{
    I16_T i, j, count = 0;

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((port_bit[i] &(0x1<<j)) != 0)
            {
                count ++;
            }
        }
    }

    if (count > RSPAN_MGR_MAX_SRC_UPLINK_NUM)
        return FALSE;
    else
        return TRUE;
}

/* Check if this port is already set as security, mirror, pvlan, trunk member and etc */
static BOOL_T RSPAN_MGR_ValidateSnmpSessionProcess(UI8_T session_id, UI8_T *port_bit , UI8_T switch_role, UI8_T port_list_type)
{
    I16_T i, j, targetPort;

    if (port_list_type == LEAF_rspanDstPort)
    {
        if (RSPAN_MGR_ValidateSnmpSessionCommonProcess(session_id, *port_bit, switch_role, port_list_type) == TRUE)
            return TRUE;
        else
            return FALSE;
    }

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((port_bit[i] &(0x1<<j)) != 0)
            {
                targetPort = (i * 8) + (8 - j);

                if (RSPAN_MGR_ValidateSnmpSessionCommonProcess(session_id, targetPort, switch_role, port_list_type) == FALSE)
                    return FALSE;
            }
        }
    }

    return TRUE;
}

static BOOL_T RSPAN_MGR_ValidateSnmpSessionCommonProcess ( UI8_T session_id, I16_T targetPort, UI8_T switch_role, UI8_T port_list_type )
{

#if (SYS_CPNT_SYSCTRL_XOR == TRUE)
    if (SYSCTRL_XOR_MGR_PermitBeingSetRspanEntry(targetPort, port_list_type) == FALSE)
    {
        return FALSE;
    }
#else  /* SYS_CPNT_SYSCTRL_XOR == FALSE */

    /* Validate the setting port is a trunk(member) or user port */
    if (SWCTRL_RspanSettingValidation(targetPort))
    {
        return FALSE;
    }

    /* if this port is used for local setting */
    if ( RSPAN_OM_IsLocalMirrorPort(targetPort))
    {
        return FALSE;
    }
#endif /* End of #if (SYS_CPNT_SYSCTRL_XOR == TRUE) */

    /* Check if this port is already used in any sessions */
    if (RSPAN_OM_ValidateRspanPortRelation(session_id, targetPort, port_list_type) == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL_T RSPAN_MGR_RemoveSnmpSessionProcess(RSPAN_OM_SessionEntry_T compared_rspan_entry)
{
    UI8_T ret, uplink;
    I16_T i, j;

    /* Check if this session is already complete */
    if (RSPAN_OM_IsSessionEntryCompleted(&compared_rspan_entry))
    {
        if (compared_rspan_entry.switch_role == VAL_rspanSwitchRole_source)
        {
            if (RSPAN_MGR_ReadyToDelMirrorChip(compared_rspan_entry, compared_rspan_entry) == FALSE)
            {
                return FALSE;
            }
        }

        if (RSPAN_MGR_DeleteSessionVlanMembership(compared_rspan_entry) == FALSE)
        {
            return FALSE;
        }

        /* Need to re-set the uplink setting for Mac-Learning issue, according to new setting of source ports */
        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
        {
            for (j = 7; j >= 0; j--)
            {
                if ((compared_rspan_entry.uplink[i]&(0x1<<j)) != 0)
                {
                    uplink = (i * 8) + (8 - j);

                    /* Re-enable Mac Learning on this port again because there is no uplink relation existing */
                    ret = RSPAN_MGR_EnableMacLearning(uplink);

                    /* When delete a uplink port,needs to reduce maximum frame size */
                    SWCTRL_ModifyMaxFrameSizeForRspan(uplink, FALSE);

                    /* update the usage of uplinnk ports for AMTR */
                    if (ret == TRUE)
                        RSPAN_OM_SetRspanUplinkPortUsage(compared_rspan_entry.session_id, FALSE);
                }
            }
        }
    }

    if (compared_rspan_entry.switch_role == VAL_rspanSwitchRole_destination)
    {
        SWCTRL_ModifyMaxFrameSizeForRspan(compared_rspan_entry.dst, FALSE);
    }

    /* remove all data of this session id from struture */
    if (RSPAN_OM_RemoveSessionIdEntry( compared_rspan_entry.session_id) == FALSE)
    {
        return FALSE;
    }

    if (compared_rspan_entry.switch_role == VAL_rspanSwitchRole_source)
    {
        RSPAN_OM_DecreaseSrcUplinkCounter(compared_rspan_entry.session_id);
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_ReadyToSetMirrorChip
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if RSPAN Configuration is ready to set chip and set the
 *            relative configuration in the chip.
 * INPUT    : The whole data structure to store the value
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_ReadyToSetMirrorChip(RSPAN_OM_SessionEntry_T is_rspan_entry_valid)
{
    I16_T i, j, k;
    UI8_T uplink_mtp [2]={0,}, mtp_cnt = 0; /* For source rspan session, there are only one uplinks allowed */
    SYS_TYPE_Uport_T src_unit_port, rx_to_port, tx_to_port;
    UI8_T src_port = 0, src_mode;
    UI8_T session_cnt = 0;
    UI16_T tpid = 0x8100; /* For now, the tpid is fixed to 0x8100. */

    /* Deal with uplink ports */
    for (i = 0; (i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST); i++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((is_rspan_entry_valid.uplink[i] & (0x1<<j)) != 0)
            {
                uplink_mtp [mtp_cnt] = (i * 8) + (8 - j);

                if (mtp_cnt < 1)
                    mtp_cnt ++;
                else
                {
                    return FALSE;
                }
            }
        }
    }

    /* Deal with source ports */
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        for (j = 7; j >= 0; j --)
        {
            if (((I16_T)is_rspan_entry_valid.src_tx[i] & (0x1<<j))!=0
                && (is_rspan_entry_valid.src_rx[i] & (0x1<<j))!=0 )
                src_mode = VAL_mirrorType_both;
            else if ( (is_rspan_entry_valid.src_tx[i] & (0x1<<j))!=0)
                src_mode = VAL_mirrorType_tx;
            else if ( (is_rspan_entry_valid.src_rx[i] & (0x1<<j))!=0)
                src_mode = VAL_mirrorType_rx;
            else
                src_mode = 0;

            for (k = 0; k < mtp_cnt && src_mode; k ++)
            {
                if (src_mode == VAL_mirrorType_both)
                {
                    tx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp [k]);
                    tx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp [k]);
                    rx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp [k]);
                    rx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp [k]);

                }
                else if (src_mode == VAL_mirrorType_tx)
                {
                    tx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp [k]);
                    tx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp [k]);
                    rx_to_port.unit = 0;
                    rx_to_port.port = 0;
                }
                else if (src_mode == VAL_mirrorType_rx)
                {
                    tx_to_port.unit = 0;
                    tx_to_port.port = 0;
                    rx_to_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp [k]);
                    rx_to_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp [k]);
                }
                else
                {
                    continue;
                }

                if (src_mode)
                {
                    src_port = (UI8_T)((i * 8) + (8 - j));
                    src_unit_port.unit = RSPAN_IFINDEX_TO_UNIT(src_port);
                    src_unit_port.port = RSPAN_IFINDEX_TO_PORT(src_port);
                    RSPAN_OM_GetSessionEntryCounter(&session_cnt);

                    /* Set Mirror relation successfully */
                    if (SWDRV_SetPortMirroring(src_unit_port, rx_to_port, tx_to_port) == FALSE)
                    {
                        return FALSE;
                    }

                    /* Set RSPAN VLAN successfully */
                    src_unit_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp[k]);
                    src_unit_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp[k]);

                    if (SWDRV_SetRspanVlanTag(src_unit_port, tpid, is_rspan_entry_valid.remote_vid) == FALSE)
                    {
                        return FALSE;
                    }
				} /* End of src_role*/
            } /* end of for (k=0;k<mtp_cnt;k++) */
        } /* end of for (j=7;j>=0;j--) */
    } /* for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++) */

	return TRUE ;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_ReadyToDelMirrorChip
 *--------------------------------------------------------------------------
 * PURPOSE  : Check if RSPAN Configuration is ready to delete chip
 *
 * INPUT    : is_rspan_entry_valid -- The whole data structure to del the value.
 *            compared_entry       -- The whole data structure to compare the value.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_ReadyToDelMirrorChip(RSPAN_OM_SessionEntry_T is_rspan_entry_valid , RSPAN_OM_SessionEntry_T compared_entry)
{
    I16_T i, j ;
    SYS_TYPE_Uport_T src_unit_port;
    UI8_T src_port = 0, src_mode = 0;
    UI8_T uplink_mtp = 0, mtp_cnt = 0; /* For source rspan session, there are only one uplinks allowed */
    UI16_T tpid = 0, tag_vid = 0;

    /* Deal with uplink ports */
    for (i = 0; (i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST); i++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((is_rspan_entry_valid.uplink[i] & (0x1<<j))!=0)
            {
                uplink_mtp = (i * 8) + (8 - j);

                if (mtp_cnt < 1)
                    mtp_cnt ++;
                else
                {
                    return FALSE;
                }
            }
        }
    }

    /* Deal with source ports */
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
    {
        for (j = 7; j >= 0; j --)
        {
            if (((I16_T)compared_entry.src_tx[i] & (0x1<<j)) != 0
                && ((I16_T)compared_entry.src_rx[i] & (0x1<<j)) != 0)
                src_mode = VAL_mirrorType_both;
            else if (((I16_T)compared_entry.src_tx[i] & (0x1<<j)) != 0)
                src_mode = VAL_mirrorType_tx;
            else if (((I16_T)compared_entry.src_rx[i] & (0x1<<j)) != 0)
                src_mode = VAL_mirrorType_rx;

            if (src_mode)
            {
                src_port = (UI8_T)((i * 8) + (8 - j));
                src_unit_port.unit = RSPAN_IFINDEX_TO_UNIT(src_port);
                src_unit_port.port = RSPAN_IFINDEX_TO_PORT(src_port);

                /* Set Mirror relation successfully. */
                if (SWDRV_DeletePortMirroring(src_unit_port,src_unit_port) == FALSE)
                {
                    return FALSE;
                }

                /* Set RSPAN VLAN successfully. */
                src_unit_port.unit = RSPAN_IFINDEX_TO_UNIT(uplink_mtp);
                src_unit_port.port = RSPAN_IFINDEX_TO_PORT(uplink_mtp);

                if (SWDRV_SetRspanVlanTag(src_unit_port, tpid, tag_vid) == FALSE)
                {
                    return FALSE;
                }
            } /* End of src_mode */

            src_mode = 0;
            src_port = 0;
            src_unit_port.unit = 0;
            src_unit_port.port = 0;
        }
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DisableMacLearning
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the Mac-Learning of uplink ports.
 * INPUT    : The uplink port.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_DisableMacLearning(UI8_T uplink)
{
    if (!SWCTRL_SetPortLearningStatus(uplink, FALSE, SWCTRL_LEARNING_DISABLED_BY_RSPAN))
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_EnableMacLearning
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable the Mac-Learning of uplink ports.
 * INPUT    : The uplink port.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_EnableMacLearning(UI8_T uplink)
{
    if (!SWCTRL_SetPortLearningStatus(uplink, TRUE, SWCTRL_LEARNING_DISABLED_BY_RSPAN))
    {
        return FALSE;
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_SetSessionVlanMembership
 *--------------------------------------------------------------------------
 * PURPOSE  : Add port members of the completed session into the corrected
 *            remote vlan membership.
 * INPUT    : is_rspan_entry_valid -- The whole data structure of specific
 *            session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is set successfully.
 *            FALSE       -- The configuration isn't set successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_SetSessionVlanMembership(RSPAN_OM_SessionEntry_T is_rspan_entry_valid)
{
    I16_T i, j, uplink = 0;

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((is_rspan_entry_valid.uplink[i]&(0x1<<j)) != 0)
            //if ( ! RSPAN_OM_IS_MEMBER (is_rspan_entry_valid.uplink[i], j ) )
            {
                uplink = (i * 8) + (8 - j);

                if (is_rspan_entry_valid.switch_role != VAL_rspanSwitchRole_source)
                {
                    if ((VLAN_MGR_AddRspanEgressPortMember(is_rspan_entry_valid.remote_vid, uplink,
                                                             VAL_dot1qVlanStatus_permanent ) == FALSE)
                        || (VLAN_MGR_SetDot1qIngressFilter(uplink, VAL_dot1qPortIngressFiltering_true) == FALSE))
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    /* If dst port is already set , then add it into RSPAN vlan tagged member list. */
    if (is_rspan_entry_valid.switch_role == VAL_rspanSwitchRole_destination &&
        is_rspan_entry_valid.dst)
    {
        if (is_rspan_entry_valid.is_tagged == VAL_rspanDstPortTag_tagged)
        {
            if (VLAN_MGR_AddRspanEgressPortMember(is_rspan_entry_valid.remote_vid,
                is_rspan_entry_valid.dst, VAL_dot1qVlanStatus_permanent) == FALSE)
            {
                return FALSE;
            }
        }
        else if (is_rspan_entry_valid.is_tagged == VAL_rspanDstPortTag_untagged)
        {
            /* Add destination port in the untagged vlan member list */
            if (VLAN_MGR_AddRspanUntagPortMember(is_rspan_entry_valid.remote_vid,
                is_rspan_entry_valid.dst , VAL_dot1qVlanStatus_permanent) == FALSE)
            {
                return FALSE;
            }
        }
    }

    return TRUE; /* Here no need to check "not found" case because the session is completed. */
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_MGR_DeleteSessionVlanMembership
 *--------------------------------------------------------------------------
 * PURPOSE  : Del. port members of the completed session into the corrected
 *            remote vlan membership.
 * INPUT    : is_rspan_entry_valid -- The whole data structure of specific
 *            session id.
 * OUTPUT   : None
 * RETURN   : TRUE        -- The configuration is deleted successfully.
 *            FALSE       -- The configuration isn't deleted successfully.
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T RSPAN_MGR_DeleteSessionVlanMembership(RSPAN_OM_SessionEntry_T is_rspan_entry_valid)
{
    I16_T i, j , uplink = 0;

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i ++)
    {
        for (j = 7; j >= 0; j --)
        {
            if ((is_rspan_entry_valid.uplink[i]&(0x1<<j)) != 0)
            //if ( RSPAN_OM_IS_MEMBER (is_rspan_entry_valid.uplink[i], j ) != 0 )
            {
                uplink = (i * 8) + (8 - j);

                if (is_rspan_entry_valid.switch_role != VAL_rspanSwitchRole_source)
                {
                    if (VLAN_MGR_DeleteRspanEgressPortMember(is_rspan_entry_valid.remote_vid, uplink,
                                                                VAL_dot1qVlanStatus_permanent) == FALSE)
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    if (is_rspan_entry_valid.is_tagged == VAL_rspanDstPortTag_untagged)
    {
       if (VLAN_MGR_DeleteRspanUntagPortMember(is_rspan_entry_valid.remote_vid,
                                                 is_rspan_entry_valid.dst, VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }
    }

    /* For RSPAN VLAN, there is only destination ports will add into uptagged members, so when deleting dest. port
     * in rspan session, dest port need to be removed from untagged and egress port members
     */
    if (is_rspan_entry_valid.is_tagged == VAL_rspanDstPortTag_tagged ||
        is_rspan_entry_valid.is_tagged == VAL_rspanDstPortTag_untagged)
    {
       if (VLAN_MGR_DeleteRspanEgressPortMember(is_rspan_entry_valid.remote_vid,
                                                is_rspan_entry_valid.dst , VAL_dot1qVlanStatus_permanent) == FALSE)
        {
            return FALSE;
        }
    }

    return TRUE; /* Here no need to check "not found" case because the session is completed. */
}

#if(RSPAN_BACK_DOOR == TRUE)
/*-------------------------------------------------------------------------
 * ROUTINE NAME: RSPAN_MGR_BackdoorMain
 *-------------------------------------------------------------------------
 * PURPOSE: Backdoor of RSPAN.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 *-------------------------------------------------------------------------
 */
static void RSPAN_MGR_BackdoorMain(void)
{

    UI16_T  ch , k = 0 ;
    BOOL_T  eof=FALSE ;
    char    buff[10] = {0,} ;
    long    vid = 0 ; /* By Tien. 04/11/2007 */
    UI8_T   session_id = 0, attribute = 0, target_port = 0 ;
	I16_T	i = 0, j = 0 ;
    RSPAN_OM_SessionEntry_T rspan_entry ;
    char    buff_port[100] = {0,} ; //display port

    /*  BODY
     */
    while ( eof == FALSE )
    {
        BACKDOOR_MGR_Printf("\n 0. Exit\n");
        BACKDOOR_MGR_Printf(" 1. Create RSPAN VLAN.\n");
        BACKDOOR_MGR_Printf(" 2. Destroy RSPAN VLAN.\n");
        BACKDOOR_MGR_Printf(" 3. Set Remote VID and Uplink ports.\n");
        BACKDOOR_MGR_Printf(" 4. Delete Remote VID and Uplink ports.\n");
        BACKDOOR_MGR_Printf(" 5. Set source ports.\n");
        BACKDOOR_MGR_Printf(" 6. Delete source ports.\n");
        BACKDOOR_MGR_Printf(" 7. Set destination port.\n");
        BACKDOOR_MGR_Printf(" 8. Delete destination port.\n");
        BACKDOOR_MGR_Printf(" 9. Dump RSPAN OM Infor.\n");
        BACKDOOR_MGR_Printf(" a. Dump Local Port Monitor.\n");
        BACKDOOR_MGR_Printf(" b. Dump Vlan membership and RSPAN VLAN.\n");
        BACKDOOR_MGR_Printf("    select =");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("(%x)",ch);

        if (!((ch>=0x41&&ch<=0x5A)||(ch>=0x61&&ch<=0x7A)))
        {
        if ((ch < 0x30)||(ch>0x39))
            continue;
        ch -= 0x30;
        }

        BACKDOOR_MGR_Printf("%d",ch);
        switch (ch)
        {
            case 0 :
                eof = TRUE;
                break;
            case 1 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_CreateRspanVlan\nThe new RSPAN VLAN (vid: 1-4094) ??\n");

                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                vid=atol(buff);
  		        BACKDOOR_MGR_Printf("\n%lu\n", vid);

                BACKDOOR_MGR_Printf("RSPAN_MGR_CreateRspanVlan: result=[%d], vid=[%lu]", RSPAN_MGR_CreateRspanVlan(vid), vid) ;
                VLAN_MGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active );

                break;
            }
            case 2 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_DeleteRspanVlan\nThe deleted RSPAN VLAN (vid: 1-4094) ??\n");

                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                vid=atol(buff);
  		        BACKDOOR_MGR_Printf("\n%lu\n", vid);

                BACKDOOR_MGR_Printf("RSPAN_MGR_DeleteRspanVlan: result=[%d], vid=[%lu]", RSPAN_MGR_DeleteRspanVlan(vid), vid) ;
            	break;
            }
            case 3 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_SetSessionRemoteVlan\nSet RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nSet RSPAN Remote VLAN (switch role: 1-3:src-inter-dst) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                attribute=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", attribute);

                BACKDOOR_MGR_Printf("\nSet RSPAN Remote VLAN (vid: 1-4094) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                vid=atol(buff);
                BACKDOOR_MGR_Printf("\n%lu\n", vid);

                BACKDOOR_MGR_Printf("\nSet RSPAN Remote VLAN (uplink: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("RSPAN_MGR_SetSessionRemoteVlan: result=[%d], session=[%d]; attribute=[%d]; vid=[%lu]; target_port=[%d]\n"
                , RSPAN_MGR_SetSessionRemoteVlan(session_id, attribute, vid, target_port), session_id, attribute, vid, target_port) ;

                break;
            }
            case 4 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_DeleteSessionRemoteVlan\nDelete RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nDelete RSPAN Remote VLAN (switch role: 1-3:src-inter-dst) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                attribute=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", attribute);

                BACKDOOR_MGR_Printf("\nDelete RSPAN Remote VLAN (vid: 1-4094) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                vid=atol(buff);
                BACKDOOR_MGR_Printf("\n%lu\n", vid);

                BACKDOOR_MGR_Printf("\nDelete RSPAN Remote VLAN (uplink: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("RSPAN_MGR_DeleteSessionRemoteVlan: result=[%d], seesion=[%d]; attribute=[%d]; vid=[%lu]; target_port=[%d]\n"
                , RSPAN_MGR_DeleteSessionRemoteVlan(session_id, attribute, vid, target_port), session_id, attribute, vid, target_port) ;

                break;
            }
            case 5 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_SetSessionSourceInterface\nSet RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nSet RSPAN Source Port (source_port: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("\nSet RSPAN mode (mode: 1-3:rx-tx-both) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                attribute=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", attribute);

                BACKDOOR_MGR_Printf("RSPAN_MGR_SetSessionSourceInterface: result=[%d], seesion=[%d]; source_port=[%d]; mode=[%d]\n"
                , RSPAN_MGR_SetSessionSourceInterface(session_id, target_port, attribute), session_id, target_port, attribute) ;

                break;
            }
            case 6 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_DeleteSessionSourceInterface\nSet RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nDelete RSPAN Source Port (source_port: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("\nDelete RSPAN mode (mode: 1-3:rx-tx-both) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                attribute=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", attribute);

                BACKDOOR_MGR_Printf("RSPAN_MGR_DeleteSessionSourceInterface: result=[%d], seesion=[%d]; source_port=[%d]; mode=[%d]\n"
                , RSPAN_MGR_DeleteSessionSourceInterface(session_id, target_port, attribute), session_id, target_port, attribute) ;

                break;
            }
            case 7 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_SetSessionDestinationInterface\nSet RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nSet RSPAN dst Port (dst_port: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("\nSet RSPAN is_tagged (is_tagged: 1-2:untagged-tagged) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                attribute=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", attribute);

                BACKDOOR_MGR_Printf("RSPAN_MGR_SetSessionDestinationInterface: result=[%d], seesion=[%d]; dst=[%d]; tag=[%d]\n"
                , RSPAN_MGR_SetSessionDestinationInterface(session_id, target_port, attribute), session_id, target_port, attribute) ;

                break;
            }
            case 8 :
            {
                BACKDOOR_MGR_Printf("\nRSPAN_MGR_DeleteSessionDestinationInterface\nSet RSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                BACKDOOR_MGR_Printf("\nDelete RSPAN dst Port (dst_port: 1-26/50) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                target_port=atoi(buff);
                BACKDOOR_MGR_Printf("\n%d\n", target_port);

                BACKDOOR_MGR_Printf("RSPAN_MGR_DeleteSessionDestinationInterface: result=[%d], seesion=[%d]; dst=[%d]\n"
                , RSPAN_MGR_DeleteSessionDestinationInterface(session_id, target_port), session_id, target_port) ;
                break;
            }
            case 9 :
            {
                UI8_T session_cnt = 0 ;

                BACKDOOR_MGR_Printf("\nRSPAN_MGR_GetRspanSessionEntry\nRSPAN Session ID (session_id: 1-2) ??\n");
                BACKDOOR_MGR_RequestKeyIn(buff, sizeof(buff)-1);
                session_id=atoi(buff);
  		        BACKDOOR_MGR_Printf("\n%d\n", session_id);

                memset ( &rspan_entry, 0, sizeof ( RSPAN_OM_SessionEntry_T )) ;

                RSPAN_OM_GetSessionEntryCounter(&session_cnt);

                if (! RSPAN_MGR_GetRspanSessionEntry(session_id, &rspan_entry) )
                {
                    BACKDOOR_MGR_Printf("\n==============================================================\n");
                    BACKDOOR_MGR_Printf("\nThere are [%d] sessions existed in the system.\n", session_cnt );
                    BACKDOOR_MGR_Printf("\nRSPAN_MGR_GetRspanSessionEntry is failed.\n") ;
                    break ;
                }

                BACKDOOR_MGR_Printf("\n==============================================================\n");
                BACKDOOR_MGR_Printf("\nThere are [%d] sessions existed in the system.\n", session_cnt );
                BACKDOOR_MGR_Printf("\nSession ID: [%d]\n", rspan_entry.session_id );
                BACKDOOR_MGR_Printf("\nSwitch Role: [%d]\n", rspan_entry.switch_role );
                BACKDOOR_MGR_Printf("\nDst: [%d]\n", rspan_entry.dst );
                BACKDOOR_MGR_Printf("\nTag: [%d]\n", rspan_entry.is_tagged );
                BACKDOOR_MGR_Printf("\nRemote Vlan: [%lu]\n", rspan_entry.remote_vid );

                memset ( buff_port, 0, sizeof (buff_port)) ;

                /* Source Ports - TX */
                for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                {
                    for (j=7;j>=0;j--)
                    {
                        if (((I16_T)rspan_entry.src_tx[i] & (0x1<<j))!=0 )
                        {
                            memset ( buff, 0, sizeof (buff)) ;
                            sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                            strncat ( buff_port, buff, sizeof(buff) );
                        }
                    }
                }
                BACKDOOR_MGR_Printf("\nSource TX ports: [%s]\n", buff_port);

                memset ( buff_port, 0, sizeof (buff_port)) ;
                /* Source Ports - RX */
                for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                {
                    for (j=7;j>=0;j--)
                    {
                        if (((I16_T)rspan_entry.src_rx[i] & (0x1<<j))!=0 )
                        {
                            memset ( buff, 0, sizeof (buff)) ;
                            sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                            strncat ( buff_port, buff, sizeof(buff) );
                        }
                    }
                }
                BACKDOOR_MGR_Printf("\nSource RX ports: [%s]\n", buff_port);

                memset ( buff_port, 0, sizeof (buff_port)) ;
                /* Uplink Ports */
                for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                {
                    for (j=7;j>=0;j--)
                    {
                        if (((I16_T)rspan_entry.uplink[i] & (0x1<<j))!=0 )
                        {
                            memset ( buff, 0, sizeof (buff)) ;
                            sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                            strncat ( buff_port, buff, sizeof(buff_port) );
                        }
                    }
                }
                BACKDOOR_MGR_Printf("\nUplink ports: [%s]\n", buff_port);
                BACKDOOR_MGR_Printf("\n==============================================================\n");

                break;
            }
   	        case 'a' :
    	    case 'A' :
        	{
        	    memset ( &rspan_entry, 0, sizeof ( RSPAN_OM_SessionEntry_T )) ;

                if (! RSPAN_OM_GetLocalSessionEntryForBackdoor(&rspan_entry) )
                {
                    BACKDOOR_MGR_Printf("\nRSPAN_OM_GetLocalSessionEntryForBackdoor is failed.\n") ;
        		    break;
                }
                BACKDOOR_MGR_Printf("\n==============================================================\n");
                BACKDOOR_MGR_Printf("\nDst: [%d]\n", rspan_entry.dst );

                memset ( buff_port, 0, sizeof (buff_port)) ;
                /* Source Ports - TX */
                for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                {
                    for (j=7;j>=0;j--)
                    {
                        if (((I16_T)rspan_entry.src_tx[i] & (0x1<<j))!=0 )
                        {
                            memset ( buff, 0, sizeof (buff)) ;
                            sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                            strncat ( buff_port, buff, sizeof(buff) );
                        }
                    }
                }
                BACKDOOR_MGR_Printf("\nSource TX ports: [%s]\n", buff_port);

                memset ( buff_port, 0, sizeof (buff_port)) ;
                /* Source Ports - RX */
                for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                {
                    for (j=7;j>=0;j--)
                    {
                        if (((I16_T)rspan_entry.src_rx[i] & (0x1<<j))!=0 )
                        {
                            memset ( buff, 0, sizeof (buff)) ;
                            sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                            strncat ( buff_port, buff, sizeof(buff) );
                        }
                    }
                }
                BACKDOOR_MGR_Printf("\nSource RX ports: [%s]\n", buff_port);

                break;
            }
    	    case 'b' :
    	    case 'B' :
        	{
        	    VLAN_MGR_Dot1qVlanStaticEntry_T vlan_entry ;

        	    for (k=1; k<=SYS_ADPT_MAX_VLAN_ID; k++)
        	    {
        	        if ( RSPAN_OM_IsRspanVlan(k) )
                    {
                        memset ( &vlan_entry, 0, sizeof (vlan_entry)) ;

                	    VLAN_OM_GetDot1qVlanStaticEntry( k, &vlan_entry);

                        BACKDOOR_MGR_Printf("\n==============================================================\n");
                	    BACKDOOR_MGR_Printf("\nVLAN ID: [%d]\n", k ) ;
                	    BACKDOOR_MGR_Printf("\nVLAN Static Name: [%s]\n",vlan_entry.dot1q_vlan_static_name);
                        BACKDOOR_MGR_Printf("\nVLAN Static Status: [%lu]\n",vlan_entry.dot1q_vlan_static_row_status);

                        memset ( buff_port, 0, sizeof (buff_port)) ;

                        /* static egress ports */
                        for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                        {
                            for (j=7;j>=0;j--)
                            {
                                if ((vlan_entry.dot1q_vlan_static_egress_ports[i] & (0x1<<j))!=0 )
                                {
                                    memset ( buff, 0, sizeof (buff)) ;
                                    sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                                    strncat ( buff_port, buff, sizeof(buff) );
                                }
                            }
                        }
                        BACKDOOR_MGR_Printf("\nEgress ports: [%s]\n", buff_port);

                        /* static untagged ports */
                        memset ( buff_port, 0, sizeof (buff_port)) ;

                        /* static_untagged ports */
                        for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                        {
                            for (j=7;j>=0;j--)
                            {
                                if ((vlan_entry.dot1q_vlan_static_untagged_ports[i] & (0x1<<j))!=0 )
                                {
                                    memset ( buff, 0, sizeof (buff)) ;
                                    sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                                    strncat ( buff_port, buff, sizeof(buff) );
                                }
                            }
                        }

                        BACKDOOR_MGR_Printf("\nUntagged ports: [%s]\n", buff_port);

                        /* static forbidden ports */
                        memset ( buff_port, 0, sizeof (buff_port)) ;

                        /* static forbidden egress ports */
                        for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
                        {
                            for (j=7;j>=0;j--)
                            {
                                if ((vlan_entry.dot1q_vlan_forbidden_egress_ports[i] & (0x1<<j))!=0 )
                                {
                                    memset ( buff, 0, sizeof (buff)) ;
                                    sprintf ( buff, "%d,", (i*8)+(8-j)) ;
                                    strncat ( buff_port, buff, sizeof(buff) );
                                }
                            }
                        }

                        BACKDOOR_MGR_Printf("\nForbidden ports: [%s]\n", buff_port);
                    }
        	    }

                break;
            }
           default :
                break;
        }   /*  end of switch    */
    }   /*  end of while    */
}   /*  RSPAN_MGR_BackdoorMain    */

#endif /*#if(RSPAN_BACK_DOOR == TRUE)*/

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
BOOL_T RSPAN_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    RSPAN_MGR_IPCMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (RSPAN_MGR_IPCMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is not master mode
     */
    if (RSPAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.ret_bool = FALSE;
        msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding RSPAN_MGR function
     */
    switch (msg_p->type.cmd)
    {
        case RSPAN_MGR_IPC_ISRSPANUPLINKPORT:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_IsRspanUplinkPortUsed(msg_p->data.ui32_v);
            break;

        case RSPAN_MGR_IPC_ISRSPANVLAN:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_IsRspanVlan(msg_p->data.ui32_v);
            break;

        case RSPAN_MGR_IPC_ISRSPANMIRRORTOPORT:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_IsRspanMirrorToPort(msg_p->data.ui32_v);
            break;

        case RSPAN_MGR_IPC_GETRSPANSESSIONENTRY:
            msgbuf_p->msg_size = (sizeof(((RSPAN_MGR_IPCMsg_T *)0)->data.rspan_sessionentry)
							     + RSPAN_MGR_MSGBUF_TYPE_SIZE);
        	msg_p->type.ret_bool =
                RSPAN_MGR_GetRspanSessionEntry(
                    msg_p->data.rspan_sessionentry.session_id,
            		&msg_p->data.rspan_sessionentry.rspan_entry);
            break;

        case RSPAN_MGR_IPC_GETNEXTRSPANSESSIONENTRY:
            msgbuf_p->msg_size = (sizeof(((RSPAN_MGR_IPCMsg_T *)0)->data.rspan_sessionentry)
                                 + RSPAN_MGR_MSGBUF_TYPE_SIZE);
        	msg_p->type.ret_bool =
                RSPAN_MGR_GetNextRspanSessionEntry(
                    &msg_p->data.rspan_sessionentry.session_id,
            		&msg_p->data.rspan_sessionentry.rspan_entry);
            break;

        case RSPAN_MGR_IPC_ISSESSIONENTRYCOMPLETED:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
        	    RSPAN_MGR_IsSessionEntryCompleted(msg_p->data.ui8_v);
            break;

        case RSPAN_MGR_IPC_SETSESSIONSOURCEINTERFACE:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
        	    RSPAN_MGR_SetSessionSourceInterface(
                    msg_p->data.u8a1_u8a2_u8a3.u8_a1,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a2,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a3);
            break;

        case RSPAN_MGR_IPC_DELETESESSIONSOURCEINTERFACE:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_DeleteSessionSourceInterface(msg_p->data.u8a1_u8a2_u8a3.u8_a1,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a2,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a3);
            break;

        case RSPAN_MGR_IPC_SETSESSIONDESTINATIONINTERFACE:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_SetSessionDestinationInterface(
                    msg_p->data.u8a1_u8a2_u8a3.u8_a1,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a2,
                    msg_p->data.u8a1_u8a2_u8a3.u8_a3);
            break;

        case RSPAN_MGR_IPC_DELETESESSIONDESTINATIONINTERFACE:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
        	    RSPAN_MGR_DeleteSessionDestinationInterface(
        	        msg_p->data.u8a1_u8a2.u8_a1,
        	        msg_p->data.u8a1_u8a2.u8_a2);
            break;

        case RSPAN_MGR_IPC_SETSESSIONREMOTEVLAN:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
        	    RSPAN_MGR_SetSessionRemoteVlan(
            	    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a1,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a2,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u32_a3,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a4);
            break;

        case RSPAN_MGR_IPC_DELETESESSIONREMOTEVLAN:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
        	    RSPAN_MGR_DeleteSessionRemoteVlan(
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a1,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a2,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u32_a3,
                    msg_p->data.u8a1_u8a2_u32a3_u8a4.u8_a4);
            break;

        case RSPAN_MGR_IPC_SETSESSIONREMOTEVLANDST:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
            msg_p->type.ret_bool =
                RSPAN_MGR_SetSessionRemoteVlanDst(
                    msg_p->data.u8a1_u8a2.u8_a1,
                    msg_p->data.u8a1_u8a2.u8_a2);
            break;

        case RSPAN_MGR_IPC_REMOVECLIWEBSESSIONID:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_RemoveCliWebSessionId(msg_p->data.ui8_v);
            break;

        case RSPAN_MGR_IPC_CREATERSPANVLAN:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_CreateRspanVlan(msg_p->data.ui32_v);
            break;

        case RSPAN_MGR_IPC_DELETERSPANVLAN:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_DeleteRspanVlan(msg_p->data.ui32_v);
            break;

        case RSPAN_MGR_IPC_SETRSPANVLANSTATUS:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_SetRspanVlanStatus(
                    msg_p->data.u32a1_u32a2.u32_a1,
                    msg_p->data.u32a1_u32a2.u32_a2);
            break;

        case RSPAN_MGR_IPC_GETRSPANVLANENTRY:
            msgbuf_p->msg_size = (sizeof(((RSPAN_MGR_IPCMsg_T *)0)->data.rspan_vlanentry)
							     + RSPAN_MGR_MSGBUF_TYPE_SIZE);
            if (msg_p->data.rspan_vlanentry.get_next)
            	msg_p->type.ret_bool =
                    RSPAN_MGR_GetNextRspanVlanEntry(&msg_p->data.rspan_vlanentry.entry);
            else
            	msg_p->type.ret_bool =
                    RSPAN_MGR_GetRspanVlanEntry(&msg_p->data.rspan_vlanentry.entry);
            break;

        case RSPAN_MGR_IPC_SETRSPANENTRYSTATUS:
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
        	msg_p->type.ret_bool =
                RSPAN_MGR_SetRspanEntryStatus(
                    &msg_p->data.rspan_sessionentryfield.entry,
                    msg_p->data.rspan_sessionentryfield.field_id);
            break;

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msgbuf_p->msg_size = RSPAN_MGR_MSGBUF_TYPE_SIZE;
            msg_p->type.ret_bool = FALSE;
    }

    return TRUE;
}

