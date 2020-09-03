/************************************************************************/
/*                                                                      */
/*   MODULE: telnetd.c                                                  */
/*   PRODUCT: pTNPD+                                                    */
/*   PURPOSE: TELNET server                                             */
/*   DATE:  08/20/1992                                                  */
/*                                                                      */
/*----------------------------------------------------------------------*/
/*                                                                      */
/*              Copyright 1992, Integrated Systems Inc.                 */
/*                      ALL RIGHTS RESERVED                             */
/*                                                                      */
/*   This computer program is the property of Integrated Systems Inc.   */
/*   Santa Clara, California, U.S.A. and may not be copied              */
/*   in any form or by any means, whether in part or in whole,          */
/*   except under license expressly granted by Integrated Systems Inc.  */
/*                                                                      */
/*   All copies of this program, whether in part or in whole, and       */
/*   whether modified or not, must display this and all other           */
/*   embedded copyright and ownership notices in full.                  */
/*                                                                      */
/************************************************************************/
/*
 * Copyright (c) 1983, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * Change LOG:
 * Mar 13, 2003. jwang
 *     1. Add Delete_All_Sessions() to clean house when switching occurs
 *     2. Clean code, merge all memory free call into tnpd_exit.
 *     3. Merge n_sessions with session_table.created_session_count.
 *     4. Merge all s_close call into tnpd_exit
 *
 * 2002.03.16, William, Patch for telnet.
 *              For unknown condition, tnpd()--> ptyflush() can't send() out
 *              packet, and get return value (-12/can't write), packets will
 *              queue and whole system be blocked. Return error code and terminate
 *              the socket to solve this. Maybe there is side-effect.
 * 2001.10.28, William,
 *             1.change system call from pSos to SYSFUN lib.
 *             2.change socket(AF_INET, SOCK_STREAM, 6) to
 *               socket(AF_INET, SOCK_STREAM, IP_PROT_TCP).
 *               IP_PROT_TCP defined in iconst.h.
 *             3.Use IP_CMN_GetMode() replace P2INIT_P2MG_Task_Start().
 * 08-26-2000, wuli  for porting phase2
 * remove include pna.h, pna_mib.h, probe.h
 * add include iproute.h, socket.h
 * change close(0) to s_close(0)
 * change AF_INET to AF_INET4
 * change server_addr.sin_addr.s_addr to server_addr.sin_addr
 * change sockaddr_in * to sockaddr *
 * change socket(AF_INET, SOCK_STREAM, TCP) to socket(AF_INET4 , SOCK_STREAM, 6)
 */

/* INCLUDE FILE DECLARATIONS
 */
/*      <<< Define some directory for special purpose >>>
 *  BACKDOOR_OPEN : interact with backdoor function of SYSFUN
 *                  if no need to interact with SYSFUN, do not define BACKDOOR_OPEN
 *  SYSLOG_OPEN   : activate SYSLOG function to log event.
 */
#define BACKDOOR_OPEN

/*
 *  <<< end of special directory definition >>>
 */
#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <netinet/tcp.h>

#include "sys_type.h"       /*  2001.10.28, William, SYS_TYPE_STACKING_MASTER_MODE */
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "l_mm.h"
#include "sysfun.h"         /*  2001.10.28, William, changed from psos.h    */

#include "nudefs.h"
#include "l_stdlib.h"
#include "cli_mgr.h"
#include "ctype.h"

#define TELOPTS             /* To define telopts[] in telnet.h */
#include "telnet.h"
#include "telnetd.h"
#include "telnet_mgr.h"
#include "telnet_om.h"

#include "tnshd.h"
#include "tnpdcfg.h"

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#endif /* SYS_CPNT_DEBUG */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif

#include <stdlib.h>
#include "sshd_mgr.h"

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_mgr.h"
#include "cluster_pmgr.h"
#include "cluster_pom.h"
#endif
#include "cli_pmgr.h"
#include "ip_lib.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef max
#define max(a,b)  ((a) < (b) ? (b) : (a))
#endif

#define TELNET_TRANSITION_MODE_SLEEP_TICK   100

/*  2001.10.28, William, service socket number must be same as defined in tn_main.c */
#define TNSHD_SERVICE_SOCKET_PORT_NO        SYS_BLD_TELNET_SERVICE_SOCKET_PORT
#define TNPD_TELNET_PORT                    SYS_DFLT_TELNET_SOCKET_PORT
#define TNPD_CLUSTER_MAX_RETRY_TIME         3

/*
 * State for recv fsm
 */
#define TS_DATA  0    /* base state */
#define TS_IAC   1    /* look for double IAC's */
#define TS_CR    2    /* CR-LF ->'s CR */
#define TS_SB    3    /* throw away begin's... */
#define TS_SE    4    /* ...end's (suboption negotiation) */
#define TS_WILL  5    /* will option negotiation */
#define TS_WONT  6    /* wont " */
#define TS_DO    7    /* do " */
#define TS_DONT  8    /* dont " */

/* */
#define LOG_INFO    0
#define LOG_WARNING 1
#define LOG_ERR     2
#define LOG_PID     0x01
#define LOG_ODELAY  0x02
#define LOG_DAEMON  0x04

/* Linux defines ECHO in /usr/include/bits/termios.h */
#ifdef ECHO /* XXX steven.jiang for warnings */
#undef ECHO
#endif

#define RAW         0x01
#define ECHO        0x02
#define CRMOD       0x04

#ifdef   BUFSIZ
   #undef BUFSIZ
#endif
#define BUFSIZ         512

#define MAXHOSTNAMELEN  64

/*
 * Telnet server.
 */
#define OPT_NO                  0        /* won't do this option */
#define OPT_YES                 1        /* will do this option */
#define OPT_YES_BUT_ALWAYS_LOOK 2
#define OPT_NO_BUT_ALWAYS_LOOK  3


/* MACRO FUNCTION DECLARATIONS
 */
 #define TELNETD_TRACE(flag, fmt, ...)                          \
    {                                                           \
        if (!((flag) & TELNETD_DEBUG_SHOW_TO_BACKDOOR_ONLY))    \
            TELNETD_DEBUG(flag, fmt, ##__VA_ARGS__);            \
        TELNETD_BD_PRINT(flag, fmt "\r\n", ##__VA_ARGS__);      \
    }

#define TELNETD_PRINT(fmt, ...)                 \
    if (telnetd_debug_print == 1)               \
        printf(fmt, ##__VA_ARGS__);             \
    else                                        \
        BACKDOOR_MGR_Printf(fmt, ##__VA_ARGS__);

#define TELNETD_BD_LOG(flag, fmt, ...)                              \
    TELNETD_BD_PRINT(flag, "[%u]%s(%d):" fmt "\r\n",               \
        SYSFUN_TaskIdSelf(),__FUNCTION__, __LINE__, ##__VA_ARGS__);

#define TELNETD_BD_PRINT(flag, fmt, ...)                            \
{                                                                   \
    if (telnetd_debug_flag & flag)                                  \
    {                                                               \
        if (telnetd_debug_flag & TELNETD_DEBUG_BD_SHOW_THREAD_IP)   \
            TELNETD_DumpCurrentThreadIp(SYSFUN_TaskIdSelf());       \
        TELNETD_PRINT(fmt, ##__VA_ARGS__);                          \
    }                                                               \
}

/*
 * I/O data buffers, pointers, and counters.
 */
#define NIACCUM(c) { pTNPD->netip++ = c; pTNPD->ncc++; }
#define BANNER    "\r\n\r\npSOSystem (%s)\r\n\r\r\n\r"

#define SB_CLEAR()  pTNPD->subpointer = pTNPD->subbuffer;
#define SB_TERM()   { pTNPD->subend = pTNPD->subpointer; SB_CLEAR(); }
#define SB_ACCUM(c) if (pTNPD->subpointer < (pTNPD->subbuffer \
    + sizeof pTNPD->subbuffer)) { *pTNPD->subpointer++ = (unsigned char) (c); }
#define SB_GET()    ((*pTNPD->subpointer++)&0xff)
#define SB_EOF()    (pTNPD->subpointer >= pTNPD->subend)

/*
 * The following are some clocks used to decide how to interpret
 * the relationship between various variables.
 */

#define settimer(x)     (pTNPD->clocks.x = ++pTNPD->clocks.system)
#define sequenceIs(x,y) (pTNPD->clocks.x < pTNPD->clocks.y)

#define TNPD_NO_EVENT                   0
#define TNPD_PROVISION_COMPLETE_EVENT   0x00000001
#define TNPD_ENTER_TRANSITION_EVENT     0x00000002
#define TNPD_WAIT_EVENT (TNPD_PROVISION_COMPLETE_EVENT | TNPD_ENTER_TRANSITION_EVENT)

#define TELNETD_KEEP_ALIVE_ENBLE    TRUE
#define TELNETD_KEEP_ALIVE_IDLE     30      /*sec*/
#define TELNETD_KEEP_ALIVE_INTERVAL 5       /*sec*/
#define TELNETD_KEEP_ALIVE_COUNT    8       /*counter*/

/* DATA TYPE DECLARATIONS
 */
struct ptnpd_t {
    char hisopts[256];
    char myopts[256];
    char ptyibuf[BUFSIZ];
    char *ptyip;
    char ptyobuf[BUFSIZ];
    char *pfrontp;
    char *pbackp;
    char netibuf[BUFSIZ];
    char *netip;
    char netobuf[BUFSIZ];
    char *nfrontp;
    char *nbackp;
    char *neturg;        /* one past last bye of urgent data */
    int  not42;
    int  pcc;
    int  ncc;
    int  pty;
    int  net;
    int  inter;
    int  SYNCHing;       /* we are in TELNET SYNCH mode */
    struct {
        int
        system,          /* what the current time is */
        echotoggle,      /* last time user entered echo unsigned character */
        modenegotiated,  /* last time operating mode negotiated */
        didnetreceive,   /* last time we read data from network */
        ttypeopt,        /* ttype will/won't received */
        ttypesubopt,     /* ttype subopt is received */
        getterminal,     /* time started to get terminal information */
        gotDM;           /* when did we last see a data mark */
    } clocks;
    char *terminaltype;
    char subbuffer[100];
    char *subpointer;
    char *subend;
    int  state;
    char terminalname[5+41];
    char sg_flags;

#if (SYS_CPNT_CLUSTER == TRUE)
    BOOL_T bRelayToMember;
    UI32_T memberIP;
#endif

};
typedef struct ptnpd_t ptnpd_t;

struct  ttychars {
    char    tc_erase;   /* erase last character */
    char    tc_kill;    /* erase entire line */
    char    tc_intrc;   /* interrupt */
    char    tc_quitc;   /* quit */
    char    tc_startc;  /* start output */
    char    tc_stopc;   /* stop output */
    char    tc_eofc;    /* end-of-file */
    char    tc_brkc;    /* input delimiter (like nl) */
    char    tc_suspc;   /* stop process signal */
    char    tc_dsuspc;  /* delayed stop process signal */
    char    tc_rprntc;  /* reprint line */
    char    tc_flushc;  /* flush output (toggles) */
    char    tc_werasc;  /* word erase */
    char    tc_lnextc;  /* literal next character */
};


/*  2002.01.24, William
 *  Record session pair, so show user could get remote site IP.
 *      tnsh_port  -- local port connect to TNSHD   (key)
 *      user_ip    -- Request's IP (remote)
 *      user_port  -- Request's port (remote)
 */
typedef struct  TNPD_SESSION_PAIR_S
{
    UI32_T          tid;                /*  the task own this session
                                         *  if tid eq. 0, the entry is free
                                         */
    UI32_T          tnsh_tid;           /*  the tnsh task-id associated with
                                         *  this socket connection.
                                         */
    /*unsigned        user_ip;*/            /*  remote ip of user       */
    /*unsigned short  user_port;*/          /*  remote port of user     */
    struct sockaddr_in6 user_sa;        /* socket address of user */
    struct sockaddr_in6 local_ip;       /* socket address of local */
    unsigned short  user_local_port;    /*  local side port to user */
    unsigned short  tnsh_port;          /*  port connect to TNSHD   */
    unsigned short  remote_tnsh_port;   /*  remote side(tnsh) port to TNSHD */
    ptnpd_t *ptnpd_p;                   /* jwang adds to collect memory */
}   TNPD_SESSION_PAIR_T;

typedef struct TNPD_SESSION_RECORD_S
{
    int max_sessions;   /*  max sessions could creat    */
    int created_session_count;  /*  created session     */
    TNPD_SESSION_PAIR_T pair[SYS_ADPT_MAX_TELNET_NUM];    /*  point to session pair table */
}   TNPD_SESSION_RECORD_T;

typedef enum
{
    MODE_OPERATING,
    MODE_IDLE
} TaskOperatingMode;

typedef struct
{
    int socket_id;
    tnpcfg_t *cfg_p;
} TELNETD_Shell_Arg_T;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    TELNET_TYPE_TRACE_ID_TNPD_MAIN=0
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void tnpd();
static void telrcv();
static void willoption();
static void wontoption();
static void dooption();
static void dontoption();
static void suboption();
static void mode();
static void intr();
static void sendbrk();
static int ptyflush( register ptnpd_t *pTNPD);
static void netclear();
static int netflush( register ptnpd_t *pTNPD);
static void tnpd_task( tnpcfg_t *cfg);
static void master_routine( tnpcfg_t *cfg);
static int  __tnpd_exit(UI32_T);
#define tnpd_exit(tid)                              \
    ({                                              \
        int    __rc;                                \
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, \
            " terminate tnpd task, tid = %u\n",    \
            (tid == 0)?SYSFUN_TaskIdSelf():tid);    \
        __rc = __tnpd_exit(tid);                    \
        __rc;                                       \
    })

static void Initialize_Session_DB(long max_session);
static void Save_Buffer_Ptr(int tid, ptnpd_t * ptnpd_p);
static void TNPD_AddSessionPair(UI32_T remote_tnsh_port, UI32_T tnsh_port,
                                UI32_T user_local_port,
                                /*UI32_T user_ip, UI32_T user_port*/
                                UI32_T user_salen, struct sockaddr *user_sa,
                                struct sockaddr *local_ip);
static void TNPD_DeleteSessionPair(UI32_T tid);
static void TNPD_RegisterTaskIdInTable(UI32_T tid);
//static void Delete_All_Sessions(void);
static BOOL_T TNPD_IsTelnetShellValid();
#if (SYS_CPNT_CLUSTER == TRUE)
static BOOL_T TELNET_Cluster_RelayToMember(register ptnpd_t *pTNPD);
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
static UI32_T TNPD_GetTelnetShellId(void);
static void TNPD_SW_WatchDogRoutine(UI32_T sw_watchdog_id);
#endif

#ifdef  BACKDOOR_OPEN
static  void TNPD_BackDoor_Menu (void);
#endif  /*  BACKDOOR_OPEN   */
static void TELNETD_DumpCurrentThreadIp(UI32_T tid);

//static int client_read_socket;

/* STATIC VARIABLE DECLARATIONS
 */
//static int n_sessions;                          /*  total created session number    */
static char doopt[] = { IAC, DO, '%', 'c', 0 };
static char dont[] = { IAC, DONT, '%', 'c', 0 };
static char will[] = { IAC, WILL, '%', 'c', 0 };
static char wont[] = { IAC, WONT, '%', 'c', 0 };

static UI32_T   tnpd_task_id;       /*  Telnet Processing Daemon task id    */
static TNPD_SESSION_RECORD_T    session_table;
static UI32_T  session_table_semaphore = 0;
static BOOL_T transition_done = FALSE;

/*isiah.2003-10-06. patch for configuration telnet port number. */
static  BOOL_T  tnpd_task_is_port_changed = FALSE;
/*basen.2008-02-27. stacking mode changed */
static BOOL_T tnpd_task_is_stkmode_changed = FALSE;
static UI32_T telnet_session_number = 0;

static UI32_T telnetd_debug_flag = TELNETD_DEBUG_BD_SHOW_FATAL_ERROR;
static UI32_T telnetd_debug_print = 0; /* 0:backdoor_mgr, 1:printf */

/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------
 *  TelNet Processing Daemon starting entry, create the Daemon.
 *  Return : -1 : fail
 *            0 : successful.
 *-----------------------------------------------------------------------*/
int tnpd_start( tnpcfg_t *cfg)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    static tnpcfg_t def_tnpcfg = {
        222,        /*  task priority   */
        4,          /*  max session     */
        0,          /*  &(trust-client_list[])  */
        {0, 0}      /*  reserve[2]      */
    };

#ifdef  BACKDOOR_OPEN
    UI8_T   ip_backdoor_name[] = "TNPD";
#endif
    /* BODY */
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack((char *)ip_backdoor_name, SYS_BLD_TELNET_GROUP_IPCMSGQ_KEY, TNPD_BackDoor_Menu);
#endif

    if (cfg == NULL)   /* use default cfg if not specified */
        cfg = &def_tnpcfg;

    /*  initialize session number counter and working record    */
    if (cfg->max_sessions > SYS_ADPT_MAX_TELNET_NUM)
        return (-1);
    Initialize_Session_DB(cfg->max_sessions);

    /* start the telnet server task */

    if(SYSFUN_SpawnThread(SYS_BLD_TELNET_DAEMON_THREAD_PRIORITY,
                          SYS_BLD_TELNET_THREAD_SCHED_POLICY,
                          SYS_BLD_TELNET_DAEMON_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          tnpd_task,
                          cfg,
                          &tnpd_task_id)!=SYSFUN_OK)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "SYSFUN_SpawnThread fail.");
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_TELNET_SERVER, tnpd_task_id, SYS_ADPT_TELNET_SERVER_SW_WATCHDOG_TIMER);
#endif

    return(0);
}   /*  end of tnpd_start   */


/* FUNCTION NAME : TNPD_GetSessionPair
 * PURPOSE:
 *      Retrieve a session pair from session table.
 *
 * INPUT:
 *      tnsh_port   -- the port connect to TNSHD.
 *
 * OUTPUT:
 *      user_ip     -- the ip of remote site in socket.
 *      user_port   -- the port of remote site in socket.
 *      tnpd_tid    -- the task id of tnpd child task.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
void TNPD_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_ip, UI32_T *user_port,UI32_T *tnpd_tid)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     i;

    /* BODY */
    if ((user_ip == NULL) || (user_port == NULL))
        return;

    *user_port = 0;
    *tnpd_tid = 0;

    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tnsh_port == tnsh_port)
        {
            L_INET_SockaddrToInaddr( ((struct sockaddr *)&session_table.pair[i].user_sa), user_ip);

            /* sockaddr to L_INET_AddrIp_T
             */
            if ( ((struct sockaddr*)&session_table.pair[i].user_sa)->sa_family == AF_INET)
            {
                struct sockaddr_in *sin4 = (struct sockaddr_in*)&session_table.pair[i].user_sa;

                *user_port = L_STDLIB_Ntoh16( sin4->sin_port );
            }
            else
            {
                struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&session_table.pair[i].user_sa;

                *user_port = L_STDLIB_Ntoh16( sin6->sin6_port );
            }

            *tnpd_tid   = session_table.pair[i].tid;
            SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
            return;
        }
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
    return;
}   /*  end of TNPD_GetSessionPair()    */

/* FUNCTION NAME : TNPD_LoginSignal
 * PURPOSE:
 *      TNSH signal TNPD, I'm use this socket.
 *
 * INPUT:
 *      tnsh_port   -- tnpd site's port connecting to TNSH.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Logout and login are backdoor functions, interact with TNPD.
 */
void TNPD_LoginSignal(UI32_T tnsh_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     i;
    /* BODY
     */
    /*  search the entry associated with tnsh_port  */
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i<session_table.max_sessions; i++)
        if (tnsh_port == session_table.pair[i].tnsh_port)
        {
            session_table.pair[i].tnsh_tid = SYSFUN_TaskIdSelf();
            break;
        }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
    return;
}   /*  end of TNPD_LoginSignal */

/* FUNCTION NAME : TNPD_LogoutSignal
 * PURPOSE:
 *      TNSH signal TNPD this socket is terminated.
 *
 * INPUT:
 *      tnsh_port   -- tnpd site's port connecting to TNSH.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
void TNPD_LogoutSignal(UI32_T tnsh_port)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     i;
    /* BODY
     */
    /*  search the entry associated with tnsh_port  */
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i<session_table.max_sessions; i++)
        if (tnsh_port == session_table.pair[i].tnsh_port)
        {
            session_table.pair[i].tnsh_port = 0;
            session_table.pair[i].tnsh_tid  = 0;
            break;
        }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
    return;
}   /*  end of TNPD_LogoutSignal    */

/* FUNCTION NAME : TNPD_SetTransitionMode
 * PURPOSE:
 *      Ready to transition.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void TNPD_SetTransitionMode(void)
{
//    transition_done = FALSE;
//    setting_transition_mode = TRUE;
}

/* FUNCTION NAME : TNPD_EnterTransitionMode
 * PURPOSE:
 *      Set operating mode as transition
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void TNPD_EnterTransitionMode(void)
{
    transition_done = FALSE;

    TELNET_OM_SetTnpdPort(SYS_DFLT_TELNET_SOCKET_PORT);
    TELNET_MGR_SetTnpdMaxSession(SYS_DFLT_TELNET_DEFAULT_MAX_SESSION);
    TELNET_MGR_SetTnpdStatus(SYS_DFLT_TELNET_DEFAULT_STATE);

    tnpd_task_is_stkmode_changed = TRUE;

    SYSFUN_SendEvent (tnpd_task_id,
        TNPD_ENTER_TRANSITION_EVENT);

    while (! transition_done)
    {
        SYSFUN_Sleep(TELNET_TRANSITION_MODE_SLEEP_TICK);
    }

}

/* FUNCTION NAME : TNPD_ProvisionComplete
 * PURPOSE:
 *      Notify that the CLI provision had been completed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *
 */
void TNPD_ProvisionComplete()
{

// printf("\r\nTNPD_ProvisionComplete");
    SYSFUN_SendEvent (tnpd_task_id,
        TNPD_PROVISION_COMPLETE_EVENT);
}


/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */
#if 0 /* XXX steven.jiang for warnings */
static unsigned long psys_inet_addr(char *ipaddr)
{
        int             i, k;
        char           *byteAddr[4];
        char            addr[32];

        /* Convert the host address from string to number */
        strncpy(addr, ipaddr, 32);
        k = 0;
        byteAddr[k++] = (char *) &addr[0];
        for (i = 0; (i < 32) && (addr[i] != '\0'); i++) {
                if (addr[i] == '.') {
                        addr[i] = '\0';
                        byteAddr[k++] = (char *) &addr[i + 1];
                }
        }

        if (k < 4)
                return (-1);

        /* return the host address */
        return ((atol(byteAddr[0]) << 24) | (atol(byteAddr[1]) << 16)
                | (atol(byteAddr[2]) << 8) | (atol(byteAddr[3]))
                | (atol(byteAddr[3])));
}
#endif /* 0 */

/* FUNCTION NAME : Initialize_Session_DB
 * PURPOSE:
 *      Create session table when telnet daemon is created.
 *
 * INPUT:
 *      max_session -- max session number allowed.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
static void Initialize_Session_DB(long max_session)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int table_size = sizeof(TNPD_SESSION_RECORD_T);
    /* BODY */
    if (0==session_table_semaphore)
    {
        if (SYSFUN_CreateSem (SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &session_table_semaphore)!=SYSFUN_OK)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR, "TNPD halt");
            return;
        }
    }
    memset ((char*)&session_table, 0, table_size);
    session_table.max_sessions          = max_session;
    session_table.created_session_count = 0;
    return;
}   /*  end of Initialize_Session_DB   */

static void Save_Buffer_Ptr(int tid, ptnpd_t * ptnpd_p)
{
    int     i;

    /*  find the entry this task own    */
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i<session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid != tid)
            continue;

        if (session_table.pair[i].ptnpd_p != 0)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "Internal error in tnshd, ptnpd duplicate");
        }
        else
        {
            session_table.pair[i].ptnpd_p = ptnpd_p;
        }
        break;
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
    return;
}   /*  end of Save_Buffer_Ptr  */

/* FUNCTION NAME : TNPD_AddSessionPair
 * PURPOSE:
 *      Add a session pair to session table.
 *
 * INPUT:
 *      remote_tnsh_port    -- remote site port of TNSH (pty) session.
 *      tnsh_port           -- the local side port connect to TNSHD.
 *      user_local_port     -- local site port of TNPD (net) session.
 *      user_ip             -- the ip of remote site in socket.
 *      user_port           -- the port of remote site in socket.(net)
 *      user_salen          -- the size of user_sa
 *      user_sa             -- the ip of remote site in socket
 *      local_ip            -- the ip of local site in socket
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
static void TNPD_AddSessionPair(UI32_T remote_tnsh_port, UI32_T tnsh_port,
                                UI32_T user_local_port,
                                /*UI32_T user_ip, UI32_T user_port*/
                                UI32_T user_salen, struct sockaddr *user_sa,
                                struct sockaddr *local_ip)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI32_T  tid=SYSFUN_TaskIdSelf();
    int     i;

    /*  find the entry this task own    */
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i<session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid == tid)
        {
            UI32_T salen = sizeof(session_table.pair[0].user_sa);

            if (user_salen < salen)
            {
                salen = user_salen;
            }

            memcpy(&session_table.pair[i].user_sa, user_sa, salen);
            memcpy(&session_table.pair[i].local_ip, local_ip, salen);

            /*session_table.pair[i].user_ip          = user_ip;
            session_table.pair[i].user_port        = user_port;*/
            session_table.pair[i].user_local_port  = user_local_port;
            session_table.pair[i].tnsh_port        = tnsh_port;
            session_table.pair[i].remote_tnsh_port = remote_tnsh_port;

            SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
            return;
        }
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
    return;
}   /*  end of TNPD_AddSessionPair  */

/* FUNCTION NAME : TNPD_DeleteSessionPair
 * PURPOSE:
 *      Delete a session pair from session table.
 *
 * INPUT:
 *      tid -- the task-id which will free both direct session.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
static void TNPD_DeleteSessionPair(UI32_T tid)
{
    int     i;
    ptnpd_t *ptnpd_p;

    assert(tid != 0);

    /*  search the entry this task used */
    for (i = 0; i < session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid == tid)
        {
            break;
        }
    }

    if (i >= session_table.max_sessions)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "cannot find session\n");
        return;
    }

    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    /*session_table.pair[i].user_ip          = 0;
    session_table.pair[i].user_port        = 0;*/
    memset(&session_table.pair[i].user_sa, 0, sizeof(session_table.pair[0].user_sa));
    memset(&session_table.pair[i].local_ip, 0, sizeof(session_table.pair[0].local_ip));
    session_table.pair[i].user_local_port  = 0;
    session_table.pair[i].tnsh_port        = 0;
    session_table.pair[i].remote_tnsh_port = 0;
    session_table.pair[i].tid              = 0;
    session_table.pair[i].tnsh_tid         = 0;

    ptnpd_p = session_table.pair[i].ptnpd_p;

    if (NULL == ptnpd_p)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " ptnpd_p = NULL\n");
    }

    if (ptnpd_p != 0)
    {
        if (ptnpd_p->net != 0)
        {
            shutdown(ptnpd_p->net, SHUT_RDWR);
            s_close(ptnpd_p->net);
            ptnpd_p->net = 0;
        }

        if (ptnpd_p->pty != 0)
        {
            shutdown(ptnpd_p->pty, SHUT_RDWR);
            s_close(ptnpd_p->pty);
            ptnpd_p->pty = 0;
        }

        L_MM_Free(ptnpd_p);
        session_table.pair[i].ptnpd_p = 0;
    }

    CLI_PMGR_DecreaseRemoteSession();
    session_table.created_session_count--;
    assert(session_table.created_session_count >= 0);
/*
    if (0 > session_table.created_session_count)
        session_table.created_session_count = 0;
*/
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_UnregisterMonitorThread(SW_WATCHDOG_MAX_MONITOR_ID + i);
#endif

    return;
}

/* FUNCTION NAME : TNPD_GetSessionName
 * PURPOSE:
 *      Get session (ip, port) according socket-id.
 *
 * INPUT:
 *      direct  --  Local(1) or Remote(2).
 *      sid     --  socket-id of the session.
 *
 * OUTPUT:
 *      ip      --  ip addr of the session.
 *      port    --  port of the session.
 *
 * RETURN:
 *      1. If session is not exist, return ip and port as zero.
 *
 * NOTES:
 */
void  TNPD_GetSessionName(UI32_T direct, int sid, UI32_T salen, struct sockaddr *sa)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     res;
    socklen_t slen = salen;

    /**ip = *port = 0;*/
    if (direct == LOCAL_SOCKET)
    {   /*  local site  */
        res = getsockname(sid, sa, /*&salen*/&slen);
    }
    else
    {   /*  remote site */
        res = getpeername(sid, sa, /*&salen*/&slen);
    }

    if (res==0)
    {
        /*
        sock_in = (struct sockaddr_in*) &sock_addr;
        *port = L_STDLIB_Ntoh16(sock_in->sin_port);
        *ip   = L_STDLIB_Ntoh32(sock_in->sin_addr.s_addr);
        */
    }
    return;
}   /*  end of TNPD_GetSessionName  */

/* FUNCTION NAME : TNPD_RegisterTaskIdInTable
 * PURPOSE:
 *      Registered spawned task id, for future reference.
 *
 * INPUT:
 *      tid -- the created task-id.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *          (  Something must be known to use this function. )
 */
static void TNPD_RegisterTaskIdInTable(UI32_T tid)
{
    /*  LOCAL VARIABLE DECLARATION
     */
    int     i;
    /*  BODY
     */
    if (session_table.max_sessions <= session_table.created_session_count)
        /*  syslog : info(session table is full */
        return;
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i < session_table.max_sessions; i++)
    {
        if (0 == session_table.pair[i].tid)
        {
            session_table.created_session_count++;
            memset(&session_table.pair[i], 0, sizeof(session_table.pair[0]));
            /*session_table.pair[i].user_ip          = 0;
            session_table.pair[i].user_port        = 0;*/
            session_table.pair[i].user_local_port  = 0;
            session_table.pair[i].tnsh_port        = 0;
            session_table.pair[i].remote_tnsh_port = 0;
            session_table.pair[i].ptnpd_p          = 0;
            session_table.pair[i].tid              = tid;
            break;
        }
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    /* register to sw watchdog with the SW_WATCHDOG_MAX_MONITOR_ID+session_id (for dynamic create server's threads)*/
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_MAX_MONITOR_ID + i, tid, SYS_ADPT_TELNET_SERVER_SW_WATCHDOG_TIMER );
#endif

    return;
}   /*  end of TNPD_RegisterTaskIdInTable   */

/* FUNCTION NAME : TNPD_IsTelnetShellValid
 * PURPOSE:
 *      Check the associated telnet shell of current task is valid.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- telnet shell is valid
 *      FALSE -- telnet shell is logout, or ERROR
 *
 */
static BOOL_T TNPD_IsTelnetShellValid()
{
    UI32_T tid = SYSFUN_TaskIdSelf();
    UI32_T i;

    /* check the shell task is valid
     */
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for (i=0; i<session_table.max_sessions; i++)
    {
        if (tid == session_table.pair[i].tid)
        {
            /* When telnet shell logout, its tid and port shall be 0
             */
            if (  (0 == session_table.pair[i].tnsh_tid)
                &&(0 == session_table.pair[i].tnsh_port)
               )
            {
                SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
                return FALSE;
            }
            SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
            return TRUE;
        }
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);

    /* no found the registered task id in session table
     */
    return FALSE;
}


#ifdef  BACKDOOR_OPEN
/*  FUNCTION NAME : TNPD_ShowAllConnection
 *  PURPOSE:
 *      Display all connections in system.
 *
 *  INPUT:
 *      None.
 *
 *  OUTPUT:
 *      None.
 *
 *  RETURN:
 *      None.
 *
 *  NOTES:
 *      None.
 */
#define TELNET_TASK_NAME_SIZE 40

void TNPD_ShowAllConnection(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int         i;
    char        td[TELNET_TASK_NAME_SIZE+1], tsh[TELNET_TASK_NAME_SIZE+1];
    /* BODY */

    BACKDOOR_MGR_Printf("\n   Client(IP,Port)- Port ( Port-Port )         (Tnpd)         (Tnsh)");
    BACKDOOR_MGR_Printf ("\n  (--------,-----)------ (----- -----) -------------- --------------");
    for (i=0; i<session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid != 0)
        {
            SYSFUN_TaskIDToName(session_table.pair[i].tid,td, TELNET_TASK_NAME_SIZE);
            SYSFUN_TaskIDToName(session_table.pair[i].tnsh_tid,tsh, TELNET_TASK_NAME_SIZE);
            /*printf ("\n%d (%08x,%5d)-%5d (%5d-%5d) %08x(%s) %08x(%s)", i,*/
            BACKDOOR_MGR_Printf ("\n%d ", i);
            if ( ((struct sockaddr*)&session_table.pair[i].user_sa)->sa_family == AF_INET)
            {
                BACKDOOR_MGR_Printf("(%08X,%5d)",
                    ((struct sockaddr_in*)&session_table.pair[i].user_sa)->sin_addr.s_addr,
                    ((struct sockaddr_in*)&session_table.pair[i].user_sa)->sin_port);
            }
            else
            {
                BACKDOOR_MGR_Printf("(%08X:%08X:%08X:%08X,%5d)",
                    ((struct sockaddr_in6*)&session_table.pair[i].user_sa)->sin6_addr.s6_addr32[0],
                    ((struct sockaddr_in6*)&session_table.pair[i].user_sa)->sin6_addr.s6_addr32[1],
                    ((struct sockaddr_in6*)&session_table.pair[i].user_sa)->sin6_addr.s6_addr32[2],
                    ((struct sockaddr_in6*)&session_table.pair[i].user_sa)->sin6_addr.s6_addr32[3],
                    ((struct sockaddr_in6*)&session_table.pair[i].user_sa)->sin6_port);
            }

            BACKDOOR_MGR_Printf ("-%5d (%5d-%5d) %08x(%s) %08x(%s)",
                    (int)session_table.pair[i].user_local_port,
                    (int)session_table.pair[i].tnsh_port,
                    (int)session_table.pair[i].remote_tnsh_port,
                    (int)session_table.pair[i].tid, td,
                    (int)session_table.pair[i].tnsh_tid, tsh);
        }
    }

}   /*  end of TNPD_ShowAllConnection   */

/* FUNCTION NAME : TNPD_BackDoor_Menu
 * PURPOSE:
 *      Display SYStem Function back door available function and accept user seletion.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static  void TNPD_BackDoor_Menu (void)
{
    char ch[5] = {0};
    int select_value;
    BOOL_T  eof = FALSE, sub_eof = FALSE;


    /*  BODY
     */
    while (!eof)
    {
        memset(ch, 0, sizeof(ch));
        BACKDOOR_MGR_Printf ("\n 0. Exit\n");
        BACKDOOR_MGR_Printf (" 1. Set debug flag\n");
        BACKDOOR_MGR_Printf (" 2. Display Telnet connection entry.\n");
        BACKDOOR_MGR_Printf (" 3. Display mode.\n");
        BACKDOOR_MGR_Printf ("    select =");
        BACKDOOR_MGR_RequestKeyIn(ch, sizeof(ch));
        select_value = atoi(ch);
        switch (select_value)
        {
            case 0:
                eof = TRUE;
                break;

            case 1:
                sub_eof = FALSE;
                while (!sub_eof)
                {
                    BACKDOOR_MGR_Printf ("\r\n");
                    BACKDOOR_MGR_Printf ("   0.  Quit\r\n");
                    BACKDOOR_MGR_Printf ("   1.  None\r\n");
                    BACKDOOR_MGR_Printf ("   2.  TELNETD_DEBUG_CONFIG\r\n");
                    BACKDOOR_MGR_Printf ("   3.  TELNETD_DEBUG_EVENT\r\n");
                    BACKDOOR_MGR_Printf ("   4.  TELNETD_DEBUG_DATABASE\r\n");
                    BACKDOOR_MGR_Printf ("   5.  TELNETD_DEBUG_PACKET\r\n");
                    BACKDOOR_MGR_Printf ("   6.  TELNETD_DEBUG_SHOW_ALL\r\n");
                    BACKDOOR_MGR_Printf ("   7.  TELNETD_DEBUG_ALL\r\n");
                    BACKDOOR_MGR_Printf ("   (Backdoor only)\r\n");
                    BACKDOOR_MGR_Printf ("   95. TELNETD_DEBUG_BD_SHOW_ERROR\r\n");
                    BACKDOOR_MGR_Printf ("   96. TELNETD_DEBUG_BD_SHOW_TRACE\r\n");
                    BACKDOOR_MGR_Printf ("   97. TELNETD_DEBUG_BD_SHOW_DETAIL\r\n");
                    BACKDOOR_MGR_Printf ("   98. TELNETD_DEBUG_BD_SHOW_THREAD_IP\r\n");
                    BACKDOOR_MGR_Printf ("   99. TELNETD_DEBUG_BD_SHOW_ALL\r\n");
                    BACKDOOR_MGR_Printf ("       select =");
                    BACKDOOR_MGR_RequestKeyIn(ch, sizeof(ch));
                    select_value = atoi(ch);
                    switch (select_value)
                    {
                        case 1:
                            telnetd_debug_flag = TELNETD_DEBUG_BD_SHOW_FATAL_ERROR;
                            break;
                        case 2:
                            telnetd_debug_flag |= TELNETD_DEBUG_CONFIG;
                            break;
                        case 3:
                            telnetd_debug_flag |= TELNETD_DEBUG_EVENT;
                            break;
                        case 4:
                            telnetd_debug_flag |= TELNETD_DEBUG_DATABASE;
                            break;
                        case 5:
                            telnetd_debug_flag |= TELNETD_DEBUG_PACKET;
                            break;
                        case 6:
                            telnetd_debug_flag |= TELNETD_DEBUG_SHOW_ALL;
                            break;
                        case 7:
                            telnetd_debug_flag |= TELNETD_DEBUG_ALL;
                            break;

                        case 95:
                            telnetd_debug_flag |= TELNETD_DEBUG_BD_SHOW_ERROR;
                            break;
                        case 96:
                            telnetd_debug_flag |= TELNETD_DEBUG_BD_SHOW_TRACE;
                            break;
                        case 97:
                            telnetd_debug_flag |= TELNETD_DEBUG_BD_SHOW_DETAIL;
                            break;
                        case 98:
                            telnetd_debug_flag |= TELNETD_DEBUG_BD_SHOW_THREAD_IP;
                            break;
                        case 99:
                            telnetd_debug_flag |= TELNETD_DEBUG_BD_SHOW_ALL;
                            break;

                        case 0:
                        default:
                            sub_eof = TRUE;
                            break;
                    }

                    BACKDOOR_MGR_Printf("\r\nSet debug flag 0x%08X\r\n", telnetd_debug_flag);
                }
                break;

            case 2:
                TNPD_ShowAllConnection();
                break;

            case 3:
                BACKDOOR_MGR_Printf ("\r\n");
                BACKDOOR_MGR_Printf ("   0.  backdoor_mgr\r\n");
                BACKDOOR_MGR_Printf ("   1.  printf\r\n");
                BACKDOOR_MGR_Printf ("    select =");
                BACKDOOR_MGR_RequestKeyIn(ch, sizeof(ch));
                select_value = atoi(ch);
                BACKDOOR_MGR_Printf ("\r\n== Change debug print to %s mode ==\r\n",
                    (select_value) ? "printf" : "backdoor_mgr");
                telnetd_debug_print = select_value;
                break;
            default :
                select_value = 0;
                break;
        }
    }   /*  end of while    */
}   /*  TNPD_BackDoor_Menu  */
#endif  /*  BACKDOOR_OPEN   */

static void TELNETD_DumpCurrentThreadIp(UI32_T tid)
{
    int     i;
    //ptnpd_t *ptnpd_p;

    if(tid == 0)
        return;

    for (i = 0; i < session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid == tid)
        {
            break;
        }
    }

    if (i >= session_table.max_sessions)
        return;

    //ptnpd_p = session_table.pair[i].ptnpd_p;

    if (((struct sockaddr*)&session_table.pair[i].user_sa)->sa_family == AF_INET)
    {
        struct sockaddr_in * v4_p = (struct sockaddr_in*)&session_table.pair[i].user_sa;

        if (v4_p->sin_addr.s_addr)
        {
            TELNETD_PRINT("==(%08X,%5d)\r\n", v4_p->sin_addr.s_addr, v4_p->sin_port);
        }
    }
    else
    {
        struct sockaddr_in6 * v6_p = (struct sockaddr_in6*)&session_table.pair[i].user_sa;

        if (v6_p->sin6_addr.s6_addr32[0] | v6_p->sin6_addr.s6_addr32[1] |
            v6_p->sin6_addr.s6_addr32[2] | v6_p->sin6_addr.s6_addr32[3])
        {
            TELNETD_PRINT("==(%08X:%08X:%08X:%08X,%5d)\r\n",
                v6_p->sin6_addr.s6_addr32[0], v6_p->sin6_addr.s6_addr32[1],
                v6_p->sin6_addr.s6_addr32[2], v6_p->sin6_addr.s6_addr32[3],
                v6_p->sin6_port);
        }
    }

    return;
}

#if 0
static void Delete_All_Sessions(void)
{
    int    i;
    UI32_T tid;

    for (i=0; i<session_table.max_sessions; i++)
    {
        tid = session_table.pair[i].tid;
        if (tid == 0)
            continue;

        tnpd_exit(tid);
    }
}
#endif

/*
 * ttloop
 *
 *    A small subroutine to flush the network output buffer, get some data
 * from the network, and pass it through the telnet state machine.  We
 * also flush the pty input buffer (by dropping its data) if it becomes
 * too full.
 */
static BOOL_T
ttloop(
    register ptnpd_t *pTNPD)
{
#define TELNETD_MAX_RECV_TIME_OUT 10 /* Only using while getting terminal type */

    if (pTNPD->nfrontp-pTNPD->nbackp) {
        netflush(pTNPD);
    }

    {
        /* Avoid to wait long time with unknown error
         * cause the watchdog reboot
         */
        struct timeval timeout;
        fd_set         read_fds;

        FD_ZERO(&read_fds);
        FD_SET(pTNPD->net, &read_fds);

        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec  = TELNETD_MAX_RECV_TIME_OUT;
        timeout.tv_usec = 0;

        if (1 != select(pTNPD->net + 1, &read_fds, NULL, NULL, &timeout))
        {
            return FALSE;
        }
    }

    pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof pTNPD->netibuf, 0);
    if (pTNPD->ncc <= 0)
    {
        /*isiah.2003-03-07*/
//        s_close(pTNPD->net);
        //tnpd_exit(0);
        return FALSE;
    }
    else
    {
        pTNPD->netip = pTNPD->netibuf;
        telrcv(pTNPD);            /* state machine */
        if (pTNPD->ncc > 0)
        {
            pTNPD->pfrontp = pTNPD->pbackp = pTNPD->ptyobuf;
            telrcv(pTNPD);
        }
    }

    return TRUE;
}

/*
 * getterminaltype
 *
 *    Ask the other end to send along its terminal type.
 * Output is the variable terminaltype filled in.
 */
static BOOL_T getterminaltype(register ptnpd_t *pTNPD)
{
#define TELNETD_MAX_TERMINAL_TYPE_TIME_OUT 10
#define TELNETD_MAX_TERMINAL_TYPE_RETRY    1

    static char sbuf[] = { IAC, DO, TELOPT_TTYPE };
    int retry = 0;
    struct timeval timeout;

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, ">> Start getterminaltype");

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, ">> Start getterminaltype");

    timeout.tv_sec = TELNETD_MAX_TERMINAL_TYPE_TIME_OUT;
    timeout.tv_usec = 0;
    setsockopt(pTNPD->net, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

    settimer(getterminal);

    memcpy(pTNPD->nfrontp, sbuf, sizeof sbuf);
    pTNPD->nfrontp += sizeof sbuf;
    pTNPD->hisopts[TELOPT_TTYPE] = OPT_YES_BUT_ALWAYS_LOOK;

    retry = 0;
    while (sequenceIs(ttypeopt, getterminal))
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL,
            ">  [IAC DO TELOPT_TTYPE] ttypeopt(%d) < getterminal(%d)",
            pTNPD->clocks.ttypeopt ,pTNPD->clocks.getterminal);

        if (retry++ > TELNETD_MAX_TERMINAL_TYPE_RETRY)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
                "<< Fail set option on net socket %d", pTNPD->net);
            return FALSE;
        }

        if (FALSE == ttloop(pTNPD))
        {
            return FALSE;
        }
    }

    if (pTNPD->hisopts[TELOPT_TTYPE] == OPT_YES) {
        static char sbbuf[] = { IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE };
        memcpy(pTNPD->nfrontp, sbbuf, sizeof sbbuf);
        pTNPD->nfrontp += sizeof sbbuf;

        retry = 0;
        while (sequenceIs(ttypesubopt, getterminal))
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL,
               ">  [IAC SB TELOPT_TTYPE TELQUAL_SEND IAC SE] ttypesubopt(%d) < getterminal(%d)",
                pTNPD->clocks.ttypesubopt ,pTNPD->clocks.getterminal);

            if (retry++ > TELNETD_MAX_TERMINAL_TYPE_RETRY)
            {
                TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
                    "<< Fail set sub_option on net socket %d", pTNPD->net);
                return FALSE;
            }

            if (FALSE == ttloop(pTNPD))
            {
                TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
                    "<< Fail ttloop on net socket %d", pTNPD->net);
                return FALSE;
            }
        }
    }

    timeout.tv_sec =  0;
    timeout.tv_usec = 0;
    setsockopt(pTNPD->net, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(struct timeval));

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, "<< End getterminaltype");
    return TRUE;
}

/*
 * Get a pty, scan input lines.
 */
/*  tnpd_main : the body of telnet service routine,
 *              handle VT protocol and exchange the data between user and TNSH
 *  argv        -- 1. config: keep configuration info.(max_session, priority,...)
 *                 2. socket: new socket which connect to remote request process.
 */
static void tnpd_main(TELNETD_Shell_Arg_T * argv)
{
    /*  LOCAL CONSTANT DECLARATION
     */
    #define SOCKET_NET  1
    #define SOCKET_PTY  2

    /*  LOCAL VARIABLE DECLARATION
     */
    int accepted_socket = argv->socket_id;
    struct sockaddr_in6 sin;
    UI32_T              sinlen;
    register ptnpd_t *pTNPD;
    char c;
    int on=1;
    int pty = 0, i = 0, tid = 0;
//    UI32_T  ip_mask;
    UI32_T  tnsh_port;
    UI32_T  remote_tnsh_port, local_user_port;
    UI32_T  open_socket=0;
#if(SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_TYPE_EntityInfo_T clusterInfo;
#endif

    /*  BODY
     */
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, 0);

#if(SYS_CPNT_CLUSTER == TRUE)
    memset(&clusterInfo,0,sizeof(CLUSTER_TYPE_EntityInfo_T));
    CLUSTER_POM_GetClusterInfo(&clusterInfo);
#endif
    tid = SYSFUN_TaskIdSelf();
    TNPD_RegisterTaskIdInTable(tid);

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
        "== Start new tnpd task, tid = %d, socket = %d ==", tid, accepted_socket);
    if (telnetd_debug_flag & TELNETD_DEBUG_BD_SHOW_TRACE)
    {
        struct sockaddr_in6 user_addr;

        TNPD_GetSessionName(REMOTE_SOCKET, accepted_socket, sizeof(user_addr),
            (struct sockaddr*)&user_addr);

        TELNETD_PRINT("  (%08X:%08X:%08X:%08X,%5d)\r\n",
            user_addr.sin6_addr.s6_addr32[0],
            user_addr.sin6_addr.s6_addr32[1],
            user_addr.sin6_addr.s6_addr32[2],
            user_addr.sin6_addr.s6_addr32[3],
            user_addr.sin6_port);
    }

    open_socket = SOCKET_NET;       /*  (net) socket open   */

    /* allocate data segment */
    if ((pTNPD = (ptnpd_t *) L_MM_Malloc(sizeof(ptnpd_t), L_MM_USER_ID2(SYS_MODULE_TELNET, TELNET_TYPE_TRACE_ID_TNPD_MAIN))) == NULL)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR, "L_MM_Malloc for ptnpd_t fail.");
        goto Exit;
    }

    memset(pTNPD, 0, sizeof(ptnpd_t));
    pTNPD->ptyip = pTNPD->ptyibuf;
    pTNPD->pfrontp = pTNPD->ptyobuf; pTNPD->pbackp = pTNPD->ptyobuf; /*sp modify ,=>;   */
    pTNPD->netip = pTNPD->netibuf;
    pTNPD->nfrontp = pTNPD->netobuf;
    pTNPD->nbackp = pTNPD->netobuf;
    pTNPD->neturg = 0;
    pTNPD->not42 = 1;
    pTNPD->subpointer = pTNPD->subbuffer;
    pTNPD->subend = pTNPD->subbuffer;
    pTNPD->state = TS_DATA;
    strcpy(pTNPD->terminalname, "TERM=");
    pTNPD->sg_flags = ECHO | CRMOD;

#if (SYS_CPNT_CLUSTER == TRUE)
    pTNPD->bRelayToMember = FALSE;
    pTNPD->memberIP = 0;
#endif

    /*
     * get terminal type.
     */
    /* Jason Chen, 09-27-2000, port to phase2 */
    pTNPD->net = accepted_socket;
    /*  pTNPD->net = f; */

    Save_Buffer_Ptr(tid, pTNPD);

    /*  we can only support vt100, so keep silence let default setting (vt100)
     *  take effect.  frankchan 2000/3/27
     */

#if (SYS_CPNT_CLUSTER == TRUE)
    if(clusterInfo.role != CLUSTER_TYPE_ACTIVE_MEMBER)
#endif
    if (FALSE == getterminaltype(pTNPD))
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR, "Call getterminaltype fail.");
        goto Exit;
    }
    /* make connection to the pSOS+ shell */
    /* wuli, 08-27-2000, TCP was defined in pna.h, 0x06
     *    if ((pty = socket(AF_INET4 , SOCK_STREAM, TCP)) < 0)
     */

    /*pTNPD->pty = pty;
    sin.sin_family = AF_INET;
    sin.sin_port = L_STDLIB_Hton16(TNSHD_SERVICE_SOCKET_PORT_NO);*/

    {
        /*UI32_T  tmp_ip, tmp_port;*/

        TNPD_GetSessionName(LOCAL_SOCKET, pTNPD->net, sizeof(sin), (struct sockaddr*)&sin/*&tmp_ip, &tmp_port*/);

        if ( ((struct sockaddr*)&sin)->sa_family == AF_INET)
        {
            struct sockaddr_in *sa4 = (struct sockaddr_in*)&sin;

            sa4->sin_port = L_STDLIB_Hton16(TNSHD_SERVICE_SOCKET_PORT_NO);
            sinlen = sizeof(struct sockaddr_in);

            pty = socket(AF_INET, SOCK_STREAM, 0);
        }
        else
        {
            struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&sin;

            sa6->sin6_port = L_STDLIB_Hton16(TNSHD_SERVICE_SOCKET_PORT_NO);
            sinlen = sizeof(struct sockaddr_in6);

            pty = socket(AF_INET6, SOCK_STREAM, 0);
        }

        /*sin.sin_addr.s_addr = L_STDLIB_Hton32(tmp_ip);*/
    }

    if (pty < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "Create socket(pty) fail.");
        goto Exit;
    }

    open_socket |= SOCKET_PTY;   /*  (pty) socket opened */
    pTNPD->pty = pty;

    /*  for changable managed vlan code.
    UI32_T  vid_ifIndex;
    NETCFG_IP_ADDRESS_MODE_T    addr_mode;
    NETIF_OM_RifNode_T          rif_node;
    vid_ifIndex = 0;
    NETCFG_GetNextIpAddressMode(&vid_ifIndex,&addr_mode);
    rif_node.vid_ifIndex   =vid_ifIndex;
    NETIF_OM_RifNode_T(*rif_node);
    sin.sin_addr = rif_node.ipAddress;
     */
    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, "Try to connect to TNSH 3 times");
    for (i=0; i < 3; i++)
    {
        /*  sin.sin_port = htons(tnsh_get_dport()); */
        if (connect(pty, (struct sockaddr *)&sin, sinlen/*sizeof(sin)*/) >= 0)
            break;

        SYSFUN_Sleep(1<<6);
    }

    if (i >= 3)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR, "Cant connect to TNSH.");
        goto Exit;
    }

    /*  Find out socket pair and save tnsh_port, user_ip, user_port
     *  into sessioin table.
     */
    struct sockaddr_in6 temp;
    struct sockaddr_in6 user;
    struct sockaddr_in6 local;

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
        "Connect to tnsh successed\r\n\r\n");

    TNPD_GetSessionName(LOCAL_SOCKET, pty, sizeof(temp), (struct sockaddr*)&temp/*&tnsh_ip, &tnsh_port*/);
    if ( ((struct sockaddr*)&temp)->sa_family == AF_INET)
    {
        tnsh_port = L_STDLIB_Ntoh16(((struct sockaddr_in*)&temp)->sin_port);
    }
    else
    {
        tnsh_port = L_STDLIB_Ntoh16(((struct sockaddr_in6*)&temp)->sin6_port);
    }

    TNPD_GetSessionName(REMOTE_SOCKET, pty, sizeof(temp), (struct sockaddr*)&temp);
    if ( ((struct sockaddr*)&temp)->sa_family == AF_INET)
    {
        remote_tnsh_port = L_STDLIB_Ntoh16(((struct sockaddr_in*)&temp)->sin_port);
    }
    else
    {
        remote_tnsh_port = L_STDLIB_Ntoh16(((struct sockaddr_in6*)&temp)->sin6_port);
    }

    TNPD_GetSessionName(LOCAL_SOCKET, pTNPD->net, sizeof(temp), (struct sockaddr*)&temp);
    if ( ((struct sockaddr*)&temp)->sa_family == AF_INET)
    {
        local_user_port = L_STDLIB_Ntoh16(((struct sockaddr_in*)&temp)->sin_port);
    }
    else
    {
        local_user_port = L_STDLIB_Ntoh16(((struct sockaddr_in6*)&temp)->sin6_port);
    }

    TNPD_GetSessionName(REMOTE_SOCKET, pTNPD->net, sizeof(user), (struct sockaddr*)&user);
    TNPD_GetSessionName(LOCAL_SOCKET, pTNPD->net, sizeof(local), (struct sockaddr*)&local);
    TNPD_AddSessionPair(remote_tnsh_port, tnsh_port, local_user_port, sizeof(user), (struct sockaddr*)&user, (struct sockaddr*)&local);

    setsockopt(pty, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));

    /* send login flag to the new shell session */
    c = 't';
    if (send(pty, &c, 1, 0) < 0)
    {
        /* fail send out a signal to TNSH */
        goto Exit;
    }
    SYSFUN_Sleep(10); /* if two session income on the same time, the second time send may error */

    tnpd(pTNPD->net, pty, pTNPD);

Exit:
    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
        "== End tnpd task, tid = %d, socket = %d ==", tid, accepted_socket);
    if (telnetd_debug_flag & TELNETD_DEBUG_BD_SHOW_TRACE)
    {
        struct sockaddr_in6 user_addr;

        TNPD_GetSessionName(REMOTE_SOCKET, accepted_socket, sizeof(user_addr),
            (struct sockaddr*)&user_addr);

        TELNETD_PRINT("  (%08X:%08X:%08X:%08X,%5d)\r\n",
            user_addr.sin6_addr.s6_addr32[0],
            user_addr.sin6_addr.s6_addr32[1],
            user_addr.sin6_addr.s6_addr32[2],
            user_addr.sin6_addr.s6_addr32[3],
            user_addr.sin6_port);
    }

    {
        on = 0;
        setsockopt(accepted_socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
    }

    if (pTNPD == NULL)
    {
        if (open_socket & SOCKET_PTY)
        {
            shutdown(pty, SHUT_RDWR);
            s_close(pty);
        }

        if (open_socket & SOCKET_NET)
        {
            shutdown(accepted_socket, SHUT_RDWR);
            s_close(accepted_socket);
        }
    }

    free(argv);
    tnpd_exit(0);
    return;
}   /*  end of tnpd_main    */

static void fatal(f, msg)
    int f;
    char *msg;
{
    char buf[64];

    (void) sprintf(buf, "tnpd: %s.\r\n", msg);
    (void) send(f, buf, strlen(buf), 0);
    /*isiah.2003-03-07*/
//    s_close(f);
    tnpd_exit(0);
    return;
}   /*  end of fatal    */

static void fatalperror(f, msg)
    int f;
    char *msg;
{
    char buf[64];

    (void) sprintf(buf, "%s: %s\r\n", msg, "Fatal Error");
    fatal(f, buf);
}   /*  end of fatalperror  */


/*
 * Check a descriptor to see if out of band data exists on it.
 */

static int  stilloob(
    int s,        /* socket number */
    register ptnpd_t *pTNPD)
{
    static struct timeval timeout = { 0, 0 };
    fd_set excepts;
    int value;

    do {
        FD_ZERO(&excepts);
        FD_SET(s, &excepts);
        value = select(s+1, (fd_set *)0, (fd_set *)0, &excepts, &timeout);
    } while ((value == -1) && (errno == EINTR));

    if (value < 0) {
        fatalperror(pTNPD->pty, "select");
    }
    if (FD_ISSET(s, &excepts)) {
        return 1;
    } else {
        return 0;
    }
}   /*  end of stilloob */


/*  tnpd : Telnet service routine body.
 *      Main loop.  Select from pty and network, and
 *      handle data to telnet receiver finite state machine.
 *          f -- socket which from remote (user) site. (net)
 *          p -- socket which to TNSHD (service routine). (pty)
 *          pTNPD -- workspace for this connection.
 */
static void tnpd( unsigned long f, unsigned long p, register ptnpd_t *pTNPD)
{
    /*  LOCAL CONSTANT DECLARATION
     */
    /*  2002.03.16, William, patch for ptyflush()   */
    #define RETRY_LIMIT_OF_CAN_NOT_WRITE    5000

    /* max wait time for checking telnet shell logout
     */
    enum {MAX_WAIT_TIME_FOR_CHECK_TELNET_SHELL_IN_SECOND = 60};

    /*  LOCAL VARIABLE DECLARATION
     */
    long on = 1, len = sizeof(struct sockaddr_in6);
    /*  2001.10.28, William, replace sockaddr_in with sockaddr,
     *              because there is not defined in socket.h, sockaddr_in is
     *              defined in Phase2 porting, sock_port.h.
     *  struct sockaddr_in myaddr;
     */
    struct  sockaddr_in6    myaddr;
    /*  2002.03.16, William, patch
     *              Add retry count to monitor ptyflush, netflush retry count,
     *              EWOULDBLOCK is no more buffer, do not add counting.
     *              net_retry_count and pty_retry_count for pty, net.
     */
    int     ret, f_status;
    int     pty_retry_count, net_retry_count;
    int     pty_flag, net_flag;
struct timeval timeout = {MAX_WAIT_TIME_FOR_CHECK_TELNET_SHELL_IN_SECOND, 0};
#if(SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_TYPE_EntityInfo_T clusterInfo;
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    UI32_T  monitor_id, session_id;
#endif

    /*  BODY
     */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    session_id = TNPD_GetTelnetShellId();
    if(session_id < session_table.max_sessions)
    {
        monitor_id = SW_WATCHDOG_MAX_MONITOR_ID + session_id;
    }
    else
    {
        return;
    }
#endif

#if (SYS_CPNT_CLUSTER == TRUE)
    memset(&clusterInfo,0,sizeof(CLUSTER_TYPE_EntityInfo_T));
    CLUSTER_POM_GetClusterInfo(&clusterInfo);
#endif

    f_status = fcntl(f, F_GETFL);
    fcntl(f, F_SETFL,f_status | O_NONBLOCK);

#if defined(SO_OOBINLINE)
    setsockopt(pTNPD->net, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on));
#endif
    /*
     * Request to do remote echo and to suppress go ahead.
     */

#if (SYS_CPNT_CLUSTER == TRUE)
    if(clusterInfo.role != CLUSTER_TYPE_ACTIVE_MEMBER)
    {
#endif
    if (!pTNPD->myopts[TELOPT_ECHO]) {
        dooption(TELOPT_ECHO, pTNPD);
    }
    if (!pTNPD->myopts[TELOPT_SGA]) {
        dooption(TELOPT_SGA, pTNPD);
    }
    /*
     * Is the client side a 4.2 (NOT 4.3) system?  We need to know this
     * because 4.2 clients are unable to deal with TCP urgent data.
     *
     * To find out, we send out a "DO ECHO".  If the remote system
     * answers "WILL ECHO" it is probably a 4.2 client, and we note
     * that fact ("WILL ECHO" ==> that the client will echo what
     * WE, the server, sends it; it does NOT mean that the client will
     * echo the terminal input).
     */
    (void) sprintf(pTNPD->nfrontp, doopt, TELOPT_ECHO);
    pTNPD->nfrontp += sizeof doopt-2;
    pTNPD->hisopts[TELOPT_ECHO] = OPT_YES_BUT_ALWAYS_LOOK;

#if (SYS_CPNT_CLUSTER == TRUE)
    }/*end of if(clusterIfo.role != CLUSTER_TYPE_ACTIVE_MEMBER)*/
#endif

    /*
     * Show banner that getty never gave.
     *
     * We put the banner in the pty input buffer.  This way, it
     * gets carriage return null processing, etc., just like all
     * other pty --> client data.
     */
    if (getsockname(f, (struct sockaddr *)&myaddr, (socklen_t *)&len) < 0)
    {
        return;
    }

    /*  Save socket pair info. for "show user"
     *      Show user (in TNSHD) will use pTNPD->pty's local (sin_port) as key,
     *      retrieve pTNPD->net's remote site(ip, sin_port).
     */
    /* Clear ptybuf[0] - where the packet information is received */
    pTNPD->ptyibuf[0] = 0;

    /*
     * Call telrcv() once to pick up anything received during
     * terminal type negotiation.
     */
    telrcv(pTNPD);

    /*  2002.03.16, William, Patch for ptyflush() fail  */
    net_retry_count = pty_retry_count = 0;

    for (;;) {
        int width;
        fd_set ibits, obits, xbits;
        register int c;
        pty_flag = net_flag = 0;

        timeout.tv_sec  = MAX_WAIT_TIME_FOR_CHECK_TELNET_SHELL_IN_SECOND;       /*  no. of seconds to block */
        timeout.tv_usec = 0;       /*  no. of micro seconds to block */

        if (pTNPD->ncc < 0 && pTNPD->pcc < 0)
        {
            break;
        }
        FD_ZERO(&ibits);
        FD_ZERO(&obits);
        FD_ZERO(&xbits);

#if (SYS_CPNT_CLUSTER == TRUE)
/* Check whether need to relay */
        if(pTNPD->bRelayToMember == TRUE)
        {
            if(FALSE == TELNET_Cluster_RelayToMember(pTNPD))
            {
                char msg_buf[50];
                sprintf(msg_buf,"%s","The remote host member is unreachable.\r\n");
                send(f, msg_buf, strlen(msg_buf), 0);
            }
        }
#endif /* SYS_CPNT_CLUSTER */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        TNPD_SW_WatchDogRoutine(monitor_id);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */

        /*
         * Never look for input if there's still
         * stuff in the corresponding output buffer
         */
        if (pTNPD->nfrontp - pTNPD->nbackp || pTNPD->pcc > 0) {
            FD_SET(f, &obits);
            FD_SET(p, &xbits);
        } else {
            FD_SET(p, &ibits);
        }
        if (pTNPD->pfrontp - pTNPD->pbackp || pTNPD->ncc > 0) {
            FD_SET(p, &obits);
        } else {
            FD_SET(f, &ibits);
        }
        if (!pTNPD->SYNCHing) {
            FD_SET(f, &xbits);
        }
        width = max(f, p) + 1;

        if ((c = select(width, &ibits, &obits, &xbits,
                        &timeout)) < 1)
        {
            /*  2001.10.28, William, Replace with ERROR in VxWorks.
             *              as described in VxWorks's doc. only return ERROR
             *              when error occurs.
            if (c == -1) {
                if (errno == EINTR) {
                    continue;
                }
            }
            */

            if (c < 0)
            {
                /* We send signal to telnet client tasks when RIF down
                 * in order to interrupt select() function, it will fall
                 * into here, so don't print error message
                 */
                /* TELNETD_ErrorPrintf("\n TELNETD: select error"); */
                return;
            }

            /* check the associated telnet shell of current task is valid
             */
            if (FALSE == TNPD_IsTelnetShellValid())
                break;

            continue;
        }

        /*
         * Something to read from the network...
         */
        if (FD_ISSET(pTNPD->net, &ibits)) {
#if    !defined(SO_OOBINLINE)
            /*
             * In 4.2 (and 4.3 beta) systems, the
             * OOB indication and data handling in the kernel
             * is such that if two separate TCP Urgent requests
             * come in, one byte of TCP data will be overlaid.
             * This is fatal for Telnet, but we try to live
             * with it.
             *
             * In addition, in 4.2 (and...), a special protocol
             * is needed to pick up the TCP Urgent data in
             * the correct sequence.
             *
             * What we do is:  if we think we are in urgent
             * mode, we look to see if we are "at the mark".
             * If we are, we do an OOB receive.  If we run
             * this twice, we will do the OOB receive twice,
             * but the second will fail, since the second
             * time we were "at the mark", but there wasn't
             * any data there (the kernel doesn't reset
             * "at the mark" until we do a normal read).
             * Once we've read the OOB data, we go ahead
             * and do normal reads.
             *
             * There is also another problem, which is that
             * since the OOB byte we read doesn't put us
             * out of OOB state, and since that byte is most
             * likely the TELNET DM (data mark), we would
             * stay in the TELNET SYNCH (SYNCHing) state.
             * So, clocks to the rescue.  If we've "just"
             * received a DM, then we test for the
             * presence of OOB data when the receive OOB
             * fails (and AFTER we did the normal mode read
             * to clear "at the mark").
             */
          if (pTNPD->SYNCHing) {
            int atmark;

            /* Jason Chen, 09-27-2000, port to phase2 */
            s_ioctl(pTNPD->net, SIOCATMARK, (char *)&atmark, 4);
            if (atmark)
            {
                pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof (pTNPD->netibuf), MSG_OOB);
                if ((pTNPD->ncc == -1) && (errno == EINVAL))
                {
                    pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof (pTNPD->netibuf), 0);
                    if (sequenceIs(didnetreceive, gotDM))
                    {
                        pTNPD->SYNCHing = stilloob(pTNPD->net, pTNPD);
                    }
                }
            }
            else
            {
                pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof (pTNPD->netibuf), 0);
            }
          }
          else
          {
            pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof (pTNPD->netibuf), 0);
          }
          settimer(didnetreceive);
#else    /* !defined(SO_OOBINLINE)) */

            pTNPD->ncc = recv(pTNPD->net, pTNPD->netibuf, sizeof (pTNPD->netibuf), 0);

#endif    /* !defined(SO_OOBINLINE)) */


            if (pTNPD->ncc < 0 && errno == EWOULDBLOCK)
            {
                pTNPD->ncc = 0;

            }
            else {
                if (pTNPD->ncc <= 0)
                {   /*  error or remote site closed, signal tnsh to terminate,
                     *  and wait tnsh terminated.
                     *  0 - EOF, remote site is closed.
                     *  <0- error occurs.
                     *  close (pty) site.
                        shutdown(pTNPD->pty, SHUT_RDWR);
                        s_close(pTNPD->pty);
                     */

                    break;
                }
                pTNPD->netip = pTNPD->netibuf;
            }
        }
        if (FD_ISSET(p, &ibits)) {
            pTNPD->pcc = recv(p, pTNPD->ptyibuf, BUFSIZ, 0);


            if (pTNPD->pcc < 0 && pTNPD->pcc == EAGAIN)
                pTNPD->pcc = 0;
            else {
                if (pTNPD->pcc <= 0)
                {   /*  tnsh terminated, just close and return to tnpd_main */
                    /*    shutdown(pTNPD->net, 2);
                        shutdown(pTNPD->, SHUT_RDWR);
                        s_close(pTNPD->net);
                     */

                    break;
                }
                pTNPD->ptyip = pTNPD->ptyibuf;
            }
        }

        while (pTNPD->pcc > 0) {
            if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp) < 2)
                break;
            c = *pTNPD->ptyip++ & 0377, pTNPD->pcc--;
            if (c == IAC)
                *pTNPD->nfrontp++ = (unsigned char) c;
            *pTNPD->nfrontp++ = (unsigned char) c;
            /* Don't do CR-NUL if we are in binary mode */
            if ((c == '\r') && (pTNPD->myopts[TELOPT_BINARY] == OPT_NO)) {
                if (pTNPD->pcc > 0 && ((*pTNPD->ptyip & 0377) == '\n')) {
                    *pTNPD->nfrontp++ = (unsigned char) *pTNPD->ptyip++ & 0377;
                    pTNPD->pcc--;
                } else
                    *pTNPD->nfrontp++ = '\0';
            }
        }
        if (FD_ISSET(f, &obits) && (pTNPD->nfrontp - pTNPD->nbackp) > 0)
        {
            /*  2002.04.02, William, check return value
             *  Org. code is netflush(pTNPD);
             */
            if ((ret=netflush(pTNPD))<0)
            {
                if (ret == EWOULDBLOCK)
                    net_flag = 2;
                else if ((net_retry_count++) >= RETRY_LIMIT_OF_CAN_NOT_WRITE)
                {
                    break;
                }
            }
            else
                net_retry_count = 0;
        }

#if (SYS_CPNT_CLUSTER == TRUE)
       if(clusterInfo.role == CLUSTER_TYPE_ACTIVE_MEMBER)
           SYSFUN_Sleep(1);
#endif /* SYS_CPNT_CLUSTER */

        if (pTNPD->ncc > 0)
            telrcv(pTNPD);
        if (FD_ISSET(p, &obits) && (pTNPD->pfrontp - pTNPD->pbackp) > 0)
        {   /*  2002.03.16, William, patch for unknown congestion.
             *  Org code is ptyflush(pTNPD);
             */
            if ((ret=ptyflush(pTNPD))<0)
            {
                if (ret==EWOULDBLOCK)
                    pty_flag = 1;
                else if ((pty_retry_count++) >= RETRY_LIMIT_OF_CAN_NOT_WRITE)
                {
                    break;
                }
            }
            else
                pty_retry_count = 0;
            /*  2002.03.16, William, end of patch   */
        }
    }   /*  end of for(;;)  */
    return;
    pty_flag; /* workaround for compiler warning:variable 'pty_flag' set but not used */
}   /*  end of tnpd */

static void telrcv(
    register ptnpd_t *pTNPD)
{
    register int c;

    while (pTNPD->ncc > 0) {
        if ((&pTNPD->ptyobuf[BUFSIZ] - pTNPD->pfrontp) < 2)
            return;
        c = *pTNPD->netip++ & 0377, pTNPD->ncc--;


        switch (pTNPD->state) {

        case TS_CR:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_CR");
            pTNPD->state = TS_DATA;
            /* Strip off \n or \0 after a \r */
            if ((c == 0) || (c == '\n')) {
                break;
            }
            /* FALL THROUGH */

        case TS_DATA:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_DATA");
            if (c == IAC) {
                pTNPD->state = TS_IAC;
                break;
            }
            if (pTNPD->inter > 0)
                break;
            /*
             * We now map \r\n ==> \r for pragmatic reasons.
             * Many client implementations send \r\n when
             * the user hits the CarriageReturn key.
             *
             * We USED to map \r\n ==> \n, since \r\n says
             * that we want to be in column 1 of the next
             * printable line, and \n is the standard
             * unix way of saying that (\r is only good
             * if CRMOD is set, which it normally is).
             */
            if ((c == '\r') && (pTNPD->hisopts[TELOPT_BINARY] == OPT_NO)) {
                pTNPD->state = TS_CR;
            }
            *pTNPD->pfrontp++ = (unsigned char) c;
            break;

        case TS_IAC:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_IAC");
            switch (c) {
            /*
             * Send the process on the pty side an
             * interrupt.  Do this with a NULL or
             * interrupt char; depending on the tty mode.
             */
            case IPR:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " IPR\r\n");
                intr(pTNPD);
                break;

            case BREAK:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " BREAK\r\n");
                sendbrk(pTNPD);
                break;

            /*
             * Are You There?
             */
            case AYT:
            {
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " AYT\r\n");
                /*  ++ Simon, 1999.07.26
                 *  strcpy(pTNPD->nfrontp, "\r\n[Yes]\r\n");
                 *  pTNPD->nfrontp += 9;
                 */
                static char alive[] = "\r\n[Yes]\r\n";
                if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp)
                    >= strlen(alive)+2)
                {
                    strcpy(pTNPD->nfrontp, alive);
                    pTNPD->nfrontp += strlen(alive);
                }
                /*-- Simon  */
            }
                break;

            /*
             * Abort Output
             */
            case AO:
                {
                    TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " AO\r\n");
                    ptyflush(pTNPD);
                    netclear(pTNPD);    /* clear buffer back */
                    *pTNPD->nfrontp++ = IAC;
                    *pTNPD->nfrontp++ = DM;
                    pTNPD->neturg = pTNPD->nfrontp-1; /* off by one XXX */
                    break;
                }

            /*
             * Erase Character and
             * Erase Line
             */
            case EC:
            case EL:
                {
                    char ch;
                    struct ttychars ptty;

                    TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " EC EL\r\n");

                    /*jingyan zheng remove warning, not clear whether assign 0 is correct*/
                    ptty.tc_erase = 0;
                    ptty.tc_kill = 0;
                    ptyflush(pTNPD);
                    ch = (c == EC) ? ptty.tc_erase : ptty.tc_kill;
                    if (ch != '\377')
                        *pTNPD->pfrontp++ = (unsigned char) ch;
                    break;
                }

            /*
             * Check for urgent data...
             */
            case DM:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " DM\r\n");
                pTNPD->SYNCHing = stilloob(pTNPD->net, pTNPD);
                settimer(gotDM);
                break;
            /*
             * Begin option subnegotiation...
             */
            case SB:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " SB\r\n");
                pTNPD->state = TS_SB;
                continue;

            case WILL:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " WILL\r\n");
                pTNPD->state = TS_WILL;
                continue;

            case WONT:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " WONT\r\n");
                pTNPD->state = TS_WONT;
                continue;

            case DO:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " DO\r\n");
                pTNPD->state = TS_DO;
                continue;

            case DONT:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " DONT\r\n");
                pTNPD->state = TS_DONT;
                continue;

            case IAC:
                TELNETD_BD_PRINT(TELNETD_DEBUG_BD_SHOW_DETAIL, " IAC\r\n");
                *pTNPD->pfrontp++ = (unsigned char) c;
                break;
            }
            pTNPD->state = TS_DATA;
            break;

        case TS_SB:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_SB");
            if (c == IAC) {
                pTNPD->state = TS_SE;
            } else {
                SB_ACCUM(c);
            }
            break;

        case TS_SE:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_SE");
            if (c != SE) {
                if (c != IAC) {
                    SB_ACCUM(IAC);
                }
                SB_ACCUM(c);
                pTNPD->state = TS_SB;
            } else {
                SB_TERM();
                suboption(pTNPD);    /* handle sub-option */
                pTNPD->state = TS_DATA;
            }
            break;

        case TS_WILL:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_WILL");
            if (pTNPD->hisopts[c] != OPT_YES)
                willoption(c, pTNPD);
            pTNPD->state = TS_DATA;
            continue;

        case TS_WONT:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_WONT");
            if (pTNPD->hisopts[c] != OPT_NO)
                wontoption(c, pTNPD);
            pTNPD->state = TS_DATA;
            continue;

        case TS_DO:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_DO");
            if (pTNPD->myopts[c] != OPT_YES)
                dooption(c, pTNPD);
            pTNPD->state = TS_DATA;
            continue;

        case TS_DONT:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TS_DONT");
            if (pTNPD->myopts[c] != OPT_NO) {
                dontoption(c, pTNPD);
            }
            pTNPD->state = TS_DATA;
            continue;

        default:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
                "tnpd: panic state=%d", pTNPD->state);

            /*isiah.2003-03-07*/
//            s_close(pTNPD->net);
//            s_close(pTNPD->pty);
            //free(pTNPD);
            tnpd_exit(0);
            return;
        }
    }
}   /*  end of telrcv   */

static void willoption(
    int option,
    register ptnpd_t *pTNPD)
{
    char *fmt;
/*  + Simon, 1999.07.26 */
    char buf[sizeof(doopt)+10];
    if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp) < sizeof(doopt))
    {
        /*  2001.10.28, William, add debug message  */
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " willoption : option info. is not enough.");
        return;     /*  -1  */
    }
/*  - Simon */

    switch (option) {
    case TELOPT_BINARY:
        mode(RAW, 0, pTNPD);
        fmt = doopt;
        break;

    case TELOPT_ECHO:
        pTNPD->not42 = 0;        /* looks like a 4.2 system */
        /*
         * Now, in a 4.2 system, to break them out of ECHOing
         * (to the terminal) mode, we need to send a "WILL ECHO".
         * Kludge upon kludge!
         */
        if (pTNPD->myopts[TELOPT_ECHO] == OPT_YES) {
            dooption(TELOPT_ECHO, pTNPD);
        }
        fmt = dont;
        break;

    case TELOPT_TTYPE:
        settimer(ttypeopt);
        if (pTNPD->hisopts[TELOPT_TTYPE] == OPT_YES_BUT_ALWAYS_LOOK) {
            pTNPD->hisopts[TELOPT_TTYPE] = (unsigned char) OPT_YES;
            return;
        }
        fmt = doopt;
        break;

    case TELOPT_SGA:
        fmt = doopt;
        break;

    case TELOPT_TM:
        fmt = dont;
        break;

    default:
        fmt = dont;
        break;
    }
    if (fmt == doopt) {
        pTNPD->hisopts[option] = (unsigned char) OPT_YES;
    } else {
        pTNPD->hisopts[option] = (unsigned char) OPT_NO;
    }
/*++ Simon, 1999.07.26
 *    (void) sprintf(pTNPD->nfrontp, fmt, option);
 *    pTNPD->nfrontp += sizeof (dont) - 2;
 */
    sprintf(buf, fmt, option);
    strcpy(pTNPD->nfrontp, buf);
    pTNPD->nfrontp += strlen(buf);
    {
        char str_op[TELNET_MAX_LENGTH_OF_TELNET_OPTION+1] = {0};

        if (option >= NTELOPTS)
        {
            snprintf(str_op, sizeof(str_op), "%d", option);
        }
        else
        {
            snprintf(str_op, sizeof(str_op), "%s", telopts[option]);
        }
        str_op[ sizeof(str_op)-1 ] = '\0';

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Received WILL %s", str_op);

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Sent %s %s", (fmt==doopt)?"DO":"DONT", str_op);
    }
    return;     /*  0   */
/*  -- Simon    */
}

static void wontoption(
    int option,
    register ptnpd_t *pTNPD)
{
    char *fmt;
/*  + Simon, 1999.07.26 */
    char buf[sizeof(doopt)+10];
    if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp) < sizeof(doopt))
    {
        return;     /* -1;  */
    }
/*- */
    switch (option) {
    case TELOPT_ECHO:
        pTNPD->not42 = 1;        /* doesn't seem to be a 4.2 system */
        break;

    case TELOPT_BINARY:
        mode(0, RAW, pTNPD);
        break;

    case TELOPT_TTYPE:
        settimer(ttypeopt);
        break;
    }

    fmt = dont;
    pTNPD->hisopts[option] = (unsigned char) OPT_NO;

/*  ++ Simon, 1999.07.26
 *    (void) sprintf(pTNPD->nfrontp, fmt, option);
 *    pTNPD->nfrontp += sizeof (doopt) - 2;
 */
    sprintf(buf, fmt, option);
    strcpy(pTNPD->nfrontp, buf);
    pTNPD->nfrontp += strlen(buf);

    {
        char str_op[TELNET_MAX_LENGTH_OF_TELNET_OPTION+1] = {0};

        if (option >= NTELOPTS)
        {
            snprintf(str_op, sizeof(str_op), "%d", option);
        }
        else
        {
            snprintf(str_op, sizeof(str_op), "%s", telopts[option]);
        }
        str_op[ sizeof(str_op)-1 ] = '\0';

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Received WONT %s", str_op);

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Sent DONT %s", str_op);
    }
    return;     /* 0;   */
/*  -- Simon    */
}   /*  end of wontoption   */

static void dooption(
    int option,
    register ptnpd_t *pTNPD)
{
    char *fmt;
/*  + Simon, 1999.07.26 */
    char buf[sizeof(doopt)+10];
    if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp) < sizeof(doopt))
    {
        return;     /* -1;  */
    }
/*  /-  */

    switch (option) {

    case TELOPT_TM:
        fmt = wont;
        break;

    case TELOPT_ECHO:
        mode(ECHO|CRMOD, 0, pTNPD);
        fmt = will;
        break;

    case TELOPT_BINARY:
        mode(RAW, 0, pTNPD);
        fmt = will;
        break;

    case TELOPT_SGA:
        fmt = will;
        break;

    default:
        fmt = wont;
        break;
    }
    if (fmt == will) {
        pTNPD->myopts[option] = (unsigned char) OPT_YES;
    } else {
        pTNPD->myopts[option] = (unsigned char) OPT_NO;
    }
/*++ Simon, 1999.07.26
 *    (void) sprintf(pTNPD->nfrontp, fmt, option);
 *    pTNPD->nfrontp += sizeof (doopt) - 2;
 */
    sprintf(buf, fmt, option);
    strcpy(pTNPD->nfrontp, buf);
    pTNPD->nfrontp += strlen(buf);

    {
        char str_op[TELNET_MAX_LENGTH_OF_TELNET_OPTION+1] = {0};

        if (option >= NTELOPTS)
        {
            snprintf(str_op, sizeof(str_op), "%d", option);
        }
        else
        {
            snprintf(str_op, sizeof(str_op), "%s", telopts[option]);
        }
        str_op[ sizeof(str_op)-1 ] = '\0';

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Received DO %s", str_op);

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Sent %s %s", (fmt==will)?"WILL":"WONT", str_op);
    }
    return;     /* 0;   */
/*  -- Simon    */
}   /*  end of dooption */

static void dontoption(
    int option,
    register ptnpd_t *pTNPD)
{
    char *fmt;
/*  + Simon, 1999.07.26 */
    char buf[sizeof(doopt)+10];
    if ((&pTNPD->netobuf[BUFSIZ] - pTNPD->nfrontp) < sizeof(doopt))
    {
        return;     /* -1;  */
    }
/*  /-  */

    switch (option) {
    case TELOPT_ECHO:        /* we should stop echoing */
        mode(0, ECHO, pTNPD);
        fmt = wont;
        break;

    default:
        fmt = wont;
        break;
    }

    if (fmt == wont)    {
        pTNPD->myopts[option] = (unsigned char) OPT_NO;
    } else {
        pTNPD->myopts[option] = (unsigned char) OPT_YES;
    }
/*  ++ Simon, 1999.07.26
 *    (void) sprintf(pTNPD->nfrontp, fmt, option);
 *    pTNPD->nfrontp += sizeof (wont) - 2;
 */
    sprintf(buf, fmt, option);
    strcpy(pTNPD->nfrontp, buf);
    pTNPD->nfrontp += strlen(buf);

    {
        char str_op[TELNET_MAX_LENGTH_OF_TELNET_OPTION+1] = {0};

        if (option >= NTELOPTS)
        {
            snprintf(str_op, sizeof(str_op), "%d", option);
        }
        else
        {
            snprintf(str_op, sizeof(str_op), "%s", telopts[option]);
        }
        str_op[ sizeof(str_op)-1 ] = '\0';

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Received DONT %s", str_op);

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL),
            "Sent WONT %s", str_op);
    }
    return;     /* 0;   */
/*  -- Simon    */
}   /*  end of dontoption   */

/*
 * suboption()
 *
 *    Look at the sub-option buffer, and try to be helpful to the other
 * side.
 *
 *    Currently we recognize:
 *
 *    Terminal type is
 */

static void suboption(
    register ptnpd_t *pTNPD)
{
    switch (SB_GET()) {
    case TELOPT_TTYPE: {        /* Yaaaay! */

        settimer(ttypesubopt);
        if (SB_GET() != TELQUAL_IS) {
            return;        /* ??? XXX but, this is the most robust */
        }
        pTNPD->terminaltype = pTNPD->terminalname+strlen(pTNPD->terminalname);

        while ((pTNPD->terminaltype < (pTNPD->terminalname + sizeof pTNPD->terminalname-1)) &&
                                    !SB_EOF()) {
            register int c;

            c = SB_GET();
            if (isupper(c)) {
                c = tolower(c);
            }
            *pTNPD->terminaltype++ = (unsigned char) c;    /* accumulate name */
        }
        *pTNPD->terminaltype = 0;
        pTNPD->terminaltype = pTNPD->terminalname;

        TELNETD_TRACE((TELNETD_DEBUG_PACKET|TELNETD_DEBUG_SHOW_ALL), "Received SB TTYPE %s", pTNPD->terminaltype);

        break;
    }

    default:
        ;
    }
}   /*  end of suboption    */

static void  mode(on, off, pTNPD)
    int on, off;
    register ptnpd_t *pTNPD;
{
    ptyflush(pTNPD);
    pTNPD->sg_flags |= (unsigned char) on;
    pTNPD->sg_flags &= (unsigned char) ~off;
    *pTNPD->pfrontp++ = 0377;
    *pTNPD->pfrontp++ = pTNPD->sg_flags;
}   /*  end of mode */

/*
 * Send interrupt to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write intr char.
 */
static void intr(
    register ptnpd_t *pTNPD)
{
    struct ttychars ptty;

    /*jingyan zheng remove warning, not clear whether assign 0 is correct*/
    ptty.tc_intrc = 0;
    ptyflush(pTNPD);
    if (pTNPD->sg_flags & RAW) {
        *pTNPD->pfrontp++ = '\0';
        return;
    }

    *pTNPD->pfrontp++ = ptty.tc_intrc;
}   /*  end of intr */

/*
 * Send quit to process on other side of pty.
 * If it is in raw mode, just write NULL;
 * otherwise, write quit char.
 */
static void sendbrk(
    register ptnpd_t *pTNPD)
{
    struct ttychars ptty;

    /*jingyan zheng remove warning, not clear whether assign 0 is correct*/
    ptty.tc_quitc = 0;
    ptyflush(pTNPD);
    if (pTNPD->sg_flags & RAW) {
        *pTNPD->pfrontp++ = '\0';
        return;
    }
    *pTNPD->pfrontp++ = ptty.tc_quitc;
}   /*  end of sendbrk  */

/*  2002.03.16, William, Patch for unknown reason.
 *  Condition : For telent, some time send out data will cause error (-12),
 *              then tnpd() enter a infinte loop and packet will queue in P2IP,
 *              cause all packets can't transmit.
 *  Action : change the function declaration and return error code to caller (tnpd).
 *      org. static void ptyflush()
 *      new. static int  ptyflush().
 *           return value : ref. send().
 *  Side effect : maybe cause telnet session unpredicatable broken, but packet won't be
 *                blocked.
 */
static int ptyflush(
    register ptnpd_t *pTNPD)
{
    int n;

    if ((n = pTNPD->pfrontp - pTNPD->pbackp) > 0)
    {
        n = send(pTNPD->pty, pTNPD->pbackp, n, 0);
        /*
         *  DBG_DumpHex (" ptyflush : data=",n,pTNPD->pbackp);
         */

    }

    if (n < 0)
        return(n);
    pTNPD->pbackp += n;
    if (pTNPD->pbackp == pTNPD->pfrontp)
        pTNPD->pbackp = pTNPD->pfrontp = pTNPD->ptyobuf;

    return (n);
}   /*  end of ptyflush */

/*
 * nextitem()
 *
 *    Return the address of the next "item" in the TELNET data
 * stream.  This will be the address of the next character if
 * the current address is a user data character, or it will
 * be the address of the character following the TELNET command
 * if the current address is a TELNET IAC ("I Am a Command")
 * character.
 */
static char *
nextitem(current)
char    *current;
{
    if ((*current&0xff) != IAC) {
        return current+1;
    }
    switch (*(current+1)&0xff) {
    case DO:
    case DONT:
    case WILL:
    case WONT:
    return current+3;
    case SB:        /* loop forever looking for the SE */
    {
        register char *look = current+2;

        for (;;) {
            if ((*look++&0xff) == IAC) {
                if ((*look++&0xff) == SE) {
                return look;
                }
            }
        }
    }
    default:
        return current+2;
    }
}   /*  end of nextitem */

/*
 * netclear()
 *
 *    We are about to do a TELNET SYNCH operation.  Clear
 * the path to the network.
 *
 *    Things are a bit tricky since we may have sent the first
 * byte or so of a previous TELNET command into the network.
 * So, we have to scan the network buffer from the beginning
 * until we are up to where we want to be.
 *
 *    A side effect of what we do, just to keep things
 * simple, is to clear the urgent data pointer.  The principal
 * caller should be setting the urgent data pointer AFTER calling
 * us in any case.
 */

static void netclear(
   register ptnpd_t *pTNPD)
{
    register char *thisitem, *next;
    char *good;
#define    wewant(p)    ((pTNPD->nfrontp > p) && ((*p&0xff) == IAC) && \
                ((*(p+1)&0xff) != EC) && ((*(p+1)&0xff) != EL))

    thisitem = pTNPD->netobuf;

    while ((next = nextitem(thisitem)) <= pTNPD->nbackp) {
        thisitem = next;
    }
    /* Now, thisitem is first before/at boundary. */
    good = pTNPD->netobuf;    /* where the good bytes go */

    while (pTNPD->nfrontp > thisitem) {
        if (wewant(thisitem)) {
            int length;

            next = thisitem;
            do {
                next = nextitem(next);
            } while (wewant(next) && (pTNPD->nfrontp > next));
            length = next-thisitem;
            memcpy(good, thisitem, length);
            good += length;
            thisitem = next;
        } else {
            thisitem = nextitem(thisitem);
        }
    }

    pTNPD->nbackp = pTNPD->netobuf;
    pTNPD->nfrontp = good;        /* next byte to be sent */
    pTNPD->neturg = 0;
}   /*  end of netclear */

/*
 *  netflush
 *        Send as much data as possible to the network,
 *    handling requests for urgent data.
static void netflush(
    register ptnpd_t *pTNPD)
 */

static int netflush(
    register ptnpd_t *pTNPD)
{
    int n;

    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL,
            "netflush");

    if ((n = pTNPD->nfrontp - pTNPD->nbackp) > 0)
    {

        {
            /* Avoid to wait long time with unknown error
             * cause the watchdog reboot
             */
            struct timeval timeout;
            fd_set         write_fds;

            FD_ZERO(&write_fds);
            FD_SET(pTNPD->net, &write_fds);

            memset(&timeout, 0, sizeof(timeout));
            timeout.tv_sec  = 10;
            timeout.tv_usec = 0;

            if (1 != select(pTNPD->net + 1, NULL, &write_fds, NULL, &timeout))
            {
                return 0;
            }
        }
        /*
         * if no urgent data, or if the other side appears to be an
         * old 4.2 client (and thus unable to survive TCP urgent data),
         * write the entire buffer in non-OOB mode.
         */
        if ((pTNPD->neturg == 0) || (pTNPD->not42 == 0))
        {
            n = send(pTNPD->net, pTNPD->nbackp, n, 0);    /* normal write */

        } else
        {
            n = pTNPD->neturg - pTNPD->nbackp;
            /*
             * In 4.2 (and 4.3) systems, there is some question about
             * what byte in a sendOOB operation is the "OOB" data.
             * To make ourselves compatible, we only send ONE byte
             * out of band, the one WE THINK should be OOB (though
             * we really have more the TCP philosophy of urgent data
             * rather than the Unix philosophy of OOB data).
             */
            if (n > 1)
            {
                n = send(pTNPD->net, pTNPD->nbackp, n-1, 0);    /* send URGENT all by itself */
            }
            else
            {
                n = send(pTNPD->net, pTNPD->nbackp, n, MSG_OOB);    /* URGENT data */
            }
        }
    }
    if (n < 0)
    {
        if (errno == EWOULDBLOCK)
            return(n);
        /* should blow this guy away... */
        return(n);
    }
    pTNPD->nbackp += n;
    if (pTNPD->nbackp >= pTNPD->neturg)
    {
        pTNPD->neturg = 0;
    }
    if (pTNPD->nbackp == pTNPD->nfrontp)
    {
        pTNPD->nbackp = pTNPD->nfrontp = pTNPD->netobuf;
    }


    return (n);
}   /*  end of netflush */

# if 0 /* XXX steven.jiang for warnings */
static int tnpd_checkhlist(
    unsigned long addr,
    char **hlist)
{
    int found;
    unsigned long raddr;

    /* check the hlist to see if the client has permission to login */
    found = 0;
    while (*hlist != NULL && (raddr = (unsigned long)psys_inet_addr(*hlist))) {
        if (raddr == addr) {
            found = 1;
            break;
        }
        hlist++;
    }
    if (found == 0)
         return(-1);
    return(0);
}
#endif /* 0 */

static int __tnpd_exit(UI32_T tid)
{
    UI32_T task_id;

    if (tid == 0)
        task_id = SYSFUN_TaskIdSelf();
    else
        task_id = tid;

    TNPD_DeleteSessionPair(task_id);
    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore, SYSFUN_TIMEOUT_WAIT_FOREVER);
    telnet_session_number--;
    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);

    if(tid == 0)
    {
        SYSFUN_DelSelfThread();
    }
    else
//        #warning Not kill the special thread
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " Telnet TASK exit\n");

    return 0;
}

/* FUNCTION NAME:  tnpd_Sigusr1
 * PURPOSE:
 *    This function is a signal handler function
 * INPUT:
 *   int signo -- signal type
 * OUTPUT:
 *    None
 * RETURN:
 *    None
 * NOTES:
 *    None
 */
void tnpd_Sigusr1(int signo)
{
    return;
};

static void tnpd_VxWorksShell(TELNETD_Shell_Arg_T  *args)
{
    /* Register signal handler function such that we can send a signal
     * to interrupt socket select() function in tnpd() with RIF down
     */
    SYSFUN_RegisterSignal(SYS_BLD_SIGNAL_NOTIFY_MESSAGE, tnpd_Sigusr1);

    return tnpd_main(args);
}

static UI32_T Get_Event(UI32_T timeout)
{
    UI32_T event_status, rcv_event;

    event_status = SYSFUN_ReceiveEvent (
         TNPD_WAIT_EVENT
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        |SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
         ,SYSFUN_EVENT_WAIT_ANY,
         timeout, &rcv_event);

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
                TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TNPD halt");
        break;
        }
        return TNPD_NO_EVENT;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    if (rcv_event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
    {
        SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_TELNET_SERVER);
    }
#endif

    if (rcv_event & TNPD_ENTER_TRANSITION_EVENT)
    {
        return TNPD_ENTER_TRANSITION_EVENT;
    }

    if (rcv_event & TNPD_PROVISION_COMPLETE_EVENT)
    {
        return TNPD_PROVISION_COMPLETE_EVENT;
    }
    return TNPD_NO_EVENT;
}

/*----------------------------------------------------------------------
 *  tnpd_task : main body of telnet service daemon.
 *---------------------------------------------------------------------
 */
static void tnpd_task( tnpcfg_t *cfg)
{
    TaskOperatingMode task_running_mode;
    UI32_T rcv_event;
    UI32_T timeout;
    //static int i=1;

    task_running_mode = MODE_IDLE;
    while (1)
    {

        timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        rcv_event = Get_Event(timeout);

        switch (task_running_mode)
        {
        case MODE_IDLE:
            if (rcv_event == TNPD_ENTER_TRANSITION_EVENT)
            {
                transition_done = TRUE;
            }
            else if (rcv_event == TNPD_PROVISION_COMPLETE_EVENT)
            {
                task_running_mode = MODE_OPERATING;
                master_routine(cfg);
            }
            break;

        case MODE_OPERATING:
            if (rcv_event == TNPD_ENTER_TRANSITION_EVENT)
            {
                // do clean house
                //jingyan zheng 2009.03.17
                //the clean house is implemented in TNSHD_ParentTask().
//                Delete_All_Sessions();
                transition_done = TRUE;
                task_running_mode = MODE_IDLE;
            }
            else
            {
               master_routine(cfg);
            }
            break;

        default:
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_DETAIL, "TNPD halt\r\n");
        }
    }
}

/* FUNCTION NAME : set_keep_alive_option
 * PURPOSE : For telnet sessions, use TCP keep-alive mechanism to detect client is alive
 *           or not.
 *           For more detail about the following definition, please see:
 *           http://tldp.org/HOWTO/html_single/TCP-Keepalive-HOWTO/
 * INPUT   : sock   -- socket ID
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE -- success or failure
 * NOTE    : None
 */
static BOOL_T set_keep_alive_option(int sock)
{
    int ret;
    int value;

#if (TELNETD_KEEP_ALIVE_ENBLE == TRUE)

    value = 1;

    ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));

    if (ret < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
            "setsockopt(SO_KEEPALIVE) fail.\r\n");
        return FALSE;
    }

    value = TELNETD_KEEP_ALIVE_COUNT;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPCNT, &value, sizeof(value));

    if (ret < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
            "setsockopt(TCP_KEEPCNT) fail.\r\n");
        return FALSE;
    }

    value = TELNETD_KEEP_ALIVE_IDLE;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPIDLE, &value, sizeof(value));

    if (ret < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
            "setsockopt(TCP_KEEPIDLE) fail.\r\n");
        return FALSE;
    }

    value = TELNETD_KEEP_ALIVE_INTERVAL;
    ret = setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &value, sizeof(value));

    if (ret < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
            "setsockopt(TCP_KEEPINTVL) fail.\r\n");
        return FALSE;
    }
#else
    value = 0;

    ret = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));

    if (ret < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_ERROR,
            "setsockopt(SO_KEEPALIVE) fail.\r\n");
        return FALSE;
    }
#endif /* #if (TELNETD_KEEP_ALIVE_ENBLE == TRUE) */

    return TRUE;
}

#define msg "TELNET Server Test"

/* FUNCTION NAME : master_routine
 * PURPOSE : Master routine in the Telnet daemon.
 * INPUT   : cfg    -- Telnet configuration structure
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
static void master_routine(tnpcfg_t *cfg)
{
    static tnpcfg_t def_tnpcfg =
    {
        222,        /*  task priority   */
        4,          /*  max session     */
        0,          /*  &(trust-client_list[])  */
        {0, 0}      /*  reserve[2]      */
    };

    UI32_T          tid;
    TELNETD_Shell_Arg_T *argv;
    struct sockaddr_in6 sin;
    struct  timeval         timeout;
    fd_set              read_fds;
    char                name[5];
    UI32_T                  port;
    int i, on = 1, id = 0;
    int s, ns, addrlen, err_no;
    int rc;
    int sin_len/* = sizeof(struct sockaddr_in)*/;
    int                     ret; //, is_connected[10];

#if (SYS_CPNT_CLUSTER == TRUE)
    CLUSTER_TYPE_EntityInfo_T clusterInfo;
    struct sockaddr_in6  peer_addr;
    UI32_T commanderIp;
#endif

    while( TELNET_MGR_GetOperationMode() == SYS_TYPE_STACKING_MASTER_MODE )
    {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        /* SYSFUN_SYSTEM_EVENT_SW_WATCHDOG event will be checked and
         * handled in Get_Event()
         * if telnet is disable, we need to get event, too.
         */
        Get_Event(SYSFUN_TIMEOUT_NOWAIT);
#endif

        /**
         ** Wait until TNPD status is enabled
         **/
        if (TELNET_MGR_GetTnpdStatus() == TELNET_STATE_DISABLED)
        {
            SYSFUN_Sleep(20);
            continue;
        }

        TELNET_MGR_GetTnpdPort(&port);
        bzero(&sin, sizeof(sin));

#if (SYS_CPNT_IPV6 == TRUE)
        s = socket(AF_INET6, SOCK_STREAM, 0);
        if (s >= 0) {
            struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&sin;

            sin6->sin6_family = AF_INET6;
            sin6->sin6_port = L_STDLIB_Hton16(port);
            sin_len = sizeof(struct sockaddr_in6);

            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
                " create IPv6 socket successed\r\n\r\n");
        }
        else
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */
        {
            s = socket(AF_INET, SOCK_STREAM, 0);

            if (s >= 0) {
                struct sockaddr_in *sin4 = (struct sockaddr_in*)&sin;

                sin4->sin_family = AF_INET;
                sin4->sin_port = L_STDLIB_Hton16(port);
                sin_len = sizeof(struct sockaddr_in);

                TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE,
                    "create IPv4 socket successed\r\n\r\n");
            }
            else {
                return;
            }
        }

        /*jingyan zheng add to fix defect ES4827G-FLF-ZZ-00130*/
        rc = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        if (rc<0)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR," ** tnpd_task : setsockopt fail. ");
            s_close(s);
            return;
        }

        /* bind socket to the tnpd well-known address */
            /*err_no=bind(s, (struct sockaddr_in *)&sin, sizeof(sin));*/
            err_no=bind(s, (struct sockaddr *)&sin, sin_len);
        if (err_no <0)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "socket bind error_no=%d\n", err_no);
            s_close(s);
            return;
        }

        /* start listening for incoming connections */
    //    if ((err_no=listen(s, 1)) < 0)
        if ((err_no=listen(s, 3)) < 0)
        {
            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "socket listen error_no=%d\n", err_no);
            s_close(s);
            return;
        }

        /* prepare select */
        FD_ZERO(&read_fds);
        FD_SET(s, &read_fds);

        while( (TELNET_MGR_GetOperationMode() == SYS_TYPE_STACKING_MASTER_MODE) && (TELNET_MGR_GetTnpdStatus() == TELNET_STATE_ENABLED) )
        {
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            /* SYSFUN_SYSTEM_EVENT_SW_WATCHDOG event will be checked and
             * handled in Get_Event()
             */
            Get_Event(SYSFUN_TIMEOUT_NOWAIT);
#endif

            if ( tnpd_task_is_port_changed == TRUE )
            {
                tnpd_task_is_port_changed = FALSE;
                break;
            }

            if ( tnpd_task_is_stkmode_changed == TRUE )
            {
                tnpd_task_is_stkmode_changed = FALSE;
                break;
            }

            FD_ZERO(&read_fds);
            FD_SET(s, &read_fds);

            /* for linux it will modify timeout value to zero
            */
            timeout.tv_sec  = 1;    /*  no.  of seconds  */
            timeout.tv_usec = 0;    /*  no. of micro seconds  */
            ret = select(s+1, &read_fds, NULL, NULL, &timeout);
            if(!ret)
                continue;

            if(FD_ISSET(s, &read_fds))
            {
                addrlen = sizeof(sin);
                ns = accept(s, (struct sockaddr *)&sin, (socklen_t *)&addrlen);
                if (ns < 0) {

                    break;
                }

                if (telnet_session_number >= TELNET_OM_GetTnpdMaxSession())
                {
                    TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                            "The max number of local acceptable session is reacheded");
                    shutdown(ns, SHUT_RDWR);
                    s_close(ns);
                    continue;
                }

                {
                    char ip_str[L_INET_MAX_IP6ADDR_STR_LEN+1];
                    L_INET_AddrIp_T   inet_client;

                    memset(&inet_client, 0, sizeof(inet_client));
                    L_INET_SockaddrToInaddr((struct sockaddr *)&sin, &inet_client);
                    L_INET_InaddrToString((L_INET_Addr_T*)&inet_client, ip_str, sizeof(ip_str));

                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_TRACE, "Accetp socket %d for %s", ns, ip_str);

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
                    if (!MGMT_IP_FLT_IsValidIpFilterAddress(MGMT_IP_FLT_TELNET,&inet_client))
                    {
                        TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                            "The peer was not trusted, IP address=%s", ip_str);
                        s_close(ns);
                        continue;
                    }
#endif /* SYS_CPNT_MGMT_IP_FLT */
                }

#if (SYS_CPNT_CLUSTER == TRUE)
                memset(&clusterInfo,0,sizeof(CLUSTER_TYPE_EntityInfo_T));
                CLUSTER_POM_GetClusterInfo(&clusterInfo);

                /* it only accept commander's connectoin if it's as a member
                 */
                if(clusterInfo.role==CLUSTER_TYPE_ACTIVE_MEMBER)
                {
                    int err_code = 0;

                    /* cluster_mgr's commanderIp is in network order
                     */
                    memcpy(&commanderIp,clusterInfo.commander_ip,sizeof(UI32_T));

                    /* getpeername will return IP in network order.
                     *
                     * If getpeername failed or remote IP is not commander's IP,
                     * close the connectio.
                     */
                    addrlen = sizeof(peer_addr);
                    err_code = getpeername(ns, (struct sockaddr *)&peer_addr, (unsigned int *)&addrlen);

                    if(err_code < 0)
                    {
                        char ip_str[L_INET_MAX_IP6ADDR_STR_LEN+1];
                        L_INET_AddrIp_T ipaddr;
                        L_INET_SockaddrToInaddr((struct sockaddr *)&sin, &ipaddr);
                        L_INET_InaddrToString((L_INET_Addr_T*)&ipaddr, ip_str, sizeof(ip_str));

                        TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                            "The peer was not a cluster commander, IP address=%s", ip_str);
                        if (-1 == s_close(ns))
                        {
                            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR,
                                "!!! Close ns(%d) fail", ns);
                            if (telnetd_debug_flag & TELNETD_DEBUG_BD_SHOW_FATAL_ERROR)
                                perror("Close ns:");
                        }
                        continue;
                    }

                    {
                        L_INET_AddrIp_T   inet_commander;

                        memset(&inet_commander, 0, sizeof(inet_commander));
                        L_INET_SockaddrToInaddr((struct sockaddr *)&peer_addr, &inet_commander);

                        /* currently, cluster only supports v4
                         */
                        if( (L_INET_ADDR_TYPE_IPV4 != inet_commander.type)
                            &&(L_INET_ADDR_TYPE_IPV4Z != inet_commander.type) )
                        {
                            TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                                "The peer address is not IPv4 address");
                            s_close(ns);
                            continue;
                        }

                        /* check if source is commander
                         */
                        if(memcmp(inet_commander.addr, clusterInfo.commander_ip, SYS_ADPT_IPV4_ADDR_LEN)!=0)
                        {
                            TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                                "The peer address is commander");
                            s_close(ns);
                            continue;
                        }
                    }
                }
#endif /* SYS_CPNT_CLUSTER */

                if (FALSE == set_keep_alive_option(ns))
                {
                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " ** tnpd_task : setsockopt fail. ");
                    s_close(ns);
                    continue;
                }

                for (i=0; i < 100; i++) {
                    if (++id > 99)
                        id = 1;
                    sprintf(name, SYS_BLD_TELNET_CHILD_TASK, id);
                    if (SYSFUN_TaskNameToID(name, &tid))
                        break;
                }
                if (i >= 100)
                {
                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " cannot build a task name");
                    s_close(ns);
                    continue;
                }

                if ( CLI_PMGR_IncreaseRemoteSession() == FALSE )
                {
                    TELNETD_TRACE((TELNETD_DEBUG_EVENT | TELNETD_DEBUG_PACKET | TELNETD_DEBUG_SHOW_ALL | TELNETD_DEBUG_BD_SHOW_ERROR),
                        "%s", "The max number of shell acceptable session is reacheded");
                    s_close(ns);
                    continue;
                }

                /* Show debug message for accepting a connection
                 */
                {
                    char ip_str[L_INET_MAX_IP6ADDR_STR_LEN+1];
                    L_INET_AddrIp_T ipaddr;
                    L_INET_SockaddrToInaddr((struct sockaddr *)&sin, &ipaddr);
                    L_INET_InaddrToString((L_INET_Addr_T*)&ipaddr, ip_str, sizeof(ip_str));

                    TELNETD_TRACE(((TELNETD_DEBUG_EVENT|TELNETD_DEBUG_PACKET)|TELNETD_DEBUG_SHOW_ALL),
                        "Accepted a connection, IP address=%s", ip_str);
                }

                argv = (TELNETD_Shell_Arg_T *) malloc(sizeof(*argv));
                if (argv == NULL)
                {
                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " Can't allocate memory for argv");
                    shutdown(ns, SHUT_RDWR);
                    s_close(ns);
                    CLI_PMGR_DecreaseRemoteSession();
                    continue;
                }

                if (cfg == NULL)   /* use default cfg if not specified */
                {
                    cfg = &def_tnpcfg;
                }

                argv->cfg_p = cfg;
                argv->socket_id = ns;

                SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
                telnet_session_number ++;
                SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);

                if(SYSFUN_SpawnThread(SYS_BLD_TELNET_DAEMON_THREAD_PRIORITY,
                          SYS_BLD_TELNET_THREAD_SCHED_POLICY,
                          name,
                          SYS_BLD_TASK_LARGE_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          tnpd_VxWorksShell,
                          argv,
                          &tid)!=SYSFUN_OK)
                {
                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "tnpd_task : no system resources for a new session\r\n");
                    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
                    telnet_session_number--;
                    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
                    s_close(ns);
                    free(argv);
                    /*move session number to CLI */
                    CLI_PMGR_DecreaseRemoteSession();

                    continue;
                }
            } /* if(FD_ISSET(s, &read_fds)) */
        } /* end mater mode and enable */

        s_close(s);
    }/* end while master mode */

    return;
}   /*  end of static void master_routine(tnpcfg_t *cfg) */



/* FUNCTION NAME : TNPD_SetTelnetRelaying
 * PURPOSE : This function set the relay status of the specified telnet task
 *           and keep the member's id which the telnet task will relay to.
 * INPUT   : UI32_T task_id, BOOL_T bRelaying, UI32_T memberId
 * OUTPUT  : none
 * RETURN  : TRUE:SUCCESS,  FALSE:FAIL
 * NOTE    : none
 */
BOOL_T TNPD_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId)
{
#if (SYS_CPNT_CLUSTER == TRUE)
    int i;
    UI8_T memberIP[SYS_ADPT_IPV4_ADDR_LEN]={0};

    SYSFUN_ENTER_CRITICAL_SECTION(session_table_semaphore,SYSFUN_TIMEOUT_WAIT_FOREVER);
    for( i = 0; i < session_table.max_sessions; i++)
    {
        if(task_id != session_table.pair[i].tid)
        {
            continue;
        }

        if(session_table.pair[i].ptnpd_p == 0)
        {
            SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
            return FALSE;
        }
        else
       {
            /*session_table.pair[i].ptnpd_p->bRelayToMember = bRelaying;*/

            if(bRelaying == TRUE)
            {
                /* convert member id to IP. */
                if (TRUE == CLUSTER_POM_MemberIdToIp(memberId,memberIP))
                {
                    /* core-layer ip is in network order.
                     */
                    memcpy(&session_table.pair[i].ptnpd_p->memberIP,memberIP,sizeof(UI32_T));
                    session_table.pair[i].ptnpd_p->memberIP = L_STDLIB_Ntoh32(session_table.pair[i].ptnpd_p->memberIP);
                    session_table.pair[i].ptnpd_p->bRelayToMember = bRelaying; /* new add */
                    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
                    return TRUE;
                }
            }
            session_table.pair[i].ptnpd_p->bRelayToMember = bRelaying;
        }
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(session_table_semaphore);
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

    return FALSE;
}

#if (SYS_CPNT_CLUSTER == TRUE)
/*--------------------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_Cluster_RelayToMember
 *--------------------------------------------------------------------------------------
 * PURPOSE : This function creates socket which connects to member's telnet port,and
             exchanges the data between the peer and member connection.
 * INPUT   : register ptnpd_t *pTNPD
 * OUTPUT  : none
 * RETURN  : TRUE
 * NOTE    : none
 *--------------------------------------------------------------------------------------*/

static BOOL_T TELNET_Cluster_RelayToMember(register ptnpd_t *pTNPD)
{
    struct sockaddr_in sin;
    struct timeval         timeout;
    long   on = 1;
    int    soc_cluster, f_status;
    int    cluster_conn_retry_count;
    int    peerRecvSize;
    int    memberRecvSize;
    int    ret;
    char                    peerRecvBuf[BUFSIZ];
    char                    memberRecvBuf[BUFSIZ];

    /* clear the receive buffer */
    memset(peerRecvBuf, 0, BUFSIZ);
    memset(memberRecvBuf, 0, BUFSIZ);
    timeout.tv_sec = 1;     /*  no.  of seconds  */
    timeout.tv_usec = 0;    /*  no. of micro seconds  */

    if (pTNPD->pfrontp - pTNPD->pbackp ||pTNPD->nfrontp-pTNPD->nbackp|| pTNPD->ncc > 0)
       return TRUE;
    /* prepare cluster connection */
    if ((soc_cluster = socket(PF_INET , SOCK_STREAM, 0)) < 0)
    {
        TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " tnpd_main : socket(cluster) create fail.");
        tnpd_exit(0);
        return FALSE;
    }
    sin.sin_family = AF_INET;
    sin.sin_port = L_STDLIB_Hton16(SYS_DFLT_TELNET_SOCKET_PORT);

    /* we get memberIP in network order
     */
    sin.sin_addr.s_addr = L_STDLIB_Hton32(pTNPD->memberIP);

    /* Try to connect at most TNPD_CLUSTER_MAX_RETRY_TIME times */
    for(cluster_conn_retry_count = 0; cluster_conn_retry_count < TNPD_CLUSTER_MAX_RETRY_TIME; cluster_conn_retry_count++)
    {
        if (connect(soc_cluster, (struct sockaddr *)&sin, sizeof(sin)) >= 0)
            break;
        SYSFUN_Sleep(1<<6);
    }

    if(cluster_conn_retry_count < TNPD_CLUSTER_MAX_RETRY_TIME)
    {
        setsockopt(soc_cluster, SOL_SOCKET, SO_KEEPALIVE, (char *)&on, sizeof(on));

        //s_ioctl(soc_cluster, SIOCNBIO, (char *)&on, 4);
        //ioctl(soc_cluster, FIONBIO, (char *)&on);
        f_status = fcntl(soc_cluster, F_GETFL);
        fcntl(soc_cluster, F_SETFL,f_status | O_NONBLOCK);

        while(1)
        {
            int width;
            fd_set ibits;

            FD_ZERO(&ibits);
            FD_SET(pTNPD->net, &ibits);
            FD_SET(soc_cluster, &ibits);

            timeout.tv_sec = 1;     /*  no.  of seconds  */
            timeout.tv_usec = 0;    /*  no. of micro seconds  */

            width = max(pTNPD->net, soc_cluster) + 1;

            if ((ret = select(width, &ibits, 0, 0,&timeout)) < 1)
            {
                if (ret < 0)
                {
                    TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, " cluster_relay select error");
                    s_close(soc_cluster);
                    return FALSE;
                }
               SYSFUN_Sleep(10);
                 continue;
            }

            SYSFUN_Sleep(2);
            /* Someting to read from the peer */
            if (/*(ret > 0) &&*/ FD_ISSET(pTNPD->net, &ibits))
            {
                peerRecvSize = recv(pTNPD->net, peerRecvBuf, sizeof(peerRecvBuf), 0);

                if (peerRecvSize < 0 && errno == EWOULDBLOCK)
                    peerRecvSize = 0;
                else
                {
                    if (peerRecvSize <= 0)
                    {
                        /*  0 - EOF, remote site is closed.
                         *  <0- error occurs.
                         */
                        break;
                    }

                    /* Send data to member */
                    if(peerRecvSize > 0)
                    {
                        if(peerRecvSize != send(soc_cluster,peerRecvBuf,peerRecvSize,MSG_DONTWAIT))
                        {
                            TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "Doesn't send all data to member");
                        }
                     }
                }
           }

           /* Something to read from member */
           else if (/*(ret > 0) &&*/ FD_ISSET(soc_cluster, &ibits))
           {
               memberRecvSize = recv(soc_cluster, memberRecvBuf, sizeof(memberRecvBuf), 0);
               if (memberRecvSize < 0 && errno == EWOULDBLOCK)
                   memberRecvSize = 0;
               else
               {
                   if (memberRecvSize <= 0)
                   {
                      /* 0 - EOF, member terminated
                       * <0- error occures.
                       */
                      break;
                   }

                 /* Send data to peer */
                 if(memberRecvSize > 0)
                 {
                     if(memberRecvSize != send(pTNPD->net,memberRecvBuf,memberRecvSize,MSG_DONTWAIT))
                     {
                          TELNETD_BD_LOG(TELNETD_DEBUG_BD_SHOW_FATAL_ERROR, "Doesn't send all data to peer\r\n");
                     }
                 }
               }
           }
        } /* end of while */

    } /* end of if(cluster_conn_retry_count < TNPD_CLUSTER_MAX_RETRY_TIME) */


      TNPD_SetTelnetRelaying(SYSFUN_TaskIdSelf(),FALSE,0);
      CLI_MGR_SetTelnetRelayingFlag(SYSFUN_TaskIdSelf(),FALSE);

      s_close(soc_cluster);
      return TRUE;
}
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */


/* FUNCTION NAME : TNPD_PortChange
 * PURPOSE:
 *      This function is an event callback when the port is changed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *          .
 */
void TNPD_PortChange(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /*  BODY
    */
    if (TELNET_MGR_GetTnpdStatus() == TELNET_STATE_ENABLED)
    {
        tnpd_task_is_port_changed = TRUE;
    }
}

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
BOOL_T TNPD_GetOptions(UI16_T port, char *myopts, char *hisopts)
{
    int i;

    for (i=0; i<session_table.max_sessions; i++)
        if (session_table.pair[i].remote_tnsh_port == port)
        {
            memcpy(myopts, session_table.pair[i].ptnpd_p->myopts, sizeof(session_table.pair[i].ptnpd_p->myopts));
            memcpy(hisopts, session_table.pair[i].ptnpd_p->hisopts, sizeof(session_table.pair[i].ptnpd_p->hisopts));
            return TRUE;
        }

    return FALSE;
}

/* FUNCTION NAME : TNPD_SendSignalToSessions
 * PURPOSE : This function send SYS_BLD_SIGNAL_NOTIFY_MESSAGE signal to
 *           related active telnet client tasks in order to interrupt the
 *           socket select() function in tnpd()
 * INPUT   : ip_addr_p  -- target local ip address
 *                         if ip_addr_p==NULL, means all telnet sessions
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 */
void TNPD_SendSignalToSessions(L_INET_AddrIp_T *ip_addr_p)
{
    int i;
    L_INET_AddrIp_T local_ip;

    for (i=0; i<session_table.max_sessions; i++)
    {
        if (session_table.pair[i].tid == 0)
            continue;

        if (ip_addr_p)
        {
            if(L_INET_SockaddrToInaddr((struct sockaddr *)&(session_table.pair[i].local_ip), &local_ip) == FALSE)
                continue;

            if ((local_ip.addrlen == ip_addr_p->addrlen) &&
                (memcmp(local_ip.addr, ip_addr_p->addr, ip_addr_p->addrlen)==0))
            {
                SYSFUN_SendSignal(session_table.pair[i].tid, SYS_BLD_SIGNAL_NOTIFY_MESSAGE);
            }
        }
        else /* all telnet sessions */
        {
            SYSFUN_SendSignal(session_table.pair[i].tid, SYS_BLD_SIGNAL_NOTIFY_MESSAGE);
        }
    }
}

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
/* FUNCTION NAME : TNPD_GetTelnetShellId
 * PURPOSE:
 *      Get the session id that associated telnet shell of current task.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 */
static UI32_T TNPD_GetTelnetShellId()
{
    UI32_T tid = SYSFUN_TaskIdSelf();
    UI32_T i;

    /* check the shell task is valid
     */
    for (i=0; i<session_table.max_sessions; i++)
    {
        if (tid == session_table.pair[i].tid)
        {
            return i;
        }
    }

    /* no found the registered task id in session table
     */
    return session_table.max_sessions;
}

/* FUNCTION NAME - TNPD_SW_WatchDogRoutine
 * PURPOSE  : This function is used for the software watch dog routine.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    :
 */
static void TNPD_SW_WatchDogRoutine(UI32_T sw_watchdog_id)
{
    UI32_T   rcv_event, wait_events;

    wait_events = SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;

    SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_NOWAIT, &rcv_event);

    if(rcv_event != 0)
    {
        if(rcv_event & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(sw_watchdog_id);
        }
    }

} /* End of CLI_IO_WatchDogRoutine()*/
#endif /* #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) */


#if (SYS_CPNT_DEBUG == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TELNETD_GetMsgPrefix
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the debug message prefix string.
 * INPUT    : flag            -- the debug flag
 *            max_string_size -- the max size of the output prefix string
 * OUTPUT   : prefix          -- the prefix string
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void TELNETD_GetMsgPrefix(UI32_T flag, char *prefix, UI32_T max_string_size)
{
#define APPEND_BIT_STRING(flag, bit, str, buf_to_append, max_buf_size) \
    if (flag & bit)\
    {\
        if (strlen(str) + strlen(buf_to_append) < max_buf_size)\
        {\
            if (strlen(buf_to_append) != 0)\
                strcat(buf_to_append, "/");\
            strcat(buf_to_append, str);\
        }\
    }

    prefix[0] = 0;

    APPEND_BIT_STRING(flag, TELNETD_DEBUG_CONFIG,     TELNETD_DEBUG_CONFIG_STRING,    prefix, max_string_size);
    APPEND_BIT_STRING(flag, TELNETD_DEBUG_EVENT,      TELNETD_DEBUG_EVENT_STRING,     prefix, max_string_size);
    APPEND_BIT_STRING(flag, TELNETD_DEBUG_DATABASE,   TELNETD_DEBUG_DATABASE_STRING,  prefix, max_string_size);
    APPEND_BIT_STRING(flag, TELNETD_DEBUG_PACKET,     TELNETD_DEBUG_PACKET_STRING,    prefix, max_string_size);
}
#endif /* #if (SYS_CPNT_DEBUG == TRUE) */
