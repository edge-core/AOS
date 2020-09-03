/* MODULE NAME:  sys_mgmt_proc.c
 * PURPOSE:
 *    for sys mgmt process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/05/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "sys_mgmt_proc_comm.h"
#include "sys_cpnt.h"
#include "backdoor_mgr.h"
#include "sysrsc_mgr.h"
#include "l_cmnlib_init.h"
#include "sys_mgmt_group.h"
#include "l_mm_backdoor.h"

#if 0 /* rich mask for linux first build */
#include "sys_mgr_init.h"
#endif /* for temp build error , rich for linux */

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_type.h"
#include "cluster_om.h"
#include "cluster_group.h"
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
#include "mib2_om.h"
#endif
#include "radius_om.h"
#include "radius_pom.h"
#include "radius_pmgr.h"
#include "tacacs_pmgr.h"

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
#if(SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
#include "led_pmgr.h"
#endif
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#include "snmp_pmgr.h"
#include "nmtr_pmgr.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#if (SYS_CPNT_AAA == TRUE)
#include "aaa_pmgr.h"
#include "aaa_pom.h"
#endif
#include "trk_pmgr.h"
#include "uc_mgr.h"
#include "l4_pmgr.h"

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
#include "oam_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for MGR thread to receive and response
 * PMGR ipc. The size of this buffer should pick the maximum of size required
 * for PMGR ipc request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */

/* cluster mgmt group
 */
#define SYS_MGMT_PROC_CLUSTER_MGMT_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(CLUSTER_MGMT_MGR_IPCMSG_T)

/* sys mgmt group
 */
#define SYS_MGMT_PROC_SYS_MGMT_GROUP_PMGR_MSGBUF_MAX_REQUIRED_SIZE sizeof(SYS_MGMT_MGR_IPCMSG_T)

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(SYS_MGMT_PROC_OM_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union SYS_MGMT_PROC_OM_MSG_U
{
#if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_OM_IPCMsg_T cluster_om_ipcmsg;
#endif
#if (SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_OM_IPCMsg_T mib2_om_ipcmsg;
#endif
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} SYS_MGMT_PROC_OM_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SYS_MGMT_PROC_Init(void);
static BOOL_T SYS_MGMT_PROC_InitiateProcessResource(void);
static void   SYS_MGMT_PROC_Create_InterCSC_Relation(void);
static void   SYS_MGMT_PROC_Create_All_Threads(void);
static void   SYS_MGMT_PROC_Daemonize_Entry(void* arg);

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
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
 *    the main entry for SYS_MGMT process
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
 *    This function is the entry point for SYS_MGMT process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(SYS_MGMT_PROC_Init()==FALSE)
    {
        printf("SYS_MGMT_Process_Init fail.\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_SYS_MGMT_PROCESS_NAME,
                            SYS_MGMT_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("SYS_MGMT_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for SYS_MGMT process.
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
static BOOL_T SYS_MGMT_PROC_Init(void)
{
    /* In SYS_MGMT_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if(SYS_MGMT_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    SYS_MGMT_PROC_Create_InterCSC_Relation();

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_SNMP == TRUE)
    SNMP_PMGR_Init();
#endif
#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_AMTR == TRUE)
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
    NMTR_PMGR_InitiateProcessResource();
 //   DHCP_PMGR_InitiateProcessResources();
 //   DOT1X_PMGR_Init();
 //   DOT1X_POM_Init();
//    EXTBRG_PMGR_InitiateProcessResource();
//    IF_PMGR_Init();
 //   IGMPSNP_PMGR_InitiateProcessResources();
//    IML_PMGR_InitiateProcessResource();
//    L2MUX_PMGR_InitiateProcessResource();
      L4_PMGR_Init();
//    LACP_PMGR_InitiateProcessResource();
//    LACP_POM_InitiateProcessResource();

#if (SYS_CPNT_LLDP==TRUE)
    LLDP_PMGR_InitiateProcessResources();
    LLDP_POM_InitiateProcessResources();
#endif

//    MGMT_IP_FLT_PMGR_Init();
    MIB2_PMGR_Init();
    NETCFG_PMGR_MAIN_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();
//    NMTR_PMGR_Init();
//    NSM_PMGR_InitiateProcessResource();
//    PRI_PMGR_InitiateProcessResources();
//    PSEC_PMGR_InitiateProcessResources();

    RADIUS_PMGR_InitiateProcessResources();
    RADIUS_POM_InitiateProcessResources();

#if (SYS_CPNT_AAA == TRUE)
    AAA_PMGR_Init();
    AAA_POM_Init();
#endif

    STKCTRL_PMGR_InitiateProcessResources();
    STKTPLG_PMGR_InitiateProcessResources();
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();

#if(SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    LED_PMGR_InitiateProcessResource();
#endif
 //   SYS_PMGR_InitiateProcessResource();
    SYSLOG_POM_InitiateProcessResource();
    TACACS_PMGR_Init();
//    TACACS_POM_Init();
//    TELNET_PMGR_InitiateProcessResource();
//    TELNET_POM_InitiateProcessResource();
    TRK_PMGR_InitiateProcessResource();
  //  USERAUTH_PMGR_Init();
    VLAN_PMGR_InitiateProcessResource();
    VLAN_POM_InitiateProcessResource();
    XSTP_PMGR_InitiateProcessResource();
    XSTP_POM_InitiateProcessResource();

#if (SYS_CPNT_CFM == TRUE)
    CFM_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    if(EFM_OAM_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in SYS_MGMT process.
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
static BOOL_T SYS_MGMT_PROC_InitiateProcessResource(void)
{
    if(SYS_MGMT_PROC_COMM_InitiateProcessResource()==FALSE)
        return FALSE;

    if(FALSE==UC_MGR_InitiateProcessResources())
    {
        printf("\r\n%s: UC_MGR_InitiateProcessResources fails", __FUNCTION__);
        return FALSE;
    }

    SYS_MGMT_GROUP_InitiateProcessResources();

#if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_GROUP_InitiateProcessResources();
#endif

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_Create_InterCSC_Relation
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
static void SYS_MGMT_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

    SYS_MGMT_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_GROUP_Create_InterCSC_Relation();
#endif

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in SYS_MGMT process will be spawned in this function.
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
static void SYS_MGMT_PROC_Create_All_Threads(void)
{
 #if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_GROUP_Create_All_Threads();
#endif
    SYS_MGMT_GROUP_Create_All_Threads();
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_MGMT_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for SYS_MGMT process.
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
static void SYS_MGMT_PROC_Main_Thread_Function_Entry(void)
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
    if(SYSFUN_CreateMsgQ(SYS_BLD_SYS_MGMT_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
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
#if (SYS_CPNT_CLUSTER == TRUE)
                case SYS_MODULE_CLUSTER:
                    need_resp = CLUSTER_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_MIB2MGMT==TRUE)
                case SYS_MODULE_MIB2MGMT:
                    need_resp = MIB2_OM_HandleIPCReqMsg(msgbuf_p);
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
 * ROUTINE NAME : SYS_MGMT_PROC_Daemonize_Entry
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
static void SYS_MGMT_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    SYS_MGMT_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    SYS_MGMT_PROC_Main_Thread_Function_Entry();

}

