/* FUNCTION NAME: add_pmgr.c
 * PURPOSE:
 *	1. Manage the ADD CSC
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date:     2009/02/13
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ADD == TRUE)

#include "sys_bld.h"
#include "sysfun.h"
#include "sys_module.h"
#include "add_type.h"
#include "add_mgr.h"

#define PMGR_IMPL_BEGIN(PARAM_TYPE) \
    enum {PARAM_SIZE = ADD_MGR_GET_MSGBUFSIZE(PARAM_TYPE)};         \
    UI8_T           msgbuf[SYSFUN_SIZE_OF_MSG( PARAM_SIZE )]={0};   \
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msgbuf;                \
    PARAM_TYPE      *param_p = ADD_MGR_MSG_DATA(msg_p);             \

#define PMGR_IMPL_SEND_MSG(IPC_MSG_CODE, RET_TYPE, failed_ret_value)\
    ADD_PMGR_SendMsg(                                               \
        IPC_MSG_CODE,                                               \
        msg_p,                                                      \
        PARAM_SIZE,                                                 \
        PARAM_SIZE,                                                 \
        (UI32_T)failed_ret_value                                    \
    );                                                              \
    if (failed_ret_value == (RET_TYPE)ADD_MGR_MSG_RETVAL(msg_p))    \
    { return failed_ret_value; }                                    \

#define PMGR_IMPL_END(RET_TYPE)                                     \
    return (RET_TYPE)ADD_MGR_MSG_RETVAL(msg_p);


static SYSFUN_MsgQ_T ipcmsgq_handle;
static void ADD_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_Initiate_System_Resources
 * ---------------------------------------------------------------------
 * PURPOSE: Init system resource.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_InitiateProcessResources()
{

    SYSFUN_Debug_Printf("%s\n", __FUNCTION__);

    if(SYSFUN_GetMsgQ(SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanEnabledId
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable the voice VLAN feature.
 * INPUT:   vid               -- the voice VLAN ID.
 *                               VAL_voiceVlanEnabledId_disabled:
 *                                 disable voice VLAN.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. The Voice VLAN ID shall be created and active first.
 *          2. The Voice VLAN ID shall not be modified when the voice
 *             VLAN feature is enabled.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanEnabledId(I32_T vid)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanEnabledIdParam_T);
    param_p->vid = vid;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_ENABLED_ID, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get voice VLAN ID.
 * INPUT:   None.
 * OUTPUT:  vid               -- the voice VLAN ID.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanId(I32_T *vid)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanIdParam_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_ID, BOOL_T, FALSE);
    *vid = param_p->vid;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVLANId
 * ---------------------------------------------------------------------
 * PURPOSE: Get RUNNING voice VLAN ID.
 * INPUT:   None.
 * OUTPUT:  vid               -- the voice VLAN ID.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanId(I32_T *vid_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanIdParam_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_ID, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *vid_p = param_p->vid;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_IsVoiceVlanEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN state.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE               - Enabled
 *          FALSE              - Disabled
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_IsVoiceVlanEnabled()
{
    PMGR_IMPL_BEGIN(ADD_MGR_IPCMsg_Type_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_IS_VOICE_VLAN_ENABLED, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanPortModeParam_T);
    param_p->lport = lport;
    param_p->mode = mode;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_MODE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortModeParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_MODE, BOOL_T, FALSE);
    *mode_p = param_p->mode;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanPortMode(UI32_T lport, UI32_T *mode_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanPortModeParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_MODE, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *mode_p = param_p->mode;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port security state on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanPortSecurityStateParam_T);
    param_p->lport = lport;
    param_p->state = state;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_SECURITY_STATE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port security state on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortSecurityStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_SECURITY_STATE, BOOL_T, FALSE);
    *state_p = param_p->state;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port security state the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanPortSecurityStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_SECURITY_STATE, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *state_p = param_p->state;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port OUI rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T state)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T);
    param_p->lport = lport;
    param_p->state = state;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_OUI_RULE_STATE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port OUI rule stateon the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_OUI_RULE_STATE, BOOL_T, FALSE);
    *state_p = param_p->state;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port OUI rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_OUI_RULE_STATE, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *state_p = param_p->state;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T state)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T);
    param_p->lport = lport;
    param_p->state = state;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_LLDP_RULE_STATE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_LLDP_RULE_STATE, BOOL_T, FALSE);
    *state_p = param_p->state;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_LLDP_RULE_STATE, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *state_p = param_p->state;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VALN port priority on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanPortPriorityParam_T);
    param_p->lport = lport;
    param_p->priority = priority;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_PRIORITY, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port priority on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortPriorityParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_PRIORITY, BOOL_T, FALSE);
    *priority_p = param_p->priority;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port priority on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanPortPriority(UI32_T lport, UI8_T *priority_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_PRIORITY, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *priority_p = param_p->priority;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_AddOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Add a recognised OUI entry.
 * INPUT:   entry             -- the OUI entry.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. Don't allow the multicast address for the OUI address.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_AddOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    PMGR_IMPL_BEGIN(ADD_MGR_AddOuiEntryParam_T);
    param_p->entry = *entry;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_ADD_OUI_ENTRY, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function gets OUI entry based on oui's MAC-address.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetOuiEntryParam_T);
    param_p->entry = *entry;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_OUI_ENTRY, BOOL_T, FALSE);
    *entry = param_p->entry;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetNextOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next OUI entry.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetNextOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetNextOuiEntryParam_T);
    param_p->entry = *entry;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_NEXT_OUI_ENTRY, BOOL_T, FALSE);
    *entry = param_p->entry;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetNextRunningOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Get next RUNNING OUI entry.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetNextRunningOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetNextRunningOuiEntryParam_T);
    param_p->entry = *entry;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI_ENTRY, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *entry = param_p->entry;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetNextRunningOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next RUNNING OUI entry.
 * INPUT:   None.
 * OUTPUT:  oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 *          description       -- the descritption of the OUI MAC address,
 *                               the max length is 30 character.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetNextRunningOui(UI8_T *oui, UI8_T *mask, char *description)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetNextRunningOuiParam_T);
    memcpy(param_p->oui, oui, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(param_p->mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    strncpy(param_p->description, description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI_ENTRY, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    memcpy(oui, param_p->oui, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(mask, param_p->mask, SYS_ADPT_MAC_ADDR_LEN);
    strncpy(description, param_p->description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_RemoveOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Remove a recognised OUI entry.
 * INPUT:   oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_RemoveOuiEntry(UI8_T *oui, UI8_T *mask)
{
    PMGR_IMPL_BEGIN(ADD_MGR_RemoveOuiEntryParam_T);
    memcpy(param_p->oui, oui, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(param_p->mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_REMOVE_OUI_ENTRY, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetOuiState
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets OUI entry status based on OUI's
 *          MAC address. If the OUI is not in the database, this function
 *          will create a new oui entry if the oui's capacity is not
 *          exceed the limit.
 * INPUT:   oui               -- the OUI MAC address (key).
 *          status            -- the status of OUI entry;
 *                               VAL_voiceVlanOuiStatus_valid,
 *                               VAL_voiceVlanOuiStatus_invalid.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetOuiState(UI8_T *oui, UI32_T state)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetOuiStateParam_T);
    memcpy(param_p->oui, oui, SYS_ADPT_MAC_ADDR_LEN);
    param_p->state = state;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_OUI_STATE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetOuiMaskAddress
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets mask-address of the sepcified OUI. If
 *          the OUI is not in the database, this function will create a
 *          new OUI entry if the OUI's capacity is not exceed the limit.
 * INPUT:   oui                - the OUI MAC address (key).
 *          mask               - the mask of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetOuiMaskAddress(UI8_T *oui, UI8_T *mask)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetOuiMaskAddressParam_T);
    memcpy(param_p->oui, oui, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(param_p->mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_OUI_MASK_ADDRESS, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetOuiDescription
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets description of the sepcified OUI. If
 *          the OUI is not in the database, this function will create a
 *          new OUI entry if the oui's capacity is not exceed the limit.
 * INPUT:   oui                - the OUI MAC address (key).
 *          description        - the description of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetOuiDescription(UI8_T *oui, char *description)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetOuiDescriptionParam_T);
    memcpy(param_p->oui, oui, SYS_ADPT_MAC_ADDR_LEN);
    strncpy(param_p->description, description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_OUI_DESCRIPTION, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   timeout           -- the aging time, minute unit.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanAgingTime(UI32_T timeout)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanAgingTimeParam_T);
    param_p->timeout = timeout;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  timeout           -- the aging time, minute unit.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanAgingTime(UI32_T *timeout_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanAgingTimeParam_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME, BOOL_T, FALSE);
    *timeout_p = param_p->timeout;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetRunningVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  timeout           -- the aging time, minute unit.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_PMGR_GetRunningVoiceVlanAgingTime(UI32_T *timeout_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_AGING_TIME, SYS_TYPE_Get_Running_Cfg_T, SYS_TYPE_GET_RUNNING_CFG_FAIL);
    *timeout_p = param_p->timeout;
    PMGR_IMPL_END(SYS_TYPE_Get_Running_Cfg_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   day/hour/minute.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetVoiceVlanAgingTimeByDayHourMinute(UI32_T day, UI32_T hour, UI32_T minute)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T);
    param_p->day = day;
    param_p->hour = hour;
    param_p->minute = minute;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanAgingTimeByDayHourMinute
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  day/hour/minute.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanAgingTimeByDayHourMinute(UI32_T *day_p, UI32_T *hour_p, UI32_T *minute_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T);
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE, BOOL_T, FALSE);
    *day_p = param_p->day;
    *hour_p = param_p->hour;
    *minute_p = param_p->minute;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_GetVoiceVlanPortRemainAge
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port remaining age on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  remain_age        -- Remaining age (mins).
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   0 means less than 1 minute.
 *
 *          ADD_TYPE_VOICE_VLAN_ERROR_NA means
 *          global voic vlan is disable or mode is disable or mode is manual
 *          or no auto join vlan now.
 *
 *          ADD_TYPE_VOICE_VLAN_ERROR_NO_START means
 *          auto join vlan now and there have phone attached
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_GetVoiceVlanPortRemainAge(UI32_T lport, I32_T *age_p)
{
    PMGR_IMPL_BEGIN(ADD_MGR_GetVoiceVlanPortRemainAgeParam_T);
    param_p->lport = lport;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_REMAIN_AGE, BOOL_T, FALSE);
    *age_p = param_p->remain_age;
    PMGR_IMPL_END(BOOL_T);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_PMGR_SetDebugPrintStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the debug print status.
 * INPUT:   state             -- the state of the debug print for the MGR;
 *                               TRUE:  enable the debug print.
 *                               FALSE: disable the debug print.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_PMGR_SetDebugPrintStatus(BOOL_T state)
{
    PMGR_IMPL_BEGIN(ADD_MGR_SetDebugPrintStatusParam_T);
    param_p->state = state;
    PMGR_IMPL_SEND_MSG(ADD_MGR_IPC_CMD_SET_DEBUG_PRINT_STATUS, BOOL_T, FALSE);
    PMGR_IMPL_END(BOOL_T);
}

#ifdef ADD_PMGR_DO_UNIT_TEST
#define SYSFUN_SendRequestMsg(handle, msg, w, x, y, z) ADD_PMGR_HandleIPCReqMsg(msg)
#define SYSFUN_GET_CSC_OPERATING_MODE() SYS_TYPE_STACKING_MASTER_MODE

struct
{
    I32_T vid;
    UI32_T mode;
    UI32_T state;
    UI8_T  priority;
    ADD_MGR_VoiceVlanOui_T entry;
    UI32_T timeout;
    UI32_T day;
    UI32_T hour;
    UI32_T minute;
}om_cfg;

/*------------------------------------------------------------------------------
 * ROUTINE NAME : ADD_PMGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T ADD_PMGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
    {
        return FALSE;
    }

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        ADD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    cmd = ADD_MGR_MSG_CMD(ipcmsg_p);
    switch(cmd)
    {
        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_ENABLED_ID:
        {
            ADD_MGR_SetVoiceVlanEnabledIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanEnabledId(param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanEnabledIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_ID:
        {
            ADD_MGR_GetVoiceVlanIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanId(&param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_ID:
        {
            ADD_MGR_GetRunningVoiceVlanIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanId(&param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_IS_VOICE_VLAN_ENABLED:
        {
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_IsVoiceVlanEnabled();
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_SetVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortMode(param_p->lport, param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_GetVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortMode(param_p->lport, &param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_GetRunningVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortMode(param_p->lport, &param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_SetVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortSecurityState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_GetVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortSecurityState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortSecurityState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortOuiRuleState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortOuiRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortLldpRuleState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortLldpRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_SetVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortPriority(param_p->lport, param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_GetVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortPriority(param_p->lport, &param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortPriority(param_p->lport, &param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_ADD_OUI_ENTRY:
        {
            ADD_MGR_AddOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_AddOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_AddOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_OUI_ENTRY:
        {
            ADD_MGR_GetOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_OUI_ENTRY:
        {
            ADD_MGR_GetNextOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI_ENTRY:
        {
            ADD_MGR_GetNextRunningOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextRunningOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextRunningOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI:
        {
            ADD_MGR_GetNextRunningOuiParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextRunningOui(param_p->oui, param_p->mask, param_p->description);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_REMOVE_OUI_ENTRY:
        {
            ADD_MGR_RemoveOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_RemoveOuiEntry(param_p->oui, param_p->mask);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_RemoveOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_STATE:
        {
            ADD_MGR_SetOuiStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiState(param_p->oui, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_MASK_ADDRESS:
        {
            ADD_MGR_SetOuiMaskAddressParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiMaskAddress(param_p->oui, param_p->mask);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiMaskAddressParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_DESCRIPTION:
        {
            ADD_MGR_SetOuiDescriptionParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiDescription(param_p->oui, param_p->description);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiDescriptionParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_SetVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanAgingTime(param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_GetVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanAgingTime(&param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanAgingTime(&param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE:
        {
            ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(param_p->day, param_p->hour, param_p->minute);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE:
        {
            ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(&param_p->day, &param_p->hour, &param_p->minute);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_DEBUG_PRINT_STATUS:
        {
            ADD_MGR_SetDebugPrintStatusParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_SetDebugPrintStatus(param_p->state);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetDebugPrintStatusParam_T);
        }
        break;

        defaule:
        {
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            return TRUE;
        }
        break;
    }

    return TRUE;
}

BOOL_T ADD_MGR_SetVoiceVlanEnabledId(I32_T vid)
{
    if (vid == 100)
        return FALSE;

    om_cfg.vid = vid;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanId(I32_T *vid)
{
    *vid = om_cfg.vid;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanId(I32_T *vid)
{
    *vid = om_cfg.vid;

    if (*vid == 3)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_IsVoiceVlanEnabled()
{
    return (om_cfg.vid != 0) ? TRUE : FALSE;
}

BOOL_T ADD_MGR_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode)
{
    if (mode > 3)
    {
        return FALSE;
    }

    om_cfg.mode = mode;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode)
{
    *mode = om_cfg.mode;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortMode(UI32_T lport, UI32_T *mode)
{
    *mode = om_cfg.mode;

    if (*mode == 1)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state)
{
    if (state > 2)
    {
        return FALSE;
    }

    om_cfg.state = state;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    if (*state == 1)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_SetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T state)
{
    if (state > 2)
    {
        return FALSE;
    }

    om_cfg.state = state;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    if (*state == 1)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_SetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T state)
{
    if (state > 2)
    {
        return FALSE;
    }

    om_cfg.state = state;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state)
{
    *state = om_cfg.state;
    if (*state == 1)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority)
{
    if (priority > 7)
    {
        return FALSE;
    }

    om_cfg.priority = priority;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority)
{
    *priority = om_cfg.priority;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortPriority(UI32_T lport, UI8_T *priority)
{
    *priority = om_cfg.priority;

    if (*priority == 1)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_AddOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    om_cfg.entry = *entry;
    return TRUE;
}

BOOL_T ADD_MGR_GetOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if (memcmp(om_cfg.entry.oui, entry->oui, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        return FALSE;
    }

    *entry = om_cfg.entry;
    return TRUE;
}

BOOL_T ADD_MGR_GetNextOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if (entry->oui[0] == 0 && entry->oui[1] == 0 && entry->oui[2] == 0 && entry->oui[3] == 0 && entry->oui[4] == 0 && entry->oui[5] == 0)
    {
        *entry = om_cfg.entry;
        return TRUE;
    }

    return FALSE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if (entry->oui[0] == 0 && entry->oui[1] == 0 && entry->oui[2] == 0 && entry->oui[3] == 0 && entry->oui[4] == 0 && entry->oui[5] == 0)
    {
        *entry = om_cfg.entry;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOui(UI8_T *oui, UI8_T *mask, char *description)
{
    if (oui[0] == 0 && oui[1] == 0 && oui[2] == 0 && oui[3] == 0 && oui[4] == 0 && oui[5] == 0)
    {
        memcpy(mask, om_cfg.entry.mask, SYS_ADPT_MAC_ADDR_LEN);
        strncpy(description, om_cfg.entry.description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
        description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN] = '\0';
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

BOOL_T ADD_MGR_RemoveOuiEntry(UI8_T *oui, UI8_T *mask)
{
    if (memcmp(om_cfg.entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        return FALSE;
    }

    memset(&om_cfg.entry, 0, sizeof(om_cfg.entry));
    return TRUE;
}

BOOL_T ADD_MGR_SetOuiState(UI8_T *oui, UI32_T state)
{
    if (memcmp(om_cfg.entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        return FALSE;
    }

    /* remove
     */
    if (state == 2)
    {
        memset(&om_cfg.entry, 0, sizeof(om_cfg.entry));
    }

    return TRUE;
}

BOOL_T ADD_MGR_SetOuiMaskAddress(UI8_T *oui, UI8_T *mask)
{
    if (memcmp(om_cfg.entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        return FALSE;
    }

    memcpy(om_cfg.entry.mask, mask, SYS_ADPT_MAC_ADDR_LEN);
    return TRUE;
}

BOOL_T ADD_MGR_SetOuiDescription(UI8_T *oui, char *description)
{
    if (memcmp(om_cfg.entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        return FALSE;
    }

    strncpy(om_cfg.entry.description, description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
    om_cfg.entry.description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN] = '\0';
    return TRUE;
}

BOOL_T ADD_MGR_SetVoiceVlanAgingTime(UI32_T timeout)
{
    if (100 < timeout)
    {
        return FALSE;
    }

    om_cfg.timeout = timeout;
    return TRUE;
}

BOOL_T ADD_MGR_GetVoiceVlanAgingTime(UI32_T *timeout)
{
    *timeout = om_cfg.timeout;
    return TRUE;
}

SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanAgingTime(UI32_T *timeout)
{
    *timeout = om_cfg.timeout;

    if (*timeout == 5)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

BOOL_T ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(UI32_T day, UI32_T hour, UI32_T minute)
{
    om_cfg.day = day;
    om_cfg.hour = hour;
    om_cfg.minute = minute;
}

BOOL_T ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(UI32_T *day, UI32_T *hour, UI32_T *minute)
{
    *day = om_cfg.day;
    *hour = om_cfg.hour;
    *minute = om_cfg.minute;
}

void ADD_MGR_SetDebugPrintStatus(BOOL_T state)
{
    printf("%s : debug %s\n", __FUNCTION__, (state == TRUE) ? "on" : "off");
}

#endif /* ADD_PMGR_DO_UNIT_TEST */

static void ADD_PMGR_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val)
{
    UI32_T ret;

    msg_p->cmd = SYS_MODULE_ADD;
    msg_p->msg_size = req_size;

    ADD_MGR_MSG_CMD(msg_p) = cmd;

    ret = SYSFUN_SendRequestMsg(ipcmsgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_WAIT_FOREVER,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                res_size,
                                msg_p);

    /* If IPC is failed, set return value as ret_val.
     */
    if ((ret != SYSFUN_OK) || (ADD_MGR_MSG_RETVAL(msg_p) == FALSE))
        ADD_MGR_MSG_RETVAL(msg_p) = ret_val;
}



#ifdef ADD_PMGR_DO_UNIT_TEST

#define ASSERT_TRUE(x) \
    if (!x) { printf("Failed on line %d\n", __LINE__); }else{printf(".");}

#define ASSERT_FALSE(x) \
    if (x) { printf("Failed on line %d\n", __LINE__); }else{printf(".");}

void test_set_voice_vlan_id()
{
    enum {MAX_STR_LEN = 50};

    {
        I32_T vid;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanEnabledId(1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanEnabledId(1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanId(&vid));
        sprintf(pmgr_result, "get vid=%d", vid);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanId(&vid));
        sprintf(mgr_result, "get vid=%d", vid);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        I32_T vid;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanEnabledId(2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanEnabledId(2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanId(&vid) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get vid=%d", vid);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanId(&vid) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get vid=%d", vid);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanEnabledId(3));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanEnabledId(3));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanId(&vid) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get vid=%d", vid);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanId(&vid) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get vid=%d", vid);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanEnabledId(100));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanEnabledId(100));
    }
}

void test_is_voice_vlan_enabled()
{
    {
        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanEnabledId(0));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanEnabledId(0));

        ASSERT_TRUE(ADD_PMGR_IsVoiceVlanEnabled() == FALSE);
        ASSERT_TRUE(ADD_MGR_IsVoiceVlanEnabled() == FALSE);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanEnabledId(1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanEnabledId(1));

        ASSERT_TRUE(ADD_PMGR_IsVoiceVlanEnabled() == TRUE);
        ASSERT_TRUE(ADD_MGR_IsVoiceVlanEnabled() == TRUE);
    }
}

void test_set_voice_vlan_port_mode()
{
    enum {MAX_STR_LEN = 50};

    {
        UI32_T mode;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortMode(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortMode(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanPortMode(1, &mode));
        sprintf(pmgr_result, "get mode=%d", mode);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanPortMode(1, &mode));
        sprintf(mgr_result, "get mode=%d", mode);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI32_T mode;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortMode(1, 2));
        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortMode(1, 2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortMode(1, &mode) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get mode=%d", mode);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortMode(1, &mode) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get mode=%d", mode);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortMode(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortMode(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortMode(1, &mode) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get mode=%d", mode);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortMode(1, &mode) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get mode=%d", mode);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanPortMode(1, 4));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanPortMode(1, 4));
    }
}

void test_set_voice_vlan_port_security_state()
{
    enum {MAX_STR_LEN = 50};

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortSecurityState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortSecurityState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanPortSecurityState(1, &state));
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanPortSecurityState(1, &state));
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortSecurityState(1, 2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortSecurityState(1, 2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortSecurityState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortSecurityState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortSecurityState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortSecurityState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortSecurityState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortSecurityState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanPortSecurityState(1, 3));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanPortSecurityState(1, 3));
    }
}

void test_set_voice_vlan_port_oui_rule_state()
{
    enum {MAX_STR_LEN = 50};

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortOuiRuleState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortOuiRuleState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanPortOuiRuleState(1, &state));
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanPortOuiRuleState(1, &state));
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortOuiRuleState(1, 2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortOuiRuleState(1, 2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortOuiRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortOuiRuleState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortOuiRuleState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortOuiRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanPortOuiRuleState(1, 3));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanPortOuiRuleState(1, 3));
    }
}

void test_set_voice_vlan_port_lldp_rule_state()
{
    enum {MAX_STR_LEN = 50};

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortLldpRuleState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortLldpRuleState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanPortLldpRuleState(1, &state));
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanPortLldpRuleState(1, &state));
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI32_T state;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortLldpRuleState(1, 2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortLldpRuleState(1, 2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortLldpRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortLldpRuleState(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortLldpRuleState(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortLldpRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get state=%d", state);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(1, &state) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get state=%d", state);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanPortLldpRuleState(1, 3));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanPortLldpRuleState(1, 3));
    }
}

void test_set_voice_vlan_port_priority()
{
    enum {MAX_STR_LEN = 50};

    {
        UI8_T priority;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortPriority(1, 5));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortPriority(1, 5));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanPortPriority(1, &priority));
        sprintf(pmgr_result, "get priority=%d", priority);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanPortPriority(1, &priority));
        sprintf(mgr_result, "get priority=%d", priority);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI8_T priority;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortPriority(1, 2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortPriority(1, 2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortPriority(1, &priority) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get priority=%d", priority);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortPriority(1, &priority) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get priority=%d", priority);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanPortPriority(1, 1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanPortPriority(1, 1));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanPortPriority(1, &priority) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get priority=%d", priority);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanPortPriority(1, &priority) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get priority=%d", priority);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanPortPriority(1, 8));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanPortPriority(1, 8));
    }
}

BOOL_T IsIdentify(ADD_MGR_VoiceVlanOui_T *e1, ADD_MGR_VoiceVlanOui_T *e2)
{
    return (memcmp(e1->oui, e2->oui, SYS_ADPT_MAC_ADDR_LEN) == 0 && memcmp(e1->mask, e2->mask, SYS_ADPT_MAC_ADDR_LEN) == 0 &&
    strcmp(e1->description, e2->description) == 0) ? TRUE : FALSE;
}

void test_set_oui()
{
    enum {MAX_STR_LEN = 50};

    {
        ADD_MGR_VoiceVlanOui_T entry = {0};
        ADD_MGR_VoiceVlanOui_T entry2= {0};

        entry.oui[0] = entry.oui[1] = entry.oui[2] = entry.oui[3] = entry.oui[4] = entry.oui[5] = 6;
        entry.mask[0] = entry.mask[1] = entry.mask[2] = entry.mask[3] = entry.mask[4] = entry.mask[5] = 0XFF;
        strcpy(entry.description, "testing...");

        ASSERT_TRUE(ADD_PMGR_AddOuiEntry(&entry));

        entry2.oui[0] = entry2.oui[1] = entry2.oui[2] = entry2.oui[3] = entry2.oui[4] = entry2.oui[5] = 6;
        ASSERT_TRUE(ADD_PMGR_GetOuiEntry(&entry2));
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        ASSERT_TRUE(ADD_PMGR_RemoveOuiEntry(entry.oui, entry.mask));
        ASSERT_FALSE(ADD_PMGR_GetOuiEntry(&entry2));
    }

    {
        ADD_MGR_VoiceVlanOui_T entry = {0};
        ADD_MGR_VoiceVlanOui_T entry2= {0};

        entry.oui[0] = entry.oui[1] = entry.oui[2] = entry.oui[3] = entry.oui[4] = entry.oui[5] = 6;
        entry.mask[0] = entry.mask[1] = entry.mask[2] = entry.mask[3] = entry.mask[4] = entry.mask[5] = 0XFF;
        strcpy(entry.description, "testing...");

        ASSERT_TRUE(ADD_PMGR_AddOuiEntry(&entry));

        entry2.oui[0] = entry2.oui[1] = entry2.oui[2] = entry2.oui[3] = entry2.oui[4] = entry2.oui[5] = 0;
        ASSERT_TRUE(ADD_PMGR_GetNextOuiEntry(&entry2));
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        ASSERT_FALSE(ADD_PMGR_GetNextOuiEntry(&entry2));

        entry2.oui[0] = entry2.oui[1] = entry2.oui[2] = entry2.oui[3] = entry2.oui[4] = entry2.oui[5] = 0;
        ASSERT_TRUE(ADD_PMGR_GetNextRunningOuiEntry(&entry2) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        ASSERT_TRUE(ADD_PMGR_GetNextOuiEntry(&entry2) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);

        entry2.oui[0] = entry2.oui[1] = entry2.oui[2] = entry2.oui[3] = entry2.oui[4] = entry2.oui[5] = 0;
        ASSERT_TRUE(ADD_PMGR_GetNextRunningOui(entry2.oui, entry2.mask, entry2.description) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        ASSERT_TRUE(ADD_PMGR_GetNextRunningOui(entry2.oui, entry2.mask, entry2.description) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);

        ASSERT_TRUE(ADD_PMGR_RemoveOuiEntry(entry.oui, entry.mask));
        ASSERT_FALSE(ADD_PMGR_GetOuiEntry(&entry2));
    }

    {
        ADD_MGR_VoiceVlanOui_T entry = {0};
        ADD_MGR_VoiceVlanOui_T entry2= {0};

        entry.oui[0] = entry.oui[1] = entry.oui[2] = entry.oui[3] = entry.oui[4] = entry.oui[5] = 6;
        entry.mask[0] = entry.mask[1] = entry.mask[2] = entry.mask[3] = entry.mask[4] = entry.mask[5] = 0XFF;
        strcpy(entry.description, "testing...");

        ASSERT_TRUE(ADD_PMGR_AddOuiEntry(&entry));

        entry2.oui[0] = entry2.oui[1] = entry2.oui[2] = entry2.oui[3] = entry2.oui[4] = entry2.oui[5] = 6;
        ASSERT_TRUE(ADD_PMGR_GetOuiEntry(&entry2));
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        entry.mask[0] = entry.mask[1] = entry.mask[2] = entry.mask[3] = entry.mask[4] = entry.mask[5] = 0X77;
        ASSERT_TRUE(ADD_PMGR_SetOuiMaskAddress(entry2.oui, entry.mask));
        ASSERT_TRUE(ADD_PMGR_GetOuiEntry(&entry2));
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        strcpy(entry.description, "abc");
        ASSERT_TRUE(ADD_PMGR_SetOuiDescription(entry2.oui, entry.description));
        ASSERT_TRUE(ADD_PMGR_GetOuiEntry(&entry2));
        ASSERT_TRUE(IsIdentify(&entry, &entry2));

        ASSERT_TRUE(ADD_PMGR_SetOuiState(entry2.oui, 2));
        ASSERT_FALSE(ADD_PMGR_GetOuiEntry(&entry2));
    }
}

void test_set_aging()
{
    enum {MAX_STR_LEN = 50};

    {
        UI32_T aging;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanAgingTime(1));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanAgingTime(1));

        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanAgingTime(&aging));
        sprintf(pmgr_result, "get aging=%d", aging);

        ASSERT_TRUE(ADD_MGR_GetVoiceVlanAgingTime(&aging));
        sprintf(mgr_result, "get aging=%d", aging);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        UI32_T aging;
        char pmgr_result[MAX_STR_LEN+1] = {0};
        char mgr_result[MAX_STR_LEN+1] = {0};

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanAgingTime(2));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanAgingTime(2));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanAgingTime(&aging) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(pmgr_result, "get aging=%d", aging);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanAgingTime(&aging) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        sprintf(mgr_result, "get aging=%d", aging);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanAgingTime(5));
        ASSERT_TRUE(ADD_MGR_SetVoiceVlanAgingTime(5));

        ASSERT_TRUE(ADD_PMGR_GetRunningVoiceVlanAgingTime(&aging) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(pmgr_result, "get aging=%d", aging);

        ASSERT_TRUE(ADD_MGR_GetRunningVoiceVlanAgingTime(&aging) == SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        sprintf(mgr_result, "get aging=%d", aging);
        ASSERT_TRUE(strcmp(pmgr_result, mgr_result)==0);
    }

    {
        ASSERT_FALSE(ADD_PMGR_SetVoiceVlanAgingTime(101));
        ASSERT_FALSE(ADD_MGR_SetVoiceVlanAgingTime(101));
    }

    {
        UI32_T day;
        UI32_T hour;
        UI32_T minute;

        ASSERT_TRUE(ADD_PMGR_SetVoiceVlanAgingTimeByDayHourMinute(1, 2, 3));
        ASSERT_TRUE(ADD_PMGR_GetVoiceVlanAgingTimeByDayHourMinute(&day, &hour, &minute));
        ASSERT_TRUE(day == 1 && hour == 2 && minute == 3);
    }
}

void test_set_debug()
{
    printf("\n");
    ADD_PMGR_SetDebugPrintStatus(TRUE);

    printf("\n");
    ADD_PMGR_SetDebugPrintStatus(FALSE);
}


int main(int argc, unsigned char* argv[])
{
    test_set_voice_vlan_id();
    test_is_voice_vlan_enabled();
    test_set_voice_vlan_port_mode();
    test_set_voice_vlan_port_security_state();
    test_set_voice_vlan_port_oui_rule_state();
    test_set_voice_vlan_port_lldp_rule_state();
    test_set_voice_vlan_port_priority();
    test_set_oui();
    test_set_aging();
    test_set_debug();

    printf("\n");
	return 0;
}
#endif /* #ifdef ADD_PMGR_DO_UNIT_TEST */

#endif  /* #if (SYS_CPNT_ADD == TRUE) */

