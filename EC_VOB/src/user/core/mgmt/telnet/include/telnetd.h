#ifndef _TELNETD
#define _TELNETD

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "tnpdcfg.h"
#include "l_inet.h"
#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#include "debug_mgr.h"
#include "sys_time.h"
#endif


#if (SYS_CPNT_DEBUG == TRUE)
#define TELNETD_DEBUG_FORMAT    "\r\n%02lu:%02lu:%02lu %s: %s: "
    #define TELNETD_DEBUG_CSC_NAME  "TELNET"

    #if 1
    #define TELNETD_DEBUG_OUT(module_id, flag, format, ...)  \
            DEBUG_MGR_Printf(module_id, DEBUG_TYPE_MATCH_ANY_ANY, ((flag)&TELNETD_DEBUG_CLASS_MASK), ((flag)&TELNETD_DEBUG_FEATURE_MASK), format, ##__VA_ARGS__)
    #else
    #define TELNETD_DEBUG_OUT(module_id, flag, format, args...)  printf(format, ##args)
    #endif

    #define TELNETD_DEBUG(flag, fmt,...) \
        {\
        enum {MAX_LEN_OF_MSG_PREFIX = sizeof(TELNETD_DEBUG_MAX_MESSAGE_PREFIX)+1};\
        int  year, month, day, hour, min, sec;\
        char msg_prefix[MAX_LEN_OF_MSG_PREFIX + 1];\
        \
        SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &min, &sec);\
        TELNETD_GetMsgPrefix(flag, msg_prefix, MAX_LEN_OF_MSG_PREFIX);\
        \
        TELNETD_DEBUG_OUT(DEBUG_TYPE_TELNET, (flag & TELNETD_DEBUG_MASK), TELNETD_DEBUG_FORMAT fmt, \
            (unsigned long)hour, (unsigned long)min, (unsigned long)sec, TELNETD_DEBUG_CSC_NAME, msg_prefix,##__VA_ARGS__);\
        }

    #define TELNETD_DEBUG_CONFIG_STRING          "cfg"
    #define TELNETD_DEBUG_EVENT_STRING           "ev"
    #define TELNETD_DEBUG_DATABASE_STRING        "db"
    #define TELNETD_DEBUG_PACKET_STRING          "pkt"
    #define TELNETD_DEBUG_MAX_MESSAGE_PREFIX     "cfg/ev/db/pkt"

    enum TELNETD_DebugFlag_E
    {
        TELNETD_DEBUG_NONE                 = DEBUG_TYPE_TELNET_NONE,
        TELNETD_DEBUG_CONFIG               = DEBUG_TYPE_TELNET_CONFIG,
        TELNETD_DEBUG_EVENT                = DEBUG_TYPE_TELNET_EVENT,
        TELNETD_DEBUG_DATABASE             = DEBUG_TYPE_TELNET_DATABASE,
        TELNETD_DEBUG_PACKET               = DEBUG_TYPE_TELNET_PACKET,
        TELNETD_DEBUG_SHOW_ALL             = DEBUG_TYPE_TELNET_SHOW_ALL,

        TELNETD_DEBUG_ALL                  = DEBUG_TYPE_TELNET_ALL,

        /* The low-order word is reserved for Debug
         * The high-order word is reserved for backdoor
         */
        TELNETD_DEBUG_MASK                 = 0X0000FFFFL,
        TELNETD_DEBUG_CLASS_MASK           = 0x0000FF00L,
        TELNETD_DEBUG_FEATURE_MASK         = 0x000000FFL,


        TELNETD_DEBUG_BD_SHOW_ERROR        = 0X04000000L,
        TELNETD_DEBUG_BD_SHOW_FATAL_ERROR  = 0X08000000L,
        TELNETD_DEBUG_BD_SHOW_TRACE        = 0X10000000L,
        TELNETD_DEBUG_BD_SHOW_DETAIL       = 0X20000000L,
        TELNETD_DEBUG_BD_SHOW_THREAD_IP    = 0X40000000L,
        TELNETD_DEBUG_BD_SHOW_ALL          = 0XFFFF0000L,

        TELNETD_DEBUG_SHOW_TO_BACKDOOR_ONLY= 0X80000000L,
    };

#else
    #define TELNETD_DEBUG(flag, fmt, args...)        ((void)0)

    enum TELNETD_DebugFlag_E
    {
        TELNETD_DEBUG_NONE                 = 0x00000000L,
        TELNETD_DEBUG_CONFIG               = 0x00000100L,
        TELNETD_DEBUG_EVENT                = 0x00000200L,
        TELNETD_DEBUG_DATABASE             = 0x00000400L,
        TELNETD_DEBUG_PACKET               = 0x00000800L,
        TELNETD_DEBUG_SHOW_ALL             = 0X000000FFL,

        TELNETD_DEBUG_ALL                  = 0x0000FFFFL,

        TELNETD_DEBUG_BD_SHOW_ERROR        = 0X04000000L,
        TELNETD_DEBUG_BD_SHOW_FATAL_ERROR  = 0X08000000L,
        TELNETD_DEBUG_BD_SHOW_TRACE        = 0X10000000L,
        TELNETD_DEBUG_BD_SHOW_DETAIL       = 0X20000000L,
        TELNETD_DEBUG_BD_SHOW_THREAD_IP    = 0X40000000L,
        TELNETD_DEBUG_BD_SHOW_ALL          = 0XFFFF0000L,

        TELNETD_DEBUG_SHOW_TO_BACKDOOR_ONLY= 0X80000000L,

        /* The low-order word is reserved for Debug
         * The high-order word is reserved for backdoor
         */
        TELNETD_DEBUG_MASK                 = 0X0000FFFFL,
    };

#endif /* #if (SYS_CPNT_DEBUG == TRUE) */


#define LOCAL_SOCKET       1
#define REMOTE_SOCKET      2

#define EWOULDBLOCK EAGAIN
#define IP_PROT_TCP AF_INET
#define s_close(fd) close(fd)

#if 0
#define TELNETD_DebugPrintf printf
#else
#define TELNETD_DebugPrintf(...) do {} while (0)
#endif
#define TELNETD_ErrorPrintf printf

/* Add telnet client, Jinnee.Jin, 2008, 3, 5 */
BOOL_T TNPD_GetOptions(UI16_T port, char *myopts, char *hisopts);

int tnpd_start(tnpcfg_t *cfg);  /*, char *argv[]    */

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
 *     1. if user_ip, user_port is zero means the session is not found.
 */
void TNPD_GetSessionPair(UI32_T tnsh_port, L_INET_AddrIp_T *user_ip/*UI32_T *user_ip*/, UI32_T *user_port,UI32_T *tnpd_tid);


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
void TNPD_LoginSignal(UI32_T tnsh_port);


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
 *      1. Logout and login are backdoor functions, interact with TNPD.
 */
void TNPD_LogoutSignal(UI32_T tnsh_port);


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
 *          (  Something must be known to use this function. )
 */
void  TNPD_GetSessionName(UI32_T direct, int sid, UI32_T salen, struct sockaddr *sa);


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
void TNPD_SetTransitionMode(void);

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
void TNPD_EnterTransitionMode(void);

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
void TNPD_ProvisionComplete(void);



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
void TNPD_PortChange(void);

/* FUNCTION NAME : TNPD_SetTelnetRelaying
 * PURPOSE : This function set the relay status of the specified telnet task
 *           and keep the member's id which the telnet task will relay to.
 * INPUT   : UI32_T task_id, BOOL_T bRelaying, UI32_T memberId
 * OUTPUT  : none
 * RETURN  : TRUE:SUCCESS,  FALSE:FAIL
 * NOTE    : none
 */
BOOL_T TNPD_SetTelnetRelaying(UI32_T task_id, BOOL_T bRelaying, UI8_T memberId);

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
void TNPD_SendSignalToSessions(L_INET_AddrIp_T *ip_addr_p);

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
 * ---------------------------------------------------------------------*/
void TELNETD_GetMsgPrefix(UI32_T flag, char *prefix, UI32_T max_string_size);
#endif /* #if (SYS_CPNT_DEBUG == TRUE) */

#endif
