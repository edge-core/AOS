#include <memory.h>
#include "psec_om.h"
#include "sysfun.h"

typedef struct
{
    UI32_T  mac_count;
    UI32_T  trap_time;	
    UI32_T  last_intrusion_time;
    UI8_T   last_intrusion_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   status;          /* VAL_portSecPortStatus_disabled/VAL_portSecPortStatus_enabled */
    UI8_T   action;
    BOOL_T   action_active;  /* TRUE/FALSE: means if action has been invoked or nots */
}PSEC_OM_PortEntry_T;

static UI32_T psec_om_semid;
static UI32_T psec_om_semid_orig_priority;
static PSEC_OM_PortEntry_T psec_om_entries[SYS_ADPT_TOTAL_NBR_OF_LPORT];


/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for PSEC objects
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   none
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_OM_InitSemaphore(void)
{
    if(SYSFUN_OK!=SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO,&psec_om_semid))
        return FALSE;
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_OM_EnterCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Enter critical region before a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   none
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_OM_EnterCriticalRegion(void)
{
    psec_om_semid_orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(psec_om_semid);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_OM_LeaveCriticalRegion
 * ------------------------------------------------------------------------
 * PURPOSE  :   Leave critical region after a task invokes the spanning
 *              tree objects.
 * INPUT    :   none
 *              none
 * OUTPUT   :   none
 * RETURN   :   TRUE/FALSE
 * NOTE     :   none
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_OM_LeaveCriticalRegion(void)
{
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(psec_om_semid, psec_om_semid_orig_priority);
    return TRUE;
}

void PSEC_OM_InitiateSystemResources()
{
    UI32_T i;

    PSEC_OM_InitSemaphore();

    memset(psec_om_entries, 0, sizeof(psec_om_entries));

    for (i=0; i < SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        psec_om_entries[i].status = (UI8_T)SYS_DFLT_PORT_SECURITY_STATUS;
        psec_om_entries[i].mac_count = SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT;
        psec_om_entries[i].action = (UI8_T)SYS_DFLT_PORT_SECURITY_ACTION_STATUS;
        psec_om_entries[i].action_active = (BOOL_T)(SYS_DFLT_PORT_SECURITY_STATUS==VAL_portSecPortStatus_disabled?FALSE:TRUE);
    }
}

BOOL_T PSEC_OM_IsValidLport(UI32_T lport)
{
    return (0 < lport && lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT) ?
        TRUE : FALSE;
}

PSEC_OM_PortEntry_T* PSEC_OM_GetPortEntry(UI32_T lport)
{
    if (!PSEC_OM_IsValidLport(lport))
        return NULL;

    return &psec_om_entries[lport -1];
}

BOOL_T PSEC_OM_GetPortSecurityStatus(UI32_T lport, UI32_T *status_p)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *status_p = (UI32_T)pe_p->status;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetPortSecurityStatus(UI32_T lport, UI32_T status)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    if (VAL_portSecPortStatus_disabled != status
        && VAL_portSecPortStatus_enabled != status
        )
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->status = (UI8_T)status;
    pe_p->action_active = FALSE;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetPortSecurityActionActive(UI32_T lport, UI32_T *state_p)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *state_p = (UI32_T)pe_p->action_active;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetPortSecurityActionActive(UI32_T lport, UI32_T state)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    if (TRUE!= state
        && FALSE != state
        )
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->action_active = (BOOL_T)state;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetLastIntrusionTime(UI32_T lport, UI32_T *seconds)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *seconds = pe_p->last_intrusion_time;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetLastIntrusionTime(UI32_T lport, UI32_T seconds)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->last_intrusion_time = seconds;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetLastIntrusionMac(UI32_T lport, UI8_T *mac)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p || NULL == mac)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    memcpy(mac, pe_p->last_intrusion_mac, SYS_ADPT_MAC_ADDR_LEN);

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetTrapTime(UI32_T lport, UI32_T *seconds)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *seconds = pe_p->trap_time;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetTrapTime(UI32_T lport, UI32_T seconds)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->trap_time = seconds;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}


BOOL_T PSEC_OM_SetLastIntrusionMac(UI32_T lport, UI8_T *mac)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p || NULL==mac)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    memcpy(pe_p->last_intrusion_mac, mac, SYS_ADPT_MAC_ADDR_LEN);

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetIntrusionAction(UI32_T lport, UI32_T *action_p)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    if (NULL == action_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *action_p = (UI32_T)pe_p->action;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetIntrusionAction(UI32_T lport, UI32_T action)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    if (VAL_portSecAction_none != action
        && VAL_portSecAction_shutdown != action
        && VAL_portSecAction_trap != action
        && VAL_portSecAction_trapAndShutdown != action
        )
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->action = (UI8_T)action;
    pe_p->action_active = FALSE;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_GetMaxMacCount(UI32_T lport, UI32_T *mac_count_p)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    *mac_count_p = pe_p->mac_count;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_SetMaxMacCount(UI32_T lport, UI32_T mac_count)
{
    PSEC_OM_PortEntry_T *pe_p = PSEC_OM_GetPortEntry(lport);

    if (NULL == pe_p)
    {
        return FALSE;
    }

    if (mac_count < MIN_portSecMaxMacCount
        || MAX_portSecMaxMacCount < mac_count
        )
    {
        return FALSE;
    }

    PSEC_OM_EnterCriticalRegion();

    pe_p->mac_count = mac_count;
    pe_p->action_active = FALSE;

    PSEC_OM_LeaveCriticalRegion();
    return TRUE;
}

BOOL_T PSEC_OM_IsEnabled(UI32_T lport)
{
    UI32_T  status;
    UI32_T  max_mac_count;

    if (FALSE == PSEC_OM_GetPortSecurityStatus(lport, &status)
        || FALSE == PSEC_OM_GetMaxMacCount(lport, &max_mac_count)
        )
    {
        return FALSE;
    }

    return (status == VAL_portSecPortStatus_enabled || 0 < max_mac_count) ?
        TRUE : FALSE;
}

const char *PSEC_OM_StrPortSecurityStatus(UI32_T psec_status)
{
    static const char *ary[] =
        {
            "Unknown",
            "Enabled",
            "Disabled"
        };

    if (VAL_portSecPortStatus_enabled != psec_status
        && VAL_portSecPortStatus_disabled != psec_status
        )
    {
        return ary[0];
    }

    return ary[psec_status];
}

const char *PSEC_OM_StrIntrusionAction(UI32_T action)
{
    static const char *ary[] =
        {
            "Unknown",
            "None",
            "Trap",
            "ShutDown",
            "Trap+ShutDown"
        };

    if (action < VAL_portSecAction_none
        || VAL_portSecAction_trapAndShutdown < action
        )
    {
        return ary[0];
    }

    return ary[action];
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_OM_HandleIPCReqMsg
 * ------------------------------------------------------------------------
 * PURPOSE  :   Handle the ipc request message for PSEC OM.
 * INPUT    :   msgbuf_p     -- input request ipc message buffer
 * OUTPUT   :   msgbuf_p     -- output response ipc message buffer
 * RETURN   :   TRUE, success; else return FALSE.
 * NOTE     :   none
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    PSEC_OM_IPCMsg_T  *msg_p;
    BOOL_T  need_resp = TRUE;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (PSEC_OM_IPCMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding PSEC_OM function
     */
    switch (msg_p->type.cmd)
    {
        case PSEC_OM_IPC_GET_MAX_MAC_COUNT:
        {
            PSEC_OM_IPCMsg_GetPortSecurityMacCount_T *data_p = PSEC_OM_MSG_DATA(msgbuf_p);
            PSEC_OM_MSG_RETVAL(msgbuf_p) = PSEC_OM_GetMaxMacCount(data_p->ifindex, &data_p->mac_count);
            msgbuf_p->msg_size = PSEC_OM_GET_MSGBUFSIZE(PSEC_OM_IPCMsg_GetPortSecurityMacCount_T);
            break;
        }

        default:
            printf("%s(): Invalid cmd.\n", __FUNCTION__);
            /* Unknown command. There is no way to idntify whether this
             * ipc message need or not need a response. If we response to
             * a asynchronous msg, then all following synchronous msg will
             * get wrong responses and that might not easy to debug.
             * If we do not response to a synchronous msg, the requester
             * will be blocked forever. It should be easy to debug that
             * error.
             */
            need_resp = FALSE;
    }

    return need_resp;
} /* End of PSEC_OM_HandleIPCReqMsg */

