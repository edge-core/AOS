/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_MGR.c
 *
 * PURPOSE: This package provides the service routines to manage VRRP (RFC 2338)
 * NOTES:   The key functions of this module are to provide interfaces for the upper layer
 *          to configure VRRP, update VRRP database information based on the configuration.
 *
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 * -------------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
*/
/*
#include <stdlib.h>
#include <time.h>
#include <sys_bld.h>
#include "vrrp_task.h"
*/
#include <stdio.h>
#include <string.h>
#include <leaf_1213.h>
#include "sys_type.h"
#include "sysfun.h"
#include "l_rstatus.h"
#include "l_stdlib.h"
#include "vlan_mgr.h"
#include "vlan_pom.h"
#include "netcfg_pom_ip.h"
#include "netcfg_type.h"
#include "vlan_lib.h"
#include "vlan_pom.h"
#include "syslog_om.h"
#include "syslog_type.h"
#include "sys_module.h"
#include "syslog_mgr.h"
#include "ip_lib.h"
#include "vrrp_type.h"
#include "vrrp_om.h"
#include "vrrp_vm.h"
#include "vrrp_mgr.h"
#include "vrrp_sys_adpt.h"
#include "stdlib.h"
#include "backdoor_mgr.h"
#include "swctrl_pmgr.h"
#include "netcfg_pmgr_nd.h"
/* LOCAL DATATYPE DECLARATION
 */

#define VrrpOperEntryAddrOwner_true	        1L
#define VrrpOperEntryAddrOwner_false	    2L

#define MAXLINE 255
/* STATIC VARIABLE DECLARATIONS
 */
/*static UI32_T   vrrp_mgr_sem_id;*/
SYSFUN_DECLARE_CSC

/* MACRO FUNCTIONS DECLARACTION
 */
#define VRRP_MGR_CHECK_ENABLE()  if(SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) return FALSE

/* EXPORTED SUBPROGRAM BODIES
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T VRRP_MGR_CreateDefaultOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);
static BOOL_T VRRP_MGR_CheckEntryValid(VRRP_OPER_ENTRY_T *vrrp_oper_entry);
static void VRRP_MGR_CheckTimer(UI32_T pass_time, UI32_T *min_expire_time);
static BOOL_T VRRP_MGR_GetPrimaryIp(UI32_T ifindex, UI8_T *SrcIp);
static BOOL_T VRRP_MGR_SemanticCheck(void *vrrp_oper_entry);
static BOOL_T VRRP_MGR_ChangeParametersPreOperation(VRRP_OPER_ENTRY_T *vrrp_oper_entry);
static BOOL_T VRRP_MGR_ChangeParametersAfterOperation(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T admin_state);
static BOOL_T VRRP_MGR_VrrpOperEntryIsDefault(VRRP_OPER_ENTRY_T *vrrp_oper_entry);
static BOOL_T VRRP_MGR_DecreaseVirtualIpUpCount(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

static void VRRP_MGR_BACKDOOR_OmConfigMenu(void);
static void VRRP_MGR_BACKDOOR_DbgConfigMenu(void);
static void VRRP_MGR_BACKDOOR_TimerConfigMenu(void);

static BOOL_T  vrrpMgrErrorLogFlag[8];
static UI32_T  vrrp_debug_flag;

BOOL_T VRRP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *msgbuf_p)
{
    VRRP_MGR_IPCMsg_T* msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (VRRP_MGR_IPCMsg_T*)msgbuf_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        msg_p->type.result_bool = FALSE;
        msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }
    switch (msg_p->type.cmd)
    {
        case VRRP_MGR_IPC_IPADDRESSSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetIpAddress(msg_p->data.arg_grp1.arg1,
                                      msg_p->data.arg_grp1.arg2,
                                      msg_p->data.arg_grp1.arg3,
                                      msg_p->data.arg_grp1.arg4);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
            break;

        case VRRP_MGR_IPC_PRIMARYIPSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetPrimaryIp(msg_p->data.arg_grp2.arg1,
                                      msg_p->data.arg_grp2.arg2,
                                      msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);;
            break;

        case VRRP_MGR_IPC_IPADDRESSGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextIpAddress(msg_p->data.arg_grp2.arg1,
                                      msg_p->data.arg_grp2.arg2,
                                      msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
            break;

        case VRRP_MGR_IPC_OPERADMINSTATUSSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetOperAdminStatus(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERADMINSTATUSGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetOperAdminStatus(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_OPERAUTHTYPESET:
            msg_p->type.result_ui32 = VRRP_MGR_SetAuthenticationType(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERAUTHTYPEGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetAuthenticationType(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_OPERAUTHKEYSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetAuthenticationKey(msg_p->data.arg_grp4.arg1,
                                      msg_p->data.arg_grp4.arg2,
                                      msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERAUTHKEYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetAuthenticationKey(msg_p->data.arg_grp4.arg1,
                                      msg_p->data.arg_grp4.arg2,
                                      msg_p->data.arg_grp4.arg3);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp4);
            break;

        case VRRP_MGR_IPC_OPERAUTHSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetAuthentication(msg_p->data.arg_grp5.arg1,
                                      msg_p->data.arg_grp5.arg2,
                                      msg_p->data.arg_grp5.arg3,
                                      msg_p->data.arg_grp5.arg4);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_PRIORITYSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetPriority(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_PRIORITYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetPriority(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_PREEMPTIONMODESET:
            msg_p->type.result_ui32 = VRRP_MGR_SetPreemptionMode(msg_p->data.arg_grp6.arg1,
                                      msg_p->data.arg_grp6.arg2,
                                      msg_p->data.arg_grp6.arg3,
                                      msg_p->data.arg_grp6.arg4);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_PREEMPTMODESET:
            msg_p->type.result_ui32 = VRRP_MGR_SetPreemptMode(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_PREEMPTMODEGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetPreemptionMode(msg_p->data.arg_grp6.arg1,
                                      msg_p->data.arg_grp6.arg2,
                                      &(msg_p->data.arg_grp6.arg3),
                                      &(msg_p->data.arg_grp6.arg4));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
            break;

        case VRRP_MGR_IPC_ADVERINTERVALSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetAdvertisementInterval(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_ADVERINTERVALGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetAdvertisementInterval(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;
#if (SYS_CPNT_VRRP_PING == TRUE)
        case VRRP_MGR_IPC_PINGSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetPingStatus(msg_p->data.arg_grp8.arg1);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif
        case VRRP_MGR_IPC_OPERROWSTATUSSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetVrrpOperRowStatus(&(msg_p->data.arg_grp7.arg1),
                                      msg_p->data.arg_grp7.arg2);
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERENTRYSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetVrrpOperEntry(&(msg_p->data.oper_entry));
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERENTRYDEL:
            msg_p->type.result_ui32 = VRRP_MGR_DeleteVrrpOperEntry(&(msg_p->data.oper_entry));
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_OPERENTRYGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextVrrpOperEntry(&(msg_p->data.oper_entry));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
            break;

        case VRRP_MGR_IPC_DEFAULTOPERENTRYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetDefaultVrrpOperEntry(&(msg_p->data.oper_entry));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.oper_entry);
            break;

        case VRRP_MGR_IPC_ASSOCIPADDRENTRYSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetVrrpAssocIpAddrEntry(&(msg_p->data.assoc_ip_addr));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
            break;

        case VRRP_MGR_IPC_ASSOCIPADDRENTRYDEL:
            msg_p->type.result_ui32 = VRRP_MGR_DeleteAssoIpEntry(&(msg_p->data.assoc_ip_addr));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
            break;

        case VRRP_MGR_IPC_ASSOCIPADDRENTRYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetVrrpAssocIpAddrEntry(&(msg_p->data.assoc_ip_addr));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
            break;

        case VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextVrrpAssocIpAddrEntry(&(msg_p->data.assoc_ip_addr));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
            break;

        case VRRP_MGR_IPC_ASSOCIPADDRENTRYGETNEXT_SNMP:
            msg_p->type.result_bool = VRRP_MGR_GetNextVrrpAssocIpAddrEntryBySnmp(&(msg_p->data.assoc_ip_addr));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.assoc_ip_addr);
            break;

        case VRRP_MGR_IPC_OPERIPADDRCOUNTGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetIpAddrCount(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_OPERSTATEGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetOperState(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_OPERPROTOCOLSET:
            msg_p->type.result_ui32 = VRRP_MGR_SetOperProtocol(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      msg_p->data.arg_grp3.arg3);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_OPERPROTOCOLGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetOperProtocol(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_VERSIONNUMBERGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetVersionNumber(&(msg_p->data.version));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.version);
            break;

        case VRRP_MGR_IPC_VRRPSYSSTATISTICSGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetVrrpSysStatistics(
                                          &(msg_p->data.vrrp_router_statistics));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.vrrp_router_statistics);
            break;

        case VRRP_MGR_IPC_VRRPSYSSTATISTICSCLEAR:
            msg_p->type.result_ui32 = VRRP_MGR_ClearVrrpSysStatistics();
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case VRRP_MGR_IPC_VRRPGROUPSTATISTICSCLEAR:
            msg_p->type.result_ui32 = VRRP_MGR_ClearVrrpGroupStatistics(msg_p->data.arg_grp8.arg1,
                                      msg_p->data.arg_grp8.arg2);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
            break;

        case VRRP_MGR_IPC_VRRPGROUPSTATISTICSGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetVrrpGroupStatistics(msg_p->data.arg_grp9.arg1,
                                      msg_p->data.arg_grp9.arg2,
                                      &(msg_p->data.arg_grp9.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
            break;

        case VRRP_MGR_IPC_VRRPGROUPSTATISTICSGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextVrrpGroupStatistics(&(msg_p->data.arg_grp9.arg1),
                                      &(msg_p->data.arg_grp9.arg2),
                                      &(msg_p->data.arg_grp9.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
            break;
        case VRRP_MGR_IPC_RUNNINGPRIORITYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpPriority(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPRIORITYGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpPriority(&(msg_p->data.arg_grp3.arg1),
                                      &(msg_p->data.arg_grp3.arg2),
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGAUTHTYPEGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpAuthType(msg_p->data.arg_grp5.arg1,
                                      msg_p->data.arg_grp5.arg2,
                                      &(msg_p->data.arg_grp5.arg3),
                                      msg_p->data.arg_grp5.arg4);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
            break;

        case VRRP_MGR_IPC_RUNNINGAUTHTYPEGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpAuthType(&(msg_p->data.arg_grp5.arg1),
                                      &(msg_p->data.arg_grp5.arg2),
                                      &(msg_p->data.arg_grp5.arg3),
                                      msg_p->data.arg_grp5.arg4);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
            break;

        case VRRP_MGR_IPC_RUNNINGADVERINTGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpAdverInt(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGADVERINTGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpAdverInt(&(msg_p->data.arg_grp3.arg1),
                                      &(msg_p->data.arg_grp3.arg2),
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPREEMPTGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpPreemptMode(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPREEMPTGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpPreemptMode(&(msg_p->data.arg_grp3.arg1),
                                      &(msg_p->data.arg_grp3.arg2),
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpPreemptDelay(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPREEMPTDELAYGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpPreemptDelay(&(msg_p->data.arg_grp3.arg1),
                                      &(msg_p->data.arg_grp3.arg2),
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPROTOCOLGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpProtocol(msg_p->data.arg_grp3.arg1,
                                      msg_p->data.arg_grp3.arg2,
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGPROTOCOLGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpProtocol(&(msg_p->data.arg_grp3.arg1),
                                      &(msg_p->data.arg_grp3.arg2),
                                      &(msg_p->data.arg_grp3.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
            break;

        case VRRP_MGR_IPC_RUNNINGASSOCIPGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningVrrpAssocIp(msg_p->data.arg_grp2.arg1,
                                      msg_p->data.arg_grp2.arg2,
                                      (UI8_T *) & (msg_p->data.arg_grp2.arg3));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
            break;

        case VRRP_MGR_IPC_RUNNINGASSOCIPGETNEXT:
            msg_p->type.result_ui32 = VRRP_MGR_GetNextRunningVrrpAssocIp(&(msg_p->data.arg_grp2.arg1),
                                      &(msg_p->data.arg_grp2.arg2),
                                      msg_p->data.arg_grp2.arg3);
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
            break;
#if (SYS_CPNT_VRRP_PING == TRUE)
        case VRRP_MGR_IPC_RUNNINGPINGSTATUSGET:
            msg_p->type.result_ui32 = VRRP_MGR_GetRunningPingStatus(&(msg_p->data.arg_grp8.arg1));
            msgbuf_p->msg_size = VRRP_MGR_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
            break;
#endif
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_ui32 = VRRP_TYPE_RESULT_FAIL;
            msgbuf_p->msg_size = VRRP_MGR_IPCMSG_TYPE_SIZE;
            break;
    }

    return TRUE;
}
/* end , wang.tong */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for vrrp_om and mgr.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_Initiate_System_Resources(void)
{
    VRRP_OM_Init();
    VRRP_VM_Init();
    vrrp_debug_flag = 0;
    return TRUE;
} /* end of VRRP_MGR_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_RegisterCallbackFunction
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will register the callback functions
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : 1. This api registers the callback functions provided in the NETCFG
 *               file for being notified by Netcfg when rif transfers from down
 *               to up or from up to down.
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_RegisterCallbackFunction(void)
{
    return TRUE;
} /* VRRP_MGR_RegisterCallbackFunction() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Enable VRRP operation while in master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VRRP_MGR_EnterMasterMode(void)
{
    VRRP_TYPE_GlobalEntry_T global_config;
    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    /* set database to default value
     */
    memset(&global_config, 0, sizeof(global_config));
#if (SYS_CPNT_VRRP_PING == TRUE)
    global_config.vrrpPingStatus = SYS_DFLT_VRRP_PING_STATUS;
#endif
    VRRP_OM_SetVrrpGlobalConfig(&global_config);
    return;
} /* VRRP_MGR_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the VRRP operation while in slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_EnterSlaveMode(void)
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();

    return;
} /* VRRP_MGR_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Disable the VRRP operation while in transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_EnterTransitionMode(void)
{
    /* clear database
     */
    VRRP_OM_ClearDatabase();

    /* set mgr in transition mode
     */
    SYSFUN_ENTER_TRANSITION_MODE();
    return;
} /* VRRP_MGR_EnterTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set operation mode into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
void VRRP_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    return;
}  /* End of VRRP_MGR_SetTransitionMode */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperationMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP operation mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : SYS_TYPE_STACKING_Master_MODE
 *            SYS_TYPE_STACKING_SLAVE_MODE
 *            SYS_TYPE_STACKING_TRANSITION_MODE
 * NOTES    : none
 *-------------------------------------------------------------------------- */
SYS_TYPE_Stacking_Mode_T VRRP_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ip address for the specific vrrp on the interface (RFC2338, p.13)
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 *            row_status: VAL_vrrpAssoIpAddrRowStatus_notInService or
 *                        VAL_vrrpAssoIpAddrRowStatus_destroy to delete this ip address
 *                        VAL_vrrpAssoIpAddrRowStatus_createAndGo or
 *                        VAL_vrrpAssoIpAddrRowStatus_active to add this ip address
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/TRUE/FALSE/
 * NOTES    : modified by Aris to fix EPR:ES3628C-PoE-20-00181
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr, UI32_T row_status)
{
    VRRP_OPER_ENTRY_T       vrrp_oper_entry;
    VRRP_ASSOC_IP_ENTRY_T   vrrp_assoc_ip_entry;
    UI32_T					result = 0;
    UI32_T                  ip_addr_trans;
    NETCFG_TYPE_InetRifConfig_T rif_config_p;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_entry;

    if (ip_addr == 0)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }

    /*Check if IP is valid address*/
    if (IP_LIB_IsMulticastIp(ip_addr) == TRUE)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }
    if (IP_LIB_IsLoopBackIp(ip_addr) == TRUE)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }
    if (IP_LIB_IsBroadcastIp(ip_addr) == TRUE)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }
    if (IP_LIB_IsTestingIp(ip_addr) == TRUE)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }

    IP_LIB_ArraytoUI32(ip_addr, &ip_addr_trans);
    if ((L_STDLIB_Ntoh32(ip_addr_trans) & 0x000000ff) == 0)
    {
        return VRRP_TYPE_IP_ADDRESSES_NOT_VAILD;
    }
    /*End IP add check*/

    if ((row_status < VAL_vrrpAssoIpAddrRowStatus_active) || (row_status > VAL_vrrpAssoIpAddrRowStatus_destroy))
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }
    VRRP_MGR_CHECK_ENABLE();
    /*Donny.Li add: Check whether the ip has been exist*/
    if ((row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
            (row_status == VAL_vrrpAssoIpAddrRowStatus_active))
    {
        vrrp_oper_entry.ifindex = 0;
        vrrp_oper_entry.vrid = 0;
        while (VRRP_OM_GetNextVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_ACCESS_SUCCESS)
        {
            vrrp_assoc_ip_entry.ifindex = vrrp_oper_entry.ifindex;
            vrrp_assoc_ip_entry.vrid = vrrp_oper_entry.vrid;
            memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            while (VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry) == VRRP_TYPE_OK)
            {
                if (memcmp(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH) == 0)
                {
                    return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_EXIST;
                }
            }
        }
    }
    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        /* if vrrp_oper_entry does not exist, create a default oper entry*/
        if ((row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) || (row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
        {
            return VRRP_TYPE_PARAMETER_ERROR;
        }
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    vlan_entry.dot1q_vlan_index = (UI16_T)if_index;
    if (VLAN_POM_GetVlanEntry(&vlan_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    /* vrrp_oper_entry exists, */
    /* 1. no associated IP address exists */
    if (vrrp_oper_entry.ip_addr_count == 0)
    {
        if ((row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                (row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
        {
            return VRRP_TYPE_PARAMETER_ERROR;
        }
        vrrp_assoc_ip_entry.ifindex = if_index;
        vrrp_assoc_ip_entry.vrid = vrid;
        memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
        vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_createAndGo;
        if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        vrrp_oper_entry.ifindex = if_index;
        vrrp_oper_entry.vrid = vrid;
        result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        vrrp_oper_entry.ip_addr_count++;
        if (VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }


        memcpy(rif_config_p.addr.addr, ip_addr, VRRP_IP_ADDR_LENGTH);
        rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;

        if (NETCFG_POM_IP_GetRifFromExactIp(&rif_config_p) == NETCFG_TYPE_OK)
        {
            if (rif_config_p.ifindex == if_index)
            {
                if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                    vrrp_oper_entry.virtual_ip_up_count++;
                /* become virtual ip onwer and vrrp master */
                vrrp_oper_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                vrrp_oper_entry.master_priority = vrrp_oper_entry.priority;
                vrrp_oper_entry.master_advertise_int = vrrp_oper_entry.advertise_interval;
                vrrp_oper_entry.master_down_int = (3 * vrrp_oper_entry.master_advertise_int) + ((256 - vrrp_oper_entry.master_priority) / 256);
                vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_true;

            }
        }
        else
        {
            memset(&rif_config_p, 0, sizeof(rif_config_p));
            memcpy(rif_config_p.addr.addr, ip_addr, VRRP_IP_ADDR_LENGTH);
            rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
            if (NETCFG_POM_IP_GetRifFromIp(&rif_config_p) == NETCFG_TYPE_OK)
            {
                if (rif_config_p.ifindex == if_index)
                {
                    if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                        vrrp_oper_entry.virtual_ip_up_count++;
                    vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;
                }
            }
            else
                vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_OK)
        {
            if (vrrp_oper_entry.ip_addr_count > 0)
            {
                vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_active;
                if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }
            }
        }
    }
    else /* 2. there is at least one associated IP address existing*/
    {
        vrrp_assoc_ip_entry.ifindex = if_index;
        vrrp_assoc_ip_entry.vrid = vrid;
        memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
        result = VRRP_OM_SearchVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry);

        /* Such IP address already exists in OM */
        if (result == VRRP_TYPE_OK)
        {
            if ((row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
                    (row_status == VAL_vrrpAssoIpAddrRowStatus_active))
            {
                return VRRP_TYPE_OK;
            }
            else if ((row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                     (row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
            {
                /* if there is only one associated ip, we must shutdown vrrp first */
                if ((vrrp_oper_entry.ip_addr_count == 1) && (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_up))
                {
                    if (VRRP_VM_ShutDown(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid) == FALSE)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
                    }

                    if (vrrp_oper_entry.owner == VrrpOperEntryAddrOwner_true)
                    {
                        vrrp_oper_entry.owner = 0;
                        vrrp_oper_entry.priority = SYS_DFLT_VRRP_PRIORITY;
                    }

                    vrrp_oper_entry.admin_state = VAL_vrrpOperAdminState_down;

                    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                    if (result != VRRP_TYPE_OK)
                    {
                        return result;
                    }
                }
                vrrp_assoc_ip_entry.ifindex = if_index;
                vrrp_assoc_ip_entry.vrid = vrid;
                memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
                vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_notInService;

                result = VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                vrrp_oper_entry.ifindex = if_index;
                vrrp_oper_entry.vrid = vrid;
                result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
                }

                vrrp_oper_entry.ip_addr_count--;
                if (VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
                }

                memset(&rif_config_p, 0, sizeof(rif_config_p));
                memcpy(rif_config_p.addr.addr, ip_addr, VRRP_IP_ADDR_LENGTH);
                rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
                if (NETCFG_POM_IP_GetRifFromIp(&rif_config_p) == NETCFG_TYPE_OK)
                {
                    if (rif_config_p.ifindex == if_index)
                    {
                        if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                            vrrp_oper_entry.virtual_ip_up_count--;
                    }
                }
                result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                if (vrrp_oper_entry.ip_addr_count == 0)
                {
                    L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_notInService, &vrrp_oper_entry.row_status,
                                  VRRP_MGR_SemanticCheck, (void*)&vrrp_oper_entry);

                    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                    if (result != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                }
                if (vrrp_oper_entry.oper_state == VAL_vrrpOperState_master)
                {
                    IP_LIB_ArraytoUI32(ip_addr, &ip_addr_trans);
                }
            } /* delete the associated ip address */
        }
        else if (result == VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST)
        {
            if ((row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                    (row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }

            if ((row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
                    (row_status == VAL_vrrpAssoIpAddrRowStatus_active))
            {

                /* shutdown and delete virtual ip */
                if ((vrrp_oper_entry.ip_addr_count == 1) && (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_up))
                {
                    if (VRRP_VM_ShutDown(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid) == FALSE)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
                    }

                    if (vrrp_oper_entry.owner == VrrpOperEntryAddrOwner_true)
                    {
                        vrrp_oper_entry.owner = 0;
                        vrrp_oper_entry.priority = SYS_DFLT_VRRP_PRIORITY;
                    }

                    vrrp_oper_entry.admin_state = VAL_vrrpOperAdminState_down;

                    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                    if (result != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                }

                /* change ip address */
                memset(&vrrp_assoc_ip_entry, 0, sizeof(vrrp_assoc_ip_entry));
                vrrp_assoc_ip_entry.ifindex = if_index;
                vrrp_assoc_ip_entry.vrid = vrid;

                if (VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry) == VRRP_TYPE_OK)
                {
                    vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_notInService;
                    if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                }

                vrrp_assoc_ip_entry.ifindex = if_index;
                vrrp_assoc_ip_entry.vrid = vrid;
                memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
                vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_createAndGo;
                if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_OPER_ENTRY_NOT_EXIST;
                }


                memcpy(rif_config_p.addr.addr, ip_addr, VRRP_IP_ADDR_LENGTH);
                rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
                if (NETCFG_POM_IP_GetRifFromExactIp(&rif_config_p) == NETCFG_TYPE_OK)
                {   /* interface is the owner of virtual ip */
                    if (rif_config_p.ifindex == if_index)
                    {
                        vrrp_oper_entry.pre_priority = vrrp_oper_entry.priority;
                        vrrp_oper_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                        vrrp_oper_entry.master_priority = vrrp_oper_entry.priority;
                        vrrp_oper_entry.master_advertise_int = vrrp_oper_entry.advertise_interval;
                        vrrp_oper_entry.master_down_int = (3 * vrrp_oper_entry.master_advertise_int) + ((256 - vrrp_oper_entry.master_priority) / 256);
                        vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_true;
                    }
                }
                else
                {
                    memset(&rif_config_p, 0, sizeof(rif_config_p));
                    memcpy(rif_config_p.addr.addr, ip_addr, VRRP_IP_ADDR_LENGTH);
                    rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
                    if (NETCFG_POM_IP_GetRifFromIp(&rif_config_p) == NETCFG_TYPE_OK)
                    {   /* interface is not the owner of virtual ip, but it has the same subnet ip */
                        if (rif_config_p.ifindex == if_index)
                        {
                            vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;
                        }
                    }
                    else
                        vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;
                }
                result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }
                if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_OK)
                {
                    if (vrrp_oper_entry.ip_addr_count > 0)
                    {
                        vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_active;
                        if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
                        {
                            return VRRP_TYPE_INTERNAL_ERROR;
                        }
                    }
                }

            }


        } /* No such IP address exists in OM */
        else
        {
            return VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST;
        }
    }

    if (row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo ||
        row_status == VAL_vrrpAssoIpAddrRowStatus_active)
    {
        if (NETCFG_TYPE_OK != NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry(
                    if_index,
                    ip_addr_trans,
                    SYS_ADPT_MAC_ADDR_LEN,
                    vrrp_oper_entry.virtual_mac))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    else if (row_status == VAL_vrrpAssoIpAddrRowStatus_destroy ||
             row_status == VAL_vrrpAssoIpAddrRowStatus_notInService)
    {
        if (NETCFG_TYPE_OK != NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry(
                    if_index,
                    ip_addr_trans,
                    SYS_ADPT_MAC_ADDR_LEN,
                    vrrp_oper_entry.virtual_mac))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetIpAddress() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPrimaryIp
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the primary ip address for the spicific vrrp on the interface (RFC2338, p.13)
 *            for MIB
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPrimaryIp(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }
    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    memcpy(vrrp_oper_entry.primary_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }


    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetPrimaryIp() */



/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next ip address for vrrp (RFC2338, p.13)
 * INPUT    : if_index: the specific interface
 *            vrid:     the specific vrrp group id
 *            ip_addr:  the buffer to get the ip address
 * OUTPUT   : next associated ip address of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr)
{
    VRRP_ASSOC_IP_ENTRY_T  vrrp_assoc_ip_entry;
    UI32_T result;

    if (ip_addr == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    vrrp_assoc_ip_entry.ifindex = if_index;
    vrrp_assoc_ip_entry.vrid = vrid;
    memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);
    result = VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry);
    if (result == VRRP_TYPE_OK)
        memcpy(ip_addr, vrrp_assoc_ip_entry.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
    return result;
} /* VRRP_MGR_GetNextIpAddress() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Set.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. Row status must be up before set enabled(1).
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T vrrp_admin_status)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            pre_admin_state, result;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_entry;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
        return VRRP_TYPE_INTERNAL_ERROR;
        }

        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);

        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    /* If we want to set admin to up, we must set acitve to row status first! */
    if (vrrp_admin_status == VAL_vrrpOperAdminState_up)
    {
        L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_active, &vrrp_oper_entry.row_status,
                      VRRP_MGR_SemanticCheck, (void*)&vrrp_oper_entry);
        if (vrrp_oper_entry.row_status != VAL_vrrpOperRowStatus_active)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    pre_admin_state = vrrp_oper_entry.admin_state;
    vrrp_oper_entry.admin_state = vrrp_admin_status;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    vlan_entry.dot1q_vlan_index = (UI16_T)if_index;
    if (VLAN_POM_GetVlanEntry(&vlan_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if ((pre_admin_state == VAL_vrrpOperAdminState_down) &&
            (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_up) &&
            (vrrp_oper_entry.virtual_ip_up_count > 0) &&
            (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up))
    {
        if (VRRP_MGR_GetPrimaryIp(vrrp_oper_entry.ifindex, vrrp_oper_entry.primary_ip_addr) == FALSE)
            memset(vrrp_oper_entry.primary_ip_addr, 0, VRRP_IP_ADDR_LENGTH);

        if (VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }            
        if (VRRP_VM_Startup(if_index, vrid) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    if ((pre_admin_state == VAL_vrrpOperAdminState_up) &&
            (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_down))
    {
        if (VRRP_VM_ShutDown(if_index, vrid) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        if (vrrp_oper_entry.ip_addr_count == 0)
        { /* if no associated ip exists, row status must be in notReady state */
            if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
            {
                return VRRP_TYPE_INTERNAL_ERROR;
            }
            L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_notInService, &vrrp_oper_entry.row_status,
                          VRRP_MGR_SemanticCheck, (void*)&vrrp_oper_entry);
            if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
            {
                return VRRP_TYPE_INTERNAL_ERROR;
            }
            result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
            if (result != VRRP_TYPE_OK)
            {
                return result;
            }
        }
    }

    if(VRRP_OM_VrrpOperEntryCount() == 1)
    {
        SWCTRL_PMGR_TrapVrrpToCpu(TRUE);
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetOperAdminStatus() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperAdminStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : vrrp_admin_status -
              VAL_vrrpOperAdminState_up 1L \
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid
 * OUTPUT   : vrrp admin status for the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_admin_status)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T result;

    if (vrrp_admin_status == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    *vrrp_admin_status = vrrp_oper_entry.admin_state;

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetOperAdminStatus() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication type for each interface (RFC2338, p.13)
 * INPUT    : if_Index, vrid,
 *            auth_type -
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_auth_type)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }
    if (vrrp_oper_auth_type != VAL_vrrpOperAuthType_noAuthentication
            && vrrp_oper_auth_type != VAL_vrrpOperAuthType_simpleTextPassword)
    {
        /*VAL_vrrpOperAuthType_ipAuthenticationHeader not yet supported*/
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    vrrp_oper_entry.auth_type = vrrp_oper_auth_type;
    memset(vrrp_oper_entry.auth_key, 0, sizeof(vrrp_oper_entry.auth_key));
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetAuthenticationType() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication type for each interface (RFC2338, p.13)
 * INPUT    : if_Index, vrid
 * OUTPUT   : authentication type of the specific ifindex and vrid
 *               VAL_vrrpOperAuthType_noAuthentication	     1L
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_auth_type)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T   result;

    if (vrrp_oper_auth_type == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    *vrrp_oper_auth_type = vrrp_oper_entry.auth_type;
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetAuthenticationType() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication key for each interface (RFC2338, p.13)
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    if (vrrp_oper_auth_key == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }
    if (strlen((char *)vrrp_oper_auth_key) > VRRP_MGR_AUTH_DATA_LENGTH)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    memset(vrrp_oper_entry.auth_key, 0, sizeof(vrrp_oper_entry.auth_key));
    memcpy(vrrp_oper_entry.auth_key, vrrp_oper_auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetAuthenticationKey() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication key for each interface (RFC2338, p.13)
 * INPUT    : ifindex, vrid
 * OUTPUT   : authentication key of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_auth_key == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    memset(vrrp_oper_auth_key, 0, sizeof(*vrrp_oper_auth_key));
    memcpy(vrrp_oper_auth_key, vrrp_oper_entry.auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetAuthenticationKey() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAuthentication
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the authentication data is set successfully.
              Otherwise, return false.
 * INPUT    : ifIndex, vrid,
 *            auth_type -
              	VAL_vrrpOperAuthType_noAuthentication	1L \
              	VAL_vrrpOperAuthType_simpleTextPassword	2L \
              	VAL_vrrpOperAuthType_ipAuthenticationHeader 3L
              auth_key
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetAuthentication(UI32_T if_index, UI8_T vrid, UI32_T auth_type, UI8_T *auth_key)
{

    if (auth_key == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    if (VRRP_TYPE_OK != VRRP_MGR_SetAuthenticationType(if_index, vrid, auth_type))
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (auth_type == VAL_vrrpOperAuthType_simpleTextPassword)
    {
        if (VRRP_TYPE_OK != VRRP_MGR_SetAuthenticationKey(if_index, vrid, auth_key))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetAuthentication() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : MIN_vrrpOperPriority	0L
 *            MAX_vrrpOperPriority	255L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPriority(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_priority)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    if ((vrrp_oper_priority < SYS_ADPT_MIN_VRRP_PRIORITY) ||
            (vrrp_oper_priority > SYS_ADPT_MAX_VRRP_PRIORITY))
    {
        /*configurable priority range from 1~254*/
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.owner == VrrpOperEntryAddrOwner_true)
    {
        return VRRP_TYPE_SET_OWNER_PRIORITY;
    }

    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    vrrp_oper_entry.priority = vrrp_oper_priority;
    vrrp_oper_entry.pre_priority = vrrp_oper_priority;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetPriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : vrrp priority of the specific ifindex oand vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetPriority(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_priority)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_priority == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }
    *vrrp_oper_priority = vrrp_oper_entry.priority;
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetPriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid, preempt mode and delay time for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode, UI32_T delay)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T	result = 0;
    UI32_T 	pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_PARAMETER_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    pre_admin_state = vrrp_oper_entry.admin_state;

    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if ((delay > SYS_ADPT_MAX_VRRP_PREEMPT_DELAY) ||
            (delay < SYS_ADPT_MIN_VRRP_PREEMPT_DELAY))
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    vrrp_oper_entry.preempt_mode = vrrp_oper_preempt_mode;
    vrrp_oper_entry.preempt_delay = delay;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetPreemptionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp (RFC2338, p.14) for MIB
 * INPUT    : ifindex, vrid, preempt mode for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetPreemptMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();
    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    pre_admin_state = vrrp_oper_entry.admin_state;

    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {

        return VRRP_TYPE_INTERNAL_ERROR;
    }
    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {

        return VRRP_TYPE_INTERNAL_ERROR;
    }

    vrrp_oper_entry.preempt_mode = vrrp_oper_preempt_mode;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {

        return result;
    }
    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {

        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
}/* VRRP_MGR_SetPreemptMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the preemption mode for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : preempt mode and preempt delay time for this group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_preempt_mode, UI32_T *delay)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_preempt_mode == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    *vrrp_oper_preempt_mode = vrrp_oper_entry.preempt_mode;
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetPreemptionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the advertisement interval for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid, advertisement interval for the group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : SYS_ADPT_MIN_VIRTUAL_ROUTER_ADVER_INT   1
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_ADVER_INT 255
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_advertise_interval)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result, pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    vrrp_oper_entry.advertise_interval = vrrp_oper_advertise_interval;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetAdvertisementInterval() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the advertisement interval for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : advertisement interval of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_advertise_interval)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_advertise_interval == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    *vrrp_oper_advertise_interval = vrrp_oper_entry.advertise_interval;
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetAdvertisementInterval() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpOperRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry->if_index, vrrp_oper_entry->vrid,
 *            action    row_status to be set to the specific vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : action --
 *            VAL_vrrpOperRowStatus_active	      1L
 *            VAL_vrrpOperRowStatus_notInService    2L
 *            VAL_vrrpOperRowStatus_notReady        3L
 *            VAL_vrrpOperRowStatus_createAndGo	  4L
 *            VAL_vrrpOperRowStatus_createAndWait   5L
 *            VAL_vrrpOperRowStatus_destroy	      6L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpOperRowStatus(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T action)
{
    BOOL_T exist=FALSE;

    if(vrrp_oper_entry == NULL)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    if(VRRP_TYPE_OK == VRRP_OM_GetVrrpOperEntry(vrrp_oper_entry))
    {
        exist = TRUE;
    }
    else
    {
        exist = FALSE;
    }

    switch(action)
    {
        case VAL_vrrpOperRowStatus_createAndWait:
        case VAL_vrrpOperRowStatus_createAndGo:
            if(exist)
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }

            VRRP_MGR_CreateDefaultOperEntry(vrrp_oper_entry);

            return VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry);
        break;
        case VAL_vrrpOperRowStatus_active:
        case VAL_vrrpOperRowStatus_notInService:
            if(!exist)
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }

            vrrp_oper_entry->row_status = action;

            return VRRP_MGR_SetVrrpOperEntry(vrrp_oper_entry);
        break;
        case VAL_vrrpOperRowStatus_destroy:
            if(!exist)
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }

            return VRRP_MGR_DeleteVrrpOperEntry(vrrp_oper_entry);
        break;
        case VAL_vrrpOperRowStatus_notReady:
        default:
            return VRRP_TYPE_PARAMETER_ERROR;
    }
} /* VRRP_MGR_SetVrrpOperRowStatus */

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_MGR_SetPingStatus(UI32_T ping_status)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    VRRP_TYPE_GlobalEntry_T global_config;

    if ((ping_status != VRRP_TYPE_PING_STATUS_ENABLE) &&
            (ping_status != VRRP_TYPE_PING_STATUS_DISABLE))
        return VRRP_TYPE_PARAMETER_ERROR;

    /* shutdown all vrrp operation entry
     */
    memset(&vrrp_oper_entry, 0, sizeof(vrrp_oper_entry));
    while (VRRP_TYPE_ACCESS_SUCCESS == VRRP_OM_GetNextVrrpOperEntry(&vrrp_oper_entry))
    {
        if (VAL_vrrpOperAdminState_up == vrrp_oper_entry.admin_state)
        {
            if (!VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry))
            {
                /* print debug message
                 */
                VRRP_BD(DBG, "pre-operation failed.");
                continue;
            }
        }
    }

    /* store configuration to database
     */
    memset(&global_config, 0, sizeof(global_config));
    VRRP_OM_GetVrrpGlobalConfig(&global_config);

    global_config.vrrpPingStatus = ping_status;
    VRRP_OM_SetVrrpGlobalConfig(&global_config);

    /* restart all vrrp operation entry
     */
    memset(&vrrp_oper_entry, 0, sizeof(vrrp_oper_entry));
    while (VRRP_TYPE_ACCESS_SUCCESS == VRRP_OM_GetNextVrrpOperEntry(&vrrp_oper_entry))
    {
        if (!VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, VAL_vrrpOperAdminState_up))
        {
            /* print debug message
             */
            VRRP_BD(DBG, "post-operation failed.");
            continue;
        }
    }

    return VRRP_TYPE_OK;
}

#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SemanticCheck
 *--------------------------------------------------------------------------
 * PURPOSE  : check wether vrrp oper row status can be "active"
 * INPUT    : vrrp_oper_entry->vrid, vrrp_oper_entry->ip_addr_count
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : row status can be made to "active" only
 *            1. vrid has been set
 *            2. at least one active associated ip address set
 *-------------------------------------------------------------------------- */
static BOOL_T VRRP_MGR_SemanticCheck(void *vrrp_oper_entry)
{
    VRRP_OPER_ENTRY_T  *current_vrrp_oper_entry;
    current_vrrp_oper_entry = (VRRP_OPER_ENTRY_T*)vrrp_oper_entry;

    if (NULL == vrrp_oper_entry)
        return FALSE;

    if ((current_vrrp_oper_entry->vrid > SYS_ADPT_MAX_VRRP_ID) ||
            (current_vrrp_oper_entry->vrid < SYS_ADPT_MIN_VRRP_ID))
        return FALSE;

    if (current_vrrp_oper_entry->ip_addr_count <= 0)
        return FALSE;

    return TRUE;
} /* VRRP_MGR_SemanticCheck() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_OPER_ENTRY_T exist_vrrp_oper_entry;
    UI32_T            pre_admin_state = VAL_vrrpOperAdminState_down;
    UI32_T            result;

    if (vrrp_oper_entry == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_MGR_CheckEntryValid(vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    exist_vrrp_oper_entry.ifindex = vrrp_oper_entry->ifindex;
    exist_vrrp_oper_entry.vrid = vrrp_oper_entry->vrid;
    result = VRRP_OM_GetVrrpOperEntry(&exist_vrrp_oper_entry);

    if (result == VRRP_TYPE_OK)
    {
        pre_admin_state = exist_vrrp_oper_entry.admin_state;
    }
    else
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&exist_vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    exist_vrrp_oper_entry.admin_state = vrrp_oper_entry->admin_state;
    exist_vrrp_oper_entry.priority = vrrp_oper_entry->priority;
    exist_vrrp_oper_entry.pre_priority = vrrp_oper_entry->priority;
    exist_vrrp_oper_entry.auth_type = vrrp_oper_entry->auth_type;
    memset(exist_vrrp_oper_entry.auth_key, 0, sizeof(exist_vrrp_oper_entry.auth_key));
    memcpy(exist_vrrp_oper_entry.auth_key, vrrp_oper_entry->auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
    exist_vrrp_oper_entry.advertise_interval = vrrp_oper_entry->advertise_interval;
    exist_vrrp_oper_entry.preempt_mode = vrrp_oper_entry->preempt_mode;
    exist_vrrp_oper_entry.oper_protocol = vrrp_oper_entry->oper_protocol;
    L_RSTATUS_Fsm(vrrp_oper_entry->row_status, &exist_vrrp_oper_entry.row_status,
                  VRRP_MGR_SemanticCheck, (void*)&exist_vrrp_oper_entry);
    result = VRRP_OM_SetVrrpOperEntry(&exist_vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if ((pre_admin_state == VAL_vrrpOperAdminState_down) &&
            (exist_vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_up) &&
            (exist_vrrp_oper_entry.virtual_ip_up_count > 0))
    {
        if (VRRP_MGR_GetPrimaryIp(vrrp_oper_entry->ifindex, vrrp_oper_entry->primary_ip_addr) == FALSE)
            memset(vrrp_oper_entry->primary_ip_addr, 0, VRRP_IP_ADDR_LENGTH);

        if (VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        if (VRRP_VM_Startup(vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    if ((pre_admin_state == VAL_vrrpOperAdminState_up) &&
            (exist_vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_down))
    {
        if (VRRP_VM_ShutDown(vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetVrrpOperEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_ASSOC_IP_ENTRY_T vrrp_assoc_ip_entry;
    UI32_T ipaddr;

    if (vrrp_oper_entry == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    VRRP_VM_ShutDown(vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid);

    memset(&vrrp_assoc_ip_entry, 0, sizeof(vrrp_assoc_ip_entry));
    vrrp_assoc_ip_entry.ifindex = vrrp_oper_entry->ifindex;
    vrrp_assoc_ip_entry.vrid = vrrp_oper_entry->vrid;
    memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
    while (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry))
    {
        IP_LIB_ArraytoUI32(vrrp_assoc_ip_entry.assoc_ip_addr, &ipaddr);

        if (NETCFG_TYPE_OK != NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry(
                    vrrp_assoc_ip_entry.ifindex,
                    ipaddr,
                    SYS_ADPT_MAC_ADDR_LEN,
                    vrrp_oper_entry->virtual_mac))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    if (VRRP_OM_DeleteVrrpOperEntry(vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if(VRRP_OM_VrrpOperEntryCount() == 0)
    {
        SWCTRL_PMGR_TrapVrrpToCpu(FALSE);
    }
    return VRRP_TYPE_OK;
} /* VRRP_MGR_DeleteVrrpOperEntry() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP operation entry
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : next ifindex, next vrid, next vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    UI32_T	result = 0;

    if (vrrp_oper_entry == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if (((vrrp_oper_entry->vrid == 0) && (vrrp_oper_entry->ifindex == 0)) ||
            ((vrrp_oper_entry->vrid <= SYS_ADPT_MAX_VRRP_ID) && (vrrp_oper_entry->vrid >= SYS_ADPT_MIN_VRRP_ID)))
    {
        result = VRRP_OM_GetNextVrrpOperEntry(vrrp_oper_entry);
        if (result != VRRP_TYPE_ACCESS_SUCCESS)
        {
            return result;
        }
    }
    else
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetNextVrrpOperEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetDefaultVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the default VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *		      VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetDefaultVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    if (vrrp_oper_entry == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if ((vrrp_oper_entry->vrid > SYS_ADPT_MAX_VRRP_ID) ||
            (vrrp_oper_entry->vrid < SYS_ADPT_MIN_VRRP_ID))
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (VRRP_MGR_CreateDefaultOperEntry(vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetDefaultVrrpOperEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_SetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_OPER_ENTRY_T       vrrp_oper_entry;
    VRRP_ASSOC_IP_ENTRY_T   vrrp_assoc_ip_entry;
    UI32_T					result = 0;
    UI32_T                  ip_addr_trans;
    NETCFG_TYPE_InetRifConfig_T rif_config_p;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_entry;

    if (NULL == vrrp_assoc_ip_addr)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /*2004.5.24 Willy add */
    /*Check if IP is valid address*/
    if (IP_LIB_IsMulticastIp(vrrp_assoc_ip_addr->assoc_ip_addr) == TRUE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (IP_LIB_IsLoopBackIp(vrrp_assoc_ip_addr->assoc_ip_addr) == TRUE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (IP_LIB_IsBroadcastIp(vrrp_assoc_ip_addr->assoc_ip_addr) == TRUE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    if (IP_LIB_IsTestingIp(vrrp_assoc_ip_addr->assoc_ip_addr) == TRUE)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    IP_LIB_ArraytoUI32(vrrp_assoc_ip_addr->assoc_ip_addr, &ip_addr_trans);
    if ((L_STDLIB_Ntoh32(ip_addr_trans) & 0x000000ff) == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /*End IP add check*/
    /*end Willy add */

    if ((vrrp_assoc_ip_addr->row_status < VAL_vrrpAssoIpAddrRowStatus_active) ||
            (vrrp_assoc_ip_addr->row_status > VAL_vrrpAssoIpAddrRowStatus_destroy))
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
            (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_active))
    {
        vrrp_oper_entry.ifindex = 0;
        vrrp_oper_entry.vrid = 0;
        while (VRRP_OM_GetNextVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_ACCESS_SUCCESS)
        {
            vrrp_assoc_ip_entry.ifindex = vrrp_oper_entry.ifindex;
            vrrp_assoc_ip_entry.vrid = vrrp_oper_entry.vrid;
            memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            while (VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry) == VRRP_TYPE_OK)
            {
                if (memcmp(vrrp_assoc_ip_entry.assoc_ip_addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH) == 0)
                {
                    if ((vrrp_assoc_ip_entry.ifindex == vrrp_assoc_ip_addr->ifindex) &&
                            (vrrp_assoc_ip_entry.vrid == vrrp_assoc_ip_addr->vrid) &&
                            (vrrp_assoc_ip_entry.row_status == vrrp_assoc_ip_addr->row_status))
                    {
                        return VRRP_TYPE_OK;
                    }

                    if ((vrrp_assoc_ip_entry.ifindex != vrrp_assoc_ip_addr->ifindex) ||
                            (vrrp_assoc_ip_entry.vrid != vrrp_assoc_ip_addr->vrid))
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }
                }
            }
        }
    }
    vrrp_oper_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
    vrrp_oper_entry.vrid = vrrp_assoc_ip_addr->vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        /* if vrrp_oper_entry does not exist, create a default oper entry*/
        if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) || (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
        {
            return VRRP_TYPE_PARAMETER_ERROR;
        }

        return VRRP_TYPE_INTERNAL_ERROR;

    }

    vlan_entry.dot1q_vlan_index = (UI16_T)vrrp_assoc_ip_addr->ifindex;
    if (VLAN_POM_GetVlanEntry(&vlan_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    /* vrrp_oper_entry exists, */
    /* 1. no associated IP address exists */
    if (vrrp_oper_entry.ip_addr_count == 0)
    {
        if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        vrrp_assoc_ip_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
        vrrp_assoc_ip_entry.vrid = vrrp_assoc_ip_addr->vrid;
        memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_createAndGo;

        if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        vrrp_oper_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
        vrrp_oper_entry.vrid = vrrp_assoc_ip_addr->vrid;
        result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }

        vrrp_oper_entry.ip_addr_count++;

        if (VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        IP_LIB_ArraytoUI32(vrrp_assoc_ip_addr->assoc_ip_addr, &ip_addr_trans);
        memcpy(rif_config_p.addr.addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;

        if (NETCFG_POM_IP_GetRifFromExactIp(&rif_config_p) == NETCFG_TYPE_OK)
        {
            if (rif_config_p.ifindex == vrrp_assoc_ip_addr->ifindex)
            {
                if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                    vrrp_oper_entry.virtual_ip_up_count++;

                vrrp_oper_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                vrrp_oper_entry.master_priority = vrrp_oper_entry.priority;
                vrrp_oper_entry.master_advertise_int = vrrp_oper_entry.advertise_interval;
                vrrp_oper_entry.master_down_int = (3 * vrrp_oper_entry.master_advertise_int) + ((256 - vrrp_oper_entry.master_priority) / 256);
                vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_true;
                IP_LIB_ArraytoUI32(vrrp_assoc_ip_addr->assoc_ip_addr, &ip_addr_trans);
            }
        }
        else
        {
            memset(&rif_config_p, 0, sizeof(rif_config_p));
            memcpy(rif_config_p.addr.addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
            rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
            if (NETCFG_POM_IP_GetRifFromIp(&rif_config_p) == NETCFG_TYPE_OK)
            {
                if (rif_config_p.ifindex == vrrp_assoc_ip_addr->ifindex)
                {
                    if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                        vrrp_oper_entry.virtual_ip_up_count++;
                    vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;
                }
            }
            else
                vrrp_oper_entry.owner = VrrpOperEntryAddrOwner_false;

        }

        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
        if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_OK)
        {
            if (vrrp_oper_entry.ip_addr_count > 0)
            {
                vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_active;
                if (VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }
            }
        }
    }
    else /* 2. there is at least one associated IP address existing*/
    {
        vrrp_assoc_ip_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
        vrrp_assoc_ip_entry.vrid = vrrp_assoc_ip_addr->vrid;
        memcpy(vrrp_assoc_ip_entry.assoc_ip_addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        result = VRRP_OM_SearchVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry);

        /* Such IP address already exists in OM */
        if (result == VRRP_TYPE_OK)
        {
            if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo) ||
                    (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_active))
            {
                return VRRP_TYPE_OK;
            }
            else if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                     (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
            {
                /* if there is only one associated ip, we must shutdown vrrp first */
                if ((vrrp_oper_entry.ip_addr_count == 1) && (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_up))
                {
                    if (VRRP_VM_ShutDown(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid) == FALSE)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }

                    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                    {
                        return VRRP_TYPE_INTERNAL_ERROR;
                    }

                    if (vrrp_oper_entry.owner == VrrpOperEntryAddrOwner_true)
                    {
                        vrrp_oper_entry.owner = 0;
                        vrrp_oper_entry.priority = SYS_DFLT_VRRP_PRIORITY;
                    }

                    vrrp_oper_entry.admin_state = VAL_vrrpOperAdminState_down;

                    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                    if (result != VRRP_TYPE_OK)
                    {
                        return result;
                    }
                }
                vrrp_assoc_ip_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
                vrrp_assoc_ip_entry.vrid = vrrp_assoc_ip_addr->vrid;
                memcpy(vrrp_assoc_ip_entry.assoc_ip_addr,  vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
                vrrp_assoc_ip_entry.row_status = VAL_vrrpAssoIpAddrRowStatus_notInService;

                result = VRRP_OM_SetVrrpAssoIpAddrEntry(&vrrp_assoc_ip_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return result;
                }

                vrrp_oper_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
                vrrp_oper_entry.vrid = vrrp_assoc_ip_addr->vrid;
                result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return result;
                }

                vrrp_oper_entry.ip_addr_count--;
                if (VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
                {
                    return VRRP_TYPE_INTERNAL_ERROR;
                }

                memset(&rif_config_p, 0, sizeof(rif_config_p));
                memcpy(rif_config_p.addr.addr, vrrp_assoc_ip_addr->assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
                rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;
                if (NETCFG_POM_IP_GetRifFromIp(&rif_config_p) == NETCFG_TYPE_OK)
                {
                    if (rif_config_p.ifindex == vrrp_assoc_ip_addr->ifindex)
                    {
                        if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
                            vrrp_oper_entry.virtual_ip_up_count--;
                    }
                }

                result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                if (result != VRRP_TYPE_OK)
                {
                    return result;
                }

                if (vrrp_oper_entry.ip_addr_count == 0)
                {
                    L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_notInService, &vrrp_oper_entry.row_status,
                                  VRRP_MGR_SemanticCheck, (void*)&vrrp_oper_entry);

                    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
                    if (result != VRRP_TYPE_OK)
                    {
                        return result;
                    }

                    if (VRRP_MGR_VrrpOperEntryIsDefault(&vrrp_oper_entry) == TRUE)
                    {
                        if(!VRRP_OM_DeleteVrrpOperEntry(&vrrp_oper_entry))
                            return VRRP_TYPE_INTERNAL_ERROR;
                    }
                }
            } /* delete the associated ip address */
        }
        else if (result == VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST)
        {
            if ((vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService) ||
                    (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_destroy))
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }
            /*Because there only can set one associated ip address, so always return false, Donny.Li add*/

            return VRRP_TYPE_INTERNAL_ERROR;

        }
        else
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    if (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_createAndGo ||
        vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_active)
    {
        if (NETCFG_TYPE_OK != NETCFG_PMGR_ND_AddRouterRedundancyProtocolIpNetToMediaEntry(
                    vrrp_assoc_ip_addr->ifindex,
                    ip_addr_trans,
                    SYS_ADPT_MAC_ADDR_LEN,
                    vrrp_oper_entry.virtual_mac))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }
    else if (vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_destroy ||
             vrrp_assoc_ip_addr->row_status == VAL_vrrpAssoIpAddrRowStatus_notInService)
    {
        if (NETCFG_TYPE_OK != NETCFG_PMGR_ND_DeleteRouterRedundancyProtocolIpNetToMediaEntry(
                    vrrp_assoc_ip_addr->ifindex,
                    ip_addr_trans,
                    SYS_ADPT_MAC_ADDR_LEN,
                    vrrp_oper_entry.virtual_mac))
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetVrrpAssocIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{

    if (vrrp_assoc_ip_addr == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    if (VRRP_OM_DeleteAssoIpEntry(vrrp_assoc_ip_addr) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_DeleteAssoIpEntry */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_OM_GetVrrpAssoIpAddrEntry(vrrp_assoc_ip_addr) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetVrrpAssocIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetNextVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_TYPE_OK != VRRP_OM_GetNextVrrpAssoIpAddress(vrrp_assoc_ip_addr))
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetNextVrrpAssocIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpAssocIpAddrEntryBySnmp
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : TRUE/FALSE
 * NOTES    :(1)use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *              vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *           (2)This API is used by snmp to getnext AssocIpAddrEntry through
 *              whole system
 *-------------------------------------------------------------------------- */
BOOL_T VRRP_MGR_GetNextVrrpAssocIpAddrEntryBySnmp(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr)
{
    VRRP_OPER_ENTRY_T oper_entry;

    VRRP_MGR_CHECK_ENABLE();

    /* check null pointer */
    if (NULL == vrrp_assoc_ip_addr)
        return FALSE;

    if (VRRP_OM_GetNextVrrpAssoIpAddress(vrrp_assoc_ip_addr) != VRRP_TYPE_OK)
    {
        /* if there's no associated ip address entry with specified (ifindex,vrid),
         * continue to getnext operEntry
         */
        memset(&oper_entry, 0, sizeof(oper_entry));
        oper_entry.ifindex = vrrp_assoc_ip_addr->ifindex;
        oper_entry.vrid = vrrp_assoc_ip_addr->vrid;

        while (VRRP_TYPE_ACCESS_SUCCESS == VRRP_OM_GetNextVrrpOperEntry(&oper_entry))
        {
            vrrp_assoc_ip_addr->ifindex = oper_entry.ifindex;
            vrrp_assoc_ip_addr->vrid = oper_entry.vrid;

            memset(vrrp_assoc_ip_addr->assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            if (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(vrrp_assoc_ip_addr))
            {
                return TRUE;
            }
        }
        return FALSE;
    }
    return TRUE;
} /* VRRP_MGR_GetNextVrrpAssocIpAddrEntry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the number of IP addresses for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : ip address count of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_MGR_GetIpAddrCount(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_ip_addr_count)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_ip_addr_count == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    *vrrp_oper_ip_addr_count = vrrp_oper_entry.ip_addr_count;
    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetIpAddrCount() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperState
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton gets the operation state of the specific vrid and ifindex
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper state of the specific vrrp group -
 *            VAL_vrrpOperState_initialize       1L
 *            VAL_vrrpOperState_backup           2L
 *            VAL_vrrpOperState_master           3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : The administrative status requested by management for VRRP.  The
 *            value enabled(1) indicates that specify VRRP virtual router should
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router
 *            is not operated. This object affects all VRRP Applicant and Registrar
 *            state machines.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperState(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_state)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;


    if (vrrp_oper_state == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    *vrrp_oper_state = vrrp_oper_entry.oper_state;

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetOperState() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Set operation protocol for the specific ifindex and vrid
 * INPUT    : ifIndex  ,  vrid, operation protocol
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_SetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_protocol)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T	result = 0;
    UI32_T	pre_admin_state = 0;

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
    {
        if (VRRP_MGR_CreateDefaultOperEntry(&vrrp_oper_entry) == FALSE)
        {
            return VRRP_TYPE_INTERNAL_ERROR;
        }

        result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
        if (result != VRRP_TYPE_OK)
        {
            return result;
        }
    }

    pre_admin_state = vrrp_oper_entry.admin_state;
    if (VRRP_MGR_ChangeParametersPreOperation(&vrrp_oper_entry) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if (vrrp_oper_entry.oper_state != VAL_vrrpOperState_initialize)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    /* when row status is "active", no other fileds except of admin state can be set, RFC2787, p.18 */
    if (vrrp_oper_entry.row_status == VAL_vrrpOperRowStatus_active)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    if ((vrrp_oper_protocol > SYS_ADPT_MAX_VRRP_OPER_PROTOCOL) ||
            (vrrp_oper_protocol < SYS_ADPT_MIN_VRRP_OPER_PROTOCOL))
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    vrrp_oper_entry.oper_protocol = vrrp_oper_protocol;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    if (VRRP_MGR_ChangeParametersAfterOperation(&vrrp_oper_entry, pre_admin_state) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_SetOperProtocol */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetOperProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.
 *            Otherwise, return false.
 * INPUT    : ifIndex  ,  vrid
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_protocol)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T            result;

    if (vrrp_oper_protocol == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    vrrp_oper_entry.ifindex = if_index;
    vrrp_oper_entry.vrid = vrid;
    result = VRRP_OM_GetVrrpOperEntry(&vrrp_oper_entry);
    if (result != VRRP_TYPE_OK)
    {
        return result;
    }

    *vrrp_oper_protocol = vrrp_oper_entry.oper_protocol;

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetOperProtocol() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVersionNumber
 *------------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vrrp version of the system.
 * INPUT    : buffer to be put in the version number
 * OUTPUT   : The vrrp version number
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *            2. It's always return "2" right now.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVersionNumber(UI32_T *version)
{

    if (version == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    *version = 2;

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetVersionNumber() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetCurrentNumVrs
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns the number of RFC 2338 Virtual Router are
 *            currently configured in the system.
 * INPUT    : buffer to be put in the current numbers of virtual routers configured
 * OUTPUT   : The current numbers of virtual routers configured
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetCurrentVrrpNum(UI32_T *vrrp_number)
{

    if (vrrp_number == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_OM_GetCurrentNumOfVRRPConfigured(vrrp_number) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetCurrentVrrpNum() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ClearVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_ClearVrrpSysStatistics(void)
{

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_OM_ClearVrrpSysStatistics() == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_ClearVrrpSysStatistics() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the system's vrrp statistics.
 * INPUT    : none
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *vrrp_router_statistics)
{

    if (vrrp_router_statistics == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    if (VRRP_OM_GetVrrpSysStatistics(vrrp_router_statistics) == FALSE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_GetVrrpSysStatistics() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ClearVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the specific vrrp group statistics.
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid)
{
    UI32_T       result;

    VRRP_MGR_CHECK_ENABLE();

    result = VRRP_OM_ClearVrrpGroupStatistics(if_index, vrid);
    if (result != TRUE)
    {
        return VRRP_TYPE_INTERNAL_ERROR;
    }

    return VRRP_TYPE_OK;
} /* VRRP_MGR_ClearVrrpGroupStatistics() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics)
{
    UI32_T   result;

    if (vrrp_group_statistics == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();

    result = VRRP_OM_GetVrrpGroupStatistics(if_index, vrid, vrrp_group_statistics);

    return result;
} /* VRRP_MGR_GetVrrpGroupStatistics() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the next specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_MGR_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics)
{
    UI32_T   result;

    if (vrrp_group_statistics == 0)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    VRRP_MGR_CHECK_ENABLE();
    result = VRRP_OM_GetNextVrrpGroupStatistics(if_index, vrid, vrrp_group_statistics);

    return result;
} /* VRRP_MGR_GetNextVrrpGroupStatistics() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_CheckEntryValid
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton checks whether the entry is valid
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_CheckEntryValid(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_MGR_CHECK_ENABLE();

    if ((vrrp_oper_entry->vrid > SYS_ADPT_MAX_VRRP_ID) ||
            (vrrp_oper_entry->vrid < SYS_ADPT_MIN_VRRP_ID))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->admin_state != VAL_vrrpOperAdminState_up) &&
            (vrrp_oper_entry->admin_state != VAL_vrrpOperAdminState_down))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->priority <= VRRP_TYPE_MIN_PRIORITY) ||
            (vrrp_oper_entry->priority >= VRRP_TYPE_MAX_PRIORITY))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->auth_type != VAL_vrrpOperAuthType_noAuthentication) &&
            (vrrp_oper_entry->auth_type != VAL_vrrpOperAuthType_simpleTextPassword) &&
            (vrrp_oper_entry->auth_type != VAL_vrrpOperAuthType_ipAuthenticationHeader))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->advertise_interval < SYS_ADPT_MIN_VRRP_ADVER_INTERVAL) ||
            (vrrp_oper_entry->advertise_interval > SYS_ADPT_MAX_VRRP_ADVER_INTERVAL))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->preempt_mode != VAL_vrrpOperPreemptMode_true) &&
            (vrrp_oper_entry->preempt_mode != VAL_vrrpOperPreemptMode_false))
    {
        return FALSE;
    }

    if ((vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_active) &&
            (vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_notInService) &&
            (vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_notReady) &&
            (vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_createAndGo) &&
            (vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_createAndWait) &&
            (vrrp_oper_entry->row_status != VAL_vrrpOperRowStatus_destroy))
    {
        return FALSE;
    }

    return TRUE;
} /* VRRP_MGR_CheckEntryValid() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ProcessTimerEvent
 *--------------------------------------------------------------------------
 * PURPOSE  : Do the process for timer event
 * INPUT    : pass_time         --  passed time interval in ticks
 * OUTPUT   : min_expire_time   --  minimum expire time interval in ticks
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
BOOL_T VRRP_MGR_ProcessTimerEvent(UI32_T pass_time, UI32_T *min_expire_time)
{
    VRRP_MGR_CHECK_ENABLE();

    if(NULL == min_expire_time)
        return FALSE;

    VRRP_MGR_CheckTimer(pass_time, min_expire_time);

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_CheckTimer
 *--------------------------------------------------------------------------
 * PURPOSE  : Do the process for each second
 * INPUT    : pass_time         --  passed time interval in ticks
 * OUTPUT   : min_expire_time   --  minimum expire time interval in ticks
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------- */
static void VRRP_MGR_CheckTimer(UI32_T pass_time, UI32_T *min_expire_time)
{
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T time_interval=0;
    UI32_T expire_time=0;
    UI32_T min_time=VRRP_TYPE_DFLT_TIMER_TICKS;

    vrrp_oper_entry.ifindex = 0;
    vrrp_oper_entry.vrid = 0;
    while (VRRP_TYPE_OK == VRRP_MGR_GetNextVrrpOperEntry(&vrrp_oper_entry))
    {
        if (vrrp_oper_entry.admin_state == VAL_vrrpOperAdminState_down)
            continue;

        if (vrrp_oper_entry.oper_state == VAL_vrrpOperState_master)
        {
            /* Check Advertisement Timer for Master */
            if (VRRP_VM_Check_Adver_Timer(&vrrp_oper_entry, pass_time) == VRRP_VM_INTERNAL_ERROR)
            {
                /* Should log error! (process adver_timer_expire error in VM) */
            }

            expire_time = vrrp_oper_entry.transmit_expire;
        }
        else if (vrrp_oper_entry.oper_state == VAL_vrrpOperState_backup)
        {
            /* Check Master Down Timer for Backup */
            if (VRRP_VM_Check_Master_Down_Timer(&vrrp_oper_entry, pass_time) == VRRP_VM_INTERNAL_ERROR)
            {
                /* Should log error! (process ms_down_timer_expire error in VM) */
            }

            expire_time = vrrp_oper_entry.master_down_expire;
        }

        if((expire_time != 0) &&
           (expire_time < min_time))
        {
            min_time = expire_time;
        }
    }/* end of while */

    *min_expire_time = min_time;
    return;
} /* VRRP_MGR_rexmtTimerExpiry() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetPrimaryIp
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get primary ip address of specific ifindex
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_GetPrimaryIp(UI32_T ifindex, UI8_T *SrcIp)
{
#define PRIMARY  VAL_netConfigPrimaryInterface_primary

    SYSLOG_OM_RecordOwnerInfo_T vrrpErrorLogS;
    NETCFG_TYPE_InetRifConfig_T rif_config_p;


    rif_config_p.ifindex = ifindex;
    if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config_p) != NETCFG_TYPE_OK)
    {
        if (vrrpMgrErrorLogFlag[0] != 1)
        {
            vrrpErrorLogS.level = SYSLOG_LEVEL_WARNING;
            vrrpErrorLogS.module_no = SYS_MODULE_VRRP;
            vrrpErrorLogS.function_no = 11;
            vrrpErrorLogS.error_no = 1;
            vrrpMgrErrorLogFlag[0] = 1;
        }
        return FALSE;
    }

    memcpy(SrcIp, rif_config_p.addr.addr, VRRP_IP_ADDR_LENGTH);
    return TRUE;
} /* VRRP_MGR_GetPrimaryIp() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ChangeParametersPreOperation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function does the preoperation before parameters change
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : 1. First, shutdown vrrp
 *            2. Second, make sure oper status is init
 *            3. Finally, set row status to notReady
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_ChangeParametersPreOperation(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T            result;

    if (vrrp_oper_entry->admin_state == VAL_vrrpOperAdminState_up)
    {
        if (VRRP_VM_ShutDown(vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) == FALSE)
            return FALSE;
    }

    vrrp_entry.ifindex = vrrp_oper_entry->ifindex;
    vrrp_entry.vrid = vrrp_oper_entry->vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return FALSE;

    if (vrrp_entry.oper_state != VAL_vrrpOperState_initialize)
        return FALSE;

    vrrp_entry.admin_state = VAL_vrrpOperAdminState_down;
    result = VRRP_OM_SetVrrpOperEntry(&vrrp_entry);
    if (result != VRRP_TYPE_OK)
        return FALSE;

    L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_notInService, &vrrp_entry.row_status,
                  VRRP_MGR_SemanticCheck, (void*)&vrrp_entry);

    if (vrrp_entry.row_status == VAL_vrrpOperRowStatus_active)
        return FALSE;

    result = VRRP_OM_SetVrrpOperEntry(&vrrp_entry);
    if (result != VRRP_TYPE_OK)
        return FALSE;

    return TRUE;
} /* VRRP_MGR_ChangeParametersPreOperation() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_ChangeParametersAfterOperation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function does the preoperation after parameters changing
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : 1. First, set row status to active
 *            2. Second, set admin to up
 *            3. Finally, startup vrrp
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_ChangeParametersAfterOperation(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T admin_state)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T            result;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_entry;

    vrrp_entry.ifindex = vrrp_oper_entry->ifindex;
    vrrp_entry.vrid = vrrp_oper_entry->vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return FALSE;

    L_RSTATUS_Fsm(VAL_vrrpOperRowStatus_active, &vrrp_entry.row_status,
                  VRRP_MGR_SemanticCheck, (void*)&vrrp_entry);

    if (admin_state == VAL_vrrpOperAdminState_up)
    {
        vrrp_entry.admin_state = VAL_vrrpOperAdminState_up;
        result = VRRP_OM_SetVrrpOperEntry(&vrrp_entry);
        if (result != VRRP_TYPE_OK)
            return FALSE;

        vlan_entry.dot1q_vlan_index = (UI16_T)vrrp_oper_entry->ifindex;
        if (VLAN_POM_GetVlanEntry(&vlan_entry) == FALSE)
        {
            return FALSE;
        }

        if (vlan_entry.if_entry.vlan_operation_status == VAL_ifOperStatus_up)
        {
            if (vrrp_oper_entry->virtual_ip_up_count > 0)
                if (VRRP_VM_Startup(vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) == FALSE)
                    return FALSE;
        }
    }
    return TRUE;
} /* VRRP_MGR_ChangeParametersAfterOperation() */

/************************************************************************
 * The following APIs are for running config
 ************************************************************************/

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPriority(UI32_T ifindex, UI8_T vrid, UI32_T *priority)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.priority != SYS_DFLT_VRRP_PRIORITY)
    {
        *priority = vrrp_entry.priority;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpPriority() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPriority(UI32_T *ifindex, UI8_T *vrid, UI32_T *priority)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    *priority = vrrp_entry.priority;
    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.priority != SYS_DFLT_VRRP_PRIORITY)
    {
        if (vrrp_entry.priority != VRRP_TYPE_MAX_PRIORITY)
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpPriority() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAuthType(UI32_T ifindex, UI8_T vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.auth_type != SYS_DFLT_VRRP_AUTH_TYPE)
    {
        *auth_type = vrrp_entry.auth_type;
        memcpy(vrrp_oper_auth_key, vrrp_entry.auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpAuthType() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : ifindex   -- next ifindex
 *            vrid      -- next vrid
 *            auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAuthType(UI32_T *ifindex, UI8_T *vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    *auth_type = vrrp_entry.auth_type;
    memcpy(vrrp_oper_auth_key, vrrp_entry.auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.auth_type != SYS_DFLT_VRRP_AUTH_TYPE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpAuthType() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAdverInt(UI32_T ifindex, UI8_T vrid, UI32_T *interval)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;

    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.advertise_interval != SYS_DFLT_VRRP_ADVER_INTERVAL)
    {
        *interval = vrrp_entry.advertise_interval;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpAdverInt() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAdverInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *interval)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    *interval = vrrp_entry.advertise_interval;
    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.advertise_interval != SYS_DFLT_VRRP_ADVER_INTERVAL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpAdverInt() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPreemptMode(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_mode)
{
    VRRP_OPER_ENTRY_T vrrp_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.preempt_mode != SYS_DFLT_VRRP_PREEMPT_MODE)
    {
        *preempt_mode = vrrp_entry.preempt_mode;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    if (vrrp_entry.preempt_delay != SYS_DFLT_VRRP_PREEMPT_DELAY)
    {
        *preempt_mode = vrrp_entry.preempt_mode;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpPreemptMode() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPreemptMode(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_mode)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.preempt_mode != SYS_DFLT_VRRP_PREEMPT_MODE)
    {
        *preempt_mode = vrrp_entry.preempt_mode;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    if (vrrp_entry.preempt_delay != SYS_DFLT_VRRP_PREEMPT_DELAY)
    {
        *preempt_mode = vrrp_entry.preempt_mode;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpPreemptMode() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpPreemptDelay(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_delay)
{
    VRRP_OPER_ENTRY_T vrrp_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.preempt_delay != SYS_DFLT_VRRP_PREEMPT_DELAY)
    {
        *preempt_delay = vrrp_entry.preempt_delay;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpPreemptDelay() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpPreemptDelay(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_delay)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.preempt_delay != SYS_DFLT_VRRP_PREEMPT_DELAY)
    {
        *preempt_delay = vrrp_entry.preempt_delay;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpPreemptDelay() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpProtocol(UI32_T ifindex, UI8_T vrid, UI32_T *protocol)
{
    VRRP_OPER_ENTRY_T vrrp_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.oper_protocol != VRRP_DEFAULT_OPER_PROTOCOL)
    {
        *protocol = vrrp_entry.oper_protocol;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpProtocol() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpProtocol(UI32_T *ifindex, UI8_T *vrid, UI32_T *protocol)
{
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
    if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    if (result == VRRP_TYPE_PARAMETER_ERROR)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    *ifindex = vrrp_entry.ifindex;
    *vrid = vrrp_entry.vrid;

    if (vrrp_entry.oper_protocol != VRRP_DEFAULT_OPER_PROTOCOL)
    {
        *protocol = vrrp_entry.oper_protocol;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpProtocol() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ip_addr_count  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningVrrpAssocIp(UI32_T ifindex, UI8_T vrid, UI8_T *ip_addr_count)
{
    VRRP_OPER_ENTRY_T vrrp_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if (vrrp_entry.ip_addr_count != VRRP_DEFAULT_IP_ADDR_COUNT)
    {
        *ip_addr_count = vrrp_entry.ip_addr_count;
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetRunningVrrpAssocIp() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetNextRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            ip_addr  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetNextRunningVrrpAssocIp(UI32_T *ifindex, UI8_T *vrid, UI8_T *ip_addr)
{
    VRRP_OPER_ENTRY_T     vrrp_entry;
    VRRP_ASSOC_IP_ENTRY_T assoc_ip_info;
    UI32_T	result = 0;

    vrrp_entry.ifindex = *ifindex;
    vrrp_entry.vrid = *vrid;
    if (*vrid == 0)
    { /* First of all entries, so we must get the first entry then get the first assoc ip */
        result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
        if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        if (result == VRRP_TYPE_PARAMETER_ERROR)
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;

        if (vrrp_entry.ip_addr_count != VRRP_DEFAULT_IP_ADDR_COUNT)
        {
            assoc_ip_info.ifindex = vrrp_entry.ifindex;
            assoc_ip_info.vrid = vrrp_entry.vrid;
            memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            if (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
            {
                memcpy(ip_addr, assoc_ip_info.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
                *ifindex = vrrp_entry.ifindex;
                *vrid = vrrp_entry.vrid;
                return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
            }
        }
    }
    else
    { /* use previous gotten assoc ip to get next assoc ip */
        assoc_ip_info.ifindex = vrrp_entry.ifindex;
        assoc_ip_info.vrid = vrrp_entry.vrid;
        memcpy(assoc_ip_info.assoc_ip_addr, ip_addr, VRRP_IP_ADDR_LENGTH);

        if (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
        {
            memcpy(ip_addr, assoc_ip_info.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
            *ifindex = vrrp_entry.ifindex;
            *vrid = vrrp_entry.vrid;
            return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
        {/* do not get next assoc ip in the current (ifindex, vrid), so get next */
            result = VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry);
            if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
                return SYS_TYPE_GET_RUNNING_CFG_FAIL;
            if (result == VRRP_TYPE_PARAMETER_ERROR)
                return SYS_TYPE_GET_RUNNING_CFG_FAIL;

            if (vrrp_entry.ip_addr_count != VRRP_DEFAULT_IP_ADDR_COUNT)
            {
                assoc_ip_info.ifindex = vrrp_entry.ifindex;
                assoc_ip_info.vrid = vrrp_entry.vrid;
                memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
                if (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
                {
                    memcpy(ip_addr, assoc_ip_info.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
                    *ifindex = vrrp_entry.ifindex;
                    *vrid = vrrp_entry.vrid;
                    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
                }
            }
        }
    }
    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
} /* VRRP_MGR_GetNextRunningVrrpAssocIp() */

#if (SYS_CPNT_VRRP_PING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_GetRunningPingStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP ping status and check whether it is default value.
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VRRP_MGR_GetRunningPingStatus(UI32_T *ping_status)
{
    VRRP_TYPE_GlobalEntry_T	global_config;

    if (NULL == ping_status)
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    memset(&global_config, 0, sizeof(global_config));
    if (VRRP_TYPE_OK != VRRP_OM_GetVrrpGlobalConfig(&global_config))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    *ping_status = global_config.vrrpPingStatus;
    if (global_config.vrrpPingStatus != SYS_DFLT_VRRP_PING_STATUS)
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SetTimerId
 *------------------------------------------------------------------------------
 * PURPOSE  : Set VRRP_GROUP periodic timer id
 * INPUT    : timer_id  	--	timer id
 * OUTPUT   : None
 * RETURN   : TRUE
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T VRRP_MGR_SetTimerId(void *timer_id_p)
{
    VRRP_MGR_CHECK_ENABLE();
    VRRP_OM_SetTimerId(timer_id_p);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalRifUp
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that rifActive.
 * INPUT    : ip_addr  -- ip address of down rif
 *            ip_mask  -- ip mask of down rif
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
BOOL_T VRRP_MGR_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    VRRP_OPER_ENTRY_T               vrrp_entry;
    VRRP_ASSOC_IP_ENTRY_T           assoc_ip_info;
    UI32_T                          ip_addr_trans;
    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = 0;

    VRRP_BD(EVENT, "ifindex[%lu],ip[%u.%u.%u.%u/%lx]",
            (unsigned long)ifindex, L_INET_EXPAND_IP(ip_addr), (unsigned long)ip_mask);

    while (VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry) == VRRP_TYPE_ACCESS_SUCCESS)
    { /* Find the vrrp oper entry existing in the specific ifindex */

        if (vrrp_entry.ifindex == ifindex)
        {
            if (VRRP_MGR_GetPrimaryIp(vrrp_entry.ifindex, vrrp_entry.primary_ip_addr) == FALSE)
                memset(vrrp_entry.primary_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            if (VRRP_OM_SetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
            {
                return FALSE;
            }

            if (vrrp_entry.oper_state == VAL_vrrpOperState_master)
            {
                assoc_ip_info.ifindex = vrrp_entry.ifindex;
                assoc_ip_info.vrid = vrrp_entry.vrid;
                memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
                while (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
                {
                    IP_LIB_ArraytoUI32(assoc_ip_info.assoc_ip_addr, &ip_addr_trans);
                    if (ip_addr_trans == ip_addr)
                    {
                        vrrp_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                        vrrp_entry.master_priority = vrrp_entry.priority;
                        vrrp_entry.owner = VrrpOperEntryAddrOwner_true;
                        if (VRRP_OM_SetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
                        {
                            return FALSE;
                        }
                    }
                }
            }
            else if (vrrp_entry.oper_state == VAL_vrrpOperState_initialize)
            {
                assoc_ip_info.ifindex = vrrp_entry.ifindex;
                assoc_ip_info.vrid = vrrp_entry.vrid;
                memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
                while (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
                { /* Find the assoc IP located in the same subnet as (ip_addr&ip_mask) */
                    IP_LIB_ArraytoUI32(assoc_ip_info.assoc_ip_addr, &ip_addr_trans);
                    if (ip_addr_trans == ip_addr)
                    {
                        vrrp_entry.virtual_ip_up_count++;

                        vrrp_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                        vrrp_entry.master_priority = vrrp_entry.priority;
                        vrrp_entry.owner = VrrpOperEntryAddrOwner_true;
                        if (VRRP_OM_SetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
                            return FALSE;

                        if (vrrp_entry.virtual_ip_up_count == 1)
                            VRRP_VM_Startup(vrrp_entry.ifindex, vrrp_entry.vrid);

                    }
                    else if ((ip_addr_trans & ip_mask) == (ip_addr & ip_mask))
                    {
                        vrrp_entry.virtual_ip_up_count++;

                        if (VRRP_OM_SetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
                            return FALSE;

                        if (vrrp_entry.virtual_ip_up_count == 1)
                            VRRP_VM_Startup(vrrp_entry.ifindex, vrrp_entry.vrid);

                    }
                }
            }
            else if (vrrp_entry.oper_state == VAL_vrrpOperState_backup)
            {
                assoc_ip_info.ifindex = vrrp_entry.ifindex;
                assoc_ip_info.vrid = vrrp_entry.vrid;
                memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
                while (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
                { /* Find the assoc IP located in the same subnet as (ip_addr&ip_mask) */
                    IP_LIB_ArraytoUI32(assoc_ip_info.assoc_ip_addr, &ip_addr_trans);
                    if (ip_addr_trans == ip_addr)
                    {
                        vrrp_entry.priority = VRRP_TYPE_MAX_PRIORITY;
                        vrrp_entry.master_priority = vrrp_entry.priority;
                        vrrp_entry.owner = VrrpOperEntryAddrOwner_true;

                        if (VRRP_OM_SetVrrpOperEntry(&vrrp_entry) != VRRP_TYPE_OK)
                            return FALSE;

                    }
                }
            }
        }
        else
            break;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalRifDown
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that rifActive.
 * INPUT    : ip_addr  -- ip address of down rif
 *            ip_mask  -- ip mask of down rif
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/

BOOL_T VRRP_MGR_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask)
{
    VRRP_OPER_ENTRY_T              vrrp_entry;
    VRRP_ASSOC_IP_ENTRY_T          assoc_ip_info;
    UI32_T                         ip_addr_trans;
    NETCFG_TYPE_InetRifConfig_T    rif_config_p;
    VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = 0;

    VRRP_BD(EVENT, "ifindex[%lu],ip[%u.%u.%u.%u/%lx]",
            (unsigned long)ifindex, L_INET_EXPAND_IP(ip_addr), (unsigned long)ip_mask);

    while (VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry) == VRRP_TYPE_ACCESS_SUCCESS)
    {
        if (vrrp_entry.ifindex != ifindex)
            break;

        /* master vrrp operation entry rif down
         */
        if (vrrp_entry.oper_state == VAL_vrrpOperState_master)
        {
            assoc_ip_info.ifindex = vrrp_entry.ifindex;
            assoc_ip_info.vrid = vrrp_entry.vrid;
            memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
            while (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
            {
                /* Find the assoc IP located in the same subnet as (ip_addr&ip_mask)
                */
                IP_LIB_ArraytoUI32(assoc_ip_info.assoc_ip_addr, &ip_addr_trans);
                if ((ip_addr_trans == ip_addr) ||
                        (ip_addr_trans & ip_mask) == (ip_addr & ip_mask))
                {

                    VRRP_VM_ShutDown(vrrp_entry.ifindex, vrrp_entry.vrid);

                    if (!VRRP_MGR_DecreaseVirtualIpUpCount(&vrrp_entry))
                        return FALSE;
                }
            }
        }

        /* backup vrrp operation entry rif down
         */
        if (vrrp_entry.oper_state == VAL_vrrpOperState_backup)
        {
            assoc_ip_info.ifindex = vrrp_entry.ifindex;
            assoc_ip_info.vrid = vrrp_entry.vrid;
            memset(assoc_ip_info.assoc_ip_addr, 0, sizeof(assoc_ip_info.assoc_ip_addr));
            while (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info))
            {
                /* Find the assoc IP located in the same subnet as (ip_addr&ip_mask)
                */
                IP_LIB_ArraytoUI32(assoc_ip_info.assoc_ip_addr, &ip_addr_trans);
                if ((ip_addr_trans & ip_mask) == (ip_addr & ip_mask))
                {
                    memset(&rif_config_p, 0, sizeof(rif_config_p));
                    memcpy(rif_config_p.addr.addr, assoc_ip_info.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
                    rif_config_p.addr.type = L_INET_ADDR_TYPE_IPV4;

                    /* rif is not existed, shutdown operation entry and decrease virtual ip count
                     */
                    if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetRifFromIp(&rif_config_p))
                    {
                        VRRP_VM_ShutDown(vrrp_entry.ifindex, vrrp_entry.vrid);

                        if (!VRRP_MGR_DecreaseVirtualIpUpCount(&vrrp_entry))
                            return FALSE;
                    }

                    vlan_entry.dot1q_vlan_index = (UI16_T)ifindex;
                    if (VLAN_POM_GetVlanEntry(&vlan_entry) == FALSE)
                    {
                        return FALSE;
                    }

                    /* if downed rif is not the same interface as associated ip,
                     * or vlan status is not up,
                     * shutdown this operation entry and decrease its virtual ip count
                     */
                    if ((rif_config_p.ifindex != ifindex) ||
                            (vlan_entry.if_entry.vlan_operation_status != VAL_ifOperStatus_up))
                    {
                        VRRP_VM_ShutDown(vrrp_entry.ifindex, vrrp_entry.vrid);

                        if (!VRRP_MGR_DecreaseVirtualIpUpCount(&vrrp_entry))
                            return FALSE;

                    }
                }
            }
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_SignalVlanInterfaceDelete
 *------------------------------------------------------------------------------
 * PURPOSE  : Signal from NETCFG_MGR to indicate that vlan interface delete.
 * INPUT    : ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/

void VRRP_MGR_SignalVlanInterfaceDelete(UI32_T ifindex)
{
    VRRP_OPER_ENTRY_T               vrrp_entry;

    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = 0;
    while (VRRP_OM_GetNextVrrpOperEntry(&vrrp_entry) == VRRP_TYPE_ACCESS_SUCCESS)
    {
        if (vrrp_entry.ifindex == ifindex)
        {
            VRRP_MGR_DeleteVrrpOperEntry(&vrrp_entry);
        }
        else
            break;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_CreateDefaultOperEntry
 *------------------------------------------------------------------------------
 * PURPOSE  : Create a default VRRP oper entry for a given (ifindex, vrid).
 * INPUT    : vrrp_oper_entry->ifindex  -- ifindex
 *            vrrp_oper_entry->vrid -- vrrp group ID
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_CreateDefaultOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    vrrp_oper_entry->virtual_mac[0] = 0x00;
    vrrp_oper_entry->virtual_mac[1] = 0x00;
    vrrp_oper_entry->virtual_mac[2] = 0x5E;
    vrrp_oper_entry->virtual_mac[3] = 0x00;
    vrrp_oper_entry->virtual_mac[4] = 0x01;
    vrrp_oper_entry->virtual_mac[5] = vrrp_oper_entry->vrid;
    vrrp_oper_entry->oper_state = VAL_vrrpOperState_initialize;
    vrrp_oper_entry->admin_state = VAL_vrrpOperAdminState_down;
    vrrp_oper_entry->ip_addr_count = 0;
    vrrp_oper_entry->master_ip_addr[0] = 0x00;
    vrrp_oper_entry->master_ip_addr[1] = 0x00;
    vrrp_oper_entry->master_ip_addr[2] = 0x00;
    vrrp_oper_entry->master_ip_addr[3] = 0x00;
    memset(vrrp_oper_entry->primary_ip_addr, 0, SYS_ADPT_IPV4_ADDR_LEN);
    vrrp_oper_entry->priority = SYS_DFLT_VRRP_PRIORITY;
    vrrp_oper_entry->pre_priority = vrrp_oper_entry->priority;
    vrrp_oper_entry->auth_type = SYS_DFLT_VRRP_AUTH_TYPE;
    memset(vrrp_oper_entry->auth_key, 0, sizeof(vrrp_oper_entry->auth_key));
    vrrp_oper_entry->advertise_interval = SYS_DFLT_VRRP_ADVER_INTERVAL;
    vrrp_oper_entry->preempt_mode = SYS_DFLT_VRRP_PREEMPT_MODE;
    vrrp_oper_entry->preempt_delay = SYS_DFLT_VRRP_PREEMPT_DELAY;
    vrrp_oper_entry->preempt_delay_start = FALSE;
    vrrp_oper_entry->oper_protocol = VRRP_DEFAULT_OPER_PROTOCOL;
    vrrp_oper_entry->row_status = VAL_vrrpOperRowStatus_notReady;
    vrrp_oper_entry->master_priority = 0;
    vrrp_oper_entry->master_advertise_int = 0;
    vrrp_oper_entry->master_down_int = 0;
    vrrp_oper_entry->transmit_expire = VRRP_TYPE_NO_EXPIRE_TIME;
    vrrp_oper_entry->master_down_expire = VRRP_TYPE_NO_EXPIRE_TIME;
    vrrp_oper_entry->preempt_delay_expire = VRRP_TYPE_NO_EXPIRE_TIME;
    vrrp_oper_entry->owner = 0;
    vrrp_oper_entry->virtual_ip_up_count = 0;
    memset(&vrrp_oper_entry->vrrp_statistic, 0, sizeof(VRRP_OM_Vrrp_Statistics_Info_T));
    memset(&(vrrp_oper_entry->assoc_ip_list),0, sizeof(vrrp_oper_entry->assoc_ip_list));
    return TRUE;
} /* VRRP_MGR_CreateDefaultOperEntry() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_VrrpOperEntryIsDefault
 *------------------------------------------------------------------------------
 * PURPOSE  : Check whether VRRP group entry are all default value
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : TRUE if VRRP operation entry is equal to default value
 *            FALSE, otherwise.
 *------------------------------------------------------------------------------*/
static BOOL_T VRRP_MGR_VrrpOperEntryIsDefault(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    if (vrrp_oper_entry->auth_type != SYS_DFLT_VRRP_AUTH_TYPE)
        return FALSE;

    if ((vrrp_oper_entry->priority != SYS_DFLT_VRRP_PRIORITY) &&
            (vrrp_oper_entry->owner == VrrpOperEntryAddrOwner_false))
        return FALSE;

    if (vrrp_oper_entry->preempt_mode != SYS_DFLT_VRRP_PREEMPT_MODE)
        return FALSE;

    if (vrrp_oper_entry->advertise_interval != SYS_DFLT_VRRP_ADVER_INTERVAL)
        return FALSE;

    if (vrrp_oper_entry->ip_addr_count != VRRP_DEFAULT_IP_ADDR_COUNT)
        return FALSE;

    return TRUE;
} /* VRRP_MGR_VrrpOperEntryIsDefault() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_DecreaseVirtualIpUpCount
 *------------------------------------------------------------------------------
 * PURPOSE  : Decrease vrrp virtual ip up count for operation entry
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : When VRRP_MGR_SignalRifDown, it should decrease virtual ip up count
 *------------------------------------------------------------------------------
 */
static BOOL_T VRRP_MGR_DecreaseVirtualIpUpCount(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    if (NULL == vrrp_oper_entry)
        return FALSE;

    if (VRRP_TYPE_OK != VRRP_OM_GetVrrpOperEntry(vrrp_oper_entry))
        return FALSE;

    vrrp_oper_entry->virtual_ip_up_count--;

    if (VRRP_TYPE_OK != VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry))
        return FALSE;

    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_GetDebugFlag
 *------------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *------------------------------------------------------------------------------
 */
BOOL_T  VRRP_MGR_BACKDOOR_GetDebugFlag(UI32_T flag)
{
    return ((vrrp_debug_flag & flag) != 0);
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_MainMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will display VRRP main menu,
 *           and select debug information
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void VRRP_MGR_BACKDOOR_MainMenu(void)
{

    char line_buffer[MAXLINE];
    int  select_value = 0;
    while (1)
    {
        VRRP_BD_MSG("\r\n------------- VRRP Backdoor Main Menu ------------");
        VRRP_BD_MSG("\r\n\t 1 : VRRP_OM configuration");
        VRRP_BD_MSG("\r\n\t 2 : VRRP debug configuration");
        VRRP_BD_MSG("\r\n\t 3 : VRRP timer configuration");
        VRRP_BD_MSG("\r\n\t 0 : Return");
        VRRP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        VRRP_BD_MSG("\r\nEnter Selection:");

        if (VRRP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
                /* VRRP_OM configuration */
            case 1:
                VRRP_MGR_BACKDOOR_OmConfigMenu();
                break;

                /* VRRP debug configuration */
            case 2:
                VRRP_MGR_BACKDOOR_DbgConfigMenu();
                break;
            case 3:
                VRRP_MGR_BACKDOOR_TimerConfigMenu();
                break;
            case 0:
                VRRP_BD_MSG("\r\n");
                return;
                break;
            default:
                break;

        }
    }
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_OmConfigMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter om config menu
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void VRRP_MGR_BACKDOOR_OmConfigMenu(void)
{
    char line_buffer[MAXLINE];
    int  select_value = 0;
    while (1)
    {
        VRRP_BD_MSG("\r\n---- VRRP OM Config Menu ----");
        VRRP_BD_MSG("\r\n\t 1 : Display vrrp operation entry");
        VRRP_BD_MSG("\r\n\t 0 : Return");
        VRRP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        VRRP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (VRRP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
                /* display vrrp operation entry */
            case 1:
                {
                    VRRP_OPER_ENTRY_T vrrp_info;
                    VRRP_ASSOC_IP_ENTRY_T  ip_entry;
                    memset(&vrrp_info, 0, sizeof(vrrp_info));
                    while (VRRP_TYPE_ACCESS_SUCCESS == VRRP_OM_GetNextVrrpOperEntry(&vrrp_info))
                    {
                        VRRP_BD_MSG("\r\n VRRP information:");
                        VRRP_BD_MSG("\r\n ifindex      =%lu", (unsigned long)vrrp_info.ifindex);
                        VRRP_BD_MSG("\r\n vrid         =%u", vrrp_info.vrid);
                        VRRP_BD_MSG("\r\n virtual mac  =%02x-%02x-%02x-%02x-%02x-%02x", L_INET_EXPAND_MAC(vrrp_info.virtual_mac));
                        VRRP_BD_MSG("\r\n oper_state   =%lu", (unsigned long)vrrp_info.oper_state);
                        VRRP_BD_MSG("\r\n admin_state  =%lu", (unsigned long)vrrp_info.admin_state);
                        VRRP_BD_MSG("\r\n priority     =%lu", (unsigned long)vrrp_info.priority);
                        VRRP_BD_MSG("\r\n pre_priority =%lu", (unsigned long)vrrp_info.pre_priority);
                        VRRP_BD_MSG("\r\n ip_addr_count=%lu", (unsigned long)vrrp_info.ip_addr_count);
                        VRRP_BD_MSG("\r\n master ip    =%u.%u.%u.%u", L_INET_EXPAND_IP(vrrp_info.master_ip_addr));
                        VRRP_BD_MSG("\r\n primary ip   =%u.%u.%u.%u", L_INET_EXPAND_IP(vrrp_info.primary_ip_addr));
                        VRRP_BD_MSG("\r\n auth_type    =%lu", (unsigned long)vrrp_info.auth_type);
                        VRRP_BD_MSG("\r\n auth_key     =%s", vrrp_info.auth_key);
                        VRRP_BD_MSG("\r\n advertise_interval =%lu", (unsigned long)vrrp_info.advertise_interval);
                        VRRP_BD_MSG("\r\n preempt_mode =%lu", (unsigned long)vrrp_info.preempt_mode);
                        VRRP_BD_MSG("\r\n preempt_delay=%lu", (unsigned long)vrrp_info.preempt_delay);
                        VRRP_BD_MSG("\r\n preempt_delay_start =%s", vrrp_info.preempt_delay_start ? "TRUE" : "FALSE");
                        VRRP_BD_MSG("\r\n virtual_router_up_time =%lu", (unsigned long)vrrp_info.virtual_router_up_time);
                        VRRP_BD_MSG("\r\n oper_protocol=%lu", (unsigned long)vrrp_info.oper_protocol);
                        VRRP_BD_MSG("\r\n row_status   =%lu", (unsigned long)vrrp_info.row_status);
                        VRRP_BD_MSG("\r\n master_priority =%lu", (unsigned long)vrrp_info.master_priority);
                        VRRP_BD_MSG("\r\n master_advertise_interval =%lu", (unsigned long)vrrp_info.master_advertise_int);
                        VRRP_BD_MSG("\r\n master_down_interval =%lu", (unsigned long)vrrp_info.master_down_int);
                        VRRP_BD_MSG("\r\n transmit_expire =%lu", (unsigned long)vrrp_info.transmit_expire);
                        VRRP_BD_MSG("\r\n master_down_expire =%lu", (unsigned long)vrrp_info.master_down_expire);
                        VRRP_BD_MSG("\r\n preempt_delay_expire =%lu", (unsigned long)vrrp_info.preempt_delay_expire);
                        VRRP_BD_MSG("\r\n owner        =%lu", (unsigned long)vrrp_info.owner);
                        VRRP_BD_MSG("\r\n virtual_ip_up_count =%lu", (unsigned long)vrrp_info.virtual_ip_up_count);
#if 0               /* display statistics */
                        VRRP_OM_Vrrp_Statistics_Info_T vrrp_statistic;
#endif
                        memset(&ip_entry, 0, sizeof(ip_entry));
                        ip_entry.ifindex = vrrp_info.ifindex;
                        ip_entry.vrid = vrrp_info.vrid;
                        if (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(&ip_entry))
                        {
                            VRRP_BD_MSG("\r\n associate ip = %u.%u.%u.%u", L_INET_EXPAND_IP(ip_entry.assoc_ip_addr));
                            VRRP_BD_MSG("\r\n row_status   = %lu", (unsigned long)ip_entry.row_status);
                        }


                    }
                }
                break;

            case 0:
            default:
                return;
                break;
        }

    }

}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_DbgConfigMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter debug config menu
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void VRRP_MGR_BACKDOOR_DbgConfigMenu(void)
{
    typedef struct VRRP_MGR_BACKDOOR_DebugStruct_S
    {
        UI8_T  debug_flag;
        char   *debug_description;
    } VRRP_MGR_BACKDOOR_DebugStruct_T;

    VRRP_MGR_BACKDOOR_DebugStruct_T debug[] ={
        /* flag              description  */
        {VRRP_BD_FLAG_DBG,  "DEBUG"},
        {VRRP_BD_FLAG_INFO, "INFO"},
        {VRRP_BD_FLAG_NOTE, "NOTE"},
        {VRRP_BD_FLAG_EVENT,"EVENT"},
        {VRRP_BD_FLAG_TIMER,"TIMER"}
    };

    char line_buffer[MAXLINE];
    int  select_value = 0;
    UI8_T index = 0;

    while (1)
    {
        VRRP_BD_MSG("\r\n------------- VRRP Set Debug Flag Menu -----------------");
        for (index = 0;index < sizeof(debug)/sizeof(debug[0]);index++)
        {
            VRRP_BD_MSG("\r\n\t %u : %s(%s)",
                        index + 1, debug[index].debug_description, (vrrp_debug_flag&debug[index].debug_flag) ? "YES" : "NO");
        }

        VRRP_BD_MSG("\r\n\t 0 : Return");
        VRRP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        VRRP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (VRRP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        if(select_value == 0)
            return;

        if((select_value-1) < sizeof(debug)/sizeof(debug[0]))
        {
            vrrp_debug_flag ^= debug[select_value-1].debug_flag;
        }
    }
}


/*-----------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_MGR_BACKDOOR_TimerConfigMenu
 *-----------------------------------------------------------------------------
 * PURPOSE : This function will enter timer config menu
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
static void VRRP_MGR_BACKDOOR_TimerConfigMenu(void)
{
    char line_buffer[MAXLINE];
    int  select_value = 0;
    void *timer_id_p = VRRP_OM_GetTimerId();
    UI32_T time_interval=0;
    while (1)
    {
        if(!SYSFUN_PeriodicTimer_Get((void *)timer_id_p, &time_interval))
            VRRP_BD_MSG("failed to get periodic timer\r\n");

        VRRP_BD_MSG("\r\n---- VRRP Timer Config Menu ----");
        VRRP_BD_MSG("\r\n\t Current Timer Interval [%lu ticks]", (unsigned long)time_interval);
        VRRP_BD_MSG("\r\n\t 1 : Restart timer with new time interval");
        VRRP_BD_MSG("\r\n\t 2 : Stop timer");
        VRRP_BD_MSG("\r\n\t 0 : Return");
        VRRP_BD_MSG("\r\n-------------------------------------------------------\r\n");
        VRRP_BD_MSG("\r\nEnter Selection:");
        select_value = 0;
        if (VRRP_BD_GetLine(line_buffer, MAXLINE) > 0)
        {
            select_value = atoi(line_buffer);
        }

        switch (select_value)
        {
            case 1:
            {
                UI32_T ticks=VRRP_TYPE_DFLT_TIMER_TICKS;

                VRRP_BD_MSG("Time interval(ticks):");
                if (VRRP_BD_GetLine(line_buffer, MAXLINE) > 0)
                {
                    ticks = atoi(line_buffer);
                }

                SYSFUN_PeriodicTimer_Restart(timer_id_p, ticks);
            }
                break;
            case 2:
                SYSFUN_PeriodicTimer_Stop(timer_id_p);
                break;
            case 0:
                return;
        }

    }

}

