/* Module Name: PING_TYPE.H
 * Purpose:
 *     This component implements the standard Ping-MIB functionality (rfc2925).
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *      2. Currently support IPv4 address only.
 *
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard ping-MIB.
 *      2007/12     --  peter_yu    Porting to linux platform.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef     _PING_TYPE_H
#define     _PING_TYPE_H


#include "sys_type.h"
#include "sys_adpt.h"
#include "l_inet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define  PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI    "SYSTEM_CLI"
#define  PING_TYPE_CTL_OWNER_INDEX_SYSTEM_WEB    "SYSTEM_WEB"

/* return code */
#define PING_TYPE_OK                                            0
#define PING_TYPE_INVALID_WORK_SPACE                            0x80000001
#define PING_TYPE_INVALID_ARG                                   0x80000002
#define PING_TYPE_NO_MORE_SOCKET                                0x80000003
#define PING_TYPE_FAIL                                          0x80000004
#define PING_TYPE_NO_MORE_ENTRY                                 0x80000005
#define PING_TYPE_NO_MORE_PROBE_TO_SEND                         0x80000006
#define PING_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID          0x80000007

#define PING_TYPE_NO_RESPONSE               19


const static UI8_T ipv4_zero_addr[SYS_ADPT_IPV4_ADDR_LEN] = {}; /* initial to all 0 */
#if (SYS_CPNT_IPV6 == TRUE)
const static UI8_T ipv6_zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {};
#endif


typedef enum
{
    PING_TYPE_CTLENTRYFIELD_OWNER_INDEX = 0,
    PING_TYPE_CTLENTRYFIELD_TEST_NAME,
    PING_TYPE_CTLENTRYFIELD_OWNER_INDEX_LEN,
    PING_TYPE_CTLENTRYFIELD_TEST_NAME_LEN,
    PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE,
    PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS,
    PING_TYPE_CTLENTRYFIELD_DATA_SIZE,
    PING_TYPE_CTLENTRYFIELD_TIMEOUT,
    PING_TYPE_CTLENTRYFIELD_PROBE_COUNT,
    PING_TYPE_CTLENTRYFIELD_ADMIN_STATUS,
    PING_TYPE_CTLENTRYFIELD_DATA_FILL,
    PING_TYPE_CTLENTRYFIELD_FREQUENCY,
    PING_TYPE_CTLENTRYFIELD_MAX_ROWS,
    PING_TYPE_CTLENTRYFIELD_STORAGE_TYPE,
    PING_TYPE_CTLENTRYFIELD_TRAP_GENERATION,
    PING_TYPE_CTLENTRYFIELD_TRAP_PROBE_FAILURE_FILTER,
    PING_TYPE_CTLENTRYFIELD_TRAP_TEST_FAILURE_FILTER,
    PING_TYPE_CTLENTRYFIELD_TYPE,
    PING_TYPE_CTLENTRYFIELD_DESCR,
    PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS_TYPE,
    PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS,
    PING_TYPE_CTLENTRYFIELD_IF_INDEX,
    PING_TYPE_CTLENTRYFIELD_BY_PASS_ROUTE_TABLE,
    PING_TYPE_CTLENTRYFIELD_DS_FIELD,
    PING_TYPE_CTLENTRYFIELD_DONT_FRAGMENT,
    PING_TYPE_CTLENTRYFIELD_ROWSTATUS
} PING_TYPE_CtlEntryField_T;

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


typedef struct
{
    char    owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];   /* key */
    char    test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];     /* key */
    UI32_T  table_index; /* Table index is 1:1 mapping to control and Result Table */
} PING_SORTLST_ELM_T;

typedef struct
{
    char    ping_ctl_owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];  /* key */
    char    ping_ctl_test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];    /* key */
    UI32_T  ping_ctl_owner_index_len;
    UI32_T  ping_ctl_test_name_len;
    UI32_T  ping_ctl_target_address_type;
    L_INET_AddrIp_T ping_ctl_target_address;   /* ipv4/v6/dns(?) address */
    UI32_T  ping_ctl_data_size;
    UI32_T  ping_ctl_timeout; /* by rfc2925, in seconds */
    UI32_T  ping_ctl_probe_count;
    UI32_T  ping_ctl_admin_status;
    UI8_T   ping_ctl_data_fill[SYS_ADPT_PING_MAX_DATA_FILL_SIZE];
    UI32_T  ping_ctl_frequency;
    UI32_T  ping_ctl_max_rows;
    UI32_T  ping_ctl_storage_type;
    UI8_T   ping_ctl_trap_generation;
    UI32_T  ping_ctl_trap_probe_failure_filter;
    UI32_T  ping_ctl_trap_test_failure_filter;
    UI32_T  ping_ctl_type;
    UI8_T   ping_ctl_descr[SYS_ADPT_PING_MAX_MISC_OPTIONS_SIZE];
    UI32_T  ping_ctl_source_address_type;
    L_INET_AddrIp_T  ping_ctl_source_address;    /* ipv4/v6/dns(?) address */
    UI32_T  ping_ctl_if_index;
    BOOL_T  ping_ctl_by_pass_route_table;
    UI32_T  ping_ctl_ds_field;
    BOOL_T  ping_ctl_dont_fragment; /* always set dont fragment bit in IP header */
    UI32_T  ping_ctl_rowstatus;
} PING_TYPE_PingCtlEntry_T;

typedef struct
{
    char    ping_ctl_owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];  /* key */   /* string */
    char    ping_ctl_test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];    /* key */   /* string */
    UI32_T  ping_ctl_owner_index_len;
    UI32_T  ping_ctl_test_name_len;
    UI32_T  ping_results_oper_status;
    UI32_T  ping_results_ip_target_address_type;
    L_INET_AddrIp_T   ping_results_ip_target_address; /* ipv4/v6/dns(?) address */
    UI32_T  ping_results_min_rtt;       /* by rfc2925, in milliseconds */
    UI32_T  ping_results_max_rtt;       /* by rfc2925, in milliseconds */
    UI32_T  ping_results_average_rtt;   /* by rfc2925, in milliseconds */
    UI32_T  ping_results_probe_responses;
    UI32_T  ping_results_sent_probes;
    UI32_T  ping_results_rtt_sum_of_squares;
    UI8_T   ping_results_last_good_probe[11]; /* day-time string */
    UI32_T  ping_results_last_good_probe_len;
} PING_TYPE_PingResultsEntry_T;

typedef struct
{
    char    ping_ctl_owner_index[SYS_ADPT_PING_MAX_NAME_SIZE + 1];  /* key */   /* string */
    char    ping_ctl_test_name[SYS_ADPT_PING_MAX_NAME_SIZE + 1];    /* key */   /* string */
    UI32_T  ping_ctl_owner_index_len;
    UI32_T  ping_ctl_test_name_len;
    UI32_T  ping_probe_history_index;                           /* key */
/* support mcast */
    UI32_T  type;                           /* type: unicast(1), multicast (2) */
    L_INET_AddrIp_T src_addr;
    UI32_T  system_tick;  /* system tick time stamp*/
/* support mcast */
    UI32_T  icmp_sequence;                  /* ICMP/ICMP6 sequence number */
    UI32_T  ping_probe_history_response; /* by rfc2925, in milliseconds */
    UI32_T  ping_probe_history_status;
    UI32_T  ping_probe_history_last_rc;
    UI8_T   ping_probe_history_time[11]; /* day-time string */
    UI32_T  ping_probe_history_time_len;
} PING_TYPE_PingProbeHistoryEntry_T;


#endif
