/*-----------------------------------------------------------------------------
 * Module Name: lldp_mgr.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementatinos for the LLDP API
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
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "leaf_1213.h"
#include "leaf_2737.h"
#include "leaf_ieeelldp.h"
#include "leaf_ieeelldpext.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "l_cvrt.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "sysfun.h"
#if (SYS_CPNT_DOT1X == TRUE)
#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_mgr.h"
#else
#include "1x_om.h"
#endif
#endif
#include "backdoor_mgr.h"
#include "gvrp_group.h"
#include "if_pmgr.h"
#include "lldp_backdoor.h"
#include "lldp_engine.h"
#include "lldp_mgr.h"
#include "lldp_om_private.h"
#include "lldp_type.h"
#include "lldp_uty.h"
#include "mib2_pom.h"
#include "netcfg_pom_ip.h" /*to remove warning about implicit declaration of functions in such head file,and add the file route to makefile.am*/
#include "netcfg_type.h"
#if (SYS_CPNT_POE == TRUE)
#include "poe_pom.h"
#endif
#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
#include "pri_pmgr.h"
#endif
#include "snmp_pmgr.h"
#include "stktplg_om.h"
#include "swctrl.h"
#include "sys_callback_mgr.h"
#include "sys_time.h"
#include "trap_event.h"
#include "vlan_lib.h"
#include "vlan_om.h"
#if (SYS_CPNT_ADD == TRUE)
#include "add_om.h"
#endif
#include "syslog_pmgr.h"

static SYS_TYPE_CallBack_T  *LLDP_MGR_NotifyTelephoneDetect_CallBackFuncList = 0;
static BOOL_T                  provision_completed = FALSE;
static BOOL_T LLDP_MGR_GetRemData_Local(UI32_T lport, UI32_T index, LLDP_OM_RemData_T** ptr);
static BOOL_T LLDP_MGR_GetNextRemData_Local(UI32_T lport, UI32_T index, LLDP_OM_RemData_T **get_data);
#if (LLDP_TYPE_MED == TRUE)
static BOOL_T LLDP_MGR_InitXMedLocLocationEntry(UI32_T location_type, LLDP_OM_XMedLocLocationEntry_T *lldp_med_location, BOOL_T force);
#endif
static L_INET_AddrIp_T LLDP_MGR_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen)
{
    L_INET_AddrIp_T addr;
    addr.type = type;
    if(L_INET_ADDR_TYPE_IPV4 == type)
        addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    else if(L_INET_ADDR_TYPE_IPV6 == type)
        addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    else{printf ("Wooops! something wring!\r\n");}
    memcpy(addr.addr,addrp,addr.addrlen);
    addr.preflen = prefixLen;
    return addr;
}
/* an array of bit masks in SNMP bit order (*MSBit first) */
static char bitstring_mask[] = { 1<<7, 1<<6, 1<<5, 1<<4, 1<<3, 1<<2, 1<<1, 1<<0 };

void
lldp_bitstring_setbit(char *bstring, int number)
{
        bstring += number / 8;
        *bstring |= bitstring_mask[(number % 8)];
}

void
lldp_bitstring_clearbit(char *bstring, int number)
{
        bstring += number / 8;
        *bstring &= ~bitstring_mask[(number % 8)];
}
int
lldp_bitstring_testbit(char *bstring, int number)
{
        bstring += number / 8;
        return ((*bstring & bitstring_mask[(number % 8)]) != 0);
}

SYSFUN_DECLARE_CSC


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProcessNotification
 *-------------------------------------------------------------------------
 * PURPOSE  : Check if the notification is necessary
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static void LLDP_MGR_ProcessNotification()
{
#if (SYS_CPNT_ADD == TRUE)
    LLDP_TYPE_NotifyTelephone_T         notify_tel_entry;
#endif

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
    LLDP_TYPE_NotifyDot3atInfo_T        *notify_dot3at_info_entry;
#endif

#if (SYS_CPNT_DCBX == TRUE)
    LLDP_TYPE_NotifyDcbxEtsInfo_T       notify_ets_entry;
    LLDP_TYPE_NotifyDcbxPfcInfo_T       notify_pfc_entry;
#endif

#if (SYS_CPNT_CN == TRUE)
    LLDP_TYPE_NotifyCnRemoteChange_T    notify_cn_entry;
#endif

#if (SYS_CPNT_SNMP == TRUE)
    UI32_T current_time ;
    LLDP_OM_SysConfigEntry_T            *sys_config;
    LLDP_OM_Statistics_T                *sys_statistics;
    TRAP_EVENT_TrapData_T               trap_data;

    #if (LLDP_TYPE_MED == TRUE)
    LLDP_TYPE_NotifyMedRemData_T        *notify_med_entry;
    #endif
#endif

#if (((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) || (LLDP_TYPE_MED == TRUE))
    L_SORT_LST_List_T                   *notify_sort_list;
#endif
    L_LINK_LST_List_T                   *notify_link_list;

    LLDP_TYPE_NotifyRemChange_T         notify_rem_change_entry;
    SWCTRL_Lport_Type_T                 port_type;
    UI32_T                              unit, port, trunk_id;
    char                                port_string[10];
    SYSLOG_OM_RecordOwnerInfo_T         owner_info;

#if (SYS_CPNT_ADD == TRUE)
    /* notify device type */
    LLDP_OM_EnterCriticalSection();
    notify_link_list = LLDP_OM_GetNotifyTelephoneTypeListPtr();
    while(L_LINK_LST_Remove_1st(notify_link_list, &notify_tel_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        GVRP_GROUP_TelephoneDetectCallbackHandler(
            notify_tel_entry.lport, notify_tel_entry.mac_addr,
            notify_tel_entry.network_addr_subtype,
            notify_tel_entry.network_addr, notify_tel_entry.network_addr_len,
            notify_tel_entry.network_addr_ifindex, notify_tel_entry.tel_exist);
        SYS_CALLBACK_MGR_TelephoneDetectCallback(SYS_MODULE_LLDP,
            notify_tel_entry.lport, notify_tel_entry.mac_addr,
            notify_tel_entry.network_addr_subtype,
            notify_tel_entry.network_addr, notify_tel_entry.network_addr_len,
            notify_tel_entry.network_addr_ifindex, notify_tel_entry.tel_exist);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();
#endif

#if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2))
    /* notify device type */
    LLDP_OM_EnterCriticalSection();
    notify_sort_list = LLDP_OM_GetNotifyDot3atInfoReceivedListPtr();
    while(L_SORT_LST_Remove_1st(notify_sort_list, &notify_dot3at_info_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        if (SWCTRL_LogicalPortToUserPort(notify_dot3at_info_entry->lport, &unit,
                &port, &trunk_id) != SWCTRL_LPORT_NORMAL_PORT)
        {
            free(notify_dot3at_info_entry);
            continue;
        }

        SYS_CALLBACK_MGR_Dot3atInfoReceivedCallback(SYS_MODULE_LLDP, unit, port,
            notify_dot3at_info_entry->power_type,
            notify_dot3at_info_entry->power_source,
            notify_dot3at_info_entry->power_priority,
            notify_dot3at_info_entry->pd_requested_power,
            notify_dot3at_info_entry->pse_allocated_power);

        free(notify_dot3at_info_entry);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();
#endif /* #if ((SYS_CPNT_POE == TRUE) && (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)) */

#if(SYS_CPNT_DCBX == TRUE)
    /* notify ETS */
    LLDP_OM_EnterCriticalSection();
    notify_link_list = LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr();
    while(L_LINK_LST_Remove_1st(notify_link_list, &notify_ets_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        SYS_CALLBACK_MGR_ReceiveDcbxEtsTlvCallback(SYS_MODULE_LLDP,
            notify_ets_entry.lport, notify_ets_entry.is_delete,
            notify_ets_entry.rem_recommend_rcvd,
            notify_ets_entry.rem_config_willing,
            notify_ets_entry.rem_config_cbs,
            notify_ets_entry.rem_config_max_tc,
            notify_ets_entry.rem_config_pri_assign_table,
            notify_ets_entry.rem_config_tc_bandwidth_table,
            notify_ets_entry.rem_config_tsa_assign_table,
            notify_ets_entry.rem_recommend_pri_assign_table,
            notify_ets_entry.rem_recommend_tc_bandwidth_table,
            notify_ets_entry.rem_recommend_tsa_assign_table);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();

    /* notify PFC */
    LLDP_OM_EnterCriticalSection();
    notify_link_list = LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr();
    while(L_LINK_LST_Remove_1st(notify_link_list, &notify_pfc_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        SYS_CALLBACK_MGR_ReceiveDcbxPfcTlvCallback(SYS_MODULE_LLDP,
            notify_pfc_entry.lport, notify_pfc_entry.is_delete,
            notify_pfc_entry.rem_mac, notify_pfc_entry.rem_willing,
            notify_pfc_entry.rem_mbc, notify_pfc_entry.rem_cap,
            notify_pfc_entry.rem_enable);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();
#endif

#if (SYS_CPNT_CN == TRUE)
    LLDP_OM_EnterCriticalSection();
    notify_link_list = LLDP_OM_GetNotifyCnRemoteChangeListPtr();
    while(L_LINK_LST_Remove_1st(notify_link_list, &notify_cn_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        SYS_CALLBACK_MGR_CnRemoteChangeCallback(SYS_MODULE_LLDP,
            notify_cn_entry.lport, notify_cn_entry.neighbor_num,
            notify_cn_entry.cnpv_indicators, notify_cn_entry.ready_indicators);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();
#endif

#if (SYS_CPNT_SNMP == TRUE)
    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    sys_statistics = LLDP_OM_GetStatisticsPtr();

    if(sys_config->something_changed_remote)
    {
        /* Enter critical section */
        LLDP_OM_EnterCriticalSection();
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        if(sys_config->notification_timer < current_time)
        {
            sys_config->notification_timer = current_time + sys_config->notification_interval * LLDP_TYPE_TIME_UNIT;
            sys_config->something_changed_remote = FALSE;
        }
        /* Leave critical section */
        LLDP_OM_LeaveCriticalSection();

        if (sys_config->something_changed_remote == FALSE)
        {
            /* Prepare data to Send Trap */
            memset(&trap_data, 0, sizeof(trap_data));
            trap_data.community_specified = FALSE;
            trap_data.trap_type = TRAP_EVENT_LLDP_REM_TABLES_CHANGED;
            trap_data.u.lldp_rem_table_changed.lldpStatsRemTablesAgeouts = sys_statistics->rem_table_ageouts;
            trap_data.u.lldp_rem_table_changed.lldpStatsRemTablesDeletes = sys_statistics->rem_table_deletes;
            trap_data.u.lldp_rem_table_changed.lldpStatsRemTablesDrops = sys_statistics->rem_table_drops;
            trap_data.u.lldp_rem_table_changed.lldpStatsRemTablesInserts = sys_statistics->rem_table_inserts;
            /* Send to Trap manger*/
            SNMP_PMGR_ReqSendTrap(&trap_data);

#if (LLDP_TYPE_MED == TRUE)
            /* send med trap */
            LLDP_OM_EnterCriticalSection();
            notify_sort_list = LLDP_OM_GetNotifyMedRemTableChangedList();
            while(L_SORT_LST_Remove_1st(notify_sort_list, &notify_med_entry))
            {
                LLDP_OM_LeaveCriticalSection();
                /* prepare med trap data */
                memset(&trap_data, 0, sizeof(trap_data));
                trap_data.community_specified = FALSE;
                trap_data.trap_type = TRAP_EVENT_LLDP_MED_TOPOLOGY_CHANGE_DETECTED;
                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisIdSubtype[0] = notify_med_entry->rem_time_mark;
                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisIdSubtype[1] = notify_med_entry->rem_local_port_num;
                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisIdSubtype[2] = notify_med_entry->rem_index;
                trap_data.u.lldp_med_topology_change.lldpRemChassisIdSubtype = notify_med_entry->rem_chassis_id_subtype;

                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisId[0] = notify_med_entry->rem_time_mark;
                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisId[1] = notify_med_entry->rem_local_port_num;
                trap_data.u.lldp_med_topology_change.instance_lldpRemChassisId[2] = notify_med_entry->rem_index;
                trap_data.u.lldp_med_topology_change.lldpRemChassisId[0] = (UI8_T)notify_med_entry->rem_chassis_id_len; /* array index 0 store the len */
                memcpy(&(trap_data.u.lldp_med_topology_change.lldpRemChassisId[1]), notify_med_entry->rem_chassis_id, notify_med_entry->rem_chassis_id_len);

                trap_data.u.lldp_med_topology_change.instance_lldpMedRemDeviceClass[0] = notify_med_entry->rem_time_mark;
                trap_data.u.lldp_med_topology_change.instance_lldpMedRemDeviceClass[1] = notify_med_entry->rem_local_port_num;
                trap_data.u.lldp_med_topology_change.instance_lldpMedRemDeviceClass[2] = notify_med_entry->rem_index;
                trap_data.u.lldp_med_topology_change.lldpMedRemDeviceClass = notify_med_entry->rem_device_class;

                SNMP_PMGR_ReqSendTrap(&trap_data);
                free(notify_med_entry);
                LLDP_OM_EnterCriticalSection();
            }
            LLDP_OM_LeaveCriticalSection();
#endif
        }
    }
#endif /* #if (SYS_CPNT_SNMP == TRUE) */

    LLDP_OM_EnterCriticalSection();
    notify_link_list = LLDP_OM_GetNotifyRemChangeListPtr();
    memset(port_string, 0, sizeof(port_string));
    while(L_LINK_LST_Remove_1st(notify_link_list, &notify_rem_change_entry))
    {
        LLDP_OM_LeaveCriticalSection();
        port_type = SWCTRL_LogicalPortToUserPort(notify_rem_change_entry.lport,
                        &unit, &port, &trunk_id);

        if (port_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            SYSFUN_Snprintf(port_string, 9, "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
        }
        else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
        {
            SYSFUN_Snprintf(port_string, 9, "Trunk %lu", (unsigned long)trunk_id);
        }
        else
        {
            SYSFUN_Snprintf(port_string, 9, "unknown");
        }

        owner_info.level = SYSLOG_LEVEL_INFO;
        owner_info.module_no = SYS_MODULE_LLDP;
        owner_info.function_no = LLDP_TYPE_LOG_FUN_NO;
        owner_info.error_no = LLDP_TYPE_LOG_ERR_NO;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info,
            LLDP_REMOTE_TABLE_CHANGED_PER_PORT_MESSAGE_INDEX,
            port_string, NULL, NULL);
        LLDP_OM_EnterCriticalSection();
    }
    LLDP_OM_LeaveCriticalSection();

    return;
}

#define LLDP_MGR_MED_COUNTRY_CODE_SIZE 2
static BOOL_T LLDP_MGR_IsCountryCodeValid(UI8_T *country_code_ar)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLE DECLARATIONS
     */

    UI32_T  i;

    /* BODY
     */

    if (country_code_ar == NULL)
    {
        return FALSE;
    }

    for (i = 0; i  < LLDP_MGR_MED_COUNTRY_CODE_SIZE; i++)
    {
        if ((country_code_ar[i] < 'A') || (country_code_ar[i] > 'Z'))
        {
            return FALSE;
        }
    }

    return TRUE;
} /* End of LLDP_MGR_IsCountryCodeValid */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetSysAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP global admin status
 * INPUT    : admin_status
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetSysAdminStatus(UI32_T admin_status)
{
    LLDP_OM_SysConfigEntry_T    *sys_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    if(sys_config->global_admin_status != admin_status)
    {
        if(admin_status == LLDP_TYPE_SYS_ADMIN_DISABLE)
        {
            UI32_T  lport = 0;
            LLDP_OM_PortConfigEntry_T   *port_config;
            while(LLDP_UTY_GetNextLogicalPort(&lport))
            {
                port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
                if(port_config->admin_status == VAL_lldpPortConfigAdminStatus_txOnly ||
                   port_config->admin_status == VAL_lldpPortConfigAdminStatus_txAndRx)
                {
                    LLDP_OM_LeaveCriticalSection();
                    LLDP_ENGINE_ConstructLLDPDU(lport, LLDP_TYPE_SHUTDOWN_LLDPDU);
                    LLDP_OM_EnterCriticalSection();
                }
            }

            LLDP_OM_ResetAll();
        }

        sys_config->global_admin_status = admin_status;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetMsgTxInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_tx_interval to determine interval at which LLDP frames are transmitted
 * INPUT    : UI32_T msg_tx_interval    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 5~32768
 *            Default value: 30 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetMsgTxInterval(UI32_T msg_tx_interval)
{
    UI32_T                      result;
    LLDP_OM_SysConfigEntry_T    *om_sys_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* check range */
    if( (msg_tx_interval < LLDP_TYPE_MIN_TX_INTERVAL) || (msg_tx_interval > LLDP_TYPE_MAX_TX_INTERVAL) )
    {
        /* need to define SYS_MODULE_LLDP for Exception Handler*/
        /* EH_MGR_Handle_Exception1(SYS_MODULE_LLDP, LLDP_MGR_SetMsgTxInterval_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "TxInterval(5-32678)");*/

        return LLDP_TYPE_RETURN_ERROR;
    }

    result      = LLDP_TYPE_RETURN_ERROR;

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get system config entry pointer from om */
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    if (om_sys_config_ptr->msg_tx_interval == msg_tx_interval)
    {
        result = LLDP_TYPE_RETURN_OK;
    }
    else
    {
        if ((om_sys_config_ptr->tx_delay * 4) <= msg_tx_interval)
        {
            om_sys_config_ptr->msg_tx_interval = msg_tx_interval;
            LLDP_ENGINE_SomethingChangedLocal(0);
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return result;
} /* End of LLDP_MGR_SetMsgTxInterval*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetMsgTxHoldMul
 *-------------------------------------------------------------------------
 * PURPOSE  : Set msg_hold time multiplier to determine the actual TTL value used in an LLDPDU.
 * INPUT    : UI32_T msg_tx_hold_mul      --  a multiplier on the msgTxInterval
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 2~10
 *            Default value: 4.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetMsgTxHoldMul(UI32_T msg_tx_hold_mul)
{
    LLDP_OM_SysConfigEntry_T    *om_sys_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* check range */
    if ((msg_tx_hold_mul < LLDP_TYPE_MIN_TX_HOLD_MUL) ||
        (msg_tx_hold_mul > LLDP_TYPE_MAX_TX_HOLD_MUL))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get system config entry pointer from om */
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    if (om_sys_config_ptr->msg_tx_hold_mul != msg_tx_hold_mul)
    {
        om_sys_config_ptr->msg_tx_hold_mul = msg_tx_hold_mul;
        LLDP_ENGINE_SomethingChangedLocal(0);
    }

    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
} /* End of SetMsgTxHoldMul */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the amount of delay from when adminStatus becomes disabled
 *            until re-initialization will be attempted.
 * INPUT    : UI32_T reinit_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~10
 *            Default value: 2 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetReinitDelay(UI32_T reinit_delay)
{
    LLDP_OM_SysConfigEntry_T    *om_sys_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* check range */
    if ((reinit_delay < LLDP_TYPE_MAX_REINIT_DELAY) ||
        (reinit_delay > LLDP_TYPE_MIN_REINIT_DELAY))
    {
        /* Exception handler */
        /* EH_MGR_Handle_Exception1(SYS_MODULE_LLDP, LLDP_MGR_SetReinitDelay_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "ReInitDelay(1-10)"); */
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get system config entry pointer from om */
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    /* if value is changed, set it */
    if (om_sys_config_ptr->reinit_delay != reinit_delay)
    {
        om_sys_config_ptr->reinit_delay = reinit_delay;
        LLDP_ENGINE_SomethingChangedLocal(0);
    }

    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
} /* End of LLDP_MGR_SetReinitDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetTxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the minimum delay between successive LLDP frame transmissions.
 * INPUT    : UI32_T tx_delay    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.3.3, IEEE Std 802.1AB-2005.
 *            Range: 1~8192
 *            Default value: 2 seconds.
 *            The recommended value is set by the following formula:
 *              1 <= lldpTxDelay <= (0.25 * lldpMessageTxInterval)
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetTxDelay(UI32_T tx_delay)
{
    UI32_T                      result;
    UI32_T                      changed;
    UI32_T                      current_time;
    UI32_T                      current_tx_delay;
    UI32_T                      lport = 0;
    LLDP_OM_SysConfigEntry_T    *om_sys_config_ptr;
    LLDP_OM_PortConfigEntry_T   *port_config = NULL;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    /* check range */
    if( (tx_delay < LLDP_TYPE_MIN_TX_DELAY) || (tx_delay > LLDP_TYPE_MAX_TX_DELAY) )
    {
        /* Exception handler */
        /* EH_MGR_Handle_Exception1(SYS_MODULE_LLDP, LLDP_MGR_SetMsgTxHoldMul_Fun_No, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "TxHoldMul(2-10)"); */
        return LLDP_TYPE_RETURN_ERROR;
    }

    changed = LLDP_TYPE_MGR_CHANGED_FALSE;

    /* +++ Enter critical region +++ */
    result              = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_EnterCriticalSection();

    /* get system config entry pointer from om */
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    current_tx_delay = om_sys_config_ptr->tx_delay;
    if (current_tx_delay == tx_delay)
    {
        result = LLDP_TYPE_RETURN_OK;
    }
    else /* if value changed */
    {
        UI32_T  tx_interval;
        tx_interval = om_sys_config_ptr->msg_tx_interval;

        /* check dependency: 1 <= tx_delay <= (0.25 * txInterval) */
        if(( LLDP_MGR_IsProvisionComplete() == TRUE) && (4 * tx_delay) > tx_interval )
        {
            /* Exception handler */
            /* EH_MGR_Handle_Exception1(SYS_MODULE_LLDP, LLDP_MGR_SetTxDelay_Fun_No, EH_TYPE_MSG_INVALID_VALUE, SYSLOG_LEVEL_INFO, "4* tx_delay < txInterval"); */
            result = LLDP_TYPE_RETURN_TX_DELAY_ERROR;
        }
        else
        {
            om_sys_config_ptr->tx_delay = tx_delay;
            result = LLDP_TYPE_RETURN_OK;
            changed = LLDP_TYPE_MGR_CHANGED_TRUE;
        }
    }

    if(changed == LLDP_TYPE_MGR_CHANGED_TRUE)
    {
        LLDP_ENGINE_SomethingChangedLocal(0);
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        while(LLDP_UTY_GetNextLogicalPort(&lport))
        {
            port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
            port_config->tx_delay_timer = current_time + (om_sys_config_ptr->tx_delay * LLDP_TYPE_TIME_UNIT);
        }
    }
    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetTxDelay */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetNotificationInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set this value to control the transmission of LLDP notifications.
 * INPUT    : UI32_T interval_time    --  time value
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Range: 5~3600
 *            Default value: 5 seconds.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetNotificationInterval(UI32_T notify_interval_time)
{
    LLDP_OM_SysConfigEntry_T    *om_sys_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* check range */
    if( (notify_interval_time < LLDP_TYPE_MIN_NOTIFY_INTERVAL) || (notify_interval_time > LLDP_TYPE_MAX_NOTIFY_INTERVAL) )
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get system config entry pointer from om*/
    om_sys_config_ptr = LLDP_OM_GetSysConfigEntryPtr();

    /* if value is changed, set it */
    if (om_sys_config_ptr->notification_interval != notify_interval_time)
    {
        om_sys_config_ptr->notification_interval = notify_interval_time;
        LLDP_ENGINE_SomethingChangedLocal(0);
    }

    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
} /* End of LLDP_MGR_SetNotificationInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get configuration entry info. for specified port.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *om_port_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get port config entry pointer from om*/
    if(LLDP_UTY_LogicalPortExisting(port_config_entry->port_num))
    {
        om_port_config_ptr = LLDP_OM_GetPortConfigEntryPtr(port_config_entry->port_num);
        port_config_entry->admin_status        = om_port_config_ptr->admin_status;
        port_config_entry->notification_enable = om_port_config_ptr->notification_enable;
        port_config_entry->basic_tlvs_tx_flag = om_port_config_ptr->basic_tlvs_tx_flag;
        result = LLDP_TYPE_RETURN_OK;
    }


    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetPortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next configuration entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * OUTPUT   : LLDP_MGR_PortConfigEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortConfigEntry(LLDP_MGR_PortConfigEntry_T *port_config_entry)
{
    LLDP_OM_PortConfigEntry_T   *om_port_config_ptr;
    UI32_T                      result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_UTY_GetNextLogicalPort(&port_config_entry->port_num))
    {
        om_port_config_ptr = LLDP_OM_GetPortConfigEntryPtr(port_config_entry->port_num);
        port_config_entry->admin_status        = om_port_config_ptr->admin_status;
        port_config_entry->notification_enable = om_port_config_ptr->notification_enable;
        port_config_entry->basic_tlvs_tx_flag = om_port_config_ptr->basic_tlvs_tx_flag;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextPortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetConfigManAddrEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : enable
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            This function is used by WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetConfigManAddrTlv(UI32_T lport, UI8_T *enabled)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        *enabled = (UI8_T)port_config->man_addr_transfer_flag;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortConfigAdminStatus
 *-------------------------------------------------------------------------
 * PURPOSE  :  Set admin_status to control whether or not a local LLDP agent is
 *             enabled (transmit and receive, transmit only, or receive only)
 *             or is disabled.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T admin_status     -- status vaule:
 *                                       txOnly(1),
 *                                       rxOnly(2),
 *                                       txAndRx(3),
 *                                       disabled(4)
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.1, IEEE Std 802.1AB-2005.
 *            Default value: txAndRx(3).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortConfigAdminStatus(UI32_T lport, UI32_T modify_admin_status)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *om_port_config_entry;
    UI32_T                      current_admin_status;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++ Enter critical region +++*/
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;

    /* get port config entry pointer from om*/
    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        om_port_config_entry = LLDP_OM_GetPortConfigEntryPtr(lport);

        current_admin_status = om_port_config_entry->admin_status;

        /* if admin_status is not change, return directly*/
        if(modify_admin_status == current_admin_status)
        {
            LLDP_OM_LeaveCriticalSection();
            return LLDP_TYPE_RETURN_OK;
        }

        /* if value are different */
        switch(modify_admin_status)
        {
            case VAL_lldpPortConfigAdminStatus_disabled:
                switch(current_admin_status)
                {
                    case VAL_lldpPortConfigAdminStatus_txAndRx:
                        LLDP_ENGINE_AdminStatusDisableRx(lport);
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusDisableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        break;
                    case VAL_lldpPortConfigAdminStatus_rxOnly:
                        LLDP_ENGINE_AdminStatusDisableRx(lport);
                        break;
                    case VAL_lldpPortConfigAdminStatus_txOnly:
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusDisableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        break;
                    default:
                        break;
                }
                break;
            case VAL_lldpPortConfigAdminStatus_txAndRx :
                switch(current_admin_status)
                {
                    case VAL_lldpPortConfigAdminStatus_disabled:
                        LLDP_ENGINE_AdminStatusEnableRx(lport);
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusEnableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        LLDP_OM_ResetPortStatistics(lport);
                        break;
                    case VAL_lldpPortConfigAdminStatus_rxOnly:
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusEnableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        break;
                    case VAL_lldpPortConfigAdminStatus_txOnly:
                        LLDP_ENGINE_AdminStatusEnableRx(lport);
                        break;
                    default:
                        break;
                }
                break;
            case VAL_lldpPortConfigAdminStatus_rxOnly :
                switch(current_admin_status)
                {
                    case VAL_lldpPortConfigAdminStatus_txAndRx:
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusDisableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        break;
                    case VAL_lldpPortConfigAdminStatus_disabled:
                        LLDP_ENGINE_AdminStatusEnableRx(lport);
                        LLDP_OM_ResetPortStatistics(lport);
                        break;
                    case VAL_lldpPortConfigAdminStatus_txOnly:
                        LLDP_ENGINE_AdminStatusEnableRx(lport);
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusDisableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        break;
                    default:
                        break;
                }
                break;
            case VAL_lldpPortConfigAdminStatus_txOnly :
                switch(current_admin_status)
                {
                    case VAL_lldpPortConfigAdminStatus_txAndRx:
                        LLDP_ENGINE_AdminStatusDisableRx(lport);
                        break;
                    case VAL_lldpPortConfigAdminStatus_rxOnly:
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusEnableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        LLDP_ENGINE_AdminStatusDisableRx(lport);
                        break;
                    case VAL_lldpPortConfigAdminStatus_disabled:
                        LLDP_OM_LeaveCriticalSection();
                        LLDP_ENGINE_AdminStatusEnableTx(lport);
                        LLDP_OM_EnterCriticalSection();
                        LLDP_OM_ResetPortStatistics(lport);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++ Leave critical region +++*/
    LLDP_OM_LeaveCriticalSection();

    return result;
} /* End of LLDP_MGR_SetPortConfigAdminStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortConfigNotificationEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : The value true(1) means that notifications are enabled;
 *            The value false(2) means that they are not.
 * INPUT    : UI32_T lport            -- lport number
 *            UI32_T status           -- status vaule:
 *                                       true(1),
 *                                       false(0),
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *            Default value: false(2).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortConfigNotificationEnable(UI32_T lport, BOOL_T status)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *om_port_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        om_port_config_ptr = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (om_port_config_ptr->notification_enable != status)
        {
            om_port_config_ptr->notification_enable = status;
            om_port_config_ptr->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetPortConfigNotificationEnable*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetPortOptionalTlvStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : UI32_T lport            -- lport number
 *            UI8_T tlv_status        -- bitmap:
 *                                       BIT_0: Port Description TLV,
 *                                       BIT_1: System Name TLV,
 *                                       BIT_2: System Description TLV,
 *                                       BIT_3: System Capabilities TLV,
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.2.1.1, IEEE Std 802.1AB-2005.
 *            Default value: no bit on (empty set).
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetPortOptionalTlvStatus(UI32_T lport, UI8_T tlv_status)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *om_port_config_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get port config entry pointer from om */
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        om_port_config_ptr = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (om_port_config_ptr->basic_tlvs_tx_flag != tlv_status)
        {
            om_port_config_ptr->basic_tlvs_tx_flag = tlv_status;
            om_port_config_ptr->something_changed_local = TRUE;

        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetPortOptionalTlvStatus */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetConfigManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : lport                 -- specified port
 *            transfer_enable       -- flag to enable/disable transfer
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetConfigManAddrTlv(UI32_T lport, BOOL_T transfer_enable)
{
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    if (port_config->man_addr_transfer_flag != transfer_enable)
    {
        port_config->man_addr_transfer_flag = transfer_enable;
        port_config->something_changed_local = TRUE;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetConfigAllPortManAddrTlv
 *-------------------------------------------------------------------------
 * PURPOSE  : Set optional TLVs status that may be included in LLDPDU.
 * INPUT    : port_list       -- specified port list with size equal to
 *                               SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 *            transfer_enable -- flag to enable/disable transfer
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetConfigAllPortManAddrTlv(UI8_T *port_list)
{
    LLDP_OM_PortConfigEntry_T   *port_config;
    UI32_T                      i, j, ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        for (j = 0; j < 8; j++)
        {
            ifindex = 8 * i + (j + 1);
            if (ifindex > SYS_ADPT_TOTAL_NBR_OF_LPORT)
                continue;

            port_config = LLDP_OM_GetPortConfigEntryPtr(ifindex);
            if ((port_list[i] << j) & 0x80)
            {
                /* in the portlist, enable */
                if(port_config->man_addr_transfer_flag != TRUE)
                {
                    port_config->man_addr_transfer_flag = TRUE;
                    port_config->something_changed_local = TRUE;
                }
            }
            else
            {
                /* not in the portlist, disable */
                if(port_config->man_addr_transfer_flag != FALSE)
                {
                    port_config->man_addr_transfer_flag = FALSE;
                    port_config->something_changed_local = TRUE;
                }
            }
        }
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetSysStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info.
 * INPUT    : LLDP_MGR_Statistics_T *statistics_entry
 * OUTPUT   : LLDP_MGR_Statistics_T *statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetSysStatisticsEntry(LLDP_MGR_Statistics_T *statistics_entry)
{
    UI32_T                      result;
    LLDP_OM_Statistics_T        *om_statistics_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get statistics entry pointer from om*/
    om_statistics_ptr = LLDP_OM_GetStatisticsPtr();
    {
        memcpy(statistics_entry, om_statistics_ptr, sizeof(LLDP_MGR_Statistics_T));
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetSysStatisticsEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get tx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortTxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry)
{
    UI32_T                     result;

    LLDP_OM_PortStatistics_T   *om_port_statistics_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get port statistics entry pointer from om*/
    if(LLDP_UTY_LogicalPortExisting(port_statistics_entry->port_num))
    {
        om_port_statistics_ptr = LLDP_OM_GetPortStatisticsEntryPtr(port_statistics_entry->port_num);
        {
            port_statistics_entry->tx_frames_total = om_port_statistics_ptr->tx_frames_total;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetPortTxStatisticsEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortTxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortTxStatisticsEntry(LLDP_MGR_PortTxStatistics_T *port_statistics_entry)
{
    UI32_T                  result;
    LLDP_OM_PortStatistics_T    *port_statistics;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();
    if(LLDP_UTY_GetNextLogicalPort(&port_statistics_entry->port_num))
    {

        port_statistics = LLDP_OM_GetPortStatisticsEntryPtr(port_statistics_entry->port_num);

        port_statistics_entry->tx_frames_total = port_statistics->tx_frames_total;
        result = LLDP_TYPE_RETURN_OK;
    }
    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextPortTxStatisticsEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get rx statistics info. for specified port.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry)
{
    UI32_T                     result;

    LLDP_OM_PortStatistics_T   *om_port_statistics_ptr;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* get port statistics entry pointer from om*/
    if(LLDP_UTY_LogicalPortExisting(port_statistics_entry->port_num))
    {
        om_port_statistics_ptr = LLDP_OM_GetPortStatisticsEntryPtr(port_statistics_entry->port_num);
        {
            port_statistics_entry->rx_frames_discarded_total    = om_port_statistics_ptr->rx_frames_discarded_total;
            port_statistics_entry->rx_frames_errors             = om_port_statistics_ptr->rx_frames_errors;
            port_statistics_entry->rx_frames_total              = om_port_statistics_ptr->rx_frames_total;
            port_statistics_entry->rx_tlvs_discarded_total      = om_port_statistics_ptr->rx_tlvs_discarded_total;
            port_statistics_entry->rx_tlvs_unrecognized_total   = om_port_statistics_ptr->rx_tlvs_unrecognized_total;
            port_statistics_entry->rx_ageouts_total             = om_port_statistics_ptr->rx_ageouts_total;
#if (SYS_CPNT_ADD == TRUE)
            port_statistics_entry->rx_telephone_total           = om_port_statistics_ptr->rx_telephone_total;
#endif
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetPortRxStatisticsEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextPortRxStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * OUTPUT   : LLDP_MGR_PortRxStatistics_T *port_statistics_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 10.5.2.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextPortRxStatisticsEntry(LLDP_MGR_PortRxStatistics_T *port_statistics_entry)
{
    UI32_T                  result;
    LLDP_OM_PortStatistics_T    *port_statistics;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_MASTER_MODE_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_GetNextLogicalPort(&port_statistics_entry->port_num))
    {
        port_statistics = LLDP_OM_GetPortStatisticsEntryPtr(port_statistics_entry->port_num);
        port_statistics_entry->rx_frames_discarded_total = port_statistics->rx_frames_discarded_total;
        port_statistics_entry->rx_frames_errors          = port_statistics->rx_frames_errors;
        port_statistics_entry->rx_frames_total           = port_statistics->rx_frames_total;
        port_statistics_entry->rx_tlvs_discarded_total   = port_statistics->rx_tlvs_discarded_total;
        port_statistics_entry->rx_tlvs_unrecognized_total= port_statistics->rx_tlvs_unrecognized_total;
        port_statistics_entry->rx_ageouts_total          = port_statistics->rx_ageouts_total;
#if (SYS_CPNT_ADD == TRUE)
        port_statistics_entry->rx_telephone_total        = port_statistics->rx_telephone_total;
#endif
        result = LLDP_TYPE_RETURN_OK;
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextPortRxStatisticsEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_LocalSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.2/9.5.6/9.5.7/9.5.8, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalSystemData(LLDP_MGR_LocalSystemData_T *system_entry)
{
    int     ip_forwarding;
#if 0
    UI16_T  tmp;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* get local system data ptr from other module*/
    system_entry->loc_chassis_id_subtype = 4;       /* Mac address*/
    SWCTRL_GetCpuMac(system_entry->loc_chassis_id);
    system_entry->loc_chassis_id_len = 6;
    MIB2_POM_GetSysName((UI8_T *)system_entry->loc_sys_name);
    system_entry->loc_sys_name_len = strlen(system_entry->loc_sys_name);
    MIB2_POM_GetSysDescr((UI8_T *)system_entry->loc_sys_desc);
    system_entry->loc_sys_desc_len = strlen(system_entry->loc_sys_desc);

    /* Need to be modified for the real case */

    system_entry->loc_sys_cap_supported = SYS_ADPT_LLDP_SYSTEM_CAPABILITIES;
    system_entry->loc_sys_cap_enabled = LLDP_TYPE_LOC_SYS_CAP_ENA_BRIDGE;

#if 0 /* Vai Wang, comment out */
    NETCFG_PMGR_GetIpForwarding(&ip_forwarding);
#endif
    ip_forwarding = VAL_ipForwarding_forwarding;
    if(ip_forwarding == VAL_ipForwarding_forwarding)
        system_entry->loc_sys_cap_enabled |= LLDP_TYPE_LOC_SYS_CAP_ENA_ROUTER;

    system_entry->loc_sys_cap_enabled &= system_entry->loc_sys_cap_supported;

#if 0
    tmp = system_entry->loc_sys_cap_supported;
    ((UI8_T*)(&system_entry->loc_sys_cap_supported))[0] = L_CVRT_ByteFlip(((UI8_T*)&tmp)[1]);
    ((UI8_T*)(&system_entry->loc_sys_cap_supported))[1] = L_CVRT_ByteFlip(((UI8_T*)&tmp)[0]);

    tmp = system_entry->loc_sys_cap_enabled;
    ((UI8_T*)(&system_entry->loc_sys_cap_enabled))[0] = L_CVRT_ByteFlip(((UI8_T*)&tmp)[1]);
    ((UI8_T*)(&system_entry->loc_sys_cap_enabled))[1] = L_CVRT_ByteFlip(((UI8_T*)&tmp)[0]);
#endif

    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_MGR_GetLocalSystemData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get statistics info. in the local system for specified port.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalPortData(LLDP_MGR_LocalPortData_T *port_entry)
{
    UI32_T  result = LLDP_TYPE_RETURN_ERROR;
    IF_MGR_IfEntry_T        if_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* Get data from other module */
    if(LLDP_UTY_LogicalPortExisting(port_entry->port_num))
    {
        SWCTRL_GetPortMac(port_entry->port_num, port_entry->loc_port_id);
        port_entry->loc_port_id_len = 6;
        port_entry->loc_port_id_subtype = 3;
        if_entry.if_index = port_entry->port_num;
        if(IF_PMGR_GetIfEntry(&if_entry))
        {
            port_entry->loc_port_desc_len = strlen((char *)if_entry.if_descr);
            strcpy(port_entry->loc_port_desc, (char *)if_entry.if_descr);
        }
        else
        {
            port_entry->loc_port_desc_len = 0;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    return result;
}/* End of LLDP_MGR_GetLocalPortData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocalPortData
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the next statistics info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : LLDP_MGR_LocalPortData_T *port_entry
 * OUTPUT   : LLDP_MGR_LocalPortData_T *port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.3/9.5.5, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalPortData(LLDP_MGR_LocalPortData_T *port_entry)
{
    UI32_T                  result;
    IF_MGR_IfEntry_T        if_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_UTY_GetNextLogicalPort(&port_entry->port_num))
    {

        /* Get data from other module */
        SWCTRL_GetPortMac(port_entry->port_num, port_entry->loc_port_id);
        port_entry->loc_port_id_len = 6;
        port_entry->loc_port_id_subtype = 3;
        if_entry.if_index = port_entry->port_num;
        if(IF_PMGR_GetIfEntry(&if_entry))
        {
            port_entry->loc_port_desc_len = strlen((char *)if_entry.if_descr);
            strcpy(port_entry->loc_port_desc, (char *)if_entry.if_descr);
        }
        else
        {
            port_entry->loc_port_desc_len = 0;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    return result;
}/* End of LLDP_MGR_GetNextLocalPortData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetConfigManAddrEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_ConfigManAddrEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocManAddrConfigEntry(LLDP_MGR_LocalManagementAddrEntry_T *loc_man_addr_config)
{
    BOOL_T  result;

    LLDP_OM_ConfigManAddrEntry_T    *om_config_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_EnterCriticalSection();

    om_config_entry = (LLDP_OM_ConfigManAddrEntry_T *)loc_man_addr_config;

    if(LLDP_OM_GetLocManAddrConfigPtr(&om_config_entry))
    {
        if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4)
        {
            NETCFG_TYPE_InetRifConfig_T  rif_config;
            rif_config.addr = LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,om_config_entry->loc_man_addr,0);
            if(NETCFG_POM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
            {
                loc_man_addr_config->loc_man_addr_if_subtype = 2;       /* 9.5.9.5: if_index */

                loc_man_addr_config->loc_man_addr_if_id = rif_config.ifindex;
            }
            else
            {
                /* No ip interface for this ip.
                 * We need to delete this entry from our loc man addr list.
                 */
                LLDP_OM_DeleteLocManAddrConfig(om_config_entry);
                result = LLDP_TYPE_RETURN_ERROR;

                LLDP_OM_LeaveCriticalSection();
                return result;
            }
        }
        else if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV6)
        {
            /* Not support now */
            return LLDP_TYPE_RETURN_ERROR;
        }
        else if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_ALL802)
        {
            loc_man_addr_config->loc_man_addr_if_subtype = 1;           /* 9.5.9.5: unknown */
        }

        memcpy(loc_man_addr_config->ports_tx_enable, om_config_entry->ports_tx_enable, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocManAddrConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully retrieved. Otherwise, false.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_ConfigManAddrEntry_T  *port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocManAddrConfigEntry(LLDP_MGR_LocalManagementAddrEntry_T *loc_man_addr_config)
{
    BOOL_T  result;
    BOOL_T  get_element;
    LLDP_OM_ConfigManAddrEntry_T    *om_config_entry;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    result = LLDP_TYPE_RETURN_ERROR;
    get_element = FALSE;
    LLDP_OM_EnterCriticalSection();


    om_config_entry = (LLDP_OM_ConfigManAddrEntry_T *)loc_man_addr_config;

    while(!get_element && LLDP_OM_GetNextLocManAddrConfigPtr(&om_config_entry))
    {
        if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4)
        {
            NETCFG_TYPE_InetRifConfig_T  rif_config;
            rif_config.addr= LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,om_config_entry->loc_man_addr,0);
            if(NETCFG_POM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
            {
                loc_man_addr_config->loc_man_addr_if_subtype = 2;       /* 9.5.9.5: if_index */

                loc_man_addr_config->loc_man_addr_if_id = rif_config.ifindex;

                break;
            }
            else
            {
                /* No ip interface for this ip. */
                continue;
            }
        }
        else if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV6)
        {
            /* Not support now */
            continue;
        }
        else if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_ALL802)
        {
            loc_man_addr_config->loc_man_addr_if_subtype = 1;           /* 9.5.9.5: unknown */
        }
        /* assign value */
        loc_man_addr_config->loc_man_addr_subtype = om_config_entry->loc_man_addr_subtype;
        memcpy(loc_man_addr_config->loc_man_addr, om_config_entry->loc_man_addr, sizeof(om_config_entry->loc_man_addr_len));
        loc_man_addr_config->loc_man_addr_len = om_config_entry->loc_man_addr_len;
        memcpy(loc_man_addr_config->ports_tx_enable, om_config_entry->ports_tx_enable, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
        get_element = TRUE;
        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetLocManAddrConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : This funtion returns true if the config man addr entry info.
 *            can be successfully set. Otherwise, false.
 * INPUT    : lport
 *            man_addr_subtype
 *            man_addr
 *            man_addr_len
 *            enable
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 12.2, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetLocManAddrConfig(UI32_T  lport,
                                    UI8_T   man_addr_subtype,
                                    UI8_T   *man_addr,
                                    UI32_T  man_addr_len,
                                    BOOL_T  enable)
{
    BOOL_T  result;
    LLDP_OM_ConfigManAddrEntry_T    man_addr_config;
    LLDP_OM_ConfigManAddrEntry_T    *om_config_entry;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_EnterCriticalSection();

    /* scan all entries to eliminate invalid management address config entry */
    memset(&man_addr_config, 0, sizeof(LLDP_OM_ConfigManAddrEntry_T));
    om_config_entry = &man_addr_config;

    while(LLDP_OM_GetNextLocManAddrConfigPtr(&om_config_entry))
    {
        memcpy(&man_addr_config, om_config_entry, sizeof(man_addr_config));

        if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4)
        {
            rif_config.addr = LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,om_config_entry->loc_man_addr,0);
            /* if this ip is not exist */
            if(NETCFG_POM_IP_GetRifFromIp(&rif_config) != NETCFG_TYPE_OK)
            {
                LLDP_OM_DeleteLocManAddrConfig(om_config_entry);
                om_config_entry = &man_addr_config;
            }
        }
        else if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV6)
        {
            /* Not support now */
            continue;
        }
    }

    /* Configure management address from input */
    memset(&man_addr_config, 0, sizeof(LLDP_OM_ConfigManAddrEntry_T));

    man_addr_config.loc_man_addr_subtype = man_addr_subtype;
    memcpy(man_addr_config.loc_man_addr, man_addr, man_addr_len);
    man_addr_config.loc_man_addr_len = man_addr_len;
    man_addr_config.ports_tx_enable[(lport-1)/8] = enable << ((lport-1) % 8);

    om_config_entry = &man_addr_config;

    /* Check whether the ip interface exists */
    if(om_config_entry->loc_man_addr_subtype == LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4)
    {
        rif_config.addr = LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,om_config_entry->loc_man_addr,0);
        if(NETCFG_POM_IP_GetRifFromIp(&rif_config) != NETCFG_TYPE_OK)
        {
            LLDP_OM_LeaveCriticalSection();
            return LLDP_TYPE_RETURN_ERROR;
        }
    }

    /* if the management address configuration entry is in table */
    if(LLDP_OM_GetLocManAddrConfigPtr(&om_config_entry))
    {
        BOOL_T  original_status;

        if(om_config_entry->ports_tx_enable[lport / 8] & 1 << (lport % 8))
            original_status = TRUE;
        else
            original_status = FALSE;

        if(enable == original_status)
        {
            LLDP_OM_LeaveCriticalSection();
            return LLDP_TYPE_RETURN_OK;
        }
        else
        {
            if(enable)
                om_config_entry->ports_tx_enable[lport / 8] |= 1 << (lport % 8);
            else
                om_config_entry->ports_tx_enable[lport / 8] &= ~(1 << (lport %8 ));

            LLDP_ENGINE_SomethingChangedLocal(lport);
        }
        result = LLDP_TYPE_RETURN_OK;
    }
    else /* not in table */
    {
        result = LLDP_OM_SetLocManAddrConfig(&man_addr_config);
        if (result == LLDP_TYPE_RETURN_OK)
            LLDP_ENGINE_SomethingChangedLocal(lport);
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            this function is for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    UI32_T      result;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result              = LLDP_TYPE_RETURN_ERROR;

    /* Get data from other module */
    if (man_addr_entry->loc_man_addr_subtype == 1) /* IPv4 */
    {
        NETCFG_TYPE_InetRifConfig_T    rif_config;

        LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4, man_addr_entry->loc_man_addr,0);
        rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
        memcpy(rif_config.addr.addr, man_addr_entry->loc_man_addr, sizeof(rif_config.addr.addr));
        if (NETCFG_POM_IP_GetRifFromExactIp(&rif_config) == NETCFG_TYPE_OK)
        {
            man_addr_entry->loc_man_addr_len = 4;
            man_addr_entry->loc_man_addr_if_subtype = 2;
            man_addr_entry->loc_man_addr_if_id = rif_config.ifindex;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    return result;
}/* End of LLDP_MGR_GetLocalManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextLocalManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This api is for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalManagementAddressTlvEntry(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    NETCFG_TYPE_InetRifConfig_T ip_addr_entry;
    UI32_T      result;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    result = LLDP_TYPE_RETURN_ERROR;

    memset(&ip_addr_entry, 0, sizeof(ip_addr_entry));
    ip_addr_entry.addr = LLDP_MGR_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4, man_addr_entry->loc_man_addr, 0);

    /* find the smallest addr which greater than input addr */
    NETCFG_POM_IP_GetRifFromExactIp(&ip_addr_entry); /* get the matched entry (if existed) to fill prefix length */
    while(NETCFG_POM_IP_GetNextRifConfig(&ip_addr_entry) == NETCFG_TYPE_OK)
    {
        if(L_INET_ADDR_TYPE_IPV4 == ip_addr_entry.addr.type)  /* should skip v6 */
        {
            man_addr_entry->loc_man_addr_subtype = 1;   /* IPv4 */
            memcpy(man_addr_entry->loc_man_addr, ip_addr_entry.addr.addr, 4);
            man_addr_entry->loc_man_addr_len = sizeof(UI32_T);
            man_addr_entry->loc_man_addr_if_subtype = 2;
            man_addr_entry->loc_man_addr_if_id = ip_addr_entry.ifindex;
            result = LLDP_TYPE_RETURN_OK;
            break;
        }
    }

    return result;

}/* End of LLDP_MGR_GetNextLocalManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)
{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    LLDP_OM_PortConfigEntry_T   *port_config;
    UI8_T                       loc_man_addr[6];
    UI32_T                      vid,i;
    BOOL_T                      getip;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    if(!VLAN_OM_GetManagementVlan(&vid))
    {
        getip = FALSE;
    }
    else
    {
        VLAN_VID_CONVERTTO_IFINDEX(vid, rif_config.ifindex);
        if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            const static UI8_T ipv4_zero_addr[SYS_ADPT_IPV4_ADDR_LEN] = {0};

            if(0!= memcmp(ipv4_zero_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN))
            {
                getip = TRUE;
                memcpy(man_addr_entry->loc_man_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN);
                man_addr_entry->loc_man_addr_subtype = LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4;
                man_addr_entry->loc_man_addr_if_id = rif_config.ifindex;
                man_addr_entry->loc_man_addr_if_subtype = 2;
                man_addr_entry->loc_man_addr_len = sizeof(UI32_T);
            }
            else
                getip = FALSE;
        }
        else
            getip = FALSE;
    }

    if(!getip)
    {
        SWCTRL_GetCpuMac(loc_man_addr);
        memcpy(man_addr_entry->loc_man_addr,loc_man_addr,sizeof(loc_man_addr));
        man_addr_entry->loc_man_addr_len = 6;
        man_addr_entry->loc_man_addr_subtype = 6;
        man_addr_entry->loc_man_addr_if_id = 0;
        man_addr_entry->loc_man_addr_if_subtype = 1;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    for(i=1;i<= SYS_ADPT_TOTAL_NBR_OF_LPORT; i ++)
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(i);
        if(!port_config)
           continue;

        if(port_config->man_addr_transfer_flag == TRUE)
        {
            lldp_bitstring_setbit((char*)man_addr_entry->ports_tx_enable,(i-1));
        }
        else
        {
            lldp_bitstring_clearbit((char*)man_addr_entry->ports_tx_enable,(i-1));
        }
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_MGR_GetLocalManagementAddress */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetLocalNextManagementAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the local system.
 * INPUT    : None
 * OUTPUT   : LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *            This function is for WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextLocalManagementAddress(LLDP_MGR_LocalManagementAddrEntry_T *man_addr_entry)

{
    NETCFG_TYPE_InetRifConfig_T rif_config;
    LLDP_OM_PortConfigEntry_T   *port_config;
    UI8_T                       loc_man_addr[6];
    UI32_T                      vid,i;
    BOOL_T                      getip;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    if(!VLAN_OM_GetManagementVlan(&vid))
    {
        getip = FALSE;
    }
    else
    {
        VLAN_VID_CONVERTTO_IFINDEX(vid, rif_config.ifindex);
        if (NETCFG_POM_IP_GetNextRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            // peter_yu
            const static UI8_T ipv4_zero_addr[SYS_ADPT_IPV4_ADDR_LEN] = {0};

            if(     (0!= memcmp(ipv4_zero_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN))
                &&  (rif_config.addr.type == L_INET_ADDR_TYPE_IPV4)
              )
            {
               if(memcmp(man_addr_entry->loc_man_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                   return LLDP_TYPE_RETURN_ERROR;
                getip = TRUE;
                memcpy(man_addr_entry->loc_man_addr, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN);
                man_addr_entry->loc_man_addr_subtype = LLDP_TYPE_MAN_ADDR_SUBTYPE_IPV4;
                man_addr_entry->loc_man_addr_if_id = rif_config.ifindex;
                man_addr_entry->loc_man_addr_if_subtype = 2;
                man_addr_entry->loc_man_addr_len = sizeof(UI32_T);
            }
            else
                getip = FALSE;
        }
        else
            getip = FALSE;
    }

    if(!getip)
    {
        SWCTRL_GetCpuMac(loc_man_addr);
        if(memcmp(man_addr_entry->loc_man_addr,loc_man_addr,sizeof(loc_man_addr)) == 0)
            return LLDP_TYPE_RETURN_ERROR;
        memcpy(man_addr_entry->loc_man_addr,loc_man_addr,sizeof(loc_man_addr));
        man_addr_entry->loc_man_addr_len = 6;
        man_addr_entry->loc_man_addr_subtype = 6;
        man_addr_entry->loc_man_addr_if_id = 0;
        man_addr_entry->loc_man_addr_if_subtype = 1;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    for(i=1;i<= SYS_ADPT_TOTAL_NBR_OF_LPORT; i ++)
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(i);
        if(port_config == NULL)
        {
            continue;
        }

        if(port_config->man_addr_transfer_flag == TRUE)
        {
            lldp_bitstring_setbit((char*)man_addr_entry->ports_tx_enable,(i-1));
        }
        else
        {
            lldp_bitstring_clearbit((char*)man_addr_entry->ports_tx_enable,(i-1));
        }
    }

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();
    return LLDP_TYPE_RETURN_OK;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemDataByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : If system_entry->index = 0, the return system entry will be
 *            the 1st data of the port
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetNextRemoteSystemDataByPort(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    UI32_T              result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T   rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.lport = system_entry->rem_local_port_num;
    rem_data.index = system_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_OM_GetNextRemDataByPort(system_entry->rem_local_port_num, &rem_data_p))
    {
        system_entry->rem_time_mark = rem_data_p->time_mark;
        system_entry->rem_index     = rem_data_p->index;

        system_entry->rem_chassis_id_subtype    = rem_data_p->rem_chassis_id_subtype;
        system_entry->rem_chassis_id_len        = rem_data_p->rem_chassis_id_len;
        memcpy(system_entry->rem_chassis_id, rem_data_p->rem_chassis_id, system_entry->rem_chassis_id_len);

        system_entry->rem_port_id_subtype       = rem_data_p->rem_port_id_subtype;
        system_entry->rem_port_id_len           = rem_data_p->rem_port_id_len;
        memcpy(system_entry->rem_port_id, rem_data_p->rem_port_id, system_entry->rem_port_id_len);

        system_entry->rem_ttl = rem_data_p->rx_info_ttl;

        system_entry->rem_port_desc_len         = rem_data_p->rem_sys_entry->rem_port_desc_len;
        memcpy(system_entry->rem_port_desc, rem_data_p->rem_sys_entry->rem_port_desc, system_entry->rem_port_desc_len);
        system_entry->rem_port_desc[system_entry->rem_port_desc_len] = 0;

        system_entry->rem_sys_name_len          = rem_data_p->rem_sys_entry->rem_sys_name_len;
        memcpy(system_entry->rem_sys_name, rem_data_p->rem_sys_entry->rem_sys_name, system_entry->rem_sys_name_len);
        system_entry->rem_sys_name[system_entry->rem_sys_name_len] = 0;

        system_entry->rem_sys_desc_len          = rem_data_p->rem_sys_entry->rem_sys_desc_len;
        memcpy(system_entry->rem_sys_desc, rem_data_p->rem_sys_entry->rem_sys_desc, system_entry->rem_sys_desc_len);
        system_entry->rem_sys_desc[system_entry->rem_sys_desc_len] = 0;

        system_entry->rem_sys_cap_supported = rem_data_p->rem_sys_entry->rem_sys_cap_supported;
        system_entry->rem_sys_cap_enabled   = rem_data_p->rem_sys_entry->rem_sys_cap_enabled;

#if 0
        /* Flip to snmp order */
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[0]);

        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[0]);
#endif

        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextRemoteSystemDataByPort*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemDataByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteSystemDataByIndex(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    UI32_T          result;

    LLDP_OM_RemData_T   *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    LLDP_OM_EnterCriticalSection();

    result              = LLDP_TYPE_RETURN_ERROR;
    if(system_entry->rem_local_port_num == 0)
        system_entry->rem_local_port_num = 1;

    if(LLDP_MGR_GetNextRemData_Local(system_entry->rem_local_port_num, system_entry->rem_index, &rem_data))
    {
        system_entry->rem_local_port_num = rem_data->lport;
        system_entry->rem_index = rem_data->index;

        system_entry->rem_chassis_id_subtype    = rem_data->rem_chassis_id_subtype;
        system_entry->rem_chassis_id_len        = rem_data->rem_chassis_id_len;
        memcpy(system_entry->rem_chassis_id, rem_data->rem_chassis_id, system_entry->rem_chassis_id_len);

        system_entry->rem_port_id_subtype       = rem_data->rem_port_id_subtype;
        system_entry->rem_port_id_len           = rem_data->rem_port_id_len;
        memcpy(system_entry->rem_port_id, rem_data->rem_port_id, system_entry->rem_port_id_len);

        system_entry->rem_port_desc_len         = rem_data->rem_sys_entry->rem_port_desc_len;
        memcpy(system_entry->rem_port_desc, rem_data->rem_sys_entry->rem_port_desc, system_entry->rem_port_desc_len);
        system_entry->rem_port_desc[system_entry->rem_port_desc_len] = 0;

        system_entry->rem_sys_name_len          = rem_data->rem_sys_entry->rem_sys_name_len;
        memcpy(system_entry->rem_sys_name, rem_data->rem_sys_entry->rem_sys_name, system_entry->rem_sys_name_len);
        system_entry->rem_sys_name[system_entry->rem_sys_name_len] = 0;

        system_entry->rem_sys_desc_len          = rem_data->rem_sys_entry->rem_sys_desc_len;
        memcpy(system_entry->rem_sys_desc, rem_data->rem_sys_entry->rem_sys_desc, system_entry->rem_sys_desc_len);
        system_entry->rem_sys_desc[system_entry->rem_sys_desc_len] = 0;

        system_entry->rem_sys_cap_supported = rem_data->rem_sys_entry->rem_sys_cap_supported;
        system_entry->rem_sys_cap_enabled   = rem_data->rem_sys_entry->rem_sys_cap_enabled;

#if 0
        /* Flip to snmp order */
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[0]);

        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[0]);
#endif

        result = LLDP_TYPE_RETURN_OK;
    }


    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextRemoteSystemDataByIndex*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteSystemData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get related TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteSystemData_T *system_entry
 * OUTPUT   : LLDP_MGR_RemoteSystemData_T *system_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by SNMP, so sytem_entry->rem_time_mark will
 *            increase by 1 if search process is reach the end and restart from
 *            the beginning.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteSystemData(LLDP_MGR_RemoteSystemData_T *system_entry)
{
    UI32_T            result= LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p         = &rem_data;
    rem_data.time_mark = system_entry->rem_time_mark;
    rem_data.lport     = system_entry->rem_local_port_num;
    rem_data.index     = system_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        system_entry->rem_time_mark      = rem_data_p->time_mark;
        system_entry->rem_local_port_num = rem_data_p->lport;
        system_entry->rem_index          = rem_data_p->index;

        system_entry->rem_chassis_id_subtype = rem_data_p->rem_chassis_id_subtype;
        system_entry->rem_chassis_id_len     = rem_data_p->rem_chassis_id_len;
        memcpy(system_entry->rem_chassis_id, rem_data_p->rem_chassis_id, system_entry->rem_chassis_id_len);

        system_entry->rem_port_id_subtype = rem_data_p->rem_port_id_subtype;
        system_entry->rem_port_id_len     = rem_data_p->rem_port_id_len;
        memcpy(system_entry->rem_port_id, rem_data_p->rem_port_id, system_entry->rem_port_id_len);

        system_entry->rem_port_desc_len = rem_data_p->rem_sys_entry->rem_port_desc_len;
        memcpy(system_entry->rem_port_desc, rem_data_p->rem_sys_entry->rem_port_desc, system_entry->rem_port_desc_len);
        system_entry->rem_port_desc[system_entry->rem_port_desc_len] = 0;

        system_entry->rem_sys_name_len = rem_data_p->rem_sys_entry->rem_sys_name_len;
        memcpy(system_entry->rem_sys_name, rem_data_p->rem_sys_entry->rem_sys_name, system_entry->rem_sys_name_len);
        system_entry->rem_sys_name[system_entry->rem_sys_name_len] = 0;

        system_entry->rem_sys_desc_len = rem_data_p->rem_sys_entry->rem_sys_desc_len;
        memcpy(system_entry->rem_sys_desc, rem_data_p->rem_sys_entry->rem_sys_desc, system_entry->rem_sys_desc_len);
        system_entry->rem_sys_desc[system_entry->rem_sys_desc_len] = 0;

        system_entry->rem_sys_cap_supported = rem_data_p->rem_sys_entry->rem_sys_cap_supported;
        system_entry->rem_sys_cap_enabled   = rem_data_p->rem_sys_entry->rem_sys_cap_enabled;

#if 0
        /* Flip to snmp order*/
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_supported))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_supported)[0]);

        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[0] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[1]);
        ((UI8_T*)(&system_entry->rem_sys_cap_enabled))[1] = L_CVRT_ByteFlip(((UI8_T*)&rem_data->rem_sys_entry->rem_sys_cap_enabled)[0]);
#endif

        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* LLDP_MGR_GetNextRemoteSystemData */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRemData_Local
 * ------------------------------------------------------------------------
 * PURPOSE  : Get remote data
 * INPUT    : lport             -- a specified port
 *            index             -- a specified index
 * OUTPUT   : void *get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_MGR_GetRemData_Local(UI32_T lport, UI32_T index, LLDP_OM_RemData_T** ptr)
{
    LLDP_OM_RemData_T       *rem_data;

    if (LLDP_UTY_LogicalPortExisting(lport) &&
        LLDP_OM_GetRemData(lport, index, &rem_data))
    {
        *ptr = rem_data;
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    UI32_T                      result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T           rem_data, *rem_data_p;
    LLDP_OM_RemManAddrEntry_T   rem_man_addr, *rem_man_addr_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data.time_mark = man_addr_entry->rem_time_mark;
    rem_data.lport     = man_addr_entry->rem_local_port_num;
    rem_data.index     = man_addr_entry->rem_index;
    rem_data_p         = &rem_data;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        rem_man_addr_p = &rem_man_addr;
        memset(rem_man_addr_p, 0, sizeof(LLDP_OM_RemManAddrEntry_T));
        rem_man_addr_p->rem_man_addr_subtype = man_addr_entry->rem_man_addr_subtype;
        memcpy(rem_man_addr_p->rem_man_addr, man_addr_entry->rem_man_addr, man_addr_entry->rem_man_addr_len);

        if(L_SORT_LST_Get(&rem_data_p->rem_man_addr_list, &rem_man_addr_p))
        {
            man_addr_entry->rem_man_addr_if_subtype = rem_man_addr_p->rem_man_addr_if_subtype;
            man_addr_entry->rem_man_addr_if_id = rem_man_addr_p->rem_man_addr_if_id;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetRemoteManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteManagementAddressTlvEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : Ref to the description in 9.5.9, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetNextRemoteManagementAddressTlvEntry(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    UI32_T                      result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T           rem_data, *rem_data_p;
    LLDP_OM_RemManAddrEntry_T   rem_man_addr, *rem_man_addr_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_man_addr_p = &rem_man_addr;

    rem_data.time_mark = man_addr_entry->rem_time_mark;
    rem_data.lport     = man_addr_entry->rem_local_port_num;
    rem_data.index     = man_addr_entry->rem_index;
    rem_data_p         = &rem_data;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        memset(rem_man_addr_p, 0, sizeof(LLDP_OM_RemManAddrEntry_T));
        rem_man_addr_p->rem_man_addr_subtype = man_addr_entry->rem_man_addr_subtype;
        memcpy(rem_man_addr_p->rem_man_addr, man_addr_entry->rem_man_addr, man_addr_entry->rem_man_addr_len);
        if (L_SORT_LST_Get_Next(&rem_data_p->rem_man_addr_list, &rem_man_addr_p))
        {
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    if (result != LLDP_TYPE_RETURN_OK)
    {
        while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if(L_SORT_LST_Get_1st(&rem_data_p->rem_man_addr_list, &rem_man_addr_p))
            {
                man_addr_entry->rem_time_mark      = rem_data_p->time_mark;
                man_addr_entry->rem_index          = rem_data_p->index;
                man_addr_entry->rem_local_port_num = rem_data_p->lport;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }
    }

    if (result == LLDP_TYPE_RETURN_OK)
    {
        man_addr_entry->rem_man_addr_subtype = rem_man_addr_p->rem_man_addr_subtype;
        memcpy(man_addr_entry->rem_man_addr, rem_man_addr_p->rem_man_addr, rem_man_addr_p->rem_man_addr_len);
        man_addr_entry->rem_man_addr_len     = rem_man_addr_p->rem_man_addr_len;
        man_addr_entry->rem_man_addr_if_subtype = rem_man_addr_p->rem_man_addr_if_subtype;
        man_addr_entry->rem_man_addr_if_id   = rem_man_addr_p->rem_man_addr_if_id;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextRemoteManagementAddressTlvEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Get management address TLV info. in the remote system.
 * INPUT    : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * OUTPUT   : LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : This function is used by CLI/WEB
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort(LLDP_MGR_RemoteManagementAddrEntry_T *man_addr_entry)
{
    UI32_T                      result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T           rem_data, *rem_data_p;
    LLDP_OM_RemManAddrEntry_T   rem_man_addr, *rem_man_addr_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p     = &rem_data;
    rem_data.lport = man_addr_entry->rem_local_port_num;
    rem_data.index = man_addr_entry->rem_index;

    rem_man_addr_p = &rem_man_addr;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(man_addr_entry->rem_local_port_num, man_addr_entry->rem_index, &rem_data_p))
    {
        memset(rem_man_addr_p, 0, sizeof(LLDP_OM_RemManAddrEntry_T));
        rem_man_addr_p->rem_man_addr_subtype = man_addr_entry->rem_man_addr_subtype;
        memcpy(rem_man_addr_p->rem_man_addr, man_addr_entry->rem_man_addr, man_addr_entry->rem_man_addr_len);
        if (L_SORT_LST_Get_Next(&rem_data_p->rem_man_addr_list, &rem_man_addr_p))
        {
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    if (result != LLDP_TYPE_RETURN_OK)
    {
        while(LLDP_OM_GetNextRemDataByPort(man_addr_entry->rem_local_port_num, &rem_data_p))
        {
            if(L_SORT_LST_Get_1st(&rem_data_p->rem_man_addr_list, &rem_man_addr_p))
            {
                man_addr_entry->rem_time_mark      = rem_data_p->time_mark;
                man_addr_entry->rem_index          = rem_data_p->index;
                man_addr_entry->rem_local_port_num = rem_data_p->lport;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }
    }

    if (result == LLDP_TYPE_RETURN_OK)
    {
        man_addr_entry->rem_man_addr_subtype = rem_man_addr_p->rem_man_addr_subtype;
        memcpy(man_addr_entry->rem_man_addr, rem_man_addr_p->rem_man_addr, rem_man_addr_p->rem_man_addr_len);
        man_addr_entry->rem_man_addr_len     = rem_man_addr_p->rem_man_addr_len;
        man_addr_entry->rem_man_addr_if_subtype = rem_man_addr_p->rem_man_addr_if_subtype;
        man_addr_entry->rem_man_addr_if_id   = rem_man_addr_p->rem_man_addr_if_id;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextRemoteManagementAddressTlvEntryByPort*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Master mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_EnterMasterMode(void)
{
    UI32_T  lport = 0, current_time;
    LLDP_OM_PortConfigEntry_T   *port_config;
    LLDP_OM_SysConfigEntry_T    *sys_config;

    /* set the system and ports default configuration */

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    sys_config->global_admin_status   = LLDP_TYPE_DEFAULT_SYS_ADMIN_STATUS;
    sys_config->msg_tx_interval       = LLDP_TYPE_DEFAULT_TX_INTERVAL;
    sys_config->msg_tx_hold_mul       = LLDP_TYPE_DEFAULT_TX_HOLD_MUL;
    sys_config->reinit_delay          = LLDP_TYPE_DEFAULT_REINIT_DELAY;
    sys_config->tx_delay              = LLDP_TYPE_DEFAULT_TX_DELAY;
    sys_config->notification_interval = LLDP_TYPE_DEFAULT_NOTIFY_INTERVAL;
    sys_config->something_changed_remote = FALSE;
    sys_config->fast_start_repeat_count = LLDP_TYPE_DEFAULT_FAST_START_REPEAT_COUNT;
    sys_config->notification_timer = 0;

    /* reset each port's admin status */
    for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport ++)
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        port_config->lport_num = lport;
        port_config->admin_status = SYS_DFLT_LLDP_PORT_ADMIN_STATUS;
        port_config->notification_enable = LLDP_TYPE_DEFAULT_PORT_NOTIFY;
        port_config->basic_tlvs_tx_flag = LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG;
        port_config->man_addr_transfer_flag = LLDP_TYPE_DEFAULT_MAN_ADDR_TLV_TRANSFER_FLAG;

        port_config->reinit_flag = TRUE;
        port_config->something_changed_local = FALSE;
        port_config->something_changed_remote = FALSE;
        port_config->transfer_timer = 0;
        port_config->tx_delay_timer = 0;
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        port_config->reinit_delay_timer = (sys_config->reinit_delay * LLDP_TYPE_TIME_UNIT) + current_time;
        port_config->notify_timer = 0;

        /* 802.1 extensions*/
        port_config->xdot1_port_vlan_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PORT_VLAN_TX;
        port_config->xdot1_protocol_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PROTOCOL_TX;
        port_config->xdot1_proto_vlan_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PROTO_VLAN_TX;
        port_config->xdot1_vlan_name_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_VLAN_NAME_TX;
        port_config->xdot1_cn_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_CN_TX;

        /* 802.3 extensions*/
        port_config->xdot3_tlvs_tx_flag = LLDP_TYPE_DEFAULT_XDOT3_PORT_CONFIG;
#if (LLDP_TYPE_MED == TRUE)
        /* LLDP-MED */
        port_config->fast_start_count = 0;
        port_config->med_device_exist = FALSE;
        port_config->lldp_med_tx_enable = LLDP_TYPE_DEFAULT_MED_TX;
        port_config->lldp_med_notification = LLDP_TYPE_DEFAULT_MED_NOTIFY;
        /* memset(&port_config->lldp_med_location, 0, sizeof(LLDP_OM_XMedRemLocationEntry_T)); */
        LLDP_MGR_InitXMedLocLocationEntry(LLDP_TYPE_DEFAULT_MED_LOCATION_TYPE, &port_config->lldp_med_location, TRUE);
#endif
#if(SYS_CPNT_DCBX == TRUE)
        port_config->xdcbx_tlvs_tx_flag = LLDP_TYPE_DEFAULT_XDCBX_PORT_CONFIG;
#endif

    }
    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    SYSFUN_ENTER_MASTER_MODE();

}/* End of LLDP_MGR_EnterMasterMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Enter Slave mode calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}/* End of LLDP_MGR_EnterSlaveMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_SetTransitionMode()
{
    SYSFUN_SET_TRANSITION_MODE();
}/* End of LLDP_MGR_SetTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Sending enter transition event to task calling by stkctrl.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_EnterTransitionMode()
{
    UI32_T                  result;

    SYSFUN_ENTER_TRANSITION_MODE();

    result              = LLDP_TYPE_RETURN_ERROR;
    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* Delete all databases and all link information */
    LLDP_OM_ResetAll();

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_EnterTransitionMode */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberAdd1st_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberAdd1st_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if 0
    LLDP_OM_PortConfigEntry_T   *trunk_port_config;
    LLDP_OM_PortConfigEntry_T   *member_port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

    /* trunk config inherits from first member */
    trunk_port_config = LLDP_OM_GetPortConfigEntryPtr(trunk_ifindex);
    member_port_config = LLDP_OM_GetPortConfigEntryPtr(member_ifindex);
    trunk_port_config->lport_num = trunk_ifindex;
    trunk_port_config->admin_status = member_port_config->admin_status;
    trunk_port_config->notification_enable = member_port_config->notification_enable;
    trunk_port_config->basic_tlvs_tx_flag = member_port_config->basic_tlvs_tx_flag;
    trunk_port_config->man_addr_transfer_flag = member_port_config->man_addr_transfer_flag;
    trunk_port_config->xdot1_port_vlan_tx_enable = member_port_config->xdot1_port_vlan_tx_enable;
    trunk_port_config->xdot1_vlan_name_tx_enable = member_port_config->xdot1_vlan_name_tx_enable;
    trunk_port_config->xdot1_proto_vlan_tx_enable = member_port_config->xdot1_proto_vlan_tx_enable;
    trunk_port_config->xdot1_protocol_tx_enable = member_port_config->xdot1_protocol_tx_enable;
    trunk_port_config->xdot3_tlvs_tx_flag = member_port_config->xdot3_tlvs_tx_flag;
    trunk_port_config->lldp_med_tx_enable = member_port_config->lldp_med_tx_enable;
    trunk_port_config->lldp_med_notification = member_port_config->lldp_med_notification;
    trunk_port_config->lldp_med_location = member_port_config->lldp_med_location;

    LLDP_OM_ResetPort(trunk_ifindex);
    trunk_port_config->something_changed_local = TRUE;
    LLDP_OM_DeleteRemDataByPort(member_ifindex);

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();
#endif
    return;
}/* End of LLDP_MGR_TrunkMemberAdd1st_CallBack*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberAdd_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port joins the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberAdd_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if 0
    LLDP_OM_PortConfigEntry_T   *trunk_port_config;
    LLDP_OM_PortConfigEntry_T   *member_port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* +++ Enter critical region +++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE == TRUE)
    /* trunk configuration sync to member*/
    trunk_port_config = LLDP_OM_GetPortConfigEntryPtr(trunk_ifindex);
    member_port_config = LLDP_OM_GetPortConfigEntryPtr(member_ifindex);
    member_port_config->admin_status = trunk_port_config->admin_status;
    member_port_config->notification_enable = trunk_port_config->notification_enable;
    member_port_config->basic_tlvs_tx_flag = trunk_port_config->basic_tlvs_tx_flag;
    member_port_config->man_addr_transfer_flag = trunk_port_config->man_addr_transfer_flag;
    member_port_config->xdot1_port_vlan_tx_enable = trunk_port_config->xdot1_port_vlan_tx_enable;
    member_port_config->xdot1_vlan_name_tx_enable = trunk_port_config->xdot1_vlan_name_tx_enable;
    member_port_config->xdot1_proto_vlan_tx_enable = trunk_port_config->xdot1_proto_vlan_tx_enable;
    member_port_config->xdot1_protocol_tx_enable = trunk_port_config->xdot1_protocol_tx_enable;
    member_port_config->xdot3_tlvs_tx_flag = trunk_port_config->xdot3_tlvs_tx_flag;
    member_port_config->lldp_med_tx_enable = trunk_port_config->lldp_med_tx_enable;
    member_port_config->lldp_med_notification = trunk_port_config->lldp_med_notification;
    member_port_config->lldp_med_location = trunk_port_config->lldp_med_location;
#endif

    LLDP_OM_ResetPort(member_ifindex);
    member_port_config->something_changed_local = TRUE;
    LLDP_OM_DeleteRemDataByPort(member_ifindex);

    /* Leave critical region*/
    LLDP_OM_LeaveCriticalSection();
#endif
    return;
}/* End of LLDP_MGR_TrunkMemberAdd_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberDelete_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberDelete_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if 0
    LLDP_OM_PortConfigEntry_T   *trunk_port_config;
    LLDP_OM_PortConfigEntry_T   *member_port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    /* inherit trunk configurations */
    trunk_port_config = LLDP_OM_GetPortConfigEntryPtr(trunk_ifindex);
    member_port_config = LLDP_OM_GetPortConfigEntryPtr(member_ifindex);
    member_port_config->admin_status = trunk_port_config->admin_status;
    member_port_config->notification_enable = trunk_port_config->notification_enable;
    member_port_config->basic_tlvs_tx_flag = trunk_port_config->basic_tlvs_tx_flag;
    member_port_config->man_addr_transfer_flag = trunk_port_config->man_addr_transfer_flag;
    member_port_config->xdot1_port_vlan_tx_enable = trunk_port_config->xdot1_port_vlan_tx_enable;
    member_port_config->xdot1_vlan_name_tx_enable = trunk_port_config->xdot1_vlan_name_tx_enable;
    member_port_config->xdot1_proto_vlan_tx_enable = trunk_port_config->xdot1_proto_vlan_tx_enable;
    member_port_config->xdot1_protocol_tx_enable = trunk_port_config->xdot1_protocol_tx_enable;
    member_port_config->xdot3_tlvs_tx_flag = trunk_port_config->xdot3_tlvs_tx_flag;
    member_port_config->lldp_med_tx_enable = trunk_port_config->lldp_med_tx_enable;
    member_port_config->lldp_med_notification = trunk_port_config->lldp_med_notification;
    member_port_config->lldp_med_location = trunk_port_config->lldp_med_location;

    LLDP_OM_ResetPort(member_ifindex);
    member_port_config->something_changed_local = TRUE;

    /*+++ Leave Critical Section +++*/
    LLDP_OM_LeaveCriticalSection();
#endif
    return;
}/* End of LLDP_MGR_TrunkMemberDelete_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_TrunkMemberDeleteLst_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Service the callback from SWCTRL when the port delete from the trunk
 *            as the 2nd or the following member
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_TrunkMemberDeleteLst_CallBack(UI32_T trunk_ifindex, UI32_T member_ifindex)
{
#if 0
    LLDP_OM_PortConfigEntry_T   *trunk_port_config;
    LLDP_OM_PortConfigEntry_T   *member_port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    /* inherit trunk configurations */
    trunk_port_config = LLDP_OM_GetPortConfigEntryPtr(trunk_ifindex);
    member_port_config = LLDP_OM_GetPortConfigEntryPtr(member_ifindex);
    member_port_config->admin_status = trunk_port_config->admin_status;
    member_port_config->notification_enable = trunk_port_config->notification_enable;
    member_port_config->basic_tlvs_tx_flag = trunk_port_config->basic_tlvs_tx_flag;
    member_port_config->man_addr_transfer_flag = trunk_port_config->man_addr_transfer_flag;
    member_port_config->xdot1_port_vlan_tx_enable = trunk_port_config->xdot1_port_vlan_tx_enable;
    member_port_config->xdot1_vlan_name_tx_enable = trunk_port_config->xdot1_vlan_name_tx_enable;
    member_port_config->xdot1_proto_vlan_tx_enable = trunk_port_config->xdot1_proto_vlan_tx_enable;
    member_port_config->xdot1_protocol_tx_enable = trunk_port_config->xdot1_protocol_tx_enable;
    member_port_config->xdot3_tlvs_tx_flag = trunk_port_config->xdot3_tlvs_tx_flag;
    member_port_config->lldp_med_tx_enable = trunk_port_config->lldp_med_tx_enable;
    member_port_config->lldp_med_notification = trunk_port_config->lldp_med_notification;
    member_port_config->lldp_med_location = trunk_port_config->lldp_med_location;

    LLDP_OM_ResetPort(member_ifindex);
    member_port_config->something_changed_local = TRUE;
    LLDP_OM_DeleteRemDataByPort(trunk_ifindex);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();
#endif
    return;
}/* End of LLDP_MGR_TrunkMemberDeleteLst_CallBack*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PvidChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : When Pvid Changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PvidChanged_CallBack(UI32_T lport, UI32_T old_vid, UI32_T new_vid)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_VlanMemberChanged_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : When vlan member changed, this function will be called.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_VlanMemberChanged_CallBack(UI32_T lport)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifySysNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifySysNameChanged()
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(0);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyVlanNameChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyVlanNameChanged(UI32_T vid)
{
    UI32_T                  lport = 0, vlan_ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);

    while(LLDP_UTY_GetNextLogicalPort(&lport))
    {
        if(VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
        {
            LLDP_ENGINE_SomethingChangedLocal(lport);
        }
    }

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyRifChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : vid_ifindex -- vid_ifindex
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyRifChanged(UI32_T vid_ifindex)
{
    UI32_T                  lport = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    while(LLDP_UTY_GetNextLogicalPort(&lport))
    {
        if(VLAN_OM_IsPortVlanMember(vid_ifindex, lport))
        {
            LLDP_ENGINE_SomethingChangedLocal(lport);
        }
    }

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyIfMauChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyIfMauChanged(UI32_T lport)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyRoutingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyRoutingChanged()
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }
    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(0);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyProtoVlanGroupIdBingingChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port, if the changed is global, lport = 0
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyProtoVlanGroupIdBindingChanged(UI32_T lport)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyPseTableChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When there is something changed, this function will be called.
 * INPUT    : lport -- the specified port
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : lport can be 0, which means changes for all lports
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyPseTableChanged(UI32_T lport)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    LLDP_ENGINE_SomethingChangedLocal(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}/* End of LLDP_MGR_GetOperationMode*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : This function will be invoke to do action when timer event come
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProcessTimerEvent()
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* 1. remove lport from reinit-delay list to cause reinitialization
     *    if reinit_flag of a port is TRUE
     */
    LLDP_ENGINE_ProcessReinitDelayPort();

    /* 2. remove lport from tx-delay list and may cause LLDPDU construction
     *    and transmission if somethingChanged == TRUE
     */
    LLDP_ENGINE_ProcessTimerEventTx();

    /* Enter critical section*/
    LLDP_OM_EnterCriticalSection();

    /* Aging out remote data */
    LLDP_OM_AgeOutRemData();

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();

    /* Notification */
    LLDP_MGR_ProcessNotification();

}/* End of LLDP_MGR_ProcessTimerEvent */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProcessRcvdPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : When there comes an lldpdu, this function will be invoked to
 *            handle the lldpdu
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProcessRcvdPDU(LLDP_TYPE_MSG_T *msg)
{

    UI32_T      tlv_type_count[LLDP_TYPE_MAX_SUPPORT_TLV] = {0};
    LLDP_OM_PortConfigEntry_T   *port_config_p;
#if (SYS_CPNT_DOT1X == TRUE)
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_Dot1XAuthControlledPortStatus_T port_auth_status;
#endif
#endif

#if (SYS_CPNT_DOT1X == TRUE)
    if (!SWCTRL_LogicalPortIsTrunkPort(msg->lport) &&
#if (SYS_CPNT_NETACCESS == TRUE)
        NETACCESS_MGR_GetDot1xPortAuthorized(msg->lport, &port_auth_status)
        && (port_auth_status == VAL_dot1xAuthAuthControlledPortStatus_unauthorized)
#else
        DOT1X_OM_Get_Port_Authorized(msg->lport) == VAL_dot1xAuthAuthControlledPortStatus_unauthorized
#endif
       )
    {
        L_MM_Mref_Release(&(msg->mem_ref));
        return;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    /* Enter critical section */
    LLDP_OM_EnterCriticalSection();

    port_config_p = LLDP_OM_GetPortConfigEntryPtr(msg->lport);
    if(port_config_p->admin_status == VAL_lldpPortConfigAdminStatus_disabled ||
       port_config_p->admin_status == VAL_lldpPortConfigAdminStatus_txOnly)
    {
        LLDP_OM_LeaveCriticalSection();
        L_MM_Mref_Release(&(msg->mem_ref));
        return;
    }

    /* recognize LLDPDU packet */
    if(LLDP_UTY_RecogLLDPDU(msg) == FALSE)
    {
        #if RX_DEBUG_PRINT
        {
            puts("Not LLDP type packet");
        }
        #endif

        LLDP_OM_LeaveCriticalSection();
        L_MM_Mref_Release(&(msg->mem_ref));

        return;
    }


    /* validate LLDPDU */
    if(LLDP_UTY_ValidateLLDPDU(msg, tlv_type_count) == LLDP_TYPE_VALIDATE_TLV_ERROR)
    {
        #if RX_DEBUG_PRINT
        {
            puts("Manadatory tlvs error");
        }
        #endif

        /* Leave critical section */
        LLDP_OM_LeaveCriticalSection();

        L_MM_Mref_Release(&(msg->mem_ref));

        return;
    }

    LLDP_UTY_InsertRemData(msg, tlv_type_count);

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();

    L_MM_Mref_Release(&(msg->mem_ref));

}/* End of LLDP_MGR_ProcessRcvdPDU */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_RegisterTelephoneDetect
 *-------------------------------------------------------------------------
 * PURPOSE  : For registration of detecting a new neighbor
 * INPUT    : fun       -- function to be call back
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void  LLDP_MGR_RegisterTelephoneDetect(void (*fun)(UI32_T lport,
                                                   UI8_T *mac_addr,
                                                   UI8_T network_addr_subtype,
                                                   UI8_T *network_addr,
                                                   UI8_T network_addr_len,
                                                   UI32_T network_addr_ifindex,
                                                   BOOL_T tel_exist))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(LLDP_MGR_NotifyTelephoneDetect_CallBackFuncList);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PortAdminEnable_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for port link up
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PortAdminEnable_CallBack(UI32_T lport)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    LLDP_OM_DeleteRemDataByPort(lport);
    LLDP_OM_ResetPort(lport);

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_PortAdminDisable_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : Callback function for port admin disable
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_PortAdminDisable_CallBack(UI32_T lport)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        LLDP_OM_LeaveCriticalSection();
        LLDP_ENGINE_ConstructLLDPDU(lport, LLDP_TYPE_SHUTDOWN_LLDPDU);
        LLDP_OM_EnterCriticalSection();
        LLDP_OM_DeleteRemDataByPort(lport);
        LLDP_OM_ResetPort(lport);
    }


    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleHotInsertion
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_HandleHotInsertion(UI32_T beg_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T lport, current_time;
    LLDP_OM_SysConfigEntry_T  *sys_config;
    LLDP_OM_PortConfigEntry_T *port_config;


    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    for(lport = beg_ifindex; lport < (beg_ifindex + number_of_port); lport++)
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        port_config->lport_num = lport;
        port_config->reinit_flag = TRUE;
        port_config->something_changed_local = FALSE;
        port_config->something_changed_remote = FALSE;
        port_config->transfer_timer = 0;
        port_config->tx_delay_timer = 0;
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        port_config->reinit_delay_timer = current_time + (sys_config->reinit_delay * LLDP_TYPE_TIME_UNIT);
        port_config->notify_timer = 0;
        port_config->fast_start_count = 0;
        port_config->med_device_exist = FALSE;

        if(!use_default)
            continue;
        port_config->admin_status = SYS_DFLT_LLDP_PORT_ADMIN_STATUS;
        port_config->notification_enable = LLDP_TYPE_DEFAULT_PORT_NOTIFY;
        port_config->basic_tlvs_tx_flag = LLDP_TYPE_DEFAULT_PORT_BASIC_TLV_TRANSFER_FLAG;
        port_config->man_addr_transfer_flag = LLDP_TYPE_DEFAULT_MAN_ADDR_TLV_TRANSFER_FLAG;

        /* 802.1 extensions*/
        port_config->xdot1_port_vlan_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PORT_VLAN_TX;
        port_config->xdot1_protocol_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PROTOCOL_TX;
        port_config->xdot1_proto_vlan_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_PROTO_VLAN_TX;
        port_config->xdot1_vlan_name_tx_enable = LLDP_TYPE_DEFAULT_XDOT1_VLAN_NAME_TX;

        /* 802.3 extensions*/
        port_config->xdot3_tlvs_tx_flag = LLDP_TYPE_DEFAULT_XDOT3_PORT_CONFIG;

#if (LLDP_TYPE_MED == TRUE)
        /* LLDP-MED */
        port_config->fast_start_count = 0;
        port_config->med_device_exist = FALSE;
        port_config->lldp_med_tx_enable = LLDP_TYPE_DEFAULT_MED_TX;
        port_config->lldp_med_notification = LLDP_TYPE_DEFAULT_MED_NOTIFY;
        /* memset(&port_config->lldp_med_location, 0, sizeof(LLDP_OM_XMedRemLocationEntry_T)); */
        LLDP_MGR_InitXMedLocLocationEntry(LLDP_TYPE_DEFAULT_MED_LOCATION_TYPE, &port_config->lldp_med_location, TRUE);
#endif

    }

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot remove.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port)
{
    UI32_T lport;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    for(lport = beg_ifindex; lport < (beg_ifindex + number_of_port); lport++)
    {
        LLDP_OM_DeleteRemDataByPort(lport);
    }

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();
}

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_LogicalPortExisting(xdot1_config_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(xdot1_config_entry->lport);
        xdot1_config_entry->port_vlan_tx_enable = port_config->xdot1_port_vlan_tx_enable;
        xdot1_config_entry->protocol_tx_enable = port_config->xdot1_protocol_tx_enable;
        xdot1_config_entry->proto_vlan_tx_enable = port_config->xdot1_proto_vlan_tx_enable;
        xdot1_config_entry->vlan_name_tx_enable = port_config->xdot1_vlan_name_tx_enable;
        xdot1_config_entry->cn_tx_enable = port_config->xdot1_cn_tx_enable;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : xdot1_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_GetNextLogicalPort(&xdot1_config_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(xdot1_config_entry->lport);
        xdot1_config_entry->port_vlan_tx_enable = port_config->xdot1_port_vlan_tx_enable;
        xdot1_config_entry->protocol_tx_enable = port_config->xdot1_protocol_tx_enable;
        xdot1_config_entry->proto_vlan_tx_enable = port_config->xdot1_proto_vlan_tx_enable;
        xdot1_config_entry->vlan_name_tx_enable = port_config->xdot1_vlan_name_tx_enable;
        result = LLDP_TYPE_RETURN_OK;
    }
    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* LLDP_MGR_GetNextXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot1ConfigEntry(LLDP_MGR_Xdot1ConfigEntry_T *xdot1_config_entry)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if (xdot1_config_entry == NULL)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((xdot1_config_entry->port_vlan_tx_enable != VAL_lldpXdot1ConfigPortVlanTxEnable_true) &&
        (xdot1_config_entry->port_vlan_tx_enable != VAL_lldpXdot1ConfigPortVlanTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((xdot1_config_entry->protocol_tx_enable != VAL_lldpXdot1ConfigProtocolTxEnable_true) &&
        (xdot1_config_entry->protocol_tx_enable != VAL_lldpXdot1ConfigProtocolTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((xdot1_config_entry->proto_vlan_tx_enable != VAL_lldpXdot1ConfigProtoVlanTxEnable_true) &&
        (xdot1_config_entry->proto_vlan_tx_enable != VAL_lldpXdot1ConfigProtoVlanTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((xdot1_config_entry->vlan_name_tx_enable != VAL_lldpXdot1ConfigVlanNameTxEnable_true) &&
        (xdot1_config_entry->vlan_name_tx_enable != VAL_lldpXdot1ConfigVlanNameTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((xdot1_config_entry->cn_tx_enable != TRUE) &&
        (xdot1_config_entry->cn_tx_enable != FALSE))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(xdot1_config_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(xdot1_config_entry->lport);

        if ((port_config->xdot1_port_vlan_tx_enable != xdot1_config_entry->port_vlan_tx_enable) ||
            (port_config->xdot1_protocol_tx_enable != xdot1_config_entry->protocol_tx_enable) ||
            (port_config->xdot1_proto_vlan_tx_enable != xdot1_config_entry->proto_vlan_tx_enable) ||
            (port_config->xdot1_vlan_name_tx_enable != xdot1_config_entry->vlan_name_tx_enable) ||
            (port_config->xdot1_cn_tx_enable != xdot1_config_entry->cn_tx_enable))
        {
            port_config->xdot1_port_vlan_tx_enable = xdot1_config_entry->port_vlan_tx_enable;
            port_config->xdot1_protocol_tx_enable = xdot1_config_entry->protocol_tx_enable;
            port_config->xdot1_proto_vlan_tx_enable = xdot1_config_entry->proto_vlan_tx_enable;
            port_config->xdot1_vlan_name_tx_enable = xdot1_config_entry->vlan_name_tx_enable;
            port_config->xdot1_cn_tx_enable = xdot1_config_entry->cn_tx_enable;
            port_config->something_changed_local = TRUE;
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetXdot1ConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigPortVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigPortVlanTxEnable(UI32_T lport, UI32_T port_vlan_tx_enable)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((port_vlan_tx_enable != VAL_lldpXdot1ConfigPortVlanTxEnable_true) &&
        (port_vlan_tx_enable != VAL_lldpXdot1ConfigPortVlanTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->xdot1_port_vlan_tx_enable != port_vlan_tx_enable)
        {
            port_config->xdot1_port_vlan_tx_enable = port_vlan_tx_enable;
            port_config->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigProtoVlanTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigProtoVlanTxEnable(UI32_T lport, UI32_T proto_vlan_tx_enable)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((proto_vlan_tx_enable != VAL_lldpXdot1ConfigProtoVlanTxEnable_true) &&
        (proto_vlan_tx_enable != VAL_lldpXdot1ConfigProtoVlanTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->xdot1_proto_vlan_tx_enable != proto_vlan_tx_enable)
        {
            port_config->xdot1_proto_vlan_tx_enable = proto_vlan_tx_enable;
            port_config->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigVlanNameTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigVlanNameTxEnable(UI32_T lport, UI32_T vlan_name_tx_enable)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((vlan_name_tx_enable != VAL_lldpXdot1ConfigVlanNameTxEnable_true) &&
        (vlan_name_tx_enable != VAL_lldpXdot1ConfigVlanNameTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->xdot1_vlan_name_tx_enable != vlan_name_tx_enable)
        {
            port_config->xdot1_vlan_name_tx_enable = vlan_name_tx_enable;
            port_config->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot1ConfigProtocolTxEnable
 *-------------------------------------------------------------------------
 * PURPOSE  : Set 802.1 extension configuraiton
 * INPUT    : xdot1_config_entry
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_SetXdot1ConfigProtocolTxEnable(UI32_T lport, UI32_T protocol_tx_enable)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if ((protocol_tx_enable != VAL_lldpXdot1ConfigProtocolTxEnable_true) &&
        (protocol_tx_enable != VAL_lldpXdot1ConfigProtocolTxEnable_false))
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->xdot1_protocol_tx_enable != protocol_tx_enable)
        {
            port_config->xdot1_protocol_tx_enable = protocol_tx_enable;
            port_config->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry)
{
    UI32_T                  result;

    VLAN_OM_Dot1qPortVlanEntry_T   dot1q_port_vlan;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_UTY_LogicalPortExisting(xdot1_loc_entry->lport) &&
       VLAN_OM_GetDot1qPortVlanEntry(xdot1_loc_entry->lport, &dot1q_port_vlan))
    {
        xdot1_loc_entry->port_vlan_id = dot1q_port_vlan.dot1q_pvid_index;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdot1LocEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local entry
 * INPUT    : xdot1_loc_entry
 * OUTPUT   : xdot1_loc_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocEntry(LLDP_MGR_Xdot1LocEntry_T *xdot1_loc_entry)
{
    UI32_T                  result;

    VLAN_OM_Dot1qPortVlanEntry_T   dot1q_port_vlan;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_UTY_GetNextLogicalPort(&xdot1_loc_entry->lport) &&
       VLAN_OM_GetDot1qPortVlanEntry(xdot1_loc_entry->lport, &dot1q_port_vlan))
    {
        xdot1_loc_entry->port_vlan_id = dot1q_port_vlan.dot1q_pvid_index;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1LocEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : Not support now
 *-------------------------------------------------------------------------
 *
 */
UI32_T  LLDP_MGR_GetXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry)
{
    UI32_T                  result;
    UI32_T                  vlan_ifindex;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    VLAN_VID_CONVERTTO_IFINDEX(xdot1_loc_proto_vlan_entry->proto_vlan_id, vlan_ifindex);
    if(VLAN_OM_IsPortVlanMember(vlan_ifindex, xdot1_loc_proto_vlan_entry->lport))
    {
        xdot1_loc_proto_vlan_entry->proto_vlan_supported = 2;
        xdot1_loc_proto_vlan_entry->proto_vlan_enabled = 2;
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local protocol vlan entry
 * INPUT    : xdot1_loc_proto_vlan_entry
 * OUTPUT   : xdot1_loc_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : Not support now
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocProtoVlanEntry(LLDP_MGR_Xdot1LocProtoVlanEntry_T *xdot1_loc_proto_vlan_entry)
{
    UI32_T                  result;
    UI32_T                  vlan_ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(!LLDP_UTY_LogicalPortExisting(xdot1_loc_proto_vlan_entry->lport))
    {
        if(LLDP_UTY_GetNextLogicalPort(&xdot1_loc_proto_vlan_entry->lport))
        {
            xdot1_loc_proto_vlan_entry->proto_vlan_id = 0;
        }
        else
        {
            /* +++Leave critical section+++ */
            LLDP_OM_LeaveCriticalSection();

            return result;
        }
    }

    {
        do
        {
            while(VLAN_OM_GetNextVlanId(0, &xdot1_loc_proto_vlan_entry->proto_vlan_id))
            {
                VLAN_VID_CONVERTTO_IFINDEX(xdot1_loc_proto_vlan_entry->proto_vlan_id, vlan_ifindex);
                if(VLAN_OM_IsPortVlanMember(vlan_ifindex, xdot1_loc_proto_vlan_entry->lport))
                {
                    xdot1_loc_proto_vlan_entry->proto_vlan_supported = 2;
                    xdot1_loc_proto_vlan_entry->proto_vlan_enabled = 2;
                    result = LLDP_TYPE_RETURN_OK;
                    break;
                }
            }

            /* In order to getting next port, the vlan_id should be reset too.*/
            if(result != LLDP_TYPE_RETURN_OK)
            {
                xdot1_loc_proto_vlan_entry->proto_vlan_id = 0;
            }
        }while(result != LLDP_TYPE_RETURN_OK && LLDP_UTY_GetNextLogicalPort(&xdot1_loc_proto_vlan_entry->lport));

    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry)
{
    UI32_T                  result;
    UI32_T                  vlan_ifindex;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(VLAN_VID_CONVERTTO_IFINDEX(xdot1_loc_vlan_name_entry->vlan_id, vlan_ifindex) > (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1))
    {
        if(VLAN_OM_IsPortVlanMember(vlan_ifindex, xdot1_loc_vlan_name_entry->lport))
        {
            VLAN_OM_Dot1qVlanCurrentEntry_T dot1q_vlan_entry;
            if(VLAN_OM_GetDot1qVlanCurrentEntry(0/*time_mark*/, xdot1_loc_vlan_name_entry->vlan_id, &dot1q_vlan_entry))
            {
                xdot1_loc_vlan_name_entry->vlan_name_len = strlen(dot1q_vlan_entry.dot1q_vlan_static_name);
                memcpy(xdot1_loc_vlan_name_entry->vlan_name,
                       dot1q_vlan_entry.dot1q_vlan_static_name,
                       xdot1_loc_vlan_name_entry->vlan_name_len);
                xdot1_loc_vlan_name_entry->vlan_name[xdot1_loc_vlan_name_entry->vlan_name_len] = 0;
                result = LLDP_TYPE_RETURN_OK;
            }
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdot1LocVlanNameEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension local vlan name entry
 * INPUT    : xdot1_loc_vlan_name_entry
 * OUTPUT   : xdot1_loc_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocVlanNameEntry(LLDP_MGR_Xdot1LocVlanNameEntry_T *xdot1_loc_vlan_name_entry)
{
    UI32_T                  result;
    UI32_T                  vlan_ifindex;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(!LLDP_UTY_LogicalPortExisting(xdot1_loc_vlan_name_entry->lport))
    {
        if(LLDP_UTY_GetNextLogicalPort(&xdot1_loc_vlan_name_entry->lport))
        {
            xdot1_loc_vlan_name_entry->vlan_id = 0;
        }
        else
        {
            /* +++Leave critical section+++ */
            LLDP_OM_LeaveCriticalSection();
            return result;
        }
    }

    do
    {
        while(VLAN_OM_GetNextVlanId(0, &xdot1_loc_vlan_name_entry->vlan_id))
        {
            VLAN_VID_CONVERTTO_IFINDEX(xdot1_loc_vlan_name_entry->vlan_id, vlan_ifindex);
            if(VLAN_OM_IsPortVlanMember(vlan_ifindex, xdot1_loc_vlan_name_entry->lport))
            {
                VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
                VLAN_OM_GetDot1qVlanCurrentEntry(0, xdot1_loc_vlan_name_entry->vlan_id, &vlan_entry);
                xdot1_loc_vlan_name_entry->vlan_name_len = strlen(vlan_entry.dot1q_vlan_static_name);
                memcpy(xdot1_loc_vlan_name_entry->vlan_name, vlan_entry.dot1q_vlan_static_name, xdot1_loc_vlan_name_entry->vlan_name_len);
                xdot1_loc_vlan_name_entry->vlan_name[xdot1_loc_vlan_name_entry->vlan_name_len] = 0;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }

        /* In order to getting next port, the vlan_id should be reset too.*/
        if(result != LLDP_TYPE_RETURN_OK)
        {
            xdot1_loc_vlan_name_entry->vlan_id = 0;
        }
    }
    while(result != LLDP_TYPE_RETURN_OK && LLDP_UTY_GetNextLogicalPort(&xdot1_loc_vlan_name_entry->lport));


    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1LocVlanNameEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry)
{
    UI32_T                  result = LLDP_TYPE_RETURN_ERROR;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(xdot1_loc_protocol_entry->lport))
    {
        if (xdot1_loc_protocol_entry->protocol_index == 1)
        {
            /* LLDP protocol*/
            xdot1_loc_protocol_entry->protocol_id[0] = 0x88;
            xdot1_loc_protocol_entry->protocol_id[1] = 0xCC;
            xdot1_loc_protocol_entry->protocol_id_len = 2;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1LocProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension protocol entry
 * INPUT    : xdot1_loc_protocol_entry
 * OUTPUT   : xdot1_loc_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1LocProtocolEntry(LLDP_MGR_Xdot1LocProtocolEntry_T *xdot1_loc_protocol_entry)
{
    UI32_T                  result;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;


    if(LLDP_UTY_GetNextLogicalPort(&xdot1_loc_protocol_entry->lport))
    {
        xdot1_loc_protocol_entry->protocol_index = 0;
    }
    else
    {
        /* +++Leave critical section+++ */
        LLDP_OM_LeaveCriticalSection();

        return result;
    }

    if(xdot1_loc_protocol_entry->protocol_index < 1)
    {
        xdot1_loc_protocol_entry->protocol_index = 1;
        xdot1_loc_protocol_entry->protocol_id[0] = 0x88;
        xdot1_loc_protocol_entry->protocol_id[1] = 0xCC;
        xdot1_loc_protocol_entry->protocol_id_len = 2;
        result = LLDP_TYPE_RETURN_OK;
    }


    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemEntry(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    UI32_T             result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot1_rem_entry->rem_time_mark;
    rem_data.lport     = xdot1_rem_entry->rem_local_port_num;
    rem_data.index     = xdot1_rem_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->xdot1_rem_port_vlan_id)
        {
            xdot1_rem_entry->rem_time_mark      = rem_data_p->time_mark;
            xdot1_rem_entry->rem_local_port_num = rem_data_p->lport;
            xdot1_rem_entry->rem_index          = rem_data_p->index;
            xdot1_rem_entry->rem_port_vlan_id   = rem_data_p->xdot1_rem_port_vlan_id;
            result = LLDP_TYPE_RETURN_OK;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1RemEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote entry
 * INPUT    : xdot1_rem_entry
 * OUTPUT   : xdot1_rem_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemEntryByIndex(LLDP_MGR_Xdot1RemEntry_T *xdot1_rem_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_MGR_GetNextRemData_Local(xdot1_rem_entry->rem_local_port_num, xdot1_rem_entry->rem_index, &rem_data))
    {
        if(rem_data->xdot1_rem_port_vlan_id)
        {
            xdot1_rem_entry->rem_port_vlan_id = rem_data->xdot1_rem_port_vlan_id;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemProtoVlanEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol vlan entry
 * INPUT    : xdot1_rem_proto_vlan_entry
 * OUTPUT   : xdot1_rem_proto_vlan_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemProtoVlanEntry(LLDP_MGR_Xdot1RemProtoVlanEntry_T *xdot1_rem_proto_vlan_entry)
{
    UI32_T                           result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T                rem_data, *rem_data_p;
    LLDP_OM_Xdot1RemProtoVlanEntry_T rem_proto_vlan, *rem_proto_vlan_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot1_rem_proto_vlan_entry->rem_time_mark;
    rem_data.lport     = xdot1_rem_proto_vlan_entry->rem_local_port_num;
    rem_data.index     = xdot1_rem_proto_vlan_entry->rem_index;

    rem_proto_vlan_p = &rem_proto_vlan;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        rem_proto_vlan_p->rem_proto_vlan_id = xdot1_rem_proto_vlan_entry->rem_proto_vlan_id;

        if(L_SORT_LST_Get_Next(&rem_data_p->xdot1_rem_proto_vlan_list, &rem_proto_vlan_p))
        {
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    if (result != LLDP_TYPE_RETURN_OK)
    {
        while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if(L_SORT_LST_Get_1st(&rem_data_p->xdot1_rem_proto_vlan_list, &rem_proto_vlan_p))
            {
                xdot1_rem_proto_vlan_entry->rem_time_mark      = rem_data_p->time_mark;
                xdot1_rem_proto_vlan_entry->rem_local_port_num = rem_data_p->lport;
                xdot1_rem_proto_vlan_entry->rem_index          = rem_data_p->index;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }
    }

    if (result == LLDP_TYPE_RETURN_OK)
    {
        xdot1_rem_proto_vlan_entry->rem_proto_vlan_id        = rem_proto_vlan_p->rem_proto_vlan_id;
        xdot1_rem_proto_vlan_entry->rem_proto_vlan_supported = rem_proto_vlan_p->rem_proto_vlan_supported;
        xdot1_rem_proto_vlan_entry->rem_proto_vlan_enabled   = rem_proto_vlan_p->rem_proto_vlan_enabled;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1RemProtoVlanEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemVlanNameEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote vlan name entry
 * INPUT    : xdot1_rem_vlan_name_entry
 * OUTPUT   : xdot1_rem_vlan_name_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemVlanNameEntry(LLDP_MGR_Xdot1RemVlanNameEntry_T *xdot1_rem_vlan_name_entry)
{
    UI32_T                  result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T       rem_data, *rem_data_p;
    LLDP_OM_Xdot1RemVlanNameEntry_T rem_vlan_name, *rem_vlan_name_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot1_rem_vlan_name_entry->rem_time_mark;
    rem_data.lport     = xdot1_rem_vlan_name_entry->rem_local_port_num;
    rem_data.index     = xdot1_rem_vlan_name_entry->rem_index;

    rem_vlan_name_p = &rem_vlan_name;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        rem_vlan_name_p->rem_vlan_id = xdot1_rem_vlan_name_entry->rem_vlan_id;
        if(L_SORT_LST_Get_Next(&rem_data_p->xdot1_rem_vlan_name_list, &rem_vlan_name_p))
        {
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    if (result != LLDP_TYPE_RETURN_OK)
    {
        while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if(L_SORT_LST_Get_1st(&rem_data_p->xdot1_rem_vlan_name_list, &rem_vlan_name_p))
            {
                xdot1_rem_vlan_name_entry->rem_time_mark      = rem_data_p->time_mark;
                xdot1_rem_vlan_name_entry->rem_local_port_num = rem_data_p->lport;
                xdot1_rem_vlan_name_entry->rem_index          = rem_data_p->index;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }
    }

    if (result == LLDP_TYPE_RETURN_OK)
    {
        xdot1_rem_vlan_name_entry->rem_vlan_id = rem_vlan_name_p->rem_vlan_id;
        xdot1_rem_vlan_name_entry->rem_vlan_name_len = rem_vlan_name_p->rem_vlan_name_len;
        memcpy(xdot1_rem_vlan_name_entry->rem_vlan_name, rem_vlan_name_p->rem_vlan_name, rem_vlan_name_p->rem_vlan_name_len);
        xdot1_rem_vlan_name_entry->rem_vlan_name[rem_vlan_name_p->rem_vlan_name_len]= 0;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1RemVlanNameEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot1RemProtocolEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.1 extension remote protocol entry
 * INPUT    : xdot1_rem_protocol_entry
 * OUTPUT   : xdot1_rem_protocol_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot1RemProtocolEntry(LLDP_MGR_Xdot1RemProtocolEntry_T *xdot1_rem_protocol_entry)
{
    UI32_T                  result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T       rem_data, *rem_data_p;
    LLDP_OM_Xdot1ProtocolEntry_T rem_proto, *rem_proto_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot1_rem_protocol_entry->rem_time_mark;
    rem_data.lport     = xdot1_rem_protocol_entry->rem_local_port_num;
    rem_data.index     = xdot1_rem_protocol_entry->rem_index;

    rem_proto_p = &rem_proto;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        rem_proto_p->rem_protocol_index = xdot1_rem_protocol_entry->rem_protocol_index;
        if(L_SORT_LST_Get_Next(&rem_data_p->xdot1_rem_protocol_list, &rem_proto_p))
        {
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    if (result != LLDP_TYPE_RETURN_OK)
    {
        while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if(L_SORT_LST_Get_1st(&rem_data_p->xdot1_rem_protocol_list, &rem_proto_p))
            {
                xdot1_rem_protocol_entry->rem_time_mark      = rem_data_p->time_mark;
                xdot1_rem_protocol_entry->rem_local_port_num = rem_data_p->lport;
                xdot1_rem_protocol_entry->rem_index          = rem_data_p->index;
                result = LLDP_TYPE_RETURN_OK;
                break;
            }
        }
    }

    if (result == LLDP_TYPE_RETURN_OK)
    {
        xdot1_rem_protocol_entry->rem_protocol_index = rem_proto_p->rem_protocol_index;
        memcpy(xdot1_rem_protocol_entry->rem_protocol_id, rem_proto_p->rem_protocol_id, rem_proto_p->rem_protocol_id_len);
        xdot1_rem_protocol_entry->rem_protocol_id_len = rem_proto_p->rem_protocol_id_len;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot1RemProtocolEntry */
#endif

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_LogicalPortExisting(xdot3_port_config_entry_p->lport))
    {
        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdot3_port_config_entry_p->lport);
        xdot3_port_config_entry_p->tlvs_tx_enable = port_config_p->xdot3_tlvs_tx_flag;
        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX) != 0 )
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX ) != 0)
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_LINK_AGG_TX ) != 0)
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV ) != 0)
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = FALSE;
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3PortConfigEntry(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_GetNextLogicalPort(&xdot3_port_config_entry_p->lport))
    {
        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdot3_port_config_entry_p->lport);
        xdot3_port_config_entry_p->tlvs_tx_enable = port_config_p->xdot3_tlvs_tx_flag;
        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX ) != 0 )
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->mac_phy_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX ) != 0)
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->power_via_mdi_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_LINK_AGG_TX ) != 0)
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->link_agg_tlv_enabled = FALSE;
        }

        if( (port_config_p->xdot3_tlvs_tx_flag & LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV ) != 0)
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = TRUE;
        }
        else
        {
            xdot3_port_config_entry_p->max_frame_size_tlv_enabled = FALSE;
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot3PortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp only
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot3PortConfigEntry(UI32_T lport, UI8_T tlvs_tx_status)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

#if (SYS_CPNT_POE == TRUE)
    if (STKTPLG_OM_IsLocalPoeDevice() == FALSE)
    {
        if (tlvs_tx_status & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX)
        {
            return LLDP_TYPE_RETURN_ERROR;
        }
    }
#else
    if (tlvs_tx_status & LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
#endif

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config_p = LLDP_OM_GetPortConfigEntryPtr(lport);
        if ((port_config_p->xdot3_tlvs_tx_flag ^ tlvs_tx_status) != 0)
        {
            port_config_p->xdot3_tlvs_tx_flag = tlvs_tx_status;
            port_config_p->something_changed_local = TRUE;
        }
        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetXdot3PortConfigEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdot3PortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension port config entry
 * INPUT    : xdot3_port_config_entry
 * OUTPUT   : xdot3_port_config_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdot3PortConfig(LLDP_MGR_Xdot3PortConfigEntry_T *xdot3_port_config_entry_p)
{
    UI32_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_UTY_LogicalPortExisting(xdot3_port_config_entry_p->lport))
    {
        UI8_T                   original_config;

        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdot3_port_config_entry_p->lport);
        original_config = port_config_p->xdot3_tlvs_tx_flag;

        if(xdot3_port_config_entry_p->mac_phy_tlv_enabled)
        {
            port_config_p->xdot3_tlvs_tx_flag |= LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX;/* mac phy*/
        }
        else
        {
            port_config_p->xdot3_tlvs_tx_flag &= (~(LLDP_TYPE_XDOT3_MAC_PHY_TLV_TX));
        }

        if(xdot3_port_config_entry_p->power_via_mdi_tlv_enabled)
        {
            port_config_p->xdot3_tlvs_tx_flag |= LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX;/* power via mdi */
        }
        else
        {
            port_config_p->xdot3_tlvs_tx_flag &= (~(LLDP_TYPE_XDOT3_POWER_VIA_MDI_TX));
        }

        if(xdot3_port_config_entry_p->link_agg_tlv_enabled)
        {
            port_config_p->xdot3_tlvs_tx_flag |= LLDP_TYPE_XDOT3_LINK_AGG_TX; /* link agg */
        }
        else
        {
            port_config_p->xdot3_tlvs_tx_flag &= (~(LLDP_TYPE_XDOT3_LINK_AGG_TX));
        }

        if(xdot3_port_config_entry_p->max_frame_size_tlv_enabled)
        {
            port_config_p->xdot3_tlvs_tx_flag |= LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV;  /* max frame size */
        }
        else
        {
            port_config_p->xdot3_tlvs_tx_flag &= (~(LLDP_TYPE_XDOT3_MAX_FRAME_SIZE_TLV));
        }

        if((original_config ^ port_config_p->xdot3_tlvs_tx_flag) != 0)
        {
            port_config_p->something_changed_local = TRUE;
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetXdot3PortConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local port entry
 * INPUT    : xdot3_loc_port_entry
 * OUTPUT   : xdot3_loc_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_loc_port_entry)
{
    UI32_T                  result;
#if (SYS_CPNT_MAU_MIB == TRUE)
    SWCTRL_IfMauEntry_T         if_mau_entry;
    SWCTRL_IfMauAutoNegEntry_T  if_mau_auto_neg_entry;
#else
    Port_Info_T      port_info;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
#if (SYS_CPNT_MAU_MIB == TRUE)
    if_mau_entry.ifMauIfIndex = xdot3_loc_port_entry->lport;
    if_mau_entry.ifMauIndex = 1;
    if_mau_auto_neg_entry.ifMauIfIndex = xdot3_loc_port_entry->lport;
    if_mau_auto_neg_entry.ifMauIndex = 1;

    if(!SWCTRL_GetIfMauEntry(&if_mau_entry))
    {
        LLDP_OM_LeaveCriticalSection();

        return result;
    }
    if(!SWCTRL_GetIfMauAutoNegEntry(&if_mau_auto_neg_entry))
    {
        LLDP_OM_LeaveCriticalSection();

        return result;
    }

#if (SYS_CPNT_SNMP_BITS_FROM_LEFT == TRUE)
    xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = (if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits) >> 16;
#else
    xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = (if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits);
#endif  /* #if (SYS_CPNT_SNMP_BITS_FROM_LEFT == TRUE) */

    xdot3_loc_port_entry->loc_port_auto_neg_enabled = if_mau_auto_neg_entry.ifMauAutoNegAdminStatus;
    xdot3_loc_port_entry->loc_port_auto_neg_supported = if_mau_entry.ifMauAutoNegSupported;
    xdot3_loc_port_entry->loc_port_oper_mau_type = if_mau_entry.ifMauType;

    result = LLDP_TYPE_RETURN_OK;
#else /* else of #if (SYS_CPNT_MAU_MIB == TRUE)*/
    memset(&port_info, 0, sizeof(Port_Info_T));
    if(SWCTRL_GetPortInfo(xdot3_loc_port_entry->lport, &port_info))
    {
        xdot3_loc_port_entry->loc_port_auto_neg_supported = VAL_lldpXdot3LocPortAutoNegSupported_true;
        xdot3_loc_port_entry->loc_port_auto_neg_enabled
         = (port_info.autoneg_state == VAL_portAutonegotiation_enabled)?VAL_lldpXdot3LocPortAutoNegEnabled_true:VAL_lldpXdot3LocPortAutoNegEnabled_false;
        /* not support MAU, use 0 for auto-negotiation advertised capability */
        xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = 0;
        /* not support MAU, use 0 for MAU type */
        xdot3_loc_port_entry->loc_port_oper_mau_type = 0;

        result = LLDP_TYPE_RETURN_OK;
    }
#endif
    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdot3LocPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local port entry
 * INPUT    : xdot3_loc_port_entry
 * OUTPUT   : xdot3_loc_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocPortEntry(LLDP_MGR_Xdot3LocPortEntry_T *xdot3_loc_port_entry)
{
    UI32_T                  result;
#if (SYS_CPNT_MAU_MIB == TRUE)
    SWCTRL_IfMauEntry_T         if_mau_entry;
    SWCTRL_IfMauAutoNegEntry_T  if_mau_auto_neg_entry;
#else
    Port_Info_T      port_info;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
#if (SYS_CPNT_MAU_MIB == TRUE)
    if(LLDP_UTY_GetNextLogicalPort(&xdot3_loc_port_entry->lport))
    {
        if_mau_entry.ifMauIfIndex = xdot3_loc_port_entry->lport;
        if_mau_entry.ifMauIndex = 1;
        if_mau_auto_neg_entry.ifMauIfIndex = xdot3_loc_port_entry->lport;
        if_mau_auto_neg_entry.ifMauIndex = 1;

        if(!SWCTRL_GetIfMauEntry(&if_mau_entry))
        {
            LLDP_OM_LeaveCriticalSection();

            return result;
        }
        if(!SWCTRL_GetIfMauAutoNegEntry(&if_mau_auto_neg_entry))
        {
            LLDP_OM_LeaveCriticalSection();

            return result;
        }

#if (SYS_CPNT_SNMP_BITS_FROM_LEFT == TRUE)
        xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = (if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits) >> 16;
#else
        xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = (if_mau_auto_neg_entry.ifMauAutoNegCapAdvertisedBits);
#endif  /* #if (SYS_CPNT_SNMP_BITS_FROM_LEFT == TRUE) */

        xdot3_loc_port_entry->loc_port_auto_neg_enabled = if_mau_auto_neg_entry.ifMauAutoNegAdminStatus;
        xdot3_loc_port_entry->loc_port_auto_neg_supported = if_mau_entry.ifMauAutoNegSupported;
        xdot3_loc_port_entry->loc_port_oper_mau_type = if_mau_entry.ifMauType;

        result = LLDP_TYPE_RETURN_OK;
    }
#else /* else of #if (SYS_CPNT_MAU_MIB == TRUE)*/
    if (LLDP_UTY_GetNextLogicalPort(&xdot3_loc_port_entry->lport))
    {
        memset(&port_info, 0, sizeof(Port_Info_T));
        if (SWCTRL_GetPortInfo(xdot3_loc_port_entry->lport, &port_info))
        {
            xdot3_loc_port_entry->loc_port_auto_neg_supported = VAL_lldpXdot3LocPortAutoNegSupported_true;
            xdot3_loc_port_entry->loc_port_auto_neg_enabled
             = (port_info.autoneg_state == VAL_portAutonegotiation_enabled)?VAL_lldpXdot3LocPortAutoNegEnabled_true:VAL_lldpXdot3LocPortAutoNegEnabled_false;
            /* not support MAU, use 0 for auto-negotiation advertised capability */
            xdot3_loc_port_entry->loc_port_auto_neg_adv_cap = 0;
            /* not support MAU, use 0 for MAU type */
            xdot3_loc_port_entry->loc_port_oper_mau_type = 0;

            result = LLDP_TYPE_RETURN_OK;
        }
    }
#endif
    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextXdot3LocPortEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry)
{
    UI32_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_POE == TRUE)
    if (    (LLDP_UTY_LogicalPortExisting(xdot3_loc_power_entry->lport) == TRUE)
         && (SWCTRL_LogicalPortIsTrunkPort(xdot3_loc_power_entry->lport) == FALSE)
         && (STKTPLG_OM_IsLocalPoeDevice() == TRUE)
       )
    {
        UI32_T  unit, port, trunk_id = 0;
        POE_OM_PsePort_T entry;

        SWCTRL_LogicalPortToUserPort(xdot3_loc_power_entry->lport, &unit, &port, &trunk_id);
        if (POE_POM_GetPsePortEntry(unit, port, &entry))
        {
            xdot3_loc_power_entry->loc_power_port_class = VAL_lldpXdot3LocPowerPortClass_pClassPSE;
            xdot3_loc_power_entry->loc_power_mdi_supported = VAL_lldpXdot3LocPowerMDISupported_true;
            xdot3_loc_power_entry->loc_power_mdi_enabled = entry.pse_port_admin_enable;
            xdot3_loc_power_entry->loc_power_pair_controlable = entry.pse_port_power_pairs_ctrl_ability;
            xdot3_loc_power_entry->loc_power_pairs = entry.pse_port_power_pairs;
            xdot3_loc_power_entry->loc_power_class = entry.pse_port_power_classifications;
            result = LLDP_TYPE_RETURN_OK;
        }
    }
#endif /* #if (SYS_CPNT_POE == TRUE) */

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local power entry
 * INPUT    : xdot3_loc_power_entry
 * OUTPUT   : xdot3_loc_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocPowerEntry(LLDP_MGR_Xdot3LocPowerEntry_T *xdot3_loc_power_entry)
{
    UI32_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_POE == TRUE)
    if (    (LLDP_UTY_GetNextLogicalPort(&xdot3_loc_power_entry->lport) == TRUE)
         && (SWCTRL_LogicalPortIsTrunkPort(xdot3_loc_power_entry->lport) == FALSE)
         && (STKTPLG_OM_IsLocalPoeDevice() == TRUE)
       )
   {
        UI32_T  unit, port, trunk_id = 0;
        POE_OM_PsePort_T entry;

        SWCTRL_LogicalPortToUserPort(xdot3_loc_power_entry->lport, &unit, &port, &trunk_id);
        if (POE_POM_GetPsePortEntry(unit, port, &entry))
        {
            xdot3_loc_power_entry->loc_power_port_class = VAL_lldpXdot3LocPowerPortClass_pClassPSE;
            xdot3_loc_power_entry->loc_power_mdi_supported = VAL_lldpXdot3LocPowerMDISupported_true;
            xdot3_loc_power_entry->loc_power_mdi_enabled = entry.pse_port_admin_enable;
            xdot3_loc_power_entry->loc_power_pair_controlable = entry.pse_port_power_pairs_ctrl_ability;
            xdot3_loc_power_entry->loc_power_pairs = entry.pse_port_power_pairs;
            xdot3_loc_power_entry->loc_power_class = entry.pse_port_power_classifications;
            result = LLDP_TYPE_RETURN_OK;
        }
    }
#endif /* #if (SYS_CPNT_POE == TRUE) */

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry)
{
    UI32_T                  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    LLDP_OM_EnterCriticalSection();

    if(SWCTRL_LogicalPortIsTrunkPort(xdot3_loc_link_agg_entry->lport))
    {
        xdot3_loc_link_agg_entry->loc_link_agg_status = 2;
        xdot3_loc_link_agg_entry->loc_link_agg_port_id = xdot3_loc_link_agg_entry->lport;
    }
    else
    {
        xdot3_loc_link_agg_entry->loc_link_agg_status = 1;
        xdot3_loc_link_agg_entry->loc_link_agg_port_id = 0;
    }

    result = LLDP_TYPE_RETURN_OK;
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local link aggregation entry
 * INPUT    : xdot3_loc_link_agg_entry
 * OUTPUT   : xdot3_loc_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocLinkAggEntry(LLDP_MGR_Xdot3LocLinkAggEntry_T *xdot3_loc_link_agg_entry)
{
    UI32_T                  result = LLDP_TYPE_RETURN_ERROR;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }
    LLDP_OM_EnterCriticalSection();

    if(!LLDP_UTY_GetNextLogicalPort(&xdot3_loc_link_agg_entry->lport))
    {
        LLDP_OM_LeaveCriticalSection();

        return result;
    }

    if(SWCTRL_LogicalPortIsTrunkPort(xdot3_loc_link_agg_entry->lport))
    {
        xdot3_loc_link_agg_entry->loc_link_agg_status = 2;
        xdot3_loc_link_agg_entry->loc_link_agg_port_id = xdot3_loc_link_agg_entry->lport;
    }
    else
    {
        xdot3_loc_link_agg_entry->loc_link_agg_status = 1;
        xdot3_loc_link_agg_entry->loc_link_agg_port_id = 0;
    }
    result = LLDP_TYPE_RETURN_OK;
    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry)
{
    UI32_T  result;
    UI32_T  untagged_size, tagged_size;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_LogicalPortExisting(xdot3_loc_max_frame_size_entry->lport))
    {
        if (SWCTRL_GetPortMaxFrameSize(xdot3_loc_max_frame_size_entry->lport, &untagged_size, &tagged_size))
        {
#if (SYS_CPNT_VLAN == TRUE)
            xdot3_loc_max_frame_size_entry->loc_max_frame_size = tagged_size;
#else
            xdot3_loc_max_frame_size_entry->loc_max_frame_size = untagged_size;
#endif
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3LocMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension local maximum frame size entry
 * INPUT    : xdot3_loc_max_frame_size_entry
 * OUTPUT   : xdot3_loc_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3LocMaxFrameSizeEntry(LLDP_MGR_Xdot3LocMaxFrameSizeEntry_T *xdot3_loc_max_frame_size_entry)
{
    UI32_T  result;
    UI32_T  untagged_size, tagged_size;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    result = LLDP_TYPE_RETURN_ERROR;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_GetNextLogicalPort(&xdot3_loc_max_frame_size_entry->lport))
    {
        if (SWCTRL_GetPortMaxFrameSize(xdot3_loc_max_frame_size_entry->lport, &untagged_size, &tagged_size))
        {
#if (SYS_CPNT_VLAN == TRUE)
            xdot3_loc_max_frame_size_entry->loc_max_frame_size = tagged_size;
#else
            xdot3_loc_max_frame_size_entry->loc_max_frame_size = untagged_size;
#endif
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    UI32_T             result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_port_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_port_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_port_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        if (rem_data_p->xdot3_rem_port_entry)
        {
            xdot3_rem_port_entry->rem_port_auto_neg_adv_cap   = rem_data_p->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap;
            xdot3_rem_port_entry->rem_port_auto_neg_enable    = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_enable;
            xdot3_rem_port_entry->rem_port_auto_neg_supported = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_support;
            xdot3_rem_port_entry->rem_port_oper_mau_type      = rem_data_p->xdot3_rem_port_entry->rem_port_oper_mau_type;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPortEntry(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    UI32_T            result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_port_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_port_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_port_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->xdot3_rem_port_entry)
        {
            xdot3_rem_port_entry->rem_time_mark               = rem_data_p->time_mark;
            xdot3_rem_port_entry->rem_local_port_num          = rem_data_p->lport;
            xdot3_rem_port_entry->rem_index                   = rem_data_p->index;
            xdot3_rem_port_entry->rem_port_auto_neg_adv_cap   = rem_data_p->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap;
            xdot3_rem_port_entry->rem_port_auto_neg_enable    = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_enable;
            xdot3_rem_port_entry->rem_port_auto_neg_supported = rem_data_p->xdot3_rem_port_entry->rem_auto_neg_support;
            xdot3_rem_port_entry->rem_port_oper_mau_type      = rem_data_p->xdot3_rem_port_entry->rem_port_oper_mau_type;
            result = LLDP_TYPE_RETURN_OK;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPortEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote port entry
 * INPUT    : xdot3_rem_port_entry
 * OUTPUT   : xdot3_rem_port_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPortEntryByIndex(LLDP_MGR_Xdot3RemPortEntry_T *xdot3_rem_port_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;
    if(LLDP_MGR_GetNextRemData_Local( xdot3_rem_port_entry->rem_local_port_num,
                               xdot3_rem_port_entry->rem_index,
                               &rem_data))
    {
        if(rem_data->xdot3_rem_port_entry)
        {
            xdot3_rem_port_entry->rem_local_port_num = rem_data->lport;
            xdot3_rem_port_entry->rem_index = rem_data->index;
            xdot3_rem_port_entry->rem_port_auto_neg_adv_cap = rem_data->xdot3_rem_port_entry->rem_port_auto_neg_adv_cap;
            xdot3_rem_port_entry->rem_port_auto_neg_enable = rem_data->xdot3_rem_port_entry->rem_auto_neg_enable;
            xdot3_rem_port_entry->rem_port_auto_neg_supported = rem_data->xdot3_rem_port_entry->rem_auto_neg_support;
            xdot3_rem_port_entry->rem_port_oper_mau_type = rem_data->xdot3_rem_port_entry->rem_port_oper_mau_type;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPowerEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPowerEntry(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry)
{
    UI32_T            result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_power_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_power_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_power_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->xdot3_rem_power_entry)
        {
            xdot3_rem_power_entry->rem_time_mark              = rem_data_p->time_mark;
            xdot3_rem_power_entry->rem_local_port_num         = rem_data_p->lport;
            xdot3_rem_power_entry->rem_index                  = rem_data_p->index;
            xdot3_rem_power_entry->rem_power_port_class       = rem_data_p->xdot3_rem_power_entry->rem_power_port_class;
            xdot3_rem_power_entry->rem_power_mdi_enabled      = rem_data_p->xdot3_rem_power_entry->rem_power_mdi_enabled;
            xdot3_rem_power_entry->rem_power_mdi_supported    = rem_data_p->xdot3_rem_power_entry->rem_power_mdi_supported;
            xdot3_rem_power_entry->rem_power_pairs            = rem_data_p->xdot3_rem_power_entry->rem_power_pairs;
            xdot3_rem_power_entry->rem_power_pair_controlable = rem_data_p->xdot3_rem_power_entry->rem_power_pair_controlable;
            xdot3_rem_power_entry->rem_power_class            = rem_data_p->xdot3_rem_power_entry->rem_power_class;
            result = LLDP_TYPE_RETURN_OK;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemPowerEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote power entry
 * INPUT    : xdot3_rem_power_entry
 * OUTPUT   : xdot3_rem_power_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemPowerEntryByIndex(LLDP_MGR_Xdot3RemPowerEntry_T *xdot3_rem_power_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_MGR_GetNextRemData_Local(xdot3_rem_power_entry->rem_local_port_num, xdot3_rem_power_entry->rem_index, &rem_data))
    {
        if(rem_data->xdot3_rem_power_entry)
        {
            xdot3_rem_power_entry->rem_local_port_num = rem_data->lport;
            xdot3_rem_power_entry->rem_index = rem_data->index;
            xdot3_rem_power_entry->rem_power_port_class = rem_data->xdot3_rem_power_entry->rem_power_port_class;
            xdot3_rem_power_entry->rem_power_mdi_enabled = rem_data->xdot3_rem_power_entry->rem_power_mdi_enabled;
            xdot3_rem_power_entry->rem_power_mdi_supported = rem_data->xdot3_rem_power_entry->rem_power_mdi_supported;
            xdot3_rem_power_entry->rem_power_pairs = rem_data->xdot3_rem_power_entry->rem_power_pairs;
            xdot3_rem_power_entry->rem_power_pair_controlable = rem_data->xdot3_rem_power_entry->rem_power_pair_controlable;
            xdot3_rem_power_entry->rem_power_class = rem_data->xdot3_rem_power_entry->rem_power_class;
            result = LLDP_TYPE_RETURN_OK;
        }
    }


    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    UI32_T             result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_link_agg_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_link_agg_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_link_agg_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->xdot3_rem_link_agg_entry)
        {
            xdot3_rem_link_agg_entry->rem_link_agg_port_id = rem_data_p->xdot3_rem_link_agg_entry->rem_link_agg_port_id;
            xdot3_rem_link_agg_entry->rem_link_agg_status  = rem_data_p->xdot3_rem_link_agg_entry->rem_link_agg_status;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemLinkAggEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemLinkAggEntry(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    UI32_T            result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_link_agg_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_link_agg_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_link_agg_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->xdot3_rem_link_agg_entry)
        {
            xdot3_rem_link_agg_entry->rem_time_mark        = rem_data_p->time_mark;
            xdot3_rem_link_agg_entry->rem_local_port_num   = rem_data_p->lport;
            xdot3_rem_link_agg_entry->rem_index            = rem_data_p->index;
            xdot3_rem_link_agg_entry->rem_link_agg_port_id = rem_data_p->xdot3_rem_link_agg_entry->rem_link_agg_port_id;
            xdot3_rem_link_agg_entry->rem_link_agg_status  = rem_data_p->xdot3_rem_link_agg_entry->rem_link_agg_status;
            result = LLDP_TYPE_RETURN_OK;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemLinkAggEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote link aggregation entry
 * INPUT    : xdot3_rem_link_agg_entry
 * OUTPUT   : xdot3_rem_link_agg_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemLinkAggEntryByIndex(LLDP_MGR_Xdot3RemLinkAggEntry_T *xdot3_rem_link_agg_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_MGR_GetNextRemData_Local(xdot3_rem_link_agg_entry->rem_local_port_num, xdot3_rem_link_agg_entry->rem_index, &rem_data))
    {
        if(rem_data->xdot3_rem_link_agg_entry)
        {
            xdot3_rem_link_agg_entry->rem_local_port_num = rem_data->lport;
            xdot3_rem_link_agg_entry->rem_index = rem_data->index;
            xdot3_rem_link_agg_entry->rem_link_agg_port_id = rem_data->xdot3_rem_link_agg_entry->rem_link_agg_port_id;
            xdot3_rem_link_agg_entry->rem_link_agg_status = rem_data->xdot3_rem_link_agg_entry->rem_link_agg_status;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    UI32_T            result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_max_frame_size_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_max_frame_size_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_max_frame_size_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        xdot3_rem_max_frame_size_entry->rem_max_frame_size = rem_data_p->xdot3_rem_max_frame_size;
        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for snmp
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntry(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    UI32_T            result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = xdot3_rem_max_frame_size_entry->rem_time_mark;
    rem_data.lport     = xdot3_rem_max_frame_size_entry->rem_local_port_num;
    rem_data.index     = xdot3_rem_max_frame_size_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        xdot3_rem_max_frame_size_entry->rem_time_mark      = rem_data_p->time_mark;
        xdot3_rem_max_frame_size_entry->rem_local_port_num = rem_data_p->lport;
        xdot3_rem_max_frame_size_entry->rem_index          = rem_data_p->index;
        xdot3_rem_max_frame_size_entry->rem_max_frame_size = rem_data_p->xdot3_rem_max_frame_size;
        result = LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get 802.3 extension remote maximum frame size entry
 * INPUT    : xdot3_rem_max_frame_size_entry
 * OUTPUT   : xdot3_rem_max_frame_size_entry
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : for WEB/CLI
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntryByIndex(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T *xdot3_rem_max_frame_size_entry)
{
    UI32_T                  result;
    LLDP_OM_RemData_T       *rem_data;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_MGR_GetNextRemData_Local(xdot3_rem_max_frame_size_entry->rem_local_port_num, xdot3_rem_max_frame_size_entry->rem_index, &rem_data))
    {
        if(rem_data->xdot3_rem_max_frame_size)
        {
            xdot3_rem_max_frame_size_entry->rem_local_port_num = rem_data->lport;
            xdot3_rem_max_frame_size_entry->rem_index = rem_data->index;
            xdot3_rem_max_frame_size_entry->rem_max_frame_size = rem_data->xdot3_rem_max_frame_size;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}
#endif

#if (LLDP_TYPE_MED == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedFastStartRepeatCount
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED fast start repeat count
 * INPUT    : repeat_count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedFastStartRepeatCount(UI32_T  repeat_count)
{
    LLDP_OM_SysConfigEntry_T    *sys_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* check range */
    if ((repeat_count <= LLDP_TYPE_MIN_FAST_START_REPEAT_COUNT) &&
        (repeat_count >= LLDP_TYPE_MAX_FAST_START_REPEAT_COUNT))
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    sys_config->fast_start_repeat_count = repeat_count;

    /* Leave critical section */
    LLDP_OM_LeaveCriticalSection();

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(port_config_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(port_config_entry->lport);

        port_config_entry->lldp_xmed_port_cap_supported = SYS_ADPT_LLDP_MED_CAPABILITY;
        port_config_entry->lldp_xmed_port_notif_enabled = port_config->lldp_med_notification;
        port_config_entry->lldp_xmed_port_tlvs_tx_enabled = port_config->lldp_med_tx_enable;
        result = TRUE;
    }
    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : None
 * OUTPUT   : port_config_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedPortConfigEntry(LLDP_MGR_XMedPortConfigEntry_T *port_config_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_GetNextLogicalPort(&port_config_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(port_config_entry->lport);

        port_config_entry->lldp_xmed_port_cap_supported = SYS_ADPT_LLDP_MED_CAPABILITY;
        port_config_entry->lldp_xmed_port_notif_enabled = port_config->lldp_med_notification;
        port_config_entry->lldp_xmed_port_tlvs_tx_enabled = port_config->lldp_med_tx_enable;
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port transfer tlvs
 * INPUT    : lport, tlvs_tx_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedPortConfigTlvsTx(UI32_T lport, UI16_T tlvs_tx_enabled)
{
    BOOL_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = FALSE;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->lldp_med_tx_enable != tlvs_tx_enabled)
        {
            port_config->lldp_med_tx_enable = tlvs_tx_enabled;
            if (port_config->med_device_exist)
                port_config->something_changed_local = TRUE;
        }
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedPortConfigTlvsTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : tlvs_tx_enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXMedPortConfigTlvsTx(UI32_T lport, UI16_T *tlvs_tx_enabled)
{
    UI32_T                  result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        if(port_config->lldp_med_tx_enable == LLDP_TYPE_DEFAULT_MED_TX)
            result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
        {
            *tlvs_tx_enabled = port_config->lldp_med_tx_enable;
            result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedPortConfigNotifEnabled
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED port notification
 * INPUT    : notif_enabled
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedPortConfigNotifEnabled(UI32_T lport, UI8_T notif_enabled)
{
    BOOL_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = FALSE;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->lldp_med_notification != notif_enabled)
        {
            port_config->lldp_med_notification = notif_enabled;
            if (port_config->med_device_exist)
                port_config->something_changed_local = TRUE;
        }
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedPortNotification
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED port configuration entry
 * INPUT    : lport
 * OUTPUT   : enabled
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXMedPortNotification(UI32_T lport, UI8_T *enabled)
{
    UI32_T                  result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        if(port_config->lldp_med_notification == LLDP_TYPE_DEFAULT_MED_NOTIFY)
            result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        else
        {
            *enabled = port_config->lldp_med_notification;
            result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : Now is support the voice vlan used.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry)
{
    BOOL_T                              result = FALSE;
    LLDP_OM_PortConfigEntry_T           *port_config;
    VLAN_OM_VlanPortEntry_T             vlan_port_entry;
    UI32_T                              vid;
#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
    PRI_MGR_Dot1dPortPriorityEntry_T    priority_entry;
    UI8_T                               priority = 0;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(!LLDP_UTY_LogicalPortExisting(loc_med_policy_entry->lport))
    {
        LLDP_OM_LeaveCriticalSection();
        return result;
    }
    port_config = LLDP_OM_GetPortConfigEntryPtr(loc_med_policy_entry->lport);

    if(loc_med_policy_entry->app_type != VAL_lldpXMedLocMediaPolicyAppType_voice)
    {
        LLDP_OM_LeaveCriticalSection();
        return result;
    }

    loc_med_policy_entry->unknown = VAL_lldpXMedLocMediaPolicyUnknown_false;

#if (SYS_CPNT_ADD == TRUE)
    if(!ADD_OM_GetVoiceVlanId((I32_T*)&vid))
        VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
    else
    {
        UI32_T vid_ifindex = 0;

        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
        if(!VLAN_OM_IsPortVlanMember(vid_ifindex, loc_med_policy_entry->lport))
            VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
    }
#else
    VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
#endif
    loc_med_policy_entry->vid = vid;

    memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));
    if (VLAN_OM_GetVlanPortEntryByIfindex(port_config->lport_num, &vlan_port_entry) == FALSE)
    {
        return FALSE;
    }

    if(vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
        loc_med_policy_entry->tagged = VAL_lldpXMedLocMediaPolicyTagged_true;
    else
        loc_med_policy_entry->tagged = VAL_lldpXMedLocMediaPolicyTagged_false;

#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
    memset(&priority_entry, 0, sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));
    PRI_PMGR_GetDot1dPortPriorityEntry(port_config->lport_num, &priority_entry);
    loc_med_policy_entry->priority = priority_entry.dot1d_port_default_user_priority;
#else
    loc_med_policy_entry->priority = 0;
#endif

    loc_med_policy_entry->dscp = 0;
    result = TRUE;


    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED media policy entry
 * INPUT    : None
 * OUTPUT   : loc_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : Now is support the voice vlan used.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocMediaPolicyEntry(LLDP_MGR_XMedLocMediaPolicyEntry_T *loc_med_policy_entry)
{
    BOOL_T                              result = FALSE;
    LLDP_OM_PortConfigEntry_T           *port_config;
    VLAN_OM_VlanPortEntry_T             vlan_port_entry;
    UI32_T                              vid;
#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
    PRI_MGR_Dot1dPortPriorityEntry_T    priority_entry;
    UI8_T                               priority = 0;
#endif

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_GetNextLogicalPort(&loc_med_policy_entry->lport))
    {
        loc_med_policy_entry->app_type = 0;
        port_config = LLDP_OM_GetPortConfigEntryPtr(loc_med_policy_entry->lport);
    }
    else
    {
        LLDP_OM_LeaveCriticalSection();
        return result;
    }

    if(loc_med_policy_entry->app_type < VAL_lldpXMedLocMediaPolicyAppType_voice)
    {
        loc_med_policy_entry->app_type = VAL_lldpXMedLocMediaPolicyAppType_voice;
        loc_med_policy_entry->unknown = VAL_lldpXMedLocMediaPolicyUnknown_false;

#if (SYS_CPNT_ADD == TRUE)
        if(!ADD_OM_GetVoiceVlanId((I32_T*)&vid))
            VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
        else
        {
            UI32_T vid_ifindex = 0;
            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

            if(!VLAN_OM_IsPortVlanMember(vid_ifindex, loc_med_policy_entry->lport))
                VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
        }
#else
        VLAN_OM_GetGlobalDefaultVlan_Ex(&vid);
#endif
        loc_med_policy_entry->vid = vid;

        memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));
        if (VLAN_OM_GetVlanPortEntryByIfindex(port_config->lport_num, &vlan_port_entry) == FALSE)
        {
            return FALSE;
        }

        if(vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_dot1qTrunk)
            loc_med_policy_entry->tagged = VAL_lldpXMedLocMediaPolicyTagged_true;
        else
            loc_med_policy_entry->tagged = VAL_lldpXMedLocMediaPolicyTagged_false;

#if (SYS_CPNT_PRI_MGMT_PORT_BASE == TRUE)
        memset(&priority_entry, 0, sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));
        PRI_PMGR_GetDot1dPortPriorityEntry(port_config->lport_num, &priority_entry);
        loc_med_policy_entry->priority = priority_entry.dot1d_port_default_user_priority;

#else
        loc_med_policy_entry->priority = 0;
#endif


        loc_med_policy_entry->dscp = 0;

        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for snmp.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(loc_location_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(loc_location_entry->lport);

        switch (loc_location_entry->location_subtype)
        {
            case VAL_lldpXMedLocLocationSubtype_coordinateBased:
#if 1 // wakka TODO
                if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased))
                {
                    result = TRUE;
                }
#endif
                break;
            case VAL_lldpXMedLocLocationSubtype_civicAddress:
                if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
                {
                    LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
                    UI8_T   *ca;

                    /* 'what' value */
                    loc_location_entry->location_info[1] = port_config->lldp_med_location.civic_addr.what;

                    /* country code */
                    memcpy(&loc_location_entry->location_info[2], port_config->lldp_med_location.civic_addr.country_code, 2);

                    ca = &loc_location_entry->location_info[4];

                    if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element!= 0 &&
                        L_SORT_LST_Get_1st(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry))
                    {
                        /* ca entrys */
                        do
                        {
                            ca[0] = ca_entry->ca_type;
                            ca[1] = ca_entry->ca_length;
                            memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                            ca += (2 + ca[1]);
                                } while (L_SORT_LST_Get_Next(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry));
                            }

                            loc_location_entry->location_info_len = ca - loc_location_entry->location_info;
                            loc_location_entry->location_info[0] = loc_location_entry->location_info_len - 1;
                        result = TRUE;
                    }
                    break;
            case VAL_lldpXMedLocLocationSubtype_elin:
                if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin))
                {
                    loc_location_entry->location_info_len = port_config->lldp_med_location.elin_addr.elin_len;
                    memcpy(loc_location_entry->location_info, port_config->lldp_med_location.elin_addr.elin, loc_location_entry->location_info_len);
                    result = TRUE;
                }
                break;
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location entry
 * INPUT    : None
 * OUTPUT   : loc_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for snmp.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;
    UI32_T                      location_type;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    location_type = loc_location_entry->location_subtype;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    while (!result)
    {
        if (location_type >= VAL_lldpXMedLocLocationSubtype_elin ||
            !LLDP_UTY_LogicalPortExisting(loc_location_entry->lport))
        {
            if(LLDP_UTY_GetNextLogicalPort(&loc_location_entry->lport))
            {
                location_type = 0;
            }
            else
            {
                break;
            }
        }

        port_config = LLDP_OM_GetPortConfigEntryPtr(loc_location_entry->lport);

        if (location_type < VAL_lldpXMedLocLocationSubtype_coordinateBased &&
            (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased)))
        {
            loc_location_entry->location_subtype = VAL_lldpXMedLocLocationSubtype_coordinateBased;
            result = TRUE;
        }
        else if (location_type < VAL_lldpXMedLocLocationSubtype_civicAddress &&
            (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress)))
        {
            LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
            UI8_T   *ca;

            loc_location_entry->location_subtype = VAL_lldpXMedLocLocationSubtype_civicAddress;

            /* 'what' value */
            loc_location_entry->location_info[1] = port_config->lldp_med_location.civic_addr.what;

            /* country code */
            memcpy(&loc_location_entry->location_info[2], port_config->lldp_med_location.civic_addr.country_code, 2);

            ca = &loc_location_entry->location_info[4];

            if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element!= 0 &&
                L_SORT_LST_Get_1st(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry))
            {
                /* ca entrys */
                do
                {
                    ca[0] = ca_entry->ca_type;
                    ca[1] = ca_entry->ca_length;
                    memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                    ca += (2 + ca[1]);
                } while (L_SORT_LST_Get_Next(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry));
            }

            loc_location_entry->location_info_len = ca - loc_location_entry->location_info;
            loc_location_entry->location_info[0] = loc_location_entry->location_info_len - 1;
            result = TRUE;
        }
        else if (location_type < VAL_lldpXMedLocLocationSubtype_elin &&
            (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin)))
        {
            loc_location_entry->location_subtype = VAL_lldpXMedLocLocationSubtype_elin;
            loc_location_entry->location_info_len = port_config->lldp_med_location.elin_addr.elin_len;
            memcpy(loc_location_entry->location_info, port_config->lldp_med_location.elin_addr.elin, loc_location_entry->location_info_len);
            result = TRUE;
        }
        location_type = VAL_lldpXMedLocLocationSubtype_elin;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location entry
 * INPUT    : loc_location_entry
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for snmp.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_SetXMedLocLocationEntry(LLDP_MGR_XMedLocLocationEntry_T *loc_location_entry)
{
    BOOL_T                      result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(loc_location_entry->lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(loc_location_entry->lport);

        switch (loc_location_entry->location_subtype)
        {
            case VAL_lldpXMedLocLocationSubtype_coordinateBased: /* coordinate based */
#if 0 // wakka: not support now
                result = TRUE;
#endif
                break;

            case VAL_lldpXMedLocLocationSubtype_civicAddress: /* civic addr */
            {
                LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
                UI8_T   *ca = 0;
                UI8_T   lci_length;

                if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
                {
                    port_config->lldp_med_location.location_type_valid &= ~ BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress);
                    if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element != 0)
                        LLDP_OM_FreeCivicAddrCaList(&port_config->lldp_med_location.civic_addr.ca_list);
                }

                if ((loc_location_entry->location_info_len >= 4) &&
                    (loc_location_entry->location_info_len <= 256))
                {
                    if (LLDP_MGR_IsCountryCodeValid(&loc_location_entry->location_info[2]) == FALSE)
                    {
                        LLDP_OM_LeaveCriticalSection();
                        return FALSE;
                    }

                    port_config->lldp_med_location.location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress);

                    lci_length = loc_location_entry->location_info[0];
                    port_config->lldp_med_location.civic_addr.what = loc_location_entry->location_info[1];
                    memcpy(port_config->lldp_med_location.civic_addr.country_code,
                       &loc_location_entry->location_info[2], 2);
                    lci_length = lci_length - 3;
                    ca = &loc_location_entry->location_info[4];

                    while (lci_length > 2)
                    {
                        ca_entry = NULL;

                        /* Reset ca_list */
                        if(port_config->lldp_med_location.civic_addr.ca_list.max_element_count == 0)
                        {
                            LLDP_OM_CreateCivicAddrCaList(&port_config->lldp_med_location.civic_addr.ca_list);
                        }

                        /* allocate a ca_entry*/
                        ca_entry = (LLDP_OM_XMedLocationCivicAddrCaTlv_T *)malloc(sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
                        if(ca_entry == NULL)
                            break;

                        ca_entry->ca_type = ca[0];
                        ca_entry->ca_length = ca[1];

                        if (ca_entry->ca_length > LLDP_TYPE_MAX_CA_VALUE_LEN)
                        {
                           free(ca_entry);
                           break;
                        }

                        memcpy(ca_entry->ca_value, &ca[2], ca[1]);

                        if (lci_length < (2 + ca_entry->ca_length))
                        {
                           free(ca_entry);
                           break;
                        }

                        /* insert to the ca_list */
                        if(!L_SORT_LST_Set(&port_config->lldp_med_location.civic_addr.ca_list, &ca_entry))
                        free(ca_entry);
                        lci_length -= (2 + ca[1]);
                        ca += (2 + ca[1]);
                    }
                }
                else
                {
                    LLDP_MGR_InitXMedLocLocationEntry(VAL_lldpXMedLocLocationSubtype_civicAddress, &port_config->lldp_med_location, TRUE);
                }

                if (port_config->med_device_exist)
                    port_config->something_changed_local = TRUE;

                result = TRUE;
                break;
            }

            case VAL_lldpXMedLocLocationSubtype_elin: /* elin */
#if 0 // wakka: not support now
                if (loc_location_entry->location_info_len == 0)
                {
                    port_config->lldp_med_location.location_type_valid &= ~ BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin);
                    port_config->lldp_med_location.elin_addr.elin_len = 0;
                }
                else
                {
                    port_config->lldp_med_location.location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin);
                    port_config->lldp_med_location.elin_addr.elin_len = loc_location_entry->location_info_len;
                    memcpy(port_config->lldp_med_location.elin_addr.elin,
                        loc_location_entry->location_info,
                        loc_location_entry->location_info_len);
                }
                result = TRUE;
#endif
                break;
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p)
{
    BOOL_T                      result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        *status_p = !!(port_config->lldp_med_location.location_type_valid & BIT_VALUE(location_type));

        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 *            status
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T status)
{
    BOOL_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = FALSE;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        if (status == !!(port_config->lldp_med_location.location_type_valid & BIT_VALUE(location_type)))
        {
            result = TRUE;
        }
        else
        {
            if (status)
            {
                result = LLDP_MGR_InitXMedLocLocationEntry(location_type, &port_config->lldp_med_location, FALSE);
            }
            else
            {
                switch (location_type)
                {
                    case VAL_lldpXMedLocLocationSubtype_coordinateBased:
                        port_config->lldp_med_location.location_type_valid &= ~ BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased);
                        result = TRUE;
                        break;
                    case VAL_lldpXMedLocLocationSubtype_civicAddress:
                        port_config->lldp_med_location.location_type_valid &= ~ BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress);
                        if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element!= 0)
                            LLDP_OM_FreeCivicAddrCaList(&port_config->lldp_med_location.civic_addr.ca_list);
                        result = TRUE;
                        break;
                    case VAL_lldpXMedLocLocationSubtype_elin:
                        port_config->lldp_med_location.location_type_valid &= ~ BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin);
                        result = TRUE;
                        break;
                }
            }

            if ((result == TRUE) && (port_config->med_device_exist))
            {
                port_config->something_changed_local = TRUE;
            }
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationStatus
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local location status
 * INPUT    : lport
 *            location_type
 * OUTPUT   : status_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationStatus(UI32_T lport, UI32_T location_type, BOOL_T *status_p)
{
    UI32_T                      result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config;
    BOOL_T                      status = FALSE;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        status = !!(port_config->lldp_med_location.location_type_valid & BIT_VALUE(location_type));

        result = (status == (location_type == LLDP_TYPE_DEFAULT_MED_LOCATION_TYPE)) ?
                    SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE :
                    SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    LLDP_OM_LeaveCriticalSection();

    if (result != SYS_TYPE_GET_RUNNING_CFG_FAIL)
        *status_p = status;

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
        {
            memcpy(country_code, &port_config->lldp_med_location.civic_addr.country_code, 2);
            result = TRUE;
        }
    }
    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    BOOL_T                      result;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (LLDP_MGR_IsCountryCodeValid(country_code) == FALSE)
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    result = FALSE;
    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        LLDP_MGR_InitXMedLocLocationEntry(VAL_lldpXMedLocLocationSubtype_civicAddress, &port_config->lldp_med_location, FALSE);
        if (memcmp(port_config->lldp_med_location.civic_addr.country_code, country_code, 2) != 0)
        {
            memcpy(port_config->lldp_med_location.civic_addr.country_code, country_code, 2);

            if (port_config->med_device_exist)
                port_config->something_changed_local = TRUE;
        }
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationCivicAddrCoutryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : country_code.
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationCivicAddrCoutryCode(UI32_T lport, UI8_T *country_code)
{
    UI32_T                      result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
        {
            memcpy(country_code, &port_config->lldp_med_location.civic_addr.country_code, 2);
            result = memcmp(country_code, LLDP_TYPE_DEFAULT_MED_LOCATION_CA_CONNTRY, 2) ?
                        SYS_TYPE_GET_RUNNING_CFG_SUCCESS :
                        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
    }
    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport
 * OUTPUT   : what
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(lport))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
    {
        *what = port_config->lldp_med_location.civic_addr.what;
    result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Set civic addr what value.
 * INPUT    : lport, what
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T what)
{
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if(!LLDP_UTY_LogicalPortExisting(lport))
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    LLDP_MGR_InitXMedLocLocationEntry(VAL_lldpXMedLocLocationSubtype_civicAddress, &port_config->lldp_med_location, FALSE);
    if (port_config->lldp_med_location.civic_addr.what != what)
    {
        port_config->lldp_med_location.civic_addr.what = what;

        if (port_config->med_device_exist)
            port_config->something_changed_local = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXMedLocLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr what value.
 * INPUT    : lport
 * OUTPUT   : what
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_MGR_GetRunningXMedLocLocationCivicAddrWhat(UI32_T lport, UI8_T *what)
{
    UI32_T                      result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(lport))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress))
    {
        *what = port_config->lldp_med_location.civic_addr.what;
        result = *what == LLDP_TYPE_DEFAULT_MED_LOCATION_CA_WHAT ?
                    SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE :
                    SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *om_ca_tlv;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(lport))
    {
        return result;
    }
    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if ((port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress)) &&
        port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element!= 0 &&
        L_SORT_LST_Get_1st(&port_config->lldp_med_location.civic_addr.ca_list, &om_ca_tlv))
    {
        memcpy(ca_entry, om_ca_tlv, sizeof(LLDP_MGR_XMedLocationCivicAddrCaEntry_T));
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport
 * OUTPUT   : ca_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_PortConfigEntry_T   *port_config;
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *om_ca_tlv;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(lport))
    {
        return result;
    }
    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    om_ca_tlv = (LLDP_OM_XMedLocationCivicAddrCaTlv_T *)ca_entry;
    if ((port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress)) &&
        port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element!= 0 &&
        L_SORT_LST_Get_Next(&port_config->lldp_med_location.civic_addr.ca_list, &om_ca_tlv))
    {
        memcpy(ca_entry, om_ca_tlv, sizeof(LLDP_MGR_XMedLocationCivicAddrCaEntry_T));
        result = TRUE;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get civic addr country code.
 * INPUT    : lport, ca_entry, set_or_unset
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry(UI32_T lport, LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry, BOOL_T set_or_unset)
{
    LLDP_OM_PortConfigEntry_T               *port_config;
    LLDP_OM_XMedLocationCivicAddrCaTlv_T    *om_ca_tlv, *tmp_ca_tlv;
    BOOL_T                                  ca_list_existed;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (!LLDP_UTY_LogicalPortExisting(lport))
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    ca_list_existed = (port_config->lldp_med_location.location_type_valid & BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress)) &&
                      (port_config->lldp_med_location.civic_addr.ca_list.max_element_count > 0);

    om_ca_tlv = (LLDP_OM_XMedLocationCivicAddrCaTlv_T *)malloc(sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));

    /* copy the parameters */
    memcpy(om_ca_tlv, ca_entry, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
    tmp_ca_tlv = om_ca_tlv;

    /* if set */
    if (set_or_unset)
    {
        LLDP_MGR_InitXMedLocLocationEntry(VAL_lldpXMedLocLocationSubtype_civicAddress, &port_config->lldp_med_location, FALSE);

        /* if the ca_list haven't not been used, create a list*/
        if (!ca_list_existed)
        {
            LLDP_OM_CreateCivicAddrCaList(&port_config->lldp_med_location.civic_addr.ca_list);
        }

        /* if this ca_entry is already exist */
        if (L_SORT_LST_Get(&port_config->lldp_med_location.civic_addr.ca_list, &om_ca_tlv))
        {
            /* copy the content */
            memcpy(om_ca_tlv, tmp_ca_tlv, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
            free(tmp_ca_tlv);
        }
        else
        {
            if (L_SORT_LST_Set(
                    &port_config->lldp_med_location.civic_addr.ca_list,
                    &om_ca_tlv) == FALSE)
            {
                free(tmp_ca_tlv);
                LLDP_OM_LeaveCriticalSection();
                return FALSE;
            }

            if (port_config->med_device_exist)
                port_config->something_changed_local = TRUE;
        }
    }
    else /* else unset */
    {
        if (ca_list_existed)
        {
            if (port_config->lldp_med_location.civic_addr.ca_list.nbr_of_element != 0)
            {
                /* get the specify entry */
                if (L_SORT_LST_Get(&port_config->lldp_med_location.civic_addr.ca_list,
                                  &om_ca_tlv))
                {
                    /* delete it from sort list */
                    L_SORT_LST_Delete(&port_config->lldp_med_location.civic_addr.ca_list, &om_ca_tlv);

                    /* free its real memory */
                    free(om_ca_tlv);

                    if (port_config->med_device_exist)
                        port_config->something_changed_local = TRUE;
                }
            }
        }

        /* free the original malloc memory */
        free(tmp_ca_tlv);
    }

    LLDP_OM_LeaveCriticalSection();
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry)
{
    BOOL_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    result = FALSE;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_POE == TRUE)
    if (    (LLDP_UTY_LogicalPortExisting(loc_poe_pse_port_entry->lport) == TRUE)
         && (SWCTRL_LogicalPortIsTrunkPort(loc_poe_pse_port_entry->lport) == FALSE)
         && (STKTPLG_OM_IsLocalPoeDevice() == TRUE)
       )
    {
        UI32_T  unit, port, trunk_id;
        POE_OM_PsePort_T   pse_port_entry;

        SWCTRL_LogicalPortToUserPort(loc_poe_pse_port_entry->lport, &unit, &port, &trunk_id);
        if (POE_POM_GetPsePortEntry(unit, port, &pse_port_entry))
        {
            loc_poe_pse_port_entry->power_av = pse_port_entry.pse_port_power_max_allocation;
            loc_poe_pse_port_entry->pd_priority = pse_port_entry.pse_port_power_priority;
            result = TRUE;
        }
    }
#endif /* #if (SYS_CPNT_POE == TRUE) */

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedLocXPoePsePortEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local port poe/pse entry
 * INPUT    : None
 * OUTPUT   : loc_poe_pse_port_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedLocXPoePsePortEntry(LLDP_MGR_XMedLocXPoePsePortEntry_T *loc_poe_pse_port_entry)
{
    BOOL_T result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    result = FALSE;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_POE == TRUE)
    if (    (LLDP_UTY_LogicalPortExisting(loc_poe_pse_port_entry->lport) == TRUE)
         && (SWCTRL_LogicalPortIsTrunkPort(loc_poe_pse_port_entry->lport) == FALSE)
         && (STKTPLG_OM_IsLocalPoeDevice() == TRUE)
       )
    {
        UI32_T  unit, port, trunk_id;
        POE_OM_PsePort_T   pse_port_entry;

        SWCTRL_LogicalPortToUserPort(loc_poe_pse_port_entry->lport, &unit, &port, &trunk_id);
        if (POE_POM_GetPsePortEntry(unit, port, &pse_port_entry))
        {
            loc_poe_pse_port_entry->power_av = pse_port_entry.pse_port_power_max_allocation;
            loc_poe_pse_port_entry->pd_priority = pse_port_entry.pse_port_power_priority;
            result = TRUE;
        }
    }
#endif /* #if (SYS_CPNT_POE == TRUE) */

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocXPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local poe entry
 * INPUT    : None
 * OUTPUT   : loc_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocXPoeEntry(LLDP_MGR_XMedLocXPoeEntry_T *loc_poe_entry)
{
    BOOL_T result;


    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    result = FALSE;

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

#if (SYS_CPNT_POE == TRUE)
    if (STKTPLG_OM_IsLocalPoeDevice() == FALSE)
    {
        loc_poe_entry->device_type = VAL_lldpXMedLocXPoEDeviceType_none;
        loc_poe_entry->pse_power_source = VAL_lldpXMedLocXPoEPSEPowerSource_unknown;
        loc_poe_entry->pd_power_req = MIN_lldpXMedLocXPoEPDPowerReq;
        loc_poe_entry->pd_power_source = VAL_lldpXMedLocXPoEPDPowerSource_unknown;
        loc_poe_entry->pd_power_priority = VAL_lldpXMedLocXPoEPSEPortPDPriority_unknown;
        result = TRUE;
    }
    else
    {
        UI32_T              unit = 0;
        POE_OM_MainPse_T    main_pse;

        if (POE_POM_GetNextMainPseEntry(&unit, &main_pse))
        {
            loc_poe_entry->device_type = VAL_lldpXMedLocXPoEDeviceType_pseDevice;
            loc_poe_entry->pse_power_source = VAL_lldpXMedLocXPoEPSEPowerSource_primary;
            result = TRUE;
        }
    }
#else
    loc_poe_entry->device_type = VAL_lldpXMedLocXPoEDeviceType_none;
    loc_poe_entry->pse_power_source = VAL_lldpXMedLocXPoEPSEPowerSource_unknown;
    loc_poe_entry->pd_power_req = MIN_lldpXMedLocXPoEPDPowerReq;
    loc_poe_entry->pd_power_source = VAL_lldpXMedLocXPoEPDPowerSource_unknown;
    loc_poe_entry->pd_power_priority = VAL_lldpXMedLocXPoEPSEPortPDPriority_unknown;
    result = TRUE;
#endif /* #if (SYS_CPNT_POE == TRUE) */

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedLocInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED local inventory entry
 * INPUT    : None
 * OUTPUT   : loc_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedLocInventoryEntry(LLDP_MGR_XMedLocInventory_T *loc_inventory_entry)
{
    UI32_T                  tmp_len;
    BOOL_T                  result = FALSE;
    int                     i;
    STKTPLG_OM_EntPhysicalEntry_T  *ent_physical_entry_for_container_p = NULL;
    STKTPLG_OM_EntPhysicalEntry_T  *ent_physical_entry_for_module_p = NULL;
    STKTPLG_OM_EntPhysicalEntry_T  ent_physical_entry[2];

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    memset(&ent_physical_entry, 0, sizeof(ent_physical_entry));

    for (i = 0;
        i < sizeof(ent_physical_entry)/sizeof(*ent_physical_entry) &&
            STKTPLG_OM_GetNextEntPhysicalEntry(&ent_physical_entry[i]); )
    {
        if (ent_physical_entry_for_container_p == NULL &&
            ent_physical_entry[i].ent_physical_class == VAL_entPhysicalClass_container)
        {
            ent_physical_entry_for_container_p = &ent_physical_entry[i++];
            continue;
        }

        if (ent_physical_entry_for_module_p == NULL &&
            ent_physical_entry[i].ent_physical_class == VAL_entPhysicalClass_module)
        {
            ent_physical_entry_for_module_p = &ent_physical_entry[i++];
            continue;
        }
    }
    if (i < sizeof(ent_physical_entry)/sizeof(*ent_physical_entry))
    {
        return result;
    }

    if((tmp_len = strlen((char*)ent_physical_entry_for_container_p->ent_physical_hardware_rev)) > LLDP_TYPE_MAX_HARDWARE_REV_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_HARDWARE_REV_LEN;
    }
    loc_inventory_entry->loc_hardware_rev_len = tmp_len;
    memcpy(loc_inventory_entry->loc_hardware_rev, ent_physical_entry_for_container_p->ent_physical_hardware_rev, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_module_p->ent_physical_firmware_rev)) > LLDP_TYPE_MAX_FIRMWARE_REV_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_FIRMWARE_REV_LEN;
    }
    loc_inventory_entry->loc_firmware_rev_len = tmp_len;
    memcpy(loc_inventory_entry->loc_firmware_rev, ent_physical_entry_for_module_p->ent_physical_firmware_rev, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_module_p->ent_physical_software_rev)) > LLDP_TYPE_MAX_SOFTWARE_REV_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_SOFTWARE_REV_LEN;
    }
    loc_inventory_entry->loc_software_rev_len = tmp_len;
    memcpy(loc_inventory_entry->loc_software_rev, ent_physical_entry_for_module_p->ent_physical_software_rev, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_serial_num)) > LLDP_TYPE_MAX_SERIAL_NUM_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_SERIAL_NUM_LEN;
    }
    loc_inventory_entry->loc_serial_num_len = tmp_len;
    memcpy(loc_inventory_entry->loc_serial_num, ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_serial_num, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_container_p->ent_physical_mfg_name)) > LLDP_TYPE_MAX_MFG_NAME_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_MFG_NAME_LEN;
    }
    loc_inventory_entry->loc_mfg_name_len = tmp_len;
    memcpy(loc_inventory_entry->loc_mfg_name, ent_physical_entry_for_container_p->ent_physical_mfg_name, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_container_p->ent_physical_model_name)) > LLDP_TYPE_MAX_MODEL_NAME_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_MODEL_NAME_LEN;
    }
    loc_inventory_entry->loc_model_name_len = tmp_len;
    memcpy(loc_inventory_entry->loc_model_name, ent_physical_entry_for_container_p->ent_physical_model_name, tmp_len);

    if((tmp_len = strlen((char*)ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_asset_id)) > LLDP_TYPE_MAX_ASSET_ID_LEN)
    {
        tmp_len = LLDP_TYPE_MAX_ASSET_ID_LEN;
    }
    loc_inventory_entry->loc_asset_id_len = tmp_len;
    memcpy(loc_inventory_entry->loc_asset_id, ent_physical_entry_for_container_p->ent_physical_entry_rw.ent_physical_asset_id, tmp_len);

    result = TRUE;

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemCapEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device capability entry
 * INPUT    : None
 * OUTPUT   : rem_cap_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetNextXMedRemCapEntry(LLDP_MGR_XMedRemCapEntry_T *rem_cap_entry)
{
    BOOL_T             result = FALSE;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_cap_entry->rem_time_mark;
    rem_data.lport     = rem_cap_entry->rem_local_port_num;
    rem_data.index     = rem_cap_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while(LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->lldp_med_device_type != 0)
        {
            rem_cap_entry->rem_time_mark      = rem_data_p->time_mark;
            rem_cap_entry->rem_local_port_num = rem_data_p->lport;
            rem_cap_entry->rem_index          = rem_data_p->index;
            rem_cap_entry->rem_device_class   = rem_data_p->lldp_med_device_type;
            rem_cap_entry->rem_cap_supported  = rem_data_p->lldp_med_cap_sup;
            rem_cap_entry->rem_cap_current    = rem_data_p->lldp_med_cap_enabled;
            result = TRUE;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemMediaPolicyEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemMediaPolicyEntry(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       rem_data, *rem_data_p;
    UI8_T                   app_type;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    LLDP_OM_EnterCriticalSection();

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_med_policy_entry->rem_time_mark;
    rem_data.lport     = rem_med_policy_entry->rem_local_port_num;
    rem_data.index     = rem_med_policy_entry->rem_index;

    if (LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_network_policy)
        {
            for (app_type = rem_med_policy_entry->rem_app_type+1;
                 app_type <= LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE;
                 app_type++)
            {
                if (rem_data_p->med_rem_network_policy->app_type[app_type-1].valid)
                {
                    result = TRUE;
                    break;
                }
            }
        }
    }

    if (!result)
    {
        while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if (rem_data_p->med_rem_network_policy)
            {
                for (app_type = VAL_lldpXMedRemMediaPolicyAppType_voice;
                     app_type <= LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE;
                     app_type++)
                {
                    if (rem_data_p->med_rem_network_policy->app_type[app_type-1].valid)
                    {
                        result = TRUE;
                        break;
                    }
                }

                if (result)
                    break;
            }
        }
    }

    if (result)
    {
        rem_med_policy_entry->rem_time_mark      = rem_data_p->time_mark;
        rem_med_policy_entry->rem_local_port_num = rem_data_p->lport;
        rem_med_policy_entry->rem_index          = rem_data_p->index;
        rem_med_policy_entry->rem_app_type       = app_type;
        rem_med_policy_entry->rem_dscp           = rem_data_p->med_rem_network_policy->app_type[app_type-1].dscp;
        rem_med_policy_entry->rem_priority       = rem_data_p->med_rem_network_policy->app_type[app_type-1].priority;
        rem_med_policy_entry->rem_tagged         = (rem_data_p->med_rem_network_policy->app_type[app_type-1].tagged)? VAL_lldpXMedRemMediaPolicyTagged_true : VAL_lldpXMedRemMediaPolicyTagged_false;
        rem_med_policy_entry->rem_unknown        = (rem_data_p->med_rem_network_policy->app_type[app_type-1].unknown)? VAL_lldpXMedRemMediaPolicyUnknown_true : VAL_lldpXMedRemMediaPolicyUnknown_false;
        rem_med_policy_entry->rem_vid            = rem_data_p->med_rem_network_policy->app_type[app_type-1].vid;
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device media policy entry by lport, index and app_type
 *            (i.e. without timemark)
 * INPUT    : None
 * OUTPUT   : rem_med_policy_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex(LLDP_MGR_XMedRemMediaPolicyEntry_T *rem_med_policy_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;
    UI8_T                   app_type;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if (rem_med_policy_entry->rem_app_type < VAL_lldpXMedRemMediaPolicyAppType_voice ||
        rem_med_policy_entry->rem_app_type > LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE)
    {
        return FALSE;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(rem_med_policy_entry->rem_local_port_num,
                          rem_med_policy_entry->rem_index,
                          &rem_data))
    {
        if(rem_data->med_rem_network_policy)
        {
            app_type = rem_med_policy_entry->rem_app_type;
            if (rem_data->med_rem_network_policy->app_type[app_type-1].valid)
            {
                rem_med_policy_entry->rem_dscp     = rem_data->med_rem_network_policy->app_type[app_type-1].dscp;
                rem_med_policy_entry->rem_priority = rem_data->med_rem_network_policy->app_type[app_type-1].priority;
                rem_med_policy_entry->rem_tagged   = (rem_data->med_rem_network_policy->app_type[app_type-1].tagged)? VAL_lldpXMedRemMediaPolicyTagged_true : VAL_lldpXMedRemMediaPolicyTagged_false;
                rem_med_policy_entry->rem_unknown  = (rem_data->med_rem_network_policy->app_type[app_type-1].unknown)? VAL_lldpXMedRemMediaPolicyUnknown_true : VAL_lldpXMedRemMediaPolicyUnknown_false;
                rem_med_policy_entry->rem_vid      = rem_data->med_rem_network_policy->app_type[app_type-1].vid;
                result = TRUE;
            }
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemInventoryEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory entry
 * INPUT    : None
 * OUTPUT   : rem_inventory_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemInventoryEntry(LLDP_MGR_XMedRemInventoryEntry_T *rem_inventory_entry)
{
    BOOL_T            result = FALSE;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_inventory_entry->rem_time_mark;
    rem_data.lport     = rem_inventory_entry->rem_local_port_num;
    rem_data.index     = rem_inventory_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_inventory)
        {
            rem_inventory_entry->rem_time_mark      = rem_data_p->time_mark;
            rem_inventory_entry->rem_local_port_num = rem_data_p->lport;
            rem_inventory_entry->rem_index          = rem_data_p->index;

            rem_inventory_entry->rem_asset_id_len = rem_data_p->med_rem_inventory->asset_id_len;
            memcpy(rem_inventory_entry->rem_asset_id,
                   rem_data_p->med_rem_inventory->asset_id,
                   rem_inventory_entry->rem_asset_id_len);

            rem_inventory_entry->rem_firmware_rev_len = rem_data_p->med_rem_inventory->firmware_revision_len;
            memcpy(rem_inventory_entry->rem_firmware_rev,
                   rem_data_p->med_rem_inventory->firmware_revision,
                   rem_inventory_entry->rem_firmware_rev_len);

            rem_inventory_entry->rem_hardware_rev_len = rem_data_p->med_rem_inventory->hardware_revision_len;
            memcpy(rem_inventory_entry->rem_hardware_rev,
                   rem_data_p->med_rem_inventory->hardware_revision,
                   rem_inventory_entry->rem_hardware_rev_len);

            rem_inventory_entry->rem_mfg_name_len = rem_data_p->med_rem_inventory->manufaturer_name_len;
            memcpy(rem_inventory_entry->rem_mfg_name,
                   rem_data_p->med_rem_inventory->manufaturer_name,
                   rem_inventory_entry->rem_mfg_name_len);

            rem_inventory_entry->rem_model_name_len = rem_data_p->med_rem_inventory->model_name_len;
            memcpy(rem_inventory_entry->rem_model_name,
                   rem_data_p->med_rem_inventory->model_name,
                   rem_inventory_entry->rem_model_name_len);

            rem_inventory_entry->rem_serial_num_len = rem_data_p->med_rem_inventory->serial_num_len;
            memcpy(rem_inventory_entry->rem_serial_num,
                   rem_data_p->med_rem_inventory->serial_num,
                   rem_inventory_entry->rem_serial_num_len);

            rem_inventory_entry->rem_software_rev_len = rem_data_p->med_rem_inventory->software_revision_len;
            memcpy(rem_inventory_entry->rem_software_rev,
                   rem_data_p->med_rem_inventory->software_revision,
                   rem_inventory_entry->rem_software_rev_len);

            result = TRUE;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemInventoryItem
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory information
 * INPUT    : None
 * OUTPUT   : buffer, len
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetXMedRemInventoryItem(UI32_T field_id,
                                         LLDP_MGR_RemDataIndex_T *rem_data_index,
                                         UI8_T *buffer,
                                         UI32_T buffer_size,
                                         UI32_T *used_size)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(rem_data_index->rem_local_port_num,
                          rem_data_index->rem_index,
                          &rem_data))
    {
        if(rem_data->med_rem_inventory)
        {
            switch(field_id)
            {
#ifdef SYS_TYPE_FID_ALL
                case SYS_TYPE_FID_ALL:
#else
                case 0:
#endif
                    if(buffer_size >= sizeof(LLDP_OM_XMedRemInventoryEntry_T))
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory,
                               sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                        *used_size = sizeof(LLDP_OM_XMedRemInventoryEntry_T);
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_HARDWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->hardware_revision,
                               rem_data->med_rem_inventory->hardware_revision_len);
                        *used_size = rem_data->med_rem_inventory->hardware_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_FRIMWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->firmware_revision,
                               rem_data->med_rem_inventory->firmware_revision_len);
                        *used_size = rem_data->med_rem_inventory->firmware_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_SOFTWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->software_revision,
                               rem_data->med_rem_inventory->software_revision_len);
                        *used_size = rem_data->med_rem_inventory->software_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_SERIAL_NUM:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->serial_num,
                               rem_data->med_rem_inventory->serial_num_len);
                        *used_size = rem_data->med_rem_inventory->serial_num_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_MFG_NAME:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->manufaturer_name,
                               rem_data->med_rem_inventory->manufaturer_name_len);
                        *used_size = rem_data->med_rem_inventory->manufaturer_name_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_MODEL_NAME:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->model_name,
                               rem_data->med_rem_inventory->model_name_len);
                        *used_size = rem_data->med_rem_inventory->model_name_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_ASSET_ID:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data->med_rem_inventory->asset_id,
                               rem_data->med_rem_inventory->asset_id_len);
                        *used_size = rem_data->med_rem_inventory->asset_id_len;
                        result = TRUE;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemInventoryItem
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device inventory information
 * INPUT    : None
 * OUTPUT   : buffer, len
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemInventoryItem(UI32_T field_id,
                                             LLDP_MGR_RemDataIndex_T *rem_data_index,
                                             UI8_T *buffer,
                                             UI32_T buffer_size,
                                             UI32_T *used_size)
{
    BOOL_T             result = FALSE;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_data_index->rem_time_mark;
    rem_data.lport     = rem_data_index->rem_local_port_num;
    rem_data.index     = rem_data_index->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_inventory)
        {
            switch(field_id)
            {
#ifdef SYS_TYPE_FID_ALL
                case SYS_TYPE_FID_ALL:
#else
                case 0:
#endif
                    if(buffer_size >= sizeof(LLDP_OM_XMedRemInventoryEntry_T))
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory,
                               sizeof(LLDP_OM_XMedRemInventoryEntry_T));
                        *used_size = sizeof(LLDP_OM_XMedRemInventoryEntry_T);
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_HARDWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->hardware_revision,
                               rem_data_p->med_rem_inventory->hardware_revision_len);
                        *used_size = rem_data_p->med_rem_inventory->hardware_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_FRIMWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->firmware_revision,
                               rem_data_p->med_rem_inventory->firmware_revision_len);
                        *used_size = rem_data_p->med_rem_inventory->firmware_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_SOFTWARE_REV:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->software_revision,
                               rem_data_p->med_rem_inventory->software_revision_len);
                        *used_size = rem_data_p->med_rem_inventory->software_revision_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORY_SERIAL_NUM:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->serial_num,
                               rem_data_p->med_rem_inventory->serial_num_len);
                        *used_size = rem_data_p->med_rem_inventory->serial_num_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_MFG_NAME:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->manufaturer_name,
                               rem_data_p->med_rem_inventory->manufaturer_name_len);
                        *used_size = rem_data_p->med_rem_inventory->manufaturer_name_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_MODEL_NAME:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->model_name,
                               rem_data_p->med_rem_inventory->model_name_len);
                        *used_size = rem_data_p->med_rem_inventory->model_name_len;
                        result = TRUE;
                    }
                    break;
                case LLDP_MGR_FID_REM_INVENTORT_ASSET_ID:
                    if(buffer_size >= LLDP_TYPE_MAX_INV_ITEM_LEN)
                    {
                        memcpy(buffer,
                               rem_data_p->med_rem_inventory->asset_id,
                               rem_data_p->med_rem_inventory->asset_id_len);
                        *used_size = rem_data_p->med_rem_inventory->asset_id_len;
                        result = TRUE;
                    }
                    break;
                default:
                    break;
            }

            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used only for SNMP.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemLocationEntry(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    BOOL_T            result = FALSE;
    LLDP_OM_RemData_T rem_data, *rem_data_p;
    UI32_T            location_subtype;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_location_entry->rem_time_mark;
    rem_data.lport     = rem_location_entry->rem_local_port_num;
    rem_data.index     = rem_location_entry->rem_index;

    location_subtype   = rem_location_entry->rem_location_subtype;

    LLDP_OM_EnterCriticalSection();

    if (LLDP_OM_GetRemDataWithTimeMark(&rem_data_p))
    {
        if (rem_data_p->med_rem_location &&
            rem_data_p->med_rem_location->location_type_valid > (BIT_VALUE(location_subtype+1)-1))
            result = TRUE;
    }

    if (!result)
    {
        while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
        {
            if (rem_data_p->med_rem_location &&
                rem_data_p->med_rem_location->location_type_valid > 0)
            {
                rem_location_entry->rem_time_mark      = rem_data_p->time_mark;
                rem_location_entry->rem_local_port_num = rem_data_p->lport;
                rem_location_entry->rem_index          = rem_data_p->index;

                location_subtype = 0;
                result = TRUE;
                break;
            }
        }
    }

    if (result)
    {
        if (location_subtype < VAL_lldpXMedRemLocationSubtype_coordinateBased &&
            rem_data_p->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_coordinateBased))
        {
#if 0 // TODO
            rem_location_entry->rem_location_subtype = 1;
#endif
        }
        else if (location_subtype < VAL_lldpXMedRemLocationSubtype_civicAddress &&
            rem_data_p->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress))
        {
            LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
            UI16_T cur_loc_len=0;
            UI8_T   *ca;

            rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_civicAddress;
            rem_location_entry->rem_location_info[1] = rem_data_p->med_rem_location->civic_addr.what;
            memcpy(&rem_location_entry->rem_location_info[2],
               rem_data_p->med_rem_location->civic_addr.country_code, 2);

            ca = &rem_location_entry->rem_location_info[4];

            if (rem_data_p->med_rem_location->civic_addr.ca_list.nbr_of_element != 0 &&
                L_SORT_LST_Get_1st(&rem_data_p->med_rem_location->civic_addr.ca_list, &ca_entry))
            {
                do
                {
                    cur_loc_len+=ca_entry->ca_length+2;

                    if(cur_loc_len > (LLDP_TYPE_MAX_LOCATION_ID_LEN - 4))
                        break;

                    ca[0] = ca_entry->ca_type;
                    ca[1] = ca_entry->ca_length;
                    memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                    ca += (2 + ca[1]);
                } while (L_SORT_LST_Get_Next(&rem_data_p->med_rem_location->civic_addr.ca_list, &ca_entry));
            }

            rem_location_entry->rem_location_info_len = ca - rem_location_entry->rem_location_info;
            rem_location_entry->rem_location_info[0] = rem_location_entry->rem_location_info_len - 1;
        }
        else if (location_subtype < VAL_lldpXMedRemLocationSubtype_civicAddress &&
            rem_data_p->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin))
        {
            rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_elin;
            rem_location_entry->rem_location_info_len = rem_data_p->med_rem_location->elin_addr.elin_len;
            memcpy(rem_location_entry->rem_location_info, rem_data_p->med_rem_location->elin_addr.elin, rem_data_p->med_rem_location->elin_addr.elin_len);
        }
        else
        {
            /* assert(0);  impossible case. */
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemLocationEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : rem_location_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API is used onlu for WEB/CLI.
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemLocationEntryByIndex(LLDP_MGR_XMedRemLocationEntry_T *rem_location_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if (LLDP_MGR_GetRemData_Local(rem_location_entry->rem_local_port_num,
                                             rem_location_entry->rem_index,
                                             &rem_data))
    {
        if(rem_data->med_rem_location)
        {
            if (rem_location_entry->rem_location_subtype < VAL_lldpXMedRemLocationSubtype_coordinateBased &&
                (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_coordinateBased)))
            {
#if 0 // TODO
                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_coordinateBased;
                result = TRUE;
#endif
            }
            else if (rem_location_entry->rem_location_subtype < VAL_lldpXMedRemLocationSubtype_civicAddress &&
                (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)))
            {
                LLDP_OM_XMedLocationCivicAddrCaTlv_T *ca_entry;
                UI16_T cur_loc_len=0;
                UI8_T   *ca;

                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_civicAddress;
                rem_location_entry->rem_location_info[1] = rem_data->med_rem_location->civic_addr.what;
                 memcpy(&rem_location_entry->rem_location_info[2],
                rem_data->med_rem_location->civic_addr.country_code, 2);

                ca = &rem_location_entry->rem_location_info[4];

                if (rem_data->med_rem_location->civic_addr.ca_list.nbr_of_element != 0 &&
                    L_SORT_LST_Get_1st(&rem_data->med_rem_location->civic_addr.ca_list, &ca_entry))
                {
                    do
                    {
                        cur_loc_len+=ca_entry->ca_length+2;

                        if(cur_loc_len > (LLDP_TYPE_MAX_LOCATION_ID_LEN - 4))
                            break;

                        ca[0] = ca_entry->ca_type;
                        ca[1] = ca_entry->ca_length;
                        memcpy(&ca[2], ca_entry->ca_value, ca[1]);
                        ca += (2 + ca[1]);
                    } while (L_SORT_LST_Get_Next(&rem_data->med_rem_location->civic_addr.ca_list, &ca_entry));
                }

                rem_location_entry->rem_location_info_len = ca - rem_location_entry->rem_location_info;
                rem_location_entry->rem_location_info[0] = rem_location_entry->rem_location_info_len - 1;
                    result = TRUE;
            }
            else if (rem_location_entry->rem_location_subtype < VAL_lldpXMedRemLocationSubtype_elin &&
                (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin)))
            {
                rem_location_entry->rem_location_subtype = VAL_lldpXMedRemLocationSubtype_elin;
                rem_location_entry->rem_location_info_len = rem_data->med_rem_location->elin_addr.elin_len;
                memcpy(rem_location_entry->rem_location_info, rem_data->med_rem_location->elin_addr.elin, rem_data->med_rem_location->elin_addr.elin_len);
                result = TRUE;
            }

        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCountryCode
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationCivicAddrCountryCode(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       UI8_T *country_code)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(rem_loc_port_num))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    if(LLDP_OM_GetRemData(rem_loc_port_num, rem_index, &rem_data))
    {
        if (rem_data->med_rem_location &&
            (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)))
        {
            memcpy(country_code, rem_data->med_rem_location->civic_addr.country_code, 2);
            result = TRUE;
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrWhat
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationCivicAddrWhat(UI32_T rem_loc_port_num,
                                                UI32_T rem_index,
                                                UI8_T *what)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(rem_loc_port_num))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    if(LLDP_OM_GetRemData(rem_loc_port_num, rem_index, &rem_data))
    {
        if (rem_data->med_rem_location &&
            (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)))
        {
            *what = rem_data->med_rem_location->civic_addr.what;
            result = TRUE;
        }
    }


    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_Get1stXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                      UI32_T rem_index,
                                                      LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *om_ca_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(rem_loc_port_num))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    if(LLDP_OM_GetRemData(rem_loc_port_num, rem_index, &rem_data))
    {
        if (rem_data->med_rem_location &&
            (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)) &&
            rem_data->med_rem_location->civic_addr.ca_list.nbr_of_element != 0)
        {
            if(L_SORT_LST_Get_1st(&rem_data->med_rem_location->civic_addr.ca_list, &om_ca_entry))
            {
                memcpy(ca_entry, om_ca_entry, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
                result = TRUE;
            }
        }
    }


    LLDP_OM_LeaveCriticalSection();
    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationCivicAddrCaEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location entry
 * INPUT    : None
 * OUTPUT   : country_code
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetNextXMedRemLocationCivicAddrCaEntry(UI32_T rem_loc_port_num,
                                                       UI32_T rem_index,
                                                       LLDP_MGR_XMedLocationCivicAddrCaEntry_T *ca_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_XMedLocationCivicAddrCaTlv_T *om_ca_entry;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(rem_loc_port_num))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    /* get the remote data*/
    if(LLDP_OM_GetRemData(rem_loc_port_num, rem_index, &rem_data))
    {
        /* if rem data has the location information*/
        if (rem_data->med_rem_location &&
            (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_civicAddress)) &&
            rem_data->med_rem_location->civic_addr.ca_list.nbr_of_element != 0)
        {
            om_ca_entry = (LLDP_OM_XMedLocationCivicAddrCaTlv_T *)ca_entry;

            if(L_SORT_LST_Get_Next(&rem_data->med_rem_location->civic_addr.ca_list, &om_ca_entry))
            {
                memcpy(ca_entry, om_ca_entry, sizeof(LLDP_OM_XMedLocationCivicAddrCaTlv_T));
                result = TRUE;
            }
        }
    }

    LLDP_OM_LeaveCriticalSection();
    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXMedRemLocationElin
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device location elin entry
 * INPUT    : None
 * OUTPUT   : rem_loc_elin_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : This API are used for CLI/WEB.
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_GetXMedRemLocationElin(UI32_T rem_loc_port_num,
                                       UI32_T rem_index,
                                       LLDP_MGR_XMedLocationElin_T *rem_loc_elin_entry)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if(!LLDP_UTY_LogicalPortExisting(rem_loc_port_num))
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    if(LLDP_OM_GetRemData(rem_loc_port_num, rem_index, &rem_data))
    {
        if (rem_data->med_rem_location &&
            (rem_data->med_rem_location->location_type_valid & BIT_VALUE(VAL_lldpXMedRemLocationSubtype_elin)))
        {
            rem_loc_elin_entry->elin_len = rem_data->med_rem_location->elin_addr.elin_len;
            memcpy(rem_loc_elin_entry->elin,
                rem_data->med_rem_location->elin_addr.elin,
                rem_data->med_rem_location->elin_addr.elin_len);
            result = TRUE;
        }
    }


    LLDP_OM_LeaveCriticalSection();
    return result;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoeEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe entry
 * INPUT    : None
 * OUTPUT   : rem_poe_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoeEntry(LLDP_MGR_XMedRemPoeEntry_T *rem_poe_entry)
{
    BOOL_T            result = FALSE;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_poe_entry->rem_time_mark;
    rem_data.lport     = rem_poe_entry->rem_local_port_num;
    rem_data.index     = rem_poe_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_ext_power)
        {
            rem_poe_entry->rem_time_mark       = rem_data_p->time_mark;
            rem_poe_entry->rem_local_port_num  = rem_data_p->lport;
            rem_poe_entry->rem_index           = rem_data_p->index;
            rem_poe_entry->rem_poe_device_type = rem_data_p->med_rem_ext_power->power_type;
            result = TRUE;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoePseEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pse entry
 * INPUT    : None
 * OUTPUT   : rem_pse_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoePseEntry(LLDP_MGR_XMedRemPoePseEntry_T *rem_pse_entry)
{
    BOOL_T             result = FALSE;
    LLDP_OM_RemData_T  rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_pse_entry->rem_time_mark;
    rem_data.lport     = rem_pse_entry->rem_local_port_num;
    rem_data.index     = rem_pse_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_ext_power && rem_data_p->med_rem_ext_power->power_type == 2)
        {
            rem_pse_entry->rem_time_mark          = rem_data_p->time_mark;
            rem_pse_entry->rem_local_port_num     = rem_data_p->lport;
            rem_pse_entry->rem_index              = rem_data_p->index;
            rem_pse_entry->rem_pse_power_av       = rem_data_p->med_rem_ext_power->power_value;
            rem_pse_entry->rem_pse_power_priority = rem_data_p->med_rem_ext_power->power_priority;
            rem_pse_entry->rem_pse_power_source   = rem_data_p->med_rem_ext_power->power_source;
            result = TRUE;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextXMedRemPoePdEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get LLDP-MED remote device poe pd entry
 * INPUT    : None
 * OUTPUT   : rem_pd_entry
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_GetNextXMedRemPoePdEntry(LLDP_MGR_XMedRemPoePdEntry_T *rem_pd_entry)
{
    BOOL_T            result = FALSE;
    LLDP_OM_RemData_T rem_data, *rem_data_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    rem_data_p = &rem_data;
    rem_data.time_mark = rem_pd_entry->rem_time_mark;
    rem_data.lport     = rem_pd_entry->rem_local_port_num;
    rem_data.index     = rem_pd_entry->rem_index;

    LLDP_OM_EnterCriticalSection();

    while (LLDP_OM_GetNextRemDataWithTimeMark(&rem_data_p))
    {
        if(rem_data_p->med_rem_ext_power && rem_data_p->med_rem_ext_power->power_type == 3)
        {
            rem_pd_entry->rem_time_mark         = rem_data_p->time_mark;
            rem_pd_entry->rem_local_port_num    = rem_data_p->lport;
            rem_pd_entry->rem_index             = rem_data_p->index;
            rem_pd_entry->rem_pd_power_req      = rem_data_p->med_rem_ext_power->power_value;
            rem_pd_entry->rem_pd_power_priority = rem_data_p->med_rem_ext_power->power_priority;
            rem_pd_entry->rem_pd_power_source   = rem_data_p->med_rem_ext_power->power_source;
            result = TRUE;
            break;
        }
    }

    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_InitXMedLocLocationEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set LLDP-MED local location entry to default value
 * INPUT    : location_type
 *            lldp_med_location
 *            force
 * OUTPUT   : lldp_med_location
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T LLDP_MGR_InitXMedLocLocationEntry(UI32_T location_type, LLDP_OM_XMedLocLocationEntry_T *lldp_med_location, BOOL_T force)
{
    BOOL_T result = FALSE;
    BOOL_T existed = !!(lldp_med_location->location_type_valid & BIT_VALUE(location_type));

    if (!existed || force)
    {
        switch (location_type)
        {
            case VAL_lldpXMedLocLocationSubtype_coordinateBased:
                lldp_med_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_coordinateBased);
                memset(&lldp_med_location->coord_addr, 0, sizeof(LLDP_OM_XMedLocationCoordinated_T));
                result = TRUE;
                break;

            case VAL_lldpXMedLocLocationSubtype_civicAddress:
                if (existed && lldp_med_location->civic_addr.ca_list.nbr_of_element != 0)
                    LLDP_OM_FreeCivicAddrCaList(&lldp_med_location->civic_addr.ca_list);
                lldp_med_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_civicAddress);
                lldp_med_location->civic_addr.what = LLDP_TYPE_DEFAULT_MED_LOCATION_CA_WHAT;
                memcpy(lldp_med_location->civic_addr.country_code, LLDP_TYPE_DEFAULT_MED_LOCATION_CA_CONNTRY, 2);
                result = TRUE;
                break;

            case VAL_lldpXMedLocLocationSubtype_elin:
                lldp_med_location->location_type_valid |= BIT_VALUE(VAL_lldpXMedLocLocationSubtype_elin);
                lldp_med_location->elin_addr.elin_len = 0;
                result = TRUE;
                break;
        }
    }
    else
    {
        result = TRUE;
    }

    return result;
}
#endif

#if (SYS_CPNT_ADD == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsTelephoneMac
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the mac address is belonged to a telephone.
 * INPUT    : lport, device_mac, device_mac_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_IsTelephoneMac(UI32_T lport, UI8_T  *device_mac, UI32_T device_mac_len)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;
    L_SORT_LST_List_T       *device_port_list;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* +++ Enter Criticial Section +++*/
    LLDP_OM_EnterCriticalSection();

    if (LLDP_UTY_LogicalPortExisting(lport))
    {
        device_port_list = LLDP_OM_GetPortDeviceList(lport);
        if (L_SORT_LST_Get_1st(device_port_list, &rem_data))
        {
            do
            {
                if ((rem_data->rem_sys_entry->rem_sys_cap_enabled & VAL_lldpRemSysCapEnabled_telephone) == 0)
                    continue;
                if (rem_data->rem_port_id_subtype != LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR)
                    continue;
                if (rem_data->rem_port_id_len != device_mac_len)
                    continue;
                if (memcmp(rem_data->rem_port_id, device_mac, device_mac_len) != 0)
                    continue;

                result = TRUE;
            } while ((result != TRUE) && L_SORT_LST_Get_Next(device_port_list, &rem_data));
        }
    }

    /* +++ Leave critical sectoin +++*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsTelephoneNetworkAddr
 *-------------------------------------------------------------------------
 * PURPOSE  : To determine if the netowrk address is belonged to a telephone.
 * INPUT    : lport, device_network_addr_subtype, device_network_addr, device_network_addr_len
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T  LLDP_MGR_IsTelephoneNetworkAddr(UI32_T lport, UI8_T device_network_addr_subtype, UI8_T  *device_network_addr, UI32_T device_network_addr_len)
{
    BOOL_T                  result = FALSE;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_RemManAddrEntry_T *rem_man_addr;
    L_SORT_LST_List_T       *device_port_list;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    /* +++ Enter Criticial Section +++*/
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(lport))
    {
        device_port_list = LLDP_OM_GetPortDeviceList(lport);
        if(L_SORT_LST_Get_1st(device_port_list, &rem_data))
        {
            do
            {
                if(L_SORT_LST_Get_1st(&rem_data->rem_man_addr_list, &rem_man_addr))
                {
                    if((rem_data->rem_sys_entry->rem_sys_cap_enabled & VAL_lldpRemSysCapEnabled_telephone) == 0)
                        continue;
                    if(rem_man_addr->rem_man_addr_subtype != device_network_addr_subtype)
                        continue;
                    if(rem_man_addr->rem_man_addr_len != device_network_addr_len)
                        continue;
                    if(memcmp(rem_man_addr->rem_man_addr, device_network_addr, device_network_addr_len) != 0)
                        continue;
                    result = TRUE;
                }
            }while(result != TRUE && L_SORT_LST_Get_Next(device_port_list, &rem_data));
        }
    }

    /* +++ Leave critical sectoin +++*/
    LLDP_OM_LeaveCriticalSection();

    return result;
}
#endif /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_DCBX == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_NotifyEtsPfcCfgChanged
 *-------------------------------------------------------------------------
 * PURPOSE  : When ets/pfc cfg is changed, this function will be called.
 * INPUT    : lport       -- the specified port
 *            is_ets_chgd -- TRUE if ets is changed
 *            is_pfc_chgd -- TRUE if pfc is changed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_NotifyEtsPfcCfgChanged(
    UI32_T  lport,
    BOOL_T  is_ets_chgd,
    BOOL_T  is_pfc_chgd)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return;
    }

    /*+++ Enter Critical Section +++*/
    LLDP_OM_EnterCriticalSection();

    if (TRUE == is_ets_chgd)
        LLDP_OM_InsertNotifyDcbxRemEtsChangedList(lport);

    if (TRUE == is_pfc_chgd)
        LLDP_OM_InsertNotifyDcbxRemPfcChangedList(lport);

    /* +++Leaver critical section+++ */
    LLDP_OM_LeaveCriticalSection();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_SetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Set DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_SetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port__config_entry_p)
{
    UI32_T                  result;

    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if (xdcbx_port__config_entry_p == NULL)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_LogicalPortExisting(xdcbx_port__config_entry_p->lport))
    {
        UI8_T                   original_config;

        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdcbx_port__config_entry_p->lport);
        original_config = port_config_p->xdcbx_tlvs_tx_flag;

        if(xdcbx_port__config_entry_p->ets_config_tlv_enable)
        {
            port_config_p->xdcbx_tlvs_tx_flag |= (1 << 0);/* ETS config*/
        }
        else
        {
            port_config_p->xdcbx_tlvs_tx_flag &= (~(1 << 0));
        }

        if(xdcbx_port__config_entry_p->ets_recommend_tlv_enable)
        {
            port_config_p->xdcbx_tlvs_tx_flag |= (1 << 1);/* ETS recommend */
        }
        else
        {
            port_config_p->xdcbx_tlvs_tx_flag &= (~(1 << 1));
        }

        if(xdcbx_port__config_entry_p->pfc_config_tlv_enable)
        {
            port_config_p->xdcbx_tlvs_tx_flag |= (1 << 2); /* PFC config */
        }
        else
        {
            port_config_p->xdcbx_tlvs_tx_flag &= (~(1 << 2));
        }

        if(xdcbx_port__config_entry_p->app_priority_tlv_enable)
        {
            port_config_p->xdcbx_tlvs_tx_flag |= (1 << 3);  /* application priority */
        }
        else
        {
            port_config_p->xdcbx_tlvs_tx_flag &= (~(1 << 3));
        }

        if((original_config ^ port_config_p->xdcbx_tlvs_tx_flag) != 0)
        {
            LLDP_ENGINE_SomethingChangedLocal(xdcbx_port__config_entry_p->lport);
        }

        result = LLDP_TYPE_RETURN_OK;
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_SetXdcbxPortConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p->lport
 * OUTPUT   : xdcbx_port__config_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p)
{
    UI32_T                  result;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    if (xdcbx_port_config_entry_p == NULL)
    {
        return LLDP_TYPE_RETURN_ERROR;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();
    result = LLDP_TYPE_RETURN_ERROR;

    if(LLDP_UTY_LogicalPortExisting(xdcbx_port_config_entry_p->lport))
    {
        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdcbx_port_config_entry_p->lport);

        if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_CON_TLV_TX) != 0 )
        {
            xdcbx_port_config_entry_p->ets_config_tlv_enable = TRUE;
        }
        else
        {
            xdcbx_port_config_entry_p->ets_config_tlv_enable = FALSE;
        }

        if(  (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_RECOM_TLV_TX ) != 0)
        {
            xdcbx_port_config_entry_p->ets_recommend_tlv_enable = TRUE;
        }
        else
        {
            xdcbx_port_config_entry_p->ets_recommend_tlv_enable = FALSE;
        }

        if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_PFC_CON_TLV_TX ) != 0)
        {
            xdcbx_port_config_entry_p->pfc_config_tlv_enable = TRUE;
        }
        else
        {
            xdcbx_port_config_entry_p->pfc_config_tlv_enable = FALSE;
        }

        if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_APP_PRI_TLV_TX ) != 0)
        {
            xdcbx_port_config_entry_p->app_priority_tlv_enable = TRUE;
        }
        else
        {
            xdcbx_port_config_entry_p->app_priority_tlv_enable = FALSE;
        }

        result = LLDP_TYPE_RETURN_OK;

    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetXdcbxPortConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetRunningXdcbxPortConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX port configuraiton
 * INPUT    : xdcbx_port__config_entry_p->lport
 * OUTPUT   : xdcbx_port__config_entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *            SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetRunningXdcbxPortConfig(LLDP_TYPE_XdcbxPortConfigEntry_T *xdcbx_port_config_entry_p)
{
    UI32_T                  result = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    LLDP_OM_PortConfigEntry_T   *port_config_p;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (xdcbx_port_config_entry_p == NULL)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_UTY_LogicalPortExisting(xdcbx_port_config_entry_p->lport))
    {
        port_config_p = LLDP_OM_GetPortConfigEntryPtr(xdcbx_port_config_entry_p->lport);

        xdcbx_port_config_entry_p->tlvs_tx_changed = port_config_p->xdcbx_tlvs_tx_flag ^ LLDP_TYPE_DEFAULT_XDCBX_PORT_CONFIG;
        if (xdcbx_port_config_entry_p->tlvs_tx_changed != 0)
        {
            xdcbx_port_config_entry_p->tlvs_tx_enable = port_config_p->xdcbx_tlvs_tx_flag;

            if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_CON_TLV_TX) != 0 )
            {
                xdcbx_port_config_entry_p->ets_config_tlv_enable = TRUE;
            }
            else
            {
                xdcbx_port_config_entry_p->ets_config_tlv_enable = FALSE;
            }

            if(  (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_ETS_RECOM_TLV_TX ) != 0)
            {
                xdcbx_port_config_entry_p->ets_recommend_tlv_enable = TRUE;
            }
            else
            {
                xdcbx_port_config_entry_p->ets_recommend_tlv_enable = FALSE;
            }

            if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_PFC_CON_TLV_TX ) != 0)
            {
                xdcbx_port_config_entry_p->pfc_config_tlv_enable = TRUE;
            }
            else
            {
                xdcbx_port_config_entry_p->pfc_config_tlv_enable = FALSE;
            }

            if( (port_config_p->xdcbx_tlvs_tx_flag & LLDP_TYPE_XDCBX_APP_PRI_TLV_TX ) != 0)
            {
                xdcbx_port_config_entry_p->app_priority_tlv_enable = TRUE;
            }
            else
            {
                xdcbx_port_config_entry_p->app_priority_tlv_enable = FALSE;
            }

            result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
        else
        {
            result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetRunningXdcbxPortConfig */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetDcbxEtsRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX remote ETS entry
 * INPUT    : lport
 * OUTPUT   : rem_ets_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetDcbxEtsRemoteData(LLDP_TYPE_DcbxRemEtsEntry_T *rem_ets_entry_p)
{
    UI32_T result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (rem_ets_entry_p == NULL)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(rem_ets_entry_p->rem_local_port_num, rem_ets_entry_p->rem_index, &rem_data))
    {
        if(rem_ets_entry_p->rem_time_mark <= rem_data->time_mark)
        {
            rem_ets_entry_p->rem_config_rcvd = (rem_data->dcbx_rem_ets_config_entry!=NULL)?TRUE:FALSE;
            rem_ets_entry_p->rem_recommend_rcvd = (rem_data->dcbx_rem_ets_recommend_entry!=NULL)?TRUE:FALSE;
            if(rem_data->dcbx_rem_ets_config_entry != NULL)
            {
                rem_ets_entry_p->rem_config_willing = rem_data->dcbx_rem_ets_config_entry->rem_willing;
                rem_ets_entry_p->rem_config_cbs = rem_data->dcbx_rem_ets_config_entry->rem_cbs;
                rem_ets_entry_p->rem_config_max_tc = rem_data->dcbx_rem_ets_config_entry->rem_max_tc;
                memcpy(rem_ets_entry_p->rem_config_pri_assign_table, rem_data->dcbx_rem_ets_config_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
                memcpy(rem_ets_entry_p->rem_config_tc_bandwidth_table, rem_data->dcbx_rem_ets_config_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
                memcpy(rem_ets_entry_p->rem_config_tsa_assign_table, rem_data->dcbx_rem_ets_config_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
            }
            if(rem_data->dcbx_rem_ets_recommend_entry != NULL)
            {
                memcpy(rem_ets_entry_p->rem_recommend_pri_assign_table, rem_data->dcbx_rem_ets_recommend_entry->rem_pri_assign_table, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
                memcpy(rem_ets_entry_p->rem_recommend_tc_bandwidth_table, rem_data->dcbx_rem_ets_recommend_entry->rem_tc_bandwidth_table, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
                memcpy(rem_ets_entry_p->rem_recommend_tsa_assign_table, rem_data->dcbx_rem_ets_recommend_entry->rem_tsa_assign_table, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
            }
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetDcbxEtsRemoteData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetDcbxPfcRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get DCBX remote PFC entry
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetDcbxPfcRemoteData(LLDP_TYPE_DcbxRemPfcEntry_T *rem_pfc_entry_p)
{
    UI32_T result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T       *rem_data;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (rem_pfc_entry_p == NULL)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(rem_pfc_entry_p->rem_local_port_num, rem_pfc_entry_p->rem_index, &rem_data))
    {
        if((rem_pfc_entry_p->rem_time_mark <= rem_data->time_mark) &&
            (rem_data->dcbx_rem_pfc_config_entry != NULL))
        {
            memcpy(rem_pfc_entry_p->rem_mac, rem_data->dcbx_rem_pfc_config_entry->rem_mac, 6);
            rem_pfc_entry_p->rem_willing = rem_data->dcbx_rem_pfc_config_entry->rem_willing;
            rem_pfc_entry_p->rem_mbc = rem_data->dcbx_rem_pfc_config_entry->rem_mbc;
            rem_pfc_entry_p->rem_cap = rem_data->dcbx_rem_pfc_config_entry->rem_cap;
            rem_pfc_entry_p->rem_enable = rem_data->dcbx_rem_pfc_config_entry->rem_enable;
            result = LLDP_TYPE_RETURN_OK;
        }
    }

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetDcbxPfcRemoteData */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextDcbxAppPriRemoteData
 *-------------------------------------------------------------------------
 * PURPOSE  : Get Next DCBX remote Application Priority entry
 * INPUT    : lport
 * OUTPUT   : rem_pfc_entry_p
 * RETURN   : LLDP_TYPE_RETURN_OK       -- success
 *            LLDP_TYPE_RETURN_ERROR    -- fail
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T  LLDP_MGR_GetNextDcbxAppPriRemoteData(LLDP_TYPE_DcbxRemAppPriEntry_T *rem_app_pri_entry_p)
{
    UI32_T result = LLDP_TYPE_RETURN_ERROR;
    LLDP_OM_RemData_T       *rem_data;
    LLDP_OM_XDcbxRemAppPriorityEntry_T *rem_app_pri_entry, *rem_app_pri_entry_tmp;
//    BOOL_T                  first_round = TRUE;
//    UI32_T                  beg_lport = 0, beg_index = 0;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return result;
    }

    if (rem_app_pri_entry_p == NULL)
    {
        return result;
    }

    /* +++Enter critical section+++ */
    LLDP_OM_EnterCriticalSection();

    if(LLDP_OM_GetRemData(rem_app_pri_entry_p->rem_local_port_num, rem_app_pri_entry_p->rem_index, &rem_data))
    {
        if(rem_app_pri_entry_p->rem_time_mark <= rem_data->time_mark)
        {
            rem_app_pri_entry = (LLDP_OM_XDcbxRemAppPriorityEntry_T *)malloc(sizeof(LLDP_OM_XDcbxRemAppPriorityEntry_T));
            rem_app_pri_entry_tmp = rem_app_pri_entry;
            rem_app_pri_entry->rem_protocol_id = rem_app_pri_entry_p->rem_protocol_id;

            if(L_SORT_LST_Get_Next(&rem_data->dcbx_rem_app_pri_list, &rem_app_pri_entry))
            {
                rem_app_pri_entry_p->rem_protocol_id = rem_app_pri_entry->rem_protocol_id;
                rem_app_pri_entry_p->rem_priority = rem_app_pri_entry->rem_priority;
                rem_app_pri_entry_p->rem_sel = rem_app_pri_entry->rem_sel;
                result = LLDP_TYPE_RETURN_OK;
            }
            free(rem_app_pri_entry_tmp);
        }

    }
#if 0
    while(result != LLDP_TYPE_RETURN_OK &&
          LLDP_MGR_GetNextRemData(&rem_app_pri_entry_p->rem_time_mark,
                                  rem_app_pri_entry_p->rem_local_port_num,
                                  rem_app_pri_entry_p->rem_index,
                                  &rem_data))
    {

        if(L_SORT_LST_Get_1st(&rem_data->dcbx_rem_app_pri_list, &rem_app_pri_entry))
        {
            rem_app_pri_entry_p->rem_protocol_id = rem_app_pri_entry->rem_protocol_id;
            rem_app_pri_entry_p->rem_priority = rem_app_pri_entry->rem_priority;
            rem_app_pri_entry_p->rem_sel = rem_app_pri_entry->rem_sel;
            result = LLDP_TYPE_RETURN_OK;
        }

        rem_app_pri_entry_p->rem_local_port_num = rem_data->lport;
        rem_app_pri_entry_p->rem_index = rem_data->index;

        if(first_round)
        {
            beg_lport = rem_data->lport;
            beg_index = rem_data->index;
            first_round = FALSE;
        }
        else
        {
            if(beg_lport == rem_data->lport &&
               beg_index == rem_data->index)
            {
                break;
            }
        }
    }
#endif

    /* +++Leave critical section+++ */
    LLDP_OM_LeaveCriticalSection();

    return result;
}/* End of LLDP_MGR_GetNextDcbxAppPriRemoteData */

#endif /* #if (SYS_CPNT_DCBX == TRUE) */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    LLDP_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (LLDP_MGR_IpcMsg_T*)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.ret_ui32 = LLDP_TYPE_RETURN_ERROR;
        msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding LLDP_MGR function
     */
    switch (msg_p->type.cmd)
    {
#if (LLDP_TYPE_DCBX == TRUE)
        case LLDP_MGR_IPC_NOTIFYETSPFCCFGCHANGED:
            LLDP_MGR_NotifyEtsPfcCfgChanged(
                msg_p->data.arg_grp_ui32_bool_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool_bool.arg_bool_1,
                msg_p->data.arg_grp_ui32_bool_bool.arg_bool_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif

        case LLDP_MGR_IPC_SETSYSADMINSTATUS:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetSysAdminStatus(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETMSGTXINTERVAL:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetMsgTxInterval(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETMSGTXHOLDMUL:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetMsgTxHoldMul(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETREINITDELAY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetReinitDelay(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETTXDELAY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetTxDelay(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETNOTIFICATIONINTERVAL:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetNotificationInterval(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETPORTCONFIGADMINSTATUS:
            msg_p->type.ret_ui32 = LLDP_MGR_SetPortConfigAdminStatus(
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETPORTADMINDISABLE:
            LLDP_MGR_PortAdminDisable_CallBack(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETPORTCONFIGNOTIFICATIONENABLE:
            msg_p->type.ret_ui32 = LLDP_MGR_SetPortConfigNotificationEnable(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETPORTOPTIONALTLVSTATUS:
            msg_p->type.ret_ui32 = LLDP_MGR_SetPortOptionalTlvStatus(
                msg_p->data.arg_grp_ui32_ui8.arg_ui32,
                msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETCONFIGMANADDRTLV:
            msg_p->type.ret_ui32 = LLDP_MGR_SetConfigManAddrTlv(
                msg_p->data.arg_grp_ui32_bool.arg_ui32,
                msg_p->data.arg_grp_ui32_bool.arg_bool);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;
        case LLDP_MGR_IPC_SETCONFIGALLPORTMANADDRTLV:
            msg_p->type.ret_ui32 = LLDP_MGR_SetConfigAllPortManAddrTlv(
                msg_p->data.arg_grp_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;
        case LLDP_MGR_IPC_GETPORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetPortConfigEntry(&msg_p->data.arg_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTPORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextPortConfigEntry(&msg_p->data.arg_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETCONFIGMANADDRTLV:
            msg_p->type.ret_ui32 = LLDP_MGR_GetConfigManAddrTlv(
                msg_p->data.arg_grp_ui32_ui8.arg_ui32,
                &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_MGR_IPC_GETSYSSTATISTICSENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetSysStatisticsEntry(&msg_p->data.arg_statistics);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_statistics);
            break;

        case LLDP_MGR_IPC_GETPORTTXSTATISTICSENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetPortTxStatisticsEntry(&msg_p->data.arg_port_tx_statistics);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_tx_statistics);
            break;

        case LLDP_MGR_IPC_GETNEXTPORTTXSTATISTICSENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextPortTxStatisticsEntry(&msg_p->data.arg_port_tx_statistics);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_tx_statistics);
            break;

        case LLDP_MGR_IPC_GETPORTRXSTATISTICSENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetPortRxStatisticsEntry(&msg_p->data.arg_port_rx_statistics);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_rx_statistics);
            break;

        case LLDP_MGR_IPC_GETNEXTPORTRXSTATISTICSENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextPortRxStatisticsEntry(&msg_p->data.arg_port_rx_statistics);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_port_rx_statistics);
            break;

        case LLDP_MGR_IPC_GETLOCALSYSTEMDATA:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetLocalSystemData(&msg_p->data.arg_local_system_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_system_data);
            break;

        case LLDP_MGR_IPC_GETLOCALPORTDATA:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetLocalPortData(&msg_p->data.arg_local_port_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_port_data);
            break;

        case LLDP_MGR_IPC_GETNEXTLOCALPORTDATA:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextLocalPortData(&msg_p->data.arg_local_port_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_port_data);
            break;

        case LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESSTLVENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetLocalManagementAddressTlvEntry(&msg_p->data.arg_local_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
            break;

        case LLDP_MGR_IPC_GETLOCALMANAGEMENTADDRESS:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetLocalManagementAddress(&msg_p->data.arg_local_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESS:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextLocalManagementAddress(&msg_p->data.arg_local_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTLOCALMANAGEMENTADDRESSTLVENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextLocalManagementAddressTlvEntry(&msg_p->data.arg_local_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_local_mgmt_addr_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATA:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextRemoteSystemData(&msg_p->data.arg_remote_system_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
            break;

        case LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYPORT:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextRemoteSystemDataByPort(&msg_p->data.arg_remote_system_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
            break;

        case LLDP_MGR_IPC_GETNEXTREMOTESYSTEMDATABYINDEX:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextRemoteSystemDataByIndex(&msg_p->data.arg_remote_system_data);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_system_data);
            break;

        case LLDP_MGR_IPC_GETREMOTEMANAGEMENTADDRESSTLVENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRemoteManagementAddressTlvEntry(&msg_p->data.arg_remote_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTREMOTEMANAGEMENTADDRESSTLVENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextRemoteManagementAddressTlvEntry(&msg_p->data.arg_remote_mgmt_addr_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_remote_mgmt_addr_entry);
            break;

#if (LLDP_TYPE_EXT_802DOT1 == TRUE)
        case LLDP_MGR_IPC_GETXDOT1CONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot1ConfigEntry(&msg_p->data.arg_xdot1_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_config_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1CONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1ConfigEntry(&msg_p->data.arg_xdot1_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_config_entry);
            break;

        case LLDP_MGR_IPC_SETXDOT1CONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot1ConfigEntry(&msg_p->data.arg_xdot1_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETXDOT1CONFIGPORTVLANTXENABLE:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot1ConfigPortVlanTxEnable(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETXDOT1CONFIGPROTOVLANTXENABLE:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot1ConfigProtoVlanTxEnable(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETXDOT1CONFIGVLANNAMETXENABLE:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot1ConfigVlanNameTxEnable(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETXDOT1CONFIGPROTOCOLTXENABLE:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot1ConfigProtocolTxEnable(msg_p->data.arg_grp_ui32_ui32.arg_ui32_1, msg_p->data.arg_grp_ui32_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETXDOT1LOCENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot1LocEntry(&msg_p->data.arg_xdot1_loc_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1LOCENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1LocEntry(&msg_p->data.arg_xdot1_loc_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT1LOCPROTOVLANENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot1LocProtoVlanEntry(&msg_p->data.arg_xdot1_loc_proto_vlan_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_proto_vlan_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOVLANENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1LocProtoVlanEntry(&msg_p->data.arg_xdot1_loc_proto_vlan_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_proto_vlan_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT1LOCVLANNAMEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot1LocVlanNameEntry(&msg_p->data.arg_xdot1_loc_vlan_name_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_vlan_name_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1LOCVLANNAMEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1LocVlanNameEntry(&msg_p->data.arg_xdot1_loc_vlan_name_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_vlan_name_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT1LOCPROTOCOLENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot1LocProtocolEntry(&msg_p->data.arg_xdot1_loc_protocol_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_protocol_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1LOCPROTOCOLENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1LocProtocolEntry(&msg_p->data.arg_xdot1_loc_protocol_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_loc_protocol_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1REMENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1RemEntry(&msg_p->data.arg_xdot1_rem_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1REMENTRYBYINDEX:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1RemEntryByIndex(&msg_p->data.arg_xdot1_rem_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOVLANENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1RemProtoVlanEntry(&msg_p->data.arg_xdot1_rem_proto_vlan_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_proto_vlan_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1REMVLANNAMEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1RemVlanNameEntry(&msg_p->data.arg_xdot1_rem_vlan_name_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_vlan_name_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT1REMPROTOCOLENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot1RemProtocolEntry(&msg_p->data.arg_xdot1_rem_protocol_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot1_rem_protocol_entry);
            break;
#endif /* #if (LLDP_TYPE_EXT_802DOT1 == TRUE) */

#if (LLDP_TYPE_EXT_802DOT3 == TRUE)
        case LLDP_MGR_IPC_GETXDOT3PORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3PortConfigEntry(&msg_p->data.arg_xdot3_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3PORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3PortConfigEntry(&msg_p->data.arg_xdot3_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_port_config_entry);
            break;

        case LLDP_MGR_IPC_SETXDOT3PORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot3PortConfigEntry(msg_p->data.arg_grp_ui32_ui8.arg_ui32, msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_SETXDOT3PORTCONFIG:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdot3PortConfig(&msg_p->data.arg_xdot3_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETXDOT3LOCPORTENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3LocPortEntry(&msg_p->data.arg_xdot3_local_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_local_port_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3LOCPORTENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3LocPortEntry(&msg_p->data.arg_xdot3_local_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_local_port_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT3LOCPOWERENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3LocPowerEntry(&msg_p->data.arg_xdot3_loc_power_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_power_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3LOCPOWERENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3LocPowerEntry(&msg_p->data.arg_xdot3_loc_power_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_power_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT3LOCLINKAGGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3LocLinkAggEntry(&msg_p->data.arg_xdot3_loc_link_agg_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_link_agg_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3LOCLINKAGGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3LocLinkAggEntry(&msg_p->data.arg_xdot3_loc_link_agg_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_link_agg_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT3LOCMAXFRAMESIZEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3LocMaxFrameSizeEntry(&msg_p->data.arg_xdot3_loc_maxframe_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_maxframe_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3LOCMAXFRAMESIZEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3LocMaxFrameSizeEntry(&msg_p->data.arg_xdot3_loc_maxframe_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_loc_maxframe_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT3REMPORTENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3RemPortEntry(&msg_p->data.arg_xdot3_rem_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3REMPORTENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3RemPortEntry(&msg_p->data.arg_xdot3_rem_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_port_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3REMPOWERENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3RemPowerEntry(&msg_p->data.arg_xdot3_rem_power_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_power_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDOT3REMPOWERENTRYBYINDEX:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3RemPowerEntryByIndex(&msg_p->data.arg_xdot3_rem_power_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_power_entry);
            break;

        case LLDP_MGR_IPC_GETXDOT3REMLINKAGGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3RemLinkAggEntry(&msg_p->data.arg_xdot3_rem_link_agg_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
            break;
        case LLDP_MGR_IPC_GETNEXTXDOT3REMLINKAGGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3RemLinkAggEntry(&msg_p->data.arg_xdot3_rem_link_agg_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_link_agg_entry);
            break;
        case LLDP_MGR_IPC_GETXDOT3REMMAXFRAMESIZEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdot3RemMaxFrameSizeEntry(&msg_p->data.arg_xdot3_rem_max_frame_size_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
            break;
        case LLDP_MGR_IPC_GETNEXTXDOT3REMMAXFRAMESIZEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextXdot3RemMaxFrameSizeEntry(&msg_p->data.arg_xdot3_rem_max_frame_size_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdot3_rem_max_frame_size_entry);
            break;
#endif /* #if (LLDP_TYPE_EXT_802DOT3 == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
        case LLDP_MGR_IPC_ISTELEPHONEMAC:
            msg_p->type.ret_bool = LLDP_MGR_IsTelephoneMac(
                msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui8,
                msg_p->data.arg_grp_ui32_ui8_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_ISTELEPHONENETWORKADDR:
            msg_p->type.ret_bool = LLDP_MGR_IsTelephoneNetworkAddr(
                msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui32_1,
                msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui8_1,
                msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui8_2,
                msg_p->data.arg_grp_ui32_ui8_ui8_ui32.arg_ui32_2);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_ADD == TRUE) */

        case LLDP_MGR_IPC_NOTIFYSYSNAMECHANGED:
            LLDP_MGR_NotifySysNameChanged();
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_NOTIFYRIFCHANGED:
            LLDP_MGR_NotifyRifChanged(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_NOTIFYROUTINGCHANGED:
            LLDP_MGR_NotifyRoutingChanged();
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_NOTIFYPSETABLECHANGED:
            LLDP_MGR_NotifyPseTableChanged(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

#if (LLDP_TYPE_MED == TRUE)
        case LLDP_MGR_IPC_SETXMEDFASTSTARTREPEATCOUNT:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedFastStartRepeatCount(msg_p->data.arg_ui32);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETXMEDPORTCONFIGENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedPortConfigEntry(&msg_p->data.arg_xmed_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDPORTCONFIGENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedPortConfigEntry(&msg_p->data.arg_xmed_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_port_config_entry);
            break;

        case LLDP_MGR_IPC_SETXMEDPORTCONFIGTLVSTX:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedPortConfigTlvsTx(msg_p->data.arg_grp_ui32_ui16.arg_ui32, msg_p->data.arg_grp_ui32_ui16.arg_ui16);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETRUNNINGXMEDPORTCONFIGTLVSTX:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXMedPortConfigTlvsTx(msg_p->data.arg_grp_ui32_ui16.arg_ui32, &msg_p->data.arg_grp_ui32_ui16.arg_ui16);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui16);
            break;

        case LLDP_MGR_IPC_SETXMEDPORTCONFIGNOTIFENABLED:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedPortConfigNotifEnabled(msg_p->data.arg_grp_ui32_ui8.arg_ui32, msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETRUNNINGXMEDPORTNOTIFICATION:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXMedPortNotification(msg_p->data.arg_grp_ui32_ui8.arg_ui32, &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCMEDIAPOLICYENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocMediaPolicyEntry(&msg_p->data.arg_xmed_loc_med_policy_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_med_policy_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDLOCMEDIAPOLICYENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedLocMediaPolicyEntry(&msg_p->data.arg_xmed_loc_med_policy_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_med_policy_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCLOCATIONENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocLocationEntry(&msg_p->data.arg_xmed_loc_location_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_location_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedLocLocationEntry(&msg_p->data.arg_xmed_loc_location_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_location_entry);
            break;

        case LLDP_MGR_IPC_SETXMEDLOCLOCATIONENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedLocLocationEntry(&msg_p->data.arg_xmed_loc_location_entry);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETXMEDLOCLOCATIONSTATUS:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocLocationStatus(msg_p->data.arg_grp_xmed_loc_status.arg_lport, msg_p->data.arg_grp_xmed_loc_status.arg_location_type, &msg_p->data.arg_grp_xmed_loc_status.arg_status);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_status);
            break;

        case LLDP_MGR_IPC_SETXMEDLOCLOCATIONSTATUS:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedLocLocationStatus(msg_p->data.arg_grp_xmed_loc_status.arg_lport, msg_p->data.arg_grp_xmed_loc_status.arg_location_type, msg_p->data.arg_grp_xmed_loc_status.arg_status);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONSTATUS:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXMedLocLocationStatus(msg_p->data.arg_grp_xmed_loc_status.arg_lport, msg_p->data.arg_grp_xmed_loc_status.arg_location_type, &msg_p->data.arg_grp_xmed_loc_status.arg_status);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_status);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocLocationCivicAddrCoutryCode(msg_p->data.arg_grp_xmed_loc_country_code.arg_lport, msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_country_code);
            break;

        case LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCOUTRYCODE:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedLocLocationCivicAddrCoutryCode(msg_p->data.arg_grp_xmed_loc_country_code.arg_lport, msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRCOUTRYCODE:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXMedLocLocationCivicAddrCoutryCode(msg_p->data.arg_grp_xmed_loc_country_code.arg_lport, msg_p->data.arg_grp_xmed_loc_country_code.arg_country_code);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_loc_country_code);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCLOCATIONCIVICADDRWHAT:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocLocationCivicAddrWhat(msg_p->data.arg_grp_ui32_ui8.arg_ui32, &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRWHAT:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedLocLocationCivicAddrWhat(msg_p->data.arg_grp_ui32_ui8.arg_ui32, msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETRUNNINGXMEDLOCLOCATIONCIVICADDRWHAT:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXMedLocLocationCivicAddrWhat(msg_p->data.arg_grp_ui32_ui8.arg_ui32, &msg_p->data.arg_grp_ui32_ui8.arg_ui8);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_ui32_ui8);
            break;

        case LLDP_MGR_IPC_GET1STXMEDLOCLOCATIONCIVICADDRCAENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_Get1stXMedLocLocationCivicAddrCaEntry(msg_p->data.arg_grp_xmed_ca_entry.arg_lport, &msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_ca_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDLOCLOCATIONCIVICADDRCAENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedLocLocationCivicAddrCaEntry(msg_p->data.arg_grp_xmed_ca_entry.arg_lport, &msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_ca_entry);
            break;

        case LLDP_MGR_IPC_SETXMEDLOCLOCATIONCIVICADDRCAENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_SetXMedLocLocationCivicAddrCaEntry(msg_p->data.arg_grp_xmed_ca_entry.arg_lport, &msg_p->data.arg_grp_xmed_ca_entry.arg_ca_entry, msg_p->data.arg_grp_xmed_ca_entry.arg_set_or_unset);
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
            break;

        case LLDP_MGR_IPC_GETXMEDLOCXPOEPSEPORTENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocXPoePsePortEntry(&msg_p->data.arg_xmed_loc_poe_pse_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_pse_port_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDLOCXPOEPSEPORTENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedLocXPoePsePortEntry(&msg_p->data.arg_xmed_loc_poe_pse_port_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_pse_port_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCXPOEENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocXPoeEntry(&msg_p->data.arg_xmed_loc_poe_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_poe_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDLOCINVENTORYENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedLocInventoryEntry(&msg_p->data.arg_xmed_loc_inventory_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_loc_inventory_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMCAPENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemCapEntry(&msg_p->data.arg_xmed_rem_cap_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_cap_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMMEDIAPOLICYENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemMediaPolicyEntry(&msg_p->data.arg_xmed_rem_med_policy_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDREMMEDIAPOLICYENTRYBYINDEX:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedRemMediaPolicyEntryByIndex(&msg_p->data.arg_xmed_rem_med_policy_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_med_policy_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMINVENTORYENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemInventoryEntry(&msg_p->data.arg_xmed_rem_inventory_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_inventory_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemLocationEntry(&msg_p->data.arg_xmed_rem_location_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_location_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONENTRYBYINDEX:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemLocationEntryByIndex(&msg_p->data.arg_xmed_rem_location_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_location_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRCOUTRYCODE:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedRemLocationCivicAddrCountryCode(msg_p->data.arg_grp_xmed_rem_country_code.arg_rem_loc_port_num, msg_p->data.arg_grp_xmed_rem_country_code.arg_rem_index, msg_p->data.arg_grp_xmed_rem_country_code.arg_country_code);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_country_code);
            break;

        case LLDP_MGR_IPC_GETXMEDREMLOCATIONCIVICADDRWHAT:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedRemLocationCivicAddrWhat(msg_p->data.arg_grp_xmed_rem_what.arg_rem_loc_port_num, msg_p->data.arg_grp_xmed_rem_what.arg_rem_index, &msg_p->data.arg_grp_xmed_rem_what.arg_what);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_what);
            break;

        case LLDP_MGR_IPC_GET1STXMEDREMLOCATIONCIVICADDRCAENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_Get1stXMedRemLocationCivicAddrCaEntry(msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_loc_port_num, msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_index, &msg_p->data.arg_grp_xmed_rem_ca_entry.arg_ca_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_ca_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMLOCATIONCIVICADDRCAENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemLocationCivicAddrCaEntry(msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_loc_port_num, msg_p->data.arg_grp_xmed_rem_ca_entry.arg_rem_index, &msg_p->data.arg_grp_xmed_rem_ca_entry.arg_ca_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_ca_entry);
            break;

        case LLDP_MGR_IPC_GETXMEDREMLOCATIONELIN:
            msg_p->type.ret_bool =
                LLDP_MGR_GetXMedRemLocationElin(msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_loc_port_num, msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_index, &msg_p->data.arg_grp_xmed_rem_loc_elin_entry.arg_rem_loc_elin_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_grp_xmed_rem_loc_elin_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMPOEENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemPoeEntry(&msg_p->data.arg_xmed_rem_poe_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_poe_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMPOEPSEENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemPoePseEntry(&msg_p->data.arg_xmed_rem_pse_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_pse_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXMEDREMPOEPDENTRY:
            msg_p->type.ret_bool =
                LLDP_MGR_GetNextXMedRemPoePdEntry(&msg_p->data.arg_xmed_rem_pd_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xmed_rem_pse_entry);
            break;
#endif /* #if (LLDP_TYPE_MED == TRUE) */

#if (LLDP_TYPE_DCBX == TRUE)
        case LLDP_MGR_IPC_SETXDCBXPORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_SetXdcbxPortConfig(&msg_p->data.arg_xdcbx_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETXDCBXPORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetXdcbxPortConfig(&msg_p->data.arg_xdcbx_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETRUNNINGXDCBXPORTCONFIGENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetRunningXdcbxPortConfig(&msg_p->data.arg_xdcbx_port_config_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_port_config_entry);
            break;

        case LLDP_MGR_IPC_GETXDCBXETSREMOTEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetDcbxEtsRemoteData(&msg_p->data.arg_xdcbx_ets_rem_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_ets_rem_entry);
            break;

        case LLDP_MGR_IPC_GETXDCBXPFCREMOTEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetDcbxPfcRemoteData(&msg_p->data.arg_xdcbx_pfc_rem_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_pfc_rem_entry);
            break;

        case LLDP_MGR_IPC_GETNEXTXDCBXAPPPRIREMOTEENTRY:
            msg_p->type.ret_ui32 =
                LLDP_MGR_GetNextDcbxAppPriRemoteData(&msg_p->data.arg_xdcbx_app_pri_rem_entry);
            msgbuf_p->msg_size = LLDP_MGR_GET_MSG_SIZE(arg_xdcbx_app_pri_rem_entry);
            break;

#endif /* #if (LLDP_TYPE_DCBX == TRUE) */

        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.ret_ui32 = LLDP_TYPE_RETURN_ERROR;
            msgbuf_p->msg_size = LLDP_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
} /* End of LLDP_MGR_HandleIPCReqMsg */


/*=============================================================================
 * Moved from lldp_om.c
 *=============================================================================
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_GetNextRemData_Local
 * ------------------------------------------------------------------------
 * PURPOSE  : Find next remote data
 * INPUT    : lport             -- a specified port
 *            index             -- a specified index
 * OUTPUT   : void *get_data
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LLDP_MGR_GetNextRemData_Local(UI32_T lport, UI32_T index, LLDP_OM_RemData_T **get_data)
{
    L_SORT_LST_List_T   *list;
    LLDP_OM_RemData_T   *rem_data;

    if (  (LLDP_OM_GetTotalNumOfRemData() == 0)
        ||(lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
        ||(lport == 0)
       )
    {
        return FALSE;
    }

    list = LLDP_OM_GetPortDeviceList(lport);
    if (L_SORT_LST_Get_1st(list, &rem_data))
    {
        do
        {
            /* if find element, return */
            if (rem_data->index > index)
            {
                *get_data = rem_data;
                return TRUE;
            }
        } while(L_SORT_LST_Get_Next(list, &rem_data));
    }

    while (TRUE)
    {
        if (!LLDP_UTY_GetNextLogicalPort(&lport))
        {
            return FALSE;
        }

        list = LLDP_OM_GetPortDeviceList(lport);
        if (L_SORT_LST_Get_1st(list, &rem_data))
        {
            *get_data = rem_data;
            return TRUE;
        }
    }
    return FALSE;
}/* End of LLDP_MGR_GetNextRemData_Local */



/*=============================================================================
 * Moved from lldp_task.h and changed name
 *=============================================================================
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Init
 *-------------------------------------------------------------------------
 * FUNCTION: Init the LLDP CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_Init(void)
{
    /* Init OM : default state is disabled */
    LLDP_OM_InitSemaphore();
    LLDP_OM_Init();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_Create_InterCSC_Relation(void)
{
    /* Register Backdoor */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("LLDP",
        SYS_BLD_GVRP_GROUP_IPCMSGQ_KEY, LLDP_BACKDOOR_Main);
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_ProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by stkctrl will tell LLDP that provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
void LLDP_MGR_ProvisionComplete(void)
{
    provision_completed = TRUE;
    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_IsProvisionComplete
 *-------------------------------------------------------------------------
 * PURPOSE  : This function calling by LLDP mgr will tell if LLDP provision is completed.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * Note     : None
 *-------------------------------------------------------------------------
 */
BOOL_T LLDP_MGR_IsProvisionComplete(void)
{
    return (provision_completed);
}


/* created for replacing lldp_task */
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_MGR_LldpduRcvd_Callback
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the callback event when receiving a LLDPDU packet.
 * INPUT   : mref_handle_p -- packet buffer and return buffer function pointer.
 *           dst_mac       -- the destination MAC address of this packet.
 *           src_mac       -- the source MAC address of this packet.
 *           tag_info      -- tag information
 *           type          -- packet type
 *           pkt_length    -- pdu length
 *           lport         -- source lport
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------------------
 */
#if 0
void LLDP_MGR_LldpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                  UI8_T *dst_mac,
                                  UI8_T *src_mac,
                                  UI16_T tag_info,
                                  UI16_T type,
                                  UI32_T length,
                                  UI32_T lport)
#else
void LLDP_MGR_LldpduRcvd_Callback(L_MM_Mref_Handle_T *mref_handle_p,
                                              UI8_T *dst_mac,
                                              UI8_T *src_mac,
                                              UI16_T tag_info,
                                              UI16_T type,
                                              UI32_T pkt_length,
                                              UI32_T  src_unit,
                                              UI32_T  src_port,
                                              UI32_T  packet_class)

#endif
{
    UI32_T              status,lport;
    LLDP_TYPE_MSG_T     msg;

    /* BODY
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToLogicalPort(src_unit,src_port,&lport)){
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    LLDP_OM_GetSysAdminStatus(&status);
    if (status == LLDP_TYPE_SYS_ADMIN_DISABLE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* msg.msg_type = 0x88CC; */
    memcpy(msg.saddr, src_mac, 6);
    msg.mem_ref = mref_handle_p;
    msg.type = type;
    msg.lport = src_port;

    #if RX_DEBUG_PRINT
    {
        int i;
        printf("Rcvd msg:\n");
        printf("src_addr:");
        for(i = 0 ;i < 6; i++)
            printf("%02X", msg.saddr[i]);
        puts("");
        printf("type: %0X", msg.type);
        printf("port: %d", msg.lport);
        printf("pdu:");
        for(i = 0; i < msg.mem_ref->pdu_len; i++)
        {
            printf("%02X", msg.mem_ref->pdu[i]);
        }
        puts("");
    }
    #endif

    LLDP_MGR_ProcessRcvdPDU(&msg);

    return;
} /* end of GARP_MGR_GarpPduRcvd_Callback */
