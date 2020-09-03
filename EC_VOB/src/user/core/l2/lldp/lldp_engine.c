/*-----------------------------------------------------------------------------
 * Module Name: lldp_engine.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the LLDP engine
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    03/06/2006 - Gordon Kao, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sysfun.h"
#if (SYS_CPNT_DOT1X == TRUE)
    #if (SYS_CPNT_NETACCESS == TRUE)
    #include "netaccess_mgr.h"
    #else
    #include "1x_pom.h"
    #endif
#endif
#include "lldp_om_private.h"
#include "lldp_type.h"
#include "lldp_uty.h"
#include "swctrl.h"
#include "sys_time.h"


#define RX_DEBUG_PRINT    0


/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_SomethingChangedLocal
 *-------------------------------------------------------------------------
 * PURPOSE  : When something changed local, must fire lldpdu if possible
 * INPUT    : UI32_T lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 10.5.4.1, IEEE Std 802.1AB-2005.
 *            this funtion needn't to enter CSC and om critical section
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_SomethingChangedLocal(UI32_T  lport)
{
    /* if lport is equal to 0, then it means global variable changed*/
    if(lport == 0)
    {
        while(LLDP_UTY_GetNextLogicalPort(&lport))
        {
            LLDP_ENGINE_SomethingChangedLocal(lport);
        }
    }
    else /* specified port */
    {

        LLDP_OM_PortConfigEntry_T   *port_config;

        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        port_config->something_changed_local = TRUE;

    }
} /* End of LLDP_ENGINE_SomethingChangedLocal */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_SomethingChangedRemote
 *-------------------------------------------------------------------------
 * PURPOSE  : When something changed local, some action must be done
 * INPUT    : UI32_T lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref to the description in 10.5.4.1, IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_SomethingChangedRemote(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T    *port_config;
    LLDP_OM_SysConfigEntry_T     *sys_config;

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if (port_config->notification_enable == VAL_lldpPortConfigNotificationEnable_true)
    {
        sys_config->something_changed_remote = TRUE;
    }

    return;
}/* End of LLDP_ENGINE_SomethingChangedRemote */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ConstructLLDPDU
 *-------------------------------------------------------------------------
 * PURPOSE  : Construct LLDPDU packet
 * INPUT    : lport, LLDPDU type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_ConstructLLDPDU(UI32_T lport, UI32_T type)
{
    UI32_T  current_time;
    LLDP_OM_PortConfigEntry_T   *port_config;
    LLDP_OM_SysConfigEntry_T    *sys_config;
    LLDP_OM_PortStatistics_T   *port_stat;
#if (SYS_CPNT_DOT1X == TRUE)
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_MGR_Dot1XAuthControlledPortStatus_T port_auth_status;
#endif
#endif

    port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    port_stat = LLDP_OM_GetPortStatisticsEntryPtr(lport);

    if(SWCTRL_isPortLinkUp(lport) == FALSE)
    {
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        port_config->transfer_timer = current_time + (sys_config->msg_tx_interval * LLDP_TYPE_TIME_UNIT);
        port_config->something_changed_local = FALSE;
        return;
    }

#if (SYS_CPNT_DOT1X == TRUE)
    if (!SWCTRL_LogicalPortIsTrunkPort(lport) &&
#if (SYS_CPNT_NETACCESS == TRUE)
/*EPR: ES3628BT-FLF-ZZ-01000
Problem:stack:DUT hot insert units may take over 1 minutes.
Rootcause:for GVRP cost 9000 ticks.When do hotinsert,and LLDP is ready to sendpkt,LLDP pmgr to get dot1x info,and netaccess_group is suspend to
          get slave reply.
Solution:Gvrp and netaccess_group in the same process,and get the database directly instead of pmgr
Files:lldp_engine.c*/
#if 0
        NETACCESS_PMGR_GetDot1xPortAuthorized(lport, &port_auth_status)
#else
        NETACCESS_MGR_GetDot1xPortAuthorized(lport, &port_auth_status)
#endif
        && (port_auth_status == VAL_dot1xAuthAuthControlledPortStatus_unauthorized)
#else
        DOT1X_OM_Get_Port_Authorized(lport) == VAL_dot1xAuthAuthControlledPortStatus_unauthorized
#endif
       )
    {
        SYS_TIME_GetSystemUpTimeByTick(&current_time);
        port_config->transfer_timer = current_time + (sys_config->msg_tx_interval * LLDP_TYPE_TIME_UNIT);
        port_config->something_changed_local = FALSE;
        return;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    /* check if the LLDPDU should be send */
    if(
        (sys_config->global_admin_status == LLDP_TYPE_SYS_ADMIN_ENABLE) &&
        (port_config->admin_status == VAL_lldpPortConfigAdminStatus_txOnly ||
         port_config->admin_status == VAL_lldpPortConfigAdminStatus_txAndRx) &&
        (port_config->reinit_delay_timer == 0) )
    {
        switch(type)
        {
            /* construct normal LLDPDU */
            case LLDP_TYPE_NORMAL_LLDPDU:
                if (port_config->tx_delay_timer == 0)
                {
                    LLDP_UTY_ConstructNormalLLDPDU(sys_config, port_config);
                    port_stat->tx_frames_total ++;

                    /* MED fast start */
                    if(port_config->fast_start_count > 0 &&
                       (--port_config->fast_start_count) > 0)
                    {
                        port_config->transfer_timer = 0;
                        port_config->tx_delay_timer = 0;
                    }
                    else
                    {
                        SYS_TIME_GetSystemUpTimeByTick(&current_time);
                        port_config->transfer_timer = current_time + (sys_config->msg_tx_interval * LLDP_TYPE_TIME_UNIT);
                        port_config->tx_delay_timer = current_time + (sys_config->tx_delay * LLDP_TYPE_TIME_UNIT);
                    }

                    port_config->something_changed_local = FALSE;
                }
                break;

            /* construct shutdown LLDPDU*/
            case LLDP_TYPE_SHUTDOWN_LLDPDU:
                LLDP_UTY_ConstructShutdownLLDPDU(lport);
                port_stat->tx_frames_total ++;
                SYS_TIME_GetSystemUpTimeByTick(&current_time);
                port_config->reinit_delay_timer = current_time + sys_config->reinit_delay * LLDP_TYPE_TIME_UNIT;
                break;

            default:
                break;
        }
    }
}/* End of LLDP_ENGINE_ConstructLLDPDU */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusEnableRx
 *-------------------------------------------------------------------------
 * PURPOSE  : Enable the LLDPDU receive mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 *
 */
UI32_T LLDP_ENGINE_AdminStatusEnableRx(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T   *om_port_config;

    /* get port config entry pointer from om */
    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
    #if 0
    /* delete all information associated with this port*/
    LLDP_OM_DeleteRemDataByPort(lport);
    LLDP_ENGINE_SomethingChangedRemote(lport);
    #endif

    /* set port admin_status
     */
    switch (om_port_config->admin_status)
    {
        case VAL_lldpPortConfigAdminStatus_disabled:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_rxOnly;
            break;
        case VAL_lldpPortConfigAdminStatus_txOnly:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_txAndRx;
            break;
        case VAL_lldpPortConfigAdminStatus_rxOnly:
        case VAL_lldpPortConfigAdminStatus_txAndRx:
            /* Do nothing
             */
            break;
        default:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_rxOnly;
            break;
    }

    return LLDP_TYPE_INIT_RX_OK;
}/* End of LLDP_ENGINE_AdminStatusEnableRx*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusEnableTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Enable the LLDPDU transmit mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusEnableTx(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T   *om_port_config;

    /* get port config pointer from om*/
    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    /* set port admin_status
     */
    switch (om_port_config->admin_status)
    {
        case VAL_lldpPortConfigAdminStatus_disabled:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_txOnly;
            break;
        case VAL_lldpPortConfigAdminStatus_rxOnly:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_txAndRx;
            break;
        case VAL_lldpPortConfigAdminStatus_txOnly:
        case VAL_lldpPortConfigAdminStatus_txAndRx:
            /* Do nothing
             */
            break;
        default:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_txOnly;
            break;
    }

    /* if the port's reinit_delay timer is zero, reinitial now
     */
    if(om_port_config->reinit_delay_timer == 0)
    {

        if(om_port_config->reinit_flag == TRUE)
        {
            /* do initial Tx process */
            om_port_config->reinit_flag = FALSE;
            om_port_config->tx_delay_timer = 0;

            /* send first packet */
            LLDP_ENGINE_ConstructLLDPDU(lport, LLDP_TYPE_NORMAL_LLDPDU);
        }
    }
    else /* else, set reinit_flag == true */
    {
        om_port_config->reinit_flag = TRUE;
    }

    return LLDP_TYPE_INIT_TX_OK;
}/* End of LLDP_ENGINE_AdminStatusEnableTx*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusDisableRx
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the receive mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusDisableRx(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T   *om_port_config;

    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if (om_port_config->admin_status == VAL_lldpPortConfigAdminStatus_disabled ||
        om_port_config->admin_status == VAL_lldpPortConfigAdminStatus_txOnly)
    {
        return LLDP_TYPE_RETURN_OK;
    }

    LLDP_OM_DeleteRemDataByPort(lport);
    LLDP_ENGINE_SomethingChangedRemote(lport);

    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    switch (om_port_config->admin_status)
    {
        case VAL_lldpPortConfigAdminStatus_rxOnly:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_disabled;
            break;

        case VAL_lldpPortConfigAdminStatus_txAndRx:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_txOnly;
            break;

        case VAL_lldpPortConfigAdminStatus_disabled:
        case VAL_lldpPortConfigAdminStatus_txOnly:
        default:
            /* Do nothing
             */
            break;
    }

    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_ENGINE_AdminStatusDisableRx*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_AdminStatusDisableTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Disable the transmit mechanism of the lport
 * INPUT    : UI32_T lport            -- lport number
 * OUTPUT   : None
 * RETURN   : LLDP_TYPE_RETURN_OK/LLDP_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_AdminStatusDisableTx(UI32_T lport)
{
    LLDP_OM_PortConfigEntry_T   *om_port_config;
    BOOL_T                      shutdown_transmit;

    om_port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

    if (om_port_config->admin_status == VAL_lldpPortConfigAdminStatus_disabled ||
        om_port_config->admin_status == VAL_lldpPortConfigAdminStatus_rxOnly)
    {
        return LLDP_TYPE_RETURN_OK;
    }

    /* if this port is already in reinitial delay */
    if(om_port_config->reinit_delay_timer != 0)
    /*if(LLDP_OM_InReinitDelayList(lport) == TRUE)*/
    {
        shutdown_transmit = FALSE;
    }
    else /* else insert to reinitial delay list */
    {
        shutdown_transmit = TRUE;
    }
    om_port_config->reinit_flag = FALSE;

    /* 2. if necessary, transfer the shutdown LLDPDU */
    if(shutdown_transmit == TRUE)
    {
        LLDP_ENGINE_ConstructLLDPDU(lport, LLDP_TYPE_SHUTDOWN_LLDPDU);
    }

    switch (om_port_config->admin_status)
    {
        case VAL_lldpPortConfigAdminStatus_txOnly:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_disabled;
            break;

        case VAL_lldpPortConfigAdminStatus_txAndRx:
            om_port_config->admin_status = VAL_lldpPortConfigAdminStatus_rxOnly;
            break;

        case VAL_lldpPortConfigAdminStatus_disabled:
        case VAL_lldpPortConfigAdminStatus_rxOnly:
        default:
            /* Do nothing
             */
            break;
    }

    return LLDP_TYPE_RETURN_OK;
} /* End of LLDP_MGR_DisableTx */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ProcessReinitDelayPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Waking up the port which is in reinit delay and be configured
 *            to tx enabled
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref. to the description in 10.5.3.3, (c), IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
void LLDP_ENGINE_ProcessReinitDelayPort()
{
    UI32_T                      lport = 0;
    UI32_T                      current_time;
    LLDP_OM_SysConfigEntry_T    *sys_config;
    LLDP_OM_PortConfigEntry_T   *port_config;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();

    if (sys_config->global_admin_status == LLDP_TYPE_SYS_ADMIN_DISABLE)
        return;
    /*EPR: ES4827G-FLF-ZZ-00201
     *Problem:LLDP:the parameter of "reinit-delay" can't work well
     *Solution:The reinit time rang is 1-10,so need add 49,just
     *         check current time is expire or not
     *Approved by:Hardsun
     *Modify file:lldp_engine.c
     */
    SYS_TIME_GetSystemUpTimeByTick(&current_time);
    while (LLDP_UTY_GetNextLogicalPort(&lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);

        /* check the reinit timer */
        if (port_config->reinit_delay_timer < current_time)
        {
            /* 1. reset reinit_delay_timer */
            port_config->reinit_delay_timer = 0;

            /* 2. if need reinit */
            if (port_config->reinit_flag)
            {
                if (port_config->admin_status == VAL_lldpPortConfigAdminStatus_txOnly ||
                    port_config->admin_status == VAL_lldpPortConfigAdminStatus_txAndRx)
                    LLDP_ENGINE_AdminStatusEnableTx(lport);
            }
        }
    }
    /* LLDP_OM_ReinitDelayListRemove(SYSFUN_GetSysTick()); */

    return;
}/* End of LLDP_ENGINE_ProcessReinitDelayPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_ENGINE_ProcessTimerEventTx
 *-------------------------------------------------------------------------
 * PURPOSE  : Check each port if the LLDPDU transmission is necessary
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Ref. to the description int 10.5.3.3, (d), IEEE Std 802.1AB-2005.
 *-------------------------------------------------------------------------
 */
UI32_T LLDP_ENGINE_ProcessTimerEventTx()
{
    LLDP_OM_SysConfigEntry_T    *sys_config;
    LLDP_OM_PortConfigEntry_T   *port_config;
    UI32_T                      current_time;
    UI32_T                      lport = 0;

    sys_config = LLDP_OM_GetSysConfigEntryPtr();
    SYS_TIME_GetSystemUpTimeByTick(&current_time);
    current_time += 49;

    while (LLDP_UTY_GetNextLogicalPort(&lport))
    {
        port_config = LLDP_OM_GetPortConfigEntryPtr(lport);
        if (port_config->tx_delay_timer < current_time)
        {
            port_config->tx_delay_timer = 0;
            if (port_config->transfer_timer < current_time || port_config->something_changed_local == TRUE)
            {
                LLDP_ENGINE_ConstructLLDPDU(lport, LLDP_TYPE_NORMAL_LLDPDU);
            }
        }
        else
        {
            if (port_config->transfer_timer < current_time)
            {
                port_config->transfer_timer = sys_config->msg_tx_interval * LLDP_TYPE_TIME_UNIT + current_time;
            }
        }

#if(SYS_CPNT_DCBX == TRUE)
        if (LLDP_OM_IncreaseDcbxTimer(lport) == TRUE)
        {   /* multi-peer condition detected */
            LLDP_TYPE_NotifyDcbxEtsInfo_T   notify_ets_entry;
            LLDP_TYPE_NotifyDcbxPfcInfo_T   notify_pfc_entry;
            L_LINK_LST_List_T               *notify_list;

            /* notify for ETS */
            notify_list = LLDP_OM_GetNotifyDcbxRemEtsChangedListPtr();
            memset(&notify_ets_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxEtsInfo_T));
            notify_ets_entry.time_mark = current_time;
            notify_ets_entry.lport = lport;
            notify_ets_entry.is_delete = TRUE;
            notify_ets_entry.rem_recommend_rcvd = FALSE;
            notify_ets_entry.rem_config_willing = FALSE;
            notify_ets_entry.rem_config_cbs = FALSE;
            notify_ets_entry.rem_config_max_tc = 0;
            memset(notify_ets_entry.rem_config_pri_assign_table, 0, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memset(notify_ets_entry.rem_config_tc_bandwidth_table, 0, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memset(notify_ets_entry.rem_config_tsa_assign_table, 0, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
            memset(notify_ets_entry.rem_recommend_pri_assign_table, 0, LLDP_TYPE_REM_ETS_PRI_ASSIGN_TABLE_LEN);
            memset(notify_ets_entry.rem_recommend_tc_bandwidth_table, 0, LLDP_TYPE_REM_ETS_TC_BANDWIDTH_TABLE_LEN);
            memset(notify_ets_entry.rem_recommend_tsa_assign_table, 0, LLDP_TYPE_REM_ETS_TSA_ASSIGN_TABLE_LEN);
            if (L_LINK_LST_Set(notify_list, &notify_ets_entry, L_LINK_LST_APPEND) == FALSE)
            {
                return LLDP_TYPE_RETURN_ERROR;
            }

            /* notify for PFC */
            notify_list = LLDP_OM_GetNotifyDcbxRemPfcChangedListPtr();
            memset(&notify_pfc_entry, 0, sizeof(LLDP_TYPE_NotifyDcbxPfcInfo_T));
            notify_pfc_entry.time_mark = current_time;
            notify_pfc_entry.lport = lport;
            notify_pfc_entry.is_delete = TRUE;
            memset(notify_pfc_entry.rem_mac, 0, 6);
            notify_pfc_entry.rem_willing = FALSE;
            notify_pfc_entry.rem_mbc = FALSE;
            notify_pfc_entry.rem_cap = 0;
            notify_pfc_entry.rem_enable = 0;
            if (L_LINK_LST_Set(notify_list, &notify_pfc_entry, L_LINK_LST_APPEND) == FALSE)
            {
                return LLDP_TYPE_RETURN_ERROR;
            }
        }
#endif /* #if(SYS_CPNT_DCBX == TRUE) */
    } /* end while */

    return LLDP_TYPE_RETURN_OK;
}/* End of LLDP_ENGINE_ProcessTimerEventTx */
