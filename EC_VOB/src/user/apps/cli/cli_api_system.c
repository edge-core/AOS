#include <ctype.h>
#include <stdio.h>
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "cli_api.h"
#include "cli_api_system.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "telnet_mgr.h"
#include "telnet_pmgr.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_ip.h"
#include "l_inet.h"
#include "stkctrl_pmgr.h"
#include "l_md5.h"
#include "sys_time.h"
#include "netcfg_pmgr_route.h"
#if (SYS_CPNT_PING == TRUE)
#include "leaf_2925p.h" /* ping MIB */
#include "ping_type.h"  /* ping MIB */
#include "ping_pmgr.h"  /* ping MIB */
#include "ping_pom.h"   /* ping MIB */
#include "ip_lib.h"     /* ping MIB */
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
#include "traceroute_pmgr.h"
#include "traceroute_pom.h"
#endif

#if 1 // TODO: wakka: wait NETCFG_PMGR_GetBestRoutingInterface() ready
#include "vlan_lib.h"
#include "vlan_pom.h"
#endif

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#endif

#if (SYS_CPNT_MOTD == TRUE)
#include "cli_mgr.h"
#endif /*#if (SYS_CPNT_MOTD == TRUE)*/

#include "sys_pmgr.h"
#include "ipal_if.h" /* for CLI_API_Show_Ip_Traffic */
#include "ipal_types.h"

#if (SYS_CPNT_SYNCE == TRUE)
#include "l_pbmp.h"
#include "sync_e_type.h"
#include "syncedrv_type.h"
#include "syncedrv_om.h"
#include "stktplg_board.h"
#include "sync_e_pmgr.h"
#endif

#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
#include "led_pmgr.h"
#endif

static UI32_T AtoIp(UI8_T *hostip_p, UI8_T *ip_p);

enum
{
    ATOIP_OK = 0,
    ATOIP_INVALID_IP,
    ATOIP_NON_IP_STRING,
};


/* DATA TYPE DECLARATIONS
 */
enum
{
    HOSTSTRING2IP_OK = 0,
    HOSTSTRING2IP_INVALID_IP,
    HOSTSTRING2IP_NON_EXISTENT_DOMAIIN,
    HOSTSTRING2IP_DNS_REQUEST_TIMED_OUT,
    HOSTSTRING2IP_NO_RESPONSE_FROM_SERVER,
};

/* command : ping */
/* FUNCTION NAME: CLI_API_Ping
 * PURPOSE:
 *          ping to a target IP address, based on rfc2925 (ping-MIB).
 * INPUT:
 *          ctrl_P
 * OUTPUT:
 *          None
 * RETURN:
 *          CLI_NO_ERROR
 * NOTES:
 *          1. create ping ctl entry and set admin_status to enabled to ping.
 */
UI32_T CLI_API_Ping(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    /* Local Variable Declaration
     */

#define INIT_MAX_DELTA_TIME 0
#define INIT_MIN_DELTA_TIME 0xffffffff

    UI32_T count, ret;
    UI32_T size, i;
    UI32_T timeout = SYS_DFLT_PING_CTL_TIME_OUT; /* in system tick unit */
    UI32_T success_num = 0;
    UI32_T send_num = 0;
    UI32_T max_delta_time = INIT_MAX_DELTA_TIME;
    UI32_T min_delta_time = INIT_MIN_DELTA_TIME;
    UI32_T total_success_delta_time = 0;
    PING_TYPE_PingCtlEntry_T    ctl_entry;
    PING_TYPE_PingResultsEntry_T result_entry;
    PING_TYPE_PingProbeHistoryEntry_T history_entry, printed_history_entry;
    //UI32_T count_printed = 0;
    UI32_T pom_rc;
    BOOL_T dont_fragment = FALSE;
    L_INET_AddrIp_T inet_addr;
    UI32_T start_time;
    UI8_T session_id;

    /* BODY
     */
    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    /*to check the input is IPv4 format*/
    if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_addr,
                                                       sizeof(inet_addr))
        ||
        L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_addr,
                                                       sizeof(inet_addr))
        )
    {
        CLI_LIB_PrintStr_1("Invalid IPv4 address - %s\r\n", arg[0]);
        return CLI_NO_ERROR;
    }

    size  = SYS_ADPT_MIN_PING_SIZE; /* 32 */
    count = 5;

    for(i=1; arg[i]; )
    {
        if (arg[i][0] == 'c') /* count */
        {
            count = atoi(arg[i+1]);
            i+=2;
        }
        else if (arg[i][0] == 's') /* size */
        {
            size = atoi(arg[i+1]);
            i+=2;
        }
        else if(!strncmp(arg[i], "dont-fragment", 4))
        {
            dont_fragment = TRUE;
            i+=1;
        }
    } /* for */

    /* for multiple telnet console, they should use different owner_index and test_name from taskid. */
    session_id = CLI_TASK_GetMySessId();

    /* DEBUG */
    //printf("%s, %d, default_string: %s\n", __FUNCTION__, __LINE__, default_string);

    /* initialize entry */
    memset(&ctl_entry, 0 , sizeof(PING_TYPE_PingCtlEntry_T));
    memset(&result_entry, 0 , sizeof(PING_TYPE_PingResultsEntry_T));
    memset(&history_entry, 0 , sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    strncpy(ctl_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(ctl_entry.ping_ctl_owner_index) - 1);
    ctl_entry.ping_ctl_owner_index[sizeof(ctl_entry.ping_ctl_owner_index) - 1] = '\0';
    ctl_entry.ping_ctl_owner_index_len = strlen((char *)ctl_entry.ping_ctl_owner_index);

    snprintf((char *)ctl_entry.ping_ctl_test_name, sizeof(ctl_entry.ping_ctl_test_name),
        "%d", session_id);
    ctl_entry.ping_ctl_test_name[sizeof(ctl_entry.ping_ctl_test_name) - 1] = '\0';
    ctl_entry.ping_ctl_test_name_len = strlen((char *)ctl_entry.ping_ctl_test_name);

    strncpy(result_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(result_entry.ping_ctl_owner_index) - 1);
    result_entry.ping_ctl_owner_index[sizeof(result_entry.ping_ctl_owner_index) - 1] = '\0';
    result_entry.ping_ctl_owner_index_len = strlen((char *)result_entry.ping_ctl_owner_index);

    snprintf((char *)result_entry.ping_ctl_test_name, sizeof(result_entry.ping_ctl_test_name),
        "%d", session_id);
    result_entry.ping_ctl_test_name[sizeof(result_entry.ping_ctl_test_name) - 1] = '\0';
    result_entry.ping_ctl_test_name_len = strlen((char *)result_entry.ping_ctl_test_name);

    /* initialize history entry */
    strncpy(history_entry.ping_ctl_owner_index,
        PING_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(history_entry.ping_ctl_owner_index) - 1);
    history_entry.ping_ctl_owner_index[sizeof(history_entry.ping_ctl_owner_index) - 1] = '\0';
    history_entry.ping_ctl_owner_index_len = strlen((char *)history_entry.ping_ctl_owner_index);

    snprintf((char *)history_entry.ping_ctl_test_name, sizeof(history_entry.ping_ctl_test_name),
        "%d", session_id);
    history_entry.ping_ctl_test_name[sizeof(history_entry.ping_ctl_test_name) - 1] = '\0';
    history_entry.ping_ctl_test_name_len = strlen((char *)history_entry.ping_ctl_test_name);

    history_entry.ping_probe_history_index = 0;
    memcpy(&printed_history_entry, &history_entry, sizeof(history_entry));


    /* admin status is disabled firstly. */
    ctl_entry.ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
    /* size */
    ctl_entry.ping_ctl_data_size = size;
    /* count */
    ctl_entry.ping_ctl_probe_count = count;

    /* address type */
    ctl_entry.ping_ctl_target_address_type = VAL_pingCtlTargetAddressType_ipv4;

    /* default time-out 5 sec.*/
    ctl_entry.ping_ctl_timeout = (SYS_DFLT_PING_CTL_TIME_OUT / SYS_BLD_TICKS_PER_SECOND);

    /* dont fragment */
    ctl_entry.ping_ctl_dont_fragment = dont_fragment;

    /* create entry */
    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_createAndGo;

    if(PING_TYPE_OK != PING_PMGR_SetCtlEntry(&ctl_entry))
    {
        CLI_LIB_PrintStr("Not enough resource; Please try later\r\n");

        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }
    /* display message */
    CLI_LIB_PrintStr("Press \"ESC\" to abort.\r\n");

    CLI_LIB_PrintStr_1("Ping to %s", arg[0]);

    memset(&inet_addr, 0, sizeof(inet_addr));

    start_time = SYSFUN_GetSysTick();

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_addr,
                                                       sizeof(inet_addr)))
    {
#if (SYS_CPNT_DNS == TRUE)
        DNS_Nslookup_CTRL_T nslookup_ctl_entry;
        DNS_Nslookup_Result_T nslookup_result_entry;
        UI32_T nslookup_ctl_index;
        UI32_T nslookup_result_index;

        memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

        strncpy((char *)nslookup_ctl_entry.CtlOwnerIndex,
            DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI,
            sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1);
        nslookup_ctl_entry.CtlOwnerIndex[sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1] = '\0';
        nslookup_ctl_entry.CtlOwnerIndexLen = strlen((char *)nslookup_ctl_entry.CtlOwnerIndex);

        snprintf((char *)nslookup_ctl_entry.OperationName, sizeof(nslookup_ctl_entry.OperationName),
            "%d", session_id);
        nslookup_ctl_entry.OperationName[sizeof(nslookup_ctl_entry.OperationName) - 1] = '\0';
        nslookup_ctl_entry.OperationNameLen = strlen((char *)nslookup_ctl_entry.OperationName);

        strncpy((char *)nslookup_ctl_entry.TargetAddress,
            arg[0],
            sizeof(nslookup_ctl_entry.TargetAddress)-1);
        nslookup_ctl_entry.TargetAddress[sizeof(nslookup_ctl_entry.TargetAddress) - 1] = '\0';
        nslookup_ctl_entry.TargetAddressLen = strlen((char *)nslookup_ctl_entry.TargetAddress);

        nslookup_ctl_entry.af_family = AF_INET;

        if (DNS_OK != DNS_PMGR_CreateSystemNslookupCtlEntry(&nslookup_ctl_entry, &nslookup_ctl_index))
        {
            CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
            ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
            PING_PMGR_SetCtlEntry(&ctl_entry);
            return CLI_NO_ERROR;
        }

        while (1)
        {
            if (CLI_IO_ReadACharNoWait() == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from Telnet rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharNoWait(ctrl_P);
                if (tmp_char!=0)
                {
                    if (tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharNoWait(ctrl_P);
                    }
                }

                /* silently destroy ctl entry
                 */
                CLI_LIB_Printf("\r\n");
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

            if (DNS_OK == DNS_PMGR_GetNslookupCtlEntryByIndex(nslookup_ctl_index, &nslookup_ctl_entry))
            {
                if (NSLOOKUP_OPSTATUS_COMPLETED == nslookup_ctl_entry.OperStatus)
                {
                    if (DNS_OK == nslookup_ctl_entry.Rc)
                    {
                        nslookup_result_index = 0;
                        memset(&nslookup_result_entry, 0, sizeof(nslookup_result_entry));

                        if (DNS_OK == DNS_PMGR_GetNslookupResultEntryByIndex(
                            nslookup_ctl_index, nslookup_result_index, &nslookup_result_entry))
                        {
                            memcpy(&inet_addr, &nslookup_result_entry.ResultsAddress_str, sizeof(inet_addr));
                            DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                            break;
                        }
                    }

                    CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                    DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                    PING_PMGR_SetCtlEntry(&ctl_entry);
                    return CLI_NO_ERROR;
                }
            }
            else
            {
                CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
            if((SYSFUN_GetSysTick() - start_time) > timeout * (count+1))
            {
                CLI_LIB_PrintStr_1("\r\nTimeout for lookup host name %s.\r\n", arg[0]);
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
                PING_PMGR_SetCtlEntry(&ctl_entry);
                return CLI_NO_ERROR;
            }

            SYSFUN_Sleep(50);
        }

        /* display ip address got from DNS */
        CLI_LIB_PrintStr_4(" [%d.%d.%d.%d],", inet_addr.addr[0], inet_addr.addr[1], inet_addr.addr[2], inet_addr.addr[3]);
#else
        /* No DNS
         */
        CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
#endif
    }

    ret = PING_PMGR_SetCtlTargetAddress(&ctl_entry, &inet_addr);
    if(PING_TYPE_OK != ret)
    {
        CLI_LIB_PrintStr("\r\nInvalid IP address.\r\n");
        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_3(" by %lu %lu-byte payload ICMP packets, timeout is %lu seconds\r\n", (unsigned long)count, (unsigned long)size, (unsigned long)(timeout / SYS_BLD_TICKS_PER_SECOND));

    /* enable admin status to start to ping.*/
    ret = PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_enabled);
    if(PING_TYPE_OK != ret)
    {
        /* silently destroy ctl entry */
        ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
        PING_PMGR_SetCtlEntry(&ctl_entry);
        return CLI_NO_ERROR;
    }

    //count_printed = 1;
    while(1)
    {
        /* break when ESC is pressed */
        if(ctrl_P->sess_type == CLI_TYPE_TELNET)
        {
            if( CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from Telnet rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharFromTelnet(ctrl_P);
                if(tmp_char!=0)
                {
                    if(tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharFromTelnet(ctrl_P);
                    }
                }
                PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_disabled);
                break;
            }
        }
        else
        {
            if( CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from UART rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharFromConsole(ctrl_P);
                if(tmp_char!=0)
                {
                    if(tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharFromConsole(ctrl_P);
                    }
                }
                PING_PMGR_SetCtlAdminStatus(&ctl_entry, VAL_pingCtlAdminStatus_disabled);
                break;
            }
        }
        pom_rc = PING_POM_GetCtlEntry(&ctl_entry);
        //CLI_LIB_PrintStr_3("%s, %d, pom_rc: %ld\n", __FUNCTION__, __LINE__, pom_rc);
        if(PING_TYPE_OK == pom_rc)
        {
            while (PING_POM_GetNextProbeHistoryEntryForCli(&history_entry) == PING_TYPE_OK)
            {

                if ((0 == strcmp(history_entry.ping_ctl_owner_index, ctl_entry.ping_ctl_owner_index)) &&
                    (0 == strcmp(history_entry.ping_ctl_test_name, ctl_entry.ping_ctl_test_name)))
                {
                    if(VAL_pingProbeHistoryStatus_responseReceived == history_entry.ping_probe_history_status)
                    {
                        /* accumulate total_success_delta_time */
                        total_success_delta_time += history_entry.ping_probe_history_response;
                        CLI_LIB_PrintStr_1("response time: %lu ms\r\n", (unsigned long)history_entry.ping_probe_history_response);
                    }
                    else if(VAL_pingProbeHistoryStatus_internalError == history_entry.ping_probe_history_status)

                    {
                        /* internal error */
                        CLI_LIB_PrintStr("internal error.\r\n");
                    }
                    else
                    {
                        /* time out */
                        CLI_LIB_PrintStr("timeout.\r\n");
                    }
                    memcpy(&printed_history_entry, &history_entry, sizeof(history_entry));
                }
                else
                {
                    memcpy(&history_entry, &printed_history_entry, sizeof(history_entry));
                }
            } /* while */

            /* print the last entry and stop */
            if (history_entry.ping_probe_history_index >= ctl_entry.ping_ctl_probe_count)
                break;

        } /* if PING_MGR_GetCtlEntry */
        /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
        if((SYSFUN_GetSysTick() - start_time) > timeout * (count+1))
        {
            break;
        }

        /* sleep for a while.  Because it is not necessary to query too frequently. */
        SYSFUN_Sleep(30);

    } /* while */

    /* display result and statistic */

    if(PING_TYPE_OK == PING_POM_GetResultsEntry(&result_entry))
    {

        send_num = result_entry.ping_results_sent_probes;
        success_num = result_entry.ping_results_probe_responses;
        min_delta_time = result_entry.ping_results_min_rtt;
        max_delta_time = result_entry.ping_results_max_rtt;

        CLI_LIB_PrintStr_1("Ping statistics for %s: \r\n", arg[0]);
        CLI_LIB_PrintStr_1(" %lu packets transmitted,", (unsigned long)send_num);
        CLI_LIB_PrintStr_4(" %lu packets received (%lu%%), %lu packets lost (%lu%%)\r\n",
                (unsigned long)success_num,
                (unsigned long)(send_num?success_num*100/send_num:0),
                (unsigned long)((send_num > success_num)? (send_num - success_num): 0),
                (unsigned long)((send_num > success_num)?(send_num - success_num)*100/send_num:0));

        CLI_LIB_PrintStr("Approximate round trip times: \r\n");
        CLI_LIB_PrintStr_3(" Minimum = %lu ms, Maximum = %lu ms, Average = %lu ms\r\n", (unsigned long)(min_delta_time == INIT_MIN_DELTA_TIME ? 0 : min_delta_time),
                (unsigned long)(max_delta_time == INIT_MAX_DELTA_TIME ? 0 : max_delta_time),
                (unsigned long)(success_num == 0 ? 0: (UI32_T)(total_success_delta_time/success_num)));


    }
    else
    {
        CLI_LIB_PrintStr("Unknown error, get result entry error.\r\n");
    }

    /* destroy ctl entry */
    ctl_entry.ping_ctl_rowstatus = VAL_pingCtlRowStatus_destroy;
    if(PING_TYPE_OK != PING_PMGR_SetCtlEntry(&ctl_entry))
    {
        CLI_LIB_PrintStr("Delete Ping Entry failed.\r\n");
        return CLI_NO_ERROR;
    }

    return CLI_NO_ERROR;
}

char *cli_api_get_tracert_code_tag(UI32_T code)
{
    switch (code)
    {
        case TRACEROUTE_TYPE_NETWORK_UNREACHABLE:
            return "!N";
        case TRACEROUTE_TYPE_HOST_UNREACHABLE:
            return "!H";
        case TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE:
            return "!P";
        case TRACEROUTE_TYPE_FRAGMENTATION_NEEDED:
            return "!F";
        case TRACEROUTE_TYPE_NO_RESPONSE:
            return "*";
        case TRACEROUTE_TYPE_EXCEED_MAX_TTL:
        case TRACEROUTE_TYPE_PORT_UNREACHABLE:
        case TRACEROUTE_TYPE_SRT_UNREACHABLE:
        case TRACEROUTE_TYPE_NETWORK_UNKNOWN:
        case TRACEROUTE_TYPE_HOST_UNKNOWN:
        default:
            return "!O";
    }
}

char *cli_api_get_tracert_code_str(UI32_T code)
{
    switch (code)
    {
        case TRACEROUTE_TYPE_NETWORK_UNREACHABLE:
            return "Network Unreachable";
        case TRACEROUTE_TYPE_HOST_UNREACHABLE:
            return "Host Unreachable";
        case TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE:
            return "Protocol Unreachable";
        case TRACEROUTE_TYPE_FRAGMENTATION_NEEDED:
            return "Fragmentation Needed";
        case TRACEROUTE_TYPE_NO_RESPONSE:
            return "No Response";
        case TRACEROUTE_TYPE_EXCEED_MAX_TTL:
        case TRACEROUTE_TYPE_PORT_UNREACHABLE:
        case TRACEROUTE_TYPE_SRT_UNREACHABLE:
        case TRACEROUTE_TYPE_NETWORK_UNKNOWN:
        case TRACEROUTE_TYPE_HOST_UNKNOWN:
        default:
            return "Other";
    }
}

char *cli_api_get_tracert_status_str(
        char    *buff_p,
        UI32_T  *code_bmp_p,
        UI32_T  code,
        UI32_T  resp)
{
    if (NULL != buff_p)
    {
        switch(code)
        {
            case TRACEROUTE_TYPE_PORT_UNREACHABLE:
            case TRACEROUTE_TYPE_EXCEED_MAX_TTL:
                if (resp > 0)
                    sprintf(buff_p,"%lu ms", (unsigned long)resp);
                else
                    sprintf(buff_p,"<10 ms");
                break;
            default:
                *code_bmp_p |= 1 << code;
                sprintf(buff_p, "%s", cli_api_get_tracert_code_tag(code));
                break;
        }
    }

    return buff_p;
}

void cli_api_show_tracert_code_bmp(UI32_T code_bmp)
{
    UI32_T  dsp_order_bit[] =
    {
        (1 << TRACEROUTE_TYPE_NO_RESPONSE),
        (1 << TRACEROUTE_TYPE_FRAGMENTATION_NEEDED),
        (1 << TRACEROUTE_TYPE_HOST_UNREACHABLE),
        (1 << TRACEROUTE_TYPE_NETWORK_UNREACHABLE),
        (1 << TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE),
        ~(
                (1 << TRACEROUTE_TYPE_NO_RESPONSE)         |
                (1 << TRACEROUTE_TYPE_FRAGMENTATION_NEEDED)|
                (1 << TRACEROUTE_TYPE_HOST_UNREACHABLE)    |
                (1 << TRACEROUTE_TYPE_NETWORK_UNREACHABLE) |
                (1 << TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE)
         )
    };

    UI32_T  dsp_order_code[] =
    {
        TRACEROUTE_TYPE_NO_RESPONSE,
        TRACEROUTE_TYPE_FRAGMENTATION_NEEDED,
        TRACEROUTE_TYPE_HOST_UNREACHABLE,
        TRACEROUTE_TYPE_NETWORK_UNREACHABLE,
        TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE,
        0xffff
    };

    UI32_T  code_max = (sizeof(dsp_order_bit) / sizeof(UI32_T));
    UI32_T  idx;
    char    buff[80];
    BOOL_T  is_fst = TRUE;

    /* Sample output:
     *
     *  Codes:
     *    * - No Response
     */
    for (idx = 0; ((code_bmp != 0) && (idx < code_max)); idx++)
    {
        if (code_bmp & dsp_order_bit[idx])
        {
            if (TRUE == is_fst)
            {
                CLI_LIB_PrintStr("Codes:\r\n");
                is_fst = FALSE;
            }

            sprintf(buff, "%3s - %s\r\n",
                    cli_api_get_tracert_code_tag(dsp_order_code[idx]),
                    cli_api_get_tracert_code_str(dsp_order_code[idx]));

            CLI_LIB_PrintStr(buff);

            code_bmp &= (~dsp_order_bit[idx]);
        }
    }
}

UI32_T CLI_API_TraceRoute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRACEROUTE == TRUE)
    UI32_T  owner_index_len = 0;
    UI32_T  test_name_len = 0;
    UI32_T  prev_hop = 0, ret = 0;
    UI32_T  i = 0, code_bmp = 0;
    UI16_T  key_ret = 0;
    char   owner_index[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1] = {0};
    char   test_name[SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE + 1] = {0};
    char    trace_status[22] = {0};
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T          ctrl_entry;
    TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T prob_history_entry;
    L_INET_AddrIp_T inet_target_addr;
    char    ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    char    buff[CLI_DEF_MAX_BUFSIZE];
    BOOL_T  has_ip = FALSE;
    UI32_T start_time;
    UI32_T  ctl_timeout, ctl_probes_per_hop;

    ctl_timeout = SYS_DFLT_TRACEROUTE_CTL_TIME_OUT; /* in system tick unit */
    ctl_probes_per_hop = SYS_DFLT_TRACEROUTE_CTL_PROBES_PER_HOP;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    /*to check the input is IPv4 format*/
    if (L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV6_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_target_addr,
                                                       sizeof(inet_target_addr))
        ||
        L_INET_RETURN_SUCCESS == L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_target_addr,
                                                       sizeof(inet_target_addr))
        )
    {
        CLI_LIB_PrintStr_1("Invalid IPv4 address - %s\r\n", arg[0]);
        return CLI_NO_ERROR;
    }

    strncpy(owner_index,
        TRACEROUTE_TYPE_CTL_OWNER_INDEX_SYSTEM_CLI,
        sizeof(owner_index) - 1);
    owner_index[sizeof(owner_index) - 1] = '\0';
    owner_index_len = strlen(owner_index);

    snprintf((char *)test_name, sizeof(test_name),
        "%d", CLI_TASK_GetMySessId());
    test_name[sizeof(test_name) - 1] = '\0';
    test_name_len = strlen(test_name);

    memset(&ctrl_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    strncpy(ctrl_entry.trace_route_ctl_owner_index, owner_index, sizeof(ctrl_entry.trace_route_ctl_owner_index) - 1);
    ctrl_entry.trace_route_ctl_owner_index[sizeof(ctrl_entry.trace_route_ctl_owner_index) - 1] = '\0';
    ctrl_entry.trace_route_ctl_owner_index_len = strlen(ctrl_entry.trace_route_ctl_owner_index);
    strncpy(ctrl_entry.trace_route_ctl_test_name, test_name, sizeof(ctrl_entry.trace_route_ctl_test_name) - 1);
    ctrl_entry.trace_route_ctl_test_name[sizeof(ctrl_entry.trace_route_ctl_test_name) - 1] = '\0';
    ctrl_entry.trace_route_ctl_test_name_len = strlen(ctrl_entry.trace_route_ctl_test_name);
    ctrl_entry.trace_route_ctl_target_address_type = L_INET_ADDR_TYPE_IPV4;
    /* max-failures */
    ctrl_entry.trace_route_ctl_max_failures = SYS_DFLT_TRACEROUTE_CTL_MAX_FAILURE;

    /* admin status is disabled firstly. */
    ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;

    ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_createAndGo;

    if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
    {
        CLI_LIB_PrintStr("Failed to start traceroute.\r\n");
        return CLI_NO_ERROR;
    }

    /* display message */
    CLI_LIB_PrintStr("Press \"ESC\" to abort.\r\n");
    CLI_LIB_PrintStr_1("Traceroute to %s", arg[0]);

    start_time = SYSFUN_GetSysTick();

    memset(&inet_target_addr, 0, sizeof(inet_target_addr));

    if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_UNSPEC,
                                                       arg[0],
                                                       (L_INET_Addr_T *) &inet_target_addr,
                                                       sizeof(inet_target_addr)))
    {
#if (SYS_CPNT_DNS == TRUE)
        DNS_Nslookup_CTRL_T nslookup_ctl_entry;
        DNS_Nslookup_Result_T nslookup_result_entry;
        UI32_T nslookup_ctl_index;
        UI32_T nslookup_result_index;

        memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

        strncpy((char *)nslookup_ctl_entry.CtlOwnerIndex,
            DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI,
            sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1);
        nslookup_ctl_entry.CtlOwnerIndex[sizeof(nslookup_ctl_entry.CtlOwnerIndex) - 1] = '\0';
        nslookup_ctl_entry.CtlOwnerIndexLen = strlen((char *)nslookup_ctl_entry.CtlOwnerIndex);

        snprintf((char *)nslookup_ctl_entry.OperationName, sizeof(nslookup_ctl_entry.OperationName),
            "%d", CLI_TASK_GetMySessId());
        nslookup_ctl_entry.OperationName[sizeof(nslookup_ctl_entry.OperationName) - 1] = '\0';
        nslookup_ctl_entry.OperationNameLen = strlen((char *)nslookup_ctl_entry.OperationName);

        strncpy((char *)nslookup_ctl_entry.TargetAddress,
            arg[0],
            sizeof(nslookup_ctl_entry.TargetAddress)-1);
        nslookup_ctl_entry.TargetAddress[sizeof(nslookup_ctl_entry.TargetAddress) - 1] = '\0';
        nslookup_ctl_entry.TargetAddressLen = strlen((char *)nslookup_ctl_entry.TargetAddress);

        nslookup_ctl_entry.af_family = AF_INET;

        if (DNS_OK != DNS_PMGR_CreateSystemNslookupCtlEntry(&nslookup_ctl_entry, &nslookup_ctl_index))
        {
            CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
            ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
            TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
            return CLI_NO_ERROR;
        }

        while (1)
        {
            if (CLI_IO_ReadACharNoWait() == 0x1B) /*ESC*/
            {
                UI8_T tmp_char;
                /* EPR ID:ES4626F-SW-FLF-38-00595
                 * Arrow keys are consisted by 3 characters
                 * and the 1st character is 0x1B
                 * Users are not happy to see the latter 2 characters being
                 * printed. So do the workaround here to remove these 2 characters
                 * from Telnet rx buffer to avoid being printed by CLI latter
                 * Cross reference to CLI_IO_GetKeyByVtTerminal() in cli_io.c
                 */

                tmp_char = CLI_IO_ReadACharNoWait(ctrl_P);
                if (tmp_char!=0)
                {
                    if (tmp_char==0x5B || tmp_char==0x4F) /* 2nd char of arrow key */
                    {
                        /* read the last char to clear arrow key in buffer
                         */
                        CLI_IO_ReadACharNoWait(ctrl_P);
                    }
                }

                /* silently destroy ctl entry
                 */
                CLI_LIB_Printf("\r\n");
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
                TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
                return CLI_NO_ERROR;
            }

            memset(&nslookup_ctl_entry, 0, sizeof(nslookup_ctl_entry));

            if (DNS_OK == DNS_PMGR_GetNslookupCtlEntryByIndex(nslookup_ctl_index, &nslookup_ctl_entry))
            {
                if (NSLOOKUP_OPSTATUS_COMPLETED == nslookup_ctl_entry.OperStatus)
                {
                    if (DNS_OK == nslookup_ctl_entry.Rc)
                    {
                        nslookup_result_index = 0;
                        memset(&nslookup_result_entry, 0, sizeof(nslookup_result_entry));

                        if (DNS_OK == DNS_PMGR_GetNslookupResultEntryByIndex(
                            nslookup_ctl_index, nslookup_result_index, &nslookup_result_entry))
                        {
                            memcpy(&inet_target_addr, &nslookup_result_entry.ResultsAddress_str, sizeof(inet_target_addr));
                            DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                            break;
                        }
                    }

                    CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                    DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                    ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
                    TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
                    return CLI_NO_ERROR;
                }
            }
            else
            {
                CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
                ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
                TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
                return CLI_NO_ERROR;
            }

            /* To avoid somthing wrong, CLI maybe halt, we wait a bit longer than total timeout, then break */
            if((SYSFUN_GetSysTick() - start_time) > ctl_timeout * (ctl_probes_per_hop + 1))
            {
                CLI_LIB_PrintStr_1("\r\nTimeout for lookup host name %s.\r\n", arg[0]);
                DNS_PMGR_Nslookup_DeleteEntry(nslookup_ctl_index);
                ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
                TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
                return CLI_NO_ERROR;
            }

            SYSFUN_Sleep(50);
        }

        /* display ip address got from DNS */
        CLI_LIB_PrintStr_4(" [%d.%d.%d.%d],", inet_target_addr.addr[0], inet_target_addr.addr[1], inet_target_addr.addr[2], inet_target_addr.addr[3]);
#else
        /* No DNS
         */
        CLI_LIB_PrintStr_1("\r\nFailed to find host %s. Please try again.\r\n", arg[0]);
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
        return CLI_NO_ERROR;
#endif
    }

    memcpy(&ctrl_entry.trace_route_ctl_target_address, &inet_target_addr, sizeof(L_INET_AddrIp_T));

    ret = TRACEROUTE_PMGR_SetTraceRouteCtlTargetAddress(&ctrl_entry);
    switch(ret)
    {
        case TRACEROUTE_TYPE_OK:
            break;
        case TRACEROUTE_TYPE_INVALID_ARG:
            CLI_LIB_PrintStr("\r\nInvalid address.\r\n");
            break;
        case TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_BROADCAST_ADDR:
            CLI_LIB_PrintStr("\r\nInvalid address. Broadcast address is not allowed.\r\n");
            break;
        case TRACEROUTE_TYPE_INVALID_TARGET_ADDR_AS_NETWORK_ID:
            CLI_LIB_PrintStr("\r\nInvalid address. Network ID is not allowed.\r\n");
            break;
        default:
            break;
    }

    if(TRACEROUTE_TYPE_OK != ret)
    {
        /* silently destroy ctl entry */
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry);
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_2(", %d hops max, timeout is %d seconds\r\n",
            SYS_DFLT_TRACEROUTE_CTL_MAX_TTL,
            SYS_DFLT_TRACEROUTE_CTL_TIME_OUT/SYS_BLD_TICKS_PER_SECOND);


    /* enable admin status to start to traceroute.*/
    ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_enabled;

    if (TRACEROUTE_TYPE_OK != TRACEROUTE_PMGR_SetTraceRouteCtlAdminStatus(&ctrl_entry))
    {
        CLI_LIB_PrintStr("Failed to enable traceroute.\r\n");
        ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
        if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to terminate traceroute.\r\n");
        }
        return CLI_NO_ERROR;
    }

    /* Sample output:
     *
     *  Press "ESC" to abort.
     *  Traceroute to 192.168.1.6, 30 hops max, timeout is 3 seconds
     *
     *  Hop Packet 1 Packet 2 Packet 3 IP Address
     *  --- -------- -------- -------- ---------------
     *    1        *        *        *
     *    2        *        *        *
     *  Codes:
     *    * - No Response
     */

    memset(&prob_history_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));

    snprintf(buff, sizeof(buff), "%-3s %-8s %-8s %-8s %-15s\r\n",
            "Hop", "Packet 1", "Packet 2", "Packet 3", "IP Address");
    CLI_LIB_PrintStr(buff);

    snprintf(buff, sizeof(buff), "%-3s %-8s %-8s %-8s %15s\r\n",
            "---", "--------", "--------", "--------", "---------------");
    CLI_LIB_PrintStr(buff);

    prev_hop = 0;
    while (1)
    {
        key_ret = 0;
        if(ctrl_P->sess_type == CLI_TYPE_TELNET)
        {
            if( CLI_IO_ReadACharFromTelnet(ctrl_P) == 0x1B) //ESC
                key_ret = TRUE;
        }
        else
        {
            if( CLI_IO_ReadACharFromConsole(ctrl_P) == 0x1B) //ESC
                key_ret = TRUE;
        }

        if (key_ret)
        {
            ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;
            if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to terminate traceroute.\r\n");
            }
            else
            {
                CLI_LIB_PrintStr("\r\nTrace stopped.\r\n");
            }
            return CLI_NO_ERROR;
        }
        ret = TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(
                owner_index, owner_index_len, test_name, test_name_len,
                prob_history_entry.trace_route_probe_history_index,
                prob_history_entry.trace_route_probe_history_hop_index,
                prob_history_entry.trace_route_probe_history_probe_index,
                &prob_history_entry);

        if ((ret == TRACEROUTE_TYPE_FAIL) || (ret == TRACEROUTE_TYPE_NO_MORE_ENTRY))
        {
            cli_api_show_tracert_code_bmp(code_bmp);
            CLI_LIB_PrintStr("\r\nTrace completed.\r\n");
            break;
        }
        else if (ret == TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE)
        {
            SYSFUN_Sleep(10);
            continue;
        }

        if (prob_history_entry.trace_route_probe_history_hop_index == 0)
            continue;

        if(prev_hop != prob_history_entry.trace_route_probe_history_hop_index)
        {
            if (i != 3 && i != 0)
            {
                CLI_LIB_PrintStr("\r\n");
                i = 0;
            }

            has_ip = FALSE;
            prev_hop = prob_history_entry.trace_route_probe_history_hop_index;
            CLI_LIB_PrintStr_1("%3d ", (int)prob_history_entry.trace_route_probe_history_hop_index);
        }

        if (FALSE == has_ip)
        {
            if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&prob_history_entry.trace_route_probe_history_haddr,
                                                               ip_str,
                                                               sizeof(ip_str)))
            {
                has_ip = TRUE;
            }
        }

        CLI_LIB_PrintStr_1("%8s ",
                cli_api_get_tracert_status_str(
                    trace_status,
                    &code_bmp,
                    prob_history_entry.trace_route_probe_history_last_rc,
                    prob_history_entry.trace_route_probe_history_response));

        if (++i == 3)
        {
            if (TRUE == has_ip)
            {
                CLI_LIB_PrintStr_1("%s", ip_str);
            }

            if(!memcmp(&prob_history_entry.trace_route_probe_history_haddr, &inet_target_addr, sizeof(L_INET_AddrIp_T)))
            {
                CLI_LIB_PrintStr("\r\n");

                cli_api_show_tracert_code_bmp(code_bmp);

                ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;

                if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
                {
                    CLI_LIB_PrintStr("Failed to terminate traceroute.\r\n");
                }
                else
                {
                    CLI_LIB_PrintStr("\r\nTrace completed.\r\n");
                }
                return CLI_NO_ERROR;
            }
            CLI_LIB_PrintStr("\r\n");
            i = 0;
        }
    } // end of while

    ctrl_entry.trace_route_ctl_rowstatus = VAL_traceRouteCtlRowStatus_destroy;

    if (TRACEROUTE_PMGR_SetTraceRouteCtlEntry(&ctrl_entry) != TRACEROUTE_TYPE_OK)
    {
        CLI_LIB_PrintStr("Failed to terminate traceroute.\r\n");
    }
#else
    CLI_LIB_PrintStr("Do not support this command in this version\r\n");
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Reload(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char choice[2] = {0};
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    char *month_ar[12] = {
        "January", "February", "March", "April", "May", "June",
        "July",  "August", "September", "October", "November", "December"
    };
#endif

    switch (cmd_idx)
    {
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_RELOAD:
            {
                if ((arg[0][0] == 'i') || (arg[0][0] == 'I'))   /* reload in */
                {
                    /*
                     * The command is:
                     * reload in {hour hour | minute minute | hour hour minute minute}
                     */
                    UI32_T  minute = 0;
                    I32_T remain_seconds;
                    SYS_TIME_DST next_reload_time;

                    if (arg[1] && ((arg[1][0] == 'h') || (arg[1][0] == 'H')))
                    {
                        /* reload in hour hour minute minute */
                        if (arg[3] && ((arg[3][0] == 'm') || (arg[3][0] == 'M')))
                        {
                            minute = atoi(arg[4]) + (atoi(arg[2]) * 60);
                        }
                        /* reload in hour hour */
                        else
                        {
                            minute = atoi(arg[2]) * 60;
                        }
                    }
                    else    /* reload in minute minute */
                    {
                        minute = atoi(arg[2]);
                    }
                    remain_seconds = minute * 60;

                    if (SYS_PMGR_QueryNextReloadInTime(remain_seconds, &next_reload_time) != TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to query reload switch in time.\r\n");
                    }
                    else
                    {
                        CLI_LIB_PrintStr("***\r\n");
                        CLI_LIB_PrintStr_2("*** --- Rebooting at %s %2ld ",
                                month_ar[next_reload_time.month-1],
                                (long)next_reload_time.day);
                        CLI_LIB_PrintStr_4("%02ld:%02ld:%02ld %ld ---\r\n",
                                (long)next_reload_time.hour,
                                (long)next_reload_time.minute,
                                (long)next_reload_time.second,
                                (long)next_reload_time.year);
                        CLI_LIB_PrintStr("***\r\n\r\n");
                        CLI_LIB_PrintStr("Are you sure to reboot the system at the specified time? <y/n>");
                        CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
                        CLI_LIB_PrintNullStr(1);

                        if ((choice[0] == 'y') || (choice[0] == 'Y'))   /* User confirmed to set reload time */
                        {
                            if (SYS_PMGR_SetReloadIn(minute) != TRUE)
                            {
                                CLI_LIB_PrintStr("Failed to set reload switch in time.\r\n");
                            }
                        }
                    }
                    return CLI_NO_ERROR;
                }
                else if ((arg[0][0] == 'a') || (arg[0][0] == 'A'))  /* reload at */
                {
                    /*
                     * The command is:
                     * reload at hour minute [{month day | day month} [year]]
                     */
                    BOOL_T is_active = FALSE;
                    SYS_RELOAD_OM_RELOADAT_DST reload_at;
                    SYS_TIME_DST next_reload_time;
                    I32_T remain_seconds;

                    reload_at.year   = 0;
                    reload_at.month  = 0;
                    reload_at.day    = 0;
                    reload_at.hour   = atoi(arg[1]);
                    reload_at.minute = atoi(arg[2]);

                    /* User has set the Month or Day argument */
                    if (arg[3] != NULL)
                    {
                        int i = 0, length = 0;
                        char month[10] = {0};

                        if (atoi(arg[3]) == 0)  /* Failed to convert to integer, so arg[3] is month */
                        {
                            strncpy(month, arg[3], sizeof(month)-1);
                            reload_at.day = atoi(arg[4]);
                        }
                        else
                        {
                            strncpy(month, arg[4], sizeof(month)-1);
                            reload_at.day = atoi(arg[3]);
                        }

                        /* Capitalize the month argument */
                        length = strlen(month);
                        if (('a' <= month[0]) && (month[0] <= 'z'))
                        {
                            month[0] = month[0] - 'a' + 'A';
                        }
                        for (i = 1; i < length; ++i)
                        {
                            if (('A' <= month[i]) && (month[i] <= 'Z'))
                            {
                                month[i] = month[i] - 'A' + 'a';
                            }
                        }
                        for (i = 0; i < 12; ++i)
                        {
                            /* The month argument may not be entered completely.
                             * Thus, we use strncmp function instead of strcmp function.
                             */
                            if (strncmp(month_ar[i], month, strlen(month)) == 0)
                            {
                                reload_at.month = i+1;
                                break;
                            }
                        }
                        if ((reload_at.month == 4) || (reload_at.month == 6) || (reload_at.month == 9) || (reload_at.month == 11))
                        {
                            if (reload_at.day > 30)
                            {
                                CLI_LIB_PrintStr_1("Error: %s only has 30 days.\r\n", month_ar[reload_at.month-1]);
                                return CLI_NO_ERROR;
                            }
                        }
                        else if (reload_at.month == 2)
                        {
                            if (reload_at.day > 29)
                            {
                                CLI_LIB_PrintStr("Error: February has at most 29 days.\r\n");
                                return CLI_NO_ERROR;
                            }
                        }

                        /* User has set the Year argument */
                        if (arg[5] != NULL)
                        {
                            reload_at.year = atoi(arg[5]);

                            /* leap year checking */
                            if ((reload_at.month == 2) && (reload_at.day > 28))
                            {
                                if (!((reload_at.year % 400 == 0) ||
                                            ((reload_at.year % 4 == 0) && (reload_at.year % 100 != 0))))
                                {
                                    CLI_LIB_PrintStr_1("Error: Year %ld is not a leap year.\r\n", (long)reload_at.year);
                                    return CLI_NO_ERROR;
                                }
                            }
                        }
                    }

                    if (SYS_PMGR_QueryNextReloadAtTime(reload_at, &next_reload_time, &is_active) != TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to query reload switch at time.\r\n");
                    }
                    else
                    {
                        if (is_active != TRUE)
                        {
                            CLI_LIB_PrintStr("Warning:\r\n");
                            CLI_LIB_PrintStr("\tYou have to setup system time first.\r\n");
                            CLI_LIB_PrintStr("\tOtherwise this function won't work.\r\n");
                        }
                        CLI_LIB_PrintStr("***\r\n");
                        CLI_LIB_PrintStr_2("*** --- Rebooting at %s %2ld ",
                                month_ar[next_reload_time.month-1],
                                (long)next_reload_time.day);
                        CLI_LIB_PrintStr_4("%02ld:%02ld:%02ld %ld ---\r\n",
                                (long)next_reload_time.hour,
                                (long)next_reload_time.minute,
                                (long)next_reload_time.second,
                                (long)next_reload_time.year);
                        CLI_LIB_PrintStr("***\r\n\r\n");

                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
                            reload_at.year   = next_reload_time.year;
                            reload_at.month  = next_reload_time.month;
                            reload_at.day    = next_reload_time.day;
                            reload_at.hour   = next_reload_time.hour;
                            reload_at.minute = next_reload_time.minute;

                            if (SYS_PMGR_SetReloadAt(&reload_at) != TRUE)
                            {
                                CLI_LIB_PrintStr("Failed to set reload switch at time.\r\n");
                            }
                        }
                        else
                        {
                            CLI_LIB_PrintStr("Are you sure to reboot the system at the specified time? <y/n>");
                            CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
                            CLI_LIB_PrintNullStr(1);

                            if ((choice[0] == 'y') || (choice[0] == 'Y'))   /* User confirmed to set reload time */
                            {
                                reload_at.year   = next_reload_time.year;
                                reload_at.month  = next_reload_time.month;
                                reload_at.day    = next_reload_time.day;
                                reload_at.hour   = next_reload_time.hour;
                                reload_at.minute = next_reload_time.minute;

                                if (SYS_PMGR_SetReloadAt(&reload_at) != TRUE)
                                {
                                    CLI_LIB_PrintStr("Failed to set reload switch at time.\r\n");
                                }
                                else
                                {
                                    /* Check if user has setup a passed time. */
                                    SYS_PMGR_GetReloadAtInfo(&reload_at, &next_reload_time, &remain_seconds, &is_active);
                                    if (remain_seconds < 0)
                                    {
                                        CLI_LIB_PrintStr("Warning:\r\n\tThe specific time you set up was passed.\r\n");
                                        CLI_LIB_PrintStr("\tYou should re-assign a correct time.\r\n");
                                    }
                                }
                            }
                        }
                    }
                    return CLI_NO_ERROR;
                }
                else if ((arg[0][0] == 'r') || (arg[0][0] == 'R'))  /* reload regularity */
                {
                    /*
                     * The command is:
                     * reload regularity hour minute [period {daily | weekly day_of_week | monthly day}]
                     */
                    BOOL_T is_active = FALSE;
                    SYS_RELOAD_OM_RELOADREGULARITY_DST reload_regularity;
                    SYS_TIME_DST next_reload_time;
                    char *day_ar[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

                    reload_regularity.hour   = atoi(arg[1]);
                    reload_regularity.minute = atoi(arg[2]);
                    reload_regularity.period = SYS_RELOAD_TYPE_REGULARITY_PERIOD_DAILY;

                    if (arg[3] != NULL)
                    {
                        /* daily */
                        if ((arg[4][0] == 'd') || (arg[4][0] == 'D'))
                        {
                            reload_regularity.period = SYS_RELOAD_TYPE_REGULARITY_PERIOD_DAILY;
                        }
                        /* weekly */
                        else if ((arg[4][0] == 'w') || (arg[4][0] == 'W'))
                        {
                            int i = 0, length = 0;
                            char day_of_week[10] = {0};

                            reload_regularity.period = SYS_RELOAD_TYPE_REGULARITY_PERIOD_WEEKLY;

                            /* Capitalize the day_of_week argument */
                            strncpy(day_of_week, arg[5], sizeof(day_of_week)-1);
                            length = strlen(day_of_week);
                            if (('a' <= day_of_week[0]) && (day_of_week[0] <= 'z'))
                            {
                                day_of_week[0] = day_of_week[0] - 'a' + 'A';
                            }
                            for (i = 1; i < length; ++i)
                            {
                                if (('A' <= day_of_week[i]) && (day_of_week[i] <= 'Z'))
                                {
                                    day_of_week[i] = day_of_week[i] - 'A' + 'a';
                                }
                            }

                            for (i = 0; i < 7; ++i)
                            {
                                if (strncmp(day_ar[i], day_of_week, strlen(day_of_week)) == 0)
                                {
                                    reload_regularity.day_of_week = i;
                                    break;
                                }
                            }
                        }
                        /* monthly */
                        else
                        {
                            reload_regularity.period = SYS_RELOAD_TYPE_REGULARITY_PERIOD_MONTHLY;
                            reload_regularity.day_of_month = atoi(arg[5]);
                        }
                    }

                    if (SYS_PMGR_QueryNextReloadRegularityTime(reload_regularity, &next_reload_time, &is_active) != TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to query switch reload interval.\r\n");
                    }
                    else
                    {
                        if (is_active != TRUE)
                        {
                            CLI_LIB_PrintStr("Warning:\r\n");
                            CLI_LIB_PrintStr("\tYou have to setup system time first.\r\n");
                            CLI_LIB_PrintStr("\tOtherwise this function won't work.\r\n");
                        }
                        CLI_LIB_PrintStr("***\r\n");
                        CLI_LIB_PrintStr_2("*** --- Rebooting regularly at %02ld:%02ld",
                                (long)reload_regularity.hour,
                                (long)reload_regularity.minute);
                        if (reload_regularity.period == SYS_RELOAD_TYPE_REGULARITY_PERIOD_DAILY)
                        {
                            CLI_LIB_PrintStr(" everyday. ---\r\n");
                        }
                        else if (reload_regularity.period == SYS_RELOAD_TYPE_REGULARITY_PERIOD_WEEKLY)
                        {
                            CLI_LIB_PrintStr_1(" every %s. ---\r\n", day_ar[reload_regularity.day_of_week]);
                        }
                        else
                        {
                            CLI_LIB_PrintStr_1(", at day %ld every month. ---\r\n", (long)reload_regularity.day_of_month);
                        }

                        CLI_LIB_PrintStr_2("*** --- Next reboot time is %s %2ld ",
                                month_ar[next_reload_time.month-1],
                                (long)next_reload_time.day);
                        CLI_LIB_PrintStr_4("%02ld:%02ld:%02ld %ld ---\r\n",
                                (long)next_reload_time.hour,
                                (long)next_reload_time.minute,
                                (long)next_reload_time.second,
                                (long)next_reload_time.year);
                        CLI_LIB_PrintStr("***\r\n\r\n");

                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
                            if (SYS_PMGR_SetReloadRegularity(&reload_regularity) != TRUE)
                            {
                                CLI_LIB_PrintStr("Failed to set switch reload interval.\r\n");
                            }
                        }
                        else
                        {
                            CLI_LIB_PrintStr("Are you sure to reboot the system regularly at the specified time? <y/n>");
                            CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
                            CLI_LIB_PrintNullStr(1);

                            if ((choice[0] == 'y') || (choice[0] == 'Y'))   /* User confirmed to set reload time */
                            {
                                if (SYS_PMGR_SetReloadRegularity(&reload_regularity) != TRUE)
                                {
                                    CLI_LIB_PrintStr("Failed to set switch reload interval.\r\n");
                                }
                            }
                        }
                    }
                    return CLI_NO_ERROR;
                }
                else if ((arg[0][0] == 'c') || (arg[0][0] == 'C'))    /* reload cancel */
                {
                    /*
                     * The command is:
                     * reload cancel [at | in | regularity]
                     */
                    CLI_LIB_PrintStr("Are you sure to cancel the reload settings? <y/n>");
                    CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
                    CLI_LIB_PrintNullStr(1);

                    if ((choice[0] == 'y') || (choice[0] == 'Y'))   /* User confirmed to cancel reload settings */
                    {
                        if (arg[1] == NULL)
                        {
                            SYS_PMGR_ReloadInCancel();
                            SYS_PMGR_ReloadAtCancel();
                            SYS_PMGR_ReloadRegularityCancel();
                        }
                        else if (arg[1][0] == 'a' || arg[1][0] == 'A')
                            SYS_PMGR_ReloadAtCancel();
                        else if (arg[1][0] == 'i' || arg[1][0] == 'I')
                            SYS_PMGR_ReloadInCancel();
                        else if (arg[1][0] == 'r' || arg[1][0] == 'R')
                            SYS_PMGR_ReloadRegularityCancel();
                    }
                    return CLI_NO_ERROR;
                }
            }
#endif  /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */
        case PRIVILEGE_EXEC_CMD_W1_RELOAD:
            {

                CLI_LIB_PrintStr("System will be restarted. Continue <y/n>? ");
                CLI_PARS_ReadLine( (char *)choice, sizeof(choice), TRUE, FALSE);
                CLI_LIB_PrintNullStr(1);

                if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
                    return CLI_NO_ERROR;

                if(arg[0]!=NULL)
                {
                    if(arg[0][0] == 'h' || arg[0][0]=='H')
                    {
                        system("echo \"2\" > /flash/of_opmode.conf");
                    }
                    else if(arg[0][0] == 'l' || arg[0][0] == 'L')
                    {
                        system("echo \"1\" > /flash/of_opmode.conf");
                    }
                }

                /* EPR ES4827G-FLF-ZZ-00045, send cold start trap after
                 * reload system. That's the ACPv3 design of
                 * STKCTRL_PMGR_ReloadSystem(). Use STKCTRL_PMGR_WarmStartSystem()
                 * to warm start system and send "warm start" trap
                 */
                /* STKCTRL_PMGR_ReloadSystem(); */
                STKCTRL_PMGR_WarmStartSystem();

                /*add by fen.wang,it is useless,for stkctrl will notify sysdrv,and sysdrv will to wait and to reload*/
#if 0
                while(1)
                {
                    if(count==1)
                    {
                        CLI_LIB_PrintStr("Session wait for rebooting..........................................\r\n");
                    }
                    count++;
                    SYSFUN_Sleep(100);
                }
#endif
                return CLI_NO_ERROR;
            }
        default:
            return CLI_ERR_INTERNAL;
    }
}

UI32_T CLI_API_FanSpeed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    BOOL_T mode = FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_FANSPEED:
            mode = TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_FANSPEED:
            mode = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (SYS_PMGR_SetFanSpeedForceFull(mode) != TRUE)
    {
        CLI_LIB_PrintStr_1("Fail to %s fan speed force full.\r\n", (mode == TRUE)? "enable":"disable");
    }

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Test_Snmp_Trap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    USED_TO_PRT_ARGS;
    CLI_LIB_PrintStr("<TBD>\r\n");
    return CLI_NO_ERROR;
}

/*configuration*/
UI32_T CLI_API_Enable_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    /*verify*/
    UI32_T privilege = 0;
    UI8_T  password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

    UI32_T encrypt_pos;
    UI32_T password_pos;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_ENABLE_PASSWORD:

            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            if (*arg[0] == 'l' || *arg[0] == 'L')
            {
                if(arg[1]==NULL)
                    return CLI_ERR_INTERNAL;

                privilege = atoi(arg[1]);
                encrypt_pos = 2;
                password_pos = 3;
            }
            else
            {
                privilege = 15;
                encrypt_pos = 0;
                password_pos = 1;
            }

            if(arg[encrypt_pos][0] == '0')/*not encrypted yet*/
            {
                /*encryption*/
                memset(password, 0, sizeof(password));
                L_MD5_MDString(password, (UI8_T *)arg[password_pos], strlen(arg[password_pos]));
            }
            else                          /*already encrypted*/
            {
                if (!str_to_nonprintable(arg[password_pos], password))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set password\r\n");
#endif
                    return CLI_NO_ERROR;
                }
            }

            if (!USERAUTH_PMGR_SetPrivilegePassword(privilege, password))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set privilege password\r\n");
#endif
            }
            else if (!USERAUTH_PMGR_SetPrivilegePasswordStatus(privilege,USERAUTH_ENTRY_VALID))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set privilege password status\r\n");
#endif
            }
            break;


        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ENABLE_PASSWORD:

            privilege = 15;

            if (arg[0] != NULL)
            {
                if (*arg[0] == 'l' || *arg[0] == 'L')
                {
                    if (arg[1] == NULL)
                        return CLI_ERR_INTERNAL;

                    privilege = atoi(arg[1]);
                }
            }

            if(privilege == 15)
            {
                /*restore default*/
                UI8_T default_enable_15_password[] = "super"; /*default: temp*/
                memset(password, 0, sizeof(password));
                L_MD5_MDString(password, default_enable_15_password, strlen((char *)default_enable_15_password));

                if (!USERAUTH_PMGR_SetPrivilegePassword(privilege, password))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set privilege password\r\n");
#endif
                }
                else if (!USERAUTH_PMGR_SetPrivilegePasswordStatus(privilege,USERAUTH_ENTRY_VALID))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set privilege password status\r\n");
#endif
                }
            }
            else if (!USERAUTH_PMGR_SetPrivilegePasswordStatus(privilege,USERAUTH_ENTRY_INVALID))
            {
                /*remove*/
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set privilege disable\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}

/*
EPR_ID:ES3628BT-FLF-ZZ-00057
Problem: CLI: The behavior of hostname command is NOT correct.
Root Cause: use error command line.
Solution: 1. use "hostname" command is modification of "system name".
2. Add "prompt" CLI command.
 */
UI32_T CLI_API_Prompt(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T prompt[SYS_ADPT_MAX_PROMPT_STRING_LEN] = {0};

    switch(cmd_idx)
    {

        case PRIVILEGE_CFG_GLOBAL_CMD_W1_PROMPT:

            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            strcpy((char *)prompt, arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_PROMPT:

            memset(prompt, 0, sizeof(prompt));
            break;
    }


    if(SYS_PMGR_SetPromptString(prompt)!= TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to configure the prompt string.\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Username(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    BOOL_T is_username_exist = FALSE;
    USERAUTH_LoginLocal_T User;
    UI8_T plain_empty_password[1+1];
    UI8_T encrypted_empty_password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1];

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    strcpy((char *)User.username, arg[0]);
    is_username_exist = USERAUTH_PMGR_GetLoginLocalUser(&User);
    memset(plain_empty_password, 0, sizeof(plain_empty_password));
    memset(encrypted_empty_password, 0, sizeof(encrypted_empty_password));
    L_MD5_MDString(encrypted_empty_password, plain_empty_password, strlen((char *)plain_empty_password));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_USERNAME:
            if(arg[1]==NULL)
                return CLI_ERR_INTERNAL;

            switch(*arg[1])
            {
                case 'a': /*access-level*/
                case 'A':
                    if(!is_username_exist)
                    {
                        if(!USERAUTH_PMGR_SetLoginLocalUserStatus((UI8_T *)arg[0], USERAUTH_ENTRY_VALID)) /*create user*/
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to add new user, max users!\r\n");
#endif
                            return CLI_NO_ERROR;
                        }

                        if(!USERAUTH_PMGR_SetLoginLocalUserPassword((UI8_T *)arg[0], encrypted_empty_password)) /*set default password, temp*/
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set default password\r\n");
#endif
                        }
                    }

                    {
                        UI32_T access_level = 0;

                        if(arg[2]==NULL)
                            return CLI_ERR_INTERNAL;

#if (SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL == TRUE)
                        access_level = atoi(arg[2]);
#else
                        switch(arg[2][0])
                        {
                            case '0':
                                access_level = 0;
                                break;

                            case '1':
                                access_level = 15;
                                break;

                            default:
                                return CLI_ERR_INTERNAL;
                        }
#endif /* SYS_CPNT_CLI_MULTI_PRIVILEGE_LEVEL */
                        if (!USERAUTH_PMGR_SetLoginLocalUserPrivilege((UI8_T *)arg[0], access_level))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set privilege\r\n");
#endif
                        }
                    }
#if 0
                    if (!USERAUTH_SetLoginLocalUserPrivilege(arg[0], atoi(arg[2])))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to set privilege\r\n");
#endif
                    }
#endif
                    break;

                case 'n': /*nopassword*/
                case 'N':
                    {
                        if(!is_username_exist)
                        {
                            if(!USERAUTH_PMGR_SetLoginLocalUserStatus((UI8_T *)arg[0], USERAUTH_ENTRY_VALID)) /*create user*/
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to add new user, max users!\r\n");
#endif
                                return CLI_NO_ERROR;
                            }

                            if(!USERAUTH_PMGR_SetLoginLocalUserPrivilege((UI8_T *)arg[0], 0)) /*0: default privilege, temp*/
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set default privilege\r\n");
#endif
                            }
                        }

                        if (!USERAUTH_PMGR_SetLoginLocalUserPassword((UI8_T *)arg[0], encrypted_empty_password))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set password\r\n");
#endif
                        }
                    }
                    break;

                case 'p': /*password*/
                case 'P':
                    {
                        UI8_T  password[SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN+1] = {0};

                        if(!is_username_exist)
                        {
                            if(!USERAUTH_PMGR_SetLoginLocalUserStatus((UI8_T *)arg[0], USERAUTH_ENTRY_VALID)) /*create user*/
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to add new user, max users!\r\n");
#endif
                                return CLI_NO_ERROR;
                            }

                            if(!USERAUTH_PMGR_SetLoginLocalUserPrivilege((UI8_T *)arg[0], 0)) /*0: default privilege, temp*/
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set default privilege\r\n");
#endif
                            }
                        }

                        if((arg[2]==NULL)||(arg[3]==NULL))
                            return CLI_ERR_INTERNAL;

                        if(arg[2][0] == '0') /*not encrypted yet*/
                        {
                            /*encryption*/
                            memset(password, 0, sizeof(password));
                            L_MD5_MDString(password, (UI8_T *)arg[3], strlen(arg[3]));
                        }
                        else                          /*already encrypted*/
                        {
                            if (!str_to_nonprintable(arg[3], password))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr("Failed to set password\r\n");
#endif
                                return CLI_NO_ERROR;
                            }
                        }

                        if (!USERAUTH_PMGR_SetLoginLocalUserPassword((UI8_T *)arg[0], password))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr("Failed to set password\r\n");
#endif
                        }
                    }/*case 'p'*/
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }/*switch(*arg[1])*/
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_USERNAME:
            if (!USERAUTH_PMGR_SetLoginLocalUserStatus((UI8_T *)arg[0], USERAUTH_ENTRY_INVALID))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to delete user\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }/*switch(cmd_idx)*/

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Hostname(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T HostNameBuf[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1];

#if defined(STRAWMAN) || defined(STRAWMANHD)/*special for STRAWMAN*/
    UI32_T unit_id;
    STKTPLG_MGR_Switch_Info_T       switch_info;

    memset(&switch_info, 0, sizeof(switch_info));

    STKTPLG_POM_GetMyUnitID(&unit_id);
    switch_info.sw_unit_index = unit_id;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_HOSTNAME:
            if (arg[0] == NULL)
            {
                /* shumin.wang modified for ES4827G-FLF-ZZ-00119 */
                memset(HostNameBuf, 0, sizeof(HostNameBuf));
            }
            else
            {
                strcpy((char *)HostNameBuf,arg[0]);
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_HOSTNAME:
            {
                if (STKTPLG_PMGR_GetSwitchInfo(&switch_info) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Can not get default hostname and set to none\r\n");
#endif
                }
                else
                {
                    sprintf((char *)HostNameBuf, "switch%lu_%s", (unsigned long)switch_info.sw_identifier, switch_info.sw_chassis_service_tag);
                }
            }
            break;
    }
#else
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_HOSTNAME:
            if(arg[0]!=NULL)
                strcpy((char *)HostNameBuf,arg[0]);
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_HOSTNAME:
            memset(HostNameBuf, 0, sizeof(HostNameBuf));
            break;
    }

#endif

    if(!MIB2_PMGR_SetHostName( HostNameBuf ))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set hostname\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Jumbo_Frame(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_JUMBO_FRAMES==TRUE)
    UI32_T status = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_JUMBO_FRAME:
            status = SWCTRL_JUMBO_FRAME_ENABLE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_JUMBO_FRAME:
            status = SWCTRL_JUMBO_FRAME_DISABLE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!SWCTRL_PMGR_SetJumboFrameStatus(status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set jumbo status\r\n");
#endif
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Authentication_Login(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AUTHENTICATION==TRUE)

    /* BOOL_T USERAUTH_SetAuthMethod(USERAUTH_Auth_Method_T login_method); */
    //#if (SYS_CPNT_TACACS == TRUE )

    USERAUTH_Auth_Method_T   auth_method[5] = {0};  //pttch
    UI8_T   i = 0;


    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_AUTHENTICATION_LOGIN:

            for (i = 0; i <USERAUTH_NUMBER_Of_AUTH_METHOD && arg[i] != NULL; i++)
            {
                switch(arg[i][0])
                {
                    case 'l':/*local*/
                    case 'L':
                        auth_method[i] = USERAUTH_AUTH_LOCAL;
                        break;

                    case 'r':/*radius*/
                    case 'R':
                        auth_method[i] = USERAUTH_AUTH_RADIUS;
                        break;

                    case 't':/*tacacs*/
                    case 'T':
                        auth_method[i] = USERAUTH_AUTH_TACACS;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_AUTHENTICATION_LOGIN: /*default*/
            auth_method[0] = USERAUTH_AUTH_LOCAL;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(! USERAUTH_PMGR_SetAuthMethod(auth_method))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set authentication method\r\n");
#endif
    }
    return CLI_NO_ERROR;
    //#else
#if 0
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    USERAUTH_Auth_Method_T login_method;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_AUTHENTICATION_LOGIN:

            if(arg[0]!=NULL)
            {
                switch(arg[0][0])
                {
                    case 'r':     /*radius*/
                    case 'R':
                        if(arg[1]) /*local*/
                            login_method = USERAUTH_AUTH_REMOTE_THEN_LOCAL;
                        else
                            login_method = USERAUTH_AUTH_REMOTE_ONLY;
                        break;

                    case 'l':
                    case 'L':
                        if(arg[1]) /*radius*/
                            login_method = USERAUTH_AUTH_LOCAL_THEN_REMOTE;
                        else
                            login_method = USERAUTH_AUTH_LOCAL_ONLY;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_AUTHENTICATION_LOGIN: /*default*/
            login_method = USERAUTH_AUTH_METHOD_DEF;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(! USERAUTH_PMGR_SetAuthMethod(login_method))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set authentication method\r\n");
#endif
    }
#endif
#endif
#endif  /* #if (SYS_CPNT_AUTHENTICATION==TRUE) */
    return CLI_NO_ERROR;

}

UI32_T CLI_API_Authentication_Enable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AUTHENTICATION==TRUE)
    /* BOOL_T USERAUTH_SetAuthMethod(USERAUTH_Auth_Method_T login_method); */
    //#if (SYS_CPNT_TACACS == TRUE )

    USERAUTH_Auth_Method_T   auth_method[5] = {0};  //pttch
    UI8_T   i = 0;


    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_AUTHENTICATION_ENABLE:

            for (i = 0; i <USERAUTH_NUMBER_Of_AUTH_METHOD && arg[i] != NULL; i++)
            {
                switch(arg[i][0])
                {
                    case 'l':/*local*/
                    case 'L':
                        auth_method[i] = USERAUTH_AUTH_LOCAL;
                        break;

                    case 'r':/*radius*/
                    case 'R':
                        auth_method[i] = USERAUTH_AUTH_RADIUS;
                        break;

                    case 't':/*tacacs*/
                    case 'T':
                        auth_method[i] = USERAUTH_AUTH_TACACS;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_AUTHENTICATION_ENABLE: /*default*/
            auth_method[0] = USERAUTH_AUTH_LOCAL;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(! USERAUTH_PMGR_SetEnableAuthMethod(auth_method))
        CLI_LIB_PrintStr("Failed to set authentication method\r\n");
    return CLI_NO_ERROR;
    //#else
#if 0
#if (SYS_CPNT_RADIUS == TRUE ||SYS_CPNT_RADIUS_AUTHENTICATION == TRUE)
    USERAUTH_Auth_Method_T login_method;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_AUTHENTICATION_LOGIN:

            if(arg[0]!=NULL)
            {
                switch(arg[0][0])
                {
                    case 'r':     /*radius*/
                    case 'R':
                        if(arg[1]) /*local*/
                            login_method = USERAUTH_AUTH_REMOTE_THEN_LOCAL;
                        else
                            login_method = USERAUTH_AUTH_REMOTE_ONLY;
                        break;

                    case 'l':
                    case 'L':
                        if(arg[1]) /*radius*/
                            login_method = USERAUTH_AUTH_LOCAL_THEN_REMOTE;
                        else
                            login_method = USERAUTH_AUTH_LOCAL_ONLY;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_AUTHENTICATION_LOGIN: /*default*/
            login_method = USERAUTH_AUTH_METHOD_DEF;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(! USERAUTH_PMGR_SetAuthMethod(login_method))
        CLI_LIB_PrintStr("Failed to set authentication method\r\n");
#endif
#endif
#endif  /* #if (SYS_CPNT_AUTHENTICATION==TRUE) */
    return CLI_NO_ERROR;

}

UI32_T CLI_API_Disconnect(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    if(arg[0]!=NULL)
    {
        if (!CLI_TASK_SetKillWorkingSpaceFlag(atoi(arg[0])))
            CLI_LIB_PrintStr("Failed to disconnect session\r\n");
    }
    else
        return CLI_ERR_INTERNAL;

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_STACKING == TRUE)
UI32_T CLI_API_PHYMAP(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    UI8_T unit_mac[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][6] = {{0}};
    UI8_T i = 0;



    for (i = 0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        CLI_LIB_ValsInMac(arg[i], unit_mac[i]);
    }

    if (!SWCTRL_PMGR_SetUnitsBaseMacAddrTable(unit_mac))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set phyical unit MAC address\r\n");
#endif
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Light_Unit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T unit;

    if (arg[0] == NULL)
    {
        LEDDRV_ShowAllUnitLED();
    }
    else
    {
        unit = atoi(arg[0]);
        LEDDRV_ShowUnitLED(unit);
    }

    return CLI_NO_ERROR;
}

#endif

#if (SYS_CPNT_STACKING == TRUE)
UI32_T CLI_API_Switch_Renumber(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    if((arg[0][0] == 'a') || (arg[0][0] == 'A'))
    {
        STKCTRL_PMGR_UnitIDReNumbering();
    }

    return CLI_NO_ERROR;
}

#if(SYS_CPNT_MASTER_BUTTON==SYS_CPNT_MASTER_BUTTON_SOFTWARE)
UI32_T CLI_API_SwitchMasterButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W3_SWITCH_MASTER_BUTTON:
            if(STKTPLG_PMGR_SetMasterButtonStatus(atoi(arg[0]), TRUE)==FALSE)
                CLI_LIB_PrintStr("Failed to set switch master button!");
            break;

        case PRIVILEGE_EXEC_CMD_W4_NO_SWITCH_MASTER_BUTTON:
            if(STKTPLG_PMGR_SetMasterButtonStatus(atoi(arg[0]), FALSE)==FALSE)
                CLI_LIB_PrintStr("Failed to reset switch master button!");
    }
    return CLI_NO_ERROR;
}
#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
UI32_T CLI_API_SwitchStackingButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W3_SWITCH_STACKING_BUTTON:
            if(STKTPLG_PMGR_SetStackingButtonStatus(TRUE)==FALSE)
                CLI_LIB_PrintStr("Failed to set switch stacking button!");
            break;

        case PRIVILEGE_EXEC_CMD_W4_NO_SWITCH_STACKING_BUTTON:
            if(STKTPLG_PMGR_SetStackingButtonStatus(FALSE)==FALSE)
                CLI_LIB_PrintStr("Failed to reset switch stacking button!");
    }
    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

#endif
UI32_T CLI_API_IP_Telnet_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TELNET == TRUE)
    switch(cmd_idx)
    {
        case  PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_TELNET_SERVER:
            if (!TELNET_PMGR_SetTnpdStatus(TELNET_STATE_ENABLED))
            {
                CLI_LIB_PrintStr("Failed to enable telnet server\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_TELNET_SERVER:
            if (!TELNET_PMGR_SetTnpdStatus(TELNET_STATE_DISABLED))
            {
                CLI_LIB_PrintStr("Failed to disable telnet server\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_TELNET_PORT:
            if(arg[0]==NULL)
            {
                return CLI_ERR_INTERNAL;
            }

            if(TELNET_PMGR_SetTnpdPort(atoi(arg[0]))==FALSE)
            {
                CLI_LIB_PrintStr("Failed to set telnet server port\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_TELNET_PORT:
            if (!TELNET_PMGR_SetTnpdPort(SYS_DFLT_TELNET_SOCKET_PORT))
            {
                CLI_LIB_PrintStr("Failed to set telnet server port as default\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_TELNET_MAXSESSIONS:
            if(arg[0]==NULL)
            {
                return CLI_ERR_INTERNAL;
            }

            if (!TELNET_PMGR_SetTnpdMaxSession(atoi(arg[0])))
            {
                CLI_LIB_PrintStr("Failed to set telnet server max sessions\r\n");
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_TELNET_MAXSESSIONS:
            if (!TELNET_PMGR_SetTnpdMaxSession(SYS_DFLT_TELNET_DEFAULT_MAX_SESSION))
            {
                CLI_LIB_PrintStr("Failed to set telnet server max sessions\r\n");
            }

            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* SYS_CPNT_TELNET */
    return CLI_NO_ERROR;
}


UI32_T CLI_API_System_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_PER_SYSTEM )
    UI32_T mtu = SYS_DFLT_PORT_MTU,jumbo_frame_status;
    BOOL_T jumbo=FALSE,no_function = FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SYSTEM_MTU:
            mtu = CLI_LIB_AtoUl(arg[0], 10);
            jumbo = FALSE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_SYSTEM_MTU_JUMBO:
            mtu = CLI_LIB_AtoUl(arg[0], 10);
            jumbo = TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYSTEM_MTU:
            no_function = TRUE;
            mtu = SYS_DFLT_PORT_MTU;
            jumbo = TRUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    } //end switch

    if (!SWCTRL_PMGR_SetSystemMTU(jumbo, mtu))
    {
        CLI_LIB_PrintStr("Failed to set system mtu!\r\n");
    }

    if (no_function)
    {
        if (!SWCTRL_PMGR_SetSystemMTU(FALSE, mtu))
        {
            CLI_LIB_PrintStr("Failed to set system mtu!\r\n");
        }

        if (!SWCTRL_POM_GetJumboFrameStatus(&jumbo_frame_status))
        {
            CLI_LIB_PrintStr("Failed to set system mtu!\r\n");
        }
        else
        {
            if (jumbo_frame_status == SWCTRL_JUMBO_FRAME_ENABLE)
            {
                if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_DISABLE))
                {
                    CLI_LIB_PrintStr("Failed to set system mtu!\r\n");
                }

                if (!SWCTRL_PMGR_SetJumboFrameStatus(SWCTRL_JUMBO_FRAME_ENABLE))
                {
                    CLI_LIB_PrintStr("Failed to set system mtu!\r\n");
                }
            }
        }
    }


#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Reload(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    CLI_LIB_ShowReloadInfo(FALSE);
#endif  /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_System_Mtu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_PER_SYSTEM )
    UI32_T jumbo,mtu;
    if(!SWCTRL_PMGR_GetSystemMTU(&jumbo, &mtu))
    {
        CLI_LIB_PrintStr("\r\ncan not get system mtu!\r\n");
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr_1("System Jumbo MTU size is %lu Bytes\r\n", (unsigned long)jumbo);

#endif
    return CLI_NO_ERROR;
}



UI32_T CLI_API_Show_Ip_Telnet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TELNET == TRUE)
    UI32_T          telentPort;
    TELNET_State_T  telnetStatus;
    UI32_T          telnetMaxSession;

    if (cmd_idx != PRIVILEGE_EXEC_CMD_W3_SHOW_IP_TELNET)
    {
        return CLI_ERR_INTERNAL;
    }

    CLI_LIB_PrintStr("IP Telnet Configuration:\r\n\r\n");

    CLI_LIB_PrintStr("Telnet Status: ");
    if (!TELNET_PMGR_GetTnpdStatus(&telnetStatus))
    {
        CLI_LIB_PrintStr("unknown\r\n");
    }
    else
    {
        if (telnetStatus == TELNET_STATE_ENABLED)
        {
            CLI_LIB_PrintStr("Enabled\r\n");
        }
        else
        {
            CLI_LIB_PrintStr("Disabled\r\n");
        }
    }

    CLI_LIB_PrintStr("Telnet Service Port: ");
    if (!TELNET_PMGR_GetTnpdPort(&telentPort))
    {
        CLI_LIB_PrintStr("unknown\r\n");
    }
    else
    {
        CLI_LIB_PrintStr_1("%lu\r\n", (unsigned long)telentPort);
    }

    CLI_LIB_PrintStr("Telnet Max Session: ");
    if (!TELNET_PMGR_GetTnpdMaxSession(&telnetMaxSession))
    {
        CLI_LIB_PrintStr("unknown\r\n");
    }
    else
    {
        CLI_LIB_PrintStr_1("%lu\r\n", (unsigned long)telnetMaxSession);
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Banner_Motd(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MOTD == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_BANNER_MOTD:

            if(CLI_MGR_SetSysBannerMsgStatus(arg[0][0]) == TRUE)
            {
                CLI_LIB_PrintStr_1("Enter TEXT message.  End with the character '%c'.\r\n", arg[0][0]);
            }
            else
            {
                CLI_LIB_PrintStr("Somebody occupies the operation.\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_BANNER_MOTD:

            if(CLI_MGR_DisableSysBannerMsg() != TRUE)
            {
                CLI_LIB_PrintStr("Somebody occupies the operation.\r\n");
            }
            break;
    }
#endif /*#if (SYS_CPNT_MOTD == TRUE)*/
    return CLI_NO_ERROR;

}

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) ||(SYS_CPNT_INTERNAL_LOOPBACK_TEST==TRUE)
static void cli_api_show_test_time(UI32_T time, UI8_T *string)
{
    int     year, month, day, hour, minute, second;

    if(time > 0)
    {
        SYS_TIME_ConvertSecondsToDateTime(time, &year, &month, &day, &hour, &minute, &second);
        /* the same format as WEB */
        sprintf((char *)string, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    }
    else
    {
        strcpy((char *)string, " ");
    }
}
#endif

#if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE)
#if (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM)
static void cli_api_convert_cable_diag_status(UI32_T status, UI8_T *string)
{
    switch (status)
    {
        case CABLE_NORMAL_CABLE:
            sprintf((char *)string, "OK");
            break;

        case CABLE_OPEN_CABLE:
            sprintf((char *)string, "Open");
            break;

        case CABLE_SHORT_CABLE:
            sprintf((char *)string, "Short");
            break;

        case CABLE_OPEN_SHORT_CABLE:
            sprintf((char *)string, "Open/Short");
            break;

        case CABLE_CROSSTALK_CABLE:
            sprintf((char *)string, "Crosstalk");
            break;

        case CABLE_UNKNOWN_CABLE:
        default:
            sprintf((char *)string, "Unknown");
            break;
    }
}

static UI32_T cli_api_show_cable_diag_one_port(UI32_T unit, UI32_T port, SWCTRL_Cable_Info_T result, UI32_T line_num)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T   result_status_ar[15]={0};  /* ex, 'Not tested yet' */
    UI8_T   result_time_ar[20] = {0};  /* ex, '2002-01-24 08:06:25' */

    sprintf((char *)buff, "Cable Diagnostics on interface Ethernet %lu/%lu:\r\n", (unsigned long)unit, (unsigned long)port);
    PROCESS_MORE_FUNC(buff);

    if(CABLE_NOT_TESTED_YET == result.cable_status)
    {
        PROCESS_MORE_FUNC(" Not tested yet.\r\n");
    }
    else
    {
        cli_api_convert_cable_diag_status(result.cable_status, result_status_ar);
        sprintf((char *)buff, " Cable %s with accuracy %ld meters.\r\n", result_status_ar, (long)result.fuzz_len);
        PROCESS_MORE_FUNC(buff);
        cli_api_convert_cable_diag_status(result.pair_state[0], result_status_ar);
        sprintf((char *)buff, "   Pair A %s, length %ld meters\r\n", result_status_ar, (long)result.pair_len[0]);
        PROCESS_MORE_FUNC(buff);
        cli_api_convert_cable_diag_status(result.pair_state[1], result_status_ar);
        sprintf((char *)buff, "   Pair B %s, length %ld meters\r\n", result_status_ar, (long)result.pair_len[1]);
        PROCESS_MORE_FUNC(buff);
        cli_api_convert_cable_diag_status(result.pair_state[2], result_status_ar);
        sprintf((char *)buff, "   Pair C %s, length %ld meters\r\n", result_status_ar, (long)result.pair_len[2]);
        PROCESS_MORE_FUNC(buff);
        cli_api_convert_cable_diag_status(result.pair_state[3], result_status_ar);
        sprintf((char *)buff, "   Pair D %s, length %ld meters\r\n", result_status_ar, (long)result.pair_len[3]);
        PROCESS_MORE_FUNC(buff);
        cli_api_show_test_time(result.last_test_time, result_time_ar);
        sprintf((char *)buff, "   Last Update 0n %s\r\n", result_time_ar);
        PROCESS_MORE_FUNC(buff);
    }
    PROCESS_MORE_FUNC("\r\n");
    return line_num;
}

#elif (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL)
static void cli_api_convert_cable_diag_status(UI32_T status, char *string)
{
    switch (status)
    {
        case CABLE_TEST_FAIL:
            sprintf(string, "TF");
            break;

        case CABLE_NORMAL_CABLE:
            sprintf(string, "OK");
            break;

        case CABLE_OPEN_CABLE:
            sprintf(string, "ON");
            break;

        case CABLE_SHORT_CABLE:
            sprintf(string, "ST");
            break;

        case CABLE_IMPEDANCE_MISMATCH:
            sprintf(string, "IE");
            break;

        case CABLE_NO_CABLE:
            sprintf(string, "NC");
            break;

        case CABLE_NOT_TEST_BEFORE:
            sprintf(string, "NT");
            break;

        case CABLE_DIAG_NOT_SUPPORTED:
            sprintf(string, "NS");
            break;

        default:
            sprintf(string, "UN");
            break;
    }
}

static void cli_api_convert_cable_diag_result(UI32_T status, UI32_T length, char *result_p)
{
    char   result_status_ar[20]={0};

    cli_api_convert_cable_diag_status(status, result_status_ar);

    if( (CABLE_TEST_FAIL == status) || (CABLE_NOT_TEST_BEFORE == status)
            || (CABLE_DIAG_NOT_SUPPORTED == status) )
    {
        strcpy(result_p, (char*)result_status_ar);
    }
    else
    {
        sprintf(result_p, "%s (%lu)", result_status_ar, (unsigned long)length);
    }
}

static UI32_T cli_api_show_cable_diag_one_port(UI32_T unit, UI32_T port, SWCTRL_Cable_Info_T result, UI32_T line_num, BOOL_T show_result)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char  tmp_buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char  result_status_ar[16]={0};  /* ex, 'Not tested yet' */
    UI8_T  result_time_ar[20] = {0};  /* ex, '2002-01-24 08:06:25' */
    CLI_API_EthStatus_T verify_ret;
    UI32_T lport;
    Port_Info_T swctr_port_info;

    verify_ret = verify_ethernet(unit, port, &lport);

    if (verify_ret == CLI_API_ETH_NOT_PRESENT) /* talor 2004-08-31 */
        return line_num;

    /* ethernet port */
    sprintf(tmp_buff, "Eth %lu/%2lu ", (unsigned long)unit, (unsigned long)port);
    strcat(buff, tmp_buff);
    memset(tmp_buff , 0, sizeof(tmp_buff));

    if(SWCTRL_POM_GetPortInfo(lport, &swctr_port_info))
    {
        if (swctr_port_info.port_type  == VAL_portType_hundredBaseTX)
        {
            strcat(buff, "FE   ");
        }
        else if(swctr_port_info.port_type  == VAL_portType_thousandBaseT)
        {
            strcat(buff, "GE   ");
        }
        else
        {
            strcat(buff, "N/A  ");
        }
    }

    /* current status */
    if (swctr_port_info.admin_state == VAL_ifAdminStatus_up)
    {
        if(show_result==TRUE)
        {
            if (swctr_port_info.link_status == VAL_ifOperStatus_up)
            {
                strcat(buff, "Up       ");
            }
            else
            {
                strcat(buff, "Down     ");
            }
        }
        else
        {
            if (result.ori_link_status == VAL_ifOperStatus_up)
            {
                strcat(buff, "Up       ");
            }
            else
            {
                strcat(buff, "Down     ");
            }
        }
    }
    else
    {
        strcat(buff, "Disabled ");
    }

    if (result.under_testing == TRUE)
    {
        sprintf(tmp_buff, "%-8s %-8s %-8s %-8s \r\n", "testing", "testing", "testing", "testing");
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));
    }
    else
    {
        cli_api_convert_cable_diag_result(result.pair1Status, result.pair1FaultLen, result_status_ar);
        sprintf(tmp_buff, "%-8s ", result_status_ar);
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));

        cli_api_convert_cable_diag_result(result.pair2Status, result.pair2FaultLen, result_status_ar);
        sprintf(tmp_buff, "%-8s ", result_status_ar);
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));

        cli_api_convert_cable_diag_result(result.pair3Status, result.pair3FaultLen, result_status_ar);
        sprintf(tmp_buff, "%-8s ", result_status_ar);
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));

        cli_api_convert_cable_diag_result(result.pair4Status, result.pair4FaultLen, result_status_ar);
        sprintf(tmp_buff, "%-8s ", result_status_ar);
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));

        cli_api_show_test_time(result.last_test_time, result_time_ar);
        sprintf(tmp_buff, "%-19s\r\n", result_time_ar);
        strcat(buff, tmp_buff);
        memset(tmp_buff , 0, sizeof(tmp_buff));
    }

    PROCESS_MORE_FUNC(buff);

    return line_num;
}
#else
#error "Chip not defined!"
#endif
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

UI32_T CLI_API_Test_Cable_Diag_Dsp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) && (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM))

    BOOL_T  is_inherit  =TRUE;
    UI32_T  l_port =0, unit, port, line_num=0;
    SWCTRL_Cable_Info_T  result;

    memset(&result, 0, sizeof(SWCTRL_Cable_Info_T));
    unit = atoi(arg[2]);
    port = atoi(strchr(arg[2], '/')+1);

    if( SWCTRL_LPORT_UNKNOWN_PORT ==
            SWCTRL_POM_UIUserPortToIfindex( unit, port,&l_port,&is_inherit))
    {
        CLI_LIB_PrintStr("Failed to convert port to ifIndex\r\n");
        return CLI_NO_ERROR;
    }

    if(SWCTRL_PMGR_ExecuteCableDiag(l_port, &result)!=TRUE)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_2("Failed to execute cable diagnostics on unit %lu, port %lu.\r\n",
                (unsigned long)unit, (unsigned long)port);
#endif
    }
    else
    {
        line_num = cli_api_show_cable_diag_one_port(unit, port, result, line_num);
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }

#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Cable_Diag_Dsp_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) && (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM))
    SWCTRL_Cable_Info_T  result;
    UI32_T line_num = 0;
    UI32_T l_port = 0;
    UI32_T  unit, port;
    BOOL_T is_inherit  =TRUE;

    memset(&result, 0, sizeof(SWCTRL_Cable_Info_T));

    if(arg[0]==NULL)    /* show all */
    {
        UI32_T  trunk_id;

        while(SWCTRL_POM_GetNextCableDiagResult(&l_port, &result)==TRUE)
        {
            SWCTRL_POM_LogicalPortToUserPort(l_port, &unit, &port, &trunk_id);

            line_num = cli_api_show_cable_diag_one_port(unit, port, result, line_num);
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
    else
    {
        unit = atoi(arg[1]);
        port = atoi(strchr(arg[1], '/')+1);

        if( SWCTRL_LPORT_UNKNOWN_PORT ==
                SWCTRL_POM_UIUserPortToIfindex( unit, port,&l_port,&is_inherit))
        {
            CLI_LIB_PrintStr("Failed to convert port to ifIndex\r\n");
            return CLI_NO_ERROR;
        }
        if(SWCTRL_POM_GetCableDiagResult(l_port, &result)!=TRUE)
        {
            CLI_LIB_PrintStr_2("Failed to get test result of interface Ethernet %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
            return CLI_NO_ERROR;
        }
        else
        {
            line_num = cli_api_show_cable_diag_one_port(unit, port, result, line_num);
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Test_Cable_Diag(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) && (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL))
    char  Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char  delemiters[2] = {0};
    char  *s;
    BOOL_T  is_inherit = TRUE;
    UI32_T  l_port =0, unit, port;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    SWCTRL_Cable_Info_T  result;
    delemiters[0] = ',';

    memset(&result, 0, sizeof(SWCTRL_Cable_Info_T));

    if ((arg[2] != NULL)&&(isdigit(arg[2][0])))
    {
        s = (char*)arg[2];

        /*get the unit*/
        unit = atoi(s);/*pttch stacking*/
        ctrl_P->CMenu.unit_id = unit;

        /*move the ptr to just after the slash */
        s = strchr(s, '/') + 1;

        while(1)
        {
            s =  CLI_LIB_Get_Token(s, Token, delemiters);

            if(CLI_LIB_CheckNullStr(Token))
            {
                if(s == 0)
                    break;
                else
                    continue;
            }
            else
            {
                CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);

                /*
                   bit7    ...   bit1 bit0 bit7    ...   bit1 bit0 bit7    ...   bit1 bit0
                   |-----------------------|-----------------------|-----------------------|--
                   |        Byte 0         |        Byte 1         |       Byte 2          |   ...
                   |-----------------------|-----------------------|-----------------------|--
                   port1  ...  port7 port8 port9 ... port15 port16 port17 ... port23 port24
                 */

                for(port=lower_val; port<=upper_val; port++)
                {
                    if( SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex( unit, port, &l_port, &is_inherit))
                    {
                        CLI_LIB_PrintStr("Failed to convert port to ifIndex\r\n");
                        return CLI_NO_ERROR;
                    }

                    if(SWCTRL_PMGR_ExecuteCableDiag(l_port, &result)!=TRUE)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to execute cable diagnostics on unit %lu, port %lu.\r\n", (unsigned long)unit, (unsigned long)port);
#endif
                    }
                }
            }

            if(s == 0)
                break;
            else
                memset(Token, 0, sizeof(Token));
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }

#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Test_Cable_Diag_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) && (SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP == SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL))
    char  Token[CLI_DEF_MAX_BUFSIZE] = {0};//pttch
    char  delemiters[2] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE + 1] = {0};
    char  *s;
    BOOL_T is_inherit = TRUE;
    UI32_T line_num = 0;
    UI32_T l_port = 0;
    UI32_T  unit, port;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    SWCTRL_Cable_Info_T result;
    delemiters[0] = ',';

    memset(&result, 0, sizeof(SWCTRL_Cable_Info_T));

    if ((arg[1] != NULL)&&(isdigit(arg[1][0])))
    {
        s = (char*)arg[1];

        /*get the unit*/
        unit = atoi(s);/*pttch stacking*/
        ctrl_P->CMenu.unit_id = unit;

        /*move the ptr to just after the slash */
        s = strchr(s, '/') + 1;
        PROCESS_MORE("TF: Test failed\r\n");
        PROCESS_MORE("OK: OK\r\n");
        PROCESS_MORE("ON: Open\r\n");
        PROCESS_MORE("ST: Short\r\n");
        PROCESS_MORE("IE: Impedance error\r\n");
        PROCESS_MORE("NC: No cable\r\n");
        PROCESS_MORE("NT: Not tested\r\n");
        PROCESS_MORE("NS: Not supported\r\n");
        PROCESS_MORE("UN: Unknown\r\n");
        PROCESS_MORE("Port     Type Link     Pair A   Pair B   Pair C   Pair D   Last\r\n");
        PROCESS_MORE("              Status   meters   meters   meters   meters   Updated\r\n");
        PROCESS_MORE("-------- ---- -------- -------- -------- -------- -------- -------------------\r\n");

        while(1)
        {
            s =  CLI_LIB_Get_Token(s, Token, delemiters);

            if(CLI_LIB_CheckNullStr(Token))
            {
                if(s == 0)
                    break;
                else
                    continue;
            }
            else
            {
                CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);

                /*
                   bit7    ...   bit1 bit0 bit7    ...   bit1 bit0 bit7    ...   bit1 bit0
                   |-----------------------|-----------------------|-----------------------|--
                   |        Byte 0         |        Byte 1         |       Byte 2          |   ...
                   |-----------------------|-----------------------|-----------------------|--
                   port1  ...  port7 port8 port9 ... port15 port16 port17 ... port23 port24
                 */
                for(port=lower_val; port<=upper_val; port++)
                {
                    if( SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex( unit, port, &l_port, &is_inherit))
                    {
                        CLI_LIB_PrintStr("Failed to convert port to ifIndex\r\n");
                        return CLI_NO_ERROR;
                    }

                    if(SWCTRL_POM_GetCableDiagResult(port, &result)!=TRUE)
                    {
                        CLI_LIB_PrintStr_2("Failed to get test result of interface Ethernet %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
                    }
                    else
                    {
                        line_num = cli_api_show_cable_diag_one_port(unit, port, result, line_num, TRUE);

                        if (line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                    }
                }
            }

            if(s == 0)
                break;
            else
                memset(Token, 0, sizeof(Token));
        }
    }
    else  /* show all */
    {
        PROCESS_MORE("TF: Test failed\r\n");
        PROCESS_MORE("OK: OK\r\n");
        PROCESS_MORE("ON: Open\r\n");
        PROCESS_MORE("ST: Short\r\n");
        PROCESS_MORE("IE: Impedance error\r\n");
        PROCESS_MORE("NC: No cable\r\n");
        PROCESS_MORE("NT: Not tested\r\n");
        PROCESS_MORE("NS: Not supported\r\n");
        PROCESS_MORE("UN: Unknown\r\n");
        PROCESS_MORE("Port     Type Link     Pair A   Pair B   Pair C   Pair D   Last\r\n");
        PROCESS_MORE("              Status   meters   meters   meters   meters   Updated\r\n");
        PROCESS_MORE("-------- ---- -------- -------- -------- -------- -------- -------------------\r\n");

        while(SWCTRL_POM_GetNextCableDiagResult(&l_port, &result)==TRUE)
        {
            UI32_T  trunk_id;
            SWCTRL_POM_LogicalPortToUserPort(l_port, &unit, &port, &trunk_id);
            line_num = cli_api_show_cable_diag_one_port(unit, port, result, line_num, TRUE);
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
#endif    /* #if (SYS_CPNT_SWCTRL_CABLE_DIAG == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Test_Loop_Internal_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_INTERNAL_LOOPBACK_TEST==TRUE)
    BOOL_T is_inherit = TRUE;
    UI32_T l_port = 0, unit = 0, port = 0;
    UI32_T result_status = 0;
    UI32_T result_time = 0;

    unit = atoi(arg[1]);
    port = atoi(strchr(arg[1], '/') + 1);
    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(unit, port, &l_port, &is_inherit))
    {
        CLI_LIB_PrintStr("Failed to convert port to ifIndex.\r\n");
        return CLI_NO_ERROR;
    }

    if (   FALSE == SWCTRL_PMGR_ExecuteInternalLoopBackTest(l_port)
        || FALSE == SWCTRL_POM_GetInternalLoopbackTestResult(l_port, &result_status, &result_time))
    {
        CLI_LIB_PrintStr ("Failed to do internal loopback on the port.\r\n");
        return CLI_NO_ERROR;
    }


    if (result_status == VAL_loopInternalResultStatus_succeeded)
    {
        CLI_LIB_PrintStr ("Internal loopback test: succeeded.\r\n");
    }
    else if (result_status == VAL_loopInternalResultStatus_failed)
    {
        CLI_LIB_PrintStr ("Internal loopback test: failed.\r\n");
    }
    else
    {
        CLI_LIB_PrintStr ("Failed to do internal loopback on the port.\r\n");
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_Show_Loop_Internal_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_INTERNAL_LOOPBACK_TEST==TRUE)

    UI32_T line_num = 0;
    UI32_T l_port = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T  result_status=0 ;
    UI32_T  result_time=0 ;
    UI8_T  result_status_ar[10] = {0};
    UI8_T  result_time_ar[20] = {0};
    BOOL_T is_inherit  =TRUE;

    sprintf((char *)buff, " \r\n Port      Test Result     Last Update\r\n");
    PROCESS_MORE(buff);
    sprintf((char *)buff, " --------  --------------  --------------------\r\n");
    PROCESS_MORE(buff);

    if(arg[0]==NULL)    /* show all */
    {
        while(SWCTRL_POM_GetNextInternalLoopbackTestResult(&l_port, &result_status, &result_time)==TRUE)
        {
            UI32_T  unit, port, trunk_id;
            SWCTRL_POM_LogicalPortToUserPort(l_port, &unit, &port, &trunk_id);

            switch(result_status)
            {
                case VAL_loopInternalResultStatus_succeeded:
                    sprintf((char *)result_status_ar, "Succeeded");
                    break;

                case VAL_loopInternalResultStatus_failed:
                    sprintf((char *)result_status_ar, "Failed");
                    break;
                case VAL_loopInternalResultStatus_notTestedYet:
                    sprintf((char *)result_status_ar, "Not test");
            }

            cli_api_show_test_time(result_time, result_time_ar);
            sprintf((char *)buff, " Eth %lu/%-2lu  %14s  %s\r\n", (unsigned long)unit, (unsigned long)port, result_status_ar, result_time_ar);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        UI32_T  unit, port;

        unit = atoi(arg[1]);
        port = atoi(strchr(arg[1], '/')+1);

        if( SWCTRL_LPORT_UNKNOWN_PORT ==SWCTRL_POM_UIUserPortToIfindex( unit, port, &l_port, &is_inherit))
        {
            CLI_LIB_PrintStr("Failed to convert port to ifIndex\r\n");
            return CLI_NO_ERROR;
        }

        if(SWCTRL_POM_GetInternalLoopbackTestResult(l_port, &result_status, &result_time)!=TRUE)
        {
            CLI_LIB_PrintStr_2("Failed to get test result of interface Ethernet %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
            return CLI_NO_ERROR;
        }
        else
        {
            switch(result_status)
            {
                case VAL_loopInternalResultStatus_succeeded:
                    sprintf((char *)result_status_ar, "Succeeded");
                    break;

                case VAL_loopInternalResultStatus_failed:
                    sprintf((char *)result_status_ar, "Failed");
                    break;
                case VAL_loopInternalResultStatus_notTestedYet:
                    sprintf((char *)result_status_ar, "Not test");
            }
            cli_api_show_test_time(result_time, result_time_ar);
            sprintf((char *)buff, " Eth %lu/%-2lu  %14s  %s\r\n", (unsigned long)unit, (unsigned long)port, result_status_ar, result_time_ar);
            PROCESS_MORE(buff);
        }
    }
#endif

    return CLI_NO_ERROR;

}

static UI32_T AtoIp(UI8_T *hostip_p, UI8_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI8_T token[20];
    int   i,j;  /* i for s[]; j for token[] */
    int   k;    /* k for ip[] */
    UI8_T temp[4];
    char mayString = FALSE;

    /* BODY */
    i = 0;
    j = 0;
    k = 0;

    while (hostip_p[i] != '\0')
    {
        if (hostip_p[i] == '.')
        {
            token[j] = '\0';
            if(atoi((char *)token) > 255|| strlen((char *)token) > 3)
            {
                mayString = TRUE;
            }
            if (strlen((char *)token) < 1||
                    atoi((char *)token) < 0 )
            {
                return ATOIP_INVALID_IP;
            }
            else if (k >= 4)
            {
                return ATOIP_INVALID_IP;
            }
            else
            {
                temp[k++] =(UI8_T)atoi((char *)token);
                i++; j = 0;
            }
        }
        else if (!(hostip_p[i] >= '0' && hostip_p[i] <= '9'))
        {
            return ATOIP_NON_IP_STRING;
        }
        else
        {
            token[j++] = hostip_p[i++];
        }

    } /* while */

    token[j] = '\0';
    if(atoi((char *)token) > 255|| strlen((char *)token) > 3)
    {
        mayString = TRUE;
    }
    if (strlen((char *)token) < 1 ||
            atoi((char *)token) < 0)
    {
        return ATOIP_INVALID_IP;
    }
    else if (k != 3)
    {
        return ATOIP_INVALID_IP;
    }

    if(mayString == TRUE)
    {
        return ATOIP_INVALID_IP;
    }

    temp[k]=(UI8_T)atoi((char *)token);

    ip_p[0] = temp[0];
    ip_p[1] = temp[1];
    ip_p[2] = temp[2];
    ip_p[3] = temp[3];

    return ATOIP_OK;
} /* End of AtoIp() */

UI32_T HostString2IP(UI8_T *hoststr_p, UI32_T *ip_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DECLARATIONS
     */
    UI32_T  ip_addr = 0;
#if (SYS_CPNT_DNS == TRUE)
    L_INET_AddrIp_T hostip_ar[8]; /* 8, MAXHOSTIPNUM */
#endif /* #if (SYS_CPNT_DNS == TRUE) */

    UI32_T  ret = 0;

    /* BODY */
    ret = AtoIp(hoststr_p, (UI8_T*)&ip_addr);
    if( ret == ATOIP_OK )
    {
        *ip_p = ip_addr;
        return HOSTSTRING2IP_OK;
    }
    else
    {
        /* according to RFC 1123, hostname could legally be entirely numeric
         * so do DNS processing if converting hostname to IP failed.
         */
#if( SYS_CPNT_DNS == TRUE )
        /* init array */
        memset(hostip_ar, 0, sizeof(L_INET_AddrIp_T) * 8);

        ret = DNS_PMGR_HostNameToIp(hoststr_p, AF_INET, hostip_ar);

        if(ret ==0)
        {
            /* return the first ip as result, need to refine in the future. */
            if(hostip_ar[0].type == L_INET_ADDR_TYPE_IPV4)
            {
                IP_LIB_ArraytoUI32(hostip_ar[0].addr, &ip_addr);
                *ip_p = ip_addr;
                return HOSTSTRING2IP_OK;
            }
            else
            {
                *ip_p=0;
                return HOSTSTRING2IP_NON_EXISTENT_DOMAIIN;
            }
        }
        else if( ret == 3/* DNS_RC_NAMEERR */ )
        {
            *ip_p = 0;
            return HOSTSTRING2IP_NON_EXISTENT_DOMAIIN;
        }
        else if( ret == 2/* DNS_RC_SERVFAIL */ )
        {
            *ip_p = 0;
            return HOSTSTRING2IP_NO_RESPONSE_FROM_SERVER;
        }
        else if( ret == 0/* DNS_RC_OK */ )
        {
            /* *ip_p = ip_addr; */
            return HOSTSTRING2IP_OK;
        }
        else if( ret == 300 )
        {
            *ip_p = 0;
            return HOSTSTRING2IP_DNS_REQUEST_TIMED_OUT;
        }
        else
        {
            *ip_p = 0;
            return HOSTSTRING2IP_NO_RESPONSE_FROM_SERVER;
        }
#else
        *ip_p = 0;
        return HOSTSTRING2IP_INVALID_IP;
#endif
    }
}

/* command: show ip traffic */
#if 0 /* output example */
IPv4 Statistics:
IPv4 recived
3886 rcvd total total received
0 header errors
IPv4 sent
0 forwarded datagrams
ICMPv4 Statistics:
ICMPv4 received
3885 input
ICMPv4 sent
12 output
UDP Statistics:
1 input

#endif
UI32_T CLI_API_Show_Ip_Traffic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    struct ip_mib ipstat;
    struct icmp_mib icmpstat;
    struct udp_mib udpstat;
    struct tcp_mib tcpstat;

    memset(&ipstat, 0, sizeof(ipstat));

    /* get IPv4 statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllIpv4Statistic(&ipstat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("IP Statistics:\r\n");

    /* IPv4 received statistic */
    CLI_LIB_PrintStr("IP received\r\n");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInReceives, "total received");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInHdrErrors, "header errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInUnknownProtos, "unknown protocols");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInAddrErrors, "address errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipInDelivers, "delivers");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipReasmReqds, "reassembly request datagrams");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipReasmOKs, "reassembly succeeded");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipReasmFails, "reassembly failed");

    /* IPv4 sent statistic */

    CLI_LIB_PrintStr("IP sent\r\n");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipForwDatagrams, "forwards datagrams");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipOutRequests, "requests");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipOutDiscards, "discards");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipOutNoRoutes, "no routes");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipFragCreates, "generated fragments");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipFragOKs, "fragment succeeded");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)ipstat.ipFragFails, "fragment failed");


    /* get ICMP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllIcmpStatistic(&icmpstat))
    {
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr("ICMP Statistics:\r\n");

    /* ICMP received statistics */
    CLI_LIB_PrintStr("ICMP received\r\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInMsgs, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInErrors, "errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInParmProbs, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInEchoReps, "echo reply messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInRedirects, "redirect messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInTimestamps, "timestamp request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInTimestampReps, "timestamp reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInSrcQuenchs, "source quench messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInAddrMasks, "address mask request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpInAddrMaskReps, "address mask reply messages");

    /* ICMP sent statistics */
    CLI_LIB_PrintStr("ICMP sent\r\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutMsgs, "output");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutErrors, "errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutDestUnreachs, "destination unreachable messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutTimeExcds, "time exceeded messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutParmProbs, "parameter problem message");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutEchos, "echo request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutEchoReps, "echo reply messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutRedirects, "redirect messages");

    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutTimestamps, "timestamp request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutTimestampReps, "timestamp reply messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutSrcQuenchs, "source quench messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutAddrMasks, "address mask request messages");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)icmpstat.icmpOutAddrMaskReps, "address mask reply messages");


    /* get UDP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllUdpStatistic(&udpstat))
    {
        return CLI_NO_ERROR;
    }

    /* UDP statistics */
    CLI_LIB_PrintStr("UDP Statistics:\r\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udpstat.udpInDatagrams, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udpstat.udpNoPorts, "no port errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udpstat.udpInErrors, "other errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)udpstat.udpOutDatagrams, "output");


    /* get TCP statistics */
    if(IPAL_RESULT_OK  != IPAL_IF_GetAllTcpStatistic(&tcpstat))
    {
        return CLI_NO_ERROR;
    }

    /* TCP statistics */
    CLI_LIB_PrintStr("TCP Statistics:\r\n");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)tcpstat.tcpInSegs, "input");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)tcpstat.tcpInErrs, "input errors");
    CLI_LIB_PrintStr_2("%20.ld %s\r\n", (long)tcpstat.tcpOutSegs, "output");
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Process_Cpu(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    enum {
        DO_NOTHING,
        SET_RISING_THRESHOLD,
        SET_FALLING_THRESHOLD,
    } config_act = DO_NOTHING;
    UI32_T arg_idx = 0;
    UI32_T value_to_set;
    BOOL_T reset_to_default;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_PROCESS_CPU:
            reset_to_default = FALSE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_PROCESS_CPU:
            reset_to_default = TRUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (arg[arg_idx])
    {
        switch (arg[arg_idx++][0])
        {
            case 'r':
            case 'R':
                if (reset_to_default)
                {
                    value_to_set = SYS_DFLT_SYSMGR_CPU_UTILIZATION_RAISING_THRESHOLD;
                }
                else if (arg[arg_idx])
                {
                    value_to_set = atoi(arg[arg_idx++]);
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                config_act = SET_RISING_THRESHOLD;
                break;

            case 'f':
            case 'F':
                if (reset_to_default)
                {
                    value_to_set = SYS_DFLT_SYSMGR_CPU_UTILIZATION_FALLING_THRESHOLD;
                }
                else if (arg[arg_idx])
                {
                    value_to_set = atoi(arg[arg_idx++]);
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                config_act = SET_FALLING_THRESHOLD;
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    switch (config_act)
    {
        case SET_RISING_THRESHOLD:
            if (!SYS_PMGR_SetCpuUtilRisingThreshold(value_to_set))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set rising threshold of CPU utilization alarm.\r\n");
#endif
            }
            break;

        case SET_FALLING_THRESHOLD:
            if (!SYS_PMGR_SetCpuUtilFallingThreshold(value_to_set))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set rising threshold of CPU utilization alarm.\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Cpu_Guard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    UI32_T value;
    BOOL_T is_set_to_default;
    BOOL_T is_enabled;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_PROCESS_CPU_GUARD:
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_PROCESS_CPU_GUARD:
            is_enabled = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_PROCESS_CPU_GUARD);

            if (!SYS_PMGR_SetCpuGuardStatus(is_enabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard status.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_HIGHWATERMARK:
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_HIGHWATERMARK:
            is_set_to_default = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_HIGHWATERMARK);

            if (is_set_to_default)
                value = SYS_DFLT_CPU_UTILIZATION_WATERMARK_HIGH;
            else if (arg[0] != NULL)
                value = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            if (!SYS_PMGR_SetCpuGuardHighWatermark(value))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard high watermark.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_LOWWATERMARK:
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_LOWWATERMARK:
            is_set_to_default = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_LOWWATERMARK);

            if (is_set_to_default)
                value = SYS_DFLT_CPU_UTILIZATION_WATERMARK_LOW;
            else if (arg[0] != NULL)
                value = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            if (!SYS_PMGR_SetCpuGuardLowWatermark(value))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard low watermark.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_MAXTHRESHOLD:
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_MAXTHRESHOLD:
            is_set_to_default = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_MAXTHRESHOLD);

            if (is_set_to_default)
                value = SYS_DFLT_CPU_GUARD_THRESHOLD_MAX;
            else if (arg[0] != NULL)
                value = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            if (!SYS_PMGR_SetCpuGuardMaxThreshold(value))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard maximum threshold.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_MINTHRESHOLD:
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_MINTHRESHOLD:
            is_set_to_default = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_MINTHRESHOLD);

            if (is_set_to_default)
                value = SYS_DFLT_CPU_GUARD_THRESHOLD_MIN;
            else if (arg[0] != NULL)
                value = atoi(arg[0]);
            else
                return CLI_ERR_INTERNAL;

            if (!SYS_PMGR_SetCpuGuardMinThreshold(value))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard minimum threshold.\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_TRAP:
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_PROCESS_CPU_GUARD_TRAP:
            is_enabled = (cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W4_PROCESS_CPU_GUARD_TRAP);

            if (!SYS_PMGR_SetCpuGuardTrapStatus(is_enabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set CPU guard trap status.\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Memory(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    enum {
        DO_NOTHING,
        SET_RISING_THRESHOLD,
        SET_FALLING_THRESHOLD,
    } config_act = DO_NOTHING;
    UI32_T arg_idx = 0;
    UI32_T value_to_set;
    BOOL_T reset_to_default;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_MEMORY:
            reset_to_default = FALSE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_MEMORY:
            reset_to_default = TRUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (arg[arg_idx])
    {
        switch (arg[arg_idx++][0])
        {
            case 'r':
            case 'R':
                if (reset_to_default)
                {
                    value_to_set = SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_RAISING_THRESHOLD;
                }
                else if (arg[arg_idx])
                {
                    value_to_set = atoi(arg[arg_idx++]);
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                config_act = SET_RISING_THRESHOLD;
                break;

            case 'f':
            case 'F':
                if (reset_to_default)
                {
                    value_to_set = SYS_DFLT_SYSMGR_MEMORY_UTILIZATION_FALLING_THRESHOLD;
                }
                else if (arg[arg_idx])
                {
                    value_to_set = atoi(arg[arg_idx++]);
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                config_act = SET_FALLING_THRESHOLD;
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    switch (config_act)
    {
        case SET_RISING_THRESHOLD:
            if (!SYS_PMGR_SetMemoryUtilRisingThreshold(value_to_set))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set rising threshold of CPU utilization alarm.\r\n");
#endif
            }
            break;

        case SET_FALLING_THRESHOLD:
            if (!SYS_PMGR_SetMemoryUtilFallingThreshold(value_to_set))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set rising threshold of CPU utilization alarm.\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Control_Plane(
        UI16_T cmd_idx,
        char *arg[],
        CLI_TASK_WorkingArea_T *ctrl_P)
{
    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_CONTROL_PLANE_MODE;
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_CRAFT_PORT == TRUE)

UI32_T CLI_API_Interface_Craft(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    memset(ctrl_P->CMenu.port_id_list, 0, sizeof(ctrl_P->CMenu.port_id_list));
    ctrl_P->CMenu.vlan_ifindex = 0;
    ctrl_P->CMenu.pchannel_id = 0;
    ctrl_P->CMenu.loopback_id = 0;

    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_CRAFT_MODE;
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Craft_Interface_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T uint_ipMask=0;
    UI8_T   byte_mask[SYS_ADPT_IPV4_ADDR_LEN];
    NETCFG_TYPE_CraftInetAddress_T craft_addr;
    UI32_T ret;
    memset(&craft_addr, 0, sizeof(craft_addr));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_IP_ADDRESS:

            if(arg[0] == NULL)
                return CLI_ERR_INTERNAL;

            if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
            {
                /* format: 192.168.1.1 255.255.255.0 */

                /* convert to byte array format */
                IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                if(!IP_LIB_IsValidNetworkMask(byte_mask))
                {
                    CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
                    return CLI_NO_ERROR;
                }

                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                   arg[0],
                                                                   (L_INET_Addr_T *) &craft_addr.addr,
                                                                   sizeof(craft_addr.addr)))
                {
                    CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                    return CLI_NO_ERROR;
                }

                craft_addr.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
            }
            else
            {
                /* format: 192.168.1.1/24 */
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                   arg[0],
                                                                   (L_INET_Addr_T *) &craft_addr.addr,
                                                                   sizeof(craft_addr.addr)))
                {
                    CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                    return CLI_NO_ERROR;
                }
                IP_LIB_CidrToMask(craft_addr.addr.preflen, byte_mask);
            }

            ret = IP_LIB_IsValidForIpConfig(craft_addr.addr.addr, byte_mask);
            switch(ret)
            {
                case IP_LIB_INVALID_IP:
                case IP_LIB_INVALID_IP_LOOPBACK_IP:
                case IP_LIB_INVALID_IP_ZERO_NETWORK:
                case IP_LIB_INVALID_IP_BROADCAST_IP:
                case IP_LIB_INVALID_IP_IN_CLASS_D:
                case IP_LIB_INVALID_IP_IN_CLASS_E:
                    CLI_LIB_PrintStr("Invalid IP address\r\n");
                    return CLI_NO_ERROR;
                    break;
                case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
                    CLI_LIB_PrintStr("Invalid IP address. Can't be network ID.\r\n");
                    return CLI_NO_ERROR;
                    break;
                case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:

                    CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast IP.\r\n");
                    return CLI_NO_ERROR;

                default:
                    break;
            }

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_UDHCPC_CRAFT_PORT == TRUE)
            CLI_MGR_SetDoneFlagUdhcpc(TRUE);
            CLI_MGR_StopUdhcpc();
#endif
            /* set craft interface's ip address*/
            craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            craft_addr.row_status = VAL_netConfigStatus_2_createAndGo;
            ret = NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr);
            switch(ret)
            {
                case NETCFG_TYPE_OK:
                    /* do nothing */
                    break;
                case NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT:
                    CLI_LIB_PrintStr("Failed. Due to a static route's nexthop conflicts on normal interface and craft interface.\r\n");
                    break;
                case NETCFG_TYPE_FAIL:
                default:
                    CLI_LIB_PrintStr("Failed to add IP address of craft port.\r\n");
                    break;
            }

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_UDHCPC_CRAFT_PORT == TRUE)
            CLI_MGR_SetDoneFlagUdhcpc(ret == NETCFG_TYPE_OK);
#endif
            break;

        case PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W3_NO_IP_ADDRESS:

            if(arg[0] == NULL)
                return CLI_ERR_INTERNAL;

            craft_addr.ifindex = SYS_ADPT_CRAFT_INTERFACE_IFINDEX;
            craft_addr.row_status = VAL_netConfigStatus_2_destroy;

            if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
            {
                    /* format: 192.168.1.1 255.255.255.0 */

                    /* convert to byte array format */
                    IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                    if(!IP_LIB_IsValidNetworkMask(byte_mask))
                    {
                            CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
                            return CLI_NO_ERROR;
                    }

                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                   arg[0],
                                                                   (L_INET_Addr_T *) &craft_addr.addr,
                                                                   sizeof(craft_addr.addr)))
                {
                    CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                    return CLI_NO_ERROR;
                }

                craft_addr.addr.preflen = IP_LIB_MaskToCidr(byte_mask);
            }
            else
            {
                /* format: 192.168.1.1/24 */
                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                   arg[0],
                                                                   (L_INET_Addr_T *) &craft_addr.addr,
                                                                   sizeof(craft_addr.addr)))
                {
                    CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                    return CLI_NO_ERROR;
                }
            }

            if(NETCFG_PMGR_IP_SetCraftInterfaceInetAddress(&craft_addr) != NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("Failed to delete IP address of craft port.\r\n");
            }
#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_UDHCPC_CRAFT_PORT == TRUE)
            else
            {
                CLI_MGR_SetDoneFlagUdhcpc(FALSE);
            }
#endif

            break;

        default:
            return CLI_ERR_INTERNAL;
    } /* switch */

    return CLI_NO_ERROR;

}
#endif /* SYS_CPNT_CRAFT_PORT */

UI32_T CLI_API_SyncE_SSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYNCE == TRUE)
    UI32_T verify_unit = atoi((char*)arg[0]);
    UI32_T verify_port = atoi(strchr((char*)arg[0],'/')+1);
    UI32_T verify_ifindex = 0;
    UI16_T priority = SYS_DFLT_SYNC_E_SSM_PRIORITY;
    CLI_API_EthStatus_T verify_ret;

    verify_ret = verify_ethernet(verify_unit, verify_port, &verify_ifindex );
    /*support on user port*/
    if( verify_ret == CLI_API_ETH_NOT_PRESENT
            || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
    {
        display_ethernet_msg(verify_ret, verify_unit, verify_port);
        return CLI_NO_ERROR;
    }

    if(arg[1]!=NULL &&(arg[1][0] == 'p' || arg[1][0] == 'P'))
        priority = atoi(arg[2]);

    if(SYNC_E_TYPE_IS_VALID_IFINDEX_RANGE(verify_ifindex) == FALSE)
    {
        CLI_LIB_PrintStr_2("Failed to SSM, not support SyncE range. (%d-%d)\r\n",
                SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX, SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_SYNCE_SSM_ETHERNET:
            if(SYNC_E_TYPE_RET_SUCCESS !=SYNC_E_PMGR_SetPortSsm(verify_ifindex, TRUE, priority))
            {
                CLI_LIB_PrintStr("Failed to SSM\r\n");
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SYNCE_SSM_ETHERNET:
            if(SYNC_E_TYPE_RET_SUCCESS!=SYNC_E_PMGR_SetPortSsm(verify_ifindex, FALSE, priority))
            {
                CLI_LIB_PrintStr("Failed to SSM\r\n");
            }
            break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_SyncE_ClockSourceSSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYNCE == TRUE)
    SYNC_E_TYPE_RET_T ret = SYNC_E_TYPE_RET_FAIL;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_CLKSRCSSM:
            ret = SYNC_E_PMGR_SetClkSrcSsm(TRUE);
            if(SYNC_E_TYPE_RET_FAIL_BAD_ARG == ret)
            {
                CLI_LIB_PrintStr("Failed to set, because auto clock source selection is enabled"NEWLINE);
            }
            else
                if(SYNC_E_TYPE_RET_FAIL_CONFLICT== ret)
                {
                    CLI_LIB_PrintStr("Failed to set, because it already specifies clock source"NEWLINE);
                }
                else
                    if(SYNC_E_TYPE_RET_SUCCESS != ret)
                    {
                        CLI_LIB_PrintStr("Failed to set clock source selection through SSM"NEWLINE);
                    }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_CLKSRCSSM:
            if(SYNC_E_TYPE_RET_SUCCESS!=SYNC_E_PMGR_SetClkSrcSsm(FALSE))
            {
                CLI_LIB_PrintStr("Failed to set clock source selection through SSM"NEWLINE);
            }
            break;
    }
#endif
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_SYNCE == TRUE)
static UI32_T cli_api_synce_display_status(UI32_T line_num, char *buff)
{
    SWCTRL_Lport_Type_T ret;
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    SYNCEDRV_TYPE_ClockSource_T* clock_src_p;
    UI32_T unit, port, trunk_id,clock_src_num;
    UI32_T clock_src_get_ret_val, i, j;
    UI8_T  pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    BOOL_T synce_enable_status, locked_status;

    /* display SyncE status
     * SyncE Status:
     * Port       Status    Clock Source
     * ---------  --------  ------------
     * Eth  1/ 9  Enabled        Yes
     * Eth  1/10  Enabled        Yes
     * Eth  1/11  Enabled        No
     * Eth  1/12  Disabled       No
     */
    sprintf(buff, "SyncE Status:"NEWLINE);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "Port       Status    Clock Source"NEWLINE);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "---------  --------  ------------"NEWLINE);
    PROCESS_MORE_FUNC(buff);
    SYNCEDRV_OM_GetSyncEEnabledPbmp(pbmp);
    clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);

    for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
            i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
    {
        ret = SWCTRL_POM_LogicalPortToUserPort(i, &unit, &port, &trunk_id);

        if(ret == SWCTRL_LPORT_TRUNK_PORT
                || ret == SWCTRL_LPORT_UNKNOWN_PORT)
            continue;

        if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
            continue;

        if(L_PBMP_GET_PORT_IN_PBMP_ARRAY(pbmp, port))
            synce_enable_status=TRUE;
        else
            synce_enable_status=FALSE;

        /* get clock source info of the port if available
         */
        for(j=0, clock_src_p=NULL; j<clock_src_num; j++)
        {
            if(clock_src_lst[j].port==port)
            {
                clock_src_p = &(clock_src_lst[j]);
                break;
            }
        }

        sprintf(buff, "Eth %2lu/%2lu  %-8s  %-3s"NEWLINE,
                (unsigned long)unit, (unsigned long)port, (synce_enable_status==TRUE)?"Enabled":"Disabled",
                (clock_src_p==NULL)?"No":"Yes");
        PROCESS_MORE_FUNC(buff);
    }
    return line_num;
}

static UI32_T cli_api_synce_display_clock_source(UI32_T line_num, char *buff)
{
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    UI32_T clock_src_num, mode, chip_op_mode, force_clock_src_ifindex;
    UI32_T clock_src_get_ret_val, i;
    const char* mode_str_p;
    BOOL_T locked_status, is_auto, is_revertive;

    clock_src_get_ret_val = SYNC_E_PMGR_GetSyncEClockSourceStatus(clock_src_lst, &clock_src_num, &locked_status);

    /* display SyncE clock source selection mode
     */
    if(SYNC_E_PMGR_GetClockSrcSelectMode(&mode)==SYNC_E_TYPE_RET_SUCCESS)
    {
        const char* mode_str_p;

        mode_str_p = SYNC_E_PMGR_ClockSrcSelectModeToStr(mode);
        sprintf(buff, "SyncE Clock Source Selection Mode: %s"NEWLINE,
                (mode_str_p==NULL)?"Unknown":mode_str_p);
        PROCESS_MORE_FUNC(buff);

        if(mode==SYNC_E_TYPE_CLOCK_SOURCE_SELECTION_MODE_AUTO)
        {
            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_auto, &is_revertive, &force_clock_src_ifindex)==TRUE)
            {
                sprintf(buff, "Revertive Switiching: %s"NEWLINE,
                        (is_revertive==TRUE)?"Enabled":"Disabled");
                PROCESS_MORE(buff);
            }
        }

    }

    /* display SyncE chip operation mode
     */
    if(SYNC_E_PMGR_GetChipOperatingMode(&chip_op_mode)==SYNC_E_TYPE_RET_SUCCESS)
    {
        mode_str_p = SYNC_E_PMGR_SyncEChipOperatingModeToStr(chip_op_mode);
        sprintf(buff, "SyncE Chip Operation Mode: %s"NEWLINE,
                (mode_str_p==NULL)?"Unknown":mode_str_p);
        PROCESS_MORE(buff);
    }
    else
    {
        sprintf(buff, "SyncE Chip Operating Mode: Error to retrieve op mode"NEWLINE);
        PROCESS_MORE(buff);
    }

    if(clock_src_get_ret_val==SYNC_E_TYPE_RET_SUCCESS)
    {
        /* display SyncE clock source locked status
         */
        sprintf(buff, "SyncE Active Clock Source Locked: %s"NEWLINE,
                (locked_status==TRUE)?"Yes":"No");
        PROCESS_MORE_FUNC(buff);
        PROCESS_MORE_FUNC(NEWLINE);

        /* display SyncE clock source status
         * SyncE active clock source locked: Yes
         * SyncE clock source status:
         * Port       Priority  Active Clock Source  Clock Status
         * ---------  --------  -------------------  ------------
         * Eth  1/ 9         1         Yes               Good
         * Eth  1/10     65535         No                Bad
         */
        sprintf(buff, "SyncE Clock Source Status:"NEWLINE);
        PROCESS_MORE_FUNC(buff);
        sprintf(buff, "Port       Priority  Active Clock Source  Clock Status"NEWLINE);
        PROCESS_MORE_FUNC(buff);
        sprintf(buff, "---------  --------  -------------------  ------------"NEWLINE);
        PROCESS_MORE_FUNC(buff);
        for(i=0; i<clock_src_num; i++)
        {
            sprintf(buff, "Eth %2u/%2lu  %8lu          %-3s               %-4s"NEWLINE,
                    1, (unsigned long)clock_src_lst[i].port, (unsigned long)clock_src_lst[i].priority,
                    (clock_src_lst[i].is_active==TRUE)?"Yes":"No",
                    (clock_src_lst[i].is_good_status==TRUE)?"Good":"Bad");
            PROCESS_MORE_FUNC(buff);
        }
    }
    return line_num;
}

static UI32_T cli_api_synce_display_ssm(UI32_T line_num, char *buff)
{
    SWCTRL_Lport_Type_T ret;
    UI32_T unit, port, trunk_id;
    SYNC_E_MGR_PortEntry_T port_entry;
    UI32_T i;

    sprintf(buff, "SyncE SSM Status:"NEWLINE);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s %-8s %-3s %-7s %-7s"NEWLINE, "Port",      "Status",   "Pri", "Tx SSM",  "Rx SSM");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s %-8s %-3s %-7s %-7s"NEWLINE, "---------", "--------", "---", "-------", "-------");
    PROCESS_MORE_FUNC(buff);
    for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
            i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
    {
        const char* tx_ql_str, *rx_ql_str;

        port_entry.ifindex = i;

        ret = SWCTRL_POM_LogicalPortToUserPort(i, &unit, &port, &trunk_id);

        if(ret == SWCTRL_LPORT_TRUNK_PORT
                || ret == SWCTRL_LPORT_UNKNOWN_PORT)
            continue;

        if(STKTPLG_BOARD_IsSyncEPort(port)==FALSE)
            continue;

        if(SYNC_E_PMGR_GetPortEntry(&port_entry) != SYNC_E_TYPE_RET_SUCCESS)
            continue;

        tx_ql_str=SYNC_E_PMGR_QualityLevelTypeToStr(port_entry.tx_ql);
        rx_ql_str=SYNC_E_PMGR_QualityLevelTypeToStr(port_entry.rx_ql);

        sprintf(buff, "Eth %2lu/%2lu %-8s %3d %-7s %-7s"NEWLINE, (unsigned long)unit, (unsigned long)port,
                port_entry.is_eanbled?"Enabled":"Disabled",
                port_entry.pri,
                tx_ql_str,
                rx_ql_str);
        PROCESS_MORE_FUNC(buff);
    }
    return line_num;
}
#endif

UI32_T CLI_API_Show_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYNCE==TRUE)
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    line_num = cli_api_synce_display_status(line_num, buff);
    if(line_num == EXIT_SESSION_MORE
            || line_num == JUMP_OUT_MORE)
    {
        goto EXIT;
    }

    line_num = cli_api_synce_display_clock_source(line_num, buff);
    if(line_num == EXIT_SESSION_MORE
            || line_num == JUMP_OUT_MORE)
    {
        goto EXIT;
    }

    line_num = cli_api_synce_display_ssm(line_num, buff);
    if(line_num == EXIT_SESSION_MORE
            || line_num == JUMP_OUT_MORE)
    {
        goto EXIT;
    }

EXIT:
#endif /* end if #if (SYS_CPNT_SYNCE==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_SyncE_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SYNCE == TRUE)
    UI32_T line_num=0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    cli_api_synce_display_status(line_num, buff);

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_SyncE_SSM(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SYNCE == TRUE)
    UI32_T line_num=0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    cli_api_synce_display_ssm(line_num, buff);

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_SyncE_ClockSource(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SYNCE == TRUE)
    UI32_T line_num=0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    cli_api_synce_display_clock_source(line_num, buff);

#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Set_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYNCE==TRUE)
    UI32_T unit = 0, i;
    UI32_T max_port_num = SWCTRL_POM_UIGetUnitPortNumber(1);
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx, port, ifindex, force_clock_src_ifindex_org;
    UI32_T clock_src_lst_len, priority;
    SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    char*  s;
    char   delemiters[2] = {',' ,'\0'};
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  not_present_port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    UI8_T  present_port_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    UI8_T  lport_pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    BOOL_T is_any_one_not_present = FALSE;
    BOOL_T is_enabled_org, is_revertive_org, is_revertive;

    if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_AUTOCLOCKSOURCESELECTING)
    {
        /* synce auto-clock-source-selecting [revertive-switching]
         *                                   arg[0]
         */

        if (arg[0])
        {
            is_revertive=TRUE;
        }
        else
        {
            is_revertive=FALSE;
        }

        if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
        {
            CLI_LIB_PrintStr("Failed to get current config of clock source selection mode\r\n");
            return CLI_NO_ERROR;
        }

        if (SYNC_E_PMGR_SetClockSrcSelectMode(TRUE, is_revertive, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to change revertive-switching status in auto-clock-source-selecting\r\n");
            return CLI_NO_ERROR;
        }

    }
    else if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_AUTOCLOCKSOURCESELECTING)
    {
        /* no synce auto-clock-source-selecting [revertive-switching]
         *                                      arg[0]
         */
        if(arg[0])
        {
            /* disable revertive-switching
             */
            if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
            {
                CLI_LIB_PrintStr("Failed to get current config of clock source selection mode\r\n");
                return CLI_NO_ERROR;
            }

            if(is_enabled_org==FALSE)
            {
                CLI_LIB_PrintStr("Revertive switching setting can only be configured in auto-clock-source-selection mode\r\n");
                return CLI_NO_ERROR;
            }

            if (SYNC_E_PMGR_SetClockSrcSelectMode(TRUE, FALSE, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to disable revertive-switching status in auto-clock-source-selecting\r\n");
                return CLI_NO_ERROR;
            }
        }
        else
        {
            /* change to manual clock-source-selecting */
            if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE /* dont care */, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to change to manual-clock-source-selecting\r\n");
                return CLI_NO_ERROR;
            }
        }
    }
    else if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_ETHERNET ||
            cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W1_SYNCE)
    {
        /* Handle synce enable START
         */
        if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_ETHERNET)
        {
            /* synce ethernet <unit/port>
             *                arg0
             * synce ethernet <unit/port> [clock-source [priority priority] ]
             *                arg0        arg1          arg2     arg3
             */
            /* get the unit
             */
            s = arg[0];
            unit = atoi(s);

            /* move the ptr to just after the slash
             */
            s = strchr(s, '/') + 1;

            while (1)
            {
                s =  CLI_LIB_Get_Token(s, Token, delemiters);

                if (CLI_LIB_CheckNullStr(Token))
                {
                    if(s == 0)
                        break;
                    else
                        continue;
                }
                else
                {
                    CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);
                    for (i=lower_val; i<=upper_val; i++)
                    {
                        if (!SWCTRL_POM_UIUserPortExisting(unit, i))
                        {
                            is_any_one_not_present = TRUE;
                            L_PBMP_SET_PORT_IN_PBMP_ARRAY(not_present_port_list, i);
                        }
                        else
                        {
                            L_PBMP_SET_PORT_IN_PBMP_ARRAY(present_port_list, i);
                        }
                    }
                }

                if (s == 0)
                    break;
                else
                    memset(Token, 0, sizeof(Token));
            }

            if (is_any_one_not_present)
            {
                UI8_T  not_present_num = 0;

                CLI_LIB_PrintStr("Port");
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
                for (i=1; i<=max_port_num; i++)
                {
                    if (L_PBMP_GET_PORT_IN_PBMP_ARRAY(not_present_port_list, i))
                    {
                        not_present_num ++;
                        CLI_LIB_PrintStr_2(" eth %lu/%lu", (unsigned long)unit, (unsigned long)i);
                    }
                }

                if (not_present_num >1)
                    CLI_LIB_PrintStr(" are not present\r\n");
                else
                    CLI_LIB_PrintStr(" is not present\r\n");

                return CLI_NO_ERROR;
            }

            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(present_port_list, max_port_num, port)
            {
                if(SWCTRL_POM_UserPortToIfindex(unit, port, &ifindex)==SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    printf("%s(%d): error to convert to lport for unit %lu port %lu\r\n",
                            __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)port);
                    continue;
                }

                L_PBMP_SET_PORT_IN_PBMP_ARRAY(lport_pbmp, ifindex);
            }
        }
        else
        {
            /* synce ethernet
             * enable synce on all synce-capable ports
             */

            for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
                    i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
            {
                if(STKTPLG_BOARD_IsSyncEPort(i)==TRUE)
                {
                    L_PBMP_SET_PORT_IN_PBMP_ARRAY(lport_pbmp, i);
                }
            }

        }

        if (SYNC_E_PMGR_EnableSyncE(lport_pbmp)!=SYNC_E_TYPE_RET_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to enable synce\r\n");
            return CLI_NO_ERROR;
        }
        /* Handle synce enable END
         */

        /* Handle clock-source and priority setting START
         */
        /* synce ethernet unit/port [clock-source [priority priority] ]
         *                arg0      arg1          arg2     arg3
         */
        if (cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_ETHERNET
                &&arg[1]!=NULL/*it has clock-source*/)
        {
            SYNC_E_TYPE_RET_T ret = SYNC_E_TYPE_RET_FAIL;
            priority=0;

            if (arg[2]!=NULL)
                priority = (UI32_T)strtoul(arg[3], NULL, 10);

            clock_src_lst_len=0;
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(present_port_list, max_port_num, port)
            {
                clock_src_lst[clock_src_lst_len].port = port;
                if(arg[3]==NULL) /* user does not specify priority */
                    clock_src_lst[clock_src_lst_len].priority = port; /* use user port id as default setting */
                else
                    clock_src_lst[clock_src_lst_len].priority = priority;
                clock_src_lst_len++;
            }

            ret = SYNC_E_PMGR_UpdateSyncEClockSource(unit, clock_src_lst, clock_src_lst_len);

            if(SYNC_E_TYPE_RET_FAIL_CONFLICT == ret)
            {
                CLI_LIB_PrintStr("Failed to set clock source because it already specifies using SSM\r\n");
                return CLI_NO_ERROR;
            }
            else
                if (SYNC_E_TYPE_RET_SUCCESS!= ret)
                {
                    CLI_LIB_PrintStr("Failed to set clock source\r\n");
                    return CLI_NO_ERROR;
                }
        }
        /* Handle priority setting END
         */
    }
    else if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_SYNCE_FORCECLOCKSOURCESELECTING)
    {
        CLI_API_EthStatus_T verify_ret;
        UI32_T verify_ifindex;

        if(arg[0]==NULL)
        {
            /* synce force-clock-source-selecting
             * use local clock as synce clock source
             */
            verify_ifindex=0;
        }
        else
        {
            /* synce force-clock-source-selecting <ethernet> unit/port>
             *                                    arg0       arg1
             */
            unit = atoi((char*)arg[1]);
            port = atoi(strchr((char*)arg[1],'/')+1);

            verify_ret = verify_ethernet(unit, port, &verify_ifindex);
            if( verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
            {
                display_ethernet_msg(verify_ret, unit, port);
                return CLI_NO_ERROR;
            }
        }

        if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE /* dont care */, verify_ifindex)!=SYNC_E_TYPE_RET_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to change to force-clock-source-selecting\r\n");
            return CLI_NO_ERROR;
        }
    }
    else if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_SYNCE || cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_ETHERNET)
    {
        if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_ETHERNET)
        {
            /* no synce ethernet <unit/port>
             *                   arg0
             * no synce ethernet <unit/port> [clock-source]
             *                   arg0        arg1
             */
            /* get the unit
             */
            s = arg[0];
            unit = atoi(s);

            /* move the ptr to just after the slash
             */
            s = strchr(s, '/') + 1;

            while (1)
            {
                s =  CLI_LIB_Get_Token(s, Token, delemiters);

                if (CLI_LIB_CheckNullStr(Token))
                {
                    if(s == 0)
                        break;
                    else
                        continue;
                }
                else
                {
                    CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx);
                    for (i=lower_val; i<=upper_val; i++)
                    {
                        if (!SWCTRL_POM_UIUserPortExisting(unit, i))
                        {
                            is_any_one_not_present = TRUE;
                            L_PBMP_SET_PORT_IN_PBMP_ARRAY(not_present_port_list, i);
                        }
                        else
                        {
                            L_PBMP_SET_PORT_IN_PBMP_ARRAY(present_port_list, i);
                        }
                    }
                }

                if (s == 0)
                    break;
                else
                    memset(Token, 0, sizeof(Token));
            }

            if (is_any_one_not_present)
            {
                UI8_T  not_present_num = 0;

                CLI_LIB_PrintStr("Port");
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
                for (i=1; i<=max_port_num; i++)
                {
                    if (L_PBMP_GET_PORT_IN_PBMP_ARRAY(not_present_port_list, i))
                    {
                        not_present_num ++;
                        CLI_LIB_PrintStr_2(" eth %lu/%lu", (unsigned long)unit, (unsigned long)i);
                    }
                }

                if (not_present_num >1)
                    CLI_LIB_PrintStr(" are not present\r\n");
                else
                    CLI_LIB_PrintStr(" is not present\r\n");

                return CLI_NO_ERROR;
            }

            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(1);
            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(present_port_list, max_port_num, port)
            {
                if(SWCTRL_POM_UserPortToIfindex(unit, port, &ifindex)==SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    printf("%s(%d): error to convert to lport for unit %lu port %lu\r\n",
                            __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)port);
                    continue;
                }

                L_PBMP_SET_PORT_IN_PBMP_ARRAY(lport_pbmp, ifindex);
            }
        }
        else
        {
            /* synce ethernet
             * disable synce on all synce-capable ports
             */

            for(i = SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX;
                    i <= SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX; i++)
            {
                if(STKTPLG_BOARD_IsSyncEPort(i)==TRUE)
                {
                    L_PBMP_SET_PORT_IN_PBMP_ARRAY(lport_pbmp, i);
                }
            }
        }

        if (cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_ETHERNET &&
                arg[1]!=NULL)
        {
            /* remove the specified port from the synce clock source
             */
            clock_src_lst_len=0;
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(unit);
            L_PBMP_FOR_EACH_PORT_IN_PBMP_ARRAY(present_port_list, max_port_num, port)
            {
                clock_src_lst[clock_src_lst_len].port=port;
                clock_src_lst_len++;
            }
            if(SYNC_E_PMGR_RemoveSyncEClockSource(unit, clock_src_lst, clock_src_lst_len)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to remove synce clock source\r\n");
                return CLI_NO_ERROR;
            }
        }
        else
        {
            /* disable the synce functionality on the specified port
             */
            if(SYNC_E_PMGR_DisableSyncE(lport_pbmp)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to disable synce\r\n");
                return CLI_NO_ERROR;
            }
        }
    }
    else if(cmd_idx==PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SYNCE_AUTOCLOCKSOURCESELECTING)
    {
        /*
         * no synce auto-clock-source-selecting [revertive-switching]
         *                                      arg[0]
         */
        if(SYNC_E_PMGR_GetClockSourceSelectModeCfg(&is_enabled_org, &is_revertive_org, &force_clock_src_ifindex_org)==FALSE)
        {
            CLI_LIB_PrintStr("Failed to get current config of clock source selection mode\r\n");
            return CLI_NO_ERROR;
        }

        if(arg[0]) /* change revertive switching setting */
        {

            if(is_enabled_org==FALSE)
            {
                CLI_LIB_PrintStr("Revertive switching setting can only be configured in auto-clock-source-selection mode\r\n");
                return CLI_NO_ERROR;
            }

            if (SYNC_E_PMGR_SetClockSrcSelectMode(TRUE, FALSE, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to change revertive-switching status in auto-clock-source-selecting\r\n");
                return CLI_NO_ERROR;
            }
        }
        else /* change to manual clock-source-selecting */
        {
            if (SYNC_E_PMGR_SetClockSrcSelectMode(FALSE, FALSE /* dont care */, force_clock_src_ifindex_org)!=SYNC_E_TYPE_RET_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to change to force-clock-source-selecting\r\n");
                return CLI_NO_ERROR;
            }
        }
    }
    else
    {
        /* unknown cmd_idx
         */
        return CLI_ERR_INTERNAL;
    }
#endif /* end if #if (SYS_CPNT_SYNCE==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_PSECheckStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
    BOOL_T pse_check_status;

    if(SWCTRL_PMGR_GetPSECheckStatus(&pse_check_status)==TRUE)
    {
        if(pse_check_status==TRUE)
        {
            CLI_LIB_PrintStr(NEWLINE"PSE Check Status: Enabled"NEWLINE);
        }
        else
        {
            CLI_LIB_PrintStr(NEWLINE"PSE Check Status: Disabled"NEWLINE);
        }
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Set_PSECheckStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_POWERSOURCECHECK:
            if(SWCTRL_PMGR_SetPSECheckStatus(FALSE)==FALSE)
                CLI_LIB_PrintStr(NEWLINE"Failed to disable PSE Check."NEWLINE);
            break;

        default:
            if(SWCTRL_PMGR_SetPSECheckStatus(TRUE)==FALSE)
                CLI_LIB_PrintStr(NEWLINE"Failed to enable PSE Check."NEWLINE);
            break;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_Show_PDPortStatus(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_HWCFG_SUPPORT_PD==TRUE)
    UI32_T lport, verify_unit, verify_port;
    UI32_T line_num = 0;
    UI32_T max_port_num;
    BOOL_T is_inherit;
    CLI_API_EthStatus_T verify_ret;
    SWCTRL_PortPD_T entry;
    char   tmp_buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

#if (SYS_HWCFG_SUPPORT_PD_MODE_DETECT == TRUE)
    PROCESS_MORE("Interface Power Source Status Operation Mode  \r\n");
    PROCESS_MORE("--------- ------------------- --------------  \r\n");
#else
    PROCESS_MORE("Interface Power Source Status   \r\n");
    PROCESS_MORE("--------- -------------------   \r\n");
#endif
    for (verify_unit=0; STKTPLG_POM_GetNextUnit(&verify_unit); )
    {
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

        for(verify_port = 1; verify_port <= max_port_num; verify_port++) /* port loop */
        {
            if(!SWCTRL_POM_UIUserPortExisting(verify_unit, verify_port))
            {
                continue;
            }

            is_inherit = FALSE;
            verify_ret = SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &lport, &is_inherit);
            if (verify_ret == SWCTRL_LPORT_UNKNOWN_PORT || verify_ret == SWCTRL_LPORT_TRUNK_PORT)
            {
                continue;
            }
            /* ethernet port
             */
            sprintf(tmp_buff, "Eth %lu/%2lu ", (unsigned long)verify_unit, (unsigned long)verify_port);
            strcat(buff, tmp_buff);
            memset(tmp_buff , 0, sizeof(tmp_buff));
            memset(&entry, 0, sizeof(SWCTRL_PortPD_T));
            entry.port_pd_ifindex = lport;
            SWCTRL_PMGR_GetPDPortStatus(&entry);

#if (SYS_HWCFG_SUPPORT_PD_MODE_DETECT == TRUE)
            switch(entry.port_pd_status)
            {
                case SWDRV_POWER_SOURCE_UP:
                    strcat(buff, " Up                  ");
                    break;
                case SWDRV_POWER_SOURCE_DOWN:
                    strcat(buff, " Down                ");
                    break;
                default:
                    strcat(buff, " None                ");
                    break;
            }

            switch(entry.port_pd_mode)
            {
                case SWDRV_POWERED_DEVICE_MODE_AF:
                    strcat(buff, "802.3af   \r\n");
                    break;
                case SWDRV_POWERED_DEVICE_MODE_AT:
                    strcat(buff, "802.3at   \r\n");
                    break;
                default:
                    strcat(buff, "None     \r\n");
                    break;
            }
#else
            switch(entry.port_pd_status)
            {
                case SWDRV_POWER_SOURCE_UP:
                    strcat(buff, " Up       \r\n");
                    break;
                case SWDRV_POWER_SOURCE_DOWN:
                    strcat(buff, " Down     \r\n");
                    break;
                default:
                    strcat(buff, " None     \r\n");
                    break;
            }
#endif
            PROCESS_MORE(buff);

        } /* end of port loop */
    } /* end of unit loop */

#endif /* end if #if (SYS_HWCFG_SUPPORT_PD==TRUE) */
    return CLI_NO_ERROR;
}
UI32_T CLI_API_Set_Alarm_Input_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if ((SYS_CPNT_ALARM_INPUT_DETECT == TRUE) && (SYS_CPNT_CLI_SHOW_ALARM_INPUT == TRUE))
    UI32_T unit, index, verify_unit, verify_index;
    SYS_MGR_SwAlarmEntry_T alarm;
    char alarm_input_buf[MAXSIZE_swAlarmInputName+1];

    memset(&alarm, 0, sizeof(SYS_MGR_SwAlarmEntry_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_ALARM_INPUT_NAME:
            {
                verify_unit = atoi((char *) arg[0]);
                verify_index = atoi((char *) arg[1]);
                strcpy(alarm.sw_alarm_input_name, arg[2]);

                if (!STKTPLG_POM_UnitExist(verify_unit))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Unit%lu does not exist\r\n", (unsigned long)verify_unit);
#endif
                    return CLI_NO_ERROR;
                }
                alarm.sw_alarm_unit_index = verify_unit;

                if(verify_index < 1 || verify_index > SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Alarm input %lu does not exist\r\n", (unsigned long)verify_index);
#endif
                }
                alarm.sw_alarm_input_index = verify_index;

                if(SYS_PMGR_SetSwAlarmInputName(&alarm)!=TRUE)
                {
                    CLI_LIB_PrintStr_2("Failed to set name of alarm input of unit %lu index %lu:\r\n", (unsigned long)verify_unit, (unsigned long)verify_index);
                }
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_ALARM_INPUT_NAME:
            if(arg[0] == NULL)
            {
                for (unit=0; STKTPLG_OM_GetNextUnit(&unit); )
                {
                    for(index=1; index <= SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT; index++)
                    {
                        memset(&alarm, 0, sizeof(SYS_MGR_SwAlarmEntry_T));

                        sprintf(alarm_input_buf, "ALARM_IN%ld", (long)index);
                        strcpy(alarm.sw_alarm_input_name, alarm_input_buf);
                        alarm.sw_alarm_unit_index = unit;
                        alarm.sw_alarm_input_index = index;
                        if(SYS_PMGR_SetSwAlarmInputName(&alarm)!=TRUE)
                        {
                            CLI_LIB_PrintStr_2("Failed to set name of alarm input of unit %lu index %lu:\r\n", (unsigned long)unit, (unsigned long)index);
                        }
                    }
                }
            }
            else
            {
                verify_unit = atoi((char *) arg[0]);
                verify_index = atoi((char *) arg[1]);

                if (!STKTPLG_POM_UnitExist(verify_unit))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Unit%lu does not exist\r\n", (unsigned long)verify_unit);
#endif
                    return CLI_NO_ERROR;
                }
                alarm.sw_alarm_unit_index = verify_unit;

                if(verify_index < 1 || verify_index > SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Alarm input %lu does not exist\r\n", (unsigned long)verify_index);
#endif
                }
                alarm.sw_alarm_input_index = verify_index;

                sprintf(alarm_input_buf, "ALARM_IN%ld", (unsigned long)verify_index);
                strcpy(alarm.sw_alarm_input_name, alarm_input_buf);

                if(SYS_PMGR_SetSwAlarmInputName(&alarm)!=TRUE)
                {
                    CLI_LIB_PrintStr_2("Failed to set name of alarm input of unit %lu index %lu:\r\n", (unsigned long)verify_unit, (unsigned long)verify_index);
                }
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_SetLocationLed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
    UI32_T my_unit_id=1;
    LEDDRV_TYPE_RET_T ret;
    BOOL_T location_led_on=FALSE;

    /* cli command syntax:
     *  location-led on
     *  location-led off
     *
     *  If arg[0][1] is 'n' or 'N', it means arg[0] is "on".
     */
    if(arg[0][1]=='n' || arg[0][1]=='N')
        location_led_on=TRUE;

    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    ret=LED_PMGR_SetLocationLED(my_unit_id, location_led_on);

    switch (ret)
    {
        case LEDDRV_TYPE_RET_ERR_HW_FAIL:
            CLI_LIB_PrintStr("A hardware error occurs when setting location led status.\r\n");
            break;
        case LEDDRV_TYPE_RET_ERR_NOT_SUPPORT:
            CLI_LIB_PrintStr("This device does not have location led.\r\n");
            break;
        case LEDDRV_TYPE_RET_OK:
            break;
        default:
            CLI_LIB_PrintStr("Unknown error\r\n");
            break;
    }
#endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_ShowLocationLed(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
    UI32_T my_unit_id=1;
    LEDDRV_TYPE_RET_T ret;
    BOOL_T location_led_on;

    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    ret=LED_PMGR_GetLocationLEDStatus(my_unit_id, &location_led_on);

    switch (ret)
    {
        case LEDDRV_TYPE_RET_ERR_HW_FAIL:
            CLI_LIB_PrintStr("A hardware error occurs when getting location led status.\r\n");
            return CLI_NO_ERROR;
            break;
        case LEDDRV_TYPE_RET_ERR_NOT_SUPPORT:
            CLI_LIB_PrintStr("This device does not have location led.\r\n");
            return CLI_NO_ERROR;
            break;
        case LEDDRV_TYPE_RET_OK:
            break;
        default:
            CLI_LIB_PrintStr("Unknown error\r\n");
            return CLI_NO_ERROR;
            break;
    }

    CLI_LIB_PrintStr("Location Led Status:");
    if(location_led_on==TRUE)
        CLI_LIB_PrintStr("On\r\n");
    else
        CLI_LIB_PrintStr("Off\r\n");


#endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_CHANGE_LOADER_BACKDOOR_PASSWORD == TRUE)
UI32_T CLI_API_Loader_Password(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#define CLI_LINUX_SHELL_OUTPUT_BUFFER_LEN 200
#define CLI_LINUX_SHELL_SCRIPT_STRING_LEN 200
#define CLI_DELETE_SYMBOLIC_LINK  system("rm -f /tmp/fw_setenv")

    char cmd[CLI_LINUX_SHELL_SCRIPT_STRING_LEN + 1]={0};
    FS_HW_Info_T *hwinfo;
    char  passwd[SYS_ADPT_MAX_PASSWORD_LEN + 1] = {0};
    char  retypepasswd[SYS_ADPT_MAX_PASSWORD_LEN + 1] = {0};
    int ret;
    char buff[CLI_LINUX_SHELL_OUTPUT_BUFFER_LEN + 1]={0};
    FILE* fp;


    /*check capability flag in hwinfo*/
    hwinfo = (FS_HW_Info_T *)malloc(sizeof(FS_HW_Info_T));
    if(hwinfo==NULL)
    {
        CLI_LIB_PrintStr("pointer is null!\n");
        return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    if(FS_ReadHardwareInfo(DUMMY_DRIVE, hwinfo)!=FS_RETURN_OK)
    {
        CLI_LIB_PrintStr("Get Hwinfo information fail.\r\n");
        free(hwinfo);
        return CLI_NO_ERROR;
    }

    if(!(hwinfo->capability & FS_HWINFO_CAPABILITY_CHANGE_LOADER_BACKDOOR_PASSWORD))
    {
        CLI_LIB_PrintStr("Operation is not allowed.\r\n");
        free(hwinfo);
        return CLI_NO_ERROR;
    }
    free(hwinfo);

    /*Get old password*/
    memset(cmd,0x00,CLI_LINUX_SHELL_SCRIPT_STRING_LEN);
    sprintf(cmd,"/usr/sbin/fw_printenv passwd|sed 's/passwd=//'");
    fp = popen(cmd, "r");
    if( fp == NULL )
    {
        CLI_LIB_PrintStr_1("ERROR, can not open file %s\r\n",cmd);
        return CLI_NO_ERROR;
    }

    while(fgets(buff,CLI_LINUX_SHELL_OUTPUT_BUFFER_LEN,fp) != NULL )
    {
    }
    pclose(fp);

    /*check envirnment crc and password is or not exist*/
    if(strlen(buff)==0)
    {
        CLI_LIB_PrintStr("Current loader password is missing.\r\n");
        return CLI_NO_ERROR;
    }

    buff[(UI32_T)strlen(buff)-1]='\0'; /* to trim the non-printable trailing char */

    /*check old password*/
    CLI_LIB_PrintStr("Old password: ");
    CLI_PARS_ReadLine(passwd, SYS_ADPT_MAX_PASSWORD_LEN + 1, TRUE, TRUE);
    CLI_LIB_PrintNullStr(1);
    CLI_LIB_Encrypt(passwd);

    if(strcmp(buff,passwd))
    {
        CLI_LIB_PrintStr("Incorrect password.\r\n");
        return CLI_NO_ERROR;
    }

    /*input new password*/
    memset(passwd,0x00,SYS_ADPT_MAX_PASSWORD_LEN+1);
    CLI_LIB_PrintStr("New password: ");
    CLI_PARS_ReadLine(passwd, SYS_ADPT_MAX_PASSWORD_LEN + 1, TRUE, TRUE);
    CLI_LIB_PrintNullStr(1);
    if(strlen(passwd)<6)
    {
        CLI_LIB_PrintStr("Illegal password.\r\n");
        return CLI_NO_ERROR;
    }

    /*retype new password */
    CLI_LIB_PrintStr("Retype new password: ");
    CLI_PARS_ReadLine(retypepasswd, SYS_ADPT_MAX_PASSWORD_LEN + 1, TRUE, TRUE);
    CLI_LIB_PrintNullStr(1);

    if(strcmp(passwd,retypepasswd))
    {
        CLI_LIB_PrintStr("Password does not match.\r\n");
        return CLI_NO_ERROR;
    }

    /*create a symbolic link for fw_setenv*/
     memset(cmd,0x00,CLI_LINUX_SHELL_SCRIPT_STRING_LEN);
     sprintf(cmd,"ln -s /usr/sbin/fw_printenv /tmp/fw_setenv");
     ret=system(cmd);
     if(ret != 0)
     {
         CLI_LIB_PrintStr_1("symbolic link error: %s\r\n",cmd);
         return CLI_NO_ERROR;
     }

    /*modify  passwd in the environment*/
    memset(cmd,0x00,CLI_LINUX_SHELL_SCRIPT_STRING_LEN);
    CLI_LIB_Encrypt(passwd);
    sprintf(cmd,"/tmp/fw_setenv passwd %s 2>/dev/null 1>/dev/null",passwd);
    ret=system(cmd);
    if(ret != 0)
    {
        CLI_LIB_PrintStr_1("config password error: %s\r\n",cmd);
        CLI_DELETE_SYMBOLIC_LINK;
        return CLI_NO_ERROR;
    }

    CLI_DELETE_SYMBOLIC_LINK;
    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_ONIE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ONIE_BOOT_MODE_SET==TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_ONIE_INSTALL:
            if (SYSFUN_ExecuteSystemShell("/usr/sbin/fw_setenv onie_boot_reason install")==SYSFUN_OK)
                CLI_LIB_PrintStr("Set ONIE mode as INSTALL mode.\r\n");
            else
                CLI_LIB_PrintStr("Failed to set ONIE mode.\r\n");
            break;
        case PRIVILEGE_EXEC_CMD_W2_ONIE_RESCUE:
            if (SYSFUN_ExecuteSystemShell("/usr/sbin/fw_setenv onie_boot_reason rescue")==SYSFUN_OK)
                CLI_LIB_PrintStr("Set ONIE mode as RESCUE mode.\r\n");
            else
                CLI_LIB_PrintStr("Failed to set ONIE mode.\r\n");
            break;
        case PRIVILEGE_EXEC_CMD_W2_ONIE_UPDATE:
            if (SYSFUN_ExecuteSystemShell("/usr/sbin/fw_setenv onie_boot_reason update")==SYSFUN_OK)
                CLI_LIB_PrintStr("Set ONIE mode as UPDATE mode.\r\n");
            else
                CLI_LIB_PrintStr("Failed to set ONIE mode.\r\n");
            break;
        default:
                CLI_LIB_PrintStr("Invalid command.\r\n");
            break;
    }
#endif /* end of #if (SYS_CPNT_ONIE_SUPPORT==TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Hardware_Profile_Portmode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    STKTPLG_TYPE_HwPortMode_T hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED;
    int cfg_port_count = 0;

    /* hardware profile portmode <port_list> {1x40g | 4x10g | reset}
     */

    if (arg[2])
    {
        switch (arg[2][0])
        {
            case '1': /* 1x40g */
                if (arg[2][2] == '1')
                    hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_1x100G;
                else
                    hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_1x40G;
                break;
            case '4': /* 4x10g */
                if (arg[2][2] == '2')
                    hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_4x25G;
                else
                    hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_4x10G;
                break;
            case 'r': /* reset */
            case 'R':
                hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED;
                break;
            default:
                return CLI_ERR_INTERNAL;
        }
    }

    if (arg[0])
    {
        switch (arg[0][0])
        {
            case 'e': /* ethernet */
            case 'E':
            {
                UI32_T unit;
                UI32_T port;
                UI32_T lower_val;
                UI32_T upper_val;
                UI32_T err_idx;
                UI32_T lport;
                char   *op_ptr;
                char   Token[CLI_DEF_MAX_BUFSIZE];
                char   delemiters[2] = {0};

                unit = atoi(arg[1]);
                op_ptr = strchr(arg[1], '/') + 1;
                delemiters[0] = ',';

                do
                {
                    memset(Token, 0, sizeof(Token));
                    op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                    if (!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                    {
                        break;
                    }

                    for (port = lower_val; port <= upper_val; port++)
                    {
                        if (!STKTPLG_PMGR_SetCfgHwPortMode(unit, port, hw_port_mode))
                        {
                            CLI_LIB_PrintStr_2("Failed to set port mode on Ethernet %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
                            continue;
                        }

                        cfg_port_count++;
                    }
                }
                while(op_ptr != 0 && !isspace(*op_ptr));
                break;
            } /* end of case (ethernet) */

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    if (cfg_port_count > 0)
    {
        CLI_LIB_PrintStr("Warning: This command will not take effect until reload.\r\n");
    }
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Hardware_Profile_Portmode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    static char *hw_port_mode_str[STKTPLG_TYPE_HW_PORT_MODE_MAX] = {
        [STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED] = "-",
        [STKTPLG_TYPE_HW_PORT_MODE_4x10G] = "4x10g",
        [STKTPLG_TYPE_HW_PORT_MODE_1x40G] = "1x40g",
        [STKTPLG_TYPE_HW_PORT_MODE_4x25G] = "4x25g",
        [STKTPLG_TYPE_HW_PORT_MODE_1x100G] = "1x100g",
    };
    STKTPLG_TYPE_HwPortModeEntry_T hw_port_mode_entry;
    char *str_ports;
    char str_ports_100g[16];
    char str_ports_40g[16];
    char str_ports_25g[16];
    char str_ports_10g[16];
    int str_ports_len = 12;
    int len_100g = 0;
    int len_40g = 0;
    int len_25g = 0;
    int len_10g = 0;
    int i;

    memset(&hw_port_mode_entry, 0, sizeof(hw_port_mode_entry));

    while (STKTPLG_OM_GetHwNextPortModeEntry(&hw_port_mode_entry))
    {
        for (i = 0; i < hw_port_mode_entry.port_info_count; i++)
        {
            switch (hw_port_mode_entry.port_info[i].port_type)
            {
                case VAL_portType_hundredGBaseQSFP:
                    len_100g = str_ports_len;
                    break;
                case VAL_portType_fortyGBaseQSFP:
                    len_40g = str_ports_len;
                    break;
                case VAL_portType_twentyFiveGBaseSFP:
                    len_25g = str_ports_len;
                    break;
                case VAL_portType_tenGBaseSFP:
                    len_10g = str_ports_len;
                    break;
            }
        }
    }

    PROCESS_MORE_F("%-*.*s%-*.*s%-*.*s%-*.*sConfig  Oper\r\n",
        len_100g, len_100g, "100G",
        len_40g, len_40g, "40G",
        len_25g, len_25g, "25G",
        len_10g, len_10g, "10G");
    PROCESS_MORE_F("%-*.*s%-*.*s%-*.*s%-*.*sMode    Mode\r\n",
        len_100g, len_100g, "Interfaces",
        len_40g, len_40g, "Interfaces",
        len_25g, len_25g, "Interfaces",
        len_10g, len_10g, "Interfaces");
    PROCESS_MORE_F("%-*.*s%-*.*s%-*.*s%-*.*s------  ------\r\n",
        len_100g, len_100g, "----------",
        len_40g, len_40g, "----------",
        len_25g, len_25g, "----------",
        len_10g, len_10g, "----------");

    memset(&hw_port_mode_entry, 0, sizeof(hw_port_mode_entry));

    while (STKTPLG_OM_GetHwNextPortModeEntry(&hw_port_mode_entry))
    {
        str_ports_100g[0] = str_ports_40g[0] = str_ports_10g[0] = 0;

        for (i = 0; i < hw_port_mode_entry.port_info_count; i++)
        {
            switch (hw_port_mode_entry.port_info[i].port_type)
            {
                case VAL_portType_hundredGBaseQSFP:
                    str_ports = str_ports_100g;
                    break;
                case VAL_portType_fortyGBaseQSFP:
                    str_ports = str_ports_40g;
                    break;
                case VAL_portType_twentyFiveGBaseSFP:
                    str_ports = str_ports_25g;
                    break;
                case VAL_portType_tenGBaseSFP:
                    str_ports = str_ports_10g;
                    break;
                default:
                    str_ports = NULL;
            }

            if (str_ports)
            {
                if (hw_port_mode_entry.port_info[i].port_num > 1)
                {
                    sprintf(str_ports, "%lu/%lu-%lu",
                        (unsigned long)hw_port_mode_entry.unit,
                        (unsigned long)hw_port_mode_entry.port_info[i].port_start,
                        (unsigned long)hw_port_mode_entry.port_info[i].port_start + hw_port_mode_entry.port_info[i].port_num - 1);
                }
                else
                {
                    sprintf(str_ports, "%lu/%lu",
                        (unsigned long)hw_port_mode_entry.unit,
                        (unsigned long)hw_port_mode_entry.port_info[i].port_start);
                }
            }
        } /* end of for (i) */

        PROCESS_MORE_F("%-*.*s%-*.*s%-*.*s%-*.*s%-6s  %-6s\r\n",
            len_100g, len_100g, str_ports_100g,
            len_40g, len_40g, str_ports_40g,
            len_25g, len_25g, str_ports_25g,
            len_10g, len_10g, str_ports_10g,
            hw_port_mode_str[hw_port_mode_entry.cfg_hw_port_mode],
            hw_port_mode_str[hw_port_mode_entry.oper_hw_port_mode]);
    } /* end of while (hw_port_mode_entry) */
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

    return CLI_NO_ERROR;
}

