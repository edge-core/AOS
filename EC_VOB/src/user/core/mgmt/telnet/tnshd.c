/* Module Name: TNSHD.C
 * Purpose:
 *      TNSHD, TelNet SHell Daemon, a responser for telnet request.
 *      Telnet daemon needs a task to handle command request, this TNSHD
 *      take care of all request from telnet client.
 *
 * Notes:
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.11.17  --  William,    Created.
 *	1.0 2002.08.02  --  William,   Patch for port 5151 hole, if remote site's ip is different
 *						from tnshd, disconnect it. Ie. deny caller from non-target.
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001.
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <signal.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_stdlib.h"
#include "pshcfg.h"
#include "telnetd.h"
#include "telnet_mgr.h"
#if ( SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
#include "sshd_mgr.h"
#endif

#include "cli_type.h"
#include "cli_task.h"
#include "cli_main.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define	TNSHD_WAIT_TCP_READY_TICKS			100


/*  2001.10.28, William, Service socket port number must be same as defined in telnetd,c
 */
/*	#define SYS_BLD_TELNET_SERVICE_SOCKET_PORT      5151    	*/
#define TNSHD_SERVICE_SOCKET_PORT_NO            SYS_BLD_TELNET_SERVICE_SOCKET_PORT
#define TNSHD_NO_EVENT                  0
#define TNSHD_PROVISION_COMPLETE_EVENT  0x00000001
#define TNSHD_ENTER_TRANSITION_EVENT    0x00000002
#define TNSHD_WAIT_EVENT                0x00000003

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void TNSHD_ParentTask ( pshcfg_t *cfg);
static void Master_Mode_Routine(pshcfg_t *cfg);
static void TNSHD_ChildTask(CLI_TASK_WorkingArea_T *work_space);
static void TNSHD_ChildTaskSessionExit(UI32_T tid, void *work_space, int socket_id);
static UI32_T Get_Event(UI32_T timeout);
static void tnshd_VxWorksShell (UI32_T  *args);

/* amytu porting 2003-2-17
 */

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T tnshd_task_id;
static BOOL_T transition_done = FALSE;
static UI32_T local_event=0;
//static int telnet_s_socket;

/* EXPORTED SUBPROGRAM BODIES
 */
/* XXX steven.jiang for warnings */
BOOL_T SSHD_PMGR_SetConnectionIDAndTnshID(UI32_T tnsh_port, UI32_T tid, UI32_T cid);

/* FUNCTION NAME : TNSHD_CreateTask
 * PURPOSE:
 *      Create TelNet SHell Daemon task. This task is waiting request
 *      and take any request on the servce-socket.
 *
 * INPUT:
 *      cfg --  system configuration information.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      0       --  TNSHD task created.
 *      -1      --  error.
 * NOTES:
 */
int TNSHD_CreateTask(pshcfg_t *cfg)
{
    if(SYSFUN_SpawnThread(SYS_BLD_TELNET_SHELL_THREAD_PRIORITY,
                          SYS_BLD_TELNET_THREAD_SCHED_POLICY,
                          SYS_BLD_TELNET_SHELL_THREAD_NAME,
                          SYS_BLD_TASK_LARGE_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          TNSHD_ParentTask,
                          cfg,
                          &tnshd_task_id)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
        return (-1);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_TELNET_PARENT, tnshd_task_id, SYS_ADPT_TELNET_PARENT_SW_WATCHDOG_TIMER);
#endif

    SYSFUN_Debug_Printf(" tnshd_start : [TNSHD_ParentTask] spawn..\n");
    return (0);
}

void TNSHD_ProvisionComplete()
{
    SYSFUN_SendEvent (tnshd_task_id,
    TNSHD_PROVISION_COMPLETE_EVENT);

}

void TNSHD_EnterTransitionMode(void)
{
    transition_done = FALSE;

    /*jingyan zheng ES3628BT-FLF-ZZ-00227*/
    SYSFUN_SendEvent (tnshd_task_id,
        TNSHD_ENTER_TRANSITION_EVENT);

    while (! transition_done)
    {
        SYSFUN_Sleep(50);
    }

}


/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */

/* telnet shell task
 * This task create, bind, listen on this socket,
 * accept on this socket, if sucess than open (create) an CLI session
 *
 */
/* FUNCTION NAME : TNSHD_ParentTask
 * PURPOSE:
 *      TNSHD, TelNet SHell Daemon, task body, the main routine.
 *      Which spawn child task to handle one client request.
 *
 * INPUT:
 *      cfg --  system configuration information.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      0       --  TNSHD task created.
 *      -1      --  error.
 * NOTES:
 */
static void TNSHD_ParentTask ( pshcfg_t *cfg)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    SYS_TYPE_Stacking_Mode_T operating_mode;
    UI32_T rcv_event, timeout;

    /* BODY */
    operating_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
    while (1)
    {
        timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        rcv_event = Get_Event(timeout);
        switch (operating_mode)
        {
        case SYS_TYPE_STACKING_SLAVE_MODE:
        case SYS_TYPE_STACKING_TRANSITION_MODE:
            if (rcv_event == TNSHD_ENTER_TRANSITION_EVENT)
            {
                transition_done = TRUE;
                local_event ^= TNSHD_ENTER_TRANSITION_EVENT; 
            }
            else if (rcv_event == TNSHD_PROVISION_COMPLETE_EVENT)
            {
                operating_mode = SYS_TYPE_STACKING_MASTER_MODE;
                local_event ^= TNSHD_PROVISION_COMPLETE_EVENT;
                Master_Mode_Routine(cfg);
            }
            break;
        case SYS_TYPE_STACKING_MASTER_MODE:
            if (rcv_event == TNSHD_ENTER_TRANSITION_EVENT)
            {
                transition_done = TRUE;
                operating_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
                local_event ^= TNSHD_ENTER_TRANSITION_EVENT;
            }
            else
            {
                Master_Mode_Routine(cfg);
            }
            break;

        default:
            SYSFUN_DBG_PRINTF("TNPD halt");
        }
    }
}

static void Master_Mode_Routine(pshcfg_t *cfg)
{
    int      sid, new_sid;       /*  socket-id   */
    UI32_T   tid, addrlen;
    int     i, on = 1;
    struct sockaddr_in6 sin;
    char     name[16];
    UI32_T  *cli_session_work_space;
    int      ret;

    fd_set              read_fds;
    struct timeval      timeout;

    memset(&sin, 0, sizeof(sin));

#if (SYS_CPNT_IPV6 == TRUE)
    sid = socket(AF_INET6, SOCK_STREAM, 0);

    if (sid >= 0) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&sin;

        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = L_STDLIB_Hton16(TNSHD_SERVICE_SOCKET_PORT_NO);
        addrlen = sizeof(struct sockaddr_in6);
    }
    else
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */
    {
        sid = socket(AF_INET, SOCK_STREAM, 0);

        if (sid >= 0) {
            struct sockaddr_in *sin4 = (struct sockaddr_in*)&sin;

            sin4->sin_family = AF_INET;
            sin4->sin_port = L_STDLIB_Hton16(TNSHD_SERVICE_SOCKET_PORT_NO);
            addrlen = sizeof(struct sockaddr_in);
        }
        else {
            perror("[TNSHD] socket(tcp)");
            return;
        }
    }

    if (setsockopt(sid, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)))
    {
        perror("[TNSHD] setsockopt");
        s_close(sid);
        return;
    }

    if (bind(sid, (struct sockaddr *)&sin, addrlen) < 0) {
        perror("[TNSHD] bind");
        s_close(sid);
        return;
    }

    if (getsockname(sid, (struct sockaddr *)&sin, (socklen_t *)&addrlen) < 0) {
        perror("[TNSHD] getsockname");
        s_close(sid);
        return;
    }
    if (listen(sid, 1) < 0) {
        perror("[TNSHD] listen");
        s_close(sid);
        return;
    }

    ret = setsockopt(sid, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
    if (ret < 0) {
        perror("[TNSHD] setsockopt");
        s_close(sid);
        SYSFUN_Debug_Printf("\r\n%s(): setsockopt() fail. errno = %d", __FUNCTION__, errno);
        return;
    }

    while(SYS_TYPE_STACKING_MASTER_MODE == TELNET_MGR_GetOperationMode())
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        /* SYSFUN_SYSTEM_EVENT_SW_WATCHDOG event will be checked and
         * handled in Get_Event()
         */
        Get_Event(SYSFUN_TIMEOUT_NOWAIT);
#endif

        FD_ZERO(&read_fds);
        FD_SET(sid, &read_fds);

        timeout.tv_sec = 1;     /*  no.  of seconds  */
        timeout.tv_usec = 0;    /*  no. of micro seconds  */
        ret = select(sid+1, &read_fds, NULL, NULL, &timeout);
        if(!ret)
           continue;
        addrlen = sizeof(sin);
        new_sid = accept(sid, (struct sockaddr *)&sin, (socklen_t *)&addrlen);
        if (new_sid < 0)
        {
            break;
        }

        /* Security checking. Accept locate connection only.
         */
        {
            struct  sockaddr_in6    sock_addr;
            struct  sockaddr *srv_p = (struct sockaddr *)&sock_addr;
            struct  sockaddr_in6    peer_addr;
            struct  sockaddr *peer_p = (struct sockaddr *)&peer_addr;

            socklen_t     srv_addr_size=sizeof(sock_addr);
            socklen_t     peer_addr_size=sizeof(peer_addr);

            /* BODY */
            /*  local site  */
            if (getsockname(new_sid, srv_p, &srv_addr_size) < 0)
            {
                s_close(new_sid);
                continue;
            }

            /*  remote site */
            if (getpeername(new_sid, peer_p, &peer_addr_size) < 0)
            {
                s_close(new_sid);
                continue;
            }

            /* the peer (telnetd) should connect by the same family
             */
            if (srv_p->sa_family != peer_p->sa_family)
            {
	            s_close(new_sid);
                continue;
            }

#if (SYS_CPNT_IPV6 == TRUE)
            if (AF_INET6 == srv_p->sa_family)
            {
                struct sockaddr_in6 *s = (struct sockaddr_in6 *)srv_p;
                struct sockaddr_in6 *p = (struct sockaddr_in6 *)peer_p;

                if (memcmp(&s->sin6_addr, &p->sin6_addr, sizeof(struct in6_addr)) != 0)
                {
    	            s_close(new_sid);
                    continue;
                }
            }
            else
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */
            if (AF_INET == srv_p->sa_family)
            {
                struct sockaddr_in *s = (struct sockaddr_in *)srv_p;
                struct sockaddr_in *p = (struct sockaddr_in *)peer_p;

                if (memcmp(&s->sin_addr, &p->sin_addr, sizeof(struct in_addr)) != 0)
                {
    	            s_close(new_sid);
                    continue;
                }
            }

        }	/*	end of port 5151 security checking	*/

        setsockopt(new_sid, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));
        /* create a task name for the new task */
        for (i=0; i < 100; i++)
        {
            sprintf(name, SYS_BLD_TELNET_SHELL_CHILD_TASK, i);
            if (SYSFUN_TaskNameToID(name, &tid))
                break;
        }

        if (i >= 100)
        {
            SYSFUN_Debug_Printf("\r\n%s(): cannot build a task name", __FUNCTION__);
            s_close(new_sid);
            continue;
        }

        cli_session_work_space = CLI_TASK_AllocatTelnetWorkingSpace();
        if (cli_session_work_space == NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s(): no more workspace for this telnet session.", __FUNCTION__);
            s_close(new_sid);
            continue;
        }

        ((CLI_TASK_WorkingArea_T*)cli_session_work_space)->socket = new_sid;

        if(SYSFUN_SpawnThread(SYS_BLD_TELNET_SHELL_THREAD_PRIORITY,
                              SYS_BLD_TELNET_THREAD_SCHED_POLICY,
                              name,
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-19, 17:15:00 */
                              (SYS_BLD_TELNET_SHELL_CHILD_TASK_STACK_SIZE * 2),
#else
                              SYS_BLD_TELNET_SHELL_CHILD_TASK_STACK_SIZE,
#endif /* ES3526MA_POE_7LF_LN */
                              SYSFUN_TASK_NO_FP,
                              tnshd_VxWorksShell,
                              cli_session_work_space,
                              &tid)!=SYSFUN_OK)

        {
            SYSFUN_Debug_Printf("\r\n%s(): spawn telnet shell task error\n", __FUNCTION__);
            CLI_TASK_FreeTelnetWorkingSpace(cli_session_work_space);
            s_close(new_sid);
        }
    }

    s_close(sid);
}

static UI32_T Get_Event(UI32_T timeout)
{
    UI32_T event_status, rcv_event;

    event_status = SYSFUN_ReceiveEvent (
         TNSHD_WAIT_EVENT
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        |SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
        ,SYSFUN_EVENT_WAIT_ANY
        ,timeout, &rcv_event);

    local_event |= rcv_event;

    if (event_status != SYSFUN_OK)
    {   /*  Exception handling  */
        switch(event_status)
        {
        case    SYSFUN_RESULT_NO_EVENT_SET:
                break;
        case    SYSFUN_RESULT_TIMEOUT:
        case    SYSFUN_RESULT_CALL_FROM_ISR:
        case    SYSFUN_RESULT_SYSFUN_NOT_INIT:
        default:
                SYSFUN_DBG_PRINTF("TNPD halt");
        break;
        }
        return TNSHD_NO_EVENT;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    if (local_event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
    {
         SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_TELNET_PARENT);
         local_event ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
    }
#endif

    if (local_event & TNSHD_ENTER_TRANSITION_EVENT)
    {
        return TNSHD_ENTER_TRANSITION_EVENT;

    }

    if (local_event & TNSHD_PROVISION_COMPLETE_EVENT)
    {
        return TNSHD_PROVISION_COMPLETE_EVENT;
    }

    return TNSHD_NO_EVENT;
}


static void tnshd_VxWorksShell (UI32_T  *args)
{
    TNSHD_ChildTask((CLI_TASK_WorkingArea_T *)args);
}

extern void CLI_TASK_DumpCliWorkingBuffer(void);

/* FUNCTION NAME : TNSHD_ChildTask
 * PURPOSE:
 *      TNSHD child task, prepare some variable and call handling routine.
 *      eg. Telnet-CLI, Telnet-DCLI...
 *
 * INPUT:
 *      work_space  --  This session's working space.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
static void TNSHD_ChildTask(CLI_TASK_WorkingArea_T *work_space)
{
    UI8_T   c;
    int     f_status;
    int     socket;
    UI32_T  tid = SYSFUN_TaskIdSelf();
    UI32_T  tnpd_side_tnsh_port;/*, tnpd_site_ip; for signaling TNPD task */
    struct sockaddr_in6 tnpd;

    assert(work_space != NULL);
    signal(SIGPIPE, SIG_IGN);

    socket = work_space->socket;

    SYSFUN_Debug_Printf("%s():start new task tid = %ld, socket = %d\r\n", __FUNCTION__, tid, socket);

    if (recv(socket, &c, 1, 0) < 0)
    {
        SYSFUN_Debug_Printf("\r\n%s(): do not receive any char. errno = %d, task id = %ld\n", __FUNCTION__, errno, tid);
        goto Exit;
    }

    f_status = fcntl(socket, F_GETFL);
    fcntl(socket, F_SETFL,f_status | O_NONBLOCK);

    /* signal remote site, TNSH is starting. */
    TNPD_GetSessionName(REMOTE_SOCKET, socket, sizeof(tnpd), (struct sockaddr*)&tnpd);
    if( ((struct sockaddr*)&tnpd)->sa_family == AF_INET)
    {
        tnpd_side_tnsh_port = L_STDLIB_Ntoh16( ((struct sockaddr_in*)&tnpd)->sin_port );
    }
    else
    {
        tnpd_side_tnsh_port = L_STDLIB_Ntoh16( ((struct sockaddr_in6*)&tnpd)->sin6_port );
    }

    /*if (tnpd_site_ip == 0)
    {
        SYSFUN_Debug_Printf("\r\n%s() line %d: tnpd_site_ip == 0", __FUNCTION__, __LINE__);
        goto Exit;
    }*/

    if (!CLI_TASK_SetSessionContext(work_space, tid, socket, CLI_TYPE_TELNET,0, 0, 0))
    {
        SYSFUN_Debug_Printf("\r\n%s() line %d: CLI_TASK_SetSessionContext() return FALSE", __FUNCTION__, __LINE__);
        goto Exit;
    }

    if (c == 's') /* a SSH connection */
    {
#if ( SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
        char c = 1;

        SYSFUN_Debug_Printf("\r\n%s() line %d: sess_id = %u", __FUNCTION__, __LINE__, CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1);

        if (!SSHD_MGR_SetConnectionIDAndTnshID(tnpd_side_tnsh_port,tid, CLI_TASK_GetMySessId() - CLI_TASK_MIN_TELNET_SESSION_ID + 1))
        {
            SYSFUN_Debug_Printf("\r\n%s(): SSHD_PMGR_SetConnectionIDAndTnshID() return FALSE", __FUNCTION__);
            goto Exit;
        }

        /* Notify Tnsh(PTY) ready
         * SESSION_ForkTnsh (sshv2/session.c)
         */

        SYSFUN_Debug_Printf("\r\n%s() line %d: pty ready", __FUNCTION__, __LINE__);

        if (send(socket, &c, 1, 0) != 1 )
        {
            SYSFUN_Debug_Printf("TNSHD_ChildTask : net is terminal.");
            goto Exit;
        }

#else
        SYSFUN_Debug_Printf("\r\n%s() line %d: the first received char 's' meanless", __FUNCTION__, __LINE__);
        goto Exit;
#endif
    }
    else if (c == 't') /* a telnet connection */
    {
        TNPD_LoginSignal(tnpd_side_tnsh_port);
    }
    else
    {
        SYSFUN_Debug_Printf("\r\n%s(): the first received char meanless, it is '%c'", __FUNCTION__, c);
        goto Exit;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    /* monitor id = max static assign monitor_id + max telnet num + session id - 1
     * Range 1: SW_WATCHDOG_MAX_MONITOR_ID ~ SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM are for server part.
     * Range 2: SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM ~ SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM*2 are for cli part
     * Due to CLI console use session 0, therefroe the telent session id need -1.
     */
    SW_WATCHDOG_MGR_RegisterMonitorThread (SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM+CLI_TASK_GetMySessId()-1, tid, SYS_ADPT_TELNET_PARENT_SW_WATCHDOG_TIMER );
#endif 

    CLI_MAIN_Enter_Main_Routine(work_space);

Exit:
    TNSHD_ChildTaskSessionExit(tid, work_space, socket);

    SYSFUN_Debug_Printf("%s():exit task tid = %ld, socket = %d\r\n", __FUNCTION__, tid, socket);

    return;
}

/*  ROUTINE NAME : TNSHD_ChildTaskSessionExit
 *  FUNCTION     : Close a socket and terminate this task.
 *  INPUT        : socket_id  -- this task handled socket-id.
 *  OUTPUT       : None.
 *  RETURN       : None.
 *  NOTES        :
 */
static void TNSHD_ChildTaskSessionExit(UI32_T tid, void *work_space, int socket_id)
{
    UI32_T  tnpd_side_tnsh_port;
    struct sockaddr_in6 tnpd;

    /* signal remote site, TNSH is terminated. */
    TNPD_GetSessionName(REMOTE_SOCKET, socket_id, sizeof(tnpd), (struct sockaddr*)&tnpd);
    if( ((struct sockaddr*)&tnpd)->sa_family == AF_INET)
    {
        tnpd_side_tnsh_port = L_STDLIB_Ntoh16( ((struct sockaddr_in*)&tnpd)->sin_port );
    }
    else
    {
        tnpd_side_tnsh_port = L_STDLIB_Ntoh16( ((struct sockaddr_in6*)&tnpd)->sin6_port );
    }

    {
        TNPD_LogoutSignal (tnpd_side_tnsh_port);
    }

    /* clear full-duplex connection */
    shutdown(socket_id, SHUT_RDWR);
    s_close(socket_id);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_UnregisterMonitorThread (SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM+CLI_TASK_GetMySessId()-1);
#endif

    CLI_TASK_FreeTelnetWorkingSpace(work_space);
    return ;

}
