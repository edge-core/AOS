/* FILE NAME  -  traceroute_mgr.c
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *              TraceRoute supported in this packet sends out prob packet using
 *              UDP datagrams and waits for ICMP reply (TIME_EXCEED or UNREACHABLE PORT)
 *              for results.  BOTH Send and receive operation are done thru socket.
 *
 *
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch product lines.
 *
 *
 *  History
 *      2003.7.1   Amytu       Redesign
 *      2007/12    peter_yu    Porting to linux platform.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 *                                  (2) Handle IPC Message.
 * ------------------------------------------------------------------------
 * Copyright(C)                   ACCTON Technology Corp. 2003-2007
 * ------------------------------------------------------------------------
 */



/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>      /* for iphdr */
#include <netinet/udp.h>     /* for udphdr */
#include <netinet/ip_icmp.h> /* for icmphdr */
#include <sys/socket.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "sys_time.h"
#include "l_stdlib.h"
#include "l_inet.h"
#include "ip_lib.h"
#include "leaf_2925.h"
#include "backdoor_mgr.h"
#include "traceroute_type.h"
#include "traceroute_mgr.h"
#include "traceroute_om.h"
#include "traceroute_om_private.h"
#include "traceroute_task.h"
#include "netcfg_pom_ip.h"
#include "errno.h"
#include "err.h"

#if (SYS_CPNT_IPV6 == TRUE)
#include "vlan_lib.h"
#include "linux/icmpv6.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define BACKDOOR_OPEN                       TRUE

#define MAX_BUF_LEN                             32
#define TRACEROUTE_MGR_TEST_NAME                "test%lu"
#define TRACEROUTE_MGR_REFRESH_TIMER            (120 *SYS_BLD_TICKS_PER_SECOND)
#define TRACEROUTE_MGR_DESTUDPPORT              33435
#define TRACEROUTE_MGR_TIMEOUT_MICROSECOND      950000 /* For each 1 second, we use 0.05 seconds for sending pkt and 0.95 seconds for waiting. */

/* MACRO FUNCTION DECLARATIONS
 */

#define TRACEROUTE_MGR_RELEASE_CSC(ret_value)        \
{                                                       \
    SYSFUN_RELEASE_CSC();                               \
    return ret_value;                                   \
} /* end of TRACEROUTE_MGR_RELEASE_CSC() */

#define DEBUG_PRINTF(args...) do { \
if (traceroute_backdoor_debug) \
   BACKDOOR_MGR_Printf(args); \
} while(0);

/* to remove the same one defined in netcfg_type.h,
 * included by netcfg_pom_ip.h
 */
#undef DUMP_INET_ADDR

#define DUMP_INET_ADDR(a)  do{\
    if(traceroute_backdoor_debug) { \
        int i;\
        printf("a.type: %d\n", a.type);\
        printf("a.addrlen: %d\n", a.addrlen);\
        printf("a.addr: ");\
        for(i=0;(i<a.addrlen) || (i<4) ;i++) /* print at least first 4 addr, in case addrlen is 0 */\
        {\
            printf("%x ",a.addr[i]);\
        }\
        printf("\n");\
        printf("a.preflen: %d\n", a.preflen);\
        printf("a.zoneid: %ld\n", (long int)a.zoneid);\
    } \
} while(0)

/* LOCAL DATATYPE DECLARATION
 */

/* format of a (udp) probe packet.
 */
typedef struct ip_packet_S
{
//  IPDU    ip;                     /* ip header */
    UI8_T   ver_len;
    UI8_T   tos;
    UI16_T  length;
    UI32_T  i_dont_use;
    UI8_T   ttl;
    UI8_T   protocol;
    UI16_T  checksum;
    UI8_T   srcIp[SYS_ADPT_IPV4_ADDR_LEN];
    UI8_T   dstIp[SYS_ADPT_IPV4_ADDR_LEN];
    // peter, PING_MGR_ICMP_T     icmp;       /* icmp header */
} __attribute__((packed, aligned(1))) ip_packet_T;


struct opacket
{
    //IPDU    ip;             /* ip header */
    //UPDU    udp;            /* udp header */
    UI8_T   packet_seq;     /* sequence number of this packet */
    UI8_T   packet_ttl;     /* ttl packet left with */
    UI32_T  packet_time;    /* time packet left */
} __attribute__((packed, aligned(1))) ;

typedef struct
{
    UI8_T   type;
    UI8_T   code;
    UI16_T  checksum;
    UI16_T  id;
    UI16_T  seq;
} __attribute__((packed, aligned(1))) ICMP6_T;


#if (SYS_CPNT_IPV6 == TRUE)
struct opacket6
{
    UI8_T   packet_seq;     /* sequence number of this packet */
    UI8_T   packet_ttl;     /* ttl packet left with */
    UI32_T  packet_time;    /* time packet left */
} __attribute__((packed, aligned(1))) ;

typedef struct
{
    UI32_T  ver_traffic_flow;
    UI16_T  length;
    UI8_T   next_header;
    UI8_T   ttl;
    UI8_T   src_addr[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T   dest_addr[SYS_ADPT_IPV6_ADDR_LEN];
} __attribute__((packed, aligned(1))) ipv6_packet_T;
#endif

/*  Workspace for each client
 */

typedef struct TRACEROUTE_MGR_WorkSpace_S
{
    struct  opacket     outpacket;   /* packet to be send */
#if (SYS_CPNT_IPV6 == TRUE)
    struct  opacket6     outpacket6; /* packet to be send */
#endif
    struct  sockaddr_in whereto;     /* destination sock addr */
#if (SYS_CPNT_IPV6 == TRUE)
    struct  sockaddr_in6 whereto6;      /* destination sock addr */
#endif

    UI32_T  owner_tid;               /* owner task ID */
    UI32_T  target_addr_type;
    L_INET_AddrIp_T src_ip;         /* the interface IP address of outbound probe */
    L_INET_AddrIp_T dst_ip;         /* target to be probed */

    UI16_T  probe_port;              /* default is (0x8000 | (time & 0x00ff)) XXXX*/
    UI16_T  ident;                   /* default is (0x8000 | (taskid & 0xffff)) */
    UI16_T  datalen;                 /* send out data len */
    UI32_T  admin_status;            /* this flag must be turned on when operation starts */
    UI16_T  rcv_pkt_len;             /* received packet length in packet */
    UI32_T  last_transmit_time;
    UI32_T  last_receive_time;
    UI32_T  unreceived_probs_per_hop;           /* Number of unreceived probs per hop */
    UI32_T  max_hop_failures;                   /* max number of consecutive hop failures before termination */
    UI32_T  hop_failures;                       /* current number of consecutive hop failures */
    UI32_T  last_access_time;

}   TRACEROUTE_MGR_WorkSpace_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void   TRACEROUTE_MGR_InitControlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctl_entry_p);
/*static void   TRACEROUTE_MGR_RefreshOM(void);*/
static UI32_T TRACEROUTE_MGR_WaitForReply(void);
static UI32_T TRACEROUTE_MGR_SendProbe(void);
static void TRACEROUTE_MGR_CheckToStartTimer(void);
static UI32_T TRACEROUTE_MGR_ResolveIcmpType(UI32_T workspace_index, UI32_T hop, UI32_T prob,
                                             UI8_T icmp_packet_type, UI8_T icmp_packet_code,
                                             struct sockaddr *from_p);


#if (BACKDOOR_OPEN == TRUE)
static void   TRACEROUTE_MGR_BackDoor_Menu(void);
static  BOOL_T  TRACEROUTE_MGR_StrToVal(UI8_T *str_p, UI32_T *value_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static TRACEROUTE_MGR_WorkSpace_T    trace_route_workspace[SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE];

static BOOL_T   traceroute_backdoor_debug;
static UI32_T   current_operation_mode;
static UI32_T   test_session = 0;
static I32_T traceroute_send_socket = -1;
static I32_T traceroute_receive_socket = -1;
#if (SYS_CPNT_IPV6 == TRUE)
static I32_T traceroute6_send_socket = -1;
static I32_T traceroute6_receive_socket = -1;
#endif
static TRACEROUTE_TYPE_TraceRouteResultsEntry_T     global_result_entry;
static TRACEROUTE_TYPE_TraceRouteCtlEntry_T         global_control_entry;

SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME : TRACEROUTE_MGR_Initiate_System_Resources
 * PURPOSE:
 *      Initialize working space of trace route utility.
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
 *      1. working space is used to limit amount of ping service could be supported.
 *      2. The max simutanious trace route requests is defined in SYS_BLD.h.
 */
void TRACEROUTE_MGR_Initiate_System_Resources (void)
{
    memset(trace_route_workspace, 0, sizeof(TRACEROUTE_MGR_WorkSpace_T)*(SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE));

    TRACEROUTE_OM_Initiate_System_Resources();
    //current_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;

} /* end of TRACEROUTE_MGR_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void TRACEROUTE_MGR_Create_InterCSC_Relation(void)
{
#if (BACKDOOR_OPEN == TRUE)
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("traceroute",
        SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY, TRACEROUTE_MGR_BackDoor_Menu);
#endif
} /* end of TRACEROUTE_MGR_Create_InterCSC_Relation */


/* FUNCTION NAME : TRACEROUTE_MGR_EnterMasterMode
 * PURPOSE:
 *      TraceRoute enters master mode.
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
 *
 */
void TRACEROUTE_MGR_EnterMasterMode (void)
{
    SYSFUN_ENTER_MASTER_MODE();
    current_operation_mode = SYS_TYPE_STACKING_MASTER_MODE;
    TRACEROUTE_OM_EnterMasterMode();
    return;
} /* end of TRACEROUTE_MGR_EnterMasterMode() */


/* FUNCTION NAME : TRACEROUTE_MGR_EnterSlaveMode
 * PURPOSE:
 *      TraceRoute enters Slave mode mode.
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
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void    TRACEROUTE_MGR_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    current_operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
    return;
} /* end of TRACEROUTE_MGR_EnterSlaveMode() */


/* FUNCTION NAME : TRACEROUTE_MGR_EnterTransitionMode
 * PURPOSE:
 *      TraceRoute enters Transition mode mode.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 */
void TRACEROUTE_MGR_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    current_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;

    memset(trace_route_workspace, 0, sizeof(TRACEROUTE_MGR_WorkSpace_T)*(SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE));
    memset(&global_result_entry,   0, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    memset(&global_control_entry,  0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    traceroute_backdoor_debug = FALSE;
    test_session = 0;
    if(traceroute_send_socket >= 0)
    {
        close(traceroute_send_socket);
        traceroute_send_socket = -1;
    }
    if(traceroute_receive_socket >= 0)
    {
        close(traceroute_receive_socket);
        traceroute_receive_socket = -1;
    }

#if (SYS_CPNT_IPV6 == TRUE)
    if(traceroute6_send_socket >= 0)
    {
        close(traceroute6_send_socket);
        traceroute6_send_socket = -1;
    }
    if(traceroute6_receive_socket >= 0)
    {
        close(traceroute6_receive_socket);
        traceroute6_receive_socket = -1;
    }
#endif
    return;
}


/* FUNCTION NAME : TRACEROUTE_MGR_SetTransitionMode
 * PURPOSE:
 *      Traceroute enters set transition mode.  Clear OM
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void TRACEROUTE_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    TRACEROUTE_OM_ClearOM();
    return;
} /* end of TRACEROUTE_MGR_SetTransitionMode() */

/* FUNCTION NAME - TRACEROUTE_MGR_GetOperationMode
 * PURPOSE  :
 *      This functions returns the current operation mode of this component
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
 *      None. */
SYS_TYPE_Stacking_Mode_T  TRACEROUTE_MGR_GetOperationMode(void)
{
    return current_operation_mode;

} /* end of TRACEROUTE_MGR_GetOperationMode() */

/* FUNCTION NAME: TRACEROUTE_MGR_TriggerTraceRoute
 * PURPOSE:
 *          To start trace route periodically
 * INPUT:
 *          None
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T  TRACEROUTE_MGR_TriggerTraceRoute(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    ret = TRACEROUTE_MGR_SendProbe();
    TRACEROUTE_MGR_WaitForReply();

    return ret;

} /* end of TRACEROUTE_MGR_TriggerTraceRoute() */


/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 */
UI32_T  TRACEROUTE_MGR_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    UI32_T res;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    if ((ctl_entry_p->trace_route_ctl_rowstatus < VAL_traceRouteCtlRowStatus_active) ||
        (ctl_entry_p->trace_route_ctl_rowstatus > VAL_traceRouteCtlRowStatus_destroy))
        return TRACEROUTE_TYPE_FAIL;

    res = TRACEROUTE_OM_IsTraceRouteControlEntryExist(ctl_entry_p->trace_route_ctl_owner_index, ctl_entry_p->trace_route_ctl_owner_index_len,
                        ctl_entry_p->trace_route_ctl_test_name,ctl_entry_p->trace_route_ctl_test_name_len);

    DEBUG_PRINTF("%s, %d, res: %lx\n", __FUNCTION__, __LINE__, (unsigned long)res);

    switch (ctl_entry_p->trace_route_ctl_rowstatus)
    {
        case VAL_traceRouteCtlRowStatus_createAndWait:
        case VAL_traceRouteCtlRowStatus_createAndGo:
            if (res == TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;

            break;

        case VAL_traceRouteCtlRowStatus_active:
        case VAL_traceRouteCtlRowStatus_destroy:
            if (res != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;

            break;
        default:
            return TRACEROUTE_TYPE_FAIL;
            break;
    } /* end of switch */

    res = TRACEROUTE_OM_SetTraceRouteCtlEntry(*ctl_entry_p);
    DEBUG_PRINTF("%s, %d, res: %lx\n", __FUNCTION__, __LINE__, (unsigned long)res);

    TRACEROUTE_MGR_CheckToStartTimer();

    return res;

} /* end of TRACEROUTE_MGR_SetTraceRouteCtlEntry() */

/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus
 * PURPOSE:
 *          To enable or disable traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_admin_status      -- The admin status of the traceroute entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    /* Local Variable Declaration
     */
    UI32_T      res;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        local_entry;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    DEBUG_PRINTF("%s, %d, admin_status: %ld\n", __FUNCTION__, __LINE__, (long)ctl_entry_p->trace_route_ctl_admin_status);

    if ((ctl_entry_p->trace_route_ctl_admin_status != VAL_traceRouteCtlAdminStatus_enabled) &&
        (ctl_entry_p->trace_route_ctl_admin_status != VAL_traceRouteCtlAdminStatus_disabled))
        return TRACEROUTE_TYPE_INVALID_ARG;

    memset(&local_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    memcpy(&local_entry, ctl_entry_p, sizeof(local_entry));

    if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&local_entry)) != TRACEROUTE_TYPE_OK)
    {
        DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "get entry failed.");
        return TRACEROUTE_TYPE_FAIL;
    }
    if (local_entry.trace_route_ctl_admin_status == ctl_entry_p->trace_route_ctl_admin_status )
        return TRACEROUTE_TYPE_OK;  //do nothing.

    // Setting admin status in a ctl_entry is only permitted when the row status is active.
    if (local_entry.trace_route_ctl_rowstatus != VAL_traceRouteCtlRowStatus_active)
    {
        DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "");
        return TRACEROUTE_TYPE_FAIL;
    }

    local_entry.trace_route_ctl_admin_status = ctl_entry_p->trace_route_ctl_admin_status;


    if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(local_entry)) != TRACEROUTE_TYPE_OK)
    {
        DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "");
        return TRACEROUTE_TYPE_FAIL;
    }

    TRACEROUTE_MGR_CheckToStartTimer();

    return res;

} /* end of TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus() */

#if 0
/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlRowStatus
 * PURPOSE:
 *          To set row status field for traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_rowstatus         -- The row status of the control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlRowStatus(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
     /* Local Variable Declaration
     */
    UI32_T      res;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        local_entry;
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    if ((ctl_entry_p->trace_route_ctl_rowstatus < VAL_traceRouteCtlRowStatus_active) ||
        (ctl_entry_p->trace_route_ctl_rowstatus > VAL_traceRouteCtlRowStatus_destroy))
        return TRACEROUTE_TYPE_INVALID_ARG;

    //memset(&local_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    memcpy(&local_entry, ctl_entry_p, sizeof(local_entry));

    if ((res = TRACEROUTE_OM_IsTraceRouteControlEntryExist(local_entry.trace_route_ctl_owner_index, local_entry.trace_route_ctl_owner_index_len,
                        local_entry.trace_route_ctl_test_name, local_entry.trace_route_ctl_test_name_len)) != TRACEROUTE_TYPE_OK)
    {
        /* this switch only initialize control entry */
        switch (ctl_entry_p->trace_route_ctl_rowstatus)
        {
            case VAL_traceRouteCtlRowStatus_createAndWait:
            case VAL_traceRouteCtlRowStatus_createAndGo:
                /* Init default value to control entry
                 */
                TRACEROUTE_MGR_InitControlEntry(&local_entry);
                local_entry.trace_route_ctl_rowstatus = ctl_entry_p->trace_route_ctl_rowstatus;

                if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(local_entry)) != TRACEROUTE_TYPE_OK)
                    return TRACEROUTE_TYPE_FAIL;

                break;
            default:
                res = TRACEROUTE_TYPE_FAIL;
                break;
        } /* end of switch */
    }
    else
    {
        if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&local_entry)) != TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

        local_entry.trace_route_ctl_rowstatus = ctl_entry_p->trace_route_ctl_rowstatus;
        res = TRACEROUTE_OM_SetTraceRouteCtlEntry(local_entry);
    }

    return res;
} /* end of TRACEROUTE_MGR_SetTraceRouteCtlRowStatus() */

/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for traceroute control entry
 * INPUT:
 *          ctl_entry_p->trace_route_ctl_owner_index       -- The owner index of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_owner_index_len   -- The length of the owner index.
 *          ctl_entry_p->trace_route_ctl_test_name         -- The test name of the trace route operation.
 *          ctl_entry_p->trace_route_ctl_test_name_len     -- The length of the test name.
 *          ctl_entry_p->trace_route_ctl_target_address    -- The target address of the traceroute operation.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
UI32_T TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    /* Local Variable Declaration
     */
    UI32_T      res;
    //UI32_T  dest_ip_address = 0;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        local_entry;
    //UI8_T   byte_dest_ip[SYS_ADPT_IPV4_ADDR_LEN];
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;


    switch(ctl_entry_p->trace_route_ctl_target_address_type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            /* IPv4 address checking */
            if(!memcmp(ctl_entry_p->trace_route_ctl_target_address.addr, traceroute_ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN))
                return TRACEROUTE_TYPE_INVALID_ARG;

            if ( IP_LIB_IsTestingIp(ctl_entry_p->trace_route_ctl_target_address.addr)   || IP_LIB_IsLoopBackIp(ctl_entry_p->trace_route_ctl_target_address.addr) ||
                 IP_LIB_IsBroadcastIp(ctl_entry_p->trace_route_ctl_target_address.addr) || IP_LIB_IsMulticastIp(ctl_entry_p->trace_route_ctl_target_address.addr) )
            {
                return TRACEROUTE_TYPE_INVALID_ARG;
            }
            break;

#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            /* IPv6 address checking */
            if(!memcmp(ctl_entry_p->trace_route_ctl_target_address.addr, traceroute_ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN))
                return TRACEROUTE_TYPE_INVALID_ARG;
            break;
#endif
        default:
            DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid trace_route_ctl_target_address_type.");
            return TRACEROUTE_TYPE_INVALID_ARG;
    }

    //memset(&local_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    memcpy(&local_entry, ctl_entry_p, sizeof(local_entry));

    if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&local_entry)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;

    /* Can not change the target address when admin_status == enable. */
    if ( local_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
        return TRACEROUTE_TYPE_FAIL;

    res = TRACEROUTE_OM_SetTraceRouteCtlTargetAddress(ctl_entry_p);

    return res;

} /* end of TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress() */
#endif

UI32_T TRACEROUTE_MGR_SetCtlEntryByField(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p, TRACEROUTE_TYPE_CtlEntryField_T field)
{
    /* Local Variable Declaration
     */
    UI32_T      res;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        local_entry;
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    memset(&local_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    memcpy(&local_entry, ctl_entry_p, sizeof(local_entry));

    if (TRACEROUTE_TYPE_OK != (res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&local_entry)))
        return res;

    /* Can not change the target address when admin_status == enable. */
    if (local_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
        return TRACEROUTE_TYPE_FAIL;

    switch (field)
    {
        case TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE:
            local_entry.trace_route_ctl_target_address_type = ctl_entry_p->trace_route_ctl_target_address_type;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS:

            if(L_INET_ADDR_TYPE_IPV4 == ctl_entry_p->trace_route_ctl_target_address_type)
            {
                if(IP_LIB_IsTestingIp(ctl_entry_p->trace_route_ctl_target_address.addr)
                    || IP_LIB_IsBroadcastIp(ctl_entry_p->trace_route_ctl_target_address.addr)
                    || IP_LIB_IsMulticastIp(ctl_entry_p->trace_route_ctl_target_address.addr)
                    || IP_LIB_IsZeroNetwork(ctl_entry_p->trace_route_ctl_target_address.addr))
                {
                    return TRACEROUTE_TYPE_INVALID_ARG;
                }
                /* check subnet broadcast and network-id */
                {
                    NETCFG_TYPE_InetRifConfig_T rif;
                    memset(&rif, 0, sizeof(rif));

                    while (NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextRifConfig(&rif))
                    {
                        UI8_T bcast_addr[SYS_ADPT_IPV4_ADDR_LEN];
                        UI8_T network_id[SYS_ADPT_IPV4_ADDR_LEN];
                        UI8_T byte_mask[SYS_ADPT_IPV4_ADDR_LEN];

                        DUMP_INET_ADDR(rif.addr);
                        IP_LIB_CidrToMask(rif.addr.preflen, byte_mask);

                        memset(bcast_addr, 0, SYS_ADPT_IPV4_ADDR_LEN);
                        memset(network_id, 0, SYS_ADPT_IPV4_ADDR_LEN);

                        if (IP_LIB_OK == IP_LIB_GetSubnetBroadcastIp(rif.addr.addr, byte_mask, bcast_addr))
                        {
                            if (!memcmp(bcast_addr, ctl_entry_p->trace_route_ctl_target_address.addr, SYS_ADPT_IPV4_ADDR_LEN))
                                return TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_BROADCAST_ADDR;
                        }
                        if (IP_LIB_OK == IP_LIB_GetNetworkID(rif.addr.addr, byte_mask, network_id))
                        {
                            if (!memcmp(network_id, ctl_entry_p->trace_route_ctl_target_address.addr, SYS_ADPT_IPV4_ADDR_LEN))
                                return TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_NETWORK_ID;
                        }

                    }
                }
            }
            else if ((L_INET_ADDR_TYPE_IPV6 == ctl_entry_p->trace_route_ctl_target_address_type) || (L_INET_ADDR_TYPE_IPV6Z == ctl_entry_p->trace_route_ctl_target_address_type))
            {
                if(IP_LIB_IsIPv6Multicast(ctl_entry_p->trace_route_ctl_target_address.addr))
                {
                    return TRACEROUTE_TYPE_INVALID_ARG;
                }

                if(L_INET_ADDR_TYPE_IPV6Z == ctl_entry_p->trace_route_ctl_target_address_type)
                {
                    if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_ZONE_ID(ctl_entry_p->trace_route_ctl_target_address.zoneid))
                    {
                        return TRACEROUTE_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID;
                    }
                }
            }
            memcpy(&local_entry.trace_route_ctl_target_address,
            &ctl_entry_p->trace_route_ctl_target_address,
            sizeof(local_entry.trace_route_ctl_target_address));
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_DATA_SIZE:
            local_entry.trace_route_ctl_data_size = ctl_entry_p->trace_route_ctl_data_size;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_TIMEOUT:
            local_entry.trace_route_ctl_timeout = ctl_entry_p->trace_route_ctl_timeout;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_PROBES_PER_HOP:
            local_entry.trace_route_ctl_probes_per_hop = ctl_entry_p->trace_route_ctl_probes_per_hop;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_PORT:
            local_entry.trace_route_ctl_probes_per_hop = ctl_entry_p->trace_route_ctl_probes_per_hop;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_TTL:
            local_entry.trace_route_ctl_max_ttl = ctl_entry_p->trace_route_ctl_max_ttl;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_DS_FIELD:
            local_entry.trace_route_ctl_ds_field = ctl_entry_p->trace_route_ctl_ds_field;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS_TYPE:
            local_entry.trace_route_ctl_source_address_type = ctl_entry_p->trace_route_ctl_source_address_type;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_UTE_CTL_SOURCE_ADDRESS:
            local_entry.trace_route_ctl_source_address = ctl_entry_p->trace_route_ctl_source_address;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_IF_INDEX:
            local_entry.trace_route_ctl_if_index = ctl_entry_p->trace_route_ctl_if_index;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_MISC_OPTIONS:
            local_entry.trace_route_ctl_misc_options_len = ctl_entry_p->trace_route_ctl_misc_options_len;
            memcpy(local_entry.trace_route_ctl_misc_options, ctl_entry_p->trace_route_ctl_misc_options, ctl_entry_p->trace_route_ctl_misc_options_len);
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_MISC_OPTIONS_LEN:
            local_entry.trace_route_ctl_misc_options_len = ctl_entry_p->trace_route_ctl_misc_options_len;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_FAILURES:
            local_entry.trace_route_ctl_max_failures = ctl_entry_p->trace_route_ctl_max_failures;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_DONT_FRAGMENT:
            local_entry.trace_route_ctl_dont_fragment = ctl_entry_p->trace_route_ctl_dont_fragment;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_INITIAL_TTL:
            local_entry.trace_route_ctl_initial_ttl = ctl_entry_p->trace_route_ctl_initial_ttl;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_FREQUENCY:
            local_entry.trace_route_ctl_frequency = ctl_entry_p->trace_route_ctl_frequency;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_STORAGE_TYPE:
            local_entry.trace_route_ctl_storage_type = ctl_entry_p->trace_route_ctl_storage_type;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_ADMIN_STATUS:
            local_entry.trace_route_ctl_admin_status = ctl_entry_p->trace_route_ctl_admin_status;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_ROWS:
            local_entry.trace_route_ctl_max_rows = ctl_entry_p->trace_route_ctl_max_rows;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_TRAP_GENERATION:
            local_entry.trace_route_ctl_probes_per_hop = ctl_entry_p->trace_route_ctl_probes_per_hop;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_DESCR:
            local_entry.trace_route_ctl_descr_len = ctl_entry_p->trace_route_ctl_descr_len;
            memcpy(local_entry.trace_route_ctl_descr, ctl_entry_p->trace_route_ctl_descr, ctl_entry_p->trace_route_ctl_descr_len);
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_DESCR_LEN:
            local_entry.trace_route_ctl_descr_len = ctl_entry_p->trace_route_ctl_descr_len;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_CREATE_HOPS_ENTRIES:
            local_entry.trace_route_ctl_create_hops_entries = ctl_entry_p->trace_route_ctl_create_hops_entries;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_TYPE:
            local_entry.trace_route_ctl_type = ctl_entry_p->trace_route_ctl_type;
            break;
        case TRACEROUTE_TYPE_CTLENTRYFIELD_ROWSTATUS:
            /* this switch only initialize control entry */
            switch (ctl_entry_p->trace_route_ctl_rowstatus)
            {
                case VAL_traceRouteCtlRowStatus_createAndWait:
                case VAL_traceRouteCtlRowStatus_createAndGo:
                    /* Init default value to control entry
                     */
                    TRACEROUTE_MGR_InitControlEntry(&local_entry);
                    local_entry.trace_route_ctl_rowstatus = ctl_entry_p->trace_route_ctl_rowstatus;
                    break;
                default:
                    local_entry.trace_route_ctl_rowstatus = ctl_entry_p->trace_route_ctl_rowstatus;
                    break;
            }
            break;
        default:
            if(TRUE == traceroute_backdoor_debug)
            {
                BACKDOOR_MGR_Printf("sorry, not support to change this field yet.\n");
            }
            return TRACEROUTE_TYPE_FAIL;
    }
    if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(local_entry)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;

    TRACEROUTE_MGR_CheckToStartTimer();

    return res;

}

/* FUNCTION NAME: TRACEROUTE_MGR_SetTraceRouteCtlSourceAddress
 * PURPOSE:
 *          To set row status field for traceroute control entry
 * INPUT:
 *          owner_index - Task name
 *          owner_index_len - The length of the task name
 *          test_name - Individual test name within local scope of each task
 *          test_name_len - The length of the test name
 *          source_addr - Use the specified IP address as the source address in outgoing probe packets
 *          source_addr_len - The length of the destination IP string
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 */
/*UI32_T  TRACEROUTE_MGR_SetTraceRouteCtlSourceAddress(UI8_T  *owner_index,
                                                            UI32_T owner_index_len,
                                                            UI8_T  *test_name,
                                                            UI32_T test_name_len,
                                                            UI8_T  *source_addr,
                                                            UI32_T source_addr_len)
{*/
    /* Local Variable Declaration
     */

    /* BODY
     */


/*}*/    /* end of TRACEROUTE_MGR_SetTraceRouteCtlSourceAddress() */

/* FUNCTION NAME : TRACEROUTE_MGR_CreateWorkSpace
 * PURPOSE:
 *      Create workspace for routing path from src to dst.
 *
 * INPUT:
 *      dst_ip : probe target IP address.
 *      src_ip : interface IP address which send out probe packet.
 *      workspace_index: Location of this entry
 * OUTPUT:
 *      None.
 * RETURN:
 *      TRACEROUTE_TYPE_OK  -- successfully create the workspace.
 *      TRACEROUTE_TYPE_INVALID_ARG -- src_ip or dst_ip is invalid value.
 *      TRACEROUTE_TYPE_NO_MORE_WORKSPACE
 *      TRACEROUTE_TYPE_NO_MORE_SOCKET
 *      TRACEROUTE_TYPE_NO_MORE_ENTRY
 *      TRACEROUTE_TYPE_FAIL
 * NOTES:
 *      1. If same task create workspace twice, cause previous one auto-free.
 */
UI32_T TRACEROUTE_MGR_CreateWorkSpace(UI32_T workspace_index, TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{

    UI32_T tid = SYSFUN_TaskIdSelf();
#if (SYS_CPNT_IPV6 == TRUE)
    UI32_T ifindex;
#endif
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    if (trace_route_workspace[workspace_index].owner_tid != 0)
        return TRACEROUTE_TYPE_FAIL;

    /* Init workspace
     */
    memset(&trace_route_workspace[workspace_index].outpacket, 0, sizeof(struct opacket));
    memset(&trace_route_workspace[workspace_index].whereto, 0,sizeof(struct sockaddr_in));
    //memset(&trace_route_workspace[workspace_index].wherefrom,0,sizeof(struct sockaddr_in));

#if (SYS_CPNT_IPV6 == TRUE)
    memset(&trace_route_workspace[workspace_index].outpacket6, 0, sizeof(struct opacket6));
    memset(&trace_route_workspace[workspace_index].whereto6, 0,sizeof(struct sockaddr_in6));
#endif
    trace_route_workspace[workspace_index].owner_tid  = (tid + workspace_index);
    switch(ctl_entry_p->trace_route_ctl_target_address_type)
    {
        // maybe we should use  ... VAL_pingCtlSourceAddressType_ipv4
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            trace_route_workspace[workspace_index].target_addr_type = ctl_entry_p->trace_route_ctl_target_address_type;
            memcpy(&trace_route_workspace[workspace_index].src_ip, &ctl_entry_p->trace_route_ctl_source_address, sizeof(L_INET_AddrIp_T));
            memcpy(&trace_route_workspace[workspace_index].dst_ip, &ctl_entry_p->trace_route_ctl_target_address, sizeof(L_INET_AddrIp_T));

            /* socket_in */
            trace_route_workspace[workspace_index].whereto.sin_family = AF_INET;
            memcpy((UI8_T *) &(trace_route_workspace[workspace_index].whereto.sin_addr.s_addr), ctl_entry_p->trace_route_ctl_target_address.addr, SYS_ADPT_IPV4_ADDR_LEN);
            if (traceroute_receive_socket < 0)
            {
                if ((traceroute_receive_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
                {
                    DEBUG_PRINTF("Create v4 rcv sock failed.\r\n");
                    return TRACEROUTE_TYPE_NO_MORE_SOCKET;
                }
                DEBUG_PRINTF("Create v4 rcv sock %ld succeed.\r\n", (long)traceroute_receive_socket);
            }
            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            trace_route_workspace[workspace_index].target_addr_type = ctl_entry_p->trace_route_ctl_target_address_type;
            memcpy(&trace_route_workspace[workspace_index].src_ip, &ctl_entry_p->trace_route_ctl_source_address, sizeof(L_INET_AddrIp_T));
            memcpy(&trace_route_workspace[workspace_index].dst_ip, &ctl_entry_p->trace_route_ctl_target_address, sizeof(L_INET_AddrIp_T));

            trace_route_workspace[workspace_index].whereto6.sin6_family = AF_INET6;
            memcpy((UI8_T *) &(trace_route_workspace[workspace_index].whereto6.sin6_addr.s6_addr), ctl_entry_p->trace_route_ctl_target_address.addr, SYS_ADPT_IPV6_ADDR_LEN);

            /* kernel's scope_id uses ifindex */
            ifindex = L_INET_ZONE_ID_TO_IFINDEX(ctl_entry_p->trace_route_ctl_target_address.zoneid);
            trace_route_workspace[workspace_index].whereto6.sin6_scope_id = ifindex;
            if (traceroute6_receive_socket < 0)
            {
                if ((traceroute6_receive_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
                {
                    DEBUG_PRINTF("Create v6 rcv sock failed.\r\n");
                    return TRACEROUTE_TYPE_NO_MORE_SOCKET;
                }
                DEBUG_PRINTF("Create v6 rcv sock %ld succeed.\r\n", (long)traceroute6_receive_socket);
            }
            break;
#endif
        default:
            DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid trace_route_ctl_target_address_type");
            return TRACEROUTE_TYPE_INVALID_ARG;
    }

    trace_route_workspace[workspace_index].probe_port = SYS_DFLT_TRACEROUTE_CTL_PORT;
    trace_route_workspace[workspace_index].ident      = 0x8000 | ((tid+workspace_index) & 0xffff);

//    trace_route_workspace[workspace_index].wherefrom.sin_family = AF_INET;
    // peter, trace_route_workspace[workspace_index].wherefrom.sin_len = sizeof(struct sockaddr_in);
    // peter, trace_route_workspace[workspace_index].wherefrom.sin_addr = src_ip;
//    memcpy((UI8_T *) &(trace_route_workspace[workspace_index].wherefrom.sin_addr.s_addr), src_ip, SYS_ADPT_IPV4_ADDR_LEN);

    //trace_route_workspace[workspace_index].whereto.sin_family = AF_INET;
    // peter, trace_route_workspace[workspace_index].whereto.sin_addr = dst_ip;
    //memcpy((UI8_T *) &(trace_route_workspace[workspace_index].whereto.sin_addr.s_addr), ctl_entry_p->trace_route_ctl_target_address.addr, SYS_ADPT_IPV4_ADDR_LEN);

    trace_route_workspace[workspace_index].datalen = sizeof(struct opacket);
    trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
    trace_route_workspace[workspace_index].max_hop_failures = ctl_entry_p->trace_route_ctl_max_failures;
    trace_route_workspace[workspace_index].hop_failures = 0;

    return TRACEROUTE_TYPE_OK;
} /* end of TRACEROUTE_MGR_CreateWorkSpace() */


/* FUNCTION NAME : TRACEROUTE_MGR_FreeWorkSpace
 * PURPOSE:
 *      Release working space to trace route utility.
 *
 * INPUT:
 *      workspace_index - the starting address of workspace handler.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_OK - the space is return to utility.
 *      TRACEROUTE_TYPE_INVALID_WORK_SPACE - the pointer is no valid pointer of working space,
 *                                     maybe not owned by this task.
 *
 * NOTES:
 *      1. After free workspace, handler will set to NULL.
 */
UI32_T  TRACEROUTE_MGR_FreeWorkSpace (UI32_T  workspace_index)
{
    int active_index;
    UI32_T traceroute_receive_socket_used = 0;
#if(SYS_CPNT_IPV6 == TRUE)
    UI32_T traceroute6_receive_socket_used = 0;
#endif
    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return TRACEROUTE_TYPE_FAIL;

    memset(&trace_route_workspace[workspace_index].outpacket, 0, sizeof(struct opacket));
    memset(&trace_route_workspace[workspace_index].whereto,0,sizeof(struct sockaddr_in));
    //memset(&trace_route_workspace[workspace_index].wherefrom,0,sizeof(struct sockaddr_in));
#if (SYS_CPNT_IPV6 == TRUE)
    memset(&trace_route_workspace[workspace_index].outpacket6, 0, sizeof(struct opacket6));
    memset(&trace_route_workspace[workspace_index].whereto6,0,sizeof(struct sockaddr_in6));

#endif
    trace_route_workspace[workspace_index].owner_tid  = 0;

    trace_route_workspace[workspace_index].target_addr_type = L_INET_ADDR_TYPE_UNKNOWN;
    //memset(trace_route_workspace[workspace_index].src_ip, 0, sizeof(L_INET_AddrIp_T));
    //memset(trace_route_workspace[workspace_index].dst_ip, 0, sizeof(L_INET_AddrIp_T));

    trace_route_workspace[workspace_index].probe_port = 0;
    trace_route_workspace[workspace_index].ident      = 0;
    trace_route_workspace[workspace_index].datalen    = 0;

    trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
    trace_route_workspace[workspace_index].max_hop_failures = 0;
    trace_route_workspace[workspace_index].hop_failures = 0;
    trace_route_workspace[workspace_index].last_transmit_time = 0;
    trace_route_workspace[workspace_index].last_receive_time = 0;
    trace_route_workspace[workspace_index].last_access_time = 0;

    /* If there is no more workspace use the v4/v6 receive sockets,
       we should close them to avoid receiving trash data.
     */
    for (active_index = 0; active_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; active_index++)
    {
        if (trace_route_workspace[active_index].admin_status != VAL_traceRouteCtlAdminStatus_enabled)
            continue;
        switch (trace_route_workspace[active_index].target_addr_type)
        {
            case L_INET_ADDR_TYPE_IPV4:
            case L_INET_ADDR_TYPE_IPV4Z:
                traceroute_receive_socket_used++;
                break;

#if(SYS_CPNT_IPV6 == TRUE)
            case L_INET_ADDR_TYPE_IPV6:
            case L_INET_ADDR_TYPE_IPV6Z:

                traceroute6_receive_socket_used++;
                break;
#endif

            default:
                DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid target_addr_type.");
                break;
        }
    }
    if(traceroute_receive_socket_used == 0)
    {
        if(traceroute_receive_socket >= 0)
        {
            close(traceroute_receive_socket);
            DEBUG_PRINTF("Closed v4 rcv sock: %ld\r\n", (long)traceroute_receive_socket);
            traceroute_receive_socket = -1;
        }
    }
#if (SYS_CPNT_IPV6 == TRUE)
    if(traceroute6_receive_socket_used == 0)
    {
        if(traceroute6_receive_socket >= 0)
        {
            close(traceroute6_receive_socket);
            DEBUG_PRINTF("Closed v6 rcv sock: %ld\r\n", (long)traceroute6_receive_socket);
            traceroute6_receive_socket = -1;
        }
    }
#endif
    return TRACEROUTE_TYPE_OK;

}

/* FUNCTION NAME: TRACEROUTE_MGR_SetWorkSpaceAdminStatus
 * PURPOSE:
 *          Sync admin status in the work space.
 * INPUT:
 *          table_index - table index to be synchronize.
 *          status - {VAL_traceRouteCtlAdminStatus_disabled|VAL_traceRouteCtlAdminStatus_enabled}.
 * OUTPUT:
 *          None
 * RETURN:
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:
 *
 */
UI32_T TRACEROUTE_MGR_SetWorkSpaceAdminStatus(UI32_T table_index, UI32_T status)
{
    /* BODY
     */
    UI32_T res;

    switch(status)
    {
        case VAL_traceRouteCtlAdminStatus_disabled:
            trace_route_workspace[table_index].admin_status = VAL_traceRouteCtlAdminStatus_disabled;
            res = TRACEROUTE_TYPE_OK;
            break;

        case VAL_traceRouteCtlAdminStatus_enabled:
            trace_route_workspace[table_index].admin_status = VAL_traceRouteCtlAdminStatus_enabled;
            res = TRACEROUTE_TYPE_OK;
            break;
        default:
            res = TRACEROUTE_TYPE_FAIL;
    }
    return res;

} /* end of TRACEROUTE_MGR_SetWorkSpaceAdminStatus() */

#if 0
/* LOCAL SUBPROGRAM IMPLEMENTATION
 */

/* FUNCTION NAME : TRACEROUTE_RefreshOM
 * PURPOSE:
 *      Destroy OM entry by timer.
 *
 * INPUT:
 *      fds
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_OK  \
 *      TRACEROUTE_TYPE_DO_NOT_SEND \
 *      TRACEROUTE_TYPE_FAIL
 *
 * NOTES:
 *      None
 */
static void   TRACEROUTE_MGR_RefreshOM(void)
{
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    UI32_T      active_index = 0;
    UI32_T      current_time, res;
    BOOL_T      get_first_entry=FALSE;


    current_time = SYSFUN_GetSysTick();

    while (1)
    {
        if (get_first_entry)
        {
            if ((res = TRACEROUTE_OM_GetFirstActiveResultsEntry(&active_index,
                        &global_result_entry)) != TRACEROUTE_TYPE_OK)
                break;
            get_first_entry = FALSE;
        }
        else
        {
            if ((res = TRACEROUTE_OM_GetNextActiveResultsEntry(&active_index,
                    &global_result_entry)) != TRACEROUTE_TYPE_OK)
                break;
        }

        if ((current_time - trace_route_workspace[active_index].last_access_time )
               >= TRACEROUTE_MGR_REFRESH_TIMER)
        {
            /* Remove entry.
             */
            if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(active_index, &list_elm)) !=  TRACEROUTE_TYPE_OK)
                break;

            memcpy(global_control_entry.trace_route_ctl_owner_index, list_elm.owner_index, list_elm.owner_index_len);
            memcpy(global_control_entry.trace_route_ctl_test_name, list_elm.test_name, list_elm.test_name_len);
            global_control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
            global_control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

            if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&global_control_entry)) != TRACEROUTE_TYPE_OK)
                break;

            global_control_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;

            if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(global_control_entry)) != TRACEROUTE_TYPE_OK)
                break;
        } /* end of if */

    } /* end of while */
    return;
} /* end of TRACEROUTE_RefreshOM() */
#endif

/* FUNCTION NAME : TRACEROUTE_MGR_SendProbe
 * PURPOSE:
 *      Send a UDP pkt to peers which will return an ICMP pkt.
 *
 * INPUT:
 *      fds
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_OK  \
 *      TRACEROUTE_TYPE_DO_NOT_SEND \
 *      TRACEROUTE_TYPE_FAIL
 *
 * NOTES:
 *      None
 */
static UI32_T TRACEROUTE_MGR_SendProbe(void)
{
    struct      opacket     *op;
    struct      sockaddr_in from;
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    int         send_length = 0;
    UI32_T      active_index = 0;
    UI32_T      res, current_time, ttl;
    UI32_T      hop, prob;
    UI16_T      seq = 0;
    int         socket_rc;
    TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T   prob_history_entry;

#if (SYS_CPNT_IPV6 == TRUE)
    struct      opacket6     *op6;
    struct      sockaddr_in6 from6;
#endif

    memset(&prob_history_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    memset(&global_result_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    memset(&global_control_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    if ((res = TRACEROUTE_OM_GetFirstActiveResultsEntry(&active_index, &global_result_entry)) != TRACEROUTE_TYPE_OK)
    {
        return TRACEROUTE_TYPE_NO_MORE_PROBE_TO_SEND;
    }

    do
    {
        current_time = SYSFUN_GetSysTick();

        DEBUG_PRINTF("%d, active index: %d\n", __LINE__, (int)active_index);
        /* Set prob history entry.
         */
        hop = global_result_entry.trace_route_results_cur_hop_count;
        prob = global_result_entry.trace_route_results_cur_probe_count;
        seq =  ((hop-1) * 3 + prob);

        /* 1 Perform checks for timing issue
         */
        if (trace_route_workspace[active_index].last_transmit_time != 0)
        {
            if ((current_time - trace_route_workspace[active_index].last_transmit_time >=
                     SYS_DFLT_TRACEROUTE_CTL_TIME_OUT))
            {
                trace_route_workspace[active_index].last_transmit_time = 0;
                trace_route_workspace[active_index].last_receive_time = 0;

                if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(active_index, &list_elm)) !=  TRACEROUTE_TYPE_OK)
                    break;
                memcpy(prob_history_entry.trace_route_ctl_owner_index,list_elm.owner_index,list_elm.owner_index_len);
                prob_history_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
                memcpy(prob_history_entry.trace_route_ctl_test_name,list_elm.test_name,list_elm.test_name_len);
                prob_history_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;
                prob_history_entry.trace_route_probe_history_index = seq;
                prob_history_entry.trace_route_probe_history_hop_index = hop;
                prob_history_entry.trace_route_probe_history_probe_index = prob;
                prob_history_entry.trace_route_probe_history_response = SYS_DFLT_TRACEROUTE_CTL_TIME_OUT;
                prob_history_entry.trace_route_probe_history_status = VAL_traceRouteProbeHistoryStatus_requestTimedOut;
                prob_history_entry.trace_route_probe_history_last_rc = TRACEROUTE_TYPE_NO_RESPONSE;
                SYS_TIME_GetDayAndTime(prob_history_entry.trace_route_probe_history_time,
                                      &prob_history_entry.trace_route_probe_history_time_len);

                if ((res = TRACEROUTE_OM_AppendProbePacketResult(active_index,
                               prob_history_entry)) !=  TRACEROUTE_TYPE_OK)
                    break;
                trace_route_workspace[active_index].unreceived_probs_per_hop++;

                if (trace_route_workspace[active_index].unreceived_probs_per_hop >= SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROBE_PER_HOP)
                {
                    trace_route_workspace[active_index].hop_failures++;
                    trace_route_workspace[active_index].unreceived_probs_per_hop = 0;
                }
                if ((trace_route_workspace[active_index].max_hop_failures != 0)&&(trace_route_workspace[active_index].hop_failures >= trace_route_workspace[active_index].max_hop_failures)) /* rfc 2925, 0 means don't termimate */
                {
                    if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(active_index, &list_elm)) !=  TRACEROUTE_TYPE_OK)
                        break;

                    memcpy(global_control_entry.trace_route_ctl_owner_index, list_elm.owner_index, list_elm.owner_index_len);
                    memcpy(global_control_entry.trace_route_ctl_test_name, list_elm.test_name, list_elm.test_name_len);
                    global_control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
                    global_control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

                    if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&global_control_entry)) != TRACEROUTE_TYPE_OK)
                        break;

                    global_control_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;

                    if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(global_control_entry)) != TRACEROUTE_TYPE_OK)
                        return TRACEROUTE_TYPE_FAIL;

                    continue;
                } /* end of if */
            }
            else
            {
                /* Get next entry because timer hasn't expires for this one.
                 */
                continue;
            }
        } /* end of if */

        /* 2  Send prob packet
         */
        if (++global_result_entry.trace_route_results_cur_probe_count >
             SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP)
        {
            global_result_entry.trace_route_results_cur_probe_count = 1;
            global_result_entry.trace_route_results_cur_hop_count++;
        }

        seq += TRACEROUTE_MGR_DESTUDPPORT;
        ttl = global_result_entry.trace_route_results_cur_hop_count;


        /* Send using socket
         */

        switch(trace_route_workspace[active_index].target_addr_type)
        {
            case L_INET_ADDR_TYPE_IPV4:
            case L_INET_ADDR_TYPE_IPV4Z:
                /* Fill in outpacket content
                 */
                op   = &(trace_route_workspace[active_index].outpacket);
                op->packet_seq  = seq;
                op->packet_ttl  = ttl;
                op->packet_time = current_time;

                memset(&from, 0, sizeof(from));
                from.sin_family = AF_INET;
                from.sin_port = L_STDLIB_Hton16(trace_route_workspace[active_index].ident);
                trace_route_workspace[active_index].whereto.sin_port = L_STDLIB_Hton16(seq);

                if ((traceroute_send_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
                {
                    traceroute_send_socket = -1;
                    return TRACEROUTE_TYPE_NO_MORE_SOCKET;
                }
                DEBUG_PRINTF("v4 send sock %ld created.\n", (long)traceroute_send_socket);

                /* set TTL */
                setsockopt (traceroute_send_socket,
                            IPPROTO_IP, IP_TTL, (char *)&ttl, sizeof(ttl));

                DEBUG_PRINTF("binding src port: %d\n", L_STDLIB_Ntoh16(from.sin_port));

                /* bind src port */
                if ((socket_rc = bind (traceroute_send_socket, (struct sockaddr *)&from,
                        sizeof(struct sockaddr_in))) < 0)
                {
                    DEBUG_PRINTF("Bind socket Fails on active index %d, rc=%s\n", (int)active_index, strerror(errno));
                    break;
                }

                send_length = sendto (traceroute_send_socket,
                              (UI8_T *)&op, trace_route_workspace[active_index].datalen, 0,
                              (struct sockaddr*)&(trace_route_workspace[active_index].whereto),
                              sizeof(trace_route_workspace[active_index].whereto));
                DEBUG_PRINTF("send traceroute v4 pkt, size:%d\n", send_length);
                break;

#if (SYS_CPNT_IPV6 == TRUE)
            case L_INET_ADDR_TYPE_IPV6:
            case L_INET_ADDR_TYPE_IPV6Z:
                /* Fill in outpacket content
                 */
                op6   = &(trace_route_workspace[active_index].outpacket6);
                op6->packet_seq  = seq;
                op6->packet_ttl  = ttl;
                op6->packet_time = current_time;

                memset(&from6, 0, sizeof(from6));
                from6.sin6_family = AF_INET6;
                from6.sin6_port = L_STDLIB_Hton16(trace_route_workspace[active_index].ident);

                trace_route_workspace[active_index].whereto6.sin6_port = L_STDLIB_Hton16(seq);

                if ((traceroute6_send_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
                {
                    traceroute6_send_socket = -1;
                    return TRACEROUTE_TYPE_NO_MORE_SOCKET;
                }

                DEBUG_PRINTF("v6 send sock %ld created.\n", (long)traceroute6_send_socket);
                /* set TTL */
                if(-1 == setsockopt(traceroute6_send_socket,
                            IPPROTO_IPV6, IPV6_UNICAST_HOPS, (char *)&ttl, sizeof(ttl)))
                {
                    DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "setsockopt failed");
                    break;
                }

                /* bind src port */
                DEBUG_PRINTF("binding v6 src port: %d\n", L_STDLIB_Ntoh16(from6.sin6_port));
                if ((socket_rc = bind (traceroute6_send_socket,(struct sockaddr *)&from6, sizeof(struct sockaddr_in6))) < 0)
                {
                    DEBUG_PRINTF("%d, Bind socket Fails on active index %d, rc=%s\n", __LINE__, (int)active_index, strerror(errno));
                    break;
                }

                send_length = sendto (traceroute6_send_socket,
                              (UI8_T *)&op6, trace_route_workspace[active_index].datalen, 0,
                              (struct sockaddr*)&(trace_route_workspace[active_index].whereto6),
                              sizeof(trace_route_workspace[active_index].whereto6));

                DEBUG_PRINTF("send traceroute v6 pkt, size:%d\n", send_length);
                break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

            default:
                DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid target_addr_type.");
                break;

        }

        if (send_length < 0)
            DEBUG_PRINTF("send traceroute v6 pkt, send_len < 0, error.\n");

        if (traceroute_send_socket >= 0)
        {
            close(traceroute_send_socket);
            traceroute_send_socket = -1;
        }

#if (SYS_CPNT_IPV6 == TRUE)
        if (traceroute6_send_socket >= 0)
        {
            close(traceroute6_send_socket);
            traceroute6_send_socket = -1;
        }
#endif
        trace_route_workspace[active_index].last_transmit_time = SYSFUN_GetSysTick();

        global_result_entry.trace_route_results_test_attempts++;

        if ((res = TRACEROUTE_OM_SetTraceRouteResultsEntry(active_index,
                       global_result_entry)) !=  TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

    } while((res = TRACEROUTE_OM_GetNextActiveResultsEntry(&active_index, &global_result_entry)) == TRACEROUTE_TYPE_OK);
    /* end of while */
    return TRACEROUTE_TYPE_OK;

}

/* FUNCTION NAME : TRACEROUTE_MGR_WaitForReply
 * PURPOSE:
 *      Wait for an ICMP pkt.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_OK       -- receive packet.
 *      TRACEROUTE_TYPE_FAIL     -- can't receive
 *      TRACEROUTE_TYPE_TIMEOUT  -- exceed max waiting time.
 *
 * NOTES:
 *      None
 */
static UI32_T TRACEROUTE_MGR_WaitForReply(void)
{
    struct      timeval wait;
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    int         cc = 0;
    fd_set      fds;
    UI32_T      active_index = 0, res = 0, sport = 0, dport = 0;
    UI8_T       icmp_pkt_type = 0, icmp_pkt_code = 0;
    UI8_T       packet[SYS_DFLT_UDP_MAX_LEN];
    BOOL_T      active_index_found = FALSE;
    struct iphdr *hip;
    struct udphdr *udp;
    struct icmphdr *icp;
    I32_T       hlen;

    struct sockaddr_storage from; /* for both sockaddr_in & sockaddr_in6 */
    socklen_t fromlen = (socklen_t)sizeof(struct sockaddr_storage);

#if (SYS_CPNT_IPV6 == TRUE)
    ipv6_packet_T *ipv6;
    ICMP6_T *icmp6;
#endif

    L_INET_AddrIp_T inet_addr;
    UI8_T tmp_buf[MAX_BUF_LEN] = {};
    I32_T max_sockfd = -1;
    UI32_T systick_start = 0,systick_current = 0;

    memset(&global_result_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    memset(&global_control_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));

    systick_start = SYSFUN_GetSysTick();

    wait.tv_sec = 0;
    wait.tv_usec = TRACEROUTE_MGR_TIMEOUT_MICROSECOND;

    while (wait.tv_usec > 0)
    {
    FD_ZERO(&fds);
        DEBUG_PRINTF("traceroute_receive_socket: %ld\n", (long)traceroute_receive_socket);

#if (SYS_CPNT_IPV6 == TRUE)
        DEBUG_PRINTF("traceroute6_receive_socket: %ld\n", (long)traceroute6_receive_socket);
#endif
        /* -1 may result in undefined behavior for FD macros */
        if(traceroute_receive_socket >= 0)
        {
    FD_SET(traceroute_receive_socket, &fds);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        if(traceroute6_receive_socket >= 0)
        {
    FD_SET(traceroute6_receive_socket, &fds);
        }
#endif
#if (SYS_CPNT_IPV6 == TRUE)
        max_sockfd = (traceroute6_receive_socket > traceroute_receive_socket) ? traceroute6_receive_socket : traceroute_receive_socket;
#else
    max_sockfd = traceroute_receive_socket;
#endif

        systick_current = SYSFUN_GetSysTick();

        wait.tv_sec = 0;
        /* the remaining time */
        /* 1 tick = 10 ms = 10^4 microsecond */
        wait.tv_usec = TRACEROUTE_MGR_TIMEOUT_MICROSECOND - ((systick_current - systick_start) * 10000);

        DEBUG_PRINTF("%d, wait.tv_usec: %d, max_sockfd: %d\r\n", __LINE__, (int) wait.tv_usec, max_sockfd) ;
        if (select(max_sockfd + 1, (fd_set*)&fds, NULL, NULL, &wait) <= 0)
        {
            continue;
        }

        memset(&inet_addr, 0, sizeof(inet_addr));

        if ((traceroute_receive_socket >= 0) && FD_ISSET(traceroute_receive_socket, &fds))
        {
            if ((cc = recvfrom(traceroute_receive_socket,
                               packet, SYS_DFLT_UDP_MAX_LEN, 0,
                            (struct sockaddr *)&from,
                               &fromlen))<=0)
            {
                /* cc is the size of data
                 * re-calculate waiting_time and wait again
                 */

                /* need to test cc == 0 for SNMP, where size 0 is allowed. */
                continue;
            }
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if ((traceroute6_receive_socket >= 0) && FD_ISSET(traceroute6_receive_socket, &fds))
        {
            if ((cc = recvfrom(traceroute6_receive_socket,
                               packet, SYS_DFLT_UDP_MAX_LEN, 0,
                            (struct sockaddr *)&from,
                            &fromlen))<=0)
            {
                /* cc is the size of data
                 * re-calculate waiting_time and wait again
                 */

                /* need to test cc == 0 for SNMP, where size 0 is allowed. */
                continue;
            }

        }
#endif
        DEBUG_PRINTF("received pkt, ss_family: %d, size:%d\n", from.ss_family, cc);

        L_INET_SockaddrToInaddr((struct sockaddr *)&from, &inet_addr);
        if (traceroute_backdoor_debug == TRUE)
        {
            L_INET_InaddrToString((L_INET_Addr_T *)&inet_addr, (char *)tmp_buf, sizeof(tmp_buf));
            printf("address: %s\r\n", tmp_buf);

            DBG_DumpHex("", cc, (char *) packet);
        }

        /* 1. Validate receive packet information
         */

        DUMP_INET_ADDR(inet_addr);

        if ((inet_addr.type == L_INET_ADDR_TYPE_IPV4)
            || (inet_addr.type == L_INET_ADDR_TYPE_IPV4Z))
        {
            /******* v4 ******* */
            hip = (struct iphdr *)packet;
            hlen = hip->ihl << 2;

            DEBUG_PRINTF("%s, %d, hlen: %ld\n", __FUNCTION__, __LINE__, (long)hlen);

            if (hlen + sizeof(struct icmphdr) > cc || hip->protocol != IPPROTO_ICMP)
                continue;

            icp = (struct icmphdr *)((UI8_T *)hip + hlen);
            icmp_pkt_type = icp->type;
            icmp_pkt_code = icp->code;
            cc -= hlen + sizeof(struct icmphdr);

            DEBUG_PRINTF("%s, %d,icmp_pkt_type: %d, icmp_pkt_code: %d\n", __FUNCTION__, __LINE__, icmp_pkt_type, icmp_pkt_code);


            if ((icmp_pkt_type == ICMP_TIME_EXCEEDED && icmp_pkt_code == ICMP_EXC_TTL) ||
                (icmp_pkt_type == ICMP_DEST_UNREACH) || (icmp_pkt_type == ICMP_ECHO))
            {
                /* add this to filter the packet sent by linux kernel
                 *   when target host does not exist (i.e. no arp reply received)
                 */
                if (  (icmp_pkt_code == ICMP_HOST_UNREACH)
                    &&(hip->saddr == hip->daddr) /* saddr == daddr == our ip */
                   )
                {
                    DEBUG_PRINTF("%s, %d,icmp_pkt_type: %d, icmp_pkt_code: %d\n", __FUNCTION__, __LINE__, icmp_pkt_type, icmp_pkt_code);
                    return TRACEROUTE_TYPE_INVALID_ARG;
                }

                /*get the ip header. because icmp error pkt always contains IP header and
                  first 8 bytes of the IP datagram(the UDP datagram sent by user).
                 */
                hip = (struct iphdr *)(icp+1);

                /* header length only use 4-bit. and the length is "number of 32-bits words"
                   in IP header => 5*4bytes=20bytes
                 */
                hlen = hip->ihl << 2;

                udp = (struct udphdr *)((UI8_T *)hip + hlen);

                /* 12-> 8, 8 is udp header size */
                if ((hlen + 8 > cc) || (hip->protocol != IPPROTO_UDP))
                    continue;

                sport = L_STDLIB_Ntoh16(udp->source);
                dport = L_STDLIB_Ntoh16(udp->dest);

            }
            else
                return TRACEROUTE_TYPE_INVALID_ARG; //continue;

        } /* v4 */
#if(SYS_CPNT_IPV6 == TRUE)
        else
        {
            /******* v6 ******* */
            icmp6 =(ICMP6_T *) packet;

            icmp_pkt_type = icmp6->type;
            icmp_pkt_code = icmp6->code;
            //            cc -= hlen + sizeof(struct icmphdr);

            DEBUG_PRINTF("%s, %d,icmp_pkt_type: %d, icmp_pkt_code: %d\n", __FUNCTION__, __LINE__, icmp_pkt_type, icmp_pkt_code);

            if ((icmp_pkt_type == ICMPV6_TIME_EXCEED && icmp_pkt_code == ICMPV6_EXC_HOPLIMIT) ||
                (icmp_pkt_type == ICMPV6_DEST_UNREACH) || (icmp_pkt_type == ICMPV6_ECHO_REQUEST))
            {
                ipv6 = (ipv6_packet_T *)(icmp6+1);

                /* if the nextheader not udp, how to deal with it ? */
                if (ipv6->next_header != IPPROTO_UDP)
                    continue;

                /* add this to filter the packet sent by linux kernel
                 *   when target host does not exist (i.e. no arp reply received)
                 */
                if (  (icmp_pkt_type == ICMPV6_DEST_UNREACH)
                    &&(icmp_pkt_code == ICMPV6_ADDR_UNREACH)
                   )
                {
                    NETCFG_TYPE_InetRifConfig_T rif_config = {0};

                    memcpy (&rif_config.addr, &inet_addr, sizeof(rif_config.addr));

                    /* for ipv6, need to check
                     *   if this packet is sent from one of our interface
                     */
                    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetRifFromExactIp(
                                                &rif_config))
                    {
                        DEBUG_PRINTF("%s, %d,icmp_pkt_type: %d, icmp_pkt_code: %d\n", __FUNCTION__, __LINE__, icmp_pkt_type, icmp_pkt_code);
                        return TRACEROUTE_TYPE_INVALID_ARG;
                    }
                }

                udp = (struct udphdr *)((UI8_T *)ipv6 + 40);

                sport = L_STDLIB_Ntoh16(udp->source);
                dport = L_STDLIB_Ntoh16(udp->dest);
            }
            else
                return TRACEROUTE_TYPE_INVALID_ARG;
        }
#endif /* SYS_CPNT_IPV6 */
        DEBUG_PRINTF("%s, %d, sport: %ld, dport: %ld\n", __FUNCTION__, __LINE__, (long)sport, (long)dport);

        /* Sweep thru existing workspace to find which index the receive socket belongs.  If none
           matches then continue;
         */
        for (active_index = 0; active_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; active_index++)
        {
            if (trace_route_workspace[active_index].admin_status != VAL_traceRouteCtlAdminStatus_enabled)
                continue;

            if (sport == trace_route_workspace[active_index].ident)
            {
                switch (trace_route_workspace[active_index].target_addr_type)
                {
                    case L_INET_ADDR_TYPE_IPV4:
                    case L_INET_ADDR_TYPE_IPV4Z:
                        if (dport == L_STDLIB_Ntoh16(trace_route_workspace[active_index].whereto.sin_port))
                            active_index_found = TRUE;
                        break;

#if(SYS_CPNT_IPV6 == TRUE)
                    case L_INET_ADDR_TYPE_IPV6:
                    case L_INET_ADDR_TYPE_IPV6Z:
                        if (dport == L_STDLIB_Ntoh16(trace_route_workspace[active_index].whereto6.sin6_port))
                            active_index_found = TRUE;
                        break;
#endif /* #if(SYS_CPNT_IPV6 == TRUE) */

                    default:
                        DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid target_addr_type.");
                        break;
                }
            }
            if (active_index_found == TRUE)
                break;

        } /* end of for */

        if (!active_index_found)
            continue;

        DEBUG_PRINTF("%s, %d, matched index: %ld.\n", __FUNCTION__, __LINE__, (long)active_index);


        if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(active_index, &list_elm)) !=  TRACEROUTE_TYPE_OK)
            break;

        memcpy(global_control_entry.trace_route_ctl_owner_index, list_elm.owner_index, list_elm.owner_index_len);
        memcpy(global_control_entry.trace_route_ctl_test_name, list_elm.test_name, list_elm.test_name_len);
        global_control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
        global_control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

        if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&global_control_entry)) != TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

        memcpy(global_result_entry.trace_route_ctl_owner_index, list_elm.owner_index, list_elm.owner_index_len);
        memcpy(global_result_entry.trace_route_ctl_test_name, list_elm.test_name, list_elm.test_name_len);
        global_result_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
        global_result_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

        if ((res = TRACEROUTE_OM_GetTraceRouteResultsEntry(&global_result_entry)) != TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

        trace_route_workspace[active_index].rcv_pkt_len = cc;
        trace_route_workspace[active_index].last_receive_time = SYSFUN_GetSysTick();

        global_result_entry.trace_route_results_test_successes++;

        if ((res = TRACEROUTE_OM_SetTraceRouteResultsEntry(active_index,
                       global_result_entry)) != TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

        if ((res = TRACEROUTE_MGR_ResolveIcmpType(active_index,
            global_result_entry.trace_route_results_cur_hop_count,
            global_result_entry.trace_route_results_cur_probe_count,
                        icmp_pkt_type, icmp_pkt_code, (struct sockaddr *)&from)) !=TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;

        trace_route_workspace[active_index].last_transmit_time = 0;
        trace_route_workspace[active_index].last_receive_time = 0;

        /* If maximum failure event is reached, this traceroute session shall be automatically terminated.
         */
        if (trace_route_workspace[active_index].unreceived_probs_per_hop >= SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROBE_PER_HOP)
        {
            trace_route_workspace[active_index].hop_failures++;
            trace_route_workspace[active_index].unreceived_probs_per_hop = 0;
        }
        if ((trace_route_workspace[active_index].max_hop_failures != 0)&&(trace_route_workspace[active_index].hop_failures >= trace_route_workspace[active_index].max_hop_failures)) /* rfc 2925, 0 means don't termimate */
        {
            global_control_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;

            if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(global_control_entry)) != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
        }

        /* If Target IP is reached, this traceroute session shall be automatically terminated.
         */
        if (global_result_entry.trace_route_results_cur_probe_count == SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP)
        {
            DEBUG_PRINTF("%d, traceroute type: %ld", __LINE__, (long)trace_route_workspace[active_index].target_addr_type);
            switch(trace_route_workspace[active_index].target_addr_type)
            {
                case L_INET_ADDR_TYPE_IPV4 :
                case L_INET_ADDR_TYPE_IPV4Z :

                    if(memcmp(&(((struct sockaddr_in *)&from)->sin_addr.s_addr), trace_route_workspace[active_index].dst_ip.addr, SYS_ADPT_IPV4_ADDR_LEN))
                        return TRACEROUTE_TYPE_INVALID_ARG;
                    break;

#if(SYS_CPNT_IPV6 == TRUE)
                case L_INET_ADDR_TYPE_IPV6 :
                case L_INET_ADDR_TYPE_IPV6Z :
                    if(memcmp(((struct sockaddr_in6 *)&from)->sin6_addr.s6_addr, trace_route_workspace[active_index].dst_ip.addr, SYS_ADPT_IPV6_ADDR_LEN))
                        return TRACEROUTE_TYPE_INVALID_ARG;
                    break;
#endif /* #if(SYS_CPNT_IPV6 == TRUE) */

                default:
                    DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid target_addr_type.");
                    return TRACEROUTE_TYPE_INVALID_ARG;
            }


            /* When the last complete path is determined, get the day and time.*/
            SYS_TIME_GetDayAndTime(global_result_entry.trace_route_results_last_good_path,
                                  &global_result_entry.trace_route_results_last_good_path_len);

            if ((res = TRACEROUTE_OM_SetTraceRouteResultsEntry(active_index, global_result_entry)) != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;

            global_control_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;

            if ((res = TRACEROUTE_OM_SetTraceRouteCtlEntry(global_control_entry)) != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
        }
    } /* end of while */

    return TRACEROUTE_TYPE_OK;

}

/* FUNCTION NAME : TRACEROUTE_MGR_CheckToStartTimer
 * PURPOSE:
 *      Check whether it has active entry to start periodic timer
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void TRACEROUTE_MGR_CheckToStartTimer(void)
{
    UI32_T active_index;

    if (TRACEROUTE_OM_GetFirstActiveResultsEntry(&active_index, &global_result_entry) == TRACEROUTE_TYPE_OK)
    {
        TRACEROUTE_TASK_PeriodicTimerStart_Callback();
    }
}

/* FUNCTION NAME : TRACEROUTE_MGR_ResolveIcmpType
 * PURPOSE:
 *      Resolute received ICMP pkt.return it's type.
 *
 * INPUT:
 *      handler -- index of traceroute workspace.
 *      hop -- current hop count
 *      prob -- current prob count
 *      packet - receive packet content
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRACEROUTE_TYPE_FAIL           (unknown reason)
 *      TRACEROURE_TYPE_EXCEED_MAX_TTL (for time exceed)
 *      TRACEROUTE_TYPE_OK             (reach it)
 *      TRACEROUTE_TYPE_NETWORK_UNREACHABLE
 *      TRACEROUTE_TYPE_HOST_UNREACHABLE
 *      TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE
 *      TRACEROUTE_TYPE_FRAGMENTATION_NEEDED
 *      TRACEROURE_TYPE_EXCEED_MAX_TTL
 *
 * NOTES:
 *      1. the sockaddr is converted to sockaddr_in/sockaddr_in6 by the target_addr_type.
 */
static UI32_T TRACEROUTE_MGR_ResolveIcmpType(UI32_T workspace_index, UI32_T hop, UI32_T prob,
                                             UI8_T icmp_packet_type, UI8_T icmp_packet_code,
                                             struct sockaddr *from_p)
{
    /* Local Variable Declaration
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T       prob_history_entry;
    UI32_T      response_time, res;
    //UI32_T      receive_source_ip;
    UI16_T      history_rc  = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
    /* BODY
     */

    response_time = trace_route_workspace[workspace_index].last_receive_time -
                    trace_route_workspace[workspace_index].last_transmit_time;

    memset(&prob_history_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    //receive_source_ip = 0;

    switch(trace_route_workspace[workspace_index].target_addr_type)
    {
        case L_INET_ADDR_TYPE_IPV4 :
        case L_INET_ADDR_TYPE_IPV4Z :
            //receive_source_ip = (UI32_T)fromp->sin_addr.s_addr;
            if (ICMP_TIME_EXCEEDED == icmp_packet_type)
            {
                trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
                trace_route_workspace[workspace_index].hop_failures = 0;
                history_rc = TRACEROUTE_TYPE_EXCEED_MAX_TTL;
                break;
            }

            switch (icmp_packet_code)
            {
                case ICMP_NET_UNREACH :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
                    break;
                case ICMP_HOST_UNREACH :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_HOST_UNREACHABLE;
                    break;
                case ICMP_PROT_UNREACH :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE;
                    break;
                case ICMP_PORT_UNREACH :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
                    trace_route_workspace[workspace_index].hop_failures = 0;
                    //receive_source_ip = *(UI32_T *)trace_route_workspace[workspace_index].dst_ip;
                    //receive_source_ip = *(UI32_T *)trace_route_workspace[workspace_index].dst_ip;
                    //simon: how come sim_addr not the target but able to reply unreach port ??
                    //fromp->sin_addr.s_addr = *(in_addr_t *)trace_route_workspace[workspace_index].dst_ip;//so that trace can stop
                    history_rc = TRACEROUTE_TYPE_PORT_UNREACHABLE;
                    break;
                case ICMP_FRAG_NEEDED :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_FRAGMENTATION_NEEDED;
                    break;
                case ICMP_SR_FAILED :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_SRT_UNREACHABLE;
                    break;
                case ICMP_NET_UNKNOWN :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_NETWORK_UNKNOWN;
                    break;
                case ICMP_HOST_UNKNOWN :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_HOST_UNKNOWN;
                    break;
                default :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
                    break;
            } /* switch */
            break;

#if(SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6 : /* for ICMPv4/v6 msg mapping, please ref. Understanding IPv6. */
        case L_INET_ADDR_TYPE_IPV6Z :
            switch (icmp_packet_type)
            {
                case ICMPV6_DEST_UNREACH :
                    switch (icmp_packet_code)
                    {
                        case ICMPV6_PORT_UNREACH :
                            trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
                            trace_route_workspace[workspace_index].hop_failures = 0;
                            history_rc = TRACEROUTE_TYPE_PORT_UNREACHABLE;
                            break;
                        case ICMPV6_NOROUTE :
                            trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                            history_rc = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
                            break;
                        case ICMPV6_ADDR_UNREACH :
                            trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                            history_rc = TRACEROUTE_TYPE_HOST_UNREACHABLE;
                            break;
                        default:
                            trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                            history_rc = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
                            break;
                    }
                    break;
                case ICMPV6_PKT_TOOBIG :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_FRAGMENTATION_NEEDED;
                    break;
                case ICMPV6_TIME_EXCEED :
                    if (icmp_packet_code == ICMPV6_EXC_HOPLIMIT)
                    {
                        trace_route_workspace[workspace_index].unreceived_probs_per_hop = 0;
                        trace_route_workspace[workspace_index].hop_failures = 0;
                        history_rc = TRACEROUTE_TYPE_EXCEED_MAX_TTL;
                    }
                    break;

                case ICMPV6_PARAMPROB :
                    if(icmp_packet_code == ICMPV6_UNK_NEXTHDR)
                    {
                        trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                        history_rc = TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE;
                    }
                    break;
                default :
                    trace_route_workspace[workspace_index].unreceived_probs_per_hop++;
                    history_rc = TRACEROUTE_TYPE_NETWORK_UNREACHABLE;
                    break;
            } /* switch */
            break;
#endif /* #if(SYS_CPNT_IPV6 == TRUE) */

        default :
            DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "Invalid target_addr_type.");
            return TRACEROUTE_TYPE_INVALID_ARG;
    }


    /* Set prob history entry.
     */
    if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(workspace_index, &list_elm)) !=  TRACEROUTE_TYPE_OK)
        return res;

    memcpy(prob_history_entry.trace_route_ctl_owner_index, list_elm.owner_index,list_elm.owner_index_len);
    prob_history_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
    memcpy(prob_history_entry.trace_route_ctl_test_name, list_elm.test_name,list_elm.test_name_len);
    prob_history_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;
    prob_history_entry.trace_route_probe_history_index = ((hop - 1) * 3 + prob);
    prob_history_entry.trace_route_probe_history_hop_index = hop;
    prob_history_entry.trace_route_probe_history_probe_index = prob;
    prob_history_entry.trace_route_probe_history_haddr_type = trace_route_workspace[workspace_index].target_addr_type;
    L_INET_SockaddrToInaddr(from_p, &prob_history_entry.trace_route_probe_history_haddr);
    prob_history_entry.trace_route_probe_history_response = (response_time * 10);
    prob_history_entry.trace_route_probe_history_status = VAL_traceRouteProbeHistoryStatus_responseReceived;
    prob_history_entry.trace_route_probe_history_last_rc = history_rc;
    SYS_TIME_GetDayAndTime(prob_history_entry.trace_route_probe_history_time,
                          &prob_history_entry.trace_route_probe_history_time_len);
    res = TRACEROUTE_OM_AppendProbePacketResult(workspace_index, prob_history_entry);
    DEBUG_PRINTF("%s, %d, res: %ld\n", __FUNCTION__, __LINE__, (long)res);
    return res;

}

/* FUNCTION NAME : TRACEROUTE_MGR_InitControlEntry
 * PURPOSE:
 *      Init control entry with default value stated in MIB
 * INPUT:
 *      ctl_entry - control entry to init.
 * OUTPUT:
 *      None.
 * RETURN:
 *      None
 * NOTES:
 *      1. Caller should be SetCtlRowStatus, not SetCtlEntry.
 */

static void TRACEROUTE_MGR_InitControlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctl_entry_p)
{
    /* BODY
     */
    ctl_entry_p->trace_route_ctl_target_address_type = SYS_DFLT_TRACEROUTE_CTL_IP_ADDRESS_TYPE;
    ctl_entry_p->trace_route_ctl_by_pass_route_table = SYS_DFLT_TRACEROUTE_CTL_BY_PASS_ROUTE_TABLE;
    ctl_entry_p->trace_route_ctl_data_size = SYS_DFLT_TRACEROUTE_CTL_DATA_SIZE;
    ctl_entry_p->trace_route_ctl_timeout = (SYS_DFLT_TRACEROUTE_CTL_TIME_OUT / SYS_BLD_TICKS_PER_SECOND);
    ctl_entry_p->trace_route_ctl_probes_per_hop = SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP;
    ctl_entry_p->trace_route_ctl_port = SYS_DFLT_TRACEROUTE_CTL_PORT;
    ctl_entry_p->trace_route_ctl_max_ttl = SYS_DFLT_TRACEROUTE_CTL_MAX_TTL;
    ctl_entry_p->trace_route_ctl_ds_field = SYS_DFLT_TRACEROUTE_CTL_DS_FIELD;
    ctl_entry_p->trace_route_ctl_source_address_type = VAL_traceRouteCtlSourceAddressType_ipv4;
    ctl_entry_p->trace_route_ctl_if_index = SYS_DFLT_TRACEROUTE_CTL_IFINDEX;
    ctl_entry_p->trace_route_ctl_max_failures = SYS_DFLT_TRACEROUTE_CTL_MAX_FAILURE;
    ctl_entry_p->trace_route_ctl_dont_fragment = SYS_DFLT_TRACEROUTE_CTL_DONT_FRAGMENT;
    ctl_entry_p->trace_route_ctl_initial_ttl = SYS_DFLT_TRACEROUTE_CTL_INITIAL_TTL;
    ctl_entry_p->trace_route_ctl_frequency = SYS_DFLT_TRACEROUTE_CTL_FREQUENCY;
    ctl_entry_p->trace_route_ctl_storage_type = SYS_DFLT_TRACEROUTE_CTL_STORAGE_TYPE;
    ctl_entry_p->trace_route_ctl_admin_status = SYS_DFLT_TRACEROUTE_CTL_ADMIN_STATUS;
    ctl_entry_p->trace_route_ctl_max_rows = SYS_DFLT_TRACEROUTE_CTL_MAX_ROWS;
    ctl_entry_p->trace_route_ctl_trap_generation = SYS_DFLT_TRACEROUTE_CTL_TRAP_GENERATION;
    ctl_entry_p->trace_route_ctl_create_hops_entries = SYS_DFLT_TRACEROUTE_CTL_CREATE_HOP_ENTRY;
    ctl_entry_p->trace_route_ctl_type = LEAF_traceRouteUsingUdpProbes;

    return;
} /* end of TRACEROUTE_MGR_InitControlEntry() */


#if (BACKDOOR_OPEN == TRUE)

static UI32_T TRACEROUTE_MGR_BD_CreateTraceRoute(L_INET_AddrIp_T *dest_addr_p)
{
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        ctl_entry;
    UI32_T      tid;
    UI32_T      res;
    char        task_name[SYSFUN_TASK_NAME_LENGTH];
    char        test_name[32];
    /* BODY
     */
    memset(task_name, 0, sizeof(task_name));
    memset(&ctl_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    memset(test_name, 0, sizeof(test_name));

    tid = SYSFUN_TaskIdSelf();

    if ((res = SYSFUN_TaskIDToName(tid, task_name, sizeof(task_name))) != SYSFUN_OK)
        return TRACEROUTE_TYPE_FAIL;
    strncpy(ctl_entry.trace_route_ctl_owner_index, task_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE);
    ctl_entry.trace_route_ctl_owner_index_len = strlen(ctl_entry.trace_route_ctl_owner_index);

    test_session++;
    snprintf(test_name, sizeof(test_name), TRACEROUTE_MGR_TEST_NAME, (unsigned long)test_session);
    strncpy(ctl_entry.trace_route_ctl_test_name, test_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE);
    ctl_entry.trace_route_ctl_test_name_len = strlen(ctl_entry.trace_route_ctl_test_name);

    /*L_INET_Ntoa(dip, ctl_entry.trace_route_ctl_target_address);
    ctl_entry.trace_route_ctl_target_address_len = strlen(ctl_entry.trace_route_ctl_target_address);
    L_INET_Ntoa(sip, ctl_entry.trace_route_ctl_source_address);
    ctl_entry.trace_route_ctl_source_address_len = strlen(ctl_entry.trace_route_ctl_source_address);
    */
    ctl_entry.trace_route_ctl_target_address_type = dest_addr_p->type;
    memcpy(&ctl_entry.trace_route_ctl_target_address, dest_addr_p, sizeof(L_INET_AddrIp_T));

   ctl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_createAndGo;

    res = TRACEROUTE_MGR_SetTraceRouteCtlEntry(&ctl_entry);

    return res;

} /* end of TRACEROUTE_MGR_CreateTraceRoute() */

static void TRACEROUTE_MGR_PrintControlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    /* Local Variable Declaration
     */
    char tmp_buf[MAX_BUF_LEN];

    /* BODY
     */
    BACKDOOR_MGR_Printf ("\ntrace_route_ctl_owner_index %s \n",ctl_entry_p->trace_route_ctl_owner_index);
    BACKDOOR_MGR_Printf ("\ntrace_route_ctl_owner_index_len %ld \n", (long)ctl_entry_p->trace_route_ctl_owner_index_len);
    BACKDOOR_MGR_Printf ("trace_route_ctl_test_name %s \n", ctl_entry_p->trace_route_ctl_test_name);
    BACKDOOR_MGR_Printf ("trace_route_ctl_test_name_len %ld \n", (long)ctl_entry_p->trace_route_ctl_test_name_len);
    BACKDOOR_MGR_Printf ("trace_route_ctl_target_address_type %d \n",(int) ctl_entry_p->trace_route_ctl_target_address_type);
    //BACKDOOR_MGR_Printf ("trace_route_ctl_target_address %s \n", ctl_entry_p->trace_route_ctl_target_address);
    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&ctl_entry_p->trace_route_ctl_target_address, tmp_buf, sizeof(tmp_buf));
    BACKDOOR_MGR_Printf("trace_route_ctl_target_address: %s\n", tmp_buf);

    BACKDOOR_MGR_Printf ("trace_route_ctl_by_pass_route_table %d \n",(int) ctl_entry_p->trace_route_ctl_by_pass_route_table);
    BACKDOOR_MGR_Printf ("trace_route_ctl_data_size %d \n", (int)ctl_entry_p->trace_route_ctl_data_size);
    BACKDOOR_MGR_Printf ("trace_route_ctl_timeout %d \n", (int)ctl_entry_p->trace_route_ctl_timeout);
    BACKDOOR_MGR_Printf ("trace_route_ctl_probes_per_hop %d \n",(int) ctl_entry_p->trace_route_ctl_probes_per_hop);
    BACKDOOR_MGR_Printf ("trace_route_ctl_port %d \n",(int) ctl_entry_p->trace_route_ctl_port);
    BACKDOOR_MGR_Printf ("trace_route_ctl_max_ttl %d \n",(int) ctl_entry_p->trace_route_ctl_max_ttl);
    BACKDOOR_MGR_Printf ("trace_route_ctl_ds_field %d \n",(int) ctl_entry_p->trace_route_ctl_ds_field);
    BACKDOOR_MGR_Printf ("trace_route_ctl_source_address_type %d \n",(int) ctl_entry_p->trace_route_ctl_source_address_type);
    //BACKDOOR_MGR_Printf ("trace_route_ctl_source_address %s \n", ctl_entry_p->trace_route_ctl_source_address);
    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&ctl_entry_p->trace_route_ctl_source_address, tmp_buf, sizeof(tmp_buf));
    BACKDOOR_MGR_Printf("trace_route_ctl_source_address: %s\n", tmp_buf);

    BACKDOOR_MGR_Printf ("trace_route_ctl_if_index %d \n",(int) ctl_entry_p->trace_route_ctl_if_index);
    BACKDOOR_MGR_Printf ("trace_route_ctl_misc_optionsaddress %s \n", ctl_entry_p->trace_route_ctl_misc_options);
    BACKDOOR_MGR_Printf ("trace_route_ctl_max_failures %d \n", (int)ctl_entry_p->trace_route_ctl_max_failures);
    BACKDOOR_MGR_Printf ("trace_route_ctl_dont_fragment %d \n",(int) ctl_entry_p->trace_route_ctl_dont_fragment);
    BACKDOOR_MGR_Printf ("trace_route_ctl_initial_ttl %d \n",(int) ctl_entry_p->trace_route_ctl_initial_ttl);
    BACKDOOR_MGR_Printf ("trace_route_ctl_frequency %d \n", (int)ctl_entry_p->trace_route_ctl_frequency);
    BACKDOOR_MGR_Printf ("trace_route_ctl_storage_type %d \n",(int) ctl_entry_p->trace_route_ctl_storage_type);
    BACKDOOR_MGR_Printf ("trace_route_ctl_admin_status %d \n",(int) ctl_entry_p->trace_route_ctl_admin_status);
    BACKDOOR_MGR_Printf ("trace_route_ctl_max_rows %d \n", (int)ctl_entry_p->trace_route_ctl_max_rows);
    BACKDOOR_MGR_Printf ("trace_route_ctl_trap_generation %d \n",(int) ctl_entry_p->trace_route_ctl_trap_generation);
    BACKDOOR_MGR_Printf ("trace_route_ctl_descr %s \n", ctl_entry_p->trace_route_ctl_descr);
    BACKDOOR_MGR_Printf ("trace_route_ctl_create_hops_entries %d \n",(int) ctl_entry_p->trace_route_ctl_create_hops_entries);
    BACKDOOR_MGR_Printf ("trace_route_ctl_type %d \n", (int)ctl_entry_p->trace_route_ctl_type);
    BACKDOOR_MGR_Printf ("trace_route_ctl_rowstatus %d \n",(int) ctl_entry_p->trace_route_ctl_rowstatus);
    return;

}

static void TRACEROUTE_MGR_PrintResultEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* Local Variable Declaration
     */
    char tmp_buf[MAX_BUF_LEN];

    /* BODY
     */
    /* only print some important parts */
    printf ("\nctl_owner_index %s \n", result_entry_p->trace_route_ctl_owner_index);
    printf ("ctl_test_name %s \n", result_entry_p->trace_route_ctl_test_name);
    printf ("ctl_owner_index_len %d\n", (int) result_entry_p->trace_route_ctl_owner_index_len);
    printf ("ctl_test_name_len %d\n", (int) result_entry_p->trace_route_ctl_test_name_len);
    printf ("results_oper_status (enabled/disabled=1/2) %d\n", (int) result_entry_p->trace_route_results_oper_status);
    printf ("results_cur_hop_count %ld\n", (long)result_entry_p->trace_route_results_cur_hop_count);
    printf ("results_cur_probe_count %ld\n", (long)result_entry_p->trace_route_results_cur_probe_count);
    printf ("results_ip_tgt_addr_type %ld\n", (long)result_entry_p->trace_route_results_ip_tgt_addr_type);

    L_INET_InaddrToString((L_INET_Addr_T *)&result_entry_p->trace_route_results_ip_tgt_addr, tmp_buf, sizeof(tmp_buf));
    printf ("trace_route_results_ip_tgt_addrs %s\n", tmp_buf);
    printf ("results_test_attempts %d\n", (int) result_entry_p->trace_route_results_test_attempts);
    printf ("results_test_successes %d\n", (int) result_entry_p->trace_route_results_test_successes);

    return;
}
#if 0
static void   TRACEROUTE_MGR_DumpWorkspace(void)
{
    /* Local Variable Declaration
     */
    UI32_T  index;
//    UI8_T   src_ip_str[18] = {0};
//    UI8_T   dest_ip_str[18] = {0};
    char tmp_buf[MAX_BUF_LEN];

    /* BODY */
    BACKDOOR_MGR_Printf ("\nID\t tid\t\t src_ip\t\t dest_ip\n");

    for (index=0; index<SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; index++)
    {
        BACKDOOR_MGR_Printf ("%1d\t %d\t",(int)index,
               (int)trace_route_workspace[index].owner_tid);
/*        L_INET_Ntoa(*(UI32_T *)trace_route_workspace[index].src_ip,src_ip_str);
        BACKDOOR_MGR_Printf ("%15s\t", src_ip_str);
        L_INET_Ntoa(*(UI32_T *)trace_route_workspace[index].dst_ip,dest_ip_str);
        BACKDOOR_MGR_Printf ("%15s\t", dest_ip_str);
*/

        printf("target_addr_type: %ld\n", trace_route_workspace[index].target_addr_type);

        memset(tmp_buf, 0, MAX_BUF_LEN);
        L_INET_InaddrToString((L_INET_Addr_T *)&trace_route_workspace[index].src_ip, tmp_buf, sizeof(tmp_buf));
        printf("src_addr: %s\n", tmp_buf);

        memset(tmp_buf, 0, MAX_BUF_LEN);
        L_INET_InaddrToString((L_INET_Addr_T *)&trace_route_workspace[index].dst_ip, tmp_buf, sizeof(tmp_buf));
        printf("dst_addr: %s\n", tmp_buf);


        BACKDOOR_MGR_Printf ("\n");
    } /* end of for */

    return;

} /* end of TRACEROUTE_MGR_DumpWorkspace() */
#endif
static void TRACEROUTE_MGR_DisplayAllWorkspaceDetail()
{
    /* Local Variable Declaration
     */
    UI32_T  index;
//    UI8_T   src_ip_str[18] = {0};
//    UI8_T   dest_ip_str[18] = {0};
    char tmp_buf[MAX_BUF_LEN];

    /* BODY */
    for (index=0; index<SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; index++)
    {

    BACKDOOR_MGR_Printf ("\n");
    BACKDOOR_MGR_Printf ("owner tid %d\n", (int)trace_route_workspace[index].owner_tid);
    BACKDOOR_MGR_Printf ("prob port %d\n",(int) trace_route_workspace[index].probe_port);
    BACKDOOR_MGR_Printf ("ident %d\n",(int) trace_route_workspace[index].ident);
/*    L_INET_Ntoa(*(UI32_T *)trace_route_workspace[workspace_index].src_ip,src_ip_str);
    BACKDOOR_MGR_Printf ("Source IP %15s\n", src_ip_str);
    L_INET_Ntoa(*(UI32_T *)trace_route_workspace[workspace_index].dst_ip,dest_ip_str);
    BACKDOOR_MGR_Printf ("Destincation IP%15s\n", dest_ip_str);
*/

    printf("target_addr_type: %ld\n", (long)trace_route_workspace[index].target_addr_type);

    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&trace_route_workspace[index].src_ip, tmp_buf, sizeof(tmp_buf));
    printf("src_addr: %s\n", tmp_buf);

    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&trace_route_workspace[index].dst_ip, tmp_buf, sizeof(tmp_buf));
    printf("dst_addr: %s\n", tmp_buf);


    BACKDOOR_MGR_Printf ("\n");
/*    BACKDOOR_MGR_Printf ("Wherefrom.sin_len %d\n", (int)trace_route_workspace[workspace_index].wherefrom.sin_len);  NOTE: wakka, not support in linux */
    //BACKDOOR_MGR_Printf ("Wherefrom.sin_family %d\n",(int) trace_route_workspace[workspace_index].wherefrom.sin_family);
    //BACKDOOR_MGR_Printf ("Wherefrom.sin_port %d\n", (int)trace_route_workspace[workspace_index].wherefrom.sin_port);
    BACKDOOR_MGR_Printf ("\n");
/*    BACKDOOR_MGR_Printf ("Whereto.sin_len %d\n", (int)trace_route_workspace[workspace_index].whereto.sin_len);  NOTE: wakka, not support in linux */
    BACKDOOR_MGR_Printf ("Whereto.sin_family %d\n",(int) trace_route_workspace[index].whereto.sin_family);
    BACKDOOR_MGR_Printf ("Whereto.sin_port %d\n",(int) L_STDLIB_Ntoh16(trace_route_workspace[index].whereto.sin_port));
    BACKDOOR_MGR_Printf ("\n");
    }
    return;

} /* end of TRACEROUTE_MGR_DumpDetailWorkspace() */

static void   TRACEROUTE_MGR_DisplayKeyIndexTable(void)
{
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    /*UI8_T   owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE];
    UI8_T   test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE];
    */
    UI32_T  local_index = 0;
    UI32_T  res;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T    control_entry;
    char tmp_buf[MAX_BUF_LEN];


    /* BODY
     */

    BACKDOOR_MGR_Printf("\nKeyIndex\t Owner Index\t Test Name \tTarget IP\n");
    BACKDOOR_MGR_Printf("-----------------------------------------------------------\n");

    for (; local_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; local_index++)
    {
        /*memset(&owner_index, 0, sizeof(UI8_T)*SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE);
        memset(&test_name, 0, sizeof(UI8_T)*SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE);
        */
        if ((res = TRACEROUTE_OM_TraceRouteTableIndexToKey(local_index, &list_elm)) == TRACEROUTE_TYPE_OK)
        {
            /* get Target IP from Ctl Entry */
            memcpy(control_entry.trace_route_ctl_owner_index, list_elm.owner_index, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
            control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;
            memcpy(control_entry.trace_route_ctl_test_name, list_elm.test_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
            control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

            if (TRACEROUTE_TYPE_OK != (res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&control_entry)))
            {
                continue;
            }

            BACKDOOR_MGR_Printf("    %d  \t  %s  \t    %s\t", (int)local_index, list_elm.owner_index, list_elm.test_name);
            //BACKDOOR_MGR_Printf ("%15s\n", control_entry.trace_route_ctl_target_address);

            //printf ("%-10ld* %-10.10s %-10.10s ", local_index, ctl_entry.ping_ctl_owner_index, ctl_entry.ping_ctl_test_name);
            memset(tmp_buf, 0, MAX_BUF_LEN);
            L_INET_InaddrToString((L_INET_Addr_T *)&control_entry.trace_route_ctl_target_address, tmp_buf, sizeof(tmp_buf));
            BACKDOOR_MGR_Printf(" %s\n", tmp_buf);
        }

    } /* end of for */
    return;
} /* end of TRACEROUTE_MGR_DisplayKeyIndexTable() */

static void TRACEROUTE_MGR_PrintProbeHistoryEntry(UI32_T workspace_index)
{
    //TRACEROUTE_TYPE_TraceRouteCtlEntry_T            control_entry;
    TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T   prob_history_entry;
    TRACEROUTE_SORTLST_ELM_T list_elm;
    UI32_T  prev_hop = 0;
/*    UI32_T  table_index;
    UI32_T  owner_index_len=0,test_name_len=0;
    UI8_T   owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE];
    UI8_T   test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE];
*/
    int i = 0;
    UI32_T  res;

    /* BODY
     */

    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    res = TRACEROUTE_OM_TraceRouteTableIndexToKey(workspace_index, &list_elm);
    if(res != TRACEROUTE_TYPE_OK)
        return;

    //memset(&control_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
    memset(&prob_history_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));

    BACKDOOR_MGR_Printf ("\n  Hops \t first  \t second \t third  \t IP addr  ");
    BACKDOOR_MGR_Printf ("\n ------------------------------------------------------- ");

    while (TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry(list_elm.owner_index, list_elm.owner_index_len,
                list_elm.test_name, list_elm.test_name_len, prob_history_entry.trace_route_probe_history_index,
                prob_history_entry.trace_route_probe_history_hop_index,
                prob_history_entry.trace_route_probe_history_probe_index, &prob_history_entry) == TRACEROUTE_TYPE_OK)
    {
        if(i++ > 20)break;
        if(prev_hop != prob_history_entry.trace_route_probe_history_hop_index)
        {
            BACKDOOR_MGR_Printf("\n%-4d \t", (int)prob_history_entry.trace_route_probe_history_hop_index);
        }
        if ((prob_history_entry.trace_route_probe_history_last_rc == TRACEROUTE_TYPE_EXCEED_MAX_TTL) ||
            (prob_history_entry.trace_route_probe_history_last_rc == TRACEROUTE_TYPE_PORT_UNREACHABLE))
        {
            if (prob_history_entry.trace_route_probe_history_response == 0)
            {
                BACKDOOR_MGR_Printf ("<10 ms \t");
            }
            else
            {
                BACKDOOR_MGR_Printf ("%4d ms \t", (int)prob_history_entry.trace_route_probe_history_response);
            }
        }
        else
        {
            BACKDOOR_MGR_Printf ("  *  \t");
        }

        if (prob_history_entry.trace_route_probe_history_probe_index == SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP)
        {
            char tmp_buf[MAX_BUF_LEN];


            /*if(memcmp(prob_history_entry.trace_route_probe_history_haddr.addr, )
            {
                BACKDOOR_MGR_Printf(" %-15s", prob_history_entry.trace_route_probe_history_haddr);
            }
            */

            memset(tmp_buf, 0, MAX_BUF_LEN);
            L_INET_InaddrToString((L_INET_Addr_T *)&prob_history_entry.trace_route_probe_history_haddr, tmp_buf, sizeof(tmp_buf));
            BACKDOOR_MGR_Printf(" %-15s", tmp_buf);


            memset(tmp_buf, 0, MAX_BUF_LEN);
            L_INET_InaddrToString((L_INET_Addr_T *)&trace_route_workspace[workspace_index].dst_ip, tmp_buf, sizeof(tmp_buf));

            BACKDOOR_MGR_Printf("ctl_entry addr: %s\n", tmp_buf);
            if(0 == memcmp(&prob_history_entry.trace_route_probe_history_haddr, &trace_route_workspace[workspace_index].dst_ip, sizeof(L_INET_AddrIp_T)))
            {
                /* reach the target address, it's the end. */
                BACKDOOR_MGR_Printf("\nTraceroute Complete.\n");
                return;
            }
        }
        prev_hop = prob_history_entry.trace_route_probe_history_hop_index;
    } /* end of while */

    return;
} /* end of TRACEROUTE_MGR_DisplayTraceRouteResult() */


/* FUNCTION NAME : TRACEROUTE_MGR_BackDoor_Menu
 * PURPOSE:
 *      For testing trace route function.
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
 *      None
 */
static void TRACEROUTE_MGR_BackDoor_Menu(void)
{
    /* Local Variable Declaration
     */
    I32_T   ch;
    BOOL_T  eof=FALSE;
    char    buf[MAX_BUF_LEN + 1];
    UI32_T  workspace_index;
    UI32_T  res;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        control_entry;
    TRACEROUTE_TYPE_TraceRouteResultsEntry_T    result_entry;
    L_INET_AddrIp_T inet_addr;
    TRACEROUTE_SORTLST_ELM_T list_elm;

    /* BODY  */

    while (! eof)
    {
        BACKDOOR_MGR_Printf ("\n 0. Exit\n");
        BACKDOOR_MGR_Printf (" 1. traceroute v4/v6\n");
        BACKDOOR_MGR_Printf (" 2. Enable / Disable TraceRoute control entry.\n");
        BACKDOOR_MGR_Printf (" 3. Display Control Entry.\n");
        BACKDOOR_MGR_Printf (" 4. Display Result Entry.\n");
        BACKDOOR_MGR_Printf (" 5. Display ProbeHistoryEntry.\n");
        BACKDOOR_MGR_Printf (" 6. Display All ProbeHistory. \n");
        BACKDOOR_MGR_Printf (" 7. Enable / Disable debug flag (%s).\n", ((traceroute_backdoor_debug)?"on":"off"));
        BACKDOOR_MGR_Printf (" 8. Display all workspace detail.\n");
        BACKDOOR_MGR_Printf (" a. Display Key <-> Index Table\n");
        BACKDOOR_MGR_Printf (" select =");

        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf ("\n");
        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;
            case '1' :

                BACKDOOR_MGR_Printf("Input target ipv4/v6 adress (A.B.C.D or X:X:X:X::X[scope_id]) = \n");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                memset(&inet_addr, 0, sizeof(L_INET_AddrIp_T));
                L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, buf, (L_INET_Addr_T *)&inet_addr, sizeof(inet_addr));
                if ((res = TRACEROUTE_MGR_BD_CreateTraceRoute(&inet_addr)) == TRACEROUTE_TYPE_OK)
                    BACKDOOR_MGR_Printf("Set Operation Sucess\n");
                else
                    BACKDOOR_MGR_Printf("Set operation Fail\n");
                break;

            case '2' :

                BACKDOOR_MGR_Printf("Input workspace index: ");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                workspace_index = atoi((char *)buf);
                res = TRACEROUTE_OM_TraceRouteTableIndexToKey(workspace_index, &list_elm);
                if(res != TRACEROUTE_TYPE_OK)
                    break;

                memset(&control_entry, 0, sizeof(control_entry));

                memcpy(&control_entry.trace_route_ctl_owner_index, list_elm.owner_index, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;

                memcpy(&control_entry.trace_route_ctl_test_name, list_elm.test_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

                BACKDOOR_MGR_Printf ("\n");
                BACKDOOR_MGR_Printf("Input Status (1)Enable (2)Disable: ");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                TRACEROUTE_MGR_StrToVal((UI8_T *)buf, &control_entry.trace_route_ctl_admin_status);
                BACKDOOR_MGR_Printf ("\n");
                res = TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus(&control_entry);
                if (res == TRACEROUTE_TYPE_OK)
                    BACKDOOR_MGR_Printf("Set Operation Sucess\n");
                else
                    BACKDOOR_MGR_Printf("Set operation Fail\n");
                break;
            case '3' :
                BACKDOOR_MGR_Printf("Input workspace index: ");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                workspace_index = atoi(buf);
                res = TRACEROUTE_OM_TraceRouteTableIndexToKey(workspace_index, &list_elm);
                if(res != TRACEROUTE_TYPE_OK)
                    break;

                memset(&control_entry, 0, sizeof(control_entry));

                memcpy(&control_entry.trace_route_ctl_owner_index, list_elm.owner_index, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                control_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;

                memcpy(&control_entry.trace_route_ctl_test_name, list_elm.test_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                control_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

                if ((res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&control_entry)) == TRACEROUTE_TYPE_OK)
                {
                    TRACEROUTE_MGR_PrintControlEntry(&control_entry);
                }
                break;
            case '4' :
                BACKDOOR_MGR_Printf("Input workspace index: ");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                workspace_index = atoi(buf);
                res = TRACEROUTE_OM_TraceRouteTableIndexToKey(workspace_index, &list_elm);
                if(res != TRACEROUTE_TYPE_OK)
                    break;

                memset(&result_entry, 0, sizeof(result_entry));

                memcpy(&result_entry.trace_route_ctl_owner_index, list_elm.owner_index, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                result_entry.trace_route_ctl_owner_index_len = list_elm.owner_index_len;

                memcpy(&result_entry.trace_route_ctl_test_name, list_elm.test_name, SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1);
                result_entry.trace_route_ctl_test_name_len = list_elm.test_name_len;

                if ((res = TRACEROUTE_OM_GetTraceRouteResultsEntry(&result_entry)) == TRACEROUTE_TYPE_OK)
                {
                    TRACEROUTE_MGR_PrintResultEntry(&result_entry);
                }
                break;
            case '5' :
                BACKDOOR_MGR_Printf("Input workspace index: ");
                BACKDOOR_MGR_RequestKeyIn((char *)buf, MAX_BUF_LEN);
                workspace_index = atoi(buf);

                TRACEROUTE_MGR_PrintProbeHistoryEntry(workspace_index);
                break;
            case '6' :
                TRACEROUTE_MGR_OM_DisplayAllProbeHistory();
                break;
            case '7' :
                traceroute_backdoor_debug  = !traceroute_backdoor_debug;
                break;
            case '8':
                TRACEROUTE_MGR_DisplayAllWorkspaceDetail();
                break;
            case 'a':
                TRACEROUTE_MGR_DisplayKeyIndexTable();
                break;
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */
}   /*  end of TRACEROUTE_MGR_BackDoor_Menu */

static  BOOL_T  TRACEROUTE_MGR_StrToVal(UI8_T *str_p, UI32_T *value_p)
{
    UI8_T   index, base, val_index, ch;
    UI32_T  str_value;
    BOOL_T  error;

    index       = 0;
    val_index   = 0;
    base        = 10;
    str_value   = 0;
    error       = FALSE;
    while ( (!error) && (str_p[index] != 0) )
    {
        ch = str_p[index];
        switch (ch)
        {
            case 'x':
            case 'X':
                if (val_index == 1)
                {
                    if (base == 8)
                        base = 16;
                    else
                        error = TRUE;
                }
                break;
            case ' ':
                index++;
                break;
            default:
                if ( (ch >= '0') && (ch <= '9'))
                {
                    if ( (val_index == 0) && (ch == '0') )
                    {
                        base = 8;
                    }
                    else
                    {
                        str_value = (UI32_T)(str_value * base) + (UI32_T)(ch - '0');
                    }
                    val_index++;
                }
                else
                {
                    error = TRUE;
                }
                break;
        } /* End of switch */
        index++;
    }
    *value_p = str_value;

    return (!error);
} /* End of TRACEROUTE_MGR_StrToVal */
#endif  /*  end of #if BACKDOOR_OPEN    */

/* FUNCTION NAME : TRACEROUTE_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for TRACEROUTE_MGR.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T TRACEROUTE_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    TRACEROUTE_MGR_IPCMsg_T *traceroute_mgr_msg_p;
    BOOL_T need_respond=TRUE;

    if(ipcmsg_p==NULL)
        return FALSE;

    traceroute_mgr_msg_p = (TRACEROUTE_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        switch(traceroute_mgr_msg_p->type.cmd)
        {
            default:
                traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_TYPE_FAIL;
                ipcmsg_p->msg_size = TRACEROUTE_MGR_MSGBUF_TYPE_SIZE;
                need_respond = TRUE;
        }

        return need_respond;
    }

    switch(traceroute_mgr_msg_p->type.cmd)
    {
        printf("%s, %d, %ld\n", __FUNCTION__, __LINE__, (long)traceroute_mgr_msg_p->type.cmd);
        case TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLENTRY:
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_MGR_SetTraceRouteCtlEntry(&traceroute_mgr_msg_p->data.ctl_entry);
            ipcmsg_p->msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctl_entry);
            need_respond = TRUE;
            break;
/*
        case TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLADMINSTATUS:
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_MGR_SetTraceRouteCtlAdminStatus(
                traceroute_mgr_msg_p->data.ctrl_ui32.owner_index,
                traceroute_mgr_msg_p->data.ctrl_ui32.owner_index_len,
                traceroute_mgr_msg_p->data.ctrl_ui32.test_name,
                traceroute_mgr_msg_p->data.ctrl_ui32.test_name_len,
                traceroute_mgr_msg_p->data.ctrl_ui32.ui32);
            ipcmsg_p->msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctrl_ui32);
            need_respond = TRUE;
            break;

        case TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLROWSTATUS:
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_MGR_SetTraceRouteCtlRowStatus(
                traceroute_mgr_msg_p->data.ctrl_ui32.owner_index,
                traceroute_mgr_msg_p->data.ctrl_ui32.owner_index_len,
                traceroute_mgr_msg_p->data.ctrl_ui32.test_name,
                traceroute_mgr_msg_p->data.ctrl_ui32.test_name_len,
                traceroute_mgr_msg_p->data.ctrl_ui32.ui32);
            ipcmsg_p->msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctrl_ui32);
            need_respond = TRUE;
            break;

        case TRACEROUTE_MGR_IPCCMD_SETTRACEROUTECTLTARGETADDRESS:
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_MGR_SetTraceRouteCtlTargetAddress(
                traceroute_mgr_msg_p->data.ctrl_target_address.owner_index,
                traceroute_mgr_msg_p->data.ctrl_target_address.owner_index_len,
                traceroute_mgr_msg_p->data.ctrl_target_address.test_name,
                traceroute_mgr_msg_p->data.ctrl_target_address.test_name_len,
                traceroute_mgr_msg_p->data.ctrl_target_address.target_addr,
                traceroute_mgr_msg_p->data.ctrl_target_address.target_addr_len);
            ipcmsg_p->msg_size = TRACEROUTE_MGR_GET_MSG_SIZE(ctrl_target_address);
            need_respond = TRUE;
            break;
*/
        case TRACEROUTE_MGR_IPCCMD_SETCTLENTRBYFIELD:
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_MGR_SetCtlEntryByField(
                &traceroute_mgr_msg_p->data.ctl_entry_ui32.ctl_entry,
                traceroute_mgr_msg_p->data.ctl_entry_ui32.ui32);

            ipcmsg_p->msg_size = TRACEROUTE_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        default:
            printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            traceroute_mgr_msg_p->type.result_ui32 = TRACEROUTE_TYPE_FAIL;
            ipcmsg_p->msg_size = TRACEROUTE_MGR_MSGBUF_TYPE_SIZE;
    }

    return need_respond;

}

