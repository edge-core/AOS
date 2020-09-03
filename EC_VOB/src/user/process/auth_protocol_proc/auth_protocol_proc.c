/* MODULE NAME:  auth_protocol_proc.c
 * PURPOSE:
 *    This is a code for implementation of process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/2/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "sys_cpnt.h"
#include "sysrsc_mgr.h"
#include "auth_protocol_proc_comm.h"
#include "auth_protocol_group.h"

#include "l_cmnlib_init.h"
#include "dev_nicdrv_pmgr.h"
#include "amtr_pmgr.h"
#include "amtr_pom.h"
#include "syslog_pmgr.h"
#include "syslog_pom.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_swdrv_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "dev_swdrvl4_pmgr.h"
#include "iml_pmgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#include "netcfg_pmgr_main.h"
#include "netcfg_pom_main.h"
#include "stkctrl_pmgr.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#include "l_mm_backdoor.h"

#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_om.h"
#endif

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_om.h"
#endif

#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_om.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(AUTH_PROTOCOL_PROC_OM_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* union all data type used for MGR IPC message to get the maximum required
 * ipc message buffer
 */

/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union AUTH_PROTOCOL_PROC_OM_MSG_U
{
#if (SYS_CPNT_TACACS == TRUE)
    TACACS_OM_IPCMsg_T tacacs_om_ipcmsg;
#endif
#if (SYS_CPNT_RADIUS == TRUE)
    RADIUS_OM_IPCMsg_T radius_om_ipcmsg;
#endif
#if (SYS_CPNT_AAA == TRUE)
    AAA_OM_IPCMsg_T aaa_om_ipcmsg;
#endif
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} AUTH_PROTOCOL_PROC_OM_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T AUTH_PROTOCOL_PROC_Init(void);
static BOOL_T AUTH_PROTOCOL_PROC_InitiateProcessResource(void);
static void AUTH_PROTOCOL_PROC_Create_InterCSC_Relation(void);
static void AUTH_PROTOCOL_PROC_Create_All_Threads(void);
static void AUTH_PROTOCOL_PROC_Daemonize_Entry(void* arg);
static void AUTH_PROTOCOL_PROC_Main_Thread_Function_Entry(void);

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for om thread
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for AUTH_PROTOCOL process
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for auth_protocol process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(AUTH_PROTOCOL_PROC_Init()==FALSE)
    {
        SYSFUN_Debug_Printf("AUTH_PROTOCOL_Process_Init fail.\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_AUTH_PROTOCOL_PROCESS_NAME,
                            AUTH_PROTOCOL_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("AUTH_PROTOCOL_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for auth_protocol process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T AUTH_PROTOCOL_PROC_Init(void)
{
     L_CMNLIB_INIT_InitiateProcessResources();

    /* In XXX_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if(AUTH_PROTOCOL_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    /* Register callback fun
     */
    AUTH_PROTOCOL_PROC_Create_InterCSC_Relation();

   /* Init PMGR
     */
#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_SNMP == TRUE)
  //  SNMP_PMGR_Init();
#endif
#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_AMTR==TRUE)
   if(AMTR_PMGR_InitiateProcessResource()==FALSE)
       return FALSE;

   if(AMTR_POM_InitiateProcessResource()==FALSE)
       return FALSE;
#endif

  //  BACKDOOR_MGR_InitateProcessResource();
  //  CLI_PMGR_InitiateProcessResource();
  //  CLI_POM_InitiateProcessResource();
 //   CLUSTER_PMGR_Init();
 //   CLUSTER_POM_Init();
 //   DBSYNC_TXT_PMGR_InitiateProcessResource();
    DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NMTRDRV_PMGR_InitiateProcessResource();
    DEV_SWDRV_PMGR_InitiateProcessResource();
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
    DEV_SWDRVL4_PMGR_InitiateProcessResource();
 //   DHCP_PMGR_InitiateProcessResources();
 //   DOT1X_PMGR_Init();
 //   DOT1X_POM_Init();
//    EXTBRG_PMGR_InitiateProcessResource();
 //   GARP_PMGR_InitiateProcessResource();
//    IF_PMGR_Init();
 //   IGMPSNP_PMGR_InitiateProcessResources();
    IML_PMGR_InitiateProcessResource();
//    L2MUX_PMGR_InitiateProcessResource();
 //   L4_PMGR_Init();
//    LACP_PMGR_InitiateProcessResource();
//    LACP_POM_InitiateProcessResource();
//    LLDP_PMGR_InitiateProcessResource();
//    MGMT_IP_FLT_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
    NETCFG_PMGR_MAIN_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();
//    NMTR_PMGR_Init();
//    NSM_PMGR_InitiateProcessResource();
//    PRI_PMGR_InitiateProcessResources();
//    PSEC_PMGR_InitiateProcessResources();
//    RADIUS_PMGR_InitiateProcessResources();
//    RADIUS_POM_InitiateProcessResources();
    STKCTRL_PMGR_InitiateProcessResources();
    STKTPLG_PMGR_InitiateProcessResources();
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();
 //   SYS_PMGR_InitiateProcessResource();
    SYSLOG_POM_InitiateProcessResource();
//    TACACS_PMGR_Init();
//    TACACS_POM_Init();
//    TELNET_PMGR_InitiateProcessResource();
//    TELNET_POM_InitiateProcessResource();
 //   TRK_PMGR_InitiateProcessResource();
  //  USERAUTH_PMGR_Init();
    VLAN_PMGR_InitiateProcessResource();
    VLAN_POM_InitiateProcessResource();
    XSTP_PMGR_InitiateProcessResource();
    XSTP_POM_InitiateProcessResource();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in auth_protocol process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T AUTH_PROTOCOL_PROC_InitiateProcessResource(void)
{
    if(AUTH_PROTOCOL_PROC_COMM_Initiate_System_Resource()==FALSE)
        return FALSE;

    AUTH_PROTOCOL_GROUP_InitiateProcessResources();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Relashionships between CSCs are created through function
 *    pointer registrations and callback functions.
 *    At this init phase, all operations related to function pointer
 *    registrations will be processed.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void AUTH_PROTOCOL_PROC_Create_InterCSC_Relation(void)
{
    AUTH_PROTOCOL_GROUP_Create_InterCSC_Relation();
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in AUTH_PROTOCOL process will be spawned in this function.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void AUTH_PROTOCOL_PROC_Create_All_Threads(void)
{
    AUTH_PROTOCOL_GROUP_Create_All_Threads();
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : XXX_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for XXX process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    The main thread of the process shall never be terminated before any other
 *    threads spawned in this process. If the main thread terminates, other
 *    threads in the same process will also be terminated.
 *
 *    The main thread is responsible for handling OM IPC request for read
 *    operations of All OMs in this process.
 *------------------------------------------------------------------------------
 */
static void AUTH_PROTOCOL_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_AUTH_PROTOCOL_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER, POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
#if (SYS_CPNT_TACACS == TRUE)
                case SYS_MODULE_TACACS:
                    need_resp = TACACS_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_AAA == TRUE)
                case SYS_MODULE_AAA:
                    need_resp = AAA_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif


#if (SYS_CPNT_RADIUS == TRUE)
                case SYS_MODULE_RADIUS:
                    need_resp = RADIUS_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                    /* Unknow command. There is no way to idntify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp=FALSE;
            }

            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
        }
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AUTH_PROTOCOL_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    After the process has been daemonized, the main thread of the process
 *    will call this function to start the main thread.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void AUTH_PROTOCOL_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    AUTH_PROTOCOL_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling IPC messages for this process
     */
    AUTH_PROTOCOL_PROC_Main_Thread_Function_Entry();

}

