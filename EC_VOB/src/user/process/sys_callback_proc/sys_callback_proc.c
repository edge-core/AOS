/* MODULE NAME: SYS_CALLBACK_PROC.C
 *
 * PURPOSE:
 *    This module implements the functionality of SYS_CALLBACK process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2013/05/21     --- Jimi, Create
 *
 * Copyright(C)      EdgeCore Corporation, 2013
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_cmnlib_init.h"
#include "sysrsc_mgr.h"
#include "l_mm_backdoor.h"
#include "sys_callback_proc_comm.h"
#include "dev_nicdrv_pmgr.h"
#include "swctrl_pom.h"
#include "amtr_pmgr.h"

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pmgr.h"
#include "ipsg_pom.h"
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_pmgr.h"
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(SYS_CALLBACK_PROC_OmMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef union
{
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} SYS_CALLBACK_PROC_OmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T SYS_CALLBACK_PROC_Init(void);
static BOOL_T SYS_CALLBACK_PROC_Init_PROC_InitiateProcessResources(void);
static BOOL_T SYS_CALLBACK_PROC_InitiateProcessResources(void);
static void   SYS_CALLBACK_PROC_Init_PROC_Create_InterCSC_Relation(void);
static void   SYS_CALLBACK_PROC_Init_PROC_Daemonize_Entry(void* arg);
static void   SYS_CALLBACK_PROC_Init_PROC_Create_All_Threads(void);
static void   SYS_CALLBACK_PROC_Init_PROC_Main_Thread_Function_Entry(void);
static void   SYS_CALLBACK_PROC_Daemonize_Entry(void* arg);
static void   SYS_CALLBACK_PROC_Create_InterCSC_Relation(void);
static void   SYS_CALLBACK_PROC_Create_All_Threads(void);
static void   SYS_CALLBACK_PROC_Main_Thread_Function_Entry(void);


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for om thread
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME : main
 * ------------------------------------------------------------------------ 
 * PURPOSE: The main entry for SYS_CALLBACK process
 * INPUT  : argc --  the size of the argv array
 *          argv --  the array of arguments
 * OUTPUT : none
 * RETURN : 0  -- Success
 *          -1 -- Error
 * NOTES  : This function is the entry point for SYS_CALLBACK process.
 * ------------------------------------------------------------------------ 
 */
int main(int argc, char* argv[])
{
	UI32_T process_id;

    /* Initialize the resource of SYS_CALLBACK process
     */
	if (SYS_CALLBACK_PROC_Init() == FALSE)
	{
		printf("\nSYS_CALLBACK_PROC_Process_Init fail.\n");
        return -1;
    }

    /* Spawn the SYS_CALLBACK process
     */
	if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_SYS_CALLBACK_PROCESS_NAME,
                            SYS_CALLBACK_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("\nSYSFUN_SpawnProcess(SYS_CALLBACK process) error.\n");
        return -1;
    }

	return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */
/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_Init
 * ------------------------------------------------------------------------ 
 * PURPOSE: Do initialization procedures for SYS_CALLBACK process.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE  -- Success
 *          FALSE -- Fail
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static BOOL_T SYS_CALLBACK_PROC_Init(void)
{
    /* In SYS_CALLBACK_PROC_Init(), following operations shall be
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

	if (SYS_CALLBACK_PROC_InitiateProcessResources() == FALSE)
	{
		return FALSE;
	}

	/* Register callback functions
	 */
	SYS_CALLBACK_PROC_Create_InterCSC_Relation();

    /* Init PMGR
     */
	return TRUE;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_InitiateProcessResources
 * ------------------------------------------------------------------------ 
 * PURPOSE: Initialize the resource used in SYS_CALLBACK process.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : TRUE  -- Success
 *          FALSE -- Fail
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static BOOL_T SYS_CALLBACK_PROC_InitiateProcessResources(void)
{
    /* Init common resource
     */
    if (SYS_CALLBACK_PROC_COMM_InitiateProcessResources() == FALSE)
    {
        return FALSE;
    }

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
	DHCPSNP_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    IPSG_PMGR_InitiateProcessResource();
    IPSG_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCP_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_POM_Init();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_AMTR == TRUE)
    AMTR_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_POM_InitiateProcessResource();
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

    return TRUE;
}


/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_Create_InterCSC_Relation
 * ------------------------------------------------------------------------ 
 * PURPOSE: Relashionships between CSCs are created through function
 *          pointer registrations and callback functions.
 *          At this init phase, all operations related to function pointer
 *          registrations will be processed.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_Daemonize_Entry
 * ------------------------------------------------------------------------ 
 * PURPOSE: After the process has been daemonized, the main thread of the
 *          process will call this function to start the main thread.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_PROC_Daemonize_Entry(void* arg)
{
	/* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
	SYS_CALLBACK_PROC_Create_All_Threads();

	/* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
	SYS_CALLBACK_PROC_Main_Thread_Function_Entry();

	return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_Create_All_Threads
 * ------------------------------------------------------------------------ 
 * PURPOSE: Spaw all threads in SYS_CALLBACK process.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_PROC_Create_All_Threads(void)
{	
    extern void SYS_CALLBACK_GROUP_Create_All_Threads(void);
    SYS_CALLBACK_GROUP_Create_All_Threads();
    return;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_PROC_Main_Thread_Function_Entry
 * ------------------------------------------------------------------------ 
 * PURPOSE: The entry function of the main thread for SYS_CALLBACK process.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : 1. The main thread of the process shall never be terminated before
 *             any other threads spawned in this process. If the main thread
 *             terminates, other threads in the same process will also be
 *             terminated.
 *          2. The main thread is responsible for handling OM IPC request for
 *             read operations of All OMs in this process.
 * ------------------------------------------------------------------------ 
 */
static void SYS_CALLBACK_PROC_Main_Thread_Function_Entry(void)
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
    if (SYSFUN_CreateMsgQ(SYS_BLD_SYS_CALLBACK_PROC_OM_IPCMSGQ_KEY,
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
                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;
					
                default:
                    printf("\n%s: Invalid IPC req cmd:%d.\n", __FUNCTION__, msgbuf_p->cmd);
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

