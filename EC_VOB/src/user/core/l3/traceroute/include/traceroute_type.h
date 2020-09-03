/* -------------------------------------------------------------------------
 * FILE NAME - traceroute_type.h
 * -------------------------------------------------------------------------
 * Purpose: This package provides the datatype declaration define in RFC2925 MIB
 * Notes: None
 *
 *
 * Modification History:
 *   By            Date      Ver.    Modification Description
 * ------------ ----------   -----   ---------------------------------------
 *   Amytu       2003-07-01          First Created
 * -------------------------------------------------------------------------
 * Copyright(C)         ACCTON Technology Corp., 2003
 * -------------------------------------------------------------------------
 */

#ifndef     _TRACEROUTE_TYPE_H
#define     _TRACEROUTE_TYPE_H


#include "leaf_2925.h"
#include "sys_adpt.h"
#include "l_inet.h"

const static UI8_T traceroute_ipv4_zero_addr[SYS_ADPT_IPV4_ADDR_LEN] = {}; /* initial to all 0 */
#if (SYS_CPNT_IPV6 == TRUE)
const static UI8_T traceroute_ipv6_zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {};
#endif

#define  TRACEROUTE_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI    "SYSTEM_CLI"
#define  TRACEROUTE_TYPE_CTL_OWNER_INDEX_SYSTEM_WEB    "SYSTEM_WEB"

/*  --  Calling Result  --  */
#define TRACEROUTE_TYPE_OK                                          0
#define TRACEROUTE_TYPE_INVALID_WORK_SPACE                          0x80000001
#define TRACEROUTE_TYPE_TIMEOUT                                     0x80000002
#define TRACEROUTE_TYPE_INVALID_ARG                                 0x80000003
#define TRACEROUTE_TYPE_NO_MORE_WORKSPACE                           0x80000004
#define TRACEROUTE_TYPE_NO_MORE_SOCKET                              0x80000005
#define TRACEROUTE_TYPE_CANNOT_SEND_PACKET                          0x80000006
#define TRACEROUTE_TYPE_OTHER_ERROR                                 0x80000007
#define TRACEROUTE_TYPE_FAIL                                        0x80000008
#define TRACEROUTE_TYPE_NO_MORE_MEMORY                              0x80000009
#define TRACEROUTE_TYPE_NO_MORE_ENTRY                               0x8000000A
#define TRACEROUTE_TYPE_DO_NOT_SEND                                 0x8000000B
#define TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE                        0x8000000C
#define TRACEROUTE_TYPE_NO_MORE_PROBE_TO_SEND                       0x8000000D
#define TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_BROADCAST_ADDR       0x8000000E
#define TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_NETWORK_ID           0x8000000F
#define TRACEROUTE_TYPE_IPV6_LINK_LOCAL_ADDR_INVALID_ZONE_ID        0x80000010

/* Return Code for TraceRoute Result
 */
#define TRACEROUTE_TYPE_NETWORK_UNREACHABLE      1
#define TRACEROUTE_TYPE_HOST_UNREACHABLE         2
#define TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE     3
#define TRACEROUTE_TYPE_FRAGMENTATION_NEEDED     4
#define TRACEROUTE_TYPE_EXCEED_MAX_TTL           5
#define TRACEROUTE_TYPE_PORT_UNREACHABLE         6
#define TRACEROUTE_TYPE_SRT_UNREACHABLE          7
#define TRACEROUTE_TYPE_NETWORK_UNKNOWN          8
#define TRACEROUTE_TYPE_HOST_UNKNOWN             9
#define TRACEROUTE_TYPE_NO_RESPONSE              19

typedef enum
{
    TRACEROUTE_CTLENTRYFIELD_OWNER_INDEX = 0,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TEST_NAME,
    TRACEROUTE_TYPE_CTLENTRYFIELD_OWNER_INDEX_LEN,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TEST_NAME_LEN,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TARGET_ADDRESS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_DATA_SIZE,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TIMEOUT,
    TRACEROUTE_TYPE_CTLENTRYFIELD_PROBES_PER_HOP,
    TRACEROUTE_TYPE_CTLENTRYFIELD_PORT,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_TTL,
    TRACEROUTE_TYPE_CTLENTRYFIELD_DS_FIELD,
    TRACEROUTE_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS_TYPE,
    TRACEROUTE_TYPE_CTLENTRYFIELD_UTE_CTL_SOURCE_ADDRESS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_IF_INDEX,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MISC_OPTIONS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MISC_OPTIONS_LEN,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_FAILURES,
    TRACEROUTE_TYPE_CTLENTRYFIELD_DONT_FRAGMENT,
    TRACEROUTE_TYPE_CTLENTRYFIELD_INITIAL_TTL,
    TRACEROUTE_TYPE_CTLENTRYFIELD_FREQUENCY,
    TRACEROUTE_TYPE_CTLENTRYFIELD_STORAGE_TYPE,
    TRACEROUTE_TYPE_CTLENTRYFIELD_ADMIN_STATUS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MAX_ROWS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TRAP_GENERATION,
    TRACEROUTE_TYPE_CTLENTRYFIELD_DESCR,
    TRACEROUTE_TYPE_CTLENTRYFIELD_DESCR_LEN,
    TRACEROUTE_TYPE_CTLENTRYFIELD_CREATE_HOPS_ENTRIES,
    TRACEROUTE_TYPE_CTLENTRYFIELD_TYPE,
    TRACEROUTE_TYPE_CTLENTRYFIELD_ROWSTATUS,
    TRACEROUTE_TYPE_CTLENTRYFIELD_MAX
} TRACEROUTE_TYPE_CtlEntryField_T;

typedef struct
{
    char    owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];    // KEY 1
    UI32_T  owner_index_len;
    char    test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];      // KEY 2
    UI32_T  test_name_len;
    UI32_T  table_index;    // Table index is 1:1 mapping to control and Result Table

} TRACEROUTE_SORTLST_ELM_T;

typedef struct
{
    char    trace_route_ctl_owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];    /* KEY 1 */
    UI32_T  trace_route_ctl_owner_index_len;
    char    trace_route_ctl_test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];        /* KEY 2 */
    UI32_T  trace_route_ctl_test_name_len;
    UI32_T  trace_route_ctl_target_address_type;
    L_INET_AddrIp_T trace_route_ctl_target_address;   /* ipv4/v6 address */

    UI32_T  trace_route_ctl_by_pass_route_table;
    UI32_T  trace_route_ctl_data_size;
    UI32_T  trace_route_ctl_timeout;
    UI32_T  trace_route_ctl_probes_per_hop;
    UI32_T  trace_route_ctl_port;
    UI32_T  trace_route_ctl_max_ttl;
    UI32_T  trace_route_ctl_ds_field;
    UI32_T  trace_route_ctl_source_address_type;
    L_INET_AddrIp_T trace_route_ctl_source_address;
    UI32_T  trace_route_ctl_if_index;
    UI8_T   trace_route_ctl_misc_options[SYS_ADPT_TRACEROUTE_MAX_MISC_OPTIONS_SIZE];
    UI32_T  trace_route_ctl_misc_options_len;
    UI32_T  trace_route_ctl_max_failures;
    UI32_T  trace_route_ctl_dont_fragment;
    UI32_T  trace_route_ctl_initial_ttl;
    UI32_T  trace_route_ctl_frequency;
    UI32_T  trace_route_ctl_storage_type;
    UI32_T  trace_route_ctl_admin_status;
    UI32_T  trace_route_ctl_max_rows;
    UI8_T   trace_route_ctl_trap_generation;
    UI8_T   trace_route_ctl_descr[SYS_ADPT_TRACEROUTE_MAX_MISC_OPTIONS_SIZE];
    UI32_T  trace_route_ctl_descr_len;
    UI32_T  trace_route_ctl_create_hops_entries;
    UI32_T  trace_route_ctl_type;
    UI32_T  trace_route_ctl_rowstatus;

} TRACEROUTE_TYPE_TraceRouteCtlEntry_T;


typedef struct
{
    char    trace_route_ctl_owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];    /* KEY 1 */
    UI32_T  trace_route_ctl_owner_index_len;
    char    trace_route_ctl_test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];        /* KEY 2 */
    UI32_T  trace_route_ctl_test_name_len;
    UI16_T  trace_route_results_oper_status;
    UI32_T  trace_route_results_cur_hop_count;
    UI32_T  trace_route_results_cur_probe_count;
    UI32_T  trace_route_results_ip_tgt_addr_type;
    L_INET_AddrIp_T trace_route_results_ip_tgt_addr;
    UI32_T  trace_route_results_test_attempts;
    UI32_T  trace_route_results_test_successes;
    UI8_T   trace_route_results_last_good_path[SIZE_traceRouteResultsLastGoodPath_2]; /*(DateAndTime)*/
    UI32_T  trace_route_results_last_good_path_len;
} TRACEROUTE_TYPE_TraceRouteResultsEntry_T;


typedef struct
{
    char    trace_route_ctl_owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];    /* KEY 1 */
    UI32_T  trace_route_ctl_owner_index_len;
    char    trace_route_ctl_test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];        /* KEY 2 */
    UI32_T  trace_route_ctl_test_name_len;
    UI32_T  trace_route_probe_history_index;        /* KEY 3 */
    UI32_T  trace_route_probe_history_hop_index;    /* KEY 4 */
    UI32_T  trace_route_probe_history_probe_index;  /* KEY 5 */
    UI32_T  trace_route_probe_history_haddr_type;
    L_INET_AddrIp_T trace_route_probe_history_haddr;
//    UI8_T   trace_route_probe_history_haddr[SYS_ADPT_TRACEROUTE_MAX_IP_ADDRESS_STRING_SIZE];
//    UI32_T  trace_route_probe_history_haddr_len;
    UI32_T  trace_route_probe_history_response;
    UI32_T  trace_route_probe_history_status;
    UI32_T  trace_route_probe_history_last_rc;
    UI8_T   trace_route_probe_history_time[SIZE_traceRouteProbeHistoryTime_2]; /*(DateAndTime)*/
    UI32_T  trace_route_probe_history_time_len;
} TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T;

typedef struct
{
    char    trace_route_ctl_owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];    /* KEY 1 */
    UI32_T  trace_route_ctl_owner_index_len;
    char    trace_route_ctl_test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1];        /* KEY 2 */
    UI32_T  trace_route_ctl_test_name_len;
    UI32_T  trace_route_hops_hop_index;
    UI32_T  trace_route_hops_ip_tgt_address_type;
    L_INET_AddrIp_T trace_route_hops_ip_tgt_address;
//    UI8_T   trace_route_hops_ip_tgt_address[SYS_ADPT_TRACEROUTE_MAX_IP_ADDRESS_STRING_SIZE];
//    UI32_T  trace_route_hops_ip_tgt_address_len;
    UI32_T  trace_route_hops_min_rtt;
    UI32_T  trace_route_hops_max_rtt;
    UI32_T  trace_route_hops_average_rtt;
    UI32_T  trace_route_hops_rtt_sum_of_squares;
    UI32_T  trace_route_hops_sent_probes;
    UI32_T  trace_route_hops_probe_responses;
    UI32_T  trace_route_hops_probe_history_time;     /*(DateAndTime)*/
    #if 0
    UI8_T   trace_route_hops_last_good_probe[SIZE_traceRouteHopsLastGoodProbe_2]; /*(DateAndTime)*/
    UI32_T  trace_route_hops_last_good_probe_len;
    #endif


} TRACEROUTE_TYPE_TraceRouteHopsEntry_T;

#endif
