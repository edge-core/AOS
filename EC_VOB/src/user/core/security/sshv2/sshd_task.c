/* MODULE NAME:  sshd_task.c
* PURPOSE:
*   SSHD initiation and SSHD task creation
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-04-02      -- Isiah , created.
*     2007-06-26      -- Rich Lee, Porting to Linux Platform
* Copyright(C)      Accton Corporation, 2003
*/



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_stdlib.h"

#if 0//rich
#include "skt_vx.h"
#include "socket.h"
#include "iproute.h"
#endif
#include "cli_proc_comm.h"
#include "sshd_type.h"
#include "sshd_task.h"
#include "sshd_mgr.h"
#include "sshd_vm.h"
#include "cli_pmgr.h"
#include "l_threadgrp.h"
#if(SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

int close(int fd);
extern void sshd_main(SSHD_TASK_SessionThreadArgs *args);

/* NAMING CONSTANT DECLARATIONS
 */
#define SSHD_TASK_EVENT_ENTER_TRANSITION    BIT_1

#ifndef s_close /* for vxworks */
    #define s_close close
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* FUNCTION NAME:  SSHD_TASK_Main
 * PURPOSE:
 *                      SSHD starting routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_TASK_CreateTask().
 */
static void SSHD_TASK_Main(void);



/* FUNCTION NAME:  SSHD_TASK_EnterMainRoutine
 * PURPOSE:
 *                      SSHD task main routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_TASK_Main().
 */
static void SSHD_TASK_EnterMainRoutine(void);



/* FUNCTION NAME:  SSHD_TASK_IsProvisionComplete
 * PURPOSE:
 *          This function will check the SSHD module can start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *
 */
BOOL_T SSHD_TASK_IsProvisionComplete(void);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
static void SSHD_TASK_CheckSoftwareWatchdogEvent(void);
#endif


/* STATIC VARIABLE DECLARATIONS
 */
static  UI32_T  sshd_task_main_id;
static  UI32_T  member_id;
static  L_THREADGRP_Handle_T        tg_handle;
static  BOOL_T  sshd_task_is_port_changed = FALSE;
static  BOOL_T  is_transition_done;
static  BOOL_T  is_provision_complete = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_TASK_Init
 * PURPOSE:
 *          This function init the message queue.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_InitiateSystemResources.
 */
BOOL_T SSHD_TASK_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    is_transition_done = FALSE;

    return TRUE;
}



/* FUNCTION NAME:  SSHD_TASK_CreateTask
 * PURPOSE:
 *                      This function create sshd main task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in SSHD_INIT_CreateTasks().
 */
BOOL_T SSHD_TASK_CreateTask(void)
{
        /* LOCAL CONSTANT DECLARATIONS
        */

        /* LOCAL VARIABLES DECLARATIONS
        */
#if 0 /* rich mask for telnet */
        /* BODY */
        if( SYSFUN_SpawnTask(   SYS_BLD_SSHD_MAIN_TASK,
                                SYS_BLD_SSHD_MAIN_TASK_PRIORITY,
                                SYS_BLD_TASK_COMMON_STACK_SIZE,
                                0,
                                SSHD_TASK_Main,
                                0,
                                &sshd_task_main_id) != SYSFUN_OK )
        {
                return FALSE;
        } /* End of if */
#else
 if(SYSFUN_SpawnThread(SYS_BLD_SSHD_CSC_THREAD_PRIORITY,
                          SYS_BLD_SSHD_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_SSHD_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          SSHD_TASK_Main,
                          0,
                          &sshd_task_main_id)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SSH_PARENT, sshd_task_main_id, SYS_ADPT_SSH_PARENT_SW_WATCHDOG_TIMER );
#endif

        return TRUE;

} /* end of SSHD_TASK_CreateTask() */



/* FUNCTION NAME : SSHD_TASK_SetTransitionMode
 * PURPOSE:
 *              Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void SSHD_TASK_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_transition_done = FALSE;
    SYSFUN_SendEvent(sshd_task_main_id, SSHD_TASK_EVENT_ENTER_TRANSITION);

	while(is_transition_done != TRUE)
		SYSFUN_Sleep(10);
}



/* FUNCTION NAME : SSHD_TASK_EnterTransitionMode
 * PURPOSE:
 *              Leave CSC Task while transition done.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void SSHD_TASK_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    /*  want task release all resources */
      SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
}



/* FUNCTION NAME:  SSHD_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the SSH module to start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *          This function is invoked in SSHD_INIT_ProvisionComplete().
 */
void SSHD_TASK_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_provision_complete = TRUE;
}



/* FUNCTION NAME:  SSHD_TASK_RegisterCallBackFunction
 * PURPOSE:
 *                      Register Callback Function.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_INIT_RegisterCallBackFunction().
 */
void SSHD_TASK_RegisterCallBackFunction(void)
{
        /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  SSHD_TASK_Main
 * PURPOSE:
 *                      SSHD starting routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_TASK_CreateTask().
 */
static void SSHD_TASK_Main(void)
{
   /* LOCAL CONSTANT DECLARATIONS
    */

   /* LOCAL VARIABLES DECLARATIONS
    */
    UI32_T                      wait_events, rcv_events, op_mode;
    UI32_T                      event_var, ret_value, member_id, sshd_state;

    /* BODY */

    /* join the thread group
     */
    tg_handle = CLI_PROC_COMM_GetSSH_GROUPTGHandle();
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_SSH_GROUP_MGR_THREAD_PRIORITY, &member_id)==FALSE)
    {
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /*  Prepare waiting event and init. event var.      */
    wait_events = SSHD_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

    while (TRUE)
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        SSHD_TASK_CheckSoftwareWatchdogEvent();
#endif

        /* request thread group execution permission
         */
        if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

        op_mode    = SSHD_MGR_GetOperationMode();
        sshd_state = SSHD_MGR_GetSshdStatus();

        /* release thread group execution permission
         */
        if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

        switch ( op_mode )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:

                if( SSHD_TASK_IsProvisionComplete() == FALSE )
                {
                    SYSFUN_Sleep(10);
                    break;
                }
                /* The SSHD_TASK_Enter_Main_Routine() is a forever loop, and will
                 * return (exit) only when SSHD subsystem enters TRANSITION_MODE/SLAVE_MODE.
                 */
                {
                    /* BODY
                     * Note: This is poor design!
                     */
                    if ( sshd_state != SSHD_STATE_DISABLED )
                    {
                        SSHD_TASK_EnterMainRoutine();
                    }
                }
                SYSFUN_Sleep(10);
                break;

            case SYS_TYPE_STACKING_TRANSITION_MODE:
                if ( (ret_value = SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY,
                                                 SYSFUN_TIMEOUT_NOWAIT, &rcv_events)) != SYSFUN_OK )
                {
                    SYSFUN_Sleep(10);
                    break;
                }

                event_var |= rcv_events;

                if (event_var==0)
                {
                    /*      Log to system : ERR--Receive Event Failure */
                    SYSFUN_Sleep(10);
                    break;
                }

                if (event_var & SSHD_TASK_EVENT_ENTER_TRANSITION )
                {
                    /* Check all ssh connection had disconnect */
                    while(1)
                    {
                        if( SSHD_VM_GetCreatedSessionNumber() == 0 )
                        {
                            break;
                        }
                        else
                        {
                            SYSFUN_Sleep(10);
                        }
                    }

                    is_transition_done    = TRUE;      /* Turn on the transition done flag */
                    event_var             = 0;
                    is_provision_complete = FALSE;
                }
                SYSFUN_Sleep(10);
                break;

            case SYS_TYPE_STACKING_SLAVE_MODE:
                /* Release allocated resource */
                event_var = 0;
                is_provision_complete = FALSE;
                SYSFUN_Sleep(10);
                break;

            default:
                /* log error; */
                SYSFUN_Sleep(10);
                break;

        } /* End of switch */

    } /* End of while */

} /* End of SSHD_TASK_Main () */



/* FUNCTION NAME:  SSHD_TASK_EnterMainRoutine
 * PURPOSE:
 *                      SSHD task main routine.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          This function is invoked in SSHD_TASK_Main().
 */
static void SSHD_TASK_EnterMainRoutine(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DECLARATIONS
     */
    struct  sockaddr_in6    server_addr, client_addr;
    int                     server_addrlen;
    int                     sockfd, newsockfd, ret, i, on = 1, id =0;
    socklen_t               addr_len;
    UI32_T                  port, task_id, op_mode, sshd_state;
    fd_set                  read_fds;
    struct  timeval         timeout;
    UI8_T                   name[6];

    /* BODY */

    /* Initiate socket */

    port = SSHD_MGR_GetSshdPort();
    memset((UI8_T *) &server_addr,0, sizeof(server_addr));

#if (SYS_CPNT_IPV6 == TRUE)
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (0 <= sockfd)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&server_addr;

        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = L_STDLIB_Hton16(port);

        server_addrlen = sizeof(struct sockaddr_in6);
    }
    else
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */
    {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (0 <= sockfd)
        {
            struct sockaddr_in *sin = (struct sockaddr_in*)&server_addr;

            sin->sin_family = AF_INET;
            sin->sin_port = L_STDLIB_Hton16(port);

            server_addrlen = sizeof(struct sockaddr_in);
        }
        else
        {
            return;
        }
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        SYSFUN_Debug_Printf("%s: SetSockOpt fail.\n", __FUNCTION__);
        s_close(sockfd);
        return;
    }

    if ( bind(sockfd, (struct sockaddr *)&server_addr, server_addrlen) < 0 )
    {
        s_close(sockfd);
        return;
    }

    /* request thread group execution permission
     */
    if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

    /*port = SSHD_MGR_GetSshdPort();*/

    /* release thread group execution permission
     */
    if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

    /* listen */
    if(listen (sockfd, 1) < 0)
    {
        s_close(sockfd);    /* close socket */
        return;
    }

    /* set socket options */
    setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, (UI8_T *)&on, sizeof(on));

    while ( 1 )
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        SSHD_TASK_CheckSoftwareWatchdogEvent();
#endif

        /* request thread group execution permission
         */
        if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);

        op_mode     = SSHD_MGR_GetOperationMode();
        sshd_state  = SSHD_MGR_GetSshdStatus();

        /* release thread group execution permission
         */
        if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);

        if (!((op_mode == SYS_TYPE_STACKING_MASTER_MODE) && (sshd_state != SSHD_STATE_DISABLED)))
            break;

        /* prepare select */
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);

        timeout.tv_sec = 1;     /*  no.  of seconds  */
        timeout.tv_usec = 0;    /*  no. of micro seconds  */

        /* select */
        ret = select(sockfd+1, &read_fds, NULL, NULL, &timeout);
        if (ret < 0 )
        {
             break;
        }

        if ( (ret > 0) && (FD_ISSET(sockfd, &read_fds)) )
        {
            SSHD_TASK_SessionThreadArgs *args_p;

            /* accept a new connection */
            addr_len = (socklen_t)sizeof(client_addr);
            newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
            if (newsockfd < 0)
            {
                s_close(sockfd);    /* close socket */
                return;
            }

#if(SYS_CPNT_MGMT_IP_FLT == TRUE)
            {
                L_INET_AddrIp_T   inet_client;

                memset(&inet_client, 0, sizeof(inet_client));
                L_INET_SockaddrToInaddr((struct sockaddr *)&client_addr, &inet_client);

                if( MGMT_IP_FLT_IsValidIpFilterAddress(MGMT_IP_FLT_TELNET, &inet_client) == FALSE )
                {
                    s_close(newsockfd);
                    continue;
                }
            }

#endif/*SYS_CPNT_MGMT_IP_FLT == TRUE*/

            /* proceed only if not exceeding max. # of concurrent sessions */
/* isiah.2004-01-02*/
/*move session number to CLI */
/*         if ( SSHD_VM_GetCreatedSessionNumber() >= SSHD_DEFAULT_MAX_SESSION_NUMBER )*/
            if ( CLI_PMGR_IncreaseRemoteSession() == FALSE )
            {
                s_close(newsockfd);
                continue;
            }

            setsockopt(newsockfd, SOL_SOCKET, SO_KEEPALIVE, (UI8_T *)&on, sizeof(on));

            for ( i = 0 ; i < 100 ; i++ )
            {
                if ( ++id > 99 )
                {
                    id = 1;
                }
                sprintf((char *)name, SYS_BLD_SSHD_CHILD_TASK, id);
                if (SYSFUN_TaskNameToID((char *)name, &task_id))
                {
                    break;
                }
            }

            if (i >= 100)
            {
                SYSFUN_Debug_Printf("sshd_task : No system resources for a new session\r\n");
                s_close(newsockfd);
                CLI_PMGR_DecreaseRemoteSession();
                continue;
            }

            args_p = malloc(sizeof(*args_p));
            if (NULL == args_p)
            {
                SYSFUN_Debug_Printf("sshd_task : Cannot allocate memory for argument\r\n");
                s_close(newsockfd);
                CLI_PMGR_DecreaseRemoteSession();
                continue;
            }
            memset(args_p, 0, sizeof(*args_p));
            args_p->accepted_socket = newsockfd;

            if(SYSFUN_SpawnThread(
                        SYS_BLD_SSHD_CSC_CHILD_THREAD_PRIORITY,
                        SYS_BLD_SSHD_CSC_CHILD_THREAD_SCHED_POLICY,
                        (char *)name,
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-19, 17:15:00 */
                        (SYS_BLD_SSHD_CHILD_TASK_STACK_SIZE * 2),
#else
                        SYS_BLD_SSHD_CHILD_TASK_STACK_SIZE,
#endif /* ES3526MA_POE_7LF_LN */
                        SYSFUN_TASK_NO_FP,
                        sshd_main,
                        args_p,
                        &task_id)!=SYSFUN_OK)
            {
                s_close(newsockfd);
/* isiah.2004-01-02*/
/*move session number to CLI */
                CLI_PMGR_DecreaseRemoteSession();
                free(args_p);
                continue;
            }   /*  end of if (SYSFUN_SpawnTask())  */
        } /* end of if ( (ret > 0) && (FD_ISSET(sockfd, &read_fds)) )*/

        /* check for sshd_socket reopen */
        if ( sshd_task_is_port_changed == TRUE )
        {
            sshd_task_is_port_changed = FALSE;
            break;
        }

    } /* end of while */

    s_close(sockfd); /* close socket */

    for ( i=0 ; i<SYS_ADPT_MAX_SSH_NUMBER ; i++ )
    {
        if ( SSHD_VM_CheckSshConnection(i) == TRUE )
        {
            CLI_PMGR_SetKillWorkingSpaceFlag(i);
            SYSFUN_Sleep(10);
        }
    }
}



/* FUNCTION NAME:  SSHD_TASK_IsProvisionComplete
 * PURPOSE:
 *          This function will check the SSHD module can start.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *
 */
BOOL_T SSHD_TASK_IsProvisionComplete(void)
{
        /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return( is_provision_complete );
}

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
/* FUNCTION NAME:  SSHD_TASK_CheckSoftwareWatchdogEvent
 * PURPOSE:
 *          This function is used to check and response to software watchdog
 *          event.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 *
 * NOTES:
 *
 */
static void SSHD_TASK_CheckSoftwareWatchdogEvent(void)
{
    UI32_T rc;
    UI32_T event=0;

    rc=SYSFUN_ReceiveEvent(SYSFUN_SYSTEM_EVENT_SW_WATCHDOG,
        SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_NOWAIT,
        &event);

    if(rc!=SYSFUN_OK && rc!=SYSFUN_RESULT_TIMEOUT)
    {
        SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveEvent Error (ret=%lu) \r\n", __FUNCTION__, rc);
    }

    if (event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
    {
         SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SSH_PARENT);
         event ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
    }
}
#endif




