/* MODULE NAME: L2_L4_PROC.C
 *
 * PURPOSE:
 *    This module implements the functionality of IP_SERVICE process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/11     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "l_cmnlib_init.h"
#include "sysfun.h"
#include "ip_service_proc_comm.h"
#include "ip_service_group.h"
#include "sysrsc_mgr.h"
#include "uc_mgr.h"
#include "dhcp_om.h"

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_om.h"
#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
#include "l4_pmgr.h"
#endif
#endif
#include "l_mm_backdoor.h"

#include "amtrl3_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "iml_pmgr.h"
#include "ipal_kernel.h"
#include "mib2_pmgr.h"
#include "netcfg_pmgr_main.h"
#include "netcfg_pom_main.h"
#include "stktplg_pom.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "syslog_pmgr.h"
#include "sys_pmgr.h"
#include "vlan_pmgr.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#include "dns_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(IP_SERVICE_PROC_OmMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef union
{
#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_OM_IPCMsg_T dhcpv6_om_ipcmsg;
#endif
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} IP_SERVICE_PROC_OmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T IP_SERVICE_PROC_Init(void);
static BOOL_T IP_SERVICE_PROC_InitiateProcessResources(void);
static void   IP_SERVICE_PROC_Create_InterCSC_Relation(void);
static void   IP_SERVICE_PROC_Daemonize_Entry(void* arg);
static void   IP_SERVICE_PROC_Create_All_Threads(void);
static void   IP_SERVICE_PROC_Main_Thread_Function_Entry(void);


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for om thread
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - main
 *-----------------------------------------------------------------------------
 * PURPOSE : The main entry for IP_SERVICE process
 *
 * INPUT   : argc --  the size of the argv array
 *           argv --  the array of arguments
 *
 * OUTPUT  : None.
 *
 * RETURN  : 0  -- Success
 *           -1 -- Error
 * NOTES   : This function is the entry point for IP_SERVICE process.
 *-----------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
	UI32_T process_id;

    /* Initialize the resource of IP_SERVICE process
     */
	if (IP_SERVICE_PROC_Init() == FALSE)
	{
		printf("\nIP_SERVICE_PROC_Init fail.\n");
        return -1;
    }

    /* Spawn the L2_L4 process
     */
	if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_IP_SERVICE_PROCESS_NAME,
                            IP_SERVICE_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("\nSYSFUN_SpawnProcess(IP_SERVICE process) error.\n");
        return -1;
    }

	return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_Init
 *-----------------------------------------------------------------------------
 * PURPOSE : Do initialization procedures for IP_SERVICE process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T IP_SERVICE_PROC_Init(void)
{
    /* In IP_SERVICE_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */

    L_CMNLIB_INIT_InitiateProcessResources();

    if (SYSRSC_MGR_AttachSystemResources() == FALSE)
    {
        printf("\n%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

	if (IP_SERVICE_PROC_InitiateProcessResources() == FALSE)
	{
		return FALSE;
	}

	/* Register callback functions
	 */
	IP_SERVICE_PROC_Create_InterCSC_Relation();

	return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the resources used in IP_SERVICE process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T IP_SERVICE_PROC_InitiateProcessResources(void)
{
    /* Init common resource
     */
    if (IP_SERVICE_PROC_COMM_InitiateProcessResource() == FALSE)
    {
        return FALSE;
    }

    /* Initiate UC_MGR because DHCP will use UC_MGR functions
     */
    if (UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf(" UC_MGR_InitiateProcessResources fail\n");
       return FALSE;
    }

    /* Init process resources for each member CSC group
     */
    IP_SERVICE_GROUP_InitiateProcessResources();

    /* Init PMGR
     */
    //AMTR_PMGR_InitiateProcessResource();
    //CLI_PMGR_InitiateProcessResource();
    //CLI_POM_InitiateProcessResource();
    //CLUSTER_PMGR_Init();
    //CLUSTER_POM_Init();
    //DBSYNC_TXT_PMGR_InitiateProcessResource();
    //DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NICDRV_PMGR_InitiateProcessResource();
    //DEV_NMTRDRV_PMGR_InitiateProcessResource();
    //DEV_SWDRV_PMGR_InitiateProcessResource();
    //DEV_SWDRVL3_PMGR_InitiateProcessResource();
    //DEV_SWDRVL4_PMGR_InitiateProcessResource();
    //DEVRM_PMGR_InitiateProcessResource();

#if (SYS_CPNT_DHCPSNP == TRUE)
    DHCPSNP_PMGR_InitiateProcessResource();
#endif

    //DHCP_PMGR_InitiateProcessResources();
    //DOT1X_PMGR_Init();
    //DOT1X_POM_Init();
    //EXTBRG_PMGR_InitiateProcessResource();
    //IF_PMGR_Init();
    //IGMPSNP_PMGR_InitiateProcessResources();
    IML_PMGR_InitiateProcessResource();
#if(SYS_CPNT_AMTRL3 == TRUE)
	AMTRL3_PMGR_InitiateProcessResource();
#endif
    //L2MUX_PMGR_InitiateProcessResource();
    //L4_PMGR_Init();
    //LACP_PMGR_InitiateProcessResource();
    //LACP_POM_InitiateProcessResource();
    //LLDP_PMGR_InitiateProcessResource();
    //MGMT_IP_FLT_PMGR_Init();
#if(SYS_CPNT_MIB2MGMT==TRUE)
    MIB2_PMGR_Init();
#endif
    NETCFG_PMGR_MAIN_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();
    //NMTR_PMGR_InitiateProcessResource();
    //NSM_PMGR_InitiateProcessResource();
    //PRI_PMGR_InitiateProcessResources();
    //PSEC_PMGR_InitiateProcessResources();
    //RADIUS_PMGR_InitiateProcessResources();
    //RADIUS_POM_InitiateProcessResources();
    //SNMP_PMGR_Init();
    //STKCTRL_PMGR_InitiateProcessResources();
    //STKTPLG_PMGR_InitiateProcessResources();
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();
    //SYS_PMGR_InitiateProcessResource();
    SYSLOG_PMGR_InitiateProcessResource();
    //SYSLOG_POM_InitiateProcessResource();
    //TACACS_PMGR_Init();
    //TACACS_POM_Init();
    //TELNET_PMGR_InitiateProcessResource();
    //TELNET_POM_InitiateProcessResource();
    //TRK_PMGR_InitiateProcessResource();
    //USERAUTH_PMGR_Init();
    VLAN_PMGR_InitiateProcessResource();
    //XSTP_PMGR_InitiateProcessResource();
    XSTP_POM_InitiateProcessResource();
    IPAL_Kernel_Init();
#if (SYS_CPNT_DNS == TRUE)
    if(DNS_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
    if(DNS_POM_InitiateProcessResource()==FALSE)
         return FALSE;
#endif

#if((SYS_CPNT_DHCPV6_RELAY == TRUE)||(SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE))
    L4_PMGR_Init();
#endif
#if (SYS_CPNT_DHCP_CLIENT_CLASSID == TRUE)  
    SYS_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_AUTO_CONFIG_STATIC_IP == TRUE)
    CLI_POM_InitiateProcessResource();
#endif
    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Relashionships between CSCs are created through function
 *           pointer registrations and callback functions.
 *           At this init phase, all operations related to function pointer
 *           registrations will be processed.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void IP_SERVICE_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

    IP_SERVICE_GROUP_Create_InterCSC_Relation();

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE : After the process has been daemonized, the main thread of the
 *           process will call this function to start the main thread.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *------------------------------------------------------------------------------
 */
static void IP_SERVICE_PROC_Daemonize_Entry(void* arg)
{
	/* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
	IP_SERVICE_PROC_Create_All_Threads();

	/* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
	IP_SERVICE_PROC_Main_Thread_Function_Entry();

	return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : Spaw all threads in IP_SERVICE process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void IP_SERVICE_PROC_Create_All_Threads(void)
{
    IP_SERVICE_GROUP_Create_All_Threads();

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_SERVICE_PROC_Main_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : The entry function of the main thread for IP_SERVICE process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : 1. The main thread of the process shall never be terminated before
 *              any other threads spawned in this process. If the main thread
 *              terminates, other threads in the same process will also be
 *              terminated.
 *           2. The main thread is responsible for handling OM IPC request for
 *              read operations of All OMs in this process.
 *-----------------------------------------------------------------------------
 */
static void IP_SERVICE_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if (SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY,
            SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);
        return;
    }

    /* create om ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_IP_SERVICE_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while (1)
    {
        /* wait for the request ipc message
         */
        if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER,
                POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            
            switch (msgbuf_p->cmd)
            {
#if (SYS_CPNT_DHCPV6 == TRUE)
                case SYS_MODULE_DHCPv6:
                    need_resp = DHCPv6_OM_HandleIPCReqMsg(msgbuf_p);
#endif /* #if (SYS_CPNT_DHCPV6 == TRUE) */
                    break;
#if (SYS_CPNT_DHCP == TRUE)
                case SYS_MODULE_DHCP:
                    need_resp = DHCP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("\n%s: Invalid IPC req cmd.\n", __FUNCTION__);
                    /* Unknow command. There is no way to idntify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp = FALSE;
            }

            if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle,
                    msgbuf_p) != SYSFUN_OK))
            {
                printf("\n%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);
            }
        }
        else
        {
            SYSFUN_Debug_Printf("\n%s: SYSFUN_ReceiveMsg fail.\n",
                __FUNCTION__);
        }
    }
}
