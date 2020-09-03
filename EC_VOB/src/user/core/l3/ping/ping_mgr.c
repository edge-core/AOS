/* Module Name: ping_mgr.c
 * Purpose:
 *      This component implements the standard Ping-MIB functionality (rfc2925).
 *      The ping task will sends out ICMP ECHO request and waits for ICMP ECHO reply.
 *      Both Send and receive operation are done through socket.
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *      2. Currently support IPv4 address only.
 *      3. The CLI_API_Ping command also been modified.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard Ping-MIB.
 *      2007/12     --  peter_yu    Porting to linux platform.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 *                                  (2) Handle IPC Message.
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include "leaf_2925p.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_inet.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "ip_lib.h"
#include "ping_mgr.h"
#include "ping_om.h"
#include "ping_om_private.h"
#include "ping_type.h"
#include "ping_task.h"
#include "sys_time.h"
#include "vlan_lib.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define BACKDOOR_OPEN                       TRUE

#define MAX_BUF_LEN                         45

#define PING_MGR_MAX_PROCESS_PACKETS        10  /* The maximun packets to process during each one second. */

#define INIT_MAX_DELTA_TIME                 0
#define INIT_MIN_DELTA_TIME                 0   /* shumin.wang fix bug ES3628BT-FLF-ZZ-00614 */

#define INIT_WAITING_TIME                   95 /* unit: tick, 100 = 1 second / 10 ms */
/* MACRO FUNCTION DECLARATIONS
 */
#define DEBUG_PRINTF(args...) do { \
if (ping_backdoor_debug) \
    BACKDOOR_MGR_Printf(args); \
} while(0);


/* LOCAL DATATYPE DECLARATION
 */
extern BOOL_T ping_backdoor_debug; /* declared in ping_mgr.h */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T   icmp_type;
    UI8_T   icmp_code;
    UI16_T  icmp_csum;
    UI16_T  icmp_id;
    UI16_T  icmp_seq;
} __attribute__((packed, aligned(1))) PING_MGR_ICMP_T;

/* format of a (icmp) probe packet.
 */
struct opacket
{
    PING_MGR_ICMP_T icmp;           /* icmp header */
};

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
    PING_MGR_ICMP_T     icmp;       /* icmp header */
} __attribute__((packed, aligned(1))) ip_packet_T;

#if (SYS_CPNT_IPV6 == TRUE)
// ipv6
typedef struct
{
    UI8_T   type;
    UI8_T   code;
    UI16_T  checksum;
    UI16_T  id;
    UI16_T  seq;
} __attribute__((packed, aligned(1))) PING_MGR_ICMP6_T;


struct opacket6
{
    PING_MGR_ICMP6_T icmp6;           /* icmp header */
};

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


/* Workspace for each client */
typedef struct PING_MGR_WorkSpace_S
{
    struct  opacket     outpacket;  /* packet to be send */
#if (SYS_CPNT_IPV6 == TRUE)
    struct  opacket6     outpacket6;  /* packet to be send */
#endif
    //struct  sockaddr_in wherefrom;  /* source sock addr */
    struct  sockaddr_in whereto;    /* destination sock addr */

#if (SYS_CPNT_IPV6 == TRUE)
    struct  sockaddr_in6 whereto6;    /* destination sock addr */
#endif
    UI32_T  owner_tid;              /* owner task ID */

    UI32_T  target_addr_type;
    L_INET_AddrIp_T src_ip;        /* the interface IP address of outbound probe */
    L_INET_AddrIp_T dst_ip;         /* target to be probed */

    UI16_T  ident;                  /* default is (0x8000 | (taskid & 0xffff)) */
    UI16_T  datalen;                /* send out data len */
    UI32_T  admin_status;           /* this flag must be turned on when operation starts */
    UI16_T  rcv_pkt_len;            /* received packet length in packet */
    UI32_T  last_transmit_time;     /* last transmit time */
    UI32_T  last_receive_time;      /* last receive time */
    UI32_T  probe_count;            /* count */
    UI32_T  sent_probes;            /* number of sent probs */
    UI32_T  received_probs;         /* received probs */
    UI32_T  unreceived_probs;       /* number of unreceived probs */
    UI32_T  last_access_time;       /* last access time */
    UI32_T  timeout_second;         /* time-out value in seconds */
    UI32_T  last_send_status;       /* last rc returned by sendto() */
    BOOL_T  mcast_ipv6_target_addr; /* target addr is ipv6 mcast */
    BOOL_T  probe_history_last_index; /* the last(max) index of probe history entries */
    BOOL_T  dont_fragment;          /* always set dont fragment bit in IP header */

}   PING_MGR_WorkSpace_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void PING_MGR_InitControlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);
static UI32_T PING_MGR_SendProbe(void);
static UI32_T PING_MGR_WaitForReply(void);
static void PING_MGR_CheckToStartTimer(void);

#if (BACKDOOR_OPEN == TRUE)
static void PING_MGR_BackDoor_Menu(void);
#endif
static unsigned short ipchksum(unsigned short *ip, int len);

/* STATIC VARIABLE DECLARATIONS
 */
static PING_MGR_WorkSpace_T ping_workspace[SYS_ADPT_MAX_PING_NUM];

static UI32_T   current_operation_mode;
static UI32_T   test_session = 0;
static I32_T   ping_send_socket = 0;
static I32_T   ping_receive_socket = 0;
#if (SYS_CPNT_IPV6 == TRUE)
static I32_T   ping6_send_socket = 0;
static I32_T   ping6_receive_socket = 0;
#endif
static PING_TYPE_PingResultsEntry_T     global_result_entry;
static PING_TYPE_PingCtlEntry_T         global_control_entry;

static PING_TYPE_PingCtlEntry_T privateMIBCtlEntry;

static I32_T waiting_time; /* unit: tick, the remaining time for PING_MGR_WaitForReply() to wait in 1 second (100 ticks) */
static UI32_T waiting_time_stamp; /* future time_stamp boundary for waiting_time */

SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : PING_MGR_Initiate_System_Resources
 * PURPOSE:
 *      Initialize working space of ping utility.
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
 *      2. The max simutanious ping requests is defined in SYS_BLD.h.
 */
void PING_MGR_Initiate_System_Resources(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ping",
        SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY, PING_MGR_BackDoor_Menu);
    memset(ping_workspace, 0, sizeof(PING_MGR_WorkSpace_T)*(SYS_ADPT_MAX_PING_NUM));
    PING_OM_Initiate_System_Resources();
} /* end of PING_MGR_Initiate_System_Resources() */


/* FUNCTION NAME : PING_MGR_EnterMasterMode
 * PURPOSE:
 *      Ping enters master mode.
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
void PING_MGR_EnterMasterMode (void)
{
    /* Local Variable Declaration
     */

    /* BODY
     */
    SYSFUN_ENTER_MASTER_MODE();
    PING_OM_EnterMasterMode();

    /* create static ctl entry for private MIB */
    memset(&privateMIBCtlEntry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    privateMIBCtlEntry.ping_ctl_owner_index_len = SYS_DFLT_PING_PRIVATE_MIB_ID_LEN;
    memcpy(privateMIBCtlEntry.ping_ctl_owner_index, SYS_DFLT_PING_PRIVATE_MIB_ID, SYS_DFLT_PING_PRIVATE_MIB_ID_LEN); /* KEY 1 */
    privateMIBCtlEntry.ping_ctl_test_name_len = SYS_DFLT_PING_PRIVATE_MIB_ID_LEN;
    memcpy(privateMIBCtlEntry.ping_ctl_test_name, SYS_DFLT_PING_PRIVATE_MIB_ID, SYS_DFLT_PING_PRIVATE_MIB_ID_LEN); /* KEY 2 */

    privateMIBCtlEntry.ping_ctl_target_address_type = VAL_pingCtlTargetAddressType_ipv4;
    /* in order to keep rowstatus will be active after creation for private MIB, target_address is MUST */
    privateMIBCtlEntry.ping_ctl_target_address.type = L_INET_ADDR_TYPE_IPV4;
    privateMIBCtlEntry.ping_ctl_target_address.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    privateMIBCtlEntry.ping_ctl_target_address.addr[0] = 0xc0;
    privateMIBCtlEntry.ping_ctl_target_address.addr[1] = 0xa8;
    privateMIBCtlEntry.ping_ctl_target_address.addr[2] = 0x01;
    privateMIBCtlEntry.ping_ctl_target_address.addr[3] = 0x01;

    privateMIBCtlEntry.ping_ctl_timeout = (SYS_DFLT_PING_CTL_TIME_OUT / SYS_BLD_TICKS_PER_SECOND);
    privateMIBCtlEntry.ping_ctl_storage_type = SYS_DFLT_PING_CTL_STORAGE_TYPE;
    privateMIBCtlEntry.ping_ctl_by_pass_route_table = SYS_DFLT_PING_CTL_BY_PASS_ROUTE_TABLE;

    /* admin status is initialized with disabled */
    privateMIBCtlEntry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
    /* size */
    privateMIBCtlEntry.ping_ctl_data_size = 32;
    /* count */
    privateMIBCtlEntry.ping_ctl_probe_count = 1;
    privateMIBCtlEntry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_createAndGo;
    if(PING_TYPE_OK != PING_MGR_SetCtlEntry(&privateMIBCtlEntry))
    {
        /* fatal error, maybe we should log error here */
        return;
    }
    return;
} /* end of PING_MGR_EnterMasterMode() */


/* FUNCTION NAME : PING_MGR_EnterSlaveMode
 * PURPOSE:
 *      Ping enters Slave mode mode.
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
void PING_MGR_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
} /* end of PING_MGR_EnterSlaveMode() */


/* FUNCTION NAME : PING_MGR_EnterTransitionMode
 * PURPOSE:
 *      Ping enters Transition mode mode.
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
void PING_MGR_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    current_operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;

    memset(ping_workspace, 0, sizeof(PING_MGR_WorkSpace_T)*(SYS_ADPT_MAX_PING_NUM));
    memset(&global_result_entry,   0, sizeof(PING_TYPE_PingResultsEntry_T));
    memset(&global_control_entry,  0, sizeof(PING_TYPE_PingCtlEntry_T));
    ping_backdoor_debug = FALSE;
    test_session = 0;

    /*
    ping_send_socket = 0;
    ping_receive_socket = 0;

    ping6_send_socket = 0;
    ping6_receive_socket = 0;
    */
    PING_MGR_CloseSocket();

    return;
} /* end of PING_MGR_EnterTransitionMode() */


/* FUNCTION NAME : PING_MGR_SetTransitionMode
 * PURPOSE:
 *      ping enters set transition mode.  Clear OM
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
void PING_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
    PING_OM_ClearOM();
    return;
} /* end of PING_MGR_SetTransitionMode() */

/* FUNCTION NAME - PING_MGR_GetOperationMode
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
SYS_TYPE_Stacking_Mode_T  PING_MGR_GetOperationMode(void)
{
    return current_operation_mode;

} /* end of PING_MGR_GetOperationMode() */


/* FUNCTION NAME : PING_MGR_CreateSocket
 * PURPOSE:
 *      Create socket for all workspace.
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
UI32_T PING_MGR_CreateSocket(void)
{
    /* LOCAL VARIABLE DECLARATION
     */

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ping_receive_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
    {
        ping_receive_socket = 0;
        return PING_TYPE_NO_MORE_SOCKET;
    }

#if (SYS_CPNT_IPV6 == TRUE)
    if ((ping6_receive_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0)
    {
        ping6_receive_socket = 0;
        return PING_TYPE_NO_MORE_SOCKET;
    }
#endif
    return PING_TYPE_OK;

} /* end of PING_MGR_CreateSocket() */


/* FUNCTION NAME : PING_MGR_CloseSocket
 * PURPOSE:
 *      Close socket for all workspace.
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
UI32_T PING_MGR_CloseSocket(void)
{
    /* IPv4 */
    if (ping_send_socket != 0)
    {
        close(ping_send_socket);
        ping_send_socket = 0;
    }
    if (ping_receive_socket != 0)
    {
        close(ping_receive_socket);
        ping_receive_socket = 0;
    }

#if (SYS_CPNT_IPV6 == TRUE)
    /* IPv6 */
    if (ping6_send_socket != 0)
    {
        close(ping6_send_socket);
        ping6_send_socket = 0;
    }
    if (ping6_receive_socket != 0)
    {
        close(ping6_receive_socket);
        ping6_receive_socket = 0;
    }
#endif
    return PING_TYPE_OK;
} /* end of PING_MGR_CloseSocket() */

/* FUNCTION NAME: PING_MGR_TriggerPing
 * PURPOSE:
 *          To scan any active workspace, then send/wait pkts.
 * INPUT:
 *          None
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 */
UI32_T  PING_MGR_TriggerPing(void)
{
    UI32_T ret;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    waiting_time_stamp = SYSFUN_GetSysTick() + INIT_WAITING_TIME;
    ret = PING_MGR_SendProbe();
    waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();

    PING_MGR_WaitForReply();

    return ret;
} /* end of PING_MGR_TriggerPing() */

/* FUNCTION NAME: PING_MGR_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 *          3. key: ping_ctl_owner_index, ping_ctl_test_name.
 */
UI32_T PING_MGR_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{

    /* Local Variable Declaration
     */
    UI32_T              res, is_exist;

    /* BODY
     */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ctrl_entry_p->ping_ctl_owner_index == NULL) || (ctrl_entry_p->ping_ctl_test_name == NULL) ||
        (ctrl_entry_p->ping_ctl_owner_index_len <= 0) || (ctrl_entry_p->ping_ctl_owner_index_len > SYS_ADPT_PING_MAX_NAME_SIZE) ||
        (ctrl_entry_p->ping_ctl_test_name_len <= 0) || (ctrl_entry_p->ping_ctl_test_name_len > SYS_ADPT_PING_MAX_NAME_SIZE))
    {
        DEBUG_PRINTF("Failed! invalid arg.\n");
        return PING_TYPE_INVALID_ARG;
    }

    DEBUG_PRINTF("%s, rowstatus: %ld\n", __FUNCTION__, (long)ctrl_entry_p->ping_ctl_rowstatus);

    if ((ctrl_entry_p->ping_ctl_rowstatus < VAL_pingCtlRowStatus_active) ||
        (ctrl_entry_p->ping_ctl_rowstatus > VAL_pingCtlRowStatus_destroy))
    {
        DEBUG_PRINTF("Failed! invalid rowstatus.\n");
        return PING_TYPE_FAIL;
    }


    /* SNMP people said that if the target ip addr is 0, the rowstatus should be changed to notInService */
    /*    if(0 == ctrl_entry_p->ping_ctl_target_address)
    {
        ctrl_entry_p->ping_ctl_rowstatus = VAL_pingCtlRowStatus_notInService;
    }
    */
    is_exist = PING_OM_IsPingCtlEntryExist(ctrl_entry_p);

    switch (ctrl_entry_p->ping_ctl_rowstatus)
    {
        case VAL_pingCtlRowStatus_createAndWait:
        case VAL_pingCtlRowStatus_createAndGo:
            if (is_exist == PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            break;
        case VAL_pingCtlRowStatus_active:
        case VAL_pingCtlRowStatus_destroy:
            if (is_exist != PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            break;
        default:
            return PING_TYPE_FAIL;
    } /* end of switch */

    res = PING_OM_SetCtlEntry(ctrl_entry_p);

    PING_MGR_CheckToStartTimer();

    return res;

} /* end of PING_MGR_SetCtlEntry() */

/* FUNCTION NAME: PING_MGR_SetCtlAdminStatus
 * PURPOSE:
 *          To enable or disable ping control entry
 * INPUT:
 *          ctl_entry_p         -- the specific control entry.
 *          ctrl_admin_status   -- the admin status of the to enable or disable the ping.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1.This API is only used by "set by filed", not "set by record".
 */
UI32_T PING_MGR_SetCtlAdminStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p , UI32_T ctrl_admin_status)
{
    /* Local Variable Declaration
     */
    UI32_T      res;
    PING_TYPE_PingCtlEntry_T        local_entry;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return PING_TYPE_FAIL;

    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    if ((ctrl_admin_status != VAL_pingCtlAdminStatus_enabled) &&
        (ctrl_admin_status != VAL_pingCtlAdminStatus_disabled))
    {
        return PING_TYPE_INVALID_ARG;
    }

    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    memcpy(&local_entry.ping_ctl_owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(&local_entry.ping_ctl_test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_GetCtlEntry(&local_entry)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }

    if ( local_entry.ping_ctl_admin_status == ctrl_admin_status )
    {
        /* do nothing.*/
        return PING_TYPE_OK;
    }

    /* Setting admin status in a ctrl_entry is only permitted when the row status is active. */
    if (local_entry.ping_ctl_rowstatus != VAL_pingCtlRowStatus_active)
    {
        return PING_TYPE_FAIL;
    }
    local_entry.ping_ctl_admin_status = ctrl_admin_status;

    if ((res = PING_OM_SetCtlEntry(&local_entry)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }

    PING_MGR_CheckToStartTimer();

    return res;
} /* end of PING_MGR_SetCtlAdminStatus() */

/* FUNCTION NAME: PING_MGR_SetCtlRowStatus
 * PURPOSE:
 *          To set row status field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the  specific control entry.
 *          ctrl_row_status -- the row status of the specified control entry.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_NO_MORE_WORKSPACE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. The PingCtlEntry should not be modified. So we create a local entry for local use.
 */
UI32_T  PING_MGR_SetCtlRowStatus(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T ctrl_row_status)
{
     /* Local Variable Declaration
     */
    UI32_T                      res;
    PING_TYPE_PingCtlEntry_T    local_entry; /* local use */

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    if ((ctrl_row_status < VAL_pingCtlRowStatus_active) ||
        (ctrl_row_status > VAL_pingCtlRowStatus_destroy))
    {
        return PING_TYPE_INVALID_ARG;
    }

    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    memcpy(&local_entry.ping_ctl_owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(&local_entry.ping_ctl_test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    local_entry.ping_ctl_owner_index_len = ctl_entry_p->ping_ctl_owner_index_len;
    local_entry.ping_ctl_test_name_len = ctl_entry_p->ping_ctl_test_name_len;

    if ((res = PING_OM_IsPingCtlEntryExist(ctl_entry_p)) != PING_TYPE_OK)
    {
        /* this switch only initialize control entry */
        switch (ctrl_row_status)
        {
            case VAL_pingCtlRowStatus_createAndWait:
            case VAL_pingCtlRowStatus_createAndGo:
                /* Init default value to control entry
                 */
                PING_MGR_InitControlEntry(&local_entry);
                local_entry.ping_ctl_rowstatus = ctrl_row_status;

                if ((res = PING_OM_SetCtlEntry(&local_entry)) != PING_TYPE_OK)
                    return PING_TYPE_FAIL;

                break;
            default:
                res = PING_TYPE_FAIL;
                break;
        } /* end of switch */
    }
    else
    {
        /* update entry value */
        if ((res = PING_OM_GetCtlEntry(&local_entry)) != PING_TYPE_OK)
        {
            return PING_TYPE_FAIL;
        }
        local_entry.ping_ctl_rowstatus = ctrl_row_status;
        res = PING_OM_SetCtlEntry(&local_entry);
    }

    PING_MGR_CheckToStartTimer();

    return res;
} /* end of PING_MGR_SetCtlRowStatus() */

/* FUNCTION NAME: PING_MGR_SetCtlTargetAddress
 * PURPOSE:
 *          To set the target address field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          target_addr     -- the target address of the remote host.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the target address for the specified index when admin_status is enabled.
 *          2. Currently we do not support the domain name query of the target address.
 */
UI32_T PING_MGR_SetCtlTargetAddress(PING_TYPE_PingCtlEntry_T *ctl_entry_p, L_INET_AddrIp_T* target_addr_p)
{
    /* Local Variable Declaration
     */
    UI32_T                      res;
    PING_TYPE_PingCtlEntry_T    local_entry;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }
    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    /*if(target_addr == 0)
    {
        return PING_TYPE_INVALID_ARG;
    }
    */
    /*if (IP_LIB_UI32toArray(target_addr, conv_addr) != IP_LIB_OK)
    {
        return PING_TYPE_FAIL;
    }
    */
    DEBUG_PRINTF("%s, type:%d\n", __FUNCTION__, target_addr_p->type);

    switch(target_addr_p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            /* IPv4 address checking */
            if(!memcmp(target_addr_p->addr, ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN))
                return PING_TYPE_INVALID_ARG;

            if(IP_LIB_IsTestingIp(target_addr_p->addr)
                || IP_LIB_IsBroadcastIp(target_addr_p->addr)
                || IP_LIB_IsMulticastIp(target_addr_p->addr)
                || IP_LIB_IsZeroNetwork(target_addr_p->addr))
            {
                return PING_TYPE_INVALID_ARG;
            }


            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
            /* IPv6 address checking */
            if(!memcmp(target_addr_p->addr, ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN))
                return PING_TYPE_INVALID_ARG;
            if(!IP_LIB_IsIPv6Multicast(target_addr_p->addr) && (0 != target_addr_p->zoneid))
                return PING_TYPE_INVALID_ARG;
            break;
        case L_INET_ADDR_TYPE_IPV6Z:
            /* IPv6 address checking */
            if(!memcmp(target_addr_p->addr, ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN))
                return PING_TYPE_INVALID_ARG;
            if(!L_INET_ADDR_IS_VALID_IPV6_LINK_LOCAL_ZONE_ID(target_addr_p->zoneid))
                return PING_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID;
            break;
#endif
        default:
            return PING_TYPE_INVALID_ARG;
    }


    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    memcpy(&local_entry.ping_ctl_owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(&local_entry.ping_ctl_test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_GetCtlEntry(&local_entry)) != PING_TYPE_OK)
        return PING_TYPE_FAIL;

    /* Can not change the target address when admin_status == enable. */
    if ( local_entry.ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
    {
        return PING_TYPE_FAIL;
    }
    local_entry.ping_ctl_target_address = *target_addr_p;
    res = PING_MGR_SetCtlEntryByField(&local_entry, PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS);

    return res;

} /* end of PING_MGR_SetCtlTargetAddress() */

/* FUNCTION NAME: PING_MGR_SetCtlDataSize
 * PURPOSE:
 *          To set the data size field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          data_size       -- the size of data portion in ICMP pkt.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the data size for the specified index when admin_status is enabled.
 *          2. SNMP range: 0..65507, CLI range: 32-512.
 */
UI32_T PING_MGR_SetCtlDataSize(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T data_size)
{
    /* Local Variable Declaration
     */
    UI32_T                      res;
    PING_TYPE_PingCtlEntry_T    local_entry;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }



    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
    memcpy(&local_entry.ping_ctl_owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(&local_entry.ping_ctl_test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_GetCtlEntry(&local_entry)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }
    /* Can not change the value when admin_status == enable. */
    if ( local_entry.ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
    {
        return PING_TYPE_FAIL;
    }
    local_entry.ping_ctl_data_size = data_size;
    res = PING_MGR_SetCtlEntryByField(&local_entry, PING_TYPE_CTLENTRYFIELD_DATA_SIZE);

    return res;
}

/* FUNCTION NAME: PING_MGR_SetCtlProbeCount
 * PURPOSE:
 *          To set the probe count field for ping control entry
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *          probe_count     -- the number of ping packet
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. We can not change the probe count for the specified index when admin_status is enabled.
 *          2. SNMP range: 1-15, CLI range: 1-16. So 0 is not allowed.
 */
UI32_T PING_MGR_SetCtlProbeCount(PING_TYPE_PingCtlEntry_T *ctl_entry_p, UI32_T probe_count)
{
    /* Local Variable Declaration
     */
    UI32_T                      res;
    PING_TYPE_PingCtlEntry_T    local_entry;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    if(probe_count == 0)
    {
        return PING_TYPE_INVALID_ARG;
    }


    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
    memcpy(&local_entry.ping_ctl_owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(&local_entry.ping_ctl_test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_GetCtlEntry(&local_entry)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }
    /* Can not change the value when admin_status == enable. */
    if ( local_entry.ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
    {
        return PING_TYPE_FAIL;
    }
    local_entry.ping_ctl_probe_count = probe_count;
    res = PING_MGR_SetCtlEntryByField(&local_entry, PING_TYPE_CTLENTRYFIELD_PROBE_COUNT);

    return res;
}

/* FUNCTION NAME: PING_MGR_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 *          PING_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T PING_MGR_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field)
{
    /* Local Variable Declaration
     */
    UI32_T                      res;
    PING_TYPE_PingCtlEntry_T    local_entry;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return PING_TYPE_FAIL;
    }

    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
    memcpy(&local_entry, ctl_entry_p, sizeof(local_entry));

    if (PING_TYPE_OK != (res = PING_OM_GetCtlEntry(&local_entry)))
        return res;

    /* Can not change the value when admin_status == enable. */
    if(local_entry.ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
    {
        return PING_TYPE_FAIL;
    }
    switch (field)
    {
        case PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE:
            local_entry.ping_ctl_target_address_type = ctl_entry_p->ping_ctl_target_address_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS:
            local_entry.ping_ctl_target_address =  ctl_entry_p->ping_ctl_target_address;
            break;
        case PING_TYPE_CTLENTRYFIELD_DATA_SIZE:
            local_entry.ping_ctl_data_size = ctl_entry_p->ping_ctl_data_size;
            break;
        case PING_TYPE_CTLENTRYFIELD_TIMEOUT:
            local_entry.ping_ctl_timeout = ctl_entry_p->ping_ctl_timeout;
            break;
        case PING_TYPE_CTLENTRYFIELD_PROBE_COUNT:
            local_entry.ping_ctl_probe_count = ctl_entry_p->ping_ctl_probe_count;
            break;
        case PING_TYPE_CTLENTRYFIELD_ADMIN_STATUS:
            local_entry.ping_ctl_admin_status = ctl_entry_p->ping_ctl_admin_status;
            break;
        case PING_TYPE_CTLENTRYFIELD_DATA_FILL:
            memcpy(local_entry.ping_ctl_data_fill, ctl_entry_p->ping_ctl_data_fill, sizeof(ctl_entry_p->ping_ctl_data_fill));
            break;
        case PING_TYPE_CTLENTRYFIELD_FREQUENCY:
            local_entry.ping_ctl_admin_status = ctl_entry_p->ping_ctl_admin_status;
            break;
        case PING_TYPE_CTLENTRYFIELD_MAX_ROWS:
            local_entry.ping_ctl_max_rows = ctl_entry_p->ping_ctl_max_rows;
            break;
        case PING_TYPE_CTLENTRYFIELD_STORAGE_TYPE:
            local_entry.ping_ctl_storage_type = ctl_entry_p->ping_ctl_storage_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_GENERATION:
            local_entry.ping_ctl_trap_generation = ctl_entry_p->ping_ctl_trap_generation;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_PROBE_FAILURE_FILTER:
            local_entry.ping_ctl_trap_probe_failure_filter = ctl_entry_p->ping_ctl_trap_probe_failure_filter;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_TEST_FAILURE_FILTER:
            local_entry.ping_ctl_trap_test_failure_filter = ctl_entry_p->ping_ctl_trap_test_failure_filter;
            break;
        case PING_TYPE_CTLENTRYFIELD_TYPE:
            local_entry.ping_ctl_type= ctl_entry_p->ping_ctl_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_DESCR:
            memcpy(local_entry.ping_ctl_descr, ctl_entry_p->ping_ctl_descr, sizeof(ctl_entry_p->ping_ctl_descr));
            break;
        case PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS_TYPE:
            local_entry.ping_ctl_source_address_type = ctl_entry_p->ping_ctl_source_address_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS:
            local_entry.ping_ctl_source_address = ctl_entry_p->ping_ctl_source_address;
            break;
        case PING_TYPE_CTLENTRYFIELD_IF_INDEX:
            local_entry.ping_ctl_if_index = ctl_entry_p->ping_ctl_if_index;
            break;
        case PING_TYPE_CTLENTRYFIELD_BY_PASS_ROUTE_TABLE:
            local_entry.ping_ctl_by_pass_route_table = ctl_entry_p->ping_ctl_by_pass_route_table;
            break;
        case PING_TYPE_CTLENTRYFIELD_DS_FIELD:
            local_entry.ping_ctl_ds_field = ctl_entry_p->ping_ctl_ds_field;
            break;
        case PING_TYPE_CTLENTRYFIELD_DONT_FRAGMENT:
            local_entry.ping_ctl_dont_fragment = ctl_entry_p->ping_ctl_dont_fragment;
            break;
        case PING_TYPE_CTLENTRYFIELD_ROWSTATUS:
            /* this switch only initialize control entry */
            switch (ctl_entry_p->ping_ctl_rowstatus)
            {
                case VAL_pingCtlRowStatus_createAndWait:
                case VAL_pingCtlRowStatus_createAndGo:
                    /* Init default value to control entry
                     */
                    PING_MGR_InitControlEntry(&local_entry);
                    local_entry.ping_ctl_rowstatus = ctl_entry_p->ping_ctl_rowstatus;
                    break;
                default:
                    local_entry.ping_ctl_rowstatus = ctl_entry_p->ping_ctl_rowstatus;
                    break;
            }
            break;
        default :
            if(TRUE == ping_backdoor_debug)
            {
                BACKDOOR_MGR_Printf("sorry, not support to change this field yet.\n");
            }
            return PING_TYPE_FAIL;
    } /* switch */

    res = PING_OM_SetCtlEntry(&local_entry);

    PING_MGR_CheckToStartTimer();

    return res;
}

/* FUNCTION NAME : PING_MGR_CreateWorkSpace
 * PURPOSE:
 *      Create workspace for routing path from src to dst.
 *
 * INPUT:
 *      dst_ip          -- probe target IP address.
 *      src_ip          -- interface IP address which send out probe packet.
 *      workspace_index -- location of this entry
 * OUTPUT:
 *      None.
 * RETURN:
 *      PING_TYPE_OK  -- successfully create the workspace.
 *      PING_TYPE_INVALID_ARG -- src_ip or dst_ip is invalid value.
 *      PING_TYPE_NO_MORE_WORKSPACE
 *      PING_TYPE_NO_MORE_SOCKET
 *      PING_TYPE_NO_MORE_ENTRY
 *      PING_TYPE_FAIL
 * NOTES:
 *      1. If same task create workspace twice, cause previous one auto-free.
 */
UI32_T PING_MGR_CreateWorkSpace(UI32_T workspace_index , PING_TYPE_PingCtlEntry_T *ctl_entry_p)
{

    UI32_T tid = SYSFUN_TaskIdSelf();
    UI32_T ifindex;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return PING_TYPE_FAIL;

    if (ping_workspace[workspace_index].owner_tid != 0)
        return PING_TYPE_FAIL;

    DEBUG_PRINTF("%s: creating workspace index: %ld\n", __FUNCTION__, (long)workspace_index);

    /* Init workspace
     */
    memset(&ping_workspace[workspace_index].outpacket, 0, sizeof(struct opacket));
    memset(&ping_workspace[workspace_index].whereto, 0, sizeof(struct sockaddr_in));
#if (SYS_CPNT_IPV6 == TRUE)
    memset(&ping_workspace[workspace_index].outpacket6, 0, sizeof(struct opacket6));
    memset(&ping_workspace[workspace_index].whereto6, 0, sizeof(struct sockaddr_in6));
#endif
    //memset(&ping_workspace[workspace_index].wherefrom,0,sizeof(struct sockaddr_in));

    ping_workspace[workspace_index].owner_tid  = (tid + workspace_index);

    switch(ctl_entry_p->ping_ctl_target_address_type)
    {
        // maybe we should use  ... VAL_pingCtlSourceAddressType_ipv4
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            ping_workspace[workspace_index].target_addr_type = ctl_entry_p->ping_ctl_target_address_type;
            memcpy(&ping_workspace[workspace_index].src_ip, &ctl_entry_p->ping_ctl_source_address, sizeof(L_INET_AddrIp_T));
            memcpy(&ping_workspace[workspace_index].dst_ip, &ctl_entry_p->ping_ctl_target_address, sizeof(L_INET_AddrIp_T));

            /* socket_in */
            ping_workspace[workspace_index].whereto.sin_family = AF_INET;
            memcpy((UI8_T *) &(ping_workspace[workspace_index].whereto.sin_addr.s_addr), ctl_entry_p->ping_ctl_target_address.addr, SYS_ADPT_IPV4_ADDR_LEN);

            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            ping_workspace[workspace_index].target_addr_type = ctl_entry_p->ping_ctl_target_address_type;
            memcpy(&ping_workspace[workspace_index].src_ip, &ctl_entry_p->ping_ctl_source_address, sizeof(L_INET_AddrIp_T));
            memcpy(&ping_workspace[workspace_index].dst_ip, &ctl_entry_p->ping_ctl_target_address, sizeof(L_INET_AddrIp_T));

            ping_workspace[workspace_index].whereto6.sin6_family = AF_INET6;
            memcpy((UI8_T *) &(ping_workspace[workspace_index].whereto6.sin6_addr.s6_addr), ctl_entry_p->ping_ctl_target_address.addr, SYS_ADPT_IPV6_ADDR_LEN);

            /* kernel's scope_id uses ifindex */
            ifindex = L_INET_ZONE_ID_TO_IFINDEX(ctl_entry_p->ping_ctl_target_address.zoneid);
            ping_workspace[workspace_index].whereto6.sin6_scope_id = ifindex;

            /* ipv6 mcast target address */
            DEBUG_PRINTF("%x %x %x %x :: %x\n",
            ctl_entry_p->ping_ctl_target_address.addr[0],
            ctl_entry_p->ping_ctl_target_address.addr[1],
            ctl_entry_p->ping_ctl_target_address.addr[2],
            ctl_entry_p->ping_ctl_target_address.addr[3],
            ctl_entry_p->ping_ctl_target_address.addr[15]);
            ping_workspace[workspace_index].mcast_ipv6_target_addr = IP_LIB_IsIPv6Multicast(ctl_entry_p->ping_ctl_target_address.addr);
            if (ping_workspace[workspace_index].mcast_ipv6_target_addr == TRUE)
                DEBUG_PRINTF("mcast_ipv6_target_addr is TRUE\n");
            break;
#endif
        default:
            return PING_TYPE_INVALID_ARG;
    }

    ping_workspace[workspace_index].ident      = 0x8000 | ((tid+workspace_index) & 0xffff);

    ping_workspace[workspace_index].last_transmit_time = 0;

    ping_workspace[workspace_index].datalen = ctl_entry_p->ping_ctl_data_size;
    ping_workspace[workspace_index].probe_count = ctl_entry_p->ping_ctl_probe_count;
    ping_workspace[workspace_index].dont_fragment = ctl_entry_p->ping_ctl_dont_fragment;
    ping_workspace[workspace_index].sent_probes = 0;
    ping_workspace[workspace_index].received_probs =0;
    ping_workspace[workspace_index].unreceived_probs = 0;
    ping_workspace[workspace_index].timeout_second = ctl_entry_p->ping_ctl_timeout;
    ping_workspace[workspace_index].probe_history_last_index = 0;
    ping_workspace[workspace_index].last_send_status = 0;
    return PING_TYPE_OK;

} /* end of PING_MGR_CreateWorkSpace() */


/* FUNCTION NAME : PING_MGR_FreeWorkSpace
 * PURPOSE:
 *      Release working space to ping utility.
 *
 * INPUT:
 *      workspace_index -- the starting address of workspace handler.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      PING_TYPE_OK - the space is return to utility.
 *      PING_TYPE_INVALID_WORK_SPACE - the pointer is no valid pointer of working space,
 *                                     maybe not owned by this task.
 *
 * NOTES:
 *      1. After free workspace, handler will set to NULL.
 */
UI32_T PING_MGR_FreeWorkSpace(UI32_T workspace_index)
{
    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return PING_TYPE_FAIL;

//    memset(&ping_workspace[workspace_index].outpacket, 0, sizeof(struct opacket));
    memset(&ping_workspace[workspace_index].whereto,0,sizeof(struct sockaddr_in));
#if (SYS_CPNT_IPV6 == TRUE)
    memset(&ping_workspace[workspace_index].whereto6,0,sizeof(struct sockaddr_in6));
#endif
    //memset(&ping_workspace[workspace_index].wherefrom,0,sizeof(struct sockaddr_in));

    ping_workspace[workspace_index].owner_tid  = 0;

    ping_workspace[workspace_index].target_addr_type = L_INET_ADDR_TYPE_UNKNOWN;
    memset(&ping_workspace[workspace_index].src_ip, 0, sizeof(L_INET_AddrIp_T));
    memset(&ping_workspace[workspace_index].dst_ip, 0, sizeof(L_INET_AddrIp_T));

    ping_workspace[workspace_index].ident      = 0;
    ping_workspace[workspace_index].datalen    = 0;

    ping_workspace[workspace_index].probe_count    = 0;
    ping_workspace[workspace_index].sent_probes     = 0;

    ping_workspace[workspace_index].received_probs      = 0;
    ping_workspace[workspace_index].unreceived_probs    = 0;
    ping_workspace[workspace_index].last_transmit_time  = 0;
    ping_workspace[workspace_index].last_receive_time   = 0;
    ping_workspace[workspace_index].last_access_time    = 0;

    /* ipv6 mcast target address */
    ping_workspace[workspace_index].mcast_ipv6_target_addr = FALSE;

    ping_workspace[workspace_index].probe_history_last_index = 0;

    return PING_TYPE_OK;

}   /* end of PING_MGR_FreeWorkSpace */

/* FUNCTION NAME: PING_MGR_SetWorkSpaceAdminStatus
 * PURPOSE:
 *          Sync admin status in the work space.
 * INPUT:
 *          table_index -- table index to be synchronize.
 *          status      -- {VAL_pingCtlAdminStatus_disabled|VAL_pingCtlAdminStatus_enabled}.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_INVALID_ARG
 *          PING_TYPE_FAIL
 * NOTES:
 *
 */
UI32_T PING_MGR_SetWorkSpaceAdminStatus(UI32_T table_index, UI32_T status)
{
    /* BODY
     */
    UI32_T res;

    switch(status)
    {
        case VAL_pingCtlAdminStatus_disabled:
            ping_workspace[table_index].admin_status = VAL_pingCtlAdminStatus_disabled;
            res = PING_TYPE_OK;
            break;

        case VAL_pingCtlAdminStatus_enabled:
            ping_workspace[table_index].admin_status = VAL_pingCtlAdminStatus_enabled;
            res = PING_TYPE_OK;
            break;
        default:
            res = PING_TYPE_FAIL;
    }
    return res;

} /* end of PING_MGR_SetWorkSpaceAdminStatus() */

/* LOCAL SUBPROGRAM IMPLEMENTATION
 */

/* FUNCTION NAME : PING_MGR_SendProbe
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
 *      PING_TYPE_OK  \
 *      PING_TYPE_DO_NOT_SEND \
 *      PING_TYPE_FAIL
 *
 * NOTES:
 *      None
 */
static UI32_T PING_MGR_SendProbe(void)
{
    struct      opacket     *op;
#if (SYS_CPNT_IPV6 == TRUE)
    struct      opacket6     *op6;
#endif
    /* struct      sockaddr_in *from; */
    PING_SORTLST_ELM_T    list_elm;
    int         send_length = 0;
    UI32_T      active_index = 0;
    UI32_T      res, current_time, seq=0;
    /* int         socket_rc; */
    UI32_T      pkt_len;
    UI32_T      owner_index_len, test_name_len;
    PING_TYPE_PingProbeHistoryEntry_T   prob_history_entry;
    /* UI8_T       zero_ip_addr[SYS_ADPT_IPV4_ADDR_LEN] = {0}; */
    int i;

    memset(&prob_history_entry, 0, sizeof(PING_TYPE_PingProbeHistoryEntry_T));
    memset(&global_result_entry, 0, sizeof(PING_TYPE_PingResultsEntry_T));
    memset(&global_control_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    if ((res = PING_OM_GetFirstActiveResultsEntry(&active_index, &global_result_entry)) != PING_TYPE_OK)
    {
        return PING_TYPE_NO_MORE_PROBE_TO_SEND;
    }

    do
    {
        /* check if already send all prob pkts and stop it. */

        /* DEBUG */
        DEBUG_PRINTF("%s: active_index: %ld\n", __FUNCTION__, (long)active_index);

        current_time = SYSFUN_GetSysTick();

        /* Set prob history entry.
         */

        /* 1 Perform checks for timing issue
         */
        if ((ping_workspace[active_index].last_transmit_time != 0)
        && (ping_workspace[active_index].probe_history_last_index < ping_workspace[active_index].sent_probes))
        {
            if ((current_time - ping_workspace[active_index].last_transmit_time >=
                     SYS_DFLT_PING_CTL_TIME_OUT))
            {
                ping_workspace[active_index].unreceived_probs++;
                //ping_workspace[active_index].last_transmit_time = 0;
                //ping_workspace[active_index].last_receive_time = 0;

                if ((res = PING_OM_PingTableIndexToKey(active_index, &list_elm)) !=  PING_TYPE_OK)
                {
                    continue;
                }
                /* get owner_index_len, test_name_len for history entry! */
                memcpy(global_control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(global_control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                if ((res = PING_OM_GetCtlEntry(&global_control_entry)) != PING_TYPE_OK)
                {
                    continue;
                }
                owner_index_len = global_control_entry.ping_ctl_owner_index_len;
                test_name_len = global_control_entry.ping_ctl_test_name_len;

                memcpy(prob_history_entry.ping_ctl_owner_index,list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(prob_history_entry.ping_ctl_test_name,list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                prob_history_entry.ping_ctl_owner_index_len = owner_index_len; /* MUST! */
                prob_history_entry.ping_ctl_test_name_len = test_name_len; /* MUST! */

                prob_history_entry.icmp_sequence = ping_workspace[active_index].sent_probes;
                /* If pkt is time outed, the response is the time-out value in milliseconds */
                prob_history_entry.ping_probe_history_response = 1000 * ping_workspace[active_index].timeout_second;
                if(ping_workspace[active_index].last_send_status) /* status has been assigned by sento() */
                {

                    prob_history_entry.ping_probe_history_status = VAL_pingProbeHistoryStatus_internalError;
                }
                else
                {
                    prob_history_entry.ping_probe_history_status = VAL_pingProbeHistoryStatus_requestTimedOut;
                }
                prob_history_entry.ping_probe_history_last_rc = PING_TYPE_NO_RESPONSE;
                SYS_TIME_GetDayAndTime(prob_history_entry.ping_probe_history_time,
                                       &prob_history_entry.ping_probe_history_time_len);
                prob_history_entry.system_tick = SYSFUN_GetSysTick();

                DEBUG_PRINTF("packet is timeout, seq: %ld\n", (long)prob_history_entry.icmp_sequence);

                if (PING_TYPE_OK != PING_OM_AppendProbePacketResult(active_index, &prob_history_entry))
                {
                    DEBUG_PRINTF("append prob history failed! seq: %ld\n", (long)prob_history_entry.icmp_sequence);
                    continue;
                }

                ping_workspace[active_index].probe_history_last_index = ping_workspace[active_index].sent_probes;

                /* stop sending prob pkt */
                if ((res = PING_OM_PingTableIndexToKey(active_index, &list_elm)) !=  PING_TYPE_OK)
                {
                    continue;
                }
                memcpy(global_control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(global_control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                if (PING_TYPE_OK != PING_OM_GetCtlEntry(&global_control_entry))
                {
                    DEBUG_PRINTF("get ctl entry failed!\n");
                    continue;
                }
                if(ping_workspace[active_index].mcast_ipv6_target_addr == FALSE) /* unicast */
                {
                        if(ping_workspace[active_index].sent_probes >= ping_workspace[active_index].probe_count)
                        {
                            global_control_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
                            ping_workspace[active_index].admin_status = VAL_pingCtlAdminStatus_disabled;

                            if (PING_TYPE_OK != PING_OM_SetCtlEntry(&global_control_entry))
                            {
                                DEBUG_PRINTF("set ctl entry failed!\n");
                                continue;
                            }
                            continue;
                        }
                } /* if mcast */
            }
            else
            {
                /* Get next entry because timer hasn't expires for this one.
                 */
                continue;
            }
        } /* end of if */

        if(ping_workspace[active_index].sent_probes >= ping_workspace[active_index].probe_count)
        {
            continue;
        }

        /* 2  Send prob packet
         */
/*        if (++global_result_entry.ping_results_cur_probe_count >
             SYS_DFLT_PING_CTL_PROBES_PER_HOP)
        {
            global_result_entry.ping_results_cur_probe_count = 1;
            global_result_entry.ping_results_cur_hop_count++;
        }

        seq += PING_MGR_DESTUDPPORT;
        ttl = global_result_entry.ping_results_cur_hop_count;
*/

        /* if src ip is not specified by user, then it won't be determinined here but by lower layer. */
        /*if (ping_backdoor_debug == TRUE)
        {
            //printf("src ip: 0x%lx\n", (UI32_T)ping_workspace[active_index].wherefrom.sin_addr.s_addr);
        }*/
#if 0 // mark-out to test automatic src ip, suggest by max.
        /* memcmp with zero_ip_addr */
        if(memcmp((UI8_T *) &(ping_workspace[active_index].wherefrom.sin_addr.s_addr), zero_ip_addr, SYS_ADPT_IPV4_ADDR_LEN))
        {
            DEBUG_PRINTF("binding src ip.\n");

            from = &(ping_workspace[active_index].wherefrom);
            if ((socket_rc = bind (ping_send_socket,(struct sockaddr *)from,
                    sizeof(struct sockaddr_in))) < 0)
            {
                DEBUG_PRINTF("Bind socket Fails on active index %d\n", (int)active_index);
                /* mark-outed by peter,
                 * when rif is down, binding will fails but
                 * we should still continue to inicrease counter then finished the ping process.
                 */
                /* break; */
            }
        }
#endif
        /* Send using socket
         */

        seq = global_result_entry.ping_results_sent_probes + 1; /* seq start from 1 */

        switch(ping_workspace[active_index].target_addr_type)
        {
            // maybe we should use  ... VAL_pingCtlSourceAddressType_ipv4
            case L_INET_ADDR_TYPE_IPV4:
            case L_INET_ADDR_TYPE_IPV4Z:

                /* Fill in outpacket content
                 */


                op = &(ping_workspace[active_index].outpacket);

                /* icmp header */
                op->icmp.icmp_type = 0x08; /* ICMP_ECHO */
                op->icmp.icmp_code = 0;
                op->icmp.icmp_csum = 0;
                op->icmp.icmp_id   = (UI16_T) ((ping_workspace[active_index].ident) & 0xffff);
                op->icmp.icmp_seq  = seq;
                pkt_len = sizeof(PING_MGR_ICMP_T) + ping_workspace[active_index].datalen;

                /* checksum */
                op->icmp.icmp_csum = ipchksum((unsigned short *)&op->icmp, pkt_len);

                if ((ping_send_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP))<0)
                {
                    ping_send_socket = 0;
                    return PING_TYPE_NO_MORE_SOCKET;
                }

                if(ping_workspace[active_index].dont_fragment == TRUE)
                {
                    int val = IP_PMTUDISC_DO; /* Always DF */
                    if(setsockopt(ping_send_socket, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val))<0)
                    {
                        DEBUG_PRINTF("Failed to set dont fragment bit in ip header.\r\n");
                        return PING_TYPE_FAIL;
                    }
                }

                DEBUG_PRINTF("%s, %d, sending pkt to addr: ",__FUNCTION__, __LINE__);
                DEBUG_PRINTF("0x%lx", (long int) ping_workspace[active_index].whereto.sin_addr.s_addr);
                DEBUG_PRINTF("%s, %d, seq: %ld\n",__FUNCTION__, __LINE__, (long)seq);

                send_length = sendto (ping_send_socket,
                                      (UI8_T *)op, pkt_len, 0,
                                      (struct sockaddr*)&(ping_workspace[active_index].whereto),
                                      sizeof(ping_workspace[active_index].whereto));

                break;

#if (SYS_CPNT_IPV6 == TRUE)
            case L_INET_ADDR_TYPE_IPV6:
            case L_INET_ADDR_TYPE_IPV6Z:
                /* Fill in outpacket content
                 */
                op6 = &(ping_workspace[active_index].outpacket6);

                /* icmp header */
                op6->icmp6.type = 128; /* ICMPv6 echo request */
                op6->icmp6.code =0;
                op6->icmp6.checksum = 0;
                op6->icmp6.id = (UI16_T) ((ping_workspace[active_index].ident) & 0xffff);
                op6->icmp6.seq = seq;
                pkt_len = sizeof(PING_MGR_ICMP6_T) + ping_workspace[active_index].datalen;

                /* checksum */
                op6->icmp6.checksum = ipchksum((unsigned short *)&op6->icmp6, pkt_len);

                if ((ping6_send_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6))<0)
                {
                    ping6_send_socket = 0;
                    DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "create ping6_send_socket failed.");
                    return PING_TYPE_NO_MORE_SOCKET;
                }

                if(ping_backdoor_debug == TRUE)
                {
                    BACKDOOR_MGR_Printf("%s, %d, sending pkt to addr: ",__FUNCTION__, __LINE__);
                    for(i=0;i<SYS_ADPT_IPV6_ADDR_LEN;i++)
                    {
                        BACKDOOR_MGR_Printf("%02x:",ping_workspace[active_index].whereto6.sin6_addr.s6_addr[i]);
                    }
                    BACKDOOR_MGR_Printf("\r\nscope_id:%d\r\n",ping_workspace[active_index].whereto6.sin6_scope_id);

                    BACKDOOR_MGR_Printf("%s, %d, seq: %ld\n",__FUNCTION__, __LINE__, (long)seq);
                }
                if(IP_LIB_IsIPv6Multicast(ping_workspace[active_index].whereto6.sin6_addr.s6_addr))
                {
                    UI32_T ifindex = ping_workspace[active_index].whereto6.sin6_scope_id;
                    DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "setsockopt(IPV6_MULTICAST_IF)." );
                    if(setsockopt(ping6_send_socket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex)) < 0)
                    {
                        DEBUG_PRINTF("%s, %d, %s\n", __FUNCTION__, __LINE__, "setsockopt(IPV6_MULTICAST_IF) failed." );
                        return PING_TYPE_FAIL;
                    }
                }

                send_length = sendto (ping6_send_socket,
                                      (UI8_T *)op6, pkt_len, 0,
                                      (struct sockaddr*)&(ping_workspace[active_index].whereto6),
                                      sizeof(ping_workspace[active_index].whereto6));
                break;
#endif
            default:
                break;//return PING_TYPE_INVALID_ARG;
        } /* switch */

        DEBUG_PRINTF("%s, %d, send_length: %d\n", __FUNCTION__, __LINE__, send_length);


        if (send_length < 0)
        {
            /* modified by peter_yu, we should continue to increase sent_probes counter.
              return PING_TYPE_FAIL;
             */
            DEBUG_PRINTF("%s, %d, sendto() failed, errno=%d.\r\n", __FUNCTION__, __LINE__, errno);
            if(errno == EMSGSIZE)
            {
                ping_workspace[active_index].last_send_status = VAL_pingProbeHistoryStatus_internalError;
            }
        }
        if (ping_send_socket != 0)
        {
            close(ping_send_socket);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        if (ping6_send_socket != 0)
        {
            close(ping6_send_socket);
        }
#endif
        ping_workspace[active_index].last_transmit_time = SYSFUN_GetSysTick();
        ping_workspace[active_index].sent_probes++;

        DEBUG_PRINTF("%s, %d, transmit_time: %ld\n", __FUNCTION__, __LINE__, (long)ping_workspace[active_index].last_transmit_time);

        /* update result entry
         */
        global_result_entry.ping_results_sent_probes++;

        /* initialize result entry min, max rtt */
        if(global_result_entry.ping_results_sent_probes ==1)
        {
            global_result_entry.ping_results_min_rtt = INIT_MIN_DELTA_TIME;
            global_result_entry.ping_results_max_rtt = INIT_MAX_DELTA_TIME;
        }
        if (PING_TYPE_OK != PING_OM_SetResultsEntry(active_index, &global_result_entry))
        {
            DEBUG_PRINTF("set result entry fail!\n");
            continue;
        }
    } while((res = PING_OM_GetNextActiveResultsEntry(&active_index, &global_result_entry)) == PING_TYPE_OK);
    /* end of while */
    return PING_TYPE_OK;

} /* end of PING_MGR_SendProbe() */

/* FUNCTION NAME : PING_MGR_WaitForReply
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
 *      PING_TYPE_OK       -- receive packet.
 *      PING_TYPE_FAIL     -- can't receive
 *      PING_TYPE_TIMEOUT  -- exceed max waiting time.
 *
 * NOTES:
 *      1. Due to for SYSFUN_GetSysTick, the unit of tick is 0.01 sec, ie. 10 ms. depending on system configuration.
 *         But the unit for ping_probe_history_response is milliseconds.
 */
static UI32_T PING_MGR_WaitForReply(void)
{
    /* Local Variable Declaration
     */
    struct      timeval wait;
    PING_SORTLST_ELM_T    list_elm;
    int         cc = 0;
    fd_set      fds;
    UI32_T      active_index, res;
    UI8_T       rcv_buf[PING_MGR_RCV_BUF_SIZE];
    BOOL_T      active_index_found = FALSE;
    UI16_T      icmp_id, icmp_seq;
    UI32_T      owner_index_len=0, test_name_len=0;

    PING_TYPE_PingProbeHistoryEntry_T       prob_history_entry;

    UI32_T response_time, receive_time_stamp;

    // v4
    struct      sockaddr_in fromp;
    int         fromlen = sizeof(struct sockaddr_in);
    ip_packet_T  *icp;

#if (SYS_CPNT_IPV6 == TRUE)
    struct      sockaddr_in6 from6;
    int         fromlen6 = sizeof(struct sockaddr_in6);
    PING_MGR_ICMP6_T *icmp6;
#endif
    L_INET_AddrIp_T inet_addr; /* from inet address */
    char tmp_buf[MAX_BUF_LEN] = {0};
    UI32_T max_sockfd;

    /* BODY */

    FD_ZERO(&fds);

    FD_SET(ping_receive_socket, &fds);
#if (SYS_CPNT_IPV6 == TRUE)
    FD_SET(ping6_receive_socket, &fds);
#endif
    memset(&global_result_entry, 0, sizeof(PING_TYPE_PingResultsEntry_T));
    memset(&global_control_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

#if (SYS_CPNT_IPV6 == TRUE)
    max_sockfd = (ping6_receive_socket > ping_receive_socket) ? ping6_receive_socket: ping_receive_socket;
#else
    max_sockfd = ping_receive_socket;
#endif
    while (waiting_time > 0) /* waiting_time, unit: 1 tick (10 ms) */
    {

        for (active_index = 0; active_index < SYS_ADPT_MAX_PING_NUM; active_index++)
        {
            if(ping_workspace[active_index].mcast_ipv6_target_addr == FALSE)
                continue;

            if((ping_workspace[active_index].last_transmit_time !=0) && (SYSFUN_GetSysTick() - ping_workspace[active_index].last_transmit_time >=
                     SYS_DFLT_PING_CTL_TIME_OUT))
            {

                /* DEBUG */
                DEBUG_PRINTF("going to disable admin_status.(1)\n");

                /* disable admin_status and stop waiting response */

                if ((res = PING_OM_PingTableIndexToKey(active_index, &list_elm)) !=  PING_TYPE_OK)
                {
                    continue;
                }
                memcpy(global_control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(global_control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                if ((res = PING_OM_GetCtlEntry(&global_control_entry)) != PING_TYPE_OK)
                {
                    /* re-calculate waiting_time and wait again */
                    waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                    continue;
                }

                ping_workspace[active_index].admin_status = VAL_pingCtlAdminStatus_disabled;

                global_control_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;

                PING_OM_SetCtlEntry(&global_control_entry);
            }
            /* else continue */
        } /* for */

        wait.tv_sec = 0;
        wait.tv_usec = 10000* waiting_time ; /* unit: microsecond, 1 tick = 10 ms = 10^4 microsecond */

        /*if(TRUE == ping_backdoor_debug)
        {
            //printf("%s, %d, wait.tv_usec: %d \n", __FUNCTION__, __LINE__, (int) wait.tv_usec);
        }*/

        /* rest socket address */
        memset(&fromp, 0, sizeof(fromp));
#if (SYS_CPNT_IPV6 == TRUE)
        memset(&from6, 0, sizeof(from6));
#endif

        /* socket processing */
        if (select(max_sockfd + 1, (fd_set*)&fds, NULL, NULL, &wait) <= 0)
        {
            /* rc == 0 is time-out event, rc < 0 is interrupt event,
             * re-calculate waiting_time and wait again
             */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }

        if(FD_ISSET(ping_receive_socket, &fds))
        {
            //printf("%d\n", __LINE__);
            if ((cc = recvfrom(ping_receive_socket,
                               rcv_buf, PING_MGR_RCV_BUF_SIZE, 0,
                               (struct sockaddr *)&fromp,
                               (socklen_t *)&fromlen))<=0)
            {
                /* cc is the size of data
                 * re-calculate waiting_time and wait again
                 */

                /* need to test cc == 0 for SNMP, where size 0 is allowed. */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }

        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if(FD_ISSET(ping6_receive_socket, &fds))
        {

            if ((cc = recvfrom(ping6_receive_socket,
                               rcv_buf, PING_MGR_RCV_BUF_SIZE, 0,
                               (struct sockaddr *)&from6,
                               (socklen_t *)&fromlen6))<=0)
            {
#if 0
                break; // 2008/1/7 07:29 pm, peter, 'break' should be refined in the future.
#endif
                /* cc is the size of data
                 * re-calculate waiting_time and wait again
                 */

                /* need to test cc == 0 for SNMP, where size 0 is allowed. */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }
        } /* FD_ISSET*/
#endif
        receive_time_stamp = SYSFUN_GetSysTick();
        if(TRUE == ping_backdoor_debug)
        {
            BACKDOOR_MGR_Printf("%s, %d\n", __FUNCTION__, __LINE__);
            BACKDOOR_MGR_Printf("receive_time_stamp: %ld\n", (long)receive_time_stamp);
            BACKDOOR_MGR_Printf("recvfrom, cc= %d \n", cc);
            DBG_DumpHex("", cc, (char *) rcv_buf);
        }

        memset(&inet_addr, 0, sizeof(inet_addr));
#if (SYS_CPNT_IPV6 == TRUE)
        L_INET_SockaddrToInaddr((struct sockaddr *)&fromp, &inet_addr);
        if((inet_addr.type != L_INET_ADDR_TYPE_IPV4)
            || (inet_addr.type != L_INET_ADDR_TYPE_IPV4Z))
        {
            L_INET_SockaddrToInaddr((struct sockaddr *)&from6, &inet_addr);
        }
#else
        L_INET_SockaddrToInaddr((struct sockaddr *)&fromp, &inet_addr);
#endif
        if( (inet_addr.type == L_INET_ADDR_TYPE_IPV4)
            || (inet_addr.type == L_INET_ADDR_TYPE_IPV4Z))
        {
            /****** v4 ******/

            /* get ip pkt from buf */
            icp = (ip_packet_T*) rcv_buf;

            /* we only care about Echo Reply type */
            if (icp->icmp.icmp_type != 0x00) /* ICMP Echo Reply */
            {
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick(); /*ES3628BT-FLF-ZZ-00359*/
                continue;
            }

            /* get identity and seq number for ICMP packet */
            icmp_id = icp->icmp.icmp_id;
            icmp_seq = icp->icmp.icmp_seq;
            /* DEBUG */
            if(TRUE == ping_backdoor_debug)
            {
                BACKDOOR_MGR_Printf("\nPING_MGR_WaitForReply: ");
                BACKDOOR_MGR_Printf("receive pkt sip=0x%02x%02x%02x%02x", icp->srcIp[0], icp->srcIp[1], icp->srcIp[2], icp->srcIp[3]);
                BACKDOOR_MGR_Printf(" dip=0x0%2x%02x%02x%02x\n", icp->dstIp[0], icp->dstIp[1], icp->dstIp[2], icp->dstIp[3]);
                BACKDOOR_MGR_Printf("icmp_type: %hu (request/reply:8/0)\n", icp->icmp.icmp_type);
                BACKDOOR_MGR_Printf("icmp_code: %hu \n", icp->icmp.icmp_code);
                BACKDOOR_MGR_Printf("icmp_id: 0x%hX \n", icmp_id);
                BACKDOOR_MGR_Printf("icmp_seq: %hu \n", icmp_seq);
            }

        }
#if (SYS_CPNT_IPV6 == TRUE)
        else
        {
            /****** v6 ******/
            //ip6 = (ipv6_packet_T*) rcv_buf;
            //icmp6 = (PING_MGR_ICMP6_T *) (ip6 + 1);
            icmp6 =(PING_MGR_ICMP6_T *) rcv_buf;
            /* we only care about Echo Reply type */
            if (icmp6->type != 129) /* ICMPv6 Echo Reply */
            {
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick(); /*ES3628BT-FLF-ZZ-00359*/
                continue;
            }

            /* get identity and seq number for ICMP packet */
            icmp_id = icmp6->id;
            icmp_seq = icmp6->seq;

            /* DEBUG */
            if(TRUE == ping_backdoor_debug)
            {
                L_INET_InaddrToString((L_INET_Addr_T *) &inet_addr, tmp_buf, sizeof(tmp_buf));

                BACKDOOR_MGR_Printf("\nPING_MGR_WaitForReply: receive from [%s]", tmp_buf);
/*
                printf("receive pkt src addr=0x%02x%02x%02x%02x", ip6->src_addr[0], icp->srcIp[1], icp->srcIp[2], icp->srcIp[3]);
                printf(" dip=0x0%2x%02x%02x%02x\n", icp->dstIp[0], icp->dstIp[1], icp->dstIp[2], icp->dstIp[3]);
*/
                BACKDOOR_MGR_Printf("icmp_type: %hu (request/reply:128/129)\n", icmp6->type);
                BACKDOOR_MGR_Printf("icmp_code: %hu \n", icmp6->code);
                BACKDOOR_MGR_Printf("icmp_id: 0x%hX \n", icmp6->id);
                BACKDOOR_MGR_Printf("icmp_seq: %hu \n", icmp6->seq);
            }

        }
#endif


        /* Sweep thru existing workspace to find which index the receive socket belongs.  If none
           matches then continue;
         */
        for (active_index = 0; active_index < SYS_ADPT_MAX_PING_NUM; active_index++)
        {
            if(TRUE == ping_backdoor_debug)
            {
                BACKDOOR_MGR_Printf("active_index: %ld ", (long)active_index);
                BACKDOOR_MGR_Printf("its ident: %d ", ping_workspace[active_index].ident);
                BACKDOOR_MGR_Printf("its admn_status: %ld\n", (long)ping_workspace[active_index].admin_status);
            }

            if (ping_workspace[active_index].admin_status != VAL_pingCtlAdminStatus_enabled)
            {
                continue; /* for */
            }

            if(icmp_id == ping_workspace[active_index].ident)
            {
                active_index_found = TRUE;
                DEBUG_PRINTF("active_index_found : %ld\n", (long)active_index);
                break;  /* for */
            }

        } /* end of for */

        if (!active_index_found)
        {
            DEBUG_PRINTF("active_index_found  is false.\n");

            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }

        if ((res = PING_OM_PingTableIndexToKey(active_index, &list_elm)) !=  PING_TYPE_OK)
        {
            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }

        if(TRUE == ping_backdoor_debug)
        {
            BACKDOOR_MGR_Printf("ident match! index : %ld\n", (long)active_index);
            BACKDOOR_MGR_Printf("icmp_seq: %d\n", icmp_seq);
        }

        /* duplicate icmp_seq will be ignore for ipv4
           or ipv6 unicast.
        */

        if( (inet_addr.type == L_INET_ADDR_TYPE_IPV4)
            || (inet_addr.type == L_INET_ADDR_TYPE_IPV4Z)
            || ((inet_addr.type == L_INET_ADDR_TYPE_IPV6 || inet_addr.type == L_INET_ADDR_TYPE_IPV6Z)
                && (ping_workspace[active_index].mcast_ipv6_target_addr == FALSE)))
        {
            if(icmp_seq <= ping_workspace[active_index].probe_history_last_index)
            {
                DEBUG_PRINTF("Duplicate icmp reply, seq no: %d, ignore it.\r\n", icmp_seq);
                /* re-calculate waiting_time and wait again */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }
        }

        /* add new entry into probe history table */
        /* history index is icmp_seq number */

        ping_workspace[active_index].received_probs++;
        ping_workspace[active_index].rcv_pkt_len = cc;
        ping_workspace[active_index].last_receive_time = receive_time_stamp;
        ping_workspace[active_index].probe_history_last_index = icmp_seq;
        DEBUG_PRINTF("probe_history_last_index: %ld.\r\n", (long)ping_workspace[active_index].probe_history_last_index);

        memset(&global_control_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
        memcpy(global_control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
        memcpy(global_control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

        if (PING_TYPE_OK!= PING_OM_GetCtlEntry(&global_control_entry))
        {
            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }
        owner_index_len = global_control_entry.ping_ctl_owner_index_len;
        test_name_len = global_control_entry.ping_ctl_test_name_len;
        memset(&prob_history_entry, 0, sizeof(PING_TYPE_PingProbeHistoryEntry_T));
        memcpy(prob_history_entry.ping_ctl_owner_index,list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
        memcpy(prob_history_entry.ping_ctl_test_name,list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
        prob_history_entry.ping_ctl_owner_index_len = owner_index_len; /* MUST! */
        prob_history_entry.ping_ctl_test_name_len = test_name_len; /* MUST! */
        prob_history_entry.icmp_sequence = icmp_seq;
        response_time = 10 * (ping_workspace[active_index].last_receive_time -
                        ping_workspace[active_index].last_transmit_time);

        prob_history_entry.ping_probe_history_response = response_time;
        prob_history_entry.ping_probe_history_status = VAL_pingProbeHistoryStatus_responseReceived;
        prob_history_entry.system_tick = SYSFUN_GetSysTick();
        SYS_TIME_GetDayAndTime(prob_history_entry.ping_probe_history_time,
                               &prob_history_entry.ping_probe_history_time_len);

        memcpy(&prob_history_entry.src_addr, &inet_addr, sizeof(inet_addr));

        if (PING_TYPE_OK != PING_OM_AppendProbePacketResult(active_index, &prob_history_entry))
        {
            DEBUG_PRINTF("append prob history failed! index: %ld\n", (long)prob_history_entry.icmp_sequence);
        }


        /* update result_entry
         */
        memcpy(global_result_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
        memcpy(global_result_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

        if (PING_TYPE_OK != PING_OM_GetResultsEntry(&global_result_entry))
        {
            DEBUG_PRINTF("PING_MGR_WaitForReply: get result entry failed!\n");
            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }

        /* update min, max, average rtt */
        /*
                *EPR_ID: ES3628BT-FLF-ZZ-00614
                *Problem: SNMP:SNMP: PingMIB->pingResultsMinRtt The value is incorrect.
                *Root Cause: pingResultsMinRtt default value is wrong.
                *Modified files:
                *        src\user\core\l3\ping\ping_mgr.c
                *Approved by: Tiger Liu
                *Modified by: Shumin Wang
                */
        if(global_result_entry.ping_results_sent_probes == 1)
        {
            global_result_entry.ping_results_min_rtt = response_time;
            global_result_entry.ping_results_max_rtt = response_time;
        }
        else
        {
            if(response_time < global_result_entry.ping_results_min_rtt)
            {
                global_result_entry.ping_results_min_rtt = response_time;
            }

            if(response_time > global_result_entry.ping_results_max_rtt)
            {
                global_result_entry.ping_results_max_rtt = response_time;
            }
        }
        global_result_entry.ping_results_average_rtt =
        (global_result_entry.ping_results_average_rtt * global_result_entry.ping_results_probe_responses + response_time)
         / (global_result_entry.ping_results_probe_responses +1);

        /* update ping_results_probe_responses */
        global_result_entry.ping_results_probe_responses++;

        /* set last good probe day-time */
        SYS_TIME_GetDayAndTime(global_result_entry.ping_results_last_good_probe, &global_result_entry.ping_results_last_good_probe_len);

        if ((res = PING_OM_SetResultsEntry(active_index, &global_result_entry)) != PING_TYPE_OK)
        {
            DEBUG_PRINTF("PING_MGR_WaitForReply: set result entry failed!\n");
            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }

        /* check if active index should change admin_status to disable */
        //if(ping_workspace[active_index].received_probs + ping_workspace[active_index].unreceived_probs >= ping_workspace[active_index].probe_count)
        if(ping_workspace[active_index].sent_probes >= ping_workspace[active_index].probe_count)
        {
            if (ping_workspace[active_index].mcast_ipv6_target_addr == TRUE)
            {
                DEBUG_PRINTF("this is mcast address(2)\n");
             if (SYSFUN_GetSysTick() - ping_workspace[active_index].last_transmit_time <
                     SYS_DFLT_PING_CTL_TIME_OUT)
                {


                /* DEBUG */
                DEBUG_PRINTF("waiting other responsing pkt...\n");

                /* re-calculate waiting_time and wait again */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();

                //ping_workspace[active_index].last_transmit_time = 0;
                //ping_workspace[active_index].last_receive_time = 0;
                continue;
                }
            }
            if ((res = PING_OM_PingTableIndexToKey(active_index, &list_elm)) !=  PING_TYPE_OK)
            {
                /* re-calculate waiting_time and wait again */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }
            memcpy(global_control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
            memcpy(global_control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

            if ((res = PING_OM_GetCtlEntry(&global_control_entry)) != PING_TYPE_OK)
            {
                /* re-calculate waiting_time and wait again */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }

            /* stop send prob pkt */

            /* DEBUG */
            DEBUG_PRINTF("going to disable admin_status.(2)\n");

            ping_workspace[active_index].admin_status = VAL_pingCtlAdminStatus_disabled;

            global_control_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;

            if ((res = PING_OM_SetCtlEntry(&global_control_entry)) != PING_TYPE_OK)
            {
                /* re-calculate waiting_time and wait again */
                waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
                continue;
            }
            /* re-calculate waiting_time and wait again */
            waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();
            continue;
        }
        /* clear timer */
        //ping_workspace[active_index].last_transmit_time = 0;
        //ping_workspace[active_index].last_receive_time = 0;

        /* re-calculate waiting_time and wait again */
        waiting_time = waiting_time_stamp - SYSFUN_GetSysTick();

    } /* end of while */
    return PING_TYPE_OK;

} /* end of PING_MGR_WaitForReply() */

/* FUNCTION NAME : PING_MGR_CheckToStartTimer
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
static void PING_MGR_CheckToStartTimer(void)
{
    UI32_T active_index;

    if (PING_OM_GetFirstActiveResultsEntry(&active_index, &global_result_entry) == PING_TYPE_OK)
    {
        PING_TASK_PeriodicTimerStart_Callback();
    }
}

/* FUNCTION NAME : PING_MGR_InitControlEntry
 * PURPOSE:
 *      Init control entry with default value stated in MIB
 * INPUT:
 *      ctrl_entry - control entry to init.
 * OUTPUT:
 *      None.
 * RETURN:
 *      None
 * NOTES:
 *      1. Caller should be SetCtlRowStatus, not SetCtlEntry.
 */
static void PING_MGR_InitControlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* Local Variable Declaration
     */
    /* BODY
     */
    ctrl_entry_p->ping_ctl_target_address_type  = SYS_DFLT_PING_CTL_IP_ADDRESS_TYPE;
    ctrl_entry_p->ping_ctl_by_pass_route_table  = SYS_DFLT_PING_CTL_BY_PASS_ROUTE_TABLE;
    ctrl_entry_p->ping_ctl_data_size            = SYS_DFLT_PING_CTL_DATA_SIZE;
    ctrl_entry_p->ping_ctl_timeout              = (SYS_DFLT_PING_CTL_TIME_OUT / SYS_BLD_TICKS_PER_SECOND);
    ctrl_entry_p->ping_ctl_probe_count          = 1; /* SNMP range 1-15 */
    ctrl_entry_p->ping_ctl_ds_field             = SYS_DFLT_PING_CTL_DS_FIELD;
    ctrl_entry_p->ping_ctl_source_address_type  = VAL_pingCtlSourceAddressType_ipv4;
    ctrl_entry_p->ping_ctl_if_index             = SYS_DFLT_PING_CTL_IFINDEX;
    ctrl_entry_p->ping_ctl_frequency            = SYS_DFLT_PING_CTL_FREQUENCY;
    ctrl_entry_p->ping_ctl_storage_type         = SYS_DFLT_PING_CTL_STORAGE_TYPE;
    ctrl_entry_p->ping_ctl_admin_status         = SYS_DFLT_PING_CTL_ADMIN_STATUS;
    ctrl_entry_p->ping_ctl_max_rows             = SYS_DFLT_PING_CTL_MAX_ROWS;
    ctrl_entry_p->ping_ctl_trap_generation      = SYS_DFLT_PING_CTL_TRAP_GENERATION;

    return;
} /* end of PING_MGR_InitControlEntry() */


#if (BACKDOOR_OPEN == TRUE)

static void   PING_MGR_PrintControlEntryInfo(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* Local Variable Declaration
     */
    char tmp_buf[MAX_BUF_LEN];

    /* BODY
     */
    BACKDOOR_MGR_Printf("\nping_ctl_owner_index %s \n", ctrl_entry_p->ping_ctl_owner_index);
    BACKDOOR_MGR_Printf("ping_ctl_test_name %s \n", ctrl_entry_p->ping_ctl_test_name);
    BACKDOOR_MGR_Printf("ping_ctl_owner_index_len %d\n", (int) ctrl_entry_p->ping_ctl_owner_index_len);
    BACKDOOR_MGR_Printf("ping_ctl_test_name_len %d\n", (int) ctrl_entry_p->ping_ctl_test_name_len);
    BACKDOOR_MGR_Printf("ping_ctl_target_address_type (ipv4/ipv6/ipv4z/ipv6z=1/2/3/4):%d \n",(int) ctrl_entry_p->ping_ctl_target_address_type);
    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&ctrl_entry_p->ping_ctl_target_address, tmp_buf, sizeof(tmp_buf));
    BACKDOOR_MGR_Printf("ping_ctl_target_address: %s\n", tmp_buf);

    BACKDOOR_MGR_Printf("ping_ctl_data_size %d\n", (int) ctrl_entry_p->ping_ctl_data_size);
    BACKDOOR_MGR_Printf("ping_ctl_timeout %d\n", (int) ctrl_entry_p->ping_ctl_timeout);
    BACKDOOR_MGR_Printf("ping_ctl_probe_count %d\n", (int) ctrl_entry_p->ping_ctl_probe_count);
    BACKDOOR_MGR_Printf("ping_ctl_admin_status (enabled/disabled=1/2) %d \n",(int) ctrl_entry_p->ping_ctl_admin_status);
    /* ping_ctl_data_fill[SYS_ADPT_PING_MAX_DATA_FILL_SIZE]; */

    BACKDOOR_MGR_Printf("ping_ctl_frequency %d \n", (int)ctrl_entry_p->ping_ctl_frequency);
    BACKDOOR_MGR_Printf("ping_ctl_max_rows %d \n", (int)ctrl_entry_p->ping_ctl_max_rows);
    BACKDOOR_MGR_Printf("ping_ctl_storage_type %d \n",(int) ctrl_entry_p->ping_ctl_storage_type);
    BACKDOOR_MGR_Printf("ping_ctl_trap_generation %d \n",(int) ctrl_entry_p->ping_ctl_trap_generation);
    /* ping_ctl_trap_probe_failure_filter; */
    /* ping_ctl_trap_test_failure_filter; */
    /* pingctltype; */

    BACKDOOR_MGR_Printf("ping_ctl_descr %s \n", ctrl_entry_p->ping_ctl_descr);
    BACKDOOR_MGR_Printf("ping_ctl_source_address_type %d \n",(int) ctrl_entry_p->ping_ctl_source_address_type);
    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&ctrl_entry_p->ping_ctl_source_address, tmp_buf, sizeof(tmp_buf));
    BACKDOOR_MGR_Printf("ping_ctl_source_address: %s\n", tmp_buf);

    BACKDOOR_MGR_Printf("ping_ctl_if_index %d \n",(int) ctrl_entry_p->ping_ctl_if_index);
    BACKDOOR_MGR_Printf("ping_ctl_by_pass_route_table %d \n",(int) ctrl_entry_p->ping_ctl_by_pass_route_table);
    BACKDOOR_MGR_Printf("ping_ctl_ds_field %d \n",(int) ctrl_entry_p->ping_ctl_ds_field);
    BACKDOOR_MGR_Printf("ping_ctl_rowstatus (active/notInService/notReady/createAndGo/createAndWait/destroy=1-6): %d \n",(int) ctrl_entry_p->ping_ctl_rowstatus);
    return;

}

static void PING_MGR_DisplayAllWorkspaceDetail(void)
{
    /* Local Variable Declaration
     */
    UI32_T  index;
    char tmp_buf[MAX_BUF_LEN];

    /* BODY */
    for (index=0; index<SYS_ADPT_MAX_PING_NUM; index++)
    {
        if(0 == ping_workspace[index].owner_tid)
        {
            BACKDOOR_MGR_Printf("Workspace[%ld] is not running. Skipped.\n", (long)index);
            continue;
        }
        BACKDOOR_MGR_Printf("Workspace index: %ld\n", (long)index);

        BACKDOOR_MGR_Printf("owner tid: %d\n", (int)ping_workspace[index].owner_tid);
        BACKDOOR_MGR_Printf("ident: %d\n",(int) ping_workspace[index].ident);
        BACKDOOR_MGR_Printf("target_addr_type: %ld\n", (long)ping_workspace[index].target_addr_type);

        memset(tmp_buf, 0, MAX_BUF_LEN);
        L_INET_InaddrToString((L_INET_Addr_T *)&ping_workspace[index].src_ip, tmp_buf, sizeof(tmp_buf));
        BACKDOOR_MGR_Printf("src_addr: %s\n", tmp_buf);

        memset(tmp_buf, 0, MAX_BUF_LEN);
        L_INET_InaddrToString((L_INET_Addr_T *)&ping_workspace[index].dst_ip, tmp_buf, sizeof(tmp_buf));
        BACKDOOR_MGR_Printf("dst_addr: %s\n", tmp_buf);

        BACKDOOR_MGR_Printf("Whereto.sin_family %d\n",(int) ping_workspace[index].whereto.sin_family);
        BACKDOOR_MGR_Printf("Whereto.sin %d\n",(int) ping_workspace[index].whereto.sin_port);
#if (SYS_CPNT_IPV6 == TRUE)
        BACKDOOR_MGR_Printf("Whereto6.sin6_family %d\n",(int) ping_workspace[index].whereto6.sin6_family);
        BACKDOOR_MGR_Printf("Whereto6.sin6_port %d\n",(int) ping_workspace[index].whereto6.sin6_port);
        BACKDOOR_MGR_Printf("Whereto6.sin6_scope_id %d\n",(int) ping_workspace[index].whereto6.sin6_scope_id);
#endif
        BACKDOOR_MGR_Printf("prob_count: %ld\n", (long)ping_workspace[index].probe_count);
        BACKDOOR_MGR_Printf("send_probs: %ld\n", (long)ping_workspace[index].sent_probes);
        BACKDOOR_MGR_Printf("received_probs: %ld\n", (long)ping_workspace[index].received_probs);
        BACKDOOR_MGR_Printf("unreceived_probs: %ld\n", (long)ping_workspace[index].unreceived_probs);
        BACKDOOR_MGR_Printf("last_transmit_time: %ld\n", (long)ping_workspace[index].last_transmit_time);
        BACKDOOR_MGR_Printf("mcast_ipv6_target_addr: %d\n", ping_workspace[index].mcast_ipv6_target_addr);

        BACKDOOR_MGR_Printf("\n");
    } /* for */

    return;

} /* end of PING_MGR_DumpDetailWorkspace() */

static void   PING_MGR_DisplayKeyIndexTable(void)
{
    /* Local Variable Declaration
     */
    PING_SORTLST_ELM_T    list_elm;
    UI8_T   owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];
    UI8_T   test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];
    UI32_T  local_index = 0;
    UI32_T  res;
    PING_TYPE_PingCtlEntry_T    ctl_entry;
    char tmp_buf[MAX_BUF_LEN];

    /* BODY
     */

    BACKDOOR_MGR_Printf("* means index is in used.\n");
    BACKDOOR_MGR_Printf("%-10.10s %-10.10s %-10.10s %-10.10s\n", "key_index",  "owner_index", "test_name", "target_ip");

    BACKDOOR_MGR_Printf("-----------------------------------------\n");

    for (; local_index < SYS_ADPT_MAX_PING_NUM; local_index++)
    {
        memset(&owner_index, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
        memset(&test_name, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

        if ((res = PING_OM_PingTableIndexToKey(local_index, &list_elm)) == PING_TYPE_OK)
        {
            memset(&ctl_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
            memcpy(ctl_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
            memcpy(ctl_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
            if (PING_TYPE_OK == PING_OM_GetCtlEntry(&ctl_entry))
            {
                BACKDOOR_MGR_Printf("%-10ld* %-10.10s %-10.10s ", (long)local_index, ctl_entry.ping_ctl_owner_index, ctl_entry.ping_ctl_test_name);
                memset(tmp_buf, 0, MAX_BUF_LEN);
                L_INET_InaddrToString((L_INET_Addr_T *)&ctl_entry.ping_ctl_target_address, tmp_buf, sizeof(tmp_buf));
                BACKDOOR_MGR_Printf(" %s\n", tmp_buf);

            }
        }
        else
        {
            /* print only index */
            BACKDOOR_MGR_Printf("%-10ld\n", (long)local_index);
        }
    } /* end of for */
    return;
} /* end of PING_MGR_DisplayKeyIndexTable() */

/* FUNCTION NAME : PING_MGR_ConvertDayTime
 * PURPOSE:
 *      Convert data from SYS_TIME_GetDayAndTime into string format.
 * INPUT:
 *      size_of_str_p       -- size of the buffer pointed by str_p
 *      day_and_time        -- the byte array data previously generated by SYS_TIME_GetDayAndTime.
 *      day_and_time_len    -- the length of day_and_time
 * OUTPUT:
 *      str_p               -- the string format of day_and_time.
 * RETURN:
 *      None
 * NOTES:
 *      1. str_p must be big enough to receive the result.
 *      2. Currently this is only used for backdoor, the correctness is not sure.
 *      3. Output example: 0007-01-01 09:03:43
 */
static UI32_T PING_MGR_ConvertDayTime(char *str_p, UI32_T size_of_str_p, UI8_T *day_and_time, UI32_T day_and_time_len)
{
    UI16_T year;

    if(str_p == NULL)
    {
        return PING_TYPE_FAIL;
    }

    /* exmple: 2007-04-02 12:59:59 */
    memcpy(&year, day_and_time, sizeof(UI16_T));
    snprintf(str_p, size_of_str_p, "%4.4hu-%2.2hu-%2.2hu %2.2hu:%2.2hu:%2.2hu",
        year,
        day_and_time[2],
        day_and_time[3],
        day_and_time[4],
        day_and_time[5],
        day_and_time[6]);

    return PING_TYPE_OK;
}

static void PING_MGR_DisplayPingResult(PING_TYPE_PingResultsEntry_T result_entry)
{
    /* Local Variable Declaration
     */
    PING_TYPE_PingProbeHistoryEntry_T   prob_history_entry;
    char    str_daytime[50];

    /* BODY
     */
    if (PING_OM_GetResultsEntry(&result_entry) != PING_TYPE_OK)
    {
        BACKDOOR_MGR_Printf("PING_OM_GetResultsEntry failed, return.");
        return;
    }

    BACKDOOR_MGR_Printf("\nSends: %d, Receives: %d\n", (int) result_entry.ping_results_sent_probes, (int) result_entry.ping_results_probe_responses);

    /* get relative prob_history_entry */
    memset(&prob_history_entry, 0, sizeof(PING_TYPE_PingProbeHistoryEntry_T));
    memcpy(prob_history_entry.ping_ctl_owner_index, result_entry.ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(prob_history_entry.ping_ctl_test_name, result_entry.ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    BACKDOOR_MGR_Printf("PING_MGR_DisplayPingResult\n");
    BACKDOOR_MGR_Printf("status: \n" \
            "responseReceived(1)\n" \
            "unknown(2)\n" \
            "internalError(3)\n" \
            "requestTimedOut(4)\n" \
            "unknownDestinationAddress(5)\n" \
            "noRouteToTarget(6)\n" \
            "interfaceInactiveToTarget(7)\n" \
            "arpFailure(8)\n" \
            "maxConcurrentLimitReached(9)\n" \
            "unableToResolveDnsName(10)\n" \
            "invalidHostAddress(11)\n\n");

   /* the filed name string may would be truncated */
   BACKDOOR_MGR_Printf("%-10.10s %-10.10s %-10.10s %-10.10s %-10.10s %-10.10s\n", "owner_index", "test_name", "history_index",
   "response_time", "status", "time stamp");

    while (PING_TYPE_OK == PING_OM_GetNextProbeHistoryEntryForCli(&prob_history_entry))
    {
        /* convert daytime */
        memset(str_daytime, 0, sizeof(str_daytime));

        if(PING_TYPE_OK != PING_MGR_ConvertDayTime(str_daytime, sizeof(str_daytime), prob_history_entry.ping_probe_history_time, prob_history_entry.ping_probe_history_time_len))
        {
            BACKDOOR_MGR_Printf("daytime converted failed.\n");
        }

        BACKDOOR_MGR_Printf("%-10.10s %-10.10s %-10d %-ldms %-10d %-20.20s\n",
        prob_history_entry.ping_ctl_owner_index,
        prob_history_entry.ping_ctl_test_name,
        (int) prob_history_entry.icmp_sequence,
        (long)prob_history_entry.ping_probe_history_response,
        (int) prob_history_entry.ping_probe_history_status,
        str_daytime);
    }

    return;
} /* end of PING_MGR_DisplayPingResult() */

static void PING_MGR_PrintResultEntryInfo(PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* Local Variable Declaration
     */
     char   str_daytime[50];
     char   tmp_buf[MAX_BUF_LEN + 1];

    /* BODY
     */
    BACKDOOR_MGR_Printf("\nping_ctl_owner_index %s \n", result_entry_p->ping_ctl_owner_index);
    BACKDOOR_MGR_Printf("ping_ctl_test_name %s \n", result_entry_p->ping_ctl_test_name);
    BACKDOOR_MGR_Printf("ping_ctl_owner_index_len %d\n", (int) result_entry_p->ping_ctl_owner_index_len);
    BACKDOOR_MGR_Printf("ping_ctl_test_name_len %d\n", (int) result_entry_p->ping_ctl_test_name_len);
    BACKDOOR_MGR_Printf("ping_results_oper_status (enabled/disabled=1/2) %d\n", (int) result_entry_p->ping_results_oper_status);
    BACKDOOR_MGR_Printf("ping_results_ip_target_address_type (ipv4/ipv6/ipv4z/ipv6z=1/2/3/4):%d \n",(int) result_entry_p->ping_results_ip_target_address_type);

    memset(tmp_buf, 0, MAX_BUF_LEN);
    L_INET_InaddrToString((L_INET_Addr_T *)&result_entry_p->ping_results_ip_target_address, tmp_buf, sizeof(tmp_buf));
    BACKDOOR_MGR_Printf("ping_results_ip_target_address: %s\n", tmp_buf);


    BACKDOOR_MGR_Printf("ping_results_min_rtt %d\n", (int) result_entry_p->ping_results_min_rtt);
    BACKDOOR_MGR_Printf("ping_results_max_rtt %d\n", (int) result_entry_p->ping_results_max_rtt);
    BACKDOOR_MGR_Printf("ping_results_average_rtt %d \n", (int)result_entry_p->ping_results_average_rtt);
    BACKDOOR_MGR_Printf("ping_results_probe_responses %d \n", (int)result_entry_p->ping_results_probe_responses);
    BACKDOOR_MGR_Printf("ping_results_sent_probes %d \n",(int) result_entry_p->ping_results_sent_probes);
    /* ping_results_rtt_sum_of_squares */


    memset(str_daytime, 0, sizeof(str_daytime));
    if( PING_TYPE_OK == PING_MGR_ConvertDayTime(str_daytime, sizeof(str_daytime), result_entry_p->ping_results_last_good_probe, result_entry_p->ping_results_last_good_probe_len))
    {
        BACKDOOR_MGR_Printf("ping_results_last_good_probe %s \n", str_daytime);
    }
    return;
}

static void PING_MGR_PrintAllProbeHistory(void)
{
    PING_OM_PrintAllProbeHistory();
    return;

}

static  BOOL_T  PING_MGR_StrToVal(char *str_p, UI32_T *value_p)
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
} /* End of PING_MGR_StrToVal */

/* FUNCTION NAME : PING_MGR_BackDoor_Menu
 * PURPOSE:
 *      For testing ping function.
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
static void PING_MGR_BackDoor_Menu(void)
{
    /* Local Variable Declaration
     */
    int ch;
    char buf[MAX_BUF_LEN] = {};
    char *terminal;

    //UI32_T  handler_index;
    UI32_T  res;
    char    owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];
    char    test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];
    UI32_T  owner_index_len=0,test_name_len=0;
    UI32_T  operation;
    UI32_T  value;
    PING_TYPE_PingCtlEntry_T        control_entry;
    PING_TYPE_PingResultsEntry_T    result_entry;

    L_INET_AddrIp_T inet_addr;
    //UI8_T tmp_buf[MAX_BUF_LEN] = {};
    L_INET_RETURN_VALUE_T rc_inet;

    /* BODY */
    while (1)
    {
        BACKDOOR_MGR_Printf("\n 0. Exit\n");
        BACKDOOR_MGR_Printf(" 1. Create control entry and ping.\n");
        BACKDOOR_MGR_Printf(" 2. Print specified control entry by index.\n");
        BACKDOOR_MGR_Printf(" 3. Set value of control entry.\n");
        BACKDOOR_MGR_Printf(" 4. Display Result Entry table.\n");
        BACKDOOR_MGR_Printf(" 5. Display Ping result. \n");
        BACKDOOR_MGR_Printf(" 6. PING_MGR_PrintAllProbeHistory \n");
        BACKDOOR_MGR_Printf(" 7. Enable / Disable debug flag.(%s) \n", ((ping_backdoor_debug)?"ON":"OFF"));
        BACKDOOR_MGR_Printf(" 8. Display all running workSpace details.\n");
        BACKDOOR_MGR_Printf(" 9. Display Key <-> Index Table\n");
        BACKDOOR_MGR_Printf(" select = ");

        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
        ch = (int) strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");
        switch (ch)
        {
            case 0:
                return;

            case 1:
            {
                UI32_T rc;
                memset(&control_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));
                memset(owner_index, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memset(test_name, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                /* owner index */
                BACKDOOR_MGR_Printf("Input Owner index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
                owner_index_len=strlen(owner_index);
                BACKDOOR_MGR_Printf("\n");
                memcpy(control_entry.ping_ctl_owner_index, owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                control_entry.ping_ctl_owner_index_len = owner_index_len;
                /* test name */

                BACKDOOR_MGR_Printf("Input Test index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(test_name, SYS_ADPT_PING_MAX_NAME_SIZE);
                test_name_len=strlen(test_name);
                BACKDOOR_MGR_Printf("\n");
                memcpy(control_entry.ping_ctl_test_name, test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                control_entry.ping_ctl_test_name_len = test_name_len;

                /* target ip addr */
                BACKDOOR_MGR_Printf("Input target ipv4/v6 adress (A.B.C.D or X:X:X:X::X[scope_id]) = \n");
                BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                BACKDOOR_MGR_Printf("buf: %s\n", buf);

                memset(&inet_addr, 0, sizeof(L_INET_AddrIp_T));
                rc_inet = L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, buf, (L_INET_Addr_T *)&inet_addr, sizeof(inet_addr));
                BACKDOOR_MGR_Printf("rc_inet = %d\n", rc_inet);
                BACKDOOR_MGR_Printf("inet_addr type: %x\n", inet_addr.type);
                BACKDOOR_MGR_Printf("inet_addr addr[0]: %x\n", inet_addr.addr[0]);

                memset(buf, 0, MAX_BUF_LEN);
                rc_inet = L_INET_InaddrToString((L_INET_Addr_T *)&inet_addr, buf, sizeof(buf));
                BACKDOOR_MGR_Printf("rc_inet = %d\n", rc_inet);
                BACKDOOR_MGR_Printf("target addr string is: %s\n", buf);
                memcpy(&control_entry.ping_ctl_target_address, &inet_addr, sizeof(inet_addr));

                /* prob count */
                BACKDOOR_MGR_Printf("Input prob count = \n");
                BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                PING_MGR_StrToVal(buf, &value);
                BACKDOOR_MGR_Printf("\n");
                control_entry.ping_ctl_probe_count = value;

                /* DEBUG*/
                BACKDOOR_MGR_Printf("owner_index: %s\n", control_entry.ping_ctl_owner_index);
                BACKDOOR_MGR_Printf("test_name: %s\n", control_entry.ping_ctl_test_name);
                BACKDOOR_MGR_Printf("owner_index_len: %d\n", (int) control_entry.ping_ctl_owner_index_len);
                BACKDOOR_MGR_Printf("test_name_len: %d\n", (int) control_entry.ping_ctl_test_name_len);

                memset(buf, 0, MAX_BUF_LEN);

                L_INET_InaddrToString((L_INET_Addr_T *) &control_entry.ping_ctl_target_address, buf, sizeof(buf));
                BACKDOOR_MGR_Printf("ping_ctl_target_address: %s\n", buf);

                memset(buf, 0, MAX_BUF_LEN);
                L_INET_InaddrToString((L_INET_Addr_T *)&control_entry.ping_ctl_source_address, buf, sizeof(buf));
                BACKDOOR_MGR_Printf("control_entry.ping_ctl_source_address: %s\n", buf);
                BACKDOOR_MGR_Printf("prob count: %ld\n", (long)control_entry.ping_ctl_probe_count);
                /* end of DEBUG */
                control_entry.ping_ctl_target_address_type = control_entry.ping_ctl_target_address.type;
                control_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_createAndGo;
                control_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_enabled;
                rc = PING_MGR_SetCtlEntry(&control_entry);
                if(rc == PING_TYPE_OK)
                {
                    BACKDOOR_MGR_Printf("PING_MGR_SetCtlEntry succeed.\n");
                }
                else
                {
                    BACKDOOR_MGR_Printf("PING_MGR_SetCtlEntry failed.\n");
                }
                break;
            }
            case 2:
            {
                PING_SORTLST_ELM_T    list_elm;
                UI32_T ctl_entry_index;
                BACKDOOR_MGR_Printf("Input Ctl entry index: \n");
                BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                PING_MGR_StrToVal(buf, &ctl_entry_index);
                BACKDOOR_MGR_Printf("\n");
                if (PING_TYPE_OK != PING_OM_PingTableIndexToKey(ctl_entry_index, &list_elm))
                {
                    BACKDOOR_MGR_Printf("no such entry.\n");
                    break;
                }
                memcpy(control_entry.ping_ctl_owner_index, list_elm.owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(control_entry.ping_ctl_test_name, list_elm.test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                if (PING_TYPE_OK == PING_OM_GetCtlEntry(&control_entry))
                {
                    PING_MGR_PrintControlEntryInfo(&control_entry);
                }
                else
                {
                    BACKDOOR_MGR_Printf("no such entry.\n");
                }

                break;
            }
            case 3:
                memset(owner_index, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memset(test_name, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                BACKDOOR_MGR_Printf("Input Owner index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
                owner_index_len=strlen(owner_index);
                BACKDOOR_MGR_Printf("\n");
                BACKDOOR_MGR_Printf("Input Test index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(test_name, SYS_ADPT_PING_MAX_NAME_SIZE);
                test_name_len=strlen(test_name);
                BACKDOOR_MGR_Printf("\n");

               /* initial key*/
                memcpy(control_entry.ping_ctl_owner_index, owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(control_entry.ping_ctl_test_name, test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                /* DEBUG*/
                BACKDOOR_MGR_Printf("owner_index: %s\n", control_entry.ping_ctl_owner_index);
                BACKDOOR_MGR_Printf("test_name: %s\n", control_entry.ping_ctl_test_name);
                /* end of DEBUG */

                BACKDOOR_MGR_Printf("(1)set ctl entry rowstatus(2)Set admin status (3)Set target addr type (4)Set target addr (5)Set prob count(6) Set data size\n");
                /* request for operation */
                BACKDOOR_MGR_Printf("operation: \n");
                BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                PING_MGR_StrToVal(buf, &operation);

                switch (operation)
                {
                    case 1:
                        /* request for value */
                        BACKDOOR_MGR_Printf("\nvalue: \n");
                        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                        PING_MGR_StrToVal(buf, &value);
                        BACKDOOR_MGR_Printf("\n");
                        if ((res = PING_MGR_SetCtlRowStatus(&control_entry, value)) == PING_TYPE_OK)
                        {
                            BACKDOOR_MGR_Printf("\nSet rowstatus OK.\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nSet rowstatus failed.\n");
                        }
                        break;
                    case 2:
                        /* request for value */
                        BACKDOOR_MGR_Printf("\nvalue: \n");
                        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                        PING_MGR_StrToVal(buf, &value);
                        if ((res = PING_MGR_SetCtlAdminStatus(&control_entry, value)) == PING_TYPE_OK)
                        {
                            BACKDOOR_MGR_Printf("\nSet admin status OK.\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nSet admin status failed.\n");
                        }
                        break;

                    /* case 3:
                        if ((res = PING_MGR_SetCtlTargetAddressType(&control_entry, value)) == PING_TYPE_OK)
                        {
                            printf("Set OK.\n");
                        }
                        else
                        {
                            printf("Set failed.\n");
                        }
                        break;
                    */
                    case 4:
                        /* request for value */
                        BACKDOOR_MGR_Printf("\ntarget ipv4/v6 address (A.B.C.D or X:X:X:X::X[scope_id]) = \n");

                        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                        memset(&inet_addr, 0 , sizeof(L_INET_AddrIp_T));
                        L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, buf, (L_INET_Addr_T *)&inet_addr, sizeof(inet_addr));

                        if ((res = PING_MGR_SetCtlTargetAddress(&control_entry, &inet_addr)) == PING_TYPE_OK)
                        {
                            BACKDOOR_MGR_Printf("\nSet OK.\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nSet failed.\n");
                        }
                        break;
                     case 5:
                        /* request for value */
                        BACKDOOR_MGR_Printf("\nvalue: \n");
                        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                        PING_MGR_StrToVal(buf, &value);
                        if ((res = PING_MGR_SetCtlProbeCount(&control_entry, value)) == PING_TYPE_OK)
                        {
                            BACKDOOR_MGR_Printf("\nSet OK.\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nSet failed.\n");
                        }
                        break;
                     case 6:
                        /* request for value */
                        BACKDOOR_MGR_Printf("\nvalue: \n");
                        BACKDOOR_MGR_RequestKeyIn(buf, MAX_BUF_LEN - 1);
                        PING_MGR_StrToVal(buf, &value);
                        if ((res = PING_MGR_SetCtlDataSize(&control_entry, value)) == PING_TYPE_OK)
                        {
                            BACKDOOR_MGR_Printf("\nSet OK.\n");
                        }
                        else
                        {
                            BACKDOOR_MGR_Printf("\nSet failed.\n");
                        }
                        break;

                     default:
                        BACKDOOR_MGR_Printf("\ninvalid operation.\n");

                } /* switch */

                break;
            case 4:
                memset(owner_index, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memset(test_name, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                BACKDOOR_MGR_Printf("Input Owner index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
                BACKDOOR_MGR_Printf("\n");
                BACKDOOR_MGR_Printf("Input Test index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(test_name, SYS_ADPT_PING_MAX_NAME_SIZE);
                BACKDOOR_MGR_Printf("\n");


                memcpy(result_entry.ping_ctl_owner_index, owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(result_entry.ping_ctl_test_name, test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                /* DEBUG*/
                BACKDOOR_MGR_Printf("owner_index: %s\n", result_entry.ping_ctl_owner_index);
                BACKDOOR_MGR_Printf("test_name: %s\n", result_entry.ping_ctl_test_name);
                /* end of DEBUG */

                if ((res = PING_OM_GetResultsEntry(&result_entry)) == PING_TYPE_OK)
                {
                    PING_MGR_PrintResultEntryInfo(&result_entry);
                }
                else
                {
                    BACKDOOR_MGR_Printf("get result entry failed.\n");
                }

                break;
            case 5:

                memset(owner_index, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memset(test_name, 0, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                BACKDOOR_MGR_Printf("Input Owner index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
                BACKDOOR_MGR_Printf("\n");
                BACKDOOR_MGR_Printf("Input Test index (in string format): \n");
                BACKDOOR_MGR_RequestKeyIn(test_name, SYS_ADPT_PING_MAX_NAME_SIZE);
                BACKDOOR_MGR_Printf("\n");


                memcpy(result_entry.ping_ctl_owner_index, owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
                memcpy(result_entry.ping_ctl_test_name, test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

                /* DEBUG*/
                BACKDOOR_MGR_Printf("owner_index: %s\n", result_entry.ping_ctl_owner_index);
                BACKDOOR_MGR_Printf("test_name: %s\n", result_entry.ping_ctl_test_name);
                /* end of DEBUG */

                PING_MGR_DisplayPingResult(result_entry);
                break;
            case 6:
                PING_MGR_PrintAllProbeHistory();
                break;
            case 7:
                if(ping_backdoor_debug == FALSE)
                {
                    ping_backdoor_debug = TRUE;
                }
                else
                {
                    ping_backdoor_debug = FALSE;
                }

                break;
            case 8:
                PING_MGR_DisplayAllWorkspaceDetail();
                break;
            case 9:
                PING_MGR_DisplayKeyIndexTable();
                break;

            default :
                break;
        }
    }   /*  end of while    */
}   /*  end of PING_MGR_BackDoor_Menu */

#endif  /*  end of #if BACKDOOR_OPEN    */


/**************************************************************************
IPCHKSUM - Checksum IP Header
**************************************************************************/
static unsigned short ipchksum(unsigned short *ip, int len)
{
    unsigned long sum = 0;
    len >>= 1;
    while (len--) {
        sum += *(ip++);
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return(((unsigned short)(~sum) & 0x0000FFFF));
}

/* FUNCTION NAME : PING_MGR_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for PING_MGR.
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
BOOL_T PING_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    PING_MGR_IPCMsg_T *ping_mgr_msg_p;
    BOOL_T need_respond=TRUE;

    if(ipcmsg_p==NULL)
        return FALSE;

    ping_mgr_msg_p = (PING_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        ping_mgr_msg_p->type.result_ui32 = PING_TYPE_FAIL;
        ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
        need_respond = TRUE;
        return need_respond;
    }

    switch(ping_mgr_msg_p->type.cmd)
    {
        case PING_MGR_IPCCMD_SETCTLENTRY:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlEntry(&(ping_mgr_msg_p->data.ctrl_entry));
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case PING_MGR_IPCCMD_SETCTLADMINSTATUS:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlAdminStatus(&(ping_mgr_msg_p->data.ctrl_entry_ui32.ctrl_entry), ping_mgr_msg_p->data.ctrl_entry_ui32.ui32);
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case PING_MGR_IPCCMD_SETCTLTARGETADDRESS:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlTargetAddress(&(ping_mgr_msg_p->data.ctrl_entry_addr.ctrl_entry), &(ping_mgr_msg_p->data.ctrl_entry_addr.addr));
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case PING_MGR_IPCCMD_SETCTLDATASIZE:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlDataSize(&(ping_mgr_msg_p->data.ctrl_entry_ui32.ctrl_entry), ping_mgr_msg_p->data.ctrl_entry_ui32.ui32);
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        case PING_MGR_IPCCMD_SETCTLPROBECOUNT:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlProbeCount(&(ping_mgr_msg_p->data.ctrl_entry_ui32.ctrl_entry), ping_mgr_msg_p->data.ctrl_entry_ui32.ui32);
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case PING_MGR_IPCCMD_SETCTLROWSTATUS:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlRowStatus(&(ping_mgr_msg_p->data.ctrl_entry_ui32.ctrl_entry), ping_mgr_msg_p->data.ctrl_entry_ui32.ui32);
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;
        case PING_MGR_IPCCMD_SETCTLENTRYBYFIELD:
            ping_mgr_msg_p->type.result_ui32 = PING_MGR_SetCtlEntryByField(
            &(ping_mgr_msg_p->data.ctrl_entry_field.ctrl_entry),
            ping_mgr_msg_p->data.ctrl_entry_field.field);

            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
            need_respond = TRUE;
            break;

        default:
            printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            ping_mgr_msg_p->type.result_ui32 = PING_TYPE_FAIL;
            ipcmsg_p->msg_size = PING_MGR_MSGBUF_TYPE_SIZE;
    }

    return need_respond;

}
