/* MODULE NAME: cmgr.c
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*     This file used to be the interface to all CSC.
*     When a action need be execused after some CSC execused firstly,
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - Give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitation}
*    {4. Any performance limitation}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    03/03/2008    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2008
*/

/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <string.h>
#include "sys_module.h"
#include "sysfun.h"
#include "cmgr.h"
#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif
#include "lldp_pmgr.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "leaf_es3626a.h"
#include "backdoor_mgr.h"
#include "netcfg_pmgr_ip.h"
#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_pmgr.h"
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    #include "oam_pmgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */
#define BACKDOOR_OPEN   1

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if BACKDOOR_OPEN
static void CMGR_Backdoor_Main(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

static UI32_T cmgr_sem_id;

/* MACRO FUNCTIONS DECLARACTION
 */
#define CMGR_LOCK()          SYSFUN_TakeSem(cmgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define CMGR_UNLOCK()        SYSFUN_GiveSem(cmgr_sem_id)

#define CMGR_PREPARE_MSG(ARGUMENT_TYPE)                                 \
    enum {MESSAGE_SIZE = CMGR_GET_MSG_SIZE(ARGUMENT_TYPE)};             \
    UI8_T           ipc_buf[SYSFUN_SIZE_OF_MSG(MESSAGE_SIZE)] = {0};    \
    SYSFUN_Msg_T    *ipc_msg_p = (SYSFUN_Msg_T*)ipc_buf;                \
    CMGR_IpcMsg_T   *cmgr_msg_p = (CMGR_IpcMsg_T*)ipc_msg_p->msg_buf;   \
    UI32_T          *msgq_key_p;                                        \
    SYSFUN_MsgQ_T   ipc_msgq_handle;                                    \
                                                                        \
    ipc_msg_p->cmd = SYS_MODULE_CMGR;                                   \
    ipc_msg_p->msg_size = MESSAGE_SIZE;

#define CMGR_SEND_MSG(KEY_LIST)                                     \
    msgq_key_p = KEY_LIST;                                          \
    while (*msgq_key_p != 0)                                        \
    {                                                               \
        if (SYSFUN_GetMsgQ(*msgq_key_p, SYSFUN_MSGQ_BIDIRECTIONAL,  \
                &ipc_msgq_handle) != SYSFUN_OK)                     \
        {                                                           \
            printf("%s: failed to get req message queue (%lu)\r\n", \
                __func__, *msgq_key_p);                             \
            return;                                                 \
        }                                                           \
        if (SYSFUN_SendRequestMsg(ipc_msgq_handle, ipc_msg_p,       \
                SYSFUN_TIMEOUT_WAIT_FOREVER,                        \
                SYSFUN_SYSTEM_EVENT_IPCMSG, 0, NULL) != SYSFUN_OK)  \
        {                                                           \
            printf("%s: failed to send req message\r\n", __func__); \
            return;                                                 \
        }                                                           \
        SYSFUN_ReleaseMsgQ(ipc_msgq_handle);                        \
        msgq_key_p++;                                               \
    }

#define CMGR_RECEIVE_MSG(KEY_LIST)                                      \
    msgq_key_p = KEY_LIST;                                              \
    while (*msgq_key_p != 0)                                            \
    {                                                                   \
        if (SYSFUN_GetMsgQ(*msgq_key_p, SYSFUN_MSGQ_BIDIRECTIONAL,      \
                &ipc_msgq_handle) != SYSFUN_OK)                         \
        {                                                               \
            printf("%s: failed to get res message queue (%lu)\r\n",     \
                __func__, *msgq_key_p);                                 \
            return;                                                     \
        }                                                               \
        if (SYSFUN_ReceiveResponseMsg(ipc_msgq_handle,                  \
                SYSFUN_TIMEOUT_WAIT_FOREVER, 0, ipc_msg_p) !=           \
                SYSFUN_OK)                                              \
        {                                                               \
            printf("%s: failed to receive res message\r\n", __func__);  \
            return;                                                     \
        }                                                               \
        SYSFUN_ReleaseMsgQ(ipc_msgq_handle);                            \
        msgq_key_p++;                                                   \
    }

/* EXPORTED SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function attach the resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CMGR_AttachSystemResources()
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_CMGR, &cmgr_sem_id);
    return;
}/* End of CMGR_AttachSystemResources*/

/* -------------------------------------------------------------------------
 * FUNCTION NAME - UI_MGR_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function initiates the resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void CMGR_InitiateSystemResources(void)
{
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_CMGR, &cmgr_sem_id);

    return;
}/* End of CMGR_InitiateProcessResources() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - CMGR_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void CMGR_Create_InterCSC_Relation(void)
{
#if BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("cmgr", SYS_BLD_CMGR_GROUP_IPCMSGQ_KEY, CMGR_Backdoor_Main);
#endif
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the port admin status after call the lldp and cfm  if status is disabled.
 *          otherwise directly call the swctrl
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetPortAdminStatus(UI32_T lport, UI32_T status)
{
    return CMGR_SetPortStatus(lport, status == VAL_ifAdminStatus_up, SWCTRL_PORT_STATUS_SET_BY_CFG);
}/*End of CMGR_SetPortAdminStatus*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_SetPortStatus
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the port admin status
 * INPUT   : ifindex        -- which port to set
 *           status         -- TRUE to be up; FALSE to be down
 *           reason         -- indicates role of caller
 *                             bitmap of SWCTRL_PORT_STATUS_SET_BY_XXX
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : CSCs that is in the layer higher than CMGR may call CMGR_SetPortStatus,
 *           CSCs that is in the layer lower than CMGR should call
 *           SYS_CALLBACK_MGR_SetPortStatusCallback() to notify CMGR to set port status
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetPortStatus(UI32_T ifindex, BOOL_T status, UI32_T reason)
{
    UI32_T shutdown_reason = 0;
    BOOL_T ret = FALSE;

    CMGR_LOCK();

    if (!status)
    {
        SWCTRL_POM_GetPortStatus(ifindex, &shutdown_reason);

        /* !!NOTE: Also need to add the corresponding PMGR init function call
         *         in the sys_mgmt_proc if you add the PMGR of new CSC here.
         *
         *         Otherwise setting ifAdminStatus via SNMP will cause
         *         exception in sys_mgmt_group.
         */
        if (shutdown_reason == 0)
        {
#if (SYS_CPNT_CFM == TRUE)
            CFM_PMGR_ProcessPortAdminDisable(ifindex);
#endif
#if (SYS_CPNT_LLDP == TRUE)
            LLDP_PMGR_SetPortAdminDisable(ifindex);
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
            EFM_OAM_PMGR_SetPortAdminDisable(ifindex);
#endif
        }
    }

    ret = SWCTRL_PMGR_SetPortStatus(ifindex, status, reason);

    CMGR_UNLOCK();

    return ret;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - CMGR_SetIpv6AddrAutoconfig
 *-------------------------------------------------------------------------
 * PURPOSE : The function set the ipv6 address autoconfig
 * INPUT   : ifindex        -- which vlan interface to set
 *           status         -- TRUE to be enable; FALSE to be disable
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T CMGR_SetIpv6AddrAutoconfig(UI32_T ifindex, BOOL_T status)
{
    BOOL_T ret = FALSE;
    CMGR_LOCK();

    if(TRUE == status)
    {

        /* when enable, we have to notify ipcfg first, then dhcpv6 */
        if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_IPv6AddrAutoconfigEnable(ifindex))
            ret = FALSE;
        else
            ret = TRUE;
#if (SYS_CPNT_DHCPV6 == TRUE)
        DHCPv6_PMGR_SignalAddrModeChanged(ifindex, status);
#endif

    }
    else
    {
        /* when disable, we have to notify dhcpv6 first, then ipcfg,
         * or it will fail to send out packet
         */
#if (SYS_CPNT_DHCPV6 == TRUE)
        DHCPv6_PMGR_SignalAddrModeChanged(ifindex, status);
#endif
        if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_IPv6AddrAutoconfigDisable(ifindex))
            ret = FALSE;
        else
            ret = TRUE;
    }

    CMGR_UNLOCK();
    return ret;
}


#if BACKDOOR_OPEN
static UI32_T CMGR_Backdoor_GetU32(char *s, UI32_T def_val)
{
    char buf[11];
    unsigned long val;

    BACKDOOR_MGR_Printf("%s: ", s);
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);

    if (*buf == 0)
    {
        val = def_val;
        BACKDOOR_MGR_Printf("%lu\n", val);
    }
    else if (sscanf(buf, "%lu", &val) != 1)
    {
        val = def_val;
        BACKDOOR_MGR_Printf(" ==> %lu\n", val);
    }
    else
    {
        BACKDOOR_MGR_Printf("\n");
    }

    return val;
}

static BOOL_T CMGR_Backdoor_GetBool(char *s, BOOL_T def_val)
{
    char buf[2];
    unsigned long val;

    BACKDOOR_MGR_Printf("%s (1:TRUE 0:FALSE): ", s);
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);

    if (*buf == 0)
    {
        val = def_val;
        BACKDOOR_MGR_Printf("%lu ==> %s\n", val, val?"TRUE":"FALSE");
    }
    else if (sscanf(buf, "%lu", &val) != 1)
    {
        val = def_val;
        BACKDOOR_MGR_Printf(" ==> %s\n", val?"TRUE":"FALSE");
    }
    else
    {
        val = !!val;
        BACKDOOR_MGR_Printf(" ==> %s\n", val?"TRUE":"FALSE");
    }
    return val;
}

static void CMGR_Backdoor_Main(void)
{
    int ch;
    while (1)
    {
        BACKDOOR_MGR_Printf("0. Exit\n");
        BACKDOOR_MGR_Printf("1. CMGR_SetPortStatus\n");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c(%d)\n", isprint(ch)?ch:'?', ch);

        switch (ch)
        {
            case '0':
                return;
            case '1':
                CMGR_SetPortStatus(
                    CMGR_Backdoor_GetU32("ifindex", 1),
                    CMGR_Backdoor_GetBool("status", TRUE),
                    CMGR_Backdoor_GetU32("reason", 1));
                break;
        }
        BACKDOOR_MGR_Printf("\n");
    }
}
#endif /* BACKDOOR_OPEN */

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
void CMGR_NotifyVlanChange(UI32_T *key_list, UI32_T cmd, UI32_T vlan_ifindex,
        UI32_T vlan_status, BOOL_T existed, BOOL_T merged)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32_bool_bool)

    cmgr_msg_p->type.cmd = cmd;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1 = vlan_ifindex;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2 = vlan_status;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 = existed;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 = merged;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyVlanMemberChange(UI32_T *key_list, UI32_T cmd,
        UI32_T vlan_ifindex, UI32_T lport, UI32_T vlan_status, BOOL_T existed,
        BOOL_T merged)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32_ui32_bool_bool)

    cmgr_msg_p->type.cmd = cmd;
    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_1 = vlan_ifindex;
    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_2 = lport;
    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_ui32_3 = vlan_status;
    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_1 = existed;
    cmgr_msg_p->data.arg_ui32_ui32_ui32_bool_bool.arg_bool_2 = merged;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyL3VlanDestroy(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T vlan_status)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_L3_VLAN_DESTROY;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = vlan_ifindex;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = vlan_status;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyPortVlanChange(UI32_T *key_list, UI32_T lport)
{
    CMGR_PREPARE_MSG(arg_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_PORT_VLAN_CHANGE;
    cmgr_msg_p->data.arg_ui32 = lport;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyPortVlanModeChange(UI32_T *key_list, UI32_T cmd, UI32_T lport,
        UI32_T mode)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = cmd;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = lport;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = mode;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyPvidChange(UI32_T *key_list, UI32_T lport, UI32_T old_pvid,
        UI32_T new_pvid)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_PVID_CHANGE;
    cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_1 = lport;
    cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_2 = old_pvid;
    cmgr_msg_p->data.arg_ui32_ui32_ui32.arg_ui32_3 = new_pvid;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyL3IfOperStatusChange(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T oper_status)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_L3_IF_OPER_STATUS_CHANGE;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = vlan_ifindex;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = oper_status;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyVlanNameChange(UI32_T *key_list, UI32_T vid)
{
    CMGR_PREPARE_MSG(arg_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_VLAN_NAME_CHANGE;
    cmgr_msg_p->data.arg_ui32 = vid;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyProtocolVlanChange(UI32_T *key_list, UI32_T lport)
{
    CMGR_PREPARE_MSG(arg_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_PROTOCOL_VLAN_CHANGE;
    cmgr_msg_p->data.arg_ui32 = lport;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyVlanMemberTagChange(UI32_T *key_list, UI32_T vlan_ifindex,
        UI32_T lport)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_VLAN_MEMBER_TAG_CHANGE;

    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = vlan_ifindex;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = lport;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyXstpPortStateChange(UI32_T *key_list, UI32_T xstid,
        UI32_T lport, BOOL_T xstp_state, BOOL_T xstp_merge)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32_bool_bool)

    cmgr_msg_p->type.cmd = CMGR_IPC_XSTP_PORT_STATE_CHANGE;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_1 = xstid;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_ui32_2 = lport;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_1 = xstp_state;
    cmgr_msg_p->data.arg_ui32_ui32_bool_bool.arg_bool_2 = xstp_merge;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyXstpVersionChange(UI32_T *key_list, UI32_T mode, UI32_T status)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_XSTP_VERSION_CHANGE;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = mode;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = status;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}

void CMGR_NotifyXstpPortTopoChange(UI32_T *key_list, UI32_T xstid, UI32_T lport)
{
    CMGR_PREPARE_MSG(arg_ui32_ui32)

    cmgr_msg_p->type.cmd = CMGR_IPC_XSTP_PORT_TOPO_CHANGE;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_1 = xstid;
    cmgr_msg_p->data.arg_ui32_ui32.arg_ui32_2 = lport;

    CMGR_SEND_MSG(key_list)
    CMGR_RECEIVE_MSG(key_list)
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
