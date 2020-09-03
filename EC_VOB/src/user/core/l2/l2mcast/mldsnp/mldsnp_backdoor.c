/* MODULE NAME: mldsnp_backdoor.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/


/* INCLUDE FILE DECLARATIONS
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mldsnp_backdoor.h"
#ifndef UNIT_TEST
#include "l_inet.h"
#include "mldsnp_timer.h"
#include "mldsnp_om.h"
#include "mldsnp_querier.h"
#include "mldsnp_engine.h"
#include "mldsnp_unknown.h"
#include "sys_bld.h"
#include "sys_time.h"
/* NAMING CONSTANT DECLARATIONS
*/
#define MLDSNP_BACKDOOR_MAXLINE 255

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
static UI32_T mldsnp_backdoor_debug_flag_g = MLDSNP_BD_FLAG_NONE;
static  BOOL_T  stop_flag = TRUE;

#define MLDSNP_BACKDOOR_FUNC_MAPP_SIZE 11
FuncName_T function_mapping[MLDSNP_BACKDOOR_FUNC_MAPP_SIZE] =
{
    {MLDSNP_QUERIER_MrdSolicitationTimeout,   "QUERIER_MrdSolicitationTimeout"}, /*0*/
    {MLDSNP_QUERIER_GeneralQueryTimeOut,      "QUERIER_GeneralQueryTimeOut"},/*1*/
    {MLDSNP_QUERIER_SpecificQueryTimeOut,     "QUERIER_SpecificQueryTimeOut"},/*2*/
    {MLDSNP_QUERIER_OtherQuerierPrentTimeout, "QUERIER_OtherQuerierPrentTimeout"},/*3*/
    {MLDSNP_QUERIER_QuerierStartTimeout,      "QUERIER_QuerierStartTimeout"},/*4*/
    {MLDSNP_QUERIER_RouterPortTimeout,        "QUERIER_RouterPortTimeout"},/*5*/
    {MLDSNP_UNKNOWN_PortRxUnknwonDataTimeout, "UNKNOWN_PortRxUnknwonDataTimeout"},/*6*/
    {MLDSNP_ENGINE_FilterTimerTimeout,        "ENGINE_FilterTimerTimeout"},/*7*/
    {MLDSNP_ENGINE_SourceTimerTimeout,        "ENGINE_SourceTimerTimeout"},/*8*/
    {MLDSNP_ENGINE_V1HostPresentTimeout,      "ENGINE_V1HostPresentTimeout"},/*9*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {MLDSNP_ENGINE_UnsolicitTimeout,          "MLDSNP_ENGINE_UnsolicitTimeout"}/*10*/
#endif
};

/* LOCAL SUBPROGRAM BODIES
*/
#if 0
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_GetNum
 *-------------------------------------------------------------------------
 * PURPOSE : the the input number from user
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  -
 *           FALSE -
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static void MLDSNP_BACKDOOR_GetNum(
    char string[])
{
    I16_T character;
    I16_T count = 0;

    while (TRUE)
    {
        character = getchar();

        if (character == EOF
                || character == '\n'
                || character == 0x0d)    /*newline*/
        {
            *(string + count) = 0;
            break;
        }

        if (character >= 0x30
                &&  character <= 0x39)
        {
            if (count + 1 <= MLDSNP_BACKDOOR_MAXLINE)
            {
                *(string + count) = (UI8_T)character;
                printf("%c", character);
                count++;
            }
        }
        else if (character == 0x08
                 && count > 0) /*backspace*/
        {
            count --;
            printf("\x08\x20\x08"); /*perform backspace on screen*/
        }
    }
    return;
}/*End of MLDSNP_BACKDOOR_GetNum*/
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowAllVlanInfo
 *-------------------------------------------------------------------------
 * PURPOSE : Show all vlan info
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static void MLDSNP_BACKDOOR_ShowAllVlanInfo()
{
    MLDSNP_OM_VlanInfo_T       vlan_info;
    MLDSNP_OM_RouterPortInfo_T r_port_info;
    UI32_T time_left = 0;
    UI16_T vid = 0;

    MLDSNP_BD_MSG("\r\n");

    while (TRUE == MLDSNP_OM_GetNextVlanInfo(&vid, &vlan_info))
    {
        MLDSNP_BD_MSG("%s\r\n", "***************************");

        MLDSNP_BD_MSG("%-32s : %d\r\n", "vid", vlan_info.vid);
        MLDSNP_BD_MSG("%-32s : %d\r\n", "querier_stauts", vlan_info.querier_runing_status);
        MLDSNP_BD_MSG("%-32s : %d\r\n", "immediate leave status", vlan_info.immediate_leave_status);
        MLDSNP_TIMER_QueryTimer(vlan_info.other_querier_present_timer_p, &time_left);
        MLDSNP_BD_MSG("%-32s : %ld\r\n", "other querier present time left", time_left);
        MLDSNP_BD_MSG("%-32s : %d\r\n", "query_interval_oper", vlan_info.query_oper_interval);
        MLDSNP_BD_MSG("%-32s : %d\r\n", "robust_value_oper",  vlan_info.robust_oper_value);

        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
        {
            MLDSNP_TIMER_QueryTimer(r_port_info.router_timer_p, &time_left);
            MLDSNP_BD_MSG("%-14s : %-8s %3d, time left %ld\r\n", "router port",
                          (MLDSNP_TYPE_JOIN_STATIC == r_port_info.attribute ? "Static" : "Dynamic"), r_port_info.port_no, time_left);
        }
        MLDSNP_BD_MSG("\r\n");
    }

    return ;
}/*End of MLDSNP_BACKDOOR_ShowAllVlanInfo*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowAllGroupInfo
 *-------------------------------------------------------------------------
 * PURPOSE :  show all group info
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static void MLDSNP_BACKDOOR_ShowAllGroupInfo()
{
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_OM_PortInfo_T   port_info;
    UI32_T time_left = 0;
    UI16_T vid       = 0;
    UI8_T gip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0}, sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0};
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1]={0};

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        MLDSNP_BD_MSG("\r\n%s\r\n", "***************************");
        MLDSNP_BD_MSG("vid   : %d\r\n", vid);
        L_INET_Ntop(L_INET_AF_INET6, gip_a, ipv6_addr_str, sizeof(ipv6_addr_str));
        MLDSNP_BD_MSG("group : %s \r\n", ipv6_addr_str);
        ipv6_addr_str[0] = '\0';
        L_INET_Ntop(L_INET_AF_INET6, sip_a, ipv6_addr_str, sizeof(ipv6_addr_str));
        MLDSNP_BD_MSG("source: %s \r\n", ipv6_addr_str);

        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&entry_info.register_port_list, &port_info))
        {
            MLDSNP_TIMER_QueryTimer(port_info.src_timer_p, &time_left);
            MLDSNP_BD_MSG("%-23s : %3d time_left %ld\r\n", "Port", port_info.port_no, time_left);
            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
            {
                MLDSNP_BD_MSG("  %-21s : %s\r\n", "Join Type", "Static");
            }
            else if (MLDSNP_TYPE_JOIN_DYNAMIC == port_info.join_type)
            {
                MLDSNP_BD_MSG("  %-21s : %s\r\n", "Join Type", "Dynamic");
            }
            else if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type)
            {
                MLDSNP_BD_MSG("  %-21s : %s\r\n", "Join Type", "Unknown");
            }
            else
            {
                MLDSNP_BD_MSG("  %-21s : %s\r\n", "Join Type", "Unknown Type");
            }

            MLDSNP_BD_MSG("  %-21s : %s\r\n", "List Type", (MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type ? "Exclude" : "Include|Request"));
            MLDSNP_BD_MSG("  %-21s : %d\r\n", "Specify Qury Count", port_info.specific_query_count);
            MLDSNP_BD_MSG("  %-21s : %lu\r\n", "Retransmit Time", port_info.rexmt_time_stamp);
            MLDSNP_BD_MSG("  %-21s : %lu\r\n", "Register Time", port_info.register_time);
            MLDSNP_TIMER_QueryTimer(port_info.ver1_host_present_timer_p, &time_left);
            MLDSNP_BD_MSG("  %-21s : %lu\r\n", "V1 Present Time Left", time_left);
        }

        if (TRUE == MLDSNP_BACKDOOR_StopCheck())
        {
            break;
        }
    }


}/*End of MLDSNP_BACKDOOR_ShowAllGroupInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowConfig
 *-------------------------------------------------------------------------
 * PURPOSE : print the global configuration
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static  void MLDSNP_BACKDOOR_ShowConfig()
{
    MLDSNP_OM_Cfg_T cfg;

    MLDSNP_OM_GetGlobalConf(&cfg);

    MLDSNP_BD_MSG("\r\n");
    MLDSNP_BD_MSG("%-30s : %s\r\n", "Status", (cfg.mldsnp_status == MLDSNP_TYPE_MLDSNP_ENABLED ? "Enabled" : "Disable"));
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Version", cfg.version);
    MLDSNP_BD_MSG("%-30s : %s\r\n", "Querier Status", cfg.querier_status == MLDSNP_TYPE_QUERIER_ENABLED ? "Enabled" : "Disable");
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    MLDSNP_BD_MSG("%-30s : %s\r\n", "Unknown Behavior", cfg.unknown_flood_behavior == MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD ? "Flood" : "To Router Port");
#endif
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Robustness value", cfg.robust_value);
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Query Interval", cfg.query_interval);
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Query Response Interval", cfg.query_response_interval);
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Listener Interval", cfg.listener_interval);
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Last Listener Query Interval", cfg.last_listner_query_interval);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Unsolicited Report Interval", cfg.unsolicited_report_interval);
#endif
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Querier Start Send Countl", cfg.querier_start_sent_count);
    MLDSNP_BD_MSG("%-30s : %d\r\n", "Router Expire Time", cfg.router_exp_time);
    {
        UI32_T time_left = 0;
        MLDSNP_Timer_T * timer_p;
        MLDSNP_OM_GetQuerierTimer(&timer_p);
        MLDSNP_TIMER_QueryTimer(timer_p, &time_left);
        MLDSNP_BD_MSG("%-30s : %ld", "Query Time", time_left);
    }
    return;
}/*End of MLDSNP_BACKDOOR_ShowAllGroupInfo*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowStaticJoinEntry
 *-------------------------------------------------------------------------
 * PURPOSE : print static join configured entry
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static  void MLDSNP_BACKDOOR_ShowStaticJoinEntry()
{
    MLDSNP_TYPE_RecordType_T rec_type;
    UI16_T next_id = 0, vid = 0, lport = 0;
    UI8_T gip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0}, sip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
    char   ipv6_addr_str[L_INET_MAX_IP6ADDR_STR_LEN+1]={0};

    while (TRUE == MLDSNP_OM_GetNextStaticPortJoinGroup(&next_id, &vid, gip_a, sip_a, &lport, &rec_type))
    {
        MLDSNP_BD_MSG("\r\n");
        MLDSNP_BD_MSG("%-10s : %d\r\n", "vid", vid);
        MLDSNP_BD_MSG("%-10s : %d\r\n", "lport", lport);
        MLDSNP_BD_MSG("%-10s : %d\r\n", "rec_type", rec_type);
        L_INET_Ntop(L_INET_AF_INET6, gip_a, ipv6_addr_str, sizeof(ipv6_addr_str));
        MLDSNP_BD_MSG("%-10s : %s\r\n", "group", ipv6_addr_str);
        L_INET_Ntop(L_INET_AF_INET6, sip_a, ipv6_addr_str, sizeof(ipv6_addr_str));
        MLDSNP_BD_MSG("%-10s : %s\r\n", "src", ipv6_addr_str);
    }
    return;
}/*End of */

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowProfileContent
 *-------------------------------------------------------------------------
 * PURPOSE : print static join configured entry
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static  void MLDSNP_BACKDOOR_ShowProfileContent()
{
    UI32_T pid = 0, i;
    UI32_T throttling_number;
    MLDSNP_OM_ProfileInfo_T *profile_p;

    MLDSNP_BD_MSG("\r\n");
    MLDSNP_BD_MSG("Total Group Ranges %lu\r\n", MLDSNP_OM_GetTotalProfileGroupRangeCount());

    while (NULL != (profile_p = MLDSNP_OM_GetNextMldProfile(&pid)))
    {
        MLDSNP_BD_MSG("Profile ID         : %lu\r\n", profile_p->pid);
        MLDSNP_BD_MSG("Total Group Ranges : %lu\r\n", profile_p->total_group_ranges);
    }

    MLDSNP_BD_MSG("Throttle number:\r\n");
    for (i = 1 ; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
    {
        MLDSNP_OM_GetPortDynamicGroupCount(i, &throttling_number);
        MLDSNP_BD_MSG("Port %lu cur %lu\r\n", i, throttling_number);
    }

    return;
}/*End of */
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_ShowCmd
 *-------------------------------------------------------------------------
 * PURPOSE : show the debug command and flag status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static void MLDSNP_BACKDOOR_ShowCmd()
{
    MLDSNP_BD_MSG("\r\n");
    MLDSNP_BD_MSG("%-30s\r\n", "01: Clear All Debug Flag.");
    MLDSNP_BD_MSG("%-30s: %s\r\n", "02: Set UI Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_UI) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "03: Set Timer Stop Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_TIMER) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "04: Set Tx Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_TX) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "05: Set Rx Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_RX) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "06: Set IPC Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_IPC) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "07: Set Trace Code Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_TRACE) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "08: Set CallBack Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_CALLBACK) ? "On" : "Off"));
    MLDSNP_BD_MSG("%-30s: %s\r\n", "09: Set Err Flag", (MLDSNP_BACKDOOR_GetDebug(MLDSNP_BD_FLAG_ERR) ? "On" : "Off"));
    MLDSNP_BD_MSG("\r\n");
    MLDSNP_BD_MSG("%-30s\r\n", "11: Show Timer List");
    MLDSNP_BD_MSG("%-30s\r\n", "12: Show All Vlan Info");
    MLDSNP_BD_MSG("%-30s\r\n", "13: Show All Group Info");
    MLDSNP_BD_MSG("%-30s\r\n", "14: Show Global Configuration");
    MLDSNP_BD_MSG("%-30s\r\n", "15: Show Static Join Entry");
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    MLDSNP_BD_MSG("%-30s\r\n", "16: Profile Content");
#endif
    MLDSNP_BD_MSG("%-30s\r\n", "99: Exit");
    MLDSNP_BD_MSG("%-30s\r\n", "Enter Your Choice:");
    stop_flag = TRUE;
    return;
}/*End of MLDSNP_BACKDOOR_ShowCmd*/


static void MLDSNP_BACKDOOR_GetFunctionName(mldsnp_bd_func_t call_back_func, char *name_p)
{
    UI16_T i;

    memset(name_p, 0, MLDSNP_BACKDOOR_MAXLINE);

    for (i = 0; i < MLDSNP_BACKDOOR_FUNC_MAPP_SIZE; i++)
    {
        if (function_mapping[i].func_callback_p == call_back_func)
        {
            strcpy(name_p, function_mapping[i].func_name_p);
            return;
        }
    }

    strcpy(name_p, "can't find");
    return;
}

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_RestAll
 *-------------------------------------------------------------------------
 * PURPOSE : reset all debug flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_RestAll()
{
    mldsnp_backdoor_debug_flag_g = MLDSNP_BD_FLAG_NONE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_SetDebugFlag
 *-------------------------------------------------------------------------
 * PURPOSE :  set the debug flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  -
 *           FALSE -
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_SetDebugFlag(UI32_T flag)
{
    mldsnp_backdoor_debug_flag_g   ^= flag;

    if (MLDSNP_BD_FLAG_TIMER == flag)
        MLDSNP_BD_MSG("time now=%ld", SYS_TIME_GetSystemTicksBy10ms());

    return;
}/* End of MLDSNP_BACKDOOR_SetDebugFlag */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_GetDebug
 *-------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T  MLDSNP_BACKDOOR_GetDebug(UI32_T flag)
{
    return ((mldsnp_backdoor_debug_flag_g & flag) != 0);
} /* End of MLDSNP_BACKDOOR_GetDebug */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : print the main menu of the mldsnp debug backdoor
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_MainMenu()
{
    I8_T ch;
    char cmd_buf[MLDSNP_BACKDOOR_MAXLINE];

    while (TRUE)
    {
        ch = 0;
        cmd_buf[0] = 0;

        MLDSNP_BACKDOOR_ShowCmd();
        /*MLDSNP_BACKDOOR_GetNum(cmd_buf);*/
        BACKDOOR_MGR_RequestKeyIn(cmd_buf, MLDSNP_BACKDOOR_MAXLINE);
        ch = atoi(cmd_buf);

        switch (ch)
        {
            case 1: /**/
                MLDSNP_BACKDOOR_RestAll();
                break;

            case 2: /*MLDSNP_BD_FLAG_UI*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_UI);
                break;

            case 3: /*MLDSNP_BD_FLAG_TIMER*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_TIMER);
                break;

            case 4: /*MLDSNP_BD_FLAG_TX*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_TX);
                break;

            case 5: /*MLDSNP_BD_FLAG_RX*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_RX);
                break;

            case 6: /*MLDSNP_BD_FLAG_IPC*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_IPC);
                break;

            case 7: /*MLDSNP_BD_FLAG_TRACE*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_TRACE);
                break;

            case 8: /*MLDSNP_BD_FLAG_CALLBACK*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_CALLBACK);
                break;

            case 9:/* MLDSNP_BD_FLAG_ERR*/
                MLDSNP_BACKDOOR_SetDebugFlag(MLDSNP_BD_FLAG_ERR);
                break;

            case 11:
                MLDSNP_BACKDOOR_ShowTimerList();
                break;

            case 12:
                MLDSNP_BACKDOOR_ShowAllVlanInfo();
                break;

            case 13:
                MLDSNP_BACKDOOR_ShowAllGroupInfo();
                break;

            case 14:
                MLDSNP_BACKDOOR_ShowConfig();
                break;

            case 15:
                MLDSNP_BACKDOOR_ShowStaticJoinEntry();
                break;
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            case 16:
                MLDSNP_BACKDOOR_ShowProfileContent();
                break;
#endif
            case 99:
            case 0:
                return;

            default:
                break;
        }
    }
    return;
}/*End of MLDSNP_BACKDOOR_MainMenu*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : print the packet content
 * INPUT   : flag - debug flag
 *               *pdu_p - the packer content
 *               len - the pdu len to print
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_PrintPdu(
    MLDSNP_BD_DebugFlag_T flag,
    UI8_T *pdu_p,
    UI16_T len)
{
#define DISPLAY_CHAR 10 /* we will print DISPLAY_CHAR*2 then change line*/
    UI16_T loop_times = len / DISPLAY_CHAR; /*each line print 10 chara*/
    UI16_T left_times = len % DISPLAY_CHAR;
    UI16_T i, shift = 0;

    if (!(mldsnp_backdoor_debug_flag_g &  flag))
    {
        return;
    }

    MLDSNP_BD_MSG("\r\n");

    for (i = 0; i < loop_times; i++)
    {
        if (0 == i % 2 && 0 != i)
            MLDSNP_BD_MSG("\r\n");

        shift = i * DISPLAY_CHAR;
        MLDSNP_BD_MSG("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x  ",
                      pdu_p[0+shift], pdu_p[1+shift], pdu_p[2+shift], pdu_p[3+shift], pdu_p[4+shift],
                      pdu_p[5+shift], pdu_p[6+shift], pdu_p[7+shift], pdu_p[8+shift], pdu_p[9+shift]);
    }
    MLDSNP_BD_MSG("\r\n");
    shift += DISPLAY_CHAR; /*move to the printed content tail*/

    for (i = 0; i < left_times; i++)
        MLDSNP_BD_MSG("%02x", pdu_p[shift+i]);

    MLDSNP_BD_MSG("\r\n");
    return;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_BACKDOOR_ShowTimerList
*------------------------------------------------------------------------------
* Purpose: This function is for debug to show all current timer content
* INPUT  : *timer_p - the new timer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is for backdoor
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_BACKDOOR_ShowTimerList()
{
    MLDSNP_TimerHead_T *timer_head_p = NULL;
    MLDSNP_Timer_T     *timer_p      = NULL;
    UI32_T time_left                 = 0;
    UI32_T cuum = 0;
    char name_a[MLDSNP_BACKDOOR_MAXLINE];

    timer_head_p = MLDSNP_TIMER_GetTimerHeadPtr();

    for (timer_p = timer_head_p->fwd_p; timer_p != NULL; timer_p = timer_p->fwd_p)
    {
        cuum += timer_p->delta_time;
    }

    if (cuum != timer_head_p->cumtime)
        MLDSNP_BD_MSG("cumtime time is wrong, timer list have problem");

    timer_p = timer_head_p->fwd_p;

    while (NULL != timer_p)
    {
        MLDSNP_TIMER_QueryTimer(timer_p, &time_left);
        MLDSNP_BD_MSG("%s\r\n", "***********************************************");
        MLDSNP_BD_MSG("vid        : %d;"
                      "lport      : %d\r\n"
                      "type       : %d;"
                      "timer left : %ld\r\n",
                      timer_p->param.vid, timer_p->param.lport, timer_p->type, time_left);
        MLDSNP_BACKDOOR_GetFunctionName(timer_p->func_p, name_a);
        MLDSNP_BD_MSG("func name  : %s, %p\r\n", name_a, timer_p->func_p);

        MLDSNP_BD_SHOW_GROUP_MSG(timer_p->param.gip_a);
        MLDSNP_BD_SHOW_SRC_MSG(timer_p->param.sip_list_a, timer_p->param.num_of_src);

        timer_p = timer_p->fwd_p;

        if (TRUE == MLDSNP_BACKDOOR_StopCheck())
        {
            break;
        }
    }

    return TRUE;
}/*End of MLDSNP_BACKDOOR_ShowTimerList*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_StopCheck
 *-------------------------------------------------------------------------
 * PURPOSE : This function check go print next record or not from input
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - stop to get next record
 *           FALSE - go to get next record
 * NOTE    : the space means get next record.
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_BACKDOOR_StopCheck()
{
    I8_T get_in_char;

    if (stop_flag == FALSE)
        return FALSE;

    MLDSNP_BD_MSG(" ");
    MLDSNP_BD_MSG("---Press Space: Next; S: don't Stop; Other: Exit\r\n");

    get_in_char = getchar();

    if (get_in_char == 's' || get_in_char == 'S')
    {
        stop_flag = FALSE;
        return FALSE;
    }


    if (get_in_char == ' ')
    {
        return FALSE;
    }

    return TRUE;
}
#else
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_GetDebug
 *-------------------------------------------------------------------------
 * PURPOSE : get the debug flag is on or off
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - this debug flag is on
 *           FALSE - this debug flag is off
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T  MLDSNP_BACKDOOR_GetDebug(UI32_T flag)
{
    return FALSE;
} /* End of MLDSNP_BACKDOOR_GetDebug */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_BACKDOOR_MainMenu
 *-------------------------------------------------------------------------
 * PURPOSE : print the packet content
 * INPUT   : flag - debug flag
 *               *pdu_p - the packer content
 *               len - the pdu len to print
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_BACKDOOR_PrintPdu(
    MLDSNP_BD_DebugFlag_T flag,
    UI8_T *pdu_p,
    UI16_T len)
{
#define DISPLAY_CHAR 10 /* we will print DISPLAY_CHAR*2 then change line*/
    UI16_T loop_times = len / DISPLAY_CHAR; /*each line print 10 chara*/
    UI16_T left_times = len % DISPLAY_CHAR;
    UI16_T i, shift = 0;

    MLDSNP_BD_MSG("\r\n");

    for (i = 0; i < loop_times; i++)
    {
        if (0 == i % 2 && 0 != i)
            MLDSNP_BD_MSG("\r\n");

        shift = i * DISPLAY_CHAR;
        MLDSNP_BD_MSG("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x  ",
                      pdu_p[0+shift], pdu_p[1+shift], pdu_p[2+shift], pdu_p[3+shift], pdu_p[4+shift],
                      pdu_p[5+shift], pdu_p[6+shift], pdu_p[7+shift], pdu_p[8+shift], pdu_p[9+shift]);
    }
    MLDSNP_BD_MSG("\r\n");
    shift += DISPLAY_CHAR; /*move to the printed content tail*/

    for (i = 0; i < left_times; i++)
        MLDSNP_BD_MSG("%02x", pdu_p[shift+i]);

    MLDSNP_BD_MSG("\r\n");
    return;
}
#endif /*#ifndef UNIT_TEST*/

