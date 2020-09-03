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
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ADD == TRUE)

#include <stdio.h>
#include <string.h>
#include "sys_dflt.h"
#include "sysfun.h"
#include "add_type.h"
#include "add_om.h"

/*#define _ADD_OM_DEBUG*/
typedef struct ADD_OM_VoiceVlanOui_S
{
    UI8_T  oui[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    BOOL_T status;                      /* TRUE, if is vaild entry                */
}ADD_OM_VoiceVlanOui_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define ADD_OM_ENTER_CRITICAL_SECTION()  add_om_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(add_om_sem_id);
#define ADD_OM_LEAVE_CRITICAL_SECTION()  SYSFUN_OM_LEAVE_CRITICAL_SECTION(add_om_sem_id, add_om_orig_priority);

#define ADD_OM_IS_LPORT(ifindex) ((ifindex != 0) && (ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT))

/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T add_om_task_id;           /* add task id                            */
static UI32_T add_om_msgq_id;           /* add message id                         */

static UI32_T add_om_sem_id;
static UI32_T add_om_orig_priority;

//static BOOL_T add_om_voice_vlan_state;  /* global state                           */
static I32_T add_om_voice_vlan_id;     /* global voice id                        */
static UI32_T add_om_voice_vlan_timeout;/* global timeour, the unit is minute     */

static ADD_OM_VoiceVlanPortEntry_T  add_om_voice_vlan_port_table[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static ADD_OM_VoiceVlanOui_T        add_om_voice_vlan_oui[SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI];

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetTaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of Task
 * INPUT:  task_id.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void ADD_OM_SetTaskId(UI32_T taskid)
{
    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_task_id = taskid ;
    ADD_OM_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_TaskId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of Task
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: Task id.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T ADD_OM_GetTaskId(void)
{
    return add_om_task_id ;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetMessageQueueId
 * ---------------------------------------------------------------------
 * PURPOSE: Set id of message queue
 * INPUT:  msgq_id.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
void ADD_OM_SetMessageQueueId(UI32_T msgq_id)
{
    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_msgq_id = msgq_id;
    ADD_OM_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetMessageQueueId
 * ---------------------------------------------------------------------
 * PURPOSE: Get id of message queue
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: Msg Q id.
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
UI32_T ADD_OM_GetMessageQueueId()
{
    return add_om_msgq_id;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_Initialize
 * ---------------------------------------------------------------------
 * PURPOSE: initialize om
 * INPUT:  none
 * OUTPUT: None.
 * RETURN: TRUE -- succeeded, FALSE -- failed
 * NOTES:  None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_Initialize()
{
    int lport, i;

    ADD_OM_ENTER_CRITICAL_SECTION();

    add_om_voice_vlan_id = SYS_DFLT_ADD_VOICE_VLAN_ID;
    add_om_voice_vlan_timeout = SYS_DFLT_ADD_VOICE_VLAN_TIMEOUT_MINUTE;

    for(i=0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        add_om_voice_vlan_oui[i].status = FALSE;
    }

    for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        add_om_voice_vlan_port_table[lport-1].mode              = SYS_DFLT_ADD_VOICE_VLAN_PORT_MODE;
        add_om_voice_vlan_port_table[lport-1].security_state    = SYS_DFLT_ADD_VOICE_VLAN_PORT_SECURITY_STATE;

        add_om_voice_vlan_port_table[lport-1].protocol          = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
        if(SYS_DFLT_ADD_VOICE_VLAN_PORT_RULE_OUI_STATUS == VAL_voiceVlanPortRuleOui_enabled)
            add_om_voice_vlan_port_table[lport-1].protocol     |= ADD_TYPE_DISCOVERY_PROTOCOL_OUI;
        if(SYS_DFLT_ADD_VOICE_VLAN_PORT_RULE_LLDP_STATUS == VAL_voiceVlanPortRuleLldp_enabled)
            add_om_voice_vlan_port_table[lport-1].protocol     |= ADD_TYPE_DISCOVERY_PROTOCOL_LLDP;

        add_om_voice_vlan_port_table[lport-1].priority          = SYS_DFLT_ADD_VOICE_VLAN_PORT_PRIORITY;
        add_om_voice_vlan_port_table[lport-1].disjoin_when      = ADD_TYPE_VOICE_VLAN_TIMER_DISABLED;
        add_om_voice_vlan_port_table[lport-1].join_state        = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
        add_om_voice_vlan_port_table[lport-1].oui_learned_count = 0;
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


BOOL_T ADD_OM_CreatSem(void)
{
    /* create semaphore */
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &add_om_sem_id) != SYSFUN_OK)
    {
        return FALSE;
    }

    return TRUE;
} /* End of NETACCESS_OM_CreatSem */


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
BOOL_T ADD_OM_GetVoiceVlanId(I32_T *vid)
{
    if(NULL == vid)
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *vid = add_om_voice_vlan_id;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
void ADD_OM_SetVoiceVlanId(I32_T vid)
{
    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_id = vid;
    ADD_OM_LEAVE_CRITICAL_SECTION();
}

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
BOOL_T ADD_OM_IsVoiceVlanEnabled()
{
    return (add_om_voice_vlan_id!=VAL_voiceVlanEnabledId_disabled)?TRUE:FALSE;
}

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
BOOL_T ADD_OM_GetVoiceVlanPortEntry(UI32_T lport, ADD_OM_VoiceVlanPortEntry_T *entry)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == entry))
    {
        return FALSE;
    }

    memset(entry, 0, sizeof(ADD_OM_VoiceVlanPortEntry_T));

    ADD_OM_ENTER_CRITICAL_SECTION();
    memcpy(entry, &add_om_voice_vlan_port_table[lport-1], sizeof(ADD_OM_VoiceVlanPortEntry_T));
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == mode))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *mode = add_om_voice_vlan_port_table[lport-1].mode;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ADD_OM_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    switch(mode)
    {
    case VAL_voiceVlanPortMode_none:
    case VAL_voiceVlanPortMode_auto:
    case VAL_voiceVlanPortMode_manual:
        ADD_OM_ENTER_CRITICAL_SECTION();
        add_om_voice_vlan_port_table[lport-1].mode = mode;
        ADD_OM_LEAVE_CRITICAL_SECTION();

        return TRUE;
    }

    return FALSE;
}


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
BOOL_T ADD_OM_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == state))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *state = add_om_voice_vlan_port_table[lport-1].security_state;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].security_state = state;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortRuleBitmap
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port protocol.
 * INPUT:   lport              - lport index.
 *          rule_bitmap        - the port rule bitmap;
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
BOOL_T ADD_OM_GetVoiceVlanPortRuleBitmap(UI32_T lport, UI8_T *rule_bitmap)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == rule_bitmap))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *rule_bitmap = add_om_voice_vlan_port_table[lport-1].protocol;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_SetVoiceVlanPortRuleBitmap(UI32_T lport, UI8_T rule_bitmap)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (rule_bitmap > ADD_TYPE_DISCOVERY_PROTOCOL_ALL))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].protocol = rule_bitmap;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == priority))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *priority = add_om_voice_vlan_port_table[lport-1].priority;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    if(/*(priority < MIN_voiceVlanPortPriority) ||*/ (priority > MAX_voiceVlanPortPriority))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].priority = priority;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_GetVoiceVlanPortDisjoinWhen(UI32_T lport, UI32_T *disjoin_when)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == disjoin_when))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *disjoin_when = add_om_voice_vlan_port_table[lport-1].disjoin_when;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortDisjoinWhen
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:   lport              - lport index.
 *          state              - TRUE/FALSE.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortDisjoinWhen(UI32_T lport, UI32_T disjoin_when)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].disjoin_when = disjoin_when;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanPortOuiLearnedCount
 * ---------------------------------------------------------------------
 * PURPOSE: Set Voice VLAN port learned device count.
 * INPUT:   lport              - lport index.
 *          count              - count value
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanPortOuiLearnedCount(UI32_T lport, UI32_T count)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].oui_learned_count = count;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanPortOuiLearnedCount
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN port learned device count.
 * INPUT:   lport              - lport index.
 * OUTPUT:  count              - count value
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanPortOuiLearnedCount(UI32_T lport, UI32_T *count)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == count))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *count = add_om_voice_vlan_port_table[lport-1].oui_learned_count;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}


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
BOOL_T ADD_OM_SetVoiceVlanPortJoinState(UI32_T lport, UI8_T protocol)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (protocol > ADD_TYPE_DISCOVERY_PROTOCOL_ALL))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].join_state |= protocol;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ADD_OM_ClearVoiceVlanPortJoinState(UI32_T lport, UI8_T protocol)
{
    if(FALSE == ADD_OM_IS_LPORT(lport))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_port_table[lport-1].join_state &= ~protocol;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
BOOL_T ADD_OM_GetVoiceVlanPortJoinState(UI32_T lport, UI8_T *join_state)
{
    if((FALSE == ADD_OM_IS_LPORT(lport)) || (NULL == join_state))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *join_state = add_om_voice_vlan_port_table[lport-1].join_state;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_IsMatchOui
 * ---------------------------------------------------------------------
 * PURPOSE: Check whether for recognised device by mac address.
 * INPUT:   mac                - mac address.
 * OUTPUT:  None.
 * RETURN:  TRUE               - recognised device.
            FALSE              - not recognised device.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_IS_MATCH_OUI(UI8_T *oui, UI8_T *mask, UI8_T *mac)
{
    if((oui[0] & mask[0]) != (mac[0] & mask[0]))
        return FALSE;
    if((oui[1] & mask[1]) != (mac[1] & mask[1]))
        return FALSE;
    if((oui[2] & mask[2]) != (mac[2] & mask[2]))
        return FALSE;
    if((oui[3] & mask[3]) != (mac[3] & mask[3]))
        return FALSE;
    if((oui[4] & mask[4]) != (mac[4] & mask[4]))
        return FALSE;
    if((oui[5] & mask[5]) != (mac[5] & mask[5]))
        return FALSE;

    return TRUE;
}

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
BOOL_T ADD_OM_IsOui(UI8_T *mac)
{
    int i;

    ADD_OM_ENTER_CRITICAL_SECTION();

    for(i = 0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if(FALSE == add_om_voice_vlan_oui[i].status)
        {
            continue;
        }

            if(TRUE == ADD_OM_IS_MATCH_OUI(add_om_voice_vlan_oui[i].oui,
                                   add_om_voice_vlan_oui[i].mask, mac))
        {
            /* Found. */
            ADD_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }

    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_AddOui
 * ---------------------------------------------------------------------
 * PURPOSE: Add a recognised OUI.
 * INPUT:   oui                - oui address.
 *          mask               - mask of the oui address.
 *          description        - descritption of this oui.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   1. Don't allow the multicast address for the OUI address.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_AddOui(UI8_T *oui, UI8_T *mask, UI8_T *description)
{
    int i;

    if((NULL == oui) || (NULL == mask))
    {
        return FALSE;
    }

    /* Don't allow the multicast address for the OUI address */
    if(oui[0] & 0x01)
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();

    /* duplicable? */
    for(i = 0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if(FALSE == add_om_voice_vlan_oui[i].status)
        {
            continue;
        }

        if(TRUE == ADD_OM_IS_MATCH_OUI(add_om_voice_vlan_oui[i].oui,
                                       add_om_voice_vlan_oui[i].mask, oui))
        {
            ADD_OM_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
    }

    for(i = 0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if(FALSE == add_om_voice_vlan_oui[i].status)
        {
            size_t len;
            add_om_voice_vlan_oui[i].status = TRUE;
            memcpy(add_om_voice_vlan_oui[i].oui, oui, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(add_om_voice_vlan_oui[i].mask, mask, SYS_ADPT_MAC_ADDR_LEN);

            if(NULL == description)
            {
                memset(add_om_voice_vlan_oui[i].description, 0, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1);
            }
            else
            {
                len = strlen((char*)description);
                if(len > SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN)
                    len = SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN;
                memcpy(add_om_voice_vlan_oui[i].description, description, len);
                add_om_voice_vlan_oui[i].description[len] = 0;
            }

            ADD_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_RemoveOui
 * ---------------------------------------------------------------------
 * PURPOSE: Remove a recognised OUI.
 * INPUT:   oui                - oui address.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_RemoveOui(UI8_T *oui, UI8_T *mask)
{
    int i;

    if((NULL == oui) || (NULL == mask))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();

    for(i = 0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        /* For user define OUI only */
        if((TRUE == add_om_voice_vlan_oui[i].status) &&
           (0 == memcmp(add_om_voice_vlan_oui[i].oui, oui, SYS_ADPT_MAC_ADDR_LEN)) &&
           (0 == memcmp(add_om_voice_vlan_oui[i].mask, mask, SYS_ADPT_MAC_ADDR_LEN)))
        {
            add_om_voice_vlan_oui[i].status = FALSE;

            ADD_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get recognised OUI.
 * INPUT:   oui               -- oui address (key).
 * OUTPUT:  oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 *          description       -- the descritption of the OUI MAC address,
 *                               the max length is 30 character.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetOui(UI8_T *oui, UI8_T *mask, UI8_T *description)
{
    int i;

    if((NULL == oui) || (NULL == mask) || (NULL == description))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();

    for(i=0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if((TRUE == add_om_voice_vlan_oui[i].status) &&
           (0    == memcmp(add_om_voice_vlan_oui[i].oui, oui, SYS_ADPT_MAC_ADDR_LEN)))
        {
            size_t desc_len;

            memcpy(oui, add_om_voice_vlan_oui[i].oui, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(mask, add_om_voice_vlan_oui[i].mask, SYS_ADPT_MAC_ADDR_LEN);
            desc_len = strlen((char*)add_om_voice_vlan_oui[i].description);
            memcpy(description, add_om_voice_vlan_oui[i].description, desc_len);
            description[desc_len] = 0;

            ADD_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


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
BOOL_T ADD_OM_GetNextOui(UI8_T *oui, UI8_T *mask, UI8_T *description)
{
    UI8_T key_of_get_first[SYS_ADPT_MAC_ADDR_LEN];
    int i = 0;

    memset(key_of_get_first, 255, sizeof(key_of_get_first));

    ADD_OM_ENTER_CRITICAL_SECTION();

    /* get first oui ? */
    if(memcmp(oui, key_of_get_first, SYS_ADPT_MAC_ADDR_LEN) != 0)
    {
        /* find get next oui index */
        for(i = 0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
        {
            if((TRUE == add_om_voice_vlan_oui[i].status) &&
               (0 == memcmp(add_om_voice_vlan_oui[i].oui, oui, SYS_ADPT_MAC_ADDR_LEN)))
            {
                ++i;
                break;
            }
        }
    }

    for(; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if(TRUE == add_om_voice_vlan_oui[i].status)
        {
            memcpy(oui, add_om_voice_vlan_oui[i].oui, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(mask, add_om_voice_vlan_oui[i].mask, SYS_ADPT_MAC_ADDR_LEN);
            strncpy(description, add_om_voice_vlan_oui[i].description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);
            description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN] = '\0';

            ADD_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set aging time-out value of Voice VLAN.
 * INPUT:   timeout value, unit is minute.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_SetVoiceVlanAgingTime(UI32_T timeout)
{
    /* check range */
    if((timeout < MIN_voiceVlanAgingTime) || (timeout > MAX_voiceVlanAgingTime))
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    add_om_voice_vlan_timeout = timeout;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_GetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get aging timeout value of Voice VLAN.
 * INPUT:   None.
 * OUTPUT:  timeout value, unit is minute.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_OM_GetVoiceVlanAgingTime(UI32_T *timeout)
{
    if(NULL == timeout)
    {
        return FALSE;
    }

    ADD_OM_ENTER_CRITICAL_SECTION();
    *timeout = add_om_voice_vlan_timeout;
    ADD_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

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
void ADD_OM_ShowVoiceVlanStatusDebug(UI32_T from_lport, UI32_T to_lport)
{
    const  size_t PROTOCOL_LEN = 30;
    UI8_T  sz_per_protocol[PROTOCOL_LEN];
    UI32_T lport;

    if(from_lport > to_lport)
        return;

    ADD_OM_ENTER_CRITICAL_SECTION();

    if(from_lport == 0)
    {
        printf("\r\nVoice VLAN State  : %s", (add_om_voice_vlan_id!=VAL_voiceVlanEnabledId_disabled)?"Enabled":"Disabled");
        printf("\r\nVoice VLAN ID     : %ld", add_om_voice_vlan_id);
        printf("\r\nVoice VLAN Timeout: %ld minutes", add_om_voice_vlan_timeout);
        printf("\r\nCurrent Port State");
        printf("\r\nPort Mode   Security Protocol JoinVLAN DisjoinWhen(%2ds) CntOfOuiLearned Pri", ADD_TYPE_TIMER_EVENT_OF_SEC);
        printf("\r\n---- ------ -------- -------- -------- ---------------- --------------- ---");

        from_lport = 1;
    }

    if(to_lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        to_lport = SYS_ADPT_TOTAL_NBR_OF_LPORT;

   for(lport = from_lport; lport <= to_lport; ++lport)
   {
       memset(sz_per_protocol, 0, PROTOCOL_LEN);
       if(add_om_voice_vlan_port_table[lport-1].protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
           strcat(sz_per_protocol, "OUI ");

       if(add_om_voice_vlan_port_table[lport-1].protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
           strcat(sz_per_protocol, "LLDP");

       if(add_om_voice_vlan_port_table[lport-1].protocol & ADD_TYPE_DISCOVERY_PROTOCOL_DHCP)
           strcat(sz_per_protocol, "DHCP");

       printf("\r\n%-4ld %-6s %-8s %-9s %-8s %-15ld %-15ld %-5d",
           lport,
           (add_om_voice_vlan_port_table[lport-1].mode==VAL_voiceVlanPortMode_auto)?"Auto":(add_om_voice_vlan_port_table[lport-1].mode==VAL_voiceVlanPortMode_manual)?"Manual":"None",
           (add_om_voice_vlan_port_table[lport-1].security_state==VAL_voiceVlanPortSecurity_enabled)?"Enabled":"Disabled",
           sz_per_protocol,
           (add_om_voice_vlan_port_table[lport-1].join_state==ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)?"LLDP":
           (add_om_voice_vlan_port_table[lport-1].join_state==ADD_TYPE_DISCOVERY_PROTOCOL_OUI)?"OUI":
           (add_om_voice_vlan_port_table[lport-1].join_state==0)?"No": "O/L",

           add_om_voice_vlan_port_table[lport-1].disjoin_when,
           add_om_voice_vlan_port_table[lport-1].oui_learned_count,
           add_om_voice_vlan_port_table[lport-1].priority
       );
    }
    ADD_OM_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_OM_ShowOui
 * ---------------------------------------------------------------------
 * PURPOSE: List all recognised OUI address.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_OM_ShowOui()
{
    int i;
    UI8_T *oui;
    UI8_T *mask;

    ADD_OM_ENTER_CRITICAL_SECTION();

    printf("\r\nstatus oui               mask              discription");
    for(i=0; i<SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI; ++i)
    {
        if(add_om_voice_vlan_oui[i].status == TRUE)
        {
            oui = &add_om_voice_vlan_oui[i].oui[0];
            mask = &add_om_voice_vlan_oui[i].mask[0];
            printf("\r\n TRUE  %02X-%02X-%02X-%02X-%02X-%02X %02X-%02X-%02X-%02X-%02X-%02X %s",
                oui[0],oui[1],oui[2],oui[3],oui[4],oui[5],
                mask[0],mask[1],mask[2],mask[3],mask[4],mask[5],
                add_om_voice_vlan_oui[i].description);
        }
        else
            printf("\r\n FALSE ----------------- ----------------- -----------");
    }

    ADD_OM_LEAVE_CRITICAL_SECTION();
}

#endif  /* #if (SYS_CPNT_ADD == TRUE) */

