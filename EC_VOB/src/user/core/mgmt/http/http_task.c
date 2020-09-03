/*
 * Project Name: Mercury
 * Module Name : HTTP_TASK.C
 * Abstract    : to be included in root.c
 * Purpose     : HTTP initiation and HTTP task creation
 *
 * History :
 *          Date        Modifier        Reason
 *        2007-07-10    Rich Lee        Porting to Linux
 * Copyright(C)      Accton Corporation, 2001
 *
 * Note    :
 */

/*------------------------------------------------------------------------
 * INCLUDE STRUCTURES
 *------------------------------------------------------------------------
 */
#include <signal.h>

#include "http_loc.h"

#include "syslog_type.h"
#include "syslog_mgr.h"

#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pom.h"
#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define HTTP_TASK_EVENT_ENTER_TRANSITION    BIT_1
#define HTTP_TASK_SOCK_INVALID              -1

/* DATA TYPE DECLARATIONS
 */

/*------------------------------------------------------------------------
 * GLOBAL VARIABLES
 *-----------------------------------------------------------------------*/
static UI32_T http_task_is_port_changed = FALSE;


/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_transition_done;
static UI32_T http_task_id;
static BOOL_T is_provision_complete = FALSE;

#if (SYS_CPNT_HTTPS == TRUE)

static UI32_T http_task_is_security_port_changed = FALSE;

#endif /* SYS_CPNT_HTTPS */


/*------------------------------------------------------------------------
 * LOCAL FUNCTION PROTOTYPES
 *------------------------------------------------------------------------
 */
static void HTTP_TASK_Main (void*);
static int HTTP_TASK_Socket(struct sockaddr *sa);
static void HTTP_TASK_SetSockPort(UI16_T port, struct sockaddr *sa);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
static void HTTP_TASK_CheckSoftwareWatchdogEvent(void);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */

void HTTP_TASK_OnIdle();


/* FUNCTION NAME:  HTTP_TASK_IsProvisionComplete
 * PURPOSE:
 *          This function will check the HTTP module can start.
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
BOOL_T HTTP_TASK_IsProvisionComplete(void);


int nthread;
pthread_t threads[HTTP_CFG_TOTAL_WORKER + 1];

/* EXPORTED SUBPROGRAM BODIES
 */

int SpawnThread(void (*fn)(HTTP_Worker_T *), HTTP_Worker_T *param) {
    UI32_T rc;
    UI32_T tid;

    rc = SYSFUN_SpawnThread(SYS_BLD_HTTP_CSC_THREAD_PRIORITY,
                            SYS_BLD_HTTP_CSC_THREAD_SCHED_POLICY,
                            "HTTP Worker",
                            SYS_BLD_HTTP_TASK_STACK_SIZE,
                            SYSFUN_TASK_NO_FP,
                            fn,
                            param,
                            &tid);
    if (rc != SYSFUN_OK) {
        printf("%s %d: SYSFUN_SpawnThread() fail, rc %lu, errno %d !!!\r\n", __FUNCTION__, __LINE__, rc, errno);
    }
    return 0;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Create_Task
 *------------------------------------------------------------------------
 * PURPOSE: This function creates the HTTP task for network
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Create_Task ()
{
   if( SYSFUN_SpawnThread(   SYS_BLD_HTTP_CSC_THREAD_PRIORITY,
                              SYS_BLD_HTTP_CSC_THREAD_SCHED_POLICY,
                              SYS_BLD_HTTP_CSC_THREAD_NAME,
#if defined ES3526MA_POE_7LF_LN
                              (SYS_BLD_TASK_COMMON_STACK_SIZE * 8),
#else
                              SYS_BLD_HTTP_TASK_STACK_SIZE,
#endif
                              SYSFUN_TASK_NO_FP,
                              HTTP_TASK_Main,
                              0,
                              &http_task_id) != SYSFUN_OK )

   {
       EH_MGR_Handle_Exception(SYS_MODULE_HTTP, HTTP_TASK_Create_Task_FUNC_NO, EH_TYPE_MSG_TASK_CREATE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
   } /* End of if */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_HTTP, http_task_id, SYS_ADPT_HTTP_SW_WATCHDOG_TIMER);
#endif
} /* end of HTTP_TASK_CreateTask () */

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Port_Changed_Callback
 *------------------------------------------------------------------------
 * PURPOSE: This function is an event callback when the port is changed
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Port_Changed (void)
{
    http_task_is_port_changed = TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_RifDestroyed
 *------------------------------------------------------------------------
 * PURPOSE: This function is an event callback when the RIF is destroyed
 *.
 * INPUT   : ip_address
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_RifDestroyed(L_INET_AddrIp_T *ip_addr_p)
{
    int i;

    for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
    {
        HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);

        ASSERT(worker != NULL);
        if (worker == NULL)
        {
            continue;
        }

        http_worker_ctrl(worker, HTTP_WORKER_DEL_CONNECTION, ip_addr_p);
    }
}

/* FUNCTION NAME:  HTTP_TASK_Init
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
 *          This function is invoked in HTTP_INIT_Initiate_System_Resources.
 */
BOOL_T HTTP_TASK_Init(void)
{
    is_transition_done = FALSE;

    return TRUE;
}

/* FUNCTION NAME : HTTP_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
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
void HTTP_TASK_SetTransitionMode()
{
    is_transition_done = FALSE;
    SYSFUN_SendEvent(http_task_id, HTTP_TASK_EVENT_ENTER_TRANSITION);
}

/* FUNCTION NAME : HTTP_TASK_EnterTransitionMode
 * PURPOSE:
 *		Leave CSC Task while transition done.
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
void HTTP_TASK_EnterTransitionMode()
{
    /*	want task release all resources	*/
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
}

#if (SYS_CPNT_HTTPS == TRUE)
/* FUNCTION NAME:  HTTP_TASK_Secure_Port_Changed
 * PURPOSE:
 *			This function set flag when security port is changed.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          none.
 * NOTES:
 *          (Something must be known to use this function.)
 */
void HTTP_TASK_Secure_Port_Changed()
{
    http_task_is_security_port_changed = TRUE;
}
#endif /* SYS_CPNT_HTTPS */

/* FUNCTION NAME:  HTTP_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the HTTP module to start.
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
 *          This function is invoked in HTTP_INIT_ProvisionComplete().
 */
void HTTP_TASK_ProvisionComplete(void)
{
    is_provision_complete = TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void HTTP_TASK_LoadConfig()
{
    char config_file_path[HTTP_CONFIG_FILE_PATH_MAX_LEN + 1];

    if (FALSE == HTTP_OM_GetConfigFilePath(config_file_path, sizeof(config_file_path)))
    {
        printf("error: failed to get config file path\r\n");
    }
    else
    {
        HTTP_MGR_LoadConfig(config_file_path);
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Main
 *------------------------------------------------------------------------
 * FUNCTION: HTTP starting routine
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : 1.called by HTTP_TASK_Create
 *------------------------------------------------------------------------
 */
static void HTTP_TASK_Main (void *param)
{
    UI32_T	                wait_events;
    UI32_T	                rcv_events;
    UI32_T	                event_var;
    UI32_T	                ret_value, member_id;
    int     check_ssl=0;

#ifdef UNICORN
   UI8_T  *oem_buf;
   if((oem_buf =(UI8_T *)UC_MGR_Allocate (UC_MGR_OEM_LOGO_INDEX,
                                              CLI_DEF_OEM_LOGO_UC_MEM_SIZE,
                                              CLI_DEF_RUNTIME_PROVISION_UC_MEM_BOUNDRY)) == NULL)
   {
      printf("Fail to allocate resource.\r\n");
      return FALSE;
   }
#endif
    /*	Prepare waiting event and init. event var.	*/
    wait_events = HTTP_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

    /* HTTP_TASK_Body: */
    while (TRUE)
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        HTTP_TASK_CheckSoftwareWatchdogEvent();
#endif

        switch ( HTTP_MGR_GetOperationMode () )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:
                if( HTTP_TASK_IsProvisionComplete() == FALSE )
                {
        			SYSFUN_Sleep(10);
             		break;
                }
                /* The HTTP_TASK_Enter_Main_Routine() is a forever loop, and will
                 * return (exit) only when HTTP subsystem enters TRANSITION_MODE/SLAVE_MODE.
                 */
                {
                    /* BODY
                     * Note: This is poor design!
                     */
#ifdef UNICORN
                    if(strcmp(oem_buf, "japan") != 0 && strcmp(oem_buf, "europe") != 0)
                    {
                        SYSFUN_Sleep(10);
                        continue;
                    }
#endif

                   HTTP_TASK_LoadConfig();

#if (SYS_CPNT_HTTPS == TRUE)
                    HTTP_MGR_Get_Certificate();
#endif /* SYS_CPNT_HTTPS */

                    HTTP_TASK_Enter_Main_Routine();
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
                	/*	Log to system : ERR--Receive Event Failure */
                    SYSFUN_Sleep(10);
                    break;
                }

                if (event_var & HTTP_TASK_EVENT_ENTER_TRANSITION )
                {
                    is_transition_done = TRUE;	/* Turn on the transition done flag */
                    event_var = 0;
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

}

/*------------------------------------------------------------------------
 * ROUTINE NAME - HTTP_TASK_Enter_Main_Routine
 *------------------------------------------------------------------------
 * PURPOSE: This function creates the HTTP task for network
 *.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *
 *------------------------------------------------------------------------
 */
void HTTP_TASK_Enter_Main_Routine ()
{
    struct timeval          timeout;
    HTTP_STATE_T            current_http_state;
    int                     ret;

#if (SYS_CPNT_HTTPS == TRUE)
    SECURE_HTTP_STATE_T     current_secure_http_state;
#endif /* SYS_CPNT_HTTPS */

    HTTP_OM_CleanAllConnectionObjects();

    {
        int i;

        for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
        {
            HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
            ASSERT(worker != NULL);

            if (i == 0)
            {
                worker->kind = HTTP_WORKER_MASTER;
            }
            else
            {
                worker->kind = HTTP_WORKER_OTHER;
            }

            worker->close = 0;

            // TODO: We need using the spying for SYSFUN. Then it can use
            //       different behavior under test and real daemon.
            SpawnThread(http_worker_main, worker);
        }
    }

    current_http_state = HTTP_MGR_Get_Http_Status();

#if (SYS_CPNT_HTTPS == TRUE)
    current_secure_http_state = HTTP_MGR_Get_Secure_Http_Status();
#endif /* #if SYS_CPNT_HTTPS */

    while (/*HTTP_MGR_GetOperationMode () == SYS_TYPE_STACKING_MASTER_MODE*/ 1)
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        HTTP_TASK_CheckSoftwareWatchdogEvent();
#endif
        /* check for socket reopen */
        if ((HTTP_MGR_Get_Http_Status() != current_http_state))
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                if (HTTP_MGR_Get_Http_Status() == HTTP_STATE_ENABLED)
                {
                    http_worker_ctrl(worker, HTTP_WORKER_START_HTTP, NULL);
                }
                else
                {
                    http_worker_ctrl(worker, HTTP_WORKER_SHUTDOWN_HTTP, NULL);
                }
            }

            current_http_state = HTTP_MGR_Get_Http_Status();
        }

        if (http_task_is_port_changed)
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                http_worker_ctrl(worker, HTTP_WORKER_RESTART_HTTP, NULL);
            }

            http_task_is_port_changed = FALSE;
        }

#if (SYS_CPNT_HTTPS == TRUE)

        if (TRUE == HTTP_MGR_Get_Certificate_Status())
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                // TODO: shall only restart HTTPS service and reload new certificate?
                http_worker_ctrl(worker, HTTP_WORKER_CLOSE, NULL);
            }

            break;
        }

        /* check for https_socket reopen */
        if ((HTTP_MGR_Get_Secure_Http_Status() != current_secure_http_state))
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                if (HTTP_MGR_Get_Secure_Http_Status() == SECURE_HTTP_STATE_ENABLED)
                {
                    http_worker_ctrl(worker, HTTP_WORKER_START_HTTPS, NULL);
                }
                else
                {
                    http_worker_ctrl(worker, HTTP_WORKER_SHUTDOWN_HTTPS, NULL);
                }
            }

            current_secure_http_state = HTTP_MGR_Get_Secure_Http_Status();
        }

        if (http_task_is_security_port_changed)
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                http_worker_ctrl(worker, HTTP_WORKER_RESTART_HTTPS, NULL);
            }

            http_task_is_security_port_changed = FALSE;
        }
#endif /* if (SYS_CPNT_HTTPS == TRUE) */

        if (HTTP_MGR_GetOperationMode () != SYS_TYPE_STACKING_MASTER_MODE)
        {
            int i;

            for (i = 0; i < HTTP_CFG_TOTAL_WORKER; ++i)
            {
                HTTP_Worker_T *worker = HTTP_OM_GetWorkerByIndex(i);
                ASSERT(worker != NULL);

                // TODO: shall only restart HTTPS service and reload new certificate?
                http_worker_ctrl(worker, HTTP_WORKER_CLOSE, NULL);
            }

            break;
        }

        SYSFUN_Sleep(100);
        ret = 0;

        if (ret <= 0)
        {
            HTTP_TASK_OnIdle();
            continue;
        }

    }

retry:
    /* Wait until all connections leave
     */
     if (HTTP_OM_HasConnectionObject() == TRUE)
     {
 #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
         HTTP_TASK_CheckSoftwareWatchdogEvent();
 #endif
         SYSFUN_Sleep(100);
         goto retry;
     }
}

/* Create socket
 */
static int HTTP_TASK_Socket(struct sockaddr *sa)
{
    int sockfd;

#if (SYS_CPNT_IPV6 == TRUE)
    sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (0 <= sockfd)
    {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)sa;

        sin6->sin6_family = AF_INET6;

        return sockfd;
    }
#endif

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (0 <= sockfd)
    {
        struct sockaddr_in *sin4 = (struct sockaddr_in*)sa;

        sin4->sin_family = AF_INET;

        return sockfd;
    }

    return HTTP_TASK_SOCK_INVALID;
}

/* set port number to sockaddr
 */
static void HTTP_TASK_SetSockPort(UI16_T port, struct sockaddr *sa)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        {
            struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

            sin->sin_port = htons(port);
            break;
        }

#if (SYS_CPNT_IPV6 == TRUE)
    case AF_INET6:
        {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;

            sin6->sin6_port = htons(port);
            break;
        }
#endif
    default:
        ASSERT(0);
        break;
    }
}

/* FUNCTION NAME:  HTTP_TASK_StartHttpService
 * PURPOSE:
 *          Start HTTP service.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - socket number.
 * NOTES:
 *          (Something must be known to use this function.)
 */
int HTTP_TASK_StartHttpService()
{
    int     sockfd;
    struct  sockaddr_in6 server_addr;
    UI32_T  port;
    int     value=1;

    port = HTTP_MGR_Get_Http_Port();
    memset((char *) &server_addr,0, sizeof(server_addr));

    sockfd = HTTP_TASK_Socket((struct sockaddr*)&server_addr);
    if (0 <= sockfd)
    {
        HTTP_TASK_SetSockPort(port, (struct sockaddr*)&server_addr);
    }
    else
    {
        printf("\r\n\r\n%s: create socket failed\r\n\r\n", __FUNCTION__);
        ASSERT(0);
        return HTTP_TASK_SOCK_INVALID;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0)
    {
        printf("%d socket setopt error \n", __LINE__);
        ASSERT(0);
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        ASSERT(0);
        EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_Init_Socket_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), "bind");
        HTTP_closesocket(sockfd);    /* close socket */
        return HTTP_TASK_SOCK_INVALID;
    }

    /* listen */
    if( listen (sockfd, HTTP_CFG_MAXWAIT) < 0)
    {
        ASSERT(0);
        EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_Init_Socket_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), "listen");
        HTTP_closesocket(sockfd);    /* close socket */
        return HTTP_TASK_SOCK_INVALID;
    }

    /* for debug
     */
    if (0)
    {
        char root_dir[1024];
        HTTP_MGR_Get_Root_Dir(root_dir, sizeof(root_dir));

        printf("server run on [%s] 0.0.0.0:%lu\r\n", root_dir, HTTP_MGR_Get_Http_Port());
    }

    return sockfd;
}

#if (SYS_CPNT_HTTPS == TRUE)
/* FUNCTION NAME:  HTTP_TASK_StartHttpsService
 * PURPOSE:
 *          Start HTTPS service.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          UI32_T - socket number.
 * NOTES:
 *          (Something must be known to use this function.)
 */
int HTTP_TASK_StartHttpsService()
{
    int                 https_sockfd;
    struct sockaddr_in6 https_server_addr;
    UI32_T              port;
    int                 value = 1;

    HTTP_MGR_Get_Secure_Port (&port);
    memset((char *) &https_server_addr,0, sizeof(https_server_addr));

    https_sockfd = HTTP_TASK_Socket((struct sockaddr*)&https_server_addr);
    if (0 <= https_sockfd)
    {
        HTTP_TASK_SetSockPort(port, (struct sockaddr*)&https_server_addr);
    }
    else
    {
        printf("\r\n\r\n%s: create socket failed\r\n\r\n", __FUNCTION__);
        ASSERT(0);
        return HTTP_TASK_SOCK_INVALID;
    }

    if(setsockopt(https_sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value))!=0)
    {
        printf("%d socket setopt error \n", __LINE__);
        ASSERT(0);
    }

    if ( bind(https_sockfd, (struct sockaddr *)&https_server_addr, sizeof(https_server_addr)) < 0)
    {
        ASSERT(0);
        EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_Init_Socket_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), "bind");
        HTTP_closesocket(https_sockfd);    /* close socket */
        return HTTP_TASK_SOCK_INVALID;
    }

    /* listen */
    if( listen (https_sockfd, HTTP_CFG_MAXWAIT) < 0)
    {
        ASSERT(0);
        EH_MGR_Handle_Exception1(SYS_MODULE_HTTP, HTTP_Init_Socket_FUNC_NO, EH_TYPE_MSG_SOCKET_OP_FAILED, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT), "listen");
        HTTP_closesocket(https_sockfd);    /* close socket */
        return HTTP_TASK_SOCK_INVALID;
    }

    return https_sockfd;
}
#endif /* SYS_CPNT_HTTPS */

/* FUNCTION NAME:  HTTP_TASK_IsProvisionComplete
 * PURPOSE:
 *          This function will check the HTTP module can start.
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
BOOL_T HTTP_TASK_IsProvisionComplete(void)
{
    return( is_provision_complete );
}

BOOL_T HTTP_TASK_IsValidMgmtRemoteAddress(int remotefd)
{
#if(SYS_CPNT_MGMT_IP_FLT == TRUE)
    struct sockaddr_in6 addr_storage;
    socklen_t addr_len;

    L_INET_AddrIp_T remote_inet;

    int ret;

    addr_len = sizeof(addr_storage);
    ret = getpeername(remotefd, (struct sockaddr *)&addr_storage, &addr_len);
    if (-1 == ret)
    {
        return FALSE;
    }

    if (L_INET_SockaddrToInaddr((struct sockaddr *)&addr_storage, &remote_inet) != TRUE)
    {
        return FALSE;
    }

    if (MGMT_IP_FLT_IsValidIpFilterAddress(MGMT_IP_FLT_HTTP, &remote_inet) != TRUE)
    {
        return FALSE;
    }
#endif /* SYS_CPNT_MGMT_IP_FLT */

    return TRUE;
}

#if (SYS_CPNT_CLUSTER == TRUE)
BOOL_T HTTP_TASK_GetClusterMemberIp(int newsockfd)
{
    CLUSTER_TYPE_EntityInfo_T cluster_info;
    L_INET_AddrIp_T   inet_commander;
    struct sockaddr_in6  peer_addr;
    UI32_T  commander_ip;
    int     addrlen;
    int     err_code = 0;

    memset(&cluster_info, 0, sizeof(CLUSTER_TYPE_EntityInfo_T));
    memset(&peer_addr, 0, sizeof(peer_addr));
    memset(&inet_commander, 0, sizeof(inet_commander));
    CLUSTER_POM_GetClusterInfo(&cluster_info);

    /* only accepts commander's connectoin if it's as a member
     */
    if(cluster_info.role != CLUSTER_TYPE_ACTIVE_MEMBER)
    {
        return TRUE;
    }

    /* cluster_mgr's commanderIp is in network order
     */
    memcpy(&commander_ip, cluster_info.commander_ip, sizeof(UI32_T));

    /* getpeername will return IP in network order.
     * If getpeername failed or remote IP is not commander's IP,
     * close the connection.
     */
    addrlen = sizeof(peer_addr);
    err_code = getpeername(newsockfd, (struct sockaddr *)&peer_addr, (int *)&addrlen);

    if(err_code < 0)
    {
        return FALSE;
    }

    L_INET_SockaddrToInaddr((struct sockaddr *)&peer_addr, &inet_commander);

    /* currently, cluster only supports v4
     */
    if( (L_INET_ADDR_TYPE_IPV4 != inet_commander.type)
        &&(L_INET_ADDR_TYPE_IPV4Z != inet_commander.type) )
    {
        return FALSE;
    }

    /* check if source is commander
     */
    if(memcmp(inet_commander.addr, cluster_info.commander_ip, SYS_ADPT_IPV4_ADDR_LEN)!=0)
    {
        return FALSE;
    }

    return TRUE;
}
#endif  /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
/* FUNCTION NAME:  HTTP_TASK_CheckSoftwareWatchdogEvent
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
static void HTTP_TASK_CheckSoftwareWatchdogEvent(void)
{
    UI32_T rc;
    UI32_T event=0;

    rc = SYSFUN_ReceiveEvent(SYSFUN_SYSTEM_EVENT_SW_WATCHDOG,
                             SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_NOWAIT,
                             &event);

    if (rc!=SYSFUN_OK && rc!=SYSFUN_RESULT_TIMEOUT)
    {
        SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveEvent Error (ret=%lu) \r\n", __FUNCTION__, rc);
    }

    if (event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
    {
         SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_HTTP);
         event ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
    }
}
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */

void HTTP_TASK_OnIdle()
{
    return;
}
