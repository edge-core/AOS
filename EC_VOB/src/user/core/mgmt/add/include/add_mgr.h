/* FUNCTION NAME: add_mgr.h
 * PURPOSE:
 *	1. Manage the ADD CSC
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#ifndef ADD_MGR_H
#define ADD_MGR_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "add_type.h"

#define VOICE_VLAN_SUPPORT_ACCTON_BACKDOOR  TRUE
#define VOICE_VLAN_DO_UNIT_TEST             TRUE

#define ADD_MAX_MSGQ_LEN                    64


#define ADD_MGR_GET_MSGBUFSIZE(type_name) \
    ((UI32_T)&((ADD_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

#define ADD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(ADD_MGR_IPCMsg_Type_T)

#define ADD_MGR_MSG_CMD(msg_p)      (((ADD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define ADD_MGR_MSG_RETVAL(msg_p)   (((ADD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)
#define ADD_MGR_MSG_DATA(msg_p)    ((void *)&((ADD_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)

/* The commands for IPC message.
 */
enum {
    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_ENABLED_ID,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_ID,
	ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_ID,

	ADD_MGR_IPC_CMD_IS_VOICE_VLAN_ENABLED,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_MODE,
	ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_MODE,
	ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_MODE,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_SECURITY_STATE,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_SECURITY_STATE,
    ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_SECURITY_STATE,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_OUI_RULE_STATE,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_OUI_RULE_STATE,
    ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_OUI_RULE_STATE,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_LLDP_RULE_STATE,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_LLDP_RULE_STATE,
    ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_LLDP_RULE_STATE,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_PRIORITY,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_PRIORITY,
    ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_PRIORITY,

    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_REMAIN_AGE,

    ADD_MGR_IPC_CMD_ADD_OUI_ENTRY,
    ADD_MGR_IPC_CMD_GET_OUI_ENTRY,
    ADD_MGR_IPC_CMD_GET_NEXT_OUI_ENTRY,
    ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI_ENTRY,
    ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI,

    ADD_MGR_IPC_CMD_REMOVE_OUI_ENTRY,

    ADD_MGR_IPC_CMD_SET_OUI_STATE,
    ADD_MGR_IPC_CMD_SET_OUI_MASK_ADDRESS,
    ADD_MGR_IPC_CMD_SET_OUI_DESCRIPTION,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME,
    ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_AGING_TIME,

    ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE,
    ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE,

    ADD_MGR_IPC_CMD_SET_DEBUG_PRINT_STATUS,
};



typedef struct
{
    UI8_T oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask[SYS_ADPT_MAC_ADDR_LEN];
    char  description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
} ADD_MGR_VoiceVlanOui_T;



typedef struct
{
    I32_T vid;
} ADD_MGR_SetVoiceVlanEnabledIdParam_T,
  ADD_MGR_GetRunningVoiceVlanIdParam_T,
  ADD_MGR_GetVoiceVlanIdParam_T;

typedef struct
{
    UI32_T lport;
    UI32_T mode;
} ADD_MGR_SetVoiceVlanPortModeParam_T,
  ADD_MGR_GetVoiceVlanPortModeParam_T,
  ADD_MGR_GetRunningVoiceVlanPortModeParam_T;

typedef struct
{
    UI32_T lport;
    UI32_T state;
} ADD_MGR_SetVoiceVlanPortSecurityStateParam_T,
  ADD_MGR_GetVoiceVlanPortSecurityStateParam_T,
  ADD_MGR_GetRunningVoiceVlanPortSecurityStateParam_T,

  ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T,
  ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T,
  ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T,

  ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T,
  ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T,
  ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T;

typedef struct
{
    UI32_T lport;
    UI8_T  priority;
} ADD_MGR_SetVoiceVlanPortPriorityParam_T,
  ADD_MGR_GetVoiceVlanPortPriorityParam_T,
  ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T;

typedef struct
{
    UI32_T lport;
    I32_T  remain_age;
} ADD_MGR_GetVoiceVlanPortRemainAgeParam_T;

typedef struct
{
    ADD_MGR_VoiceVlanOui_T entry;
} ADD_MGR_AddOuiEntryParam_T,
  ADD_MGR_GetOuiEntryParam_T,
  ADD_MGR_GetNextOuiEntryParam_T,
  ADD_MGR_GetNextRunningOuiEntryParam_T;

typedef struct
{
    UI8_T  oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask[SYS_ADPT_MAC_ADDR_LEN];
} ADD_MGR_RemoveOuiEntryParam_T,
  ADD_MGR_SetOuiMaskAddressParam_T;

typedef struct
{
    UI8_T  oui[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T state;
} ADD_MGR_SetOuiStateParam_T;

typedef struct
{
    UI8_T  oui[SYS_ADPT_MAC_ADDR_LEN];
    char   description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
} ADD_MGR_SetOuiDescriptionParam_T;

typedef struct
{
    UI8_T  oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask[SYS_ADPT_MAC_ADDR_LEN];
    char   description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
} ADD_MGR_GetNextRunningOuiParam_T;

typedef struct
{
    UI32_T  timeout;
} ADD_MGR_SetVoiceVlanAgingTimeParam_T,
  ADD_MGR_GetVoiceVlanAgingTimeParam_T,
  ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T;

typedef struct
{
    UI32_T day;
    UI32_T hour;
    UI32_T minute;
} ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T,
  ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T;

typedef struct
{
    BOOL_T  state;
} ADD_MGR_SetDebugPrintStatusParam_T;

typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
}ADD_MGR_IPCMsg_Type_T;

typedef union
{
    ADD_MGR_SetVoiceVlanEnabledIdParam_T p1;
}ADD_MGR_IPCMsg_Data_T;

typedef struct
{
    ADD_MGR_IPCMsg_Type_T    type;
    ADD_MGR_IPCMsg_Data_T    data;
}ADD_MGR_IPCMsg_T;

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
BOOL_T ADD_MGR_Initiate_System_Resources(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_CurrentOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of add's
 *           task.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   add_operation_mode
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T ADD_MGR_CurrentOperationMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_EnterMasterMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_EnterSlaveMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ----------------------------------------------------------------------
 */
void ADD_MGR_SetTransitionMode(void);

/*----------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterTransition Mode
 *----------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *----------------------------------------------------------------------
 */
void ADD_MGR_EnterTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsVoiceVlanEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN state.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE               - Enabled
 *          FALSE              - Disabled
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_IsVoiceVlanEnabled();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get voice VLAN ID.
 * INPUT:   None.
 * OUTPUT:  vid               -- the voice VLAN ID.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanId(I32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVLANId
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanId(I32_T *vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanEnabledId
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
BOOL_T ADD_MGR_SetVoiceVlanEnabledId(I32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortMode
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
BOOL_T ADD_MGR_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortMode
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortMode(UI32_T lport, UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortMode
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
BOOL_T ADD_MGR_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortSecurityState
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
BOOL_T ADD_MGR_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortSecurityState
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortSecurityState
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
BOOL_T ADD_MGR_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortOuiRuleState
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
BOOL_T ADD_MGR_GetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortLldpRuleState
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
BOOL_T ADD_MGR_GetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortOuiRuleState
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortLldpRuleState
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortOuiRuleState
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
BOOL_T ADD_MGR_SetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortLldpRuleState
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
BOOL_T ADD_MGR_SetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortPriority
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
BOOL_T ADD_MGR_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortPriority
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortPriority(UI32_T lport, UI8_T *priority);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortPriority
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
BOOL_T ADD_MGR_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortRemainAge
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
BOOL_T ADD_MGR_GetVoiceVlanPortRemainAge(UI32_T lport, I32_T *age_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AddOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Add a recognised OUI entry.
 * INPUT:   entry             -- the OUI entry.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. Don't allow the multicast address for the OUI address.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_AddOuiEntry(ADD_MGR_VoiceVlanOui_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_RemoveOuiEntry
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
BOOL_T ADD_MGR_RemoveOuiEntry(UI8_T *oui, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiState
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
BOOL_T ADD_MGR_SetOuiState(UI8_T *oui, UI32_T state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiMaskAddress
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
BOOL_T ADD_MGR_SetOuiMaskAddress(UI8_T *oui, UI8_T *mask);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiDescription
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
BOOL_T ADD_MGR_SetOuiDescription(UI8_T *oui, UI8_T *description);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function gets OUI entry based on oui's MAC-address.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetOuiEntry(ADD_MGR_VoiceVlanOui_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextOui
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
BOOL_T ADD_MGR_GetNextOuiEntry(ADD_MGR_VoiceVlanOui_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextRunningOuiEntry
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOuiEntry(ADD_MGR_VoiceVlanOui_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextRunningOui
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOui(UI8_T *oui, UI8_T *mask, UI8_T *description);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  timeout           -- the aging time, minute unit.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanAgingTime(UI32_T *timeout);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanAgingTime
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
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanAgingTime(UI32_T *timeout);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   timeout           -- the aging time, minute unit.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanAgingTime(UI32_T timeout);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  day/hour/minute.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(UI32_T *day, UI32_T *hour, UI32_T *minute);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   day/hour/minute.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(UI32_T day, UI32_T hour, UI32_T minute);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanEnabledPortList
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN enabled port list.
 * INPUT  : None.
 * OUTPUT : port_ar  - The port list. The size of the port list must larger
 *                     than SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST.
 * RETURN : None.
 * NOTES  : The rule for mapping the lport index to port list:
 *          1. Logic port 1-8 map to port list byte 0.
 *          2. Logic port 1 map to bit 7 of byte 0.
 *             Logic port 2 map to bit 6 of byte 0.
 *             Logic port 3 map to bit 5 of byte 0, etc.
 *          3. Logic port 9-15 map to port list byte 1.
 *          4. Logic port 9 map to bit 7 of byte 1, etc.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_GetVoiceVlanEnabledPortList(UI8_T port_ar[]);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessTimeoutMessage
 * ---------------------------------------------------------------------
 * PURPOSE: Process the timeout message.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. This function would be invoked by periodic timer event.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_ProcessTimeoutMessage(UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetDebugPrintStatus
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
void ADD_MGR_SetDebugPrintStatus(BOOL_T state);



BOOL_T ADD_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
void ADD_MGR_ProcessTimeoutEvent();



void ADD_MGR_CLI_ProvisionComplete_CallBack();
void ADD_MGR_VLAN_VlanMemberDelete_CallBack(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: Process the new MAC address.
 *          1. Check for security issue
 *          2. Auto join to Voice VLAN when OUI bit is trun-on
 *          ### 3. Remap the MAC to new priority queue
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 *          is_add
 * OUTPUT:  None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_ProcessNewMac(UI32_T vid, UI8_T *mac, UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AMTR_SetStaticMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function by user set a static mac address.
 * INPUT:   vid                         -- the VLAN ID of the MAC address
 *          mac                         -- the MAC address
 *          lport                       -- the logic port index
 * RETURN:  TRUE                        -- Authorized.
 *          FALSE                       -- None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_AMTR_SetStaticMacCheck_CallBack(UI32_T vid, UI8_T *mac, UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessAgedMac
 * ---------------------------------------------------------------------
 * PURPOSE: This function be callbacked when AMTR notify a new/delete mac
 *          address.
 * INPUT:   vid                        -- the VLAN ID of the MAC address
 *          mac                        -- the MAC address
 *          lport                      -- the logic port index
 *          is_age                     -- TRUE, an aged mac address.
 *                                        FALSE, else.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_AMTR_EditAddrNotify_CallBack(UI32_T vid, UI8_T *mac, UI32_T lport, BOOL_T is_age);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_LLDP_TelephoneDetect_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be callbacked when LLDP notify a came/left
 *          phone notify.
 * INPUT:   lport                - logic port index
 *          mac_addr             - the MAC address
 *          network_addr_subtype - the subtype of the network address
 *          network_addr         - the network address
 *          network_addr_len     - the length of the network address
 *          network_addr_ifindex - the VLAN ID
 *          tel_exist            - TRUE, a came phone notify. FALSE,else.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_LLDP_TelephoneDetect_CallBack(UI32_T  lport,
                                           UI8_T  *mac_addr,
                                           UI8_T   network_addr_subtype,
                                           UI8_T  *network_addr,
                                           UI8_T   network_addr_len,
                                           UI32_T  network_addr_ifindex,
                                           BOOL_T  tel_exist);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PortLinkUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be invoked when port link up.
 * INPUT:   unit                - unit
 *          port                - port
 * OUTPUT:  None.
 * RETURN:
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_MGR_PortLinkUp_CallBack(UI32_T unit, UI32_T port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PortLinkDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be invoked when port link down.
 * INPUT:   unit                - unit
 *          port                - port
 * OUTPUT:  None.
 * RETURN:
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_MGR_PortLinkDown_CallBack(UI32_T unit, UI32_T port);

#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessAgedMac
 * ---------------------------------------------------------------------
 * PURPOSE: Process the aged mac address.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_ProcessAgedMac(UI32_T vid, UI8_T *mac, UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AuthorizeNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: To authorize the new MAC.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, success to authorize the new MAC address. FALSE, else.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_AuthorizeNewMac(UI32_T vid, UI8_T *mac, UI32_T lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AutoJoinToVoiceVlanByNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: To authorize the new MAC.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, success to authorize the new MAC address. FALSE, else.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_AutoJoinVoiceVlanByNewMac(UI32_T vid, UI8_T *mac, UI32_T lport);

#endif /* #if (VOICE_VLAN_DO_UNIT_TEST == TRUE) */

#endif /*ADD_MGR_H*/
