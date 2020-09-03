/*-----------------------------------------------------------------------------
 * Module Name: lldp_backdoor.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the CFM backdoor
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    11/17/2005 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sysfun.h"
#include "cfm_backdoor.h"
#include "cfm_om.h"
#include "cfm_type.h"
#include "cfm_engine.h"
#include "swctrl.h"
#include "sysfun.h"

#include "backdoor_mgr.h"
#if (SYS_CPNT_CFM == TRUE)
#define MAXLINE 255

static void CFM_BACKDOOR_Engine();
static void CFM_BACKDOOR_ShowCmd();
static void CFM_BACKDOOR_GetNum(UI8_T string[]);
static void CFM_BACKDOOR_Engine();
static void CFM_BACKDOOR_ShowGlobalConfig();
static void CFM_BACKDOOR_ShowPortConfig();
static void CFM_BACKDOOR_ShowMdInfo();
static void CFM_BACKDOOR_ShowMaInfo();
static void CFM_BACKDOOR_ShowMpInfo();
static void CFM_BACKDOOR_ShowRemoteMepInfo();
static void CFM_BACKDOOR_ShowLinktraceCache();
static void CFM_BACKDOOR_ShowPendingLTRList();
static void CFM_BACKDOOR_ShowEorrorList();
static void CFM_BACKDOOR_DelayMeasure(void);

static void CFM_BACKDOOR_ShowChassisID(UI32_T type, UI8_T id[], UI32_T length);
static BOOL_T CFM_BACKDOOR_STOP ();
static void CFM_BACKDOOR_RestAll();

static  UI32_T          CFM_BACKDOOR_DebugFlag = CFM_BACKDOOR_DEBUG_FLAG_NONE;
extern CFM_OM_MEP_T mep_tmp3_g;
extern CFM_OM_LTR_T ltr_tmp_g;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - CFM_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

void CFM_BACKDOOR_Main(void)
{
    BACKDOOR_MGR_Printf("CFM Backdoor!!");
    CFM_BACKDOOR_Engine();
    return;
}

static void CFM_BACKDOOR_ShowCmd()
{
    BACKDOOR_MGR_Printf("\r\n 01: Show Global Configuration");
    BACKDOOR_MGR_Printf("\r\n 02: Show Each Logical Port Configuration");
    BACKDOOR_MGR_Printf("\r\n 03: Show MD info");
    BACKDOOR_MGR_Printf("\r\n 04: Show MA info");
    BACKDOOR_MGR_Printf("\r\n 05: Show MP info");
    BACKDOOR_MGR_Printf("\r\n 06: Show Remote Mep");
    BACKDOOR_MGR_Printf("\r\n 07: Show Link Trace Cache");
    BACKDOOR_MGR_Printf("\r\n 08: Show LTR Pending List");
    BACKDOOR_MGR_Printf("\r\n 09: Show Error List");
    BACKDOOR_MGR_Printf("\r\n \r\n");
    BACKDOOR_MGR_Printf("\r\n 10: Reset All");
    BACKDOOR_MGR_Printf("\r\n 11: Show Packet Receive Flow   : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 12: Show Packet Transmit Flow  : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 13: Show Timer Time out        : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TIMER)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 14: Show State Machine State   : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 15: Show Trap                  : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TRAP)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 16: Show UI                    : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_UI)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 17: Show Throughput Measurement: %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_TM)?"On":"Off"));
#if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE)
    BACKDOOR_MGR_Printf("\r\n 18: Show Delay Measurement     : %s",(CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_DM)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 19: Delay Measurement Test");
#endif
    BACKDOOR_MGR_Printf("\r\n 20: Show CCM Packet Recieve    : %s", (CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_CCM)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 21: Show LT Packet Recieve     : %s", (CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_LT)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 22: Show LB Packet Recieve     : %s", (CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_LB)?"On":"Off"));
    BACKDOOR_MGR_Printf("\r\n 23: Show AIS Packet Recieve    : %s", (CFM_BACKDOOR_Debug(CFM_BACKDOOR_DEBUG_FLAG_AIS)?"On":"Off"));

    BACKDOOR_MGR_Printf("\r\n 99: quit\r\n");
    BACKDOOR_MGR_Printf("input: ");
}

static void CFM_BACKDOOR_GetNum(UI8_T string[])
{
    BACKDOOR_MGR_RequestKeyIn(string, 4);
}

static void CFM_BACKDOOR_DelayMeasure(void)
{
    UI32_T          src_mep_id, dst_mep_id, count;
    unsigned int    dst_mac_x[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T           line_buffer[80], i;
    UI8_T           md_name[80], ma_name[80], dst_mac[SYS_ADPT_MAC_ADDR_LEN];

    BACKDOOR_MGR_Printf("\r\n Md name: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);
    strcpy((char *)md_name, (char *)line_buffer);

    BACKDOOR_MGR_Printf("\r\n Ma name: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);
    strcpy((char *)ma_name, (char *)line_buffer);

    BACKDOOR_MGR_Printf("\r\n Src Mep id: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);
    src_mep_id = atoi((char *) line_buffer);

    BACKDOOR_MGR_Printf("\r\n Dst Mep id: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);
    dst_mep_id = atoi((char *) line_buffer);

    if (dst_mep_id == 0)
    {
        BACKDOOR_MGR_Printf("\r\n Dst Mac: ");
        BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);

        if (sscanf((char *) line_buffer, "%02x-%02x-%02x-%02x-%02x-%02x",
                   &dst_mac_x[0], &dst_mac_x[1], &dst_mac_x[2],
                   &dst_mac_x[3], &dst_mac_x[4], &dst_mac_x[5]) != 6)
        {
            BACKDOOR_MGR_Printf("\r\n wrong mac");
            return;
        }

        for (i =0; i < SYS_ADPT_MAC_ADDR_LEN; i++)
          dst_mac[i] = dst_mac_x[i];
    }

    BACKDOOR_MGR_Printf("\r\n Count: ");
    BACKDOOR_MGR_RequestKeyIn(line_buffer, 32);
    count = atoi((char *) line_buffer);

    if (CFM_TYPE_CONFIG_ERROR == CFM_MGR_DoDelayMeasureByDMM(
                                  src_mep_id, dst_mep_id, dst_mac,
                                  md_name, strlen((char *)md_name),
                                  ma_name, strlen((char *)ma_name),
                                  count, 1, 1, 64, 7))
    {
        BACKDOOR_MGR_Printf("\r\n CFM_MGR_DoDelayMeasureByDMM failed");
    }
}

static void CFM_BACKDOOR_Engine()
{
    BOOL_T  engine_continue = TRUE;
    UI8_T   ch;
    UI8_T   cmd_buf[MAXLINE];

    while(engine_continue)
    {
        ch = 0;
        cmd_buf[0] = 0;

        CFM_BACKDOOR_ShowCmd();
        CFM_BACKDOOR_GetNum(cmd_buf);

        ch = atoi((char *) cmd_buf);

        switch(ch)
        {
            case 99:
                engine_continue = FALSE;
                break;
            case 1: /*show global configuration*/
                CFM_BACKDOOR_ShowGlobalConfig();
                break;
            case 2: /*show port configure*/
                CFM_BACKDOOR_ShowPortConfig();
                break;
            case 3: /*show maitanence domain info*/
                CFM_BACKDOOR_ShowMdInfo();
                break;
            case 4: /*show maintenance association info*/
                CFM_BACKDOOR_ShowMaInfo();
                break;
            case 5:/*show mainenance association info*/
                CFM_BACKDOOR_ShowMpInfo();
                break;
            case 6:/*show remtoep info*/
                CFM_BACKDOOR_ShowRemoteMepInfo();
                break;
            case 7:/*show link trace reply stored info*/
                CFM_BACKDOOR_ShowLinktraceCache();
                break;
            case 8:
                CFM_BACKDOOR_ShowPendingLTRList();
                break;
            case 9:/*show error list ifno*/
                CFM_BACKDOOR_ShowEorrorList();
                break;
            case 10: /*rest all debug flag*/
                CFM_BACKDOOR_RestAll();
                break;
            case 11:/*show packet flow path and content*/
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_RX_PACKET_FLOW);
                break;
            case 12:/*show packet transmit and content*/
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_TX_PACKET_FLOW);
                break;
            case 13:/*show timer event*/
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_TIMER);
                break;
            case 14:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_MACHINE_STATE);
                break;
            case 15:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_TRAP);
                break;
            case 16:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_UI);
                break;
            case 17:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_TM);
                break;
#if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE)
            case 18:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_DM);
                break;
            case 19:
                CFM_BACKDOOR_DelayMeasure();
                break;
#endif /* #if (SYS_CPNT_CFM_DELAY_MEASUREMENT == TRUE) */
            case 20:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_CCM);
                break;
            case 21:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_LT);
                break;
            case 22:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_LB);
                break;
            case 23:
                CFM_BACKDOOR_SetDebugFlag(CFM_BACKDOOR_DEBUG_FLAG_AIS);
                break;
            default:
                continue;
        }
    }
}

static void CFM_BACKDOOR_ShowGlobalConfig()
{
    {/*cfm status*/
        CFM_TYPE_CfmStatus_T status;
        CFM_OM_GetCFMGlobalStatus(&status);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "global cfm status", (status?"Enabled":"Disabled"));
    }
    {
        /*cross check configuration*/
        UI32_T delay;
        CFM_OM_GetCrossCheckStartDelay(&delay);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %lu sec", "Cross Check Start Delay", (long)delay);
    }
    {
        UI32_T linkTraceHoldTime;
        CFM_OM_GetLinkTraceCacheHoldTime(&linkTraceHoldTime);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %lu min", "Linktrace cache hold time", (long)linkTraceHoldTime/60);
    }
    {  /*link trace cache size*/
        UI32_T linkTraceSize;
        CFM_OM_GetLinkTraceCacheSize(&linkTraceSize);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %lu entries", "Linktrace cache size", (long)linkTraceSize);
    }
    {
        CFM_TYPE_LinktraceStatus_T linkTraceCacheStatus;
        CFM_OM_GetLinkTraceCacheStatus(&linkTraceCacheStatus);
        switch(linkTraceCacheStatus)
        {
            case CFM_TYPE_LINKTRACE_STATUS_ENABLE:
            BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "Linktrace cache status","Enabled");
            break;
            case CFM_TYPE_LINKTRACE_STATUS_DISABLE:
            BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "Linktrace cache status","Disabled");
            break;
        }
    }
    {  /*snmp trap global configuration*/
        BOOL_T trap_enabled;
        CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_UP, &trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCcMepUpTrap", (trap_enabled?"TRUE":"FALSE"));

        CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_MEP_DOWN, &trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s","snmpCcMepDownTrap", (trap_enabled?"TRUE":"FALSE"));

        CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_CONFIG, &trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCcConfigTrap", (trap_enabled?"TRUE":"FALSE"));

        CFM_OM_GetSNMPCcStatus(CFM_TYPE_SNMP_TRAPS_CC_LOOP, &trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCcLoopTrap", (trap_enabled?"TRUE":"FALSE"));

        CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_UNKNOWN, &trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCrossCheckMepUnKnownTrap",(trap_enabled?"TRUE":"FALSE"));


        CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MEP_MISSING,&trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCrossCheckMepMissingTrap",(trap_enabled?"TRUE":"FALSE"));

        CFM_OM_GetSNMPCrossCheckStatus(CFM_TYPE_SNMP_TRAPS_CROSS_CHECK_MA_UP,&trap_enabled);
        BACKDOOR_MGR_Printf("\r\n  %-36s: %s", "snmpCrossCheckMaUpTrap", (trap_enabled?"TRUE":"FALSE"));

    }

    if(FALSE == CFM_BACKDOOR_STOP())
    {
        return;
    }

    BACKDOOR_MGR_Printf("\r\n");
    return;
}

static void CFM_BACKDOOR_ShowPortConfig()
{
    UI32_T lport=0;
    CFM_TYPE_CfmStatus_T status;
    UI32_T num=0;

    while(TRUE == SWCTRL_GetNextLogicalPort(&lport))
    {
        if (FALSE == CFM_OM_GetCFMPortStatus(lport, &status))
        {
            BACKDOOR_MGR_Printf("\r\n  logical port %lu get port status failed",(long)lport+1);
            continue;
        }

        switch(status)
        {
            case CFM_TYPE_CFM_STATUS_ENABLE:
                BACKDOOR_MGR_Printf("\r\n  logical port %lu cfm status : Enabled",(long)lport+1);
            break;
            case CFM_TYPE_CFM_STATUS_DISABLE:
                BACKDOOR_MGR_Printf("\r\n  logical port %lu cfm status : Disabled",(long)lport+1);
         }
         if(++num>20)
         {
            if(FALSE == CFM_BACKDOOR_STOP())
            {
                return;
            }

             num=0;
         }
    }
    BACKDOOR_MGR_Printf("\r\n");
    return;
}

static void CFM_BACKDOOR_ShowMdInfo()
{
    UI32_T md_index=0;
    CFM_OM_MD_T *md_p;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Maintenance Domain Info       ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    while( NULL !=( md_p=CFM_OM_GetNextMdByIndex(md_index)))
    {
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu","index", (long)md_p->index);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "level", md_p->level);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "name", md_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "fngAlarmTime", (long)md_p->fng_alarm_time);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "fngResetTime", (long)md_p->fng_reset_time);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "lowPrDef", md_p->low_pri_def);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "archive Hold Time", (long)md_p->mep_achive_holdtime/60);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d",  "mhf create", md_p->mhf_creation);
        md_index=md_p->index;

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }

    }
    BACKDOOR_MGR_Printf("\r\n");
    return;
}

static void CFM_BACKDOOR_ShowMaInfo()
{
    UI32_T md_index=0;
    UI32_T ma_index=0;
    CFM_OM_MD_T *md_p;
    CFM_OM_MA_T *ma_p;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Maintenance association Info       ");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    while( NULL !=( md_p=CFM_OM_GetNextMdByIndex(md_index)))
    {
        ma_index=0;
        while(NULL!=(ma_p=CFM_OM_GetNextMaByMaIndex(md_p->index, ma_index)))
        {
            BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "index", (long)ma_p->index);
            BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "name", ma_p->name_a);
            BACKDOOR_MGR_Printf("\r\n  %-20s: %ld", "vid num", (long)ma_p->num_of_vids);
            BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "CCM interval level", ma_p->ccm_interval);
            switch(ma_p->ccm_status)
            {
            case CFM_TYPE_CCM_STATUS_ENABLE:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "CCM status ","Enabled");
                break;
            case  CFM_TYPE_CCM_STATUS_DISABLE:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "CCM status","Disabled");
                break;
            default:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "CCM status", "Invalid");
                break;
            }
            switch(ma_p->cross_check_status)
            {
            case CFM_TYPE_CROSS_CHECK_STATUS_ENABLE:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "Crosscheck status", "Enabled");
                break;
            case  CFM_TYPE_CROSS_CHECK_STATUS_DISABLE:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "Crosscheck status", "Disabled");
                break;
            default:
                BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "Crosscheck status", "Invalid");
                break;
            }

            /*print ais informatoin*/
            BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "AIS Status", (ma_p->ais_status == CFM_TYPE_AIS_STATUS_ENABLE?"Enabled":"Disabled"));
            BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "AIS Suppress Status", (ma_p->ais_status == CFM_TYPE_AIS_STATUS_ENABLE?"Enabled":"Disabled"));
            BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "AIS level", ma_p->ais_send_level);
            BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "AIS period", ma_p->ais_period);

            BACKDOOR_MGR_Printf("\r\n  %-20s: ","MA vlan lis");
            {
                UI32_T i=0;

                for(i=1; i<=SYS_DFLT_DOT1QMAXVLANID; i++)
                {
                    if(ma_p->vid_bitmap_a[(i-1)/8] &(1 << (7 - ((i-1)%8))))
                        BACKDOOR_MGR_Printf("%d,", i);

                }
            }
            BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "MHF create", ma_p->mhf_creation);
            BACKDOOR_MGR_Printf("\r\n  %-20s: %ld", "rmep_down_counter", (long)ma_p->remote_mep_down_counter);

            ma_index=ma_p->index;
            if(FALSE == CFM_BACKDOOR_STOP())
            {
                return;
            }

        }

        md_index=md_p->index;
    }
    BACKDOOR_MGR_Printf("\r\n");
    return;
}

static void CFM_BACKDOOR_ShowMpInfo()
{
    UI32_T nxt_md_index=0;
    UI32_T nxt_ma_index=0;
    UI32_T nxt_lport=0;
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;
    CFM_OM_MIP_T mip;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Maintenance association Point Info ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    while(TRUE == CFM_OM_GetNextMep(nxt_md_index, nxt_ma_index, 0, nxt_lport, CFM_OM_LPORT_MD_MA_KEY, mep_p))
    {
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP md name", mep_p->md_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP ma name", mep_p->ma_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP id", (long)mep_p->identifier);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lport", (long)mep_p->lport);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %02x-%02x-%02x-%02x-%02x-%02x", "MEP MAC", mep_p->mac_addr_a[0],
                                     mep_p->mac_addr_a[1], mep_p->mac_addr_a[2], mep_p->mac_addr_a[3],
                                     mep_p->mac_addr_a[4], mep_p->mac_addr_a[5]);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %d", "MEP primary vid", mep_p->primary_vid);

        switch(mep_p->direction)
        {
        case CFM_TYPE_MP_DIRECTION_UP:
            BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP Direction", "Up");
            break;
        case CFM_TYPE_MP_DIRECTION_DOWN:
            BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP Direction", "Down");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP Direction", "Invalid");
        }

        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP active",(mep_p->active?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-28s: idx: %2d, last : %lu", "MEP CCM timer", mep_p->cci_while_timer_idx, (long)CFM_TIMER_QureyTime(mep_p->cci_while_timer_idx));
        BACKDOOR_MGR_Printf("\r\n  %-28s: idx: %2d, last : %lu", "MEP Error timer", mep_p->error_ccm_while_timer_idx, (long)CFM_TIMER_QureyTime(mep_p->error_ccm_while_timer_idx));
        BACKDOOR_MGR_Printf("\r\n  %-28s: idx: %2d, last : %lu", "MEP xcon timer" , mep_p->xcon_ccm_while_timer_idx, (long)CFM_TIMER_QureyTime(mep_p->xcon_ccm_while_timer_idx));
        BACKDOOR_MGR_Printf("\r\n  %-28s: idx: %2d, last : %lu", "MEP FNG  timer", mep_p->fng_while_timer_idx, (long)CFM_TIMER_QureyTime(mep_p->fng_while_timer_idx));

        switch(mep_p->fng_machine_state)
        {
            case CFM_TYPE_FNG_STATE_CLERING:
                BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP FNG State","Clearing");
                break;
            case CFM_TYPE_FNG_STATE_DEFECT:
                BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP FNG State", "Defect");
                break;
            case CFM_TYPE_FNG_STATE_REPORTED:
                BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP FNG State", "Reported");
                break;
            case CFM_TYPE_FNG_STATE_RESET:
                BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP FNG State", "Reset");
                break;
            default:
                BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP FNG State", "Invalid");
        }

        BACKDOOR_MGR_Printf("\r\n  %-28s: %d", "MEP Hi Pr Defect ", mep_p->highest_pri_defect);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP some rdi defect",(mep_p->some_rdi_defect?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP err mac status",(mep_p->err_mac_status?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP some rmep ccm defect",(mep_p->some_rmep_ccm_defect?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP err ccm defect",(mep_p->error_ccm_defect?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-28s: %s", "MEP xcon ccm defect",(mep_p->xcon_ccm_defect?"TRUE":"FALSE"));

        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lbr Bad Msdu", (long)mep_p->lbr_bad_msdu);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lbr in", (long)mep_p->lbr_in);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lbr in out of order", (long)mep_p->lbr_in_out_of_order);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lbr next trans id", (long)mep_p->next_lbm_trans_id);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP lbm trans seq num", (long)mep_p->transmit_lbm_seq_number);

        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP ltr next seq num", (long)mep_p->ltm_next_seq_number);

        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP rmep_port_down_cnt",  (long)mep_p->rmep_port_down_cnt);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP rmep_inf_down_cnt", (long)mep_p->rmep_inf_down_cnt);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP rmep_ccm_loss_cnt", (long)mep_p->rmep_ccm_loss_cnt);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP rmep_rdi_on_cnt",   (long)mep_p->rmep_rdi_on_cnt);
        BACKDOOR_MGR_Printf("\r\n  %-28s: %lu", "MEP rmep_lrn_cnt",   (long)mep_p->rmep_lrn_cnt);
        BACKDOOR_MGR_Printf(" ");

        nxt_md_index=mep_p->md_p->index;
        nxt_ma_index=mep_p->ma_p->index;
        nxt_lport=mep_p->lport;

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }

        BACKDOOR_MGR_Printf("\r\n---------------------------------------------------------------------------");

    }

    nxt_md_index =0;
    nxt_ma_index =0;
    nxt_lport=0;
    while(TRUE == CFM_OM_GetNextMip(nxt_md_index, nxt_ma_index, nxt_lport,CFM_OM_LPORT_MD_MA_KEY, &mip))
    {
        BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "Mip md name", mip.md_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "Mip ma name", mip.ma_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "lport", (long)mip.lport);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x", "mip mac", mip.mac_address_a[0], mip.mac_address_a[1], mip.mac_address_a[2]
                                                          , mip.mac_address_a[3], mip.mac_address_a[4], mip.mac_address_a[5]);
        nxt_md_index=mip.md_p->index;
        nxt_ma_index=mip.ma_p->index;
        nxt_lport=mip.lport;

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }

        BACKDOOR_MGR_Printf("\r\n---------------------------------------------------------------------------");
    }
    BACKDOOR_MGR_Printf(" ");
}

static void CFM_BACKDOOR_ShowRemoteMepInfo()
{
    UI32_T md_index=0;
    UI32_T ma_index=0;
    UI32_T mep_id=0;
    CFM_OM_REMOTE_MEP_T remote_mep;
    CFM_OM_MEP_T *mep_p=&mep_tmp3_g;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Show Remote MEP information ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    while(TRUE== CFM_OM_GetNextRemoteMep(md_index,  ma_index, mep_id, CFM_OM_MD_MA_MEP_KEY, &remote_mep))
    {
        md_index=remote_mep.md_p->index;
        ma_index=remote_mep.ma_p->index;
        mep_id=remote_mep.identifier;

        if((0!=remote_mep.rcvd_mep_id)
            &&(TRUE == CFM_OM_GetMep(md_index, ma_index, remote_mep.rcvd_mep_id, 0, CFM_OM_MD_MA_MEP_KEY, mep_p)))
        {
            BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "received by Mep", (long)mep_p->identifier);
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "received by Mep","No Mep receive this remote mep CCM");
        }

        BACKDOOR_MGR_Printf("\r\n  %-23s: %ld", "received lport", (long)remote_mep.rcvd_lport);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "md name", remote_mep.md_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "ma name", remote_mep.ma_p->name_a);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "remote mep id", (long)remote_mep.identifier);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "sequence", (long)remote_mep.next_sequence);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "mep up", remote_mep.mep_up?"TRUE":"FALSE");

        switch(remote_mep.machine_state)
        {
        case CFM_TYPE_REMOTE_MEP_STATE_FAILD:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "machine state","FAILED");
            break;
        case CFM_TYPE_REMOTE_MEP_STATE_IDLE:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "machine state","IDLE");
            break;
        case CFM_TYPE_REMOTE_MEP_STATE_OK:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "machine state","OK");
            break;
        case CFM_TYPE_REMOTE_MEP_STATE_START:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "machine state","START");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "machine state","Invalid");
            break;
        }

        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "Failed Ok Time", (long)remote_mep.failed_ok_time);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %02x-%02x-%02x-%02x-%02x-%02x", "mac address", remote_mep.mac_addr_a[0],remote_mep.mac_addr_a[1],remote_mep.mac_addr_a[2],remote_mep.mac_addr_a[3],remote_mep.mac_addr_a[4],remote_mep.mac_addr_a[5]);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "RDI present",(remote_mep.rdi?"TRUE":"FALSE"));

        switch(remote_mep.port_status)
        {
        case CFM_TYPE_PORT_STATUS_BLOCKED:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Port Status","BLOCKED");
            break;
        case CFM_TYPE_PORT_STATUS_NO_PORT_STATE_TLV:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Port Status","No Port State");
            break;
        case CFM_TYPE_PORT_STATUS_UP:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Port Status","UP");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Port Status","Invalid");
        }

        if(remote_mep.interface_status == CFM_TYPE_INTERFACE_STATUS_UP)
        {
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Inf Status", "UP");
        }
        else
        {
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Inf Status", "NOT UP");
        }
        CFM_BACKDOOR_ShowChassisID(remote_mep.sender_chassis_id_sub_type, remote_mep.sender_chassis_id, remote_mep.sender_chassis_id_length);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %d.%d.%d.%d", "Management Address",
                    remote_mep.man_address[0], remote_mep.man_address[1],
                    remote_mep.man_address[2], remote_mep.man_address[3]);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "ccLifeTime", (long)remote_mep.cc_life_time);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "ageOfLastCc", (long)CFM_TIMER_QureyTime(remote_mep.rmep_while_timer_idx));
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "frameLoss", (long)remote_mep.frame_loss);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "packetReceived", (long)remote_mep.packet_received);
        BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "packetError", (long)remote_mep.packet_error);
        BACKDOOR_MGR_Printf("\r\n  %-23s: idx: %2d, last: %lu", "archive hold time last", remote_mep.archive_hold_timer_idx, (long)CFM_TIMER_QureyTime(remote_mep.archive_hold_timer_idx));
        BACKDOOR_MGR_Printf("\r\n  %-23s: idx: %2d, last: %lu", "rmep while timer last", remote_mep.rmep_while_timer_idx, (long)CFM_TIMER_QureyTime(remote_mep.rmep_while_timer_idx));

        BACKDOOR_MGR_Printf("\r\n---------------------------------------------------------------------------");

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }
    }
}

static void CFM_BACKDOOR_ShowLinktraceCache()
{
    CFM_OM_LTR_T    *ltr_p=&ltr_tmp_g;
    UI32_T          md_index=0,ma_index=0,mep_id=0,seq_num=0,rcvd_order=0;
    UI32_T          num=0;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Show Link Trace Reply Information ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    while(TRUE == CFM_OM_GetNextLtr(md_index, ma_index, mep_id,  seq_num, rcvd_order, CFM_OM_MD_MA_MEP_SEQ_REC_KEY, ltr_p))
    {
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "num", (long)++num);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "md index", (long)ltr_p->md_index);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "ma index", (long)ltr_p->ma_index);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "recevied mep id", (long)ltr_p->rcvd_mep_id);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "sequence number", (long)ltr_p->seq_number);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %lu", "received order", (long)ltr_p->receive_order);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "forwarder",(ltr_p->forwarded?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-20s: %s", "terminal mep",(ltr_p->terminal_mep?"TRUE":"FALSE"));
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", "last Egress Identifier",ltr_p->last_egress_identifier[0],ltr_p->last_egress_identifier[1],ltr_p->last_egress_identifier[2],ltr_p->last_egress_identifier[3],ltr_p->last_egress_identifier[4],ltr_p->last_egress_identifier[5],ltr_p->last_egress_identifier[6],ltr_p->last_egress_identifier[7]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", "next Egress Identifer",ltr_p->next_egress_identifier[0],ltr_p->next_egress_identifier[1],ltr_p->next_egress_identifier[2],ltr_p->next_egress_identifier[3],ltr_p->next_egress_identifier[4],ltr_p->next_egress_identifier[5],ltr_p->next_egress_identifier[6],ltr_p->next_egress_identifier[7]);
        CFM_BACKDOOR_ShowChassisID(ltr_p->chassis_id_subtype, ltr_p->chassis_id,ltr_p->chassis_id_length);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x", "management address", ltr_p->mgmt_addr[0], ltr_p->mgmt_addr[1], ltr_p->mgmt_addr[2], ltr_p->mgmt_addr[3]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x", "ingress port mac",ltr_p->ingress_mac[0],ltr_p->ingress_mac[1],ltr_p->ingress_mac[2],ltr_p->ingress_mac[3],ltr_p->ingress_mac[4],ltr_p->ingress_mac[5]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", "igress port id", ltr_p->ingress_port_Id[0], ltr_p->ingress_port_Id[0], ltr_p->ingress_port_Id[1], ltr_p->ingress_port_Id[2], ltr_p->ingress_port_Id[3], ltr_p->ingress_port_Id[4], ltr_p->ingress_port_Id[5], ltr_p->ingress_port_Id[6]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "ingress action",ltr_p->ingress_action);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x", "egress port mac",ltr_p->egress_mac[0],ltr_p->egress_mac[1],ltr_p->egress_mac[2],ltr_p->egress_mac[3],ltr_p->egress_mac[4],ltr_p->egress_mac[5]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x", "egress port id",ltr_p->egress_port_id[0], ltr_p->egress_port_id[0], ltr_p->egress_port_id[1], ltr_p->egress_port_id[2], ltr_p->egress_port_id[3], ltr_p->egress_port_id[4], ltr_p->egress_port_id[5], ltr_p->egress_port_id[6]);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "egress action",ltr_p->egress_action);
        BACKDOOR_MGR_Printf("\r\n  %-20s: %d", "relay action",ltr_p->relay_action);
        BACKDOOR_MGR_Printf("\r\n\r\n");

        md_index=ltr_p->md_index;
        ma_index=ltr_p->ma_index;
        mep_id=ltr_p->rcvd_mep_id;
        seq_num=ltr_p->seq_number;
        rcvd_order=ltr_p->receive_order;

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }
    }

    BACKDOOR_MGR_Printf("\r\n %20s: %lu", "total num", (long)num);
}

static void CFM_BACKDOOR_ShowPendingLTRList()
{
    CFM_OM_LTR_QUEUE_HEAD_T *ltr_queue_head=CFM_OM_GetLtrQueueHeadPtr();
    CFM_OM_LTR_QueueElement_T *temp_element_p;
    UI32_T num=0;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Show Pending LTR information ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    temp_element_p=ltr_queue_head->first_pdu_p;
    while(NULL!=temp_element_p)
    {
        BACKDOOR_MGR_Printf("\r\n  %-10s: %lu", "num", (long)++num);
        BACKDOOR_MGR_Printf("\r\n  %-10s: %02x-%02x-%02x-%02x-%02x-%02x", "src", temp_element_p->src_mac_a[0],temp_element_p->src_mac_a[2],temp_element_p->src_mac_a[2],temp_element_p->src_mac_a[3],temp_element_p->src_mac_a[4],temp_element_p->src_mac_a[5]);
        BACKDOOR_MGR_Printf("\r\n  %-10s: %02x-%02x-%02x-%02x-%02x-%02x", "dst", temp_element_p->dst_mac_a[0],temp_element_p->dst_mac_a[2],temp_element_p->dst_mac_a[2],temp_element_p->dst_mac_a[3],temp_element_p->dst_mac_a[4],temp_element_p->dst_mac_a[5]);
        BACKDOOR_MGR_Printf("\r\n  %-10s: %lu", "lport", (long)temp_element_p->lport);
        BACKDOOR_MGR_Printf("\r\n  %-10s: %d", "vid", temp_element_p->vid);
        BACKDOOR_MGR_Printf("\r\n  %-10s: %d", "level", temp_element_p->level);
        BACKDOOR_MGR_Printf("\r\n");

        temp_element_p=temp_element_p->next_element_p;

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }
    }
    BACKDOOR_MGR_Printf("\r\n  %30s: %lu", "total in pending list", (long)num);
}

static void CFM_BACKDOOR_ShowEorrorList()
{
    UI32_T          lport=0;
    UI16_T          vid=0;
    CFM_OM_Error_T  error;

    BACKDOOR_MGR_Printf("\r\n\r\n");
    BACKDOOR_MGR_Printf("\r\n****************************************");
    BACKDOOR_MGR_Printf("\r\n  Show Error List Content ");
    BACKDOOR_MGR_Printf("\r\n****************************************");

    while(TRUE == CFM_OM_GetNextErrorInfo(&vid, &lport, &error))
    {
        BACKDOOR_MGR_Printf("\r\n  %-25s: %d",  "error level", error.level);
        BACKDOOR_MGR_Printf("\r\n  %-25s: %ld", "error vid",   (long)error.vlan_id);
        BACKDOOR_MGR_Printf("\r\n  %-25s: %ld", "error lport", (long)error.lport);
        BACKDOOR_MGR_Printf("\r\n  %-25s: %lu", "error remote mep id", (long)error.mep_id);
        switch(error.reason)
        {
        case CFM_TYPE_CONFIG_ERROR_LEAK:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason", "CFM leak");
            break;
        case CFM_TYPE_CONFIG_ERROR_VIDS:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason", "MEP ID");
            break;
        case CFM_TYPE_CONFIG_ERROR_OVERLAPPED_LEVELS:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason", "Overlapped levels");
            break;
        case CFM_TYPE_CONFIG_ERROR_EXCESSIVE_LEVELS:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason", "excessive levels");
            break;
        case CFM_TYPE_CONFIG_ERROR_AIS:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason", "recevie AIS");
            break;
        default:
            BACKDOOR_MGR_Printf("\r\n  %-25s: %s", "error reason"," None");
        }
        BACKDOOR_MGR_Printf("\r\n ");

        if(FALSE == CFM_BACKDOOR_STOP())
        {
            return;
        }
    }
    BACKDOOR_MGR_Printf("\r\n\r\n ");
}

static void CFM_BACKDOOR_RestAll()
{
    CFM_BACKDOOR_DebugFlag=CFM_BACKDOOR_DEBUG_FLAG_NONE;
}

void    CFM_BACKDOOR_SetDebugFlag(UI32_T flag)
{
    CFM_BACKDOOR_DebugFlag   ^= flag;
    return;
}/* End of CFM_BACKDOOR_SetDebugFlag */

BOOL_T  CFM_BACKDOOR_Debug(UI32_T flag)
{
    return ( (CFM_BACKDOOR_DebugFlag & flag) != 0);
} /* End of CFM_BACKDOOR_Debug */

static void CFM_BACKDOOR_ShowChassisID(UI32_T type, UI8_T id[], UI32_T length)
{
    BACKDOOR_MGR_Printf("\r\n  %-23s: %lu", "Chassis ID length", (long)length);

    switch(type)
    {
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_CHASSIS:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "Chassis");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_IFALIAS:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "IFALIAS");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_IFNAME:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "IFNAME");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_LOCAL:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "LOCAL");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_MAC_ADDR:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "MAC_ADDR");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_NETWORK_ADDR:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "NETWORK_ADDR");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_PORT:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "PORT");
            break;
        case CFM_TYPE_TLV_CHASSIS_ID_SUBTYPE_RESERVED:
            BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID Subtype", "RESERVED");
            break;
    }

    BACKDOOR_MGR_Printf("\r\n  %-23s: %s", "Chassis ID", id);

}

static BOOL_T CFM_BACKDOOR_STOP ()
{
    I8_T get_in_char;

    BACKDOOR_MGR_Printf(" \r\n");
    BACKDOOR_MGR_Printf("---Press Space: Next     Press Other: End");
    get_in_char=BACKDOOR_MGR_GetChar();
    if(get_in_char!=' ')
    {
        return FALSE;
    }

    return TRUE;
}
#endif /*#if (SYS_CPNT_CFM == TRUE)*/

