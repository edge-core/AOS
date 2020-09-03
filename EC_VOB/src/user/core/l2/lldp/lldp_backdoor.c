/*-----------------------------------------------------------------------------
 * Module Name: lldp_backdoor.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the LLDP backdoor
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
#include "l_stdlib.h"
#include "l_threadgrp.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "lldp_backdoor.h"
#include "lldp_om_private.h"
#include "lldp_mgr.h"
#include "lldp_type.h"
#include "swctrl.h"


#define MAXLINE 255


static void LLDP_BACKDOOR_Engine();
static void LLDP_BACKDOOR_ShowCmd();
static void LLDP_BACKDOOR_ShowSysConfig();
static void LLDP_BACKDOOR_SetSysConfig();
static void LLDP_BACKDOOR_ShowPortInfo(UI32_T lport);
static void LLDP_BACKDOOR_SetPortConfig(UI32_T lport);
static void LLDP_BACKDOOR_ShowSystemStatistics();
static void LLDP_BACKDOOR_ShowPortStatistics(UI32_T lport);
static void LLDP_BACKDOOR_ShowLocManAddrConfig();
static void LLDP_BACKDOOR_SetLocManAddrConfig(UI32_T lport);
static void LLDP_BACKDOOR_ShowRemDataByPort(UI32_T lport);
static void LLDP_BACKDOOR_ShowRemManAddrByPort(UI32_T lport);
static void LLDP_BACKDOOR_ShowLocLocationEntry(UI32_T lport);
static void LLDP_BACKDOOR_SetLocLocationEntry(UI32_T lport);
static void LLDP_BACKDOOR_SetLocLocatoinCountry(UI32_T lport);
static void LLDP_BACKDOOR_SetLocLocationWhat(UI32_T lport);
static void LLDP_BACKDOOR_SetLocLocationCaEntry(UI32_T lport);


static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;
static UI32_T               lldp_backdoor_debug_flag;


/* ------------------------------------------------------------------------
 * ROUTINE NAME - LLDP_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

void LLDP_BACKDOOR_Main(void)
{
    tg_handle = L2_L4_PROC_COMM_GetGvrpGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\r\n%s: L_THREADGRP_Join fail.\r\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("\r\nLLDP Backdoor");
    LLDP_BACKDOOR_Engine();

    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LLDP_BACKDOOR_GetDebugFlag
 * ------------------------------------------------------------------------
 * FUNCTION : This function is to get debug flag
 * INPUT    : id -- LLDP_BACKDOOR_DEBUG_XXX
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T LLDP_BACKDOOR_GetDebugFlag(UI32_T id)
{
    return !!(lldp_backdoor_debug_flag & id);
}

static void LLDP_BACKDOOR_Engine()
{
    BOOL_T engine_continue = TRUE;
    UI8_T input;
    char cmd_buf[MAXLINE+1] = {0};
    UI32_T lport;

    while (engine_continue)
    {
        input = 0;

        LLDP_BACKDOOR_ShowCmd();
        BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
        input = atoi(cmd_buf);

        switch (input)
        {
            case 1:
                LLDP_BACKDOOR_ShowSysConfig();
                break;
            case 2:
                LLDP_BACKDOOR_SetSysConfig();
                break;
            case 3:
                BACKDOOR_MGR_Printf("\r\nWhich port: ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_ShowPortInfo(lport);
                }
                break;
            case 4:
                BACKDOOR_MGR_Printf("\r\nWhich port: ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetPortConfig(lport);
                }
                break;
            case 5:
                LLDP_BACKDOOR_ShowSystemStatistics();
                break;
            case 6:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_ShowPortStatistics(lport);
                }
                break;
            case 7:
                LLDP_BACKDOOR_ShowLocManAddrConfig();
                break;
            case 8:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetLocManAddrConfig(lport);
                }
                break;
            case 9:
                BACKDOOR_MGR_Printf("\r\nTotal number of remote data:%ld\r\n", (long)LLDP_OM_GetTotalNumOfRemData());
                BACKDOOR_MGR_Printf("Which port: ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_ShowRemDataByPort(lport);
                }
                break;
            case 10:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_ShowRemManAddrByPort(lport);;
                }
                break;
            case 11:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_ShowLocLocationEntry(lport);;
                }
                break;
            case 12:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetLocLocationEntry(lport);;
                }
                break;
            case 13:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if(SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetLocLocatoinCountry(lport);;
                }
                break;
            case 14:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetLocLocationWhat(lport);
                }
                break;
            case 15:
                BACKDOOR_MGR_Printf("\r\nWhich port:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                lport = atoi(cmd_buf);
                if (SWCTRL_LogicalPortExisting(lport) == TRUE)
                {
                    LLDP_BACKDOOR_SetLocLocationCaEntry(lport);
                }
                break;
            default:
                engine_continue = FALSE;
                break;
        } /* end of switch */
    } /* end of while */
}

static void LLDP_BACKDOOR_ShowCmd()
{
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf(" 1: show system config\r\n");
    BACKDOOR_MGR_Printf(" 2: set system config\r\n");
    BACKDOOR_MGR_Printf(" 3: show port info\r\n");
    BACKDOOR_MGR_Printf(" 4: set port config\r\n");
    BACKDOOR_MGR_Printf(" 5: show system statistics\r\n");
    BACKDOOR_MGR_Printf(" 6: show port statistics\r\n");
    BACKDOOR_MGR_Printf(" 7: show local management address config\r\n");
    BACKDOOR_MGR_Printf(" 8: set local management address config\r\n");
    BACKDOOR_MGR_Printf(" 9: show remote data\r\n");
    BACKDOOR_MGR_Printf("10: show remote management addr\r\n");
    BACKDOOR_MGR_Printf("11: show local location entry\r\n");
    BACKDOOR_MGR_Printf("12: set local location entry\r\n");
    BACKDOOR_MGR_Printf("13: set local location coutry\r\n");
    BACKDOOR_MGR_Printf("14: set local location what\r\n");
    BACKDOOR_MGR_Printf("15: set local location ca entries\r\n");
    BACKDOOR_MGR_Printf("other: quit\r\n");
    BACKDOOR_MGR_Printf("input: ");
}

static void LLDP_BACKDOOR_ShowSysConfig()
{
    LLDP_OM_SysConfigEntry_T    *sys_config;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_global_admin_status: %ld\r\n", (long)sys_config->global_admin_status);
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_msg_tx_interval: %ld\r\n", (long)sys_config->msg_tx_interval);
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_msg_tx_hold_mul: %ld\r\n", (long)sys_config->msg_tx_hold_mul);
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_reinit_delay: %ld\r\n", (long)sys_config->reinit_delay);
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_tx_delay: %ld\r\n", (long)sys_config->tx_delay);
    BACKDOOR_MGR_Printf("LLDP_SYSCFG_notification_interval: %ld\r\n", (long)sys_config->notification_interval);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_SetSysConfig()
{
    char cmd_buf[MAXLINE+1] = {0};
    int input = 0;
    int value = 0;
    LLDP_OM_SysConfigEntry_T    *sys_config;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("1. LLDP_SYSCFG_global_admin_status: %lu\r\n", (unsigned long)sys_config->global_admin_status);
        BACKDOOR_MGR_Printf("2. LLDP_SYSCFG_msg_tx_interval: %lu\r\n", (unsigned long)sys_config->msg_tx_interval);
        BACKDOOR_MGR_Printf("3. LLDP_SYSCFG_msg_tx_hold_mul: %lu\r\n", (unsigned long)sys_config->msg_tx_hold_mul);
        BACKDOOR_MGR_Printf("4. LLDP_SYSCFG_reinit_delay: %lu\r\n", (unsigned long)sys_config->reinit_delay);
        BACKDOOR_MGR_Printf("5. LLDP_SYSCFG_tx_delay: %lu\r\n", (unsigned long)sys_config->tx_delay);
        BACKDOOR_MGR_Printf("6. LLDP_SYSCFG_notification_interval: %lu\r\n", (unsigned long)sys_config->notification_interval);
        BACKDOOR_MGR_Printf("other. exit\r\n");
        BACKDOOR_MGR_Printf("choose:");

        BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
        input = atoi(cmd_buf);
        switch(input)
        {
            case 1:
                BACKDOOR_MGR_Printf("\r\nInput value (0. disable, 1.enable): ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                if(value == 1) LLDP_MGR_SetSysAdminStatus(LLDP_TYPE_SYS_ADMIN_ENABLE);
                if(value == 0) LLDP_MGR_SetSysAdminStatus(LLDP_TYPE_SYS_ADMIN_DISABLE);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 2:
                BACKDOOR_MGR_Printf("\r\nInput value [5-32768]:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LLDP_MGR_SetMsgTxInterval(value);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 3:
                BACKDOOR_MGR_Printf("\r\nInput value [2-10]:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LLDP_MGR_SetMsgTxHoldMul(value);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 4:
                BACKDOOR_MGR_Printf("\r\nInput value [1-10]:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LLDP_MGR_SetReinitDelay(value);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 5:
                BACKDOOR_MGR_Printf("\r\nInput value [1-8192]:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LLDP_MGR_SetTxDelay(value);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 6:
                BACKDOOR_MGR_Printf("\r\nInput value [5-3600]:");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                LLDP_MGR_SetNotificationInterval(value);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            default:
                BACKDOOR_MGR_Printf("\r\n");
                return;
        } /* end of switch */
    } /* end of while (1) */
}

static void LLDP_BACKDOOR_ShowPortInfo(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T   *port_config;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("-- Port Configuration -- \r\n");
    BACKDOOR_MGR_Printf("LLDP_PORT_CFG_admin_status: %hu\r\n", port_config->admin_status);
    BACKDOOR_MGR_Printf("LLDP_PORT_CFG_notification_enable: %lu\r\n", (unsigned long)port_config->notification_enable);
    BACKDOOR_MGR_Printf("LLDP_PORT_CFG_basic_tlvs_tx_flag: %hu\r\n", port_config->basic_tlvs_tx_flag);
    BACKDOOR_MGR_Printf("LLDP_PORT_CFG_man_addr_transfer_flag: %dhur\n", port_config->man_addr_transfer_flag);
    BACKDOOR_MGR_Printf("-- Port Status --\r\n");
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_reinit_flag: %hu\r\n", port_config->reinit_flag);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_something_changed_local: %hu\r\n", port_config->something_changed_local);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_something_changed_remote: %hu\r\n", port_config->something_changed_remote);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_transfer_timer: %lu\r\n", (unsigned long)port_config->transfer_timer);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_tx_delay_timer: %lu\r\n", (unsigned long)port_config->tx_delay_timer);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_reinit_delay_timer: %lu\r\n", (unsigned long)port_config->reinit_delay_timer);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_notify_timer: %lu\r\n", (unsigned long)port_config->notify_timer);
    BACKDOOR_MGR_Printf("LLDP_PORT_STATUS_dcbx_peer_num: %hu\r\n", port_config->rx_dcbx_peer_num);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_SetPortConfig(UI32_T lport)
{
    char cmd_buf[MAXLINE+1];
    int input = 0;
    int value = 0;
    LLDP_OM_PortConfigEntry_T   *port_config;

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("port_num: %ld\r\n", (long)lport);
        BACKDOOR_MGR_Printf("1. LLDP_PORT_CFG_admin_status: %d\r\n", port_config->admin_status);
        BACKDOOR_MGR_Printf("2. LLDP_PORT_CFG_notification_enable: %ld\r\n", (long)port_config->notification_enable);
        BACKDOOR_MGR_Printf("3. LLDP_PORT_CFG_basic_tlvs_tx_flag: %d\r\n", port_config->basic_tlvs_tx_flag);
        BACKDOOR_MGR_Printf("4. LLDP_PORT_CFG_man_addr_transfer_flag: %d\r\n", port_config->man_addr_transfer_flag);
        BACKDOOR_MGR_Printf("5. LLDP_PORT_CFG_dot1_port_vlan_tlvs_tx_flag: %d\r\n", port_config->xdot1_port_vlan_tx_enable);
        BACKDOOR_MGR_Printf("6. LLDP_PORT_CFG_dot1_protocol_tlvs_tx_flag: %d\r\n", port_config->xdot1_protocol_tx_enable);
        BACKDOOR_MGR_Printf("7. LLDP_PORT_CFG_dot1_proto_vlan_tlvs_tx_flag: %d\r\n", port_config->xdot1_proto_vlan_tx_enable);
        BACKDOOR_MGR_Printf("8. LLDP_PORT_CFG_dot1_vlan_name_tlvs_tx_flag: %d\r\n", port_config->xdot1_vlan_name_tx_enable);
        BACKDOOR_MGR_Printf("9. LLDP_PORT_CFG_dot3_tlvs_tx_flag: %d\r\n", port_config->xdot3_tlvs_tx_flag);
        BACKDOOR_MGR_Printf("other. exit\r\n");
        BACKDOOR_MGR_Printf("choose:");
        BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
        input = atoi(cmd_buf);

        switch(input)
        {
            case 1:
                BACKDOOR_MGR_Printf("\r\nInput value (1.tx, 2.rx, 3.rx_tx, 4.disable): ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                if(value == 1) LLDP_MGR_SetPortConfigAdminStatus(lport, VAL_lldpPortConfigAdminStatus_txOnly);
                if(value == 2) LLDP_MGR_SetPortConfigAdminStatus(lport, VAL_lldpPortConfigAdminStatus_rxOnly);
                if(value == 3) LLDP_MGR_SetPortConfigAdminStatus(lport, VAL_lldpPortConfigAdminStatus_txAndRx);
                if(value == 4) LLDP_MGR_SetPortConfigAdminStatus(lport, VAL_lldpPortConfigAdminStatus_disabled);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 2:
                BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                if(value == 1) LLDP_MGR_SetPortConfigNotificationEnable(lport, TRUE);
                if(value == 2) LLDP_MGR_SetPortConfigNotificationEnable(lport, FALSE);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 3:
                BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                if(value == 1) port_config->basic_tlvs_tx_flag = LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG;
                if(value == 2) port_config->basic_tlvs_tx_flag = 0;
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
            case 4:
                BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                value = atoi(cmd_buf);
                /* Get execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                if(value == 1) LLDP_MGR_SetConfigManAddrTlv(lport, TRUE);
                if(value == 2) LLDP_MGR_SetConfigManAddrTlv(lport, FALSE);
                /* Release execution permission from the thread group handler if necessary
                 */
                L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                break;
#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
            case 5:
                {
                    LLDP_MGR_Xdot1ConfigEntry_T     xdot1_config_entry;

                    BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                    BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                    value = atoi(cmd_buf);
                    xdot1_config_entry.lport = lport;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    LLDP_MGR_GetXdot1ConfigEntry(&xdot1_config_entry);
                    if(value == 1) xdot1_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_true;
                    if(value == 2) xdot1_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_false;
                    LLDP_MGR_SetXdot1ConfigEntry(&xdot1_config_entry);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;
            case 6:
                {
                    LLDP_MGR_Xdot1ConfigEntry_T     xdot1_config_entry;

                    BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                    BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                    value = atoi(cmd_buf);
                    xdot1_config_entry.lport = lport;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    LLDP_MGR_GetXdot1ConfigEntry(&xdot1_config_entry);
                    if(value == 1) xdot1_config_entry.protocol_tx_enable= VAL_lldpXdot1ConfigProtocolTxEnable_true;
                    if(value == 2) xdot1_config_entry.protocol_tx_enable = VAL_lldpXdot1ConfigProtocolTxEnable_false;
                    LLDP_MGR_SetXdot1ConfigEntry(&xdot1_config_entry);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;
            case 7:
                {
                    LLDP_MGR_Xdot1ConfigEntry_T     xdot1_config_entry;

                    BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                    BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                    value = atoi(cmd_buf);
                    xdot1_config_entry.lport = lport;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    LLDP_MGR_GetXdot1ConfigEntry(&xdot1_config_entry);
                    if(value == 1) xdot1_config_entry.proto_vlan_tx_enable= VAL_lldpXdot1ConfigProtoVlanTxEnable_true;
                    if(value == 2) xdot1_config_entry.proto_vlan_tx_enable = VAL_lldpXdot1ConfigProtoVlanTxEnable_false;
                    LLDP_MGR_SetXdot1ConfigEntry(&xdot1_config_entry);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;
            case 8:
                {
                    LLDP_MGR_Xdot1ConfigEntry_T     xdot1_config_entry;

                    BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                    BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                    value = atoi(cmd_buf);
                    xdot1_config_entry.lport = lport;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    LLDP_MGR_GetXdot1ConfigEntry(&xdot1_config_entry);
                    if(value == 1) xdot1_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_true;
                    if(value == 2) xdot1_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_false;
                    LLDP_MGR_SetXdot1ConfigEntry(&xdot1_config_entry);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */
#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
            case 9:
                {
                    LLDP_MGR_Xdot3PortConfigEntry_T xdot3_port_config_entry;

                    BACKDOOR_MGR_Printf("\r\nInput value (1.enable, 2.disable): ");
                    BACKDOOR_MGR_RequestKeyIn(cmd_buf, MAXLINE);
                    value = atoi(cmd_buf);
                    xdot3_port_config_entry.lport = lport;
                    /* Get execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
                    LLDP_MGR_GetXdot3PortConfigEntry(&xdot3_port_config_entry);
                    if(value == 1)
                    {
                        port_config->xdot3_tlvs_tx_flag = LLDP_TYPE_DEFAULT_XDOT3_PORT_CONFIG;
                    }
                    if(value == 2)
                    {
                        port_config->xdot3_tlvs_tx_flag = 0;
                    }
                    LLDP_MGR_SetXdot3PortConfig(&xdot3_port_config_entry);
                    /* Release execution permission from the thread group handler if necessary
                     */
                    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
                }
                break;
#endif
            default:
                BACKDOOR_MGR_Printf("\r\n");
                return;
        } /* end of switch */
    } /* end of while */

}

static void LLDP_BACKDOOR_ShowSystemStatistics()
{
    LLDP_OM_Statistics_T    *sys_stat;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    sys_stat = LLDP_OM_GetStatisticsPtr();
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("System statistics information:\r\n");
    BACKDOOR_MGR_Printf("rem_table_last_change_time:%ld\r\n", (long)sys_stat->rem_table_last_change_time);
    BACKDOOR_MGR_Printf("rem_table_inserts:%ld\r\n", (long)sys_stat->rem_table_inserts);
    BACKDOOR_MGR_Printf("rem_table_deletes:%ld\r\n", (long)sys_stat->rem_table_deletes);
    BACKDOOR_MGR_Printf("rem_table_drops:%ld\r\n", (long)sys_stat->rem_table_drops);
    BACKDOOR_MGR_Printf("rem_table_ageouts:%ld\r\n", (long)sys_stat->rem_table_ageouts);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_ShowPortStatistics(UI32_T lport)
{
    LLDP_OM_PortStatistics_T    *port_stat;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    port_stat = LLDP_OM_GetPortStatisticsEntryPtr(lport);
    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("Port statistics information:\r\n");
    BACKDOOR_MGR_Printf("tx_frames_total:%ld\r\n", (long)port_stat->tx_frames_total);
    BACKDOOR_MGR_Printf("rx_frames_discarded_total:%ld\r\n", (long)port_stat->rx_frames_discarded_total);
    BACKDOOR_MGR_Printf("rx_frames_errors:%ld\r\n", (long)port_stat->rx_frames_errors);
    BACKDOOR_MGR_Printf("rx_frames_total:%ld\r\n", (long)port_stat->rx_frames_total);
    BACKDOOR_MGR_Printf("rx_tlvs_discarded_total:%ld\r\n", (long)port_stat->rx_tlvs_discarded_total);
    BACKDOOR_MGR_Printf("rx_tlvs_unrecognized_total:%ld\r\n", (long)port_stat->rx_tlvs_unrecognized_total);
    BACKDOOR_MGR_Printf("rx_ageouts_total:%ld\r\n", (long)port_stat->rx_ageouts_total);
    BACKDOOR_MGR_Printf("rx_telelphone_total:%ld\r\n", (long)port_stat->rx_telephone_total);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_ShowLocManAddrConfig()
{
    int i;
    LLDP_MGR_LocalManagementAddrEntry_T    man_config;

    BACKDOOR_MGR_Printf("\r\n-- Local management address configuration --\r\n");
    memset(&man_config, 0, sizeof(LLDP_MGR_LocalManagementAddrEntry_T));

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    while(LLDP_MGR_GetNextLocManAddrConfigEntry(&man_config) != LLDP_TYPE_RETURN_ERROR)
    {
        BACKDOOR_MGR_Printf("loc_man_address_subtype: %d\r\n", man_config.loc_man_addr_subtype);
        BACKDOOR_MGR_Printf("loc_man_address:");
        for(i = 0; i < man_config.loc_man_addr_len; i++)
            BACKDOOR_MGR_Printf("%02X", man_config.loc_man_addr[i]);
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("loc_man_addr_len:%ld\r\n", (long)man_config.loc_man_addr_len);
        BACKDOOR_MGR_Printf("tx_enable_ports:");
        for(i = 0; i < 24; i++)
        {
            if(man_config.ports_tx_enable[i/8] & (1 << i % 8))
            {
                BACKDOOR_MGR_Printf("port_%d\t", (i + 1));
            }
        }
        BACKDOOR_MGR_Printf("\r\n");
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_SetLocManAddrConfig(UI32_T lport)
{
    UI8_T   addr[4];

    addr[0] = 192;
    addr[1] = 168;
    addr[2] = 1;
    addr[3] = 1;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    LLDP_MGR_SetLocManAddrConfig(lport,
                                 LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4,
                                 addr,
                                 4,
                                 TRUE);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_ShowRemDataByPort(UI32_T lport)
{
    LLDP_OM_RemData_T                       *rem_data;
    LLDP_MGR_RemoteSystemData_T             rem_sys_data;
    LLDP_MGR_RemoteManagementAddrEntry_T    rem_man_addr_entry;
    LLDP_MGR_Xdot1RemEntry_T                xdot1_rem_entry;
    LLDP_MGR_Xdot1RemProtocolEntry_T        xdot1_rem_protocol_entry;
    LLDP_MGR_Xdot1RemProtoVlanEntry_T       xdot1_rem_proto_vlan_entry;
    LLDP_MGR_Xdot1RemVlanNameEntry_T        xdot1_rem_vlan_name_entry;
    LLDP_MGR_Xdot3RemPortEntry_T            xdot3_rem_port_entry;
    LLDP_MGR_Xdot3RemPowerEntry_T           xdot3_rem_power_entry;
    LLDP_MGR_Xdot3RemLinkAggEntry_T         xdot3_rem_link_agg_entry;
    LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T    xdot3_rem_max_frame_size_entry;
#if (LLDP_TYPE_MED == TRUE)
    LLDP_MGR_XMedRemCapEntry_T              xmed_rem_cap_entry;
    LLDP_MGR_XMedRemMediaPolicyEntry_T      xmed_rem_med_policy_entry;
    LLDP_MGR_XMedRemInventoryEntry_T        xmed_rem_inv_entry;
    LLDP_MGR_XMedRemPoeEntry_T              xmed_rem_poe_entry;
    LLDP_MGR_XMedRemPoePseEntry_T           xmed_rem_poe_pse_entry;
    LLDP_MGR_XMedRemPoePdEntry_T            xmed_rem_pd_entry;
    LLDP_MGR_XMedRemLocationEntry_T         xmed_rem_loc_entry;
#endif
    int i;
    UI8_T  app_type;

    memset(&rem_sys_data, 0, sizeof(LLDP_MGR_RemoteSystemData_T));
    memset(&rem_man_addr_entry, 0, sizeof(LLDP_MGR_RemoteManagementAddrEntry_T));
    memset(&xdot1_rem_entry, 0, sizeof(LLDP_MGR_Xdot1RemEntry_T));
    memset(&xdot1_rem_protocol_entry, 0, sizeof(LLDP_MGR_Xdot1RemProtocolEntry_T));
    memset(&xdot1_rem_proto_vlan_entry, 0, sizeof(LLDP_MGR_Xdot1RemProtoVlanEntry_T));
    memset(&xdot1_rem_vlan_name_entry, 0, sizeof(LLDP_MGR_Xdot1RemVlanNameEntry_T));
    memset(&xdot3_rem_port_entry, 0, sizeof(LLDP_MGR_Xdot3RemPortEntry_T));
    memset(&xdot3_rem_power_entry, 0, sizeof(LLDP_MGR_Xdot3RemPowerEntry_T));
    memset(&xdot3_rem_link_agg_entry, 0, sizeof(LLDP_MGR_Xdot3RemLinkAggEntry_T));
    memset(&xdot3_rem_max_frame_size_entry, 0, sizeof(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T));
#if (LLDP_TYPE_MED == TRUE)
    memset(&xmed_rem_cap_entry, 0, sizeof(LLDP_MGR_XMedRemCapEntry_T));
    memset(&xmed_rem_med_policy_entry, 0, sizeof(LLDP_MGR_XMedRemMediaPolicyEntry_T));
    memset(&xmed_rem_inv_entry, 0, sizeof(LLDP_MGR_XMedRemInventoryEntry_T));
    memset(&xmed_rem_poe_entry, 0, sizeof(LLDP_MGR_XMedRemPoeEntry_T));
    memset(&xmed_rem_poe_pse_entry, 0, sizeof(LLDP_MGR_XMedRemPoePseEntry_T));
    memset(&xmed_rem_pd_entry, 0, sizeof(LLDP_MGR_XMedRemPoePdEntry_T));
    memset(&xmed_rem_loc_entry, 0, sizeof(LLDP_MGR_XMedRemLocationEntry_T));
#endif

    rem_sys_data.rem_time_mark = 0;
    rem_sys_data.rem_local_port_num = lport;
    rem_sys_data.rem_index = 0;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    while(LLDP_MGR_GetNextRemoteSystemDataByIndex(&rem_sys_data))
    {
        LLDP_OM_GetRemData(rem_sys_data.rem_local_port_num, rem_sys_data.rem_index, &rem_data);
        if(rem_sys_data.rem_local_port_num != lport)
            break;
        BACKDOOR_MGR_Printf("\r\nCurrent_time: %lu\r\n", (unsigned long)SYSFUN_GetSysTick());
        BACKDOOR_MGR_Printf("Expire_time: %lu\r\n", (unsigned long)rem_data->age_out_time);
        BACKDOOR_MGR_Printf("RemDataInfo:\r\n");
        BACKDOOR_MGR_Printf("time_mark: %lu\r\n", (unsigned long)rem_data->time_mark);
        BACKDOOR_MGR_Printf("lport:%lu\r\n", (unsigned long)rem_sys_data.rem_local_port_num);
        BACKDOOR_MGR_Printf("index:%lu\r\n", (unsigned long)rem_sys_data.rem_index);
        BACKDOOR_MGR_Printf("chassis_id:");
        for(i = 0; i < rem_sys_data.rem_chassis_id_len; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_sys_data.rem_chassis_id[i]);
        }
        BACKDOOR_MGR_Printf("\r\nport_id:");
        for(i = 0; i < rem_sys_data.rem_port_id_len; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_sys_data.rem_port_id[i]);
        }
        BACKDOOR_MGR_Printf("\r\nport_desc:");
        for(i = 0; i < rem_sys_data.rem_port_desc_len; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_sys_data.rem_port_desc[i]);
        }
        BACKDOOR_MGR_Printf("\r\nsystem_desc:");
        for(i = 0; i < rem_sys_data.rem_sys_desc_len; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_sys_data.rem_sys_desc[i]);
        }
        BACKDOOR_MGR_Printf("\r\nsystem_name:");
        for(i = 0; i < rem_sys_data.rem_sys_name_len; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_sys_data.rem_sys_name[i]);
        }
        BACKDOOR_MGR_Printf("\r\nsystem_capabiltiy supported:");
        BACKDOOR_MGR_Printf("%04X", rem_sys_data.rem_sys_cap_supported);
        BACKDOOR_MGR_Printf("\r\nsystem_capabiltiy enabled:");
        BACKDOOR_MGR_Printf("%04X", rem_sys_data.rem_sys_cap_enabled);
        BACKDOOR_MGR_Printf("\r\n");
        rem_man_addr_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        rem_man_addr_entry.rem_index = rem_sys_data.rem_index;
        while(LLDP_OM_GetNextRemManAddrByIndex(&rem_man_addr_entry))
        {
            BACKDOOR_MGR_Printf("man_addr:");
            for(i = 0;i < rem_man_addr_entry.rem_man_addr_len; i++)
            {
                BACKDOOR_MGR_Printf("%02X", rem_man_addr_entry.rem_man_addr[i]);
            }
            BACKDOOR_MGR_Printf("\r\n");
            BACKDOOR_MGR_Printf("if:%ld\r\n", (long)rem_man_addr_entry.rem_man_addr_if_id);
        }

        /* extensions xdot1 */
#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
        xdot1_rem_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot1_rem_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXdot1RemEntry(&xdot1_rem_entry);
        BACKDOOR_MGR_Printf("xdot1_rem_port_vlan_id: %ld\r\n", (long)xdot1_rem_entry.rem_port_vlan_id);

        xdot1_rem_protocol_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        rem_man_addr_entry.rem_index = rem_sys_data.rem_index;

        while(LLDP_OM_GetNextXdot1RemProtocolEntryByIndex(&xdot1_rem_protocol_entry))
        {
            BACKDOOR_MGR_Printf("xdot1_rem_protocol_index: %ld\r\n", (long)xdot1_rem_protocol_entry.rem_protocol_index);
            BACKDOOR_MGR_Printf("xdot1_rem_protocol_id:");
            for(i = 0; i < xdot1_rem_protocol_entry.rem_protocol_id_len; i++)
            {
                BACKDOOR_MGR_Printf("%02X", xdot1_rem_protocol_entry.rem_protocol_id[i]);
            }
            BACKDOOR_MGR_Printf("\r\n");
        }

        xdot1_rem_proto_vlan_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot1_rem_proto_vlan_entry.rem_index = rem_sys_data.rem_index;

        while(LLDP_OM_GetNextXdot1RemProtoVlanEntryByIndex(&xdot1_rem_proto_vlan_entry))
        {
            BACKDOOR_MGR_Printf("xdot1_rem_proto_vlan_supported: %d\r\n", xdot1_rem_proto_vlan_entry.rem_proto_vlan_supported);
            BACKDOOR_MGR_Printf("xdot1_rem_proto_vlan_enabled: %d\r\n", xdot1_rem_proto_vlan_entry.rem_proto_vlan_enabled);
            BACKDOOR_MGR_Printf("xdot1_rem_proto_vlan_id: %ld\r\n", (long)xdot1_rem_proto_vlan_entry.rem_proto_vlan_id);
        }

        xdot1_rem_vlan_name_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot1_rem_vlan_name_entry.rem_index = rem_sys_data.rem_index;

        while(LLDP_OM_GetNextXdot1RemVlanNameEntryByIndex(&xdot1_rem_vlan_name_entry))
        {
            BACKDOOR_MGR_Printf("xdot1_rem_vlan_name_id: %ld\r\n", (long)xdot1_rem_vlan_name_entry.rem_vlan_id);
            BACKDOOR_MGR_Printf("xdot1_rem_vlan_name: ");
            for(i = 0; i < xdot1_rem_vlan_name_entry.rem_vlan_name_len; i++)
                BACKDOOR_MGR_Printf("%c", xdot1_rem_vlan_name_entry.rem_vlan_name[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        xdot1_rem_protocol_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot1_rem_protocol_entry.rem_index = rem_sys_data.rem_index;
        while(LLDP_OM_GetNextXdot1RemProtocolEntryByIndex(&xdot1_rem_protocol_entry))
        {
            BACKDOOR_MGR_Printf("xdot1_rem_protocol_ident_index:%ld\r\n", (long)xdot1_rem_protocol_entry.rem_protocol_index);
            BACKDOOR_MGR_Printf("xdot1_rem_protocol_ident:");
            for(i = 0; i < xdot1_rem_protocol_entry.rem_protocol_id_len; i++)
            {
                BACKDOOR_MGR_Printf("%02X", xdot1_rem_protocol_entry.rem_protocol_id[i]);
            }
            BACKDOOR_MGR_Printf("\r\n");

        }
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

        /* extensions xdot3 */
#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
        xdot3_rem_port_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot3_rem_port_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXdot3RemPortEntry(&xdot3_rem_port_entry);

        BACKDOOR_MGR_Printf("xdot3_rem_port_auto_neg_supported: %d\r\n",
        xdot3_rem_port_entry.rem_port_auto_neg_supported);

        BACKDOOR_MGR_Printf("xdot3_rem_port_auto_neg_enable: %d\r\n",
        xdot3_rem_port_entry.rem_port_auto_neg_enable);

        BACKDOOR_MGR_Printf("xdot3_rem_port_oper_mau_type: %d\r\n",
        xdot3_rem_port_entry.rem_port_oper_mau_type);

        BACKDOOR_MGR_Printf("xdot3_rem_port_auto_neg_adv_cap:");
        for(i = 0; i < 2; i++)
            BACKDOOR_MGR_Printf("%02X",((UI8_T *)&xdot3_rem_port_entry.rem_port_auto_neg_adv_cap)[i]);
        BACKDOOR_MGR_Printf("\r\n");

        xdot3_rem_power_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot3_rem_power_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXdot3RemPowerEntry(&xdot3_rem_power_entry);
        BACKDOOR_MGR_Printf("rem_power_class:%d\r\nrem_power_mdi_supported:%d\r\nrem_power_mdi_enabled:%d\r\nrem_power_pairs:%d\r\nrem_power_pair_controlable:%d\r\nrem_power_port_class:%d\r\n",
               xdot3_rem_power_entry.rem_power_class,
               xdot3_rem_power_entry.rem_power_mdi_supported,
               xdot3_rem_power_entry.rem_power_mdi_enabled,
               xdot3_rem_power_entry.rem_power_pairs,
               xdot3_rem_power_entry.rem_power_pair_controlable,
               xdot3_rem_power_entry.rem_power_port_class);

        xdot3_rem_link_agg_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot3_rem_link_agg_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXdot3RemLinkAggEntry(&xdot3_rem_link_agg_entry);
        BACKDOOR_MGR_Printf("rem_link_agg_status:%d\r\nrem_link_agg_port_id:%ld\r\n",
               xdot3_rem_link_agg_entry.rem_link_agg_status,
               (long)xdot3_rem_link_agg_entry.rem_link_agg_port_id);

        xdot3_rem_max_frame_size_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xdot3_rem_max_frame_size_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXdot3RemMaxFrameSizeEntry(&xdot3_rem_max_frame_size_entry);
        BACKDOOR_MGR_Printf("rem_max_frame_size:%ld\r\n", (long)xdot3_rem_max_frame_size_entry.rem_max_frame_size);
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (LLDP_TYPE_MED == TRUE)
        xmed_rem_cap_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xmed_rem_cap_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXMedRemCapEntry(&xmed_rem_cap_entry);
        BACKDOOR_MGR_Printf("xmed_rem_cap_device_class:%d\r\nxmed_rem_cap_support:%d\r\nxmed_rem_cap_current:%d\r\n",
                xmed_rem_cap_entry.rem_device_class,
                xmed_rem_cap_entry.rem_cap_supported,
                xmed_rem_cap_entry.rem_cap_current);

        xmed_rem_med_policy_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xmed_rem_med_policy_entry.rem_index = rem_sys_data.rem_index;

        for (app_type = VAL_lldpXMedRemMediaPolicyAppType_voice;
             app_type <= LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE;
             app_type++)
        {
            xmed_rem_med_policy_entry.rem_app_type = app_type;
            if (LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex(&xmed_rem_med_policy_entry))
                BACKDOOR_MGR_Printf("xmed_rem_med_policy_app_type:%d\r\nxmed_rem_med_policy_rem_vid:%ld\r\nxmed_rem_med_policy_rem_priority:%d\r\nxmed_rem_med_policy_rem_dscp:%d\r\nxmed_rem_med_policy_rem_unknown:%d\r\nxmed_rem_med_policy_rem_tagged:%d\r\n",
                    app_type,
                    (long)xmed_rem_med_policy_entry.rem_vid,
                    xmed_rem_med_policy_entry.rem_priority,
                    xmed_rem_med_policy_entry.rem_dscp,
                    xmed_rem_med_policy_entry.rem_unknown,
                    xmed_rem_med_policy_entry.rem_tagged);
        }

        xmed_rem_poe_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xmed_rem_poe_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXMedRemPoeEntry(&xmed_rem_poe_entry);
        if(xmed_rem_poe_entry.rem_poe_device_type == 2)
        {
            BACKDOOR_MGR_Printf("rem_poe_device_type:pse\r\n");
            xmed_rem_poe_pse_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
            xmed_rem_poe_pse_entry.rem_index = rem_sys_data.rem_index;
            LLDP_OM_GetXMedRemPoePseEntry(&xmed_rem_poe_pse_entry);
            BACKDOOR_MGR_Printf("rem_pse_power_av:%d\r\nrem_pse_power_priority:%d\r\nrem_pse_power_source:%d\r\n",
                   xmed_rem_poe_pse_entry.rem_pse_power_av,
                   xmed_rem_poe_pse_entry.rem_pse_power_priority,
                   xmed_rem_poe_pse_entry.rem_pse_power_source);

        }
        else if (xmed_rem_poe_entry.rem_poe_device_type == 3)
        {
            BACKDOOR_MGR_Printf("rem_poe_device_type:pd\r\n");

            xmed_rem_pd_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
            xmed_rem_pd_entry.rem_index = rem_sys_data.rem_index;
            LLDP_OM_GetXMedRemPoePdEntry(&xmed_rem_pd_entry);
            BACKDOOR_MGR_Printf("rem_pd_power_req:%d\r\nrem_pd_power_priority:%d\r\nrem_pd_power_source:%d\r\n",
                   xmed_rem_pd_entry.rem_pd_power_req,
                   xmed_rem_pd_entry.rem_pd_power_priority,
                   xmed_rem_pd_entry.rem_pd_power_source);
        }
        else
            BACKDOOR_MGR_Printf("rem_poe_device_type:%d\r\n", xmed_rem_poe_entry.rem_poe_device_type);

        xmed_rem_loc_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xmed_rem_loc_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXMedRemLocationEntry(&xmed_rem_loc_entry);
        BACKDOOR_MGR_Printf("xmed_rem_loc_subtype:%d\r\n", xmed_rem_loc_entry.rem_location_subtype);
        BACKDOOR_MGR_Printf("xmed_rem_location:");
        for(i = 0; i < xmed_rem_loc_entry.rem_location_info_len; i++)
            BACKDOOR_MGR_Printf("%02X", xmed_rem_loc_entry.rem_location_info[i]);
        BACKDOOR_MGR_Printf("\r\n");

        xmed_rem_inv_entry.rem_local_port_num = rem_sys_data.rem_local_port_num;
        xmed_rem_inv_entry.rem_index = rem_sys_data.rem_index;
        LLDP_OM_GetXMedRemInventoryEntry(&xmed_rem_inv_entry);
        if(xmed_rem_inv_entry.rem_asset_id_len != 0)
        {
            BACKDOOR_MGR_Printf("xmed_rem_inv_asset_id:");
            for(i = 0; i < xmed_rem_inv_entry.rem_asset_id_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_asset_id[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_firmware_rev_len != 0)
        {
            BACKDOOR_MGR_Printf("xmed_rem_inv_firware_rev:");
            for(i = 0; i < xmed_rem_inv_entry.rem_firmware_rev_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_firmware_rev[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_hardware_rev_len != 0)
        {
            BACKDOOR_MGR_Printf("xmed_rem_hardware_rev:");
            for(i = 0; i < xmed_rem_inv_entry.rem_hardware_rev_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_hardware_rev[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_mfg_name_len != 0)
        {
            BACKDOOR_MGR_Printf("xmed_rem_mfg_name:");
            for(i = 0; i < xmed_rem_inv_entry.rem_mfg_name_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_mfg_name[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_model_name_len != 0)
        {
            BACKDOOR_MGR_Printf("xmed_rem_model_name:");
            for(i = 0; i < xmed_rem_inv_entry.rem_model_name_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_model_name[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_serial_num_len != 0)
        {
            BACKDOOR_MGR_Printf("rem_serial_num:");
            for(i = 0; i < xmed_rem_inv_entry.rem_serial_num_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_serial_num[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }

        if(xmed_rem_inv_entry.rem_software_rev_len != 0)
        {
            BACKDOOR_MGR_Printf("rem_software_rev:");
            for(i = 0; i < xmed_rem_inv_entry.rem_software_rev_len; i++)
                BACKDOOR_MGR_Printf("%02X", xmed_rem_inv_entry.rem_software_rev[i]);
            BACKDOOR_MGR_Printf("\r\n");
        }
#endif /* #if (LLDP_TYPE_MED == TRUE) */

    } /* end of while LLDP_MGR_GetNextRemoteSystemDataByIndex(&rem_sys_data) */

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_ShowRemManAddrByPort(UI32_T lport)
{
    LLDP_MGR_RemoteManagementAddrEntry_T rem_man_addr_entry;
    int i;

    memset(&rem_man_addr_entry, 0, sizeof(LLDP_MGR_RemoteManagementAddrEntry_T));
    rem_man_addr_entry.rem_local_port_num = lport;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    while(LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort(&rem_man_addr_entry))
    {
        BACKDOOR_MGR_Printf("\r\n-- Remote Management Address --\r\n");
        BACKDOOR_MGR_Printf("lport:%lu\r\n", (unsigned long)rem_man_addr_entry.rem_local_port_num);
        BACKDOOR_MGR_Printf("index:%lu\r\n", (unsigned long)rem_man_addr_entry.rem_index);
        BACKDOOR_MGR_Printf("rem_man_addr_subtype: %d\r\n", rem_man_addr_entry.rem_man_addr_subtype);
        BACKDOOR_MGR_Printf("rem_man_addr_len: %lu\r\n", (unsigned long)rem_man_addr_entry.rem_man_addr_len);
        BACKDOOR_MGR_Printf("rem_man_addr:");
        for(i = 0; i < rem_man_addr_entry.rem_man_addr_len-1; i++)
        {
            BACKDOOR_MGR_Printf("%02X", rem_man_addr_entry.rem_man_addr[i]);
        }
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("rem_man_addr_if_subtype:%d\r\n", rem_man_addr_entry.rem_man_addr_if_subtype);
        BACKDOOR_MGR_Printf("rem_man_addr_if_id:%lu\r\n", (unsigned long)rem_man_addr_entry.rem_man_addr_if_id);
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
}

static void LLDP_BACKDOOR_ShowLocLocationEntry(UI32_T lport)
{
#if (LLDP_TYPE_MED == TRUE)
    LLDP_MGR_XMedLocLocationEntry_T loc_location_entry;
    LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;
    BOOL_T ret;

    memset(&loc_location_entry, 0, sizeof(LLDP_MGR_XMedLocLocationEntry_T));
    memset(&ca_entry, 0, sizeof(LLDP_MGR_XMedLocationCivicAddrCaEntry_T));

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    loc_location_entry.lport = lport;
    loc_location_entry.location_subtype = VAL_lldpXMedLocLocationSubtype_civicAddress;
    if (LLDP_MGR_GetXMedLocLocationEntry(&loc_location_entry) == TRUE)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("Location Data Format: Civic Address LCI\r\n");
        BACKDOOR_MGR_Printf("What: %hu\r\n", loc_location_entry.location_info[1]);
        BACKDOOR_MGR_Printf("Country Code: %c%c\r\n",
            loc_location_entry.location_info[2],
            loc_location_entry.location_info[3]);
        for (ret = LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry(lport, &ca_entry);
             ret;
             ret = LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry(lport, &ca_entry))
        {
            BACKDOOR_MGR_Printf("CA Type %hu: %.*s\r\n", ca_entry.ca_type,
                ca_entry.ca_length, ca_entry.ca_value);
        }
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
#endif
}

static void LLDP_BACKDOOR_SetLocLocationEntry(UI32_T lport)
{
#if (LLDP_TYPE_MED == TRUE)
    LLDP_MGR_XMedLocLocationEntry_T loc_location_entry;
    UI8_T *ca = 0;
    UI8_T lci_length = 0;

    memset(&loc_location_entry, 0, sizeof(LLDP_MGR_XMedLocLocationEntry_T));
    loc_location_entry.lport = lport;
    loc_location_entry.location_subtype = VAL_lldpXMedLocLocationSubtype_civicAddress;
    loc_location_entry.location_info[1] = 2; /* what */
    loc_location_entry.location_info[2] = 'T';
    loc_location_entry.location_info[3] = 'W';

    lci_length = 3;
    ca = &loc_location_entry.location_info[4];

    /* national */
    ca[0] = 1;
    ca[1] = strlen("Taiwan");
    memcpy(&ca[2], "Taiwan", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* county */
    ca[0] = 2;
    ca[1] = strlen("Hsinchu");
    memcpy(&ca[2], "Hsinchu", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* district */
    ca[0] = 4;
    ca[1] = strlen("Science-based industrial park");
    memcpy(&ca[2], "Science-based industrial park", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* street */
    ca[0] = 6;
    ca[1] = strlen("Creation Rd. III");
    memcpy(&ca[2], "Creation Rd. III", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* house number */
    ca[0] = 19;
    ca[1] = 1;
    ca[2] = '1';
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* name */
    ca[0] = 23;
    ca[1] = strlen("Accton Technology Corporation");
    memcpy(&ca[2], "Accton Technology Corporation", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* unit */
    ca[0] = 26;
    ca[1] = strlen("R&D");
    memcpy(&ca[2], "R&D", ca[1]);
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    /* floor */
    ca[0] = 27;
    ca[1] = 1;
    ca[2] = '2';
    lci_length += (2 + ca[1]);
    ca += (2 + ca[1]);

    loc_location_entry.location_info[0] = lci_length;
    loc_location_entry.location_info_len = lci_length + 1;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    LLDP_MGR_SetXMedLocLocationEntry(&loc_location_entry);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
#endif
}

static void LLDP_BACKDOOR_SetLocLocatoinCountry(UI32_T lport)
{
#if (LLDP_TYPE_MED == TRUE)
    UI8_T country_code[] = "US";

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode(lport, country_code);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
#endif
}

static void LLDP_BACKDOOR_SetLocLocationWhat(UI32_T lport)
{
#if (LLDP_TYPE_MED == TRUE)
    UI8_T what = 3;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    LLDP_MGR_SetXMedLocLocationCivicAddrWhat(lport, what);

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
#endif
}

static void LLDP_BACKDOOR_SetLocLocationCaEntry(UI32_T lport)
{
#if (LLDP_TYPE_MED == TRUE)
    int i;
    LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;

    /* Get execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);

    for(i = 0; i < 10; i ++)
    {
        memset(&ca_entry, 0, sizeof(LLDP_MGR_XMedLocationCivicAddrCaEntry_T));
        ca_entry.ca_type = i;
        ca_entry.ca_length = strlen("test");
        memcpy(ca_entry.ca_value, "test", ca_entry.ca_length);
        LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry(lport, &ca_entry, TRUE);
    }

    /* Release execution permission from the thread group handler if necessary
     */
    L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
#endif
}
