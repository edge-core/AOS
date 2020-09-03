/* FUNCTION NAME: add_om.h
 * PURPOSE:
 *	1. ADD local object management
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */

#ifndef	ADD_OM_H
#define	ADD_OM_H

typedef struct ADD_OM_VoiceVlanPortEntry_S
{
    /* volatile data */
    UI32_T disjoin_when;                /* 0, disable disjoin timer               */
    UI32_T oui_learned_count;           /* number of oui phone                    */

    UI32_T mode;                        /* port mode                              */

    UI32_T security_state;              /* port security state                    */

    UI8_T  protocol;                    /* support discovery protocol             */
    UI8_T  priority;                    /* remap prioirty for Voice VLAN          */

    /* For auto-voice-vlan */
    UI8_T  join_state;                   /* TRUE, if auto join to voice vlan now   */
}ADD_OM_VoiceVlanPortEntry_T;


#if 0
#define ADD_OM_IS_MATCH_OUI(oui, mask, mac) (\
    (((*(UI32_T*)(&oui[0])) & (*(UI32_T*)(&mask[0]))) == ((*(UI32_T*)(&mac[0])) & (*(UI32_T*)(&mask[0])))) &&\
    (((*(UI16_T*)(&oui[4])) & (*(UI16_T*)(&mask[4]))) == ((*(UI16_T*)(&mac[4])) & (*(UI16_T*)(&mask[4]))))\
    )
#endif

BOOL_T ADD_OM_IS_MATCH_OUI(UI8_T *oui, UI8_T *mask, UI8_T *mac);


/*------------------------------------------------------------------------
 * DEFAULT VARIABLES DECLARTIONS
 *-----------------------------------------------------------------------*/
/* Need to sync. sys_dflt.h */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Task
 * INPUT:   task_id.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_OM_SetTaskId(UI32_T taskid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_TaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Task
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  Task id.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
UI32_T ADD_OM_GetTaskId();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetMessageQueueId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of message queue.
 * INPUT:   msgq_id           - message queue ID.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_OM_SetMessageQueueId(UI32_T msgq_id);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetMessageQueueId
 * ---------------------------------------------------------------------
 * PURPOSE: Get ID of message queue.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  Message queue ID.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
UI32_T ADD_OM_GetMessageQueueId();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize om
 * INPUT:   None
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_Initialize();


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: get voice vlan id
 * INPUT:   None
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanId(I32_T *vid);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: set voice vlan id
 * INPUT:   vid                - Voice VLAN ID
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_OM_SetVoiceVlanId(I32_T vid);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_IsVoiceVlanEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN state.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE               - Enabled
 *          FALSE              - Disabled
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_IsVoiceVlanEnabled();


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port mode.
 * INPUT:   lport              - lport index.
 * OUTPUT:  entry              - the entry of the lport.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortEntry(UI32_T lport, ADD_OM_VoiceVlanPortEntry_T *entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port mode.
 * INPUT:   lport              - lport index.
 * OUTPUT:  mode               - VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_auto,
 *                               VAL_voiceVlanPortMode_manual.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port mode.
 * INPUT:   lport              - lport index.
 *          mode               - VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_auto,
 *                               VAL_voiceVlanPortMode_manual.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port security state.
 * INPUT:   lport              - lport index.
 * OUTPUT:  state              - VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port security state.
 * INPUT:   lport              - lport index.
 *          state              - VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortRuleBitmap
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port protocol.
 * INPUT:   lport              - lport index.
 *          rule_bitmap        - the port rule bitmap.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_OUI
 *                                   Use AMTR learning.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_LLDP
 *                                   Use 802.1AB protocol.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_DHCP
 *                                   Use DHCP protocol.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortRuleBitmap(UI32_T lport, UI8_T *rule_bitmap);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortRuleBitmap
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port protocol.
 * INPUT:   lport              - lport index.
 *          rule_bitmap        - the port rule bitmap.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_OUI
 *                                   Use AMTR learning.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_LLDP
 *                                   Use 802.1AB protocol.
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_DHCP
 *                                   Use DHCP protocol.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortRuleBitmap(UI32_T lport, UI8_T rule_bitmap);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port priority.
 * INPUT:   lport              - lport index.
 * OUTPUT:  prioirty           - the lport priority.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port priority.
 * INPUT:   lport              - lport index.
 *          prioiryt           - the lport priority.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortDisjoinTimerState
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port disjoin timer state.
 * INPUT:   lport              - lport index.
 * OUTPUT:  state              - TRUE/FALSE.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortDisjoinTimerState(UI32_T lport, BOOL_T *state);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortDisjoinTimerState
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port disjoin timer state.
 * INPUT:   lport              - lport index.
 *          state              - TRUE/FALSE.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortDisjoinTimerState(UI32_T lport, BOOL_T state);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortDisjoinWhen
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:   lport              - lport index.
 * OUTPUT:  state              - TRUE/FALSE.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortDisjoinWhen(UI32_T lport, UI32_T *disjoin_when);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortDisjoinWhen
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:   lport              - lport index.
 *          state              - TRUE/FALSE.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortDisjoinWhen(UI32_T lport, UI32_T disjoin_when);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortLearnedCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port learned device count.
 * INPUT:   lport              - lport index.
 *          count              - count value
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortOuiLearnedCount(UI32_T lport, UI32_T count);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortLearnedCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port learned device count.
 * INPUT:   lport              - lport index.
 * OUTPUT:  count              - count value
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortOuiLearnedCount(UI32_T lport, UI32_T *count);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortAutoJoinFlag
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port join state by discovery protocol.
 * INPUT:   lport              - lport index.
 *          protocol           - the bitmap of the discovery protocol.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortJoinState(UI32_T lport, UI8_T protocol);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_ClearVoiceVlanPortJoinState
 * ---------------------------------------------------------------------
 * PURPOSE: Clear Voice VLAN port join state by discovery protocol.
 * INPUT:   lport              - lport index.
 *          protocol           - the bitmap of the discovery protocol.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortJoinState(UI32_T lport, UI8_T *join_state);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortJoinState
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port join state.
 * INPUT:   lport              - lport index.
 * OUTPUT:  join_state         - the bitmap of the discovery protocol.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_ClearVoiceVlanPortJoinState(UI32_T lport, UI8_T protocol);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_IsVoiceVlanPortAutojJoin
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port auto join flag.
 * INPUT:   lport              - lport index.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_IsVoiceVlanPortAutoJoin(UI32_T lport);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_ShowVoiceVlanStatusDebug
 * ---------------------------------------------------------------------
 * PURPOSE: ADD_OM_ShowVoiceVlanStatusDebug.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_OM_ShowVoiceVlanStatusDebug(UI32_T from_lport, UI32_T to_lport);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_IsOui
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether for recognised device by mac address.
 * INPUT:   mac                - mac address.
 * OUTPUT:  None.
 * RETURN:  TRUE               - recognised device.
            FALSE              - not recognised device.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_IsOui(UI8_T *mac);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_AddOui
 * ---------------------------------------------------------------------
 * PURPOSE: Add a recognised OUI.
 * INPUT:   oui                - oui address.
 *          mask               - mask of the oui address.
 *          description        - descritption of this oui, the max length
 *                               is 30 character.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_AddOui(UI8_T *oui, UI8_T *mask, UI8_T *description);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_RemoveOui
 * ---------------------------------------------------------------------
 * PURPOSE: Remove a recognised OUI.
 * INPUT:   oui                - oui address.
 *          mask               - mask of the oui address.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_RemoveOui(UI8_T *oui, UI8_T *mask);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetOui
 * ---------------------------------------------------------------------
 * PURPOSE: .
 * INPUT:   oui                - oui address (key).
 * OUTPUT:  oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 *          description       -- the descritption of the OUI MAC address,
 *                               the max length is 30 character.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetOui(UI8_T *oui, UI8_T *mask, UI8_T *description);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetNextOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next recognised OUI.
 * INPUT:   None.
 * OUTPUT:  oui                - oui address.
 *          mask               - mask of the oui address.
 *          description        - descritption of this oui.
 * RETURN:  TRUE/FALSE.
 * NOTES:   To get first recognised OUI by pass OUI = FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetNextOui(UI8_T *oui, UI8_T *mask, UI8_T *description);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_ShowRecognisedDevice
 * ---------------------------------------------------------------------
 * PURPOSE: List all recognised OUI address.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_OM_ShowOui();


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set aging timeout value of Voice VLAN.
 * INPUT:   timeout value, unit is minute.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanAgingTime(UI32_T timeout);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get aging tim-out value of Voice VLAN.
 * INPUT:   None.
 * OUTPUT:  timeout value, unit is minute.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanAgingTime(UI32_T *timeout);

#endif /*ADD_OM_H*/
