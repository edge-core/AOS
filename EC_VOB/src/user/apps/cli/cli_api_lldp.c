#include "sys_cpnt.h"
#include "cli_api.h"
#include "cli_api_lldp.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "lldp_type.h"
#include "l_inet.h"
#include "leaf_ieeelldpext.h"

#if (SYS_CPNT_LLDP == TRUE)

#define CLI_API_Lldp_PrintHex(buf, len) do {     \
    UI32_T i;                                    \
    if (len > 0)                                 \
    {                                            \
        CLI_LIB_PrintStr_N("%02X", buf[0]);      \
        for (i = 1; i < len; i++)                \
            CLI_LIB_PrintStr_N("-%02X", buf[i]); \
    }                                            \
} while (0)

static UI32_T show_one_lldp_config(UI32_T lport, UI32_T line_num);
static UI32_T show_one_lldp_local(UI32_T lport, UI32_T line_num);
static UI32_T show_one_lldp_remote(UI32_T lport, UI32_T line_num);
#if (SYS_CPNT_LLDP_MED == TRUE)
static void   replace_nonprintable_char_with_period(UI8_T *input, UI32_T input_len, char *out_buf, UI32_T out_len);
#endif  /* #if (SYS_CPNT_LLDP_MED==TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp and no lldp in global
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_LLDP:
        {
            if(LLDP_PMGR_SetSysAdminStatus(LLDP_TYPE_SYS_ADMIN_ENABLE) != LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to enable LLDP.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_LLDP:
        {
            if(LLDP_PMGR_SetSysAdminStatus(LLDP_TYPE_SYS_ADMIN_DISABLE) != LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to disable LLDP.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
   return CLI_NO_ERROR;
}

/***Hold time Multiplier***/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_Holdtime
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp holdtime-multiplier and no lldp
 *            holdtime-multiplier in global
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_Holdtime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_HOLDTIMEMULTIPLIER:
        {
            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            if(LLDP_PMGR_SetMsgTxHoldMul(atoi(arg[0]))!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set hold time multiplier.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_HOLDTIMEMULTIPLIER:
        {
            if(LLDP_PMGR_SetMsgTxHoldMul(LLDP_TYPE_DEFAULT_TX_HOLD_MUL)!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set hold time multiplier to default value.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
    return CLI_NO_ERROR;
}

/***Notification Interval***/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_NotificationInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp notification-interval and no lldp
 *            notification-interval in global
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_NotificationInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_NOTIFICATIONINTERVAL:
        {
            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            if(LLDP_PMGR_SetNotificationInterval(atoi(arg[0]))!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set notification interval.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_NOTIFICATIONINTERVAL:
        {
            if(LLDP_PMGR_SetNotificationInterval(LLDP_TYPE_DEFAULT_NOTIFY_INTERVAL)!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set notification interval to default value.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
   }
   return CLI_NO_ERROR;
}

/***Transmission Interval ***/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_TxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp refresh-interval and no lldp
 *            refresh-interval in global.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_TxInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_REFRESHINTERVAL:
        {
            if(arg[0]==NULL)
            {
                return CLI_ERR_INTERNAL;
            }

            if(LLDP_PMGR_SetMsgTxInterval(atoi(arg[0]))!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set refresh interval.\r\nThe refresh interval must be greater than or equal to 4 * delay interval.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_REFRESHINTERVAL:
        {
            if(LLDP_PMGR_SetMsgTxInterval(LLDP_TYPE_DEFAULT_TX_INTERVAL)!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set refresh interval to default value.\r\nThe refresh interval must be greater than or equal to 4 * delay interval.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
   return CLI_NO_ERROR;
}

/***Reinitialization Delay ***/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_ReinitDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp reinitialization-delay and no lldp
 *            reinitialization-delay in global.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_ReinitDelay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_REINITDELAY:
        {
            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            if(LLDP_PMGR_SetReinitDelay(atoi(arg[0]))!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set reinitialization delay.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_REINITDELAY:
        {
            if(LLDP_PMGR_SetReinitDelay(LLDP_TYPE_DEFAULT_REINIT_DELAY)!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set reinitialization delay to default value.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
    }
   return CLI_NO_ERROR;
}

/***Delay Interval ***/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_TxDelay
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp tx-delay and no lldp
 *            tx-delay in global.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_TxDelay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_TXDELAY:
        {
            if(LLDP_PMGR_SetTxDelay(atoi(arg[0]))!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set delay interval.\r\n(4*delay-interval) must be less than or equal to refresh interval.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_TXDELAY:
        {
            if(LLDP_PMGR_SetTxDelay(LLDP_TYPE_DEFAULT_TX_DELAY)!=LLDP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to set delay interval to default value.\r\n(4*delay-interval) must be less than or equal to refresh interval.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }
   }
   return CLI_NO_ERROR;
}

/*****************************************************************************************/
/****eth****/
UI32_T CLI_API_Lldp_PortNotification_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i       = 0;
    UI32_T status  = 0;
    UI32_T lport   = 0;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_NOTIFICATION:
            status = VAL_lldpPortConfigNotificationEnable_true;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_NOTIFICATION:
            status =  VAL_lldpPortConfigNotificationEnable_false;
            break;

        default:
           return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (LLDP_PMGR_SetPortConfigNotificationEnable(lport, status)!=LLDP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to %s notification on ethernet %s.\r\n", status == TRUE ? "enable" : "disable", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to %s notification on ethernet %lu/%lu.\r\n", status == TRUE ? "enable" : "disable",
                                                                                (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortAdminStatus_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp admin-status and no lldp
 *            admin-status in interface eth.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortAdminStatus_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T admin_status    = 0;
    UI32_T i               = 0;
    UI32_T ifindex         = 0;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_ADMINSTATUS:
            if(arg[0]!=NULL)
            {
                /**rx-only**/
                if (*(arg[0]) == 'R' || *(arg[0]) == 'r')
                     admin_status = VAL_lldpPortConfigAdminStatus_rxOnly;
                /**tx-rx**/
                else if ((*(arg[0]) == 'T' || *(arg[0]) == 't')&&(*(arg[0]+3) == 'R' || *(arg[0]+3) == 'r'))
                    admin_status = VAL_lldpPortConfigAdminStatus_txAndRx;
                /**tx only**/
                else
                    admin_status = VAL_lldpPortConfigAdminStatus_txOnly;
            }
            else
                 return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_ADMINSTATUS:
            admin_status = VAL_lldpPortConfigAdminStatus_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                if (LLDP_PMGR_SetPortConfigAdminStatus(ifindex, admin_status)!=LLDP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set admin-status on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set admin-status on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
        }
    }
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortTlvStatus_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (tlv) in interface eth.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortTlvStatus_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    UI32_T  verify_ret;
    UI32_T  i = 0;
    UI8_T   tlv = 0;
    LLDP_MGR_PortConfigEntry_T  port_config_entry;


    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                port_config_entry.port_num = lport;

                if (LLDP_PMGR_GetPortConfigEntry(&port_config_entry) != LLDP_TYPE_RETURN_OK)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set basic-tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }

                tlv = port_config_entry.basic_tlvs_tx_flag;

                if(arg[0]!=NULL)
                {
                    switch(cmd_idx)
                    {
                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_BASICTLV:
                            /** Management Address **/
                            if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                            {
                                if(LLDP_PMGR_SetConfigManAddrTlv(lport,TRUE)!=LLDP_TYPE_RETURN_OK)
                                {
                                    CLI_LIB_PrintStr_N("Failed to enable management-address\r\n");
                                }
                            }
                            /** Port Description **/
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                tlv |= LLDP_TYPE_TX_PORT_DESC_TLV;
                            }
                            /** System Name **/
                            else if(*(arg[0]+7) == 'N' || *(arg[0]+7) == 'n')
                            {
                                tlv |= LLDP_TYPE_TX_SYS_NAME_TLV;
                            }
                            /** System Description **/
                            else if(*(arg[0]+7) == 'D' || *(arg[0]+7) == 'd')
                            {
                                tlv |= LLDP_TYPE_TX_SYS_DESC_TLV;
                            }
                            /** System Capabilities **/
                            else if(*(arg[0]+7) == 'C' || *(arg[0]+7) == 'c')
                            {
                                tlv |= LLDP_TYPE_TX_SYS_CAP_TLV;
                            }
                            break;

                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_BASICTLV:
                            /** Management Address **/
                            if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                            {
                                if(LLDP_PMGR_SetConfigManAddrTlv(lport,FALSE)!=LLDP_TYPE_RETURN_OK)
                                {
                                    CLI_LIB_PrintStr_N("Failed to disable management-address\r\n");
                                }
                            }
                            /** Port Description **/
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                tlv &= ~(LLDP_TYPE_TX_PORT_DESC_TLV);
                            }
                            /** System Name **/
                            else if(*(arg[0]+7) == 'N' || *(arg[0]+7) == 'n')
                            {
                                tlv &= ~(LLDP_TYPE_TX_SYS_NAME_TLV);
                            }
                            /** System Description **/
                            else if(*(arg[0]+7) == 'D' || *(arg[0]+7) == 'd')
                            {
                                tlv &= ~(LLDP_TYPE_TX_SYS_DESC_TLV);
                            }
                            /** System Capabilities **/
                            else if(*(arg[0]+7) == 'C' || *(arg[0]+7) == 'c')
                            {
                                tlv &= ~(LLDP_TYPE_TX_SYS_CAP_TLV);
                            }
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                    } /*switch(cmd_idx)*/
                } /*if(arg[0]!=NULL)*/
                else
                    return CLI_ERR_INTERNAL;

                if (LLDP_PMGR_SetPortOptionalTlvStatus(lport, tlv)!=LLDP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set basic-tlv on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set basic-tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
        }
    }

    return CLI_NO_ERROR;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortDot1TlvStatus_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (802.1 Organizationally Specific TLVs) in interface eth.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortDot1TlvStatus_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_EXT==TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    UI32_T  verify_ret;
    UI32_T  i = 0;
    LLDP_MGR_Xdot1ConfigEntry_T  port_config_entry;


    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                port_config_entry.lport = lport;
                if(!LLDP_PMGR_GetXdot1ConfigEntry(&port_config_entry))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot1tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }

                if(arg[0]!=NULL)
                {
                    switch(cmd_idx)
                    {
                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_DOT1TLV:
                            /** vlan-name **/
                            if (*(arg[0]) == 'V' || *(arg[0]) == 'v')
                            {
                                port_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_true;
                            }
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                /** pvid **/
                                if (*(arg[0]+1) == 'V' || *(arg[0]+1) == 'v')
                                {
                                    port_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_true;
                                }
                                /** proto-vid **/
                                else if (*(arg[0]+6) == 'V' || *(arg[0]+6) == 'v')
                                {
                                    port_config_entry.proto_vlan_tx_enable = VAL_lldpXdot1ConfigProtoVlanTxEnable_true;
                                }
                                /** proto-ident **/
                                else if (*(arg[0]+6) == 'I' || *(arg[0]+6) == 'i')
                                {
                                    port_config_entry.protocol_tx_enable = VAL_lldpXdot1ConfigProtocolTxEnable_true;
                                }
                            }
#if (SYS_CPNT_CN == TRUE)
                            /** cn **/
                            else if (*(arg[0]) == 'C' || *(arg[0]) == 'c')
                            {
                                port_config_entry.cn_tx_enable = TRUE;
                            }
#endif
                            break;

                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_DOT1TLV:
                            /** vlan-name **/
                            if (*(arg[0]) == 'V' || *(arg[0]) == 'v')
                            {
                                port_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_false;
                            }
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                /** pvid **/
                                if (*(arg[0]+1) == 'V' || *(arg[0]+1) == 'v')
                                {
                                    port_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_false;
                                }
                                /** proto-vid **/
                                else if (*(arg[0]+6) == 'V' || *(arg[0]+6) == 'v')
                                {
                                    port_config_entry.proto_vlan_tx_enable = VAL_lldpXdot1ConfigProtoVlanTxEnable_false;
                                }
                                /** proto-ident **/
                                else if (*(arg[0]+6) == 'I' || *(arg[0]+6) == 'i')
                                {
                                    port_config_entry.protocol_tx_enable = VAL_lldpXdot1ConfigProtocolTxEnable_false;
                                }
                            }
#if (SYS_CPNT_CN == TRUE)
                            /** cn **/
                            else if (*(arg[0]) == 'C' || *(arg[0]) == 'c')
                            {
                                port_config_entry.cn_tx_enable = FALSE;
                            }
#endif
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
                else
                    return CLI_ERR_INTERNAL;

                if (LLDP_PMGR_SetXdot1ConfigEntry(&port_config_entry)!=LLDP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot1tlv on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot1tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
        }
    }
#endif  /* #if (SYS_CPNT_LLDP_EXT==TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortDot3TlvStatus_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (802.3 Organizationally Specific TLVs) in interface eth.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortDot3TlvStatus_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_EXT==TRUE)
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    UI32_T  verify_ret;
    UI32_T  i = 0;
    LLDP_MGR_Xdot3PortConfigEntry_T  port_config_entry;

    memset(&port_config_entry, 0, sizeof(LLDP_MGR_Xdot3PortConfigEntry_T));

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                port_config_entry.lport = lport;
                if(!LLDP_PMGR_GetXdot3PortConfigEntry(&port_config_entry))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot3tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }

                if(arg[0]!=NULL)
                {
                    switch(cmd_idx)
                    {
                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_DOT3TLV:
                            /** link-agg **/
                            if (*(arg[0]) == 'L' || *(arg[0]) == 'l')
                            {
                                port_config_entry.link_agg_tlv_enabled = TRUE;
                            }
                            /** poe **/
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                port_config_entry.power_via_mdi_tlv_enabled = TRUE;
                            }
                            else if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                            {
                                /** mac-phy **/
                                if (*(arg[0]+2) == 'C' || *(arg[0]+2) == 'c')
                                {
                                    port_config_entry.mac_phy_tlv_enabled = TRUE;
                                }
                                /** max-frame **/
                                else if (*(arg[0]+2) == 'X' || *(arg[0]+2) == 'x')
                                {
                                    port_config_entry.max_frame_size_tlv_enabled = TRUE;
                                }
                            }
                            break;

                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_DOT3TLV:
                            /** link-agg **/
                            if (*(arg[0]) == 'L' || *(arg[0]) == 'l')
                            {
                                port_config_entry.link_agg_tlv_enabled = FALSE;
                            }
                            /** poe **/
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                port_config_entry.power_via_mdi_tlv_enabled = FALSE;
                            }
                            else if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                            {
                                /** mac-phy **/
                                if (*(arg[0]+2) == 'C' || *(arg[0]+2) == 'c')
                                {
                                    port_config_entry.mac_phy_tlv_enabled = FALSE;
                                }
                                /** max-frame **/
                                else if (*(arg[0]+2) == 'X' || *(arg[0]+2) == 'x')
                                {
                                    port_config_entry.max_frame_size_tlv_enabled = FALSE;
                                }
                            }
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
                else
                    return CLI_ERR_INTERNAL;

                if (LLDP_PMGR_SetXdot3PortConfig(&port_config_entry)!=LLDP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot3tlv on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set dot3tlv on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
        }
    }
#endif  /* #if (SYS_CPNT_LLDP_EXT==TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortNotification_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp admin-status and no lldp
 *            admin-status in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortNotification_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T status  = 0;
    UI32_T ifindex = 0;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_NOTIFICATION:
            status = VAL_lldpPortConfigNotificationEnable_true;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_NOTIFICATION:
            status = VAL_lldpPortConfigNotificationEnable_false;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (LLDP_PMGR_SetPortConfigNotificationEnable(ifindex, status)!=LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set notification.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortAdminStatus_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T admin_status    = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_ADMINSTATUS:
            if(arg[0]!=NULL)
            {
                /**rx-only**/
                if (*(arg[0]) == 'R' || *(arg[0]) == 'r')
                     admin_status = VAL_lldpPortConfigAdminStatus_rxOnly;
                /**tx-rx**/
                else if ((*(arg[0]) == 'T' || *(arg[0]) == 't')&&(*(arg[0]+3) == 'R' || *(arg[0]+3) == 'r'))
                    admin_status = VAL_lldpPortConfigAdminStatus_txAndRx;
                /**tx only**/
                else
                    admin_status = VAL_lldpPortConfigAdminStatus_txOnly;
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_ADMINSTATUS:
            admin_status = VAL_lldpPortConfigAdminStatus_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }


    if (LLDP_PMGR_SetPortConfigAdminStatus(ifindex, admin_status)!=LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set admin-status.\r\n");
#endif
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortTlvStatus_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (tlv) in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortTlvStatus_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T  lport   = 0;
    UI8_T   tlv = 0;
    LLDP_MGR_PortConfigEntry_T  port_config_entry;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    port_config_entry.port_num = lport;
    if (LLDP_PMGR_GetPortConfigEntry(&port_config_entry) != LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set basic-tlv Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
        return CLI_NO_ERROR;
    }

    tlv = port_config_entry.basic_tlvs_tx_flag;

    if(arg[0]!=NULL)
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_BASICTLV:
                /** Management Address **/
                if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                {
                    if(LLDP_PMGR_SetConfigManAddrTlv(lport, TRUE)!=LLDP_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr_N("Failed to enable management-address\r\n");
                     }
                }

                /** Port Description **/
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    tlv |= (LLDP_TYPE_TX_PORT_DESC_TLV);
                }
                /** System Name **/
                else if(*(arg[0]+7) == 'N' || *(arg[0]+7) == 'n')
                {
                    tlv |= (LLDP_TYPE_TX_SYS_NAME_TLV);
                }
                /** System Description **/
                else if(*(arg[0]+7) == 'D' || *(arg[0]+7) == 'd')
                {
                    tlv |= (LLDP_TYPE_TX_SYS_DESC_TLV);
                }
                /** System Capabilities **/
                else if(*(arg[0]+7) == 'C' || *(arg[0]+7) == 'c')
                {
                    tlv |= (LLDP_TYPE_TX_SYS_CAP_TLV);
                }
                break;

            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_BASICTLV:
                /** Management Address **/
                if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                {
                    if(LLDP_PMGR_SetConfigManAddrTlv(lport,FALSE)!=LLDP_TYPE_RETURN_OK)
                    {
                        CLI_LIB_PrintStr_N("Failed to disable management-address\r\n");
                    }
                }
                /** Port Description **/
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    tlv &= ~(LLDP_TYPE_TX_PORT_DESC_TLV);
                }
                /** System Name **/
                else if(*(arg[0]+7) == 'N' || *(arg[0]+7) == 'n')
                {
                    tlv &= ~(LLDP_TYPE_TX_SYS_NAME_TLV);
                }
                /** System Description **/
                else if(*(arg[0]+7) == 'D' || *(arg[0]+7) == 'd')
                {
                    tlv &= ~(LLDP_TYPE_TX_SYS_DESC_TLV);
                }
                /** System Capabilities **/
                else if(*(arg[0]+7) == 'C' || *(arg[0]+7) == 'c')
                {
                    tlv &= ~(LLDP_TYPE_TX_SYS_CAP_TLV);
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    if (LLDP_PMGR_SetPortOptionalTlvStatus(lport, tlv) != LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set basic-tlv Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortDot1TlvStatus_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (802.1 Organizationally Specific TLVs) in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortDot1TlvStatus_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T  lport   = 0;
    LLDP_MGR_Xdot1ConfigEntry_T  port_config_entry;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    port_config_entry.lport = lport;

    if(!LLDP_PMGR_GetXdot1ConfigEntry(&port_config_entry))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set dot1tlv Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
        return CLI_NO_ERROR;
    }

    if(arg[0]!=NULL)
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_DOT1TLV:
                /** vlan-name **/
                if (*(arg[0]) == 'V' || *(arg[0]) == 'v')
                {
                    port_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_true;
                }
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    /** pvid **/
                    if (*(arg[0]+1) == 'V' || *(arg[0]+1) == 'v')
                    {
                        port_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_true;
                    }
                    /** proto-vid **/
                    else if (*(arg[0]+6) == 'V' || *(arg[0]+6) == 'v')
                    {
                        port_config_entry.proto_vlan_tx_enable = VAL_lldpXdot1ConfigProtoVlanTxEnable_true;
                    }
                    /** protocol-ident **/
                    else if (*(arg[0]+6) == 'I' || *(arg[0]+6) == 'i')
                    {
                        port_config_entry.protocol_tx_enable = VAL_lldpXdot1ConfigProtocolTxEnable_true;
                    }
                }
#if (SYS_CPNT_CN == TRUE)
                /** cn **/
                else if (*(arg[0]) == 'C' || *(arg[0]) == 'c')
                {
                    port_config_entry.cn_tx_enable = TRUE;
                }
#endif
                break;

            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_DOT1TLV:
                /** vlan-name **/
                if (*(arg[0]) == 'V' || *(arg[0]) == 'v')
                {
                    port_config_entry.vlan_name_tx_enable = VAL_lldpXdot1ConfigVlanNameTxEnable_false;
                }
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    /** pvid **/
                    if (*(arg[0]+1) == 'V' || *(arg[0]+1) == 'v')
                    {
                        port_config_entry.port_vlan_tx_enable = VAL_lldpXdot1ConfigPortVlanTxEnable_false;
                    }
                    /** proto-vid **/
                    else if (*(arg[0]+6) == 'V' || *(arg[0]+6) == 'v')
                    {
                        port_config_entry.proto_vlan_tx_enable = VAL_lldpXdot1ConfigProtoVlanTxEnable_false;
                    }
                    /** protocol-ident **/
                    else if (*(arg[0]+6) == 'I' || *(arg[0]+6) == 'i')
                    {
                        port_config_entry.protocol_tx_enable = VAL_lldpXdot1ConfigProtocolTxEnable_false;
                    }
                }
#if (SYS_CPNT_CN == TRUE)
                /** cn **/
                else if (*(arg[0]) == 'C' || *(arg[0]) == 'c')
                {
                    port_config_entry.cn_tx_enable = FALSE;
                }
#endif
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    if (LLDP_PMGR_SetXdot1ConfigEntry(&port_config_entry) != LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set dot1tlv Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Lldp_PortDot3TlvStatus_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp config (tlv) and no lldp
 *            config (802.3 Organizationally Specific TLVs) in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Lldp_PortDot3TlvStatus_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T  lport   = 0;
    LLDP_MGR_Xdot3PortConfigEntry_T  port_config_entry;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    memset(&port_config_entry, 0, sizeof(LLDP_MGR_Xdot3PortConfigEntry_T));
    port_config_entry.lport = lport;

    if(!LLDP_PMGR_GetXdot3PortConfigEntry(&port_config_entry))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set dot3tlv on Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
        return CLI_NO_ERROR;
    }

    if(arg[0]!=NULL)
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_DOT3TLV:
                /** link-agg **/
                if (*(arg[0]) == 'L' || *(arg[0]) == 'l')
                {
                    port_config_entry.link_agg_tlv_enabled = TRUE;
                }
                /** poe **/
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    port_config_entry.power_via_mdi_tlv_enabled = TRUE;
                }
                else if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                {
                    /** mac-phy **/
                    if (*(arg[0]+2) == 'C' || *(arg[0]+2) == 'c')
                    {
                        port_config_entry.mac_phy_tlv_enabled = TRUE;
                    }
                    /** max-frame **/
                    else if (*(arg[0]+2) == 'X' || *(arg[0]+2) == 'x')
                    {
                        port_config_entry.max_frame_size_tlv_enabled = TRUE;
                    }
                }
                break;

            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_DOT3TLV:
                /** link-agg **/
                if (*(arg[0]) == 'L' || *(arg[0]) == 'l')
                {
                    port_config_entry.link_agg_tlv_enabled = FALSE;
                }
                /** poe **/
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    port_config_entry.power_via_mdi_tlv_enabled = FALSE;
                }
                else if (*(arg[0]) == 'M' || *(arg[0]) == 'm')
                {
                    /** mac-phy **/
                    if (*(arg[0]+2) == 'C' || *(arg[0]+2) == 'c')
                    {
                        port_config_entry.mac_phy_tlv_enabled = FALSE;
                    }
                    /** max-frame **/
                    else if (*(arg[0]+2) == 'X' || *(arg[0]+2) == 'x')
                    {
                        port_config_entry.max_frame_size_tlv_enabled = FALSE;
                    }
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    if (LLDP_PMGR_SetXdot3PortConfig(&port_config_entry) != LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set dot3tlv on Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_LLDP_MED==TRUE)
/*****************************************************************************************/
static void replace_nonprintable_char_with_period(UI8_T *input, UI32_T input_len, char *out_buf, UI32_T out_len)
{
    if (input == NULL || input_len == 0 || out_buf == NULL || out_len == 0)
    {
        return;
    }

    for (; 1 <= input_len && 1 <= out_len; ++ input, ++ out_buf, -- input_len, -- out_len)
    {
        if ((*input < 0x20) || (0x7E < *input))
        {
            *out_buf = 0x2E;
        }
        else
        {
            *out_buf = *input;
        }
    }

    *out_buf = '\0';
}
#endif  /* #if (SYS_CPNT_LLDP_MED==TRUE) */

static UI32_T show_one_lldp_config(UI32_T lport, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI32_T  unit_id;
    UI32_T  port_id;
    UI32_T  trunk_id;
    BOOL_T  is_trunk;

    LLDP_MGR_PortConfigEntry_T  port_config_entry;

    switch (SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id))
    {
        case SWCTRL_LPORT_NORMAL_PORT:
        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            is_trunk = FALSE;
            break;
        case SWCTRL_LPORT_TRUNK_PORT:
            is_trunk = TRUE;
            break;
        default:
            return JUMP_OUT_MORE;
    }

    port_config_entry.port_num = lport;

    if (LLDP_PMGR_GetPortConfigEntry(&port_config_entry) == LLDP_TYPE_RETURN_OK)
    {
        LLDP_MGR_Xdot3PortConfigEntry_T dot3_port_config;
        LLDP_MGR_Xdot1ConfigEntry_T     dot1_port_config;
        UI8_T   tlv, enabled;
        BOOL_T  first_line;

        PROCESS_MORE_FUNC("LLDP Port Configuration Detail\r\n");

        if(is_trunk)
        {
            CLI_LIB_PrintStr_N(" Port                           : Trunk %lu", (unsigned long)trunk_id);
        }
        else
        {
            CLI_LIB_PrintStr_N(" Port                           : Eth %lu/%lu", (unsigned long)unit_id, (unsigned long)port_id);
        }
        PROCESS_MORE_FUNC("\r\n");

        if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_txOnly)
        {
            CLI_LIB_PrintStr_N(" Admin Status                   : Tx");
        }
        else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_rxOnly)
        {
            CLI_LIB_PrintStr_N(" Admin Status                   : Rx");
        }
        else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_txAndRx)
        {
            CLI_LIB_PrintStr_N(" Admin Status                   : Tx-Rx");
        }
        else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_disabled)
        {
            CLI_LIB_PrintStr_N(" Admin Status                   : Disabled");
        }
        PROCESS_MORE_FUNC("\r\n");

        if (port_config_entry.notification_enable == VAL_lldpPortConfigNotificationEnable_false)
        {
            CLI_LIB_PrintStr_N(" Notification Enabled           : False");
        }
        else
        {
            CLI_LIB_PrintStr_N(" Notification Enabled           : True");
        }
        PROCESS_MORE_FUNC("\r\n");

        CLI_LIB_PrintStr_N(" Basic TLVs Advertised          :");
        tlv = port_config_entry.basic_tlvs_tx_flag;
        first_line = TRUE;

        /* testing Port Description tlv */
        if ((tlv & LLDP_TYPE_TX_PORT_DESC_TLV) != 0)
        {
            PROCESS_MORE_FUNC(" port-description\r\n");
            first_line = FALSE;
        }

        /* testing System Name tlv */
        if ((tlv & LLDP_TYPE_TX_SYS_NAME_TLV) != 0)
        {
            if (first_line == TRUE)
            {
                PROCESS_MORE_FUNC(" system-name\r\n");
                first_line = FALSE;
            }
            else
            {
                PROCESS_MORE_FUNC("                                  system-name\r\n");
            }
        }

        /* testing System Description tlv */
        if ((tlv & LLDP_TYPE_TX_SYS_DESC_TLV) != 0)
        {
            if (first_line == TRUE)
            {
                PROCESS_MORE_FUNC(" system-description\r\n");
                first_line = FALSE;
            }
            else
            {
                PROCESS_MORE_FUNC("                                  system-description\r\n");
            }
        }

        /* testing System Capabilities tlv */
        if ((tlv & LLDP_TYPE_TX_SYS_CAP_TLV) != 0)
        {
            if (first_line == TRUE)
            {
                PROCESS_MORE_FUNC(" system-capabilities\r\n");
                first_line = FALSE;
            }
            else
            {
                PROCESS_MORE_FUNC("                                  system-capabilities\r\n");
            }
        }

        if (LLDP_PMGR_GetConfigManAddrTlv(port_config_entry.port_num, &enabled) == LLDP_TYPE_RETURN_OK)
        {
            if(enabled == 1)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" management-ip-address\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  management-ip-address\r\n");
                }
            }
        }

        if (first_line == TRUE)
            PROCESS_MORE_FUNC("\r\n");
#if (SYS_CPNT_LLDP_EXT==TRUE)
        dot1_port_config.lport = lport;
        if (LLDP_PMGR_GetXdot1ConfigEntry(&dot1_port_config))
        {
            CLI_LIB_PrintStr_N(" 802.1 specific TLVs Advertised :");
            first_line = TRUE;

            if (dot1_port_config.port_vlan_tx_enable == VAL_lldpXdot1ConfigPortVlanTxEnable_true)
            {
                PROCESS_MORE_FUNC(" port-vid\r\n");
                first_line = FALSE;
            }

            if (dot1_port_config.vlan_name_tx_enable == VAL_lldpXdot1ConfigVlanNameTxEnable_true)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" vlan-name\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  vlan-name\r\n");
                }
            }

            if (dot1_port_config.proto_vlan_tx_enable == VAL_lldpXdot1ConfigProtoVlanTxEnable_true)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" proto-vlan\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  proto-vlan\r\n");
                }
            }

            if (dot1_port_config.protocol_tx_enable == VAL_lldpXdot1ConfigProtocolTxEnable_true)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" proto-ident\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  proto-ident\r\n");
                }
            }

#if (SYS_CPNT_CN == TRUE)
            if (dot1_port_config.cn_tx_enable == TRUE)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" cn\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  cn\r\n");
                }
            }
#endif

            if (first_line == TRUE)
                PROCESS_MORE_FUNC("\r\n");
        }

        dot3_port_config.lport = lport;
        if (LLDP_PMGR_GetXdot3PortConfigEntry(&dot3_port_config))
        {
            CLI_LIB_PrintStr_N(" 802.3 specific TLVs Advertised :");
            first_line = TRUE;

            if (dot3_port_config.mac_phy_tlv_enabled)
            {
                PROCESS_MORE_FUNC(" mac-phy\r\n");
                first_line = FALSE;
            }

#if (SYS_CPNT_POE==TRUE)
            if (dot3_port_config.power_via_mdi_tlv_enabled)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" poe\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  poe\r\n");
                }
            }
#endif

            if (dot3_port_config.link_agg_tlv_enabled)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" link-agg\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  link-agg\r\n");
                }
            }

            if (dot3_port_config.max_frame_size_tlv_enabled)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC(" max-frame\r\n");
                    first_line = FALSE;
                }
                else
                {
                    PROCESS_MORE_FUNC("                                  max-frame\r\n");
                }
            }

            if (first_line == TRUE)
                PROCESS_MORE_FUNC("\r\n");
        }
#endif  /* #if (SYS_CPNT_LLDP_EXT==TRUE) */
#if(SYS_CPNT_LLDP_MED == TRUE)
        {
            LLDP_MGR_XMedPortConfigEntry_T  med_port_config_entry;

            med_port_config_entry.lport = port_config_entry.port_num;

            if (LLDP_PMGR_GetXMedPortConfigEntry(&med_port_config_entry))
            {
                UI16_T lldp_xmed_port_tlvs_tx_enabled = med_port_config_entry.lldp_xmed_port_tlvs_tx_enabled;
                BOOL_T status;

                if (VAL_lldpXMedPortConfigNotifEnable_true == med_port_config_entry.lldp_xmed_port_notif_enabled)
                {
                    PROCESS_MORE_FUNC(" MED Notification Status        : Enabled\r\n");
                }
                else
                {
                    PROCESS_MORE_FUNC(" MED Notification Status        : Disabled\r\n");
                }

                CLI_LIB_PrintStr_N(" MED Enabled TLVs Advertised    :");
                first_line = TRUE;

                if (lldp_xmed_port_tlvs_tx_enabled & LLDP_TYPE_MED_CAP_TX)
                {
                    PROCESS_MORE_FUNC(" med-cap\r\n");
                    first_line = FALSE;
                }
                if (lldp_xmed_port_tlvs_tx_enabled& LLDP_TYPE_MED_NETWORK_POLICY_TX)
                {
                    if (first_line == TRUE)
                    {
                        PROCESS_MORE_FUNC(" network-policy\r\n");
                        first_line = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  network-policy\r\n");
                    }
                }
                if (lldp_xmed_port_tlvs_tx_enabled& LLDP_TYPE_MED_LOCATION_IDENT_TX)
                {
                    if (first_line == TRUE)
                    {
                        PROCESS_MORE_FUNC(" location\r\n");
                        first_line = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  location\r\n");
                    }
                }
    #if (SYS_CPNT_POE==TRUE)
                if ((lldp_xmed_port_tlvs_tx_enabled& LLDP_TYPE_MED_EXT_PD_TX) ||
                    (lldp_xmed_port_tlvs_tx_enabled & LLDP_TYPE_MED_EXT_PSE_TX))
                {
                    if (first_line == TRUE)
                    {
                        PROCESS_MORE_FUNC(" ext-poe\r\n");
                        first_line = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  ext-poe\r\n");
                    }
                }
    #endif
                if (lldp_xmed_port_tlvs_tx_enabled & LLDP_TYPE_MED_INVENTORY_TX)
                {
                    if (first_line == TRUE)
                    {
                        PROCESS_MORE_FUNC(" inventory\r\n");
                        first_line = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  inventory\r\n");
                    }
                }
                if (first_line == TRUE)
                    PROCESS_MORE_FUNC("\r\n");

                /* location config */
                PROCESS_MORE_FUNC(" MED Location Identification\r\n");

                if (LLDP_PMGR_GetXMedLocLocationStatus(med_port_config_entry.lport, VAL_lldpXMedLocLocationSubtype_coordinateBased, &status) &&
                    status == TRUE)
                {
#if 0 // wakka: not implelemt
                    PROCESS_MORE_FUNC("  Location Data Format : Coordinate-based LCI\r\n");
                    PROCESS_MORE_FUNC("  Location ID          :\r\n");
                    CLI_LIB_PrintStr_N("      %.*s", (int)loc_location_entry.location_info_len, loc_location_entry.location_info);
                    PROCESS_MORE_FUNC("\r\n");
#endif
                }

                if (LLDP_PMGR_GetXMedLocLocationStatus(med_port_config_entry.lport, VAL_lldpXMedLocLocationSubtype_civicAddress, &status) &&
                    status == TRUE)
                {
                    LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;
                    UI8_T country_code[2], what;
                    BOOL_T ret;

                    PROCESS_MORE_FUNC("  Location Data Format : Civic Address LCI\r\n");

                    if (LLDP_PMGR_GetXMedLocLocationCivicAddrCoutryCode(med_port_config_entry.lport, country_code))
                    {
                        CLI_LIB_PrintStr_N("  Country Name         : %.*s", 2, country_code);
                        PROCESS_MORE_FUNC("\r\n");
                    }
                    if (LLDP_PMGR_GetXMedLocLocationCivicAddrWhat(med_port_config_entry.lport, &what))
                    {
                        switch (what)
                        {
                            case 0:
                                CLI_LIB_PrintStr_N("  What                 : 0 - DHCP Server");
                                break;

                            case 1:
                                CLI_LIB_PrintStr_N("  What                 : 1 - Nearest Network Element");
                                break;

                            case 2:
                                CLI_LIB_PrintStr_N("  What                 : 2 - DHCP Client");
                                break;
                        }
                        PROCESS_MORE_FUNC("\r\n");
                    }

                    for (ret = LLDP_PMGR_Get1stXMedLocLocationCivicAddrCaEntry(med_port_config_entry.lport, &ca_entry);
                        ret;
                        ret = LLDP_PMGR_GetNextXMedLocLocationCivicAddrCaEntry(med_port_config_entry.lport, &ca_entry))
                    {
                        CLI_LIB_PrintStr_N("  CA Type %-3u", ca_entry.ca_type);
                        CLI_LIB_PrintStr_N("          : %.*s", ca_entry.ca_length, ca_entry.ca_value);
                        PROCESS_MORE_FUNC("\r\n");
                    }
                }

                if (LLDP_PMGR_GetXMedLocLocationStatus(med_port_config_entry.lport, VAL_lldpXMedLocLocationSubtype_elin, &status) &&
                    status == TRUE)
                {
#if 0 // wakka: not implelemt
                    PROCESS_MORE_FUNC("  Location Data Format : ECS ELIN\r\n");
                    PROCESS_MORE_FUNC("  ELIN                 :\r\n");
                    CLI_LIB_PrintStr_N("      %.*s", (int)loc_location_entry.location_info_len, loc_location_entry.location_info);
                    PROCESS_MORE_FUNC("\r\n");
#endif
                }
            }
        }
#endif

#if(SYS_CPNT_DCBX == TRUE)
        {
            LLDP_TYPE_XdcbxPortConfigEntry_T  dcbx_port_config_entry;
            BOOL_T is_first = TRUE;

            dcbx_port_config_entry.lport = lport;
            if(LLDP_PMGR_GetXdcbxPortConfig(&dcbx_port_config_entry) == LLDP_TYPE_RETURN_OK)
            {
                CLI_LIB_PrintStr_N(" DCBX specific TLVs Advertised  : ");
                if(dcbx_port_config_entry.ets_config_tlv_enable)
                {
                    if(is_first)
                    {
                        PROCESS_MORE_FUNC("ets-config\r\n");
                        is_first = FALSE;
                    }
                }
                if(dcbx_port_config_entry.ets_recommend_tlv_enable)
                {
                    if(is_first)
                    {
                        PROCESS_MORE_FUNC("ets-recommend\r\n");
                        is_first = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  ets-recommend\r\n");
                    }
                }
                if(dcbx_port_config_entry.pfc_config_tlv_enable)
                {
                    if(is_first)
                    {
                        PROCESS_MORE_FUNC("pfc-config\r\n");
                        is_first = FALSE;
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("                                  pfc-config\r\n");
                    }
                }
            }
        }
#endif
    }

    return line_num;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_LldpConfig
 *-------------------------------------------------------------------------
 * PURPOSE  : Show the information of LLDP config (global and interface)
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_LldpConfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T unit, i, trunk_id;
    UI32_T err_idx;
    UI32_T lport;
    CLI_API_EthStatus_T verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char *s;
    char delemiters[2] = {0};

    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_SHOW_LLDP_CONFIG_DETAIL:
        {
            if (arg[0][0] == 'e' || arg[0][0] == 'E')
            {
                s = arg[1];

                /*get the unit*/
                unit = atoi(s);

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
                        for(i=lower_val; i<=upper_val; i++)
                        {
                            verify_ret_e = verify_ethernet(unit, i, &lport);

                            if ((verify_ret_e == CLI_API_ETH_NOT_PRESENT) || (verify_ret_e == CLI_API_ETH_UNKNOWN_PORT))
                            {
                                display_ethernet_msg(verify_ret_e, unit, i);
                                continue;
                            }

                            if ((line_num = show_one_lldp_config(lport, line_num)) ==  JUMP_OUT_MORE)
                            {
                                return CLI_NO_ERROR;
                            }
                            else if (line_num == EXIT_SESSION_MORE)
                            {
                                return CLI_EXIT_SESSION;
                            }
                            else
                            {
                                PROCESS_MORE("\r\n");
                            }
                        }
                    }
                    if(s == 0)
                        break;
                    else
                        memset(Token, 0, sizeof(Token));
                }

            }
            else if (arg[0][0] == 'p' || arg[0][0] == 'P')
            {
                trunk_id = atoi(arg[1]);
                if ((verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, trunk_id);
                    return CLI_NO_ERROR;
                }

                if((line_num = show_one_lldp_config(lport, line_num)) ==  JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                else
                {
                    PROCESS_MORE("\r\n");
                }
            }
            break;
        }
        case PRIVILEGE_EXEC_CMD_W3_SHOW_LLDP_CONFIG:
        {
            LLDP_MGR_SysConfigEntry_T   sys_config_entry;
            LLDP_MGR_PortConfigEntry_T  port_config_entry;
#if (SYS_CPNT_LLDP_MED == TRUE)
            LLDP_MGR_XMedConfig_T       xmed_config_entry;
#endif
            UI32_T  admin_status;
            UI32_T  unit, port, trunk_id;

            port_config_entry.port_num = 0;

            PROCESS_MORE("LLDP Global Configuration\r\n");

            if (LLDP_POM_GetSysAdminStatus(&admin_status) == LLDP_TYPE_RETURN_OK)
            {
                if (admin_status == LLDP_TYPE_SYS_ADMIN_ENABLE)
                {
                    PROCESS_MORE_FUNC(" LLDP Enabled                 : Yes\r\n");
                }
                else if (admin_status == LLDP_TYPE_SYS_ADMIN_DISABLE)
                {
                    PROCESS_MORE_FUNC(" LLDP Enabled                 : No\r\n");
                }
            }

            LLDP_POM_GetSysConfigEntry(&sys_config_entry);
            PROCESS_MORE_1(" LLDP Transmit Interval       : %lu seconds\r\n", (unsigned long)sys_config_entry.message_tx_interval);
            PROCESS_MORE_1(" LLDP Hold Time Multiplier    : %lu\r\n", (unsigned long)sys_config_entry.message_tx_hold_multiplier);
            PROCESS_MORE_1(" LLDP Delay Interval          : %lu seconds\r\n", (unsigned long)sys_config_entry.tx_delay);
            PROCESS_MORE_1(" LLDP Re-initialization Delay : %lu seconds\r\n", (unsigned long)sys_config_entry.reinit_delay);
            PROCESS_MORE_1(" LLDP Notification Interval   : %lu seconds\r\n", (unsigned long)sys_config_entry.notification_interval);
#if (SYS_CPNT_LLDP_MED == TRUE)
            if(TRUE == LLDP_POM_GetXMedConfigEntry(&xmed_config_entry))
            {
                CLI_LIB_PrintStr_N(" LLDP MED Fast Start Count    : %d",xmed_config_entry.lldp_xmed_fast_start_repeat_count);
                PROCESS_MORE_FUNC("\r\n");
            }
#endif

            PROCESS_MORE("\r\n");
            PROCESS_MORE("LLDP Port Configuration\r\n");
            PROCESS_MORE(" Port     Admin Status Notification Enabled\r\n");
            PROCESS_MORE(" -------- ------------ --------------------\r\n");

            while(LLDP_PMGR_GetNextPortConfigEntry(&port_config_entry) == LLDP_TYPE_RETURN_OK)
            {
                SWCTRL_Lport_Type_T port_type = SWCTRL_POM_LogicalPortToUserPort(port_config_entry.port_num, &unit, &port, &trunk_id);

                if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                {
                    CLI_LIB_PrintStr_N(" Eth %lu/%-2lu ", (unsigned long)unit, (unsigned long)port);
                }
                else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    CLI_LIB_PrintStr_N(" Trunk %-2lu ", (unsigned long)trunk_id);
                }

                if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_txOnly)
                {
                    CLI_LIB_PrintStr_N("Tx           ");
                }
                else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_rxOnly)
                {
                    CLI_LIB_PrintStr_N("Rx           ");
                }
                else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_txAndRx)
                {
                    CLI_LIB_PrintStr_N("Tx-Rx        ");
                }
                else if (port_config_entry.admin_status == VAL_lldpPortConfigAdminStatus_disabled)
                {
                    CLI_LIB_PrintStr_N("Disabled     ");
                }

                if (port_config_entry.notification_enable == VAL_lldpPortConfigNotificationEnable_false)
                {
                    CLI_LIB_PrintStr_N("False");
                }
                else
                {
                    CLI_LIB_PrintStr_N("True");
                }

                PROCESS_MORE("\r\n");
            }

            PROCESS_MORE("\r\n");
            break;
        }
    }
    return CLI_NO_ERROR;
}

static UI32_T show_one_lldp_local(UI32_T lport, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI32_T  unit_id;
    UI32_T  port_id;
    UI32_T  trunk_id;
    BOOL_T  is_trunk;

    LLDP_MGR_LocalPortData_T    port_entry;

    switch (SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id))
    {
        case SWCTRL_LPORT_NORMAL_PORT:
        case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            is_trunk = FALSE;
            break;
        case SWCTRL_LPORT_TRUNK_PORT:
            is_trunk = TRUE;
            break;
        default:
            return JUMP_OUT_MORE;
    }

    port_entry.port_num = lport;

    if (LLDP_PMGR_GetLocalPortData(&port_entry) == LLDP_TYPE_RETURN_OK)
    {
        PROCESS_MORE_FUNC("LLDP Local Port Information Detail\r\n");

        if (is_trunk)
        {
            CLI_LIB_PrintStr_N(" Port             : Trunk %lu", (unsigned long)trunk_id);
        }
        else
        {
            CLI_LIB_PrintStr_N(" Port             : Eth %lu/%lu", (unsigned long)unit_id, (unsigned long)port_id);
        }
        PROCESS_MORE_FUNC("\r\n");

        /* port type */
        if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Interface alias\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_PORT)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Port\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : MAC Address\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_NETWORK_ADDR)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Network Address\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Interface name\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Agent Circuit ID\r\n");
        }
        else if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Local\r\n");
        }
        else
        {
            PROCESS_MORE_FUNC(" Port ID Type     : Reserved\r\n");
        }

        /* port ID */
        CLI_LIB_PrintStr_N(" Port ID          : %02X-%02X-%02X-", port_entry.loc_port_id[0], port_entry.loc_port_id[1], port_entry.loc_port_id[2]);
        CLI_LIB_PrintStr_N("%02X-%02X-%02X", port_entry.loc_port_id[3], port_entry.loc_port_id[4], port_entry.loc_port_id[5]);
        PROCESS_MORE_FUNC("\r\n");

        /* port desc */
        CLI_LIB_PrintStr_N(" Port Description : %s", port_entry.loc_port_desc);
        PROCESS_MORE_FUNC("\r\n");

#if (SYS_CPNT_LLDP_MED == TRUE)
        /* med supported capabilities */
        {
            LLDP_MGR_XMedPortConfigEntry_T med_port_config_entry;

            char caption_str[] = " MED Capability   : ";
            char padding_str[] = "                    ";
            int count = 0;

            med_port_config_entry.lport = lport;

            if (LLDP_PMGR_GetXMedPortConfigEntry(&med_port_config_entry))
            {
                UI16_T lldp_xmed_port_cap_supported = med_port_config_entry.lldp_xmed_port_cap_supported;

                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_CAP_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("LLDP-MED Capabilities\r\n");
                }
                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_NETWORK_POLICY_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Network Policy\r\n");
                }
                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_LOCATION_IDENT_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Location Identification\r\n");
                }
                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_EXT_PSE_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PSE\r\n");
                }
                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_EXT_PD_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PD\r\n");
                }
                if (lldp_xmed_port_cap_supported & LLDP_TYPE_MED_INVENTORY_TX)
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Inventory\r\n");
                }
            }
        }
#endif
    }
    return line_num;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_LldpLocal
 *-------------------------------------------------------------------------
 * PURPOSE  : Show the information of LLDP local devices (global and interface)
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_LldpLocal(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T unit, i, trunk_id;
    UI32_T err_idx;
    UI32_T lport;
    CLI_API_EthStatus_T verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char *s;
    char delemiters[2] = {0};

    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W5_SHOW_LLDP_INFO_LOCALDEVICE_DETAIL:
        {
            if (arg[0][0] == 'e' || arg[0][0] == 'E')
            {
                s = arg[1];

                /*get the unit*/
                unit = atoi(s);

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

                        for(i=lower_val; i<=upper_val; i++)
                        {
                            verify_ret_e = verify_ethernet(unit, i, &lport);

                            if ((verify_ret_e == CLI_API_ETH_NOT_PRESENT) || (verify_ret_e == CLI_API_ETH_UNKNOWN_PORT))
                            {
                                display_ethernet_msg(verify_ret_e, unit, i);
                                continue;
                            }

                            if ((line_num = show_one_lldp_local(lport, line_num)) ==  JUMP_OUT_MORE)
                            {
                                return CLI_NO_ERROR;
                            }
                            else if (line_num == EXIT_SESSION_MORE)
                            {
                                return CLI_EXIT_SESSION;
                            }
                            else
                            {
                                PROCESS_MORE("\r\n");
                            }
                        }
                    }
                    if(s == 0)
                        break;
                    else
                        memset(Token, 0, sizeof(Token));
                }
            }
            else if (arg[0][0] == 'p' || arg[0][0] == 'P')
            {
                trunk_id = atoi(arg[1]);
                if ((verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, trunk_id);
                    return CLI_NO_ERROR;
                }

                if((line_num = show_one_lldp_local(lport, line_num)) ==  JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
                else
                {
                    PROCESS_MORE("\r\n");
                }
            }
            break;
        }
        case PRIVILEGE_EXEC_CMD_W4_SHOW_LLDP_INFO_LOCALDEVICE:
        {
            LLDP_MGR_LocalSystemData_T  system_entry;

            PROCESS_MORE("LLDP Local Global Information\r\n");
            memset(&system_entry, 0, sizeof(system_entry));
            if (LLDP_PMGR_GetLocalSystemData(&system_entry) == LLDP_TYPE_RETURN_OK)
            {
                /* Chassis type */
                if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_CHASSIS)
                {
                    PROCESS_MORE(" Chassis Type                : Chassis Component\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFALIAS)
                {
                    PROCESS_MORE(" Chassis Type                : Interface Alias\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_PORT)
                {
                    PROCESS_MORE(" Chassis Type                : Port Component\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_MAC_ADDR)
                {
                    PROCESS_MORE(" Chassis Type                : MAC Address\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_NETWORK_ADDR)
                {
                    PROCESS_MORE(" Chassis Type                : Address\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFNAME)
                {
                    PROCESS_MORE(" Chassis Type                : Interface name\r\n");
                }
                else if(system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_LOCAL)
                {
                    PROCESS_MORE(" Chassis Type                : Locally Assigned\r\n");
                }
                else
                {
                    PROCESS_MORE(" Chassis Type                : Reserved\r\n");
                }

                /* Chassis ID */
                CLI_LIB_PrintStr_N(" Chassis ID                  : ");
                if (system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFALIAS ||
                    system_entry.loc_chassis_id_subtype == LLDP_TYPE_CHASSIS_ID_SUBTYPE_IFNAME)
                {
                    CLI_LIB_PrintStr_N("%.*s", (int)system_entry.loc_chassis_id_len, system_entry.loc_chassis_id);
                }
                else
                {
                    CLI_API_Lldp_PrintHex(system_entry.loc_chassis_id, system_entry.loc_chassis_id_len);
                }
                PROCESS_MORE("\r\n");

                /* system name */
                CLI_LIB_PrintStr_N(" System Name                 : ");
                if(strlen((char *)system_entry.loc_sys_name) != 0)
                {
                   PROCESS_MORE_1("%s\r\n", system_entry.loc_sys_name);
                }
                else
                {
                   PROCESS_MORE("\r\n");
                }

                /* system descr */
                {
                    int tmp_len;

                    tmp_len = system_entry.loc_sys_desc_len;

                    if(tmp_len <= 54)
                    {
                        CLI_LIB_PrintStr_N(" System Description          : %s", system_entry.loc_sys_desc);
                    }
                    else
                    {
                        int i;

                        CLI_LIB_PrintStr_N(" System Description          : ");

                        for(i = 0; i < tmp_len; i++)
                        {
                            if( i != 0 && (i % 47) == 0)
                            {
                                PROCESS_MORE("\r\n");
                                CLI_LIB_PrintStr_N("                               ");
                            }
                            CLI_LIB_PutChar(system_entry.loc_sys_desc[i]);
                        }
                    }
                    PROCESS_MORE("\r\n");
                }

                /* system support */
                {
                    int tmp_len;

                    CLI_LIB_PrintStr_N(" System Capabilities Support : ");
                    tmp_len = 31;

                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_OTHER)!=0)
                    {
                        strcat(buff, "Other, ");
                        tmp_len += 7;
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_REPEATER)!=0)
                    {
                        strcat(buff, "Repeater, ");
                        tmp_len += 10;
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_BRIDGE)!=0)
                    {
                        strcat(buff, "Bridge, ");
                        tmp_len += 8;
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_WLAN_ACCESS_POINT)!=0)
                    {
                        strcat(buff, "WLAN Access Point, ");
                        tmp_len += 19;
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_ROUTER)!=0)
                    {
                        strcat(buff, "Router, ");
                        tmp_len += 8;
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_TELEPHONE)!=0)
                    {

                        strcat(buff, "Telephone, ");
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_DOCSIS_CABLE_DEVICE)!=0)
                    {
                        strcat(buff, "DOCSIS Cable Device, ");
                    }
                    if((system_entry.loc_sys_cap_supported & LLDP_TYPE_LOC_SYS_CAP_SUP_STATION_ONLY)!=0)
                    {
                        strcat(buff, "Station Only, ");
                    }

                    sprintf(&buff[strlen(buff) - 2],"\r\n");
                    PROCESS_MORE(buff);
                    memset(buff, 0, sizeof(buff));
                }

                /* system capabilities enable */
                {
                    CLI_LIB_PrintStr_N(" System Capabilities Enabled : ");
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_OTHER)!=0)
                    {
                        strcat (buff,"Other, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_REPEATER)!=0)
                    {
                        strcat (buff,"Repeater, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_BRIDGE)!=0)
                    {
                        strcat (buff,"Bridge, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_WLAN_ACCESS_POINT)!=0)
                    {
                        strcat (buff,"WLAN Access Point, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_ROUTER)!=0)
                    {
                        strcat (buff,"Router, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_TELEPHONE)!=0)
                    {
                        strcat (buff,"Telephone, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_DOCSIS_CABLE_DEVICE)!=0)
                    {
                        strcat (buff,"DOCSIS Cable Device, ");
                    }
                    if((system_entry.loc_sys_cap_enabled & LLDP_TYPE_LOC_SYS_CAP_ENA_STATION_ONLY)!=0)
                    {
                        strcat (buff,"Station Only, ");
                    }

                    sprintf (&buff[strlen(buff) - 2],"\r\n");
                    PROCESS_MORE(buff);
                    memset(buff, 0, sizeof(buff));
                }

                /* management address */
                {
                    LLDP_MGR_LocalManagementAddrEntry_T man_addr_entry;

                    if (LLDP_PMGR_GetLocalManagementAddress(&man_addr_entry) == LLDP_TYPE_RETURN_OK)
                    {
                        if(man_addr_entry.loc_man_addr_subtype==1)
                        {
                            CLI_LIB_PrintStr_N(" Management Address          : ");
                            CLI_LIB_PrintStr_N("%d.%d.%d.%d (IPv4)",
                                man_addr_entry.loc_man_addr[0],man_addr_entry.loc_man_addr[1],
                                man_addr_entry.loc_man_addr[2],man_addr_entry.loc_man_addr[3]);
                            PROCESS_MORE("\r\n");
                        }
                        else if(man_addr_entry.loc_man_addr_subtype==2)
                        {
                            CLI_LIB_PrintStr_N("  Management Address          : ");
                            CLI_LIB_PrintStr_N("%02X-%02X-%02X-", man_addr_entry.loc_man_addr[0],man_addr_entry.loc_man_addr[1],man_addr_entry.loc_man_addr[2]);
                            CLI_LIB_PrintStr_N("%02X-%02X-%02X (IPv6)", man_addr_entry.loc_man_addr[3],man_addr_entry.loc_man_addr[4],man_addr_entry.loc_man_addr[5]);
                            PROCESS_MORE("\r\n");
                        }
                        else
                        {
                            CLI_LIB_PrintStr_N("  Management Address          : %02X-%02X-%02X-", man_addr_entry.loc_man_addr[0],man_addr_entry.loc_man_addr[1],man_addr_entry.loc_man_addr[2]);
                            CLI_LIB_PrintStr_N("%02X-%02X-%02X (MAC Address)", man_addr_entry.loc_man_addr[3],man_addr_entry.loc_man_addr[4],man_addr_entry.loc_man_addr[5]);
                            PROCESS_MORE("\r\n");
                        }
                    }
                }

                /* LLDP port information */
                {
                    LLDP_MGR_LocalPortData_T    port_entry;
                    UI32_T port;

                    port_entry.port_num = 0;

                    PROCESS_MORE("\r\n");
                    PROCESS_MORE("LLDP Local Port Information\r\n");
                    PROCESS_MORE(" Port     Port ID Type     Port ID           Port Description\r\n");
                    PROCESS_MORE(" -------- ---------------- ----------------- ---------------------------------\r\n");

                    while (LLDP_PMGR_GetNextLocalPortData(&port_entry) == LLDP_TYPE_RETURN_OK)
                    {
                        SWCTRL_Lport_Type_T port_type = SWCTRL_POM_LogicalPortToUserPort(port_entry.port_num, &unit, &port, &trunk_id);

                        /* interface */
                        if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                        {
                            CLI_LIB_PrintStr_N(" Eth %lu/%-2lu ", (unsigned long)unit, (unsigned long)port);
                        }
                        else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                        {
                            CLI_LIB_PrintStr_N(" Trunk %-2lu ", (unsigned long)trunk_id);
                        }

                        /* port type */
                        if(port_entry.loc_port_id_subtype == 1)
                        {
                            CLI_LIB_PrintStr_N("Interface Alias  ");
                        }
                        else if(port_entry.loc_port_id_subtype == 2)
                        {
                            CLI_LIB_PrintStr_N("Port Component   ");
                        }
                        else if(port_entry.loc_port_id_subtype == 3)
                        {
                            CLI_LIB_PrintStr_N("MAC Address      ");
                        }
                        else if(port_entry.loc_port_id_subtype == 4)
                        {
                            CLI_LIB_PrintStr_N("Network Address  ");
                        }
                        else if(port_entry.loc_port_id_subtype == 5)
                        {
                            CLI_LIB_PrintStr_N("Interface Name   ");
                        }
                        else if(port_entry.loc_port_id_subtype == 6)
                        {
                            CLI_LIB_PrintStr_N("Agent Circuit ID ");
                        }
                        else if(port_entry.loc_port_id_subtype == 7)
                        {
                            CLI_LIB_PrintStr_N("Locally Assigned ");
                        }
                        else
                        {
                            CLI_LIB_PrintStr_N("Reserved         ");
                        }

                        /* port id */
                        if (port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS ||
                            port_entry.loc_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME)
                        {
                            CLI_LIB_PrintStr_N("%-17.*s ", (int)port_entry.loc_port_id_len, port_entry.loc_port_id);
                        }
                        else
                        {
                            CLI_API_Lldp_PrintHex(port_entry.loc_port_id, port_entry.loc_port_id_len);
                            if (port_entry.loc_port_id_len <= 6)
                                CLI_LIB_PrintStr_N("%*s ", (int)(18 - port_entry.loc_port_id_len * 3), "");
                        }

                        /* port descr */
                        CLI_LIB_PrintStr_N("%-33s", port_entry.loc_port_desc);

                        PROCESS_MORE("\r\n");
                    }
                }

                PROCESS_MORE("\r\n");
            }
            break;
        }
    }

    return CLI_NO_ERROR;
}

static UI32_T show_one_lldp_remote(UI32_T lport, UI32_T line_num)
{
    UI32_T                      unit_id, port_id, trunk_id;
    /* left_width means max width of field name at left (excluding spaces before
     * and after), right_width means max width of content at right (excluding
     * spaces before)
     */
    int                         left_width, right_width, tmp_len, i, j;
    LLDP_MGR_RemoteSystemData_T system_entry;
    BOOL_T                      is_trunk;
    char                        buff[CLI_DEF_MAX_BUFSIZE] = {0};
    BOOL_T                      first_line;

    switch (SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id))
    {
    case SWCTRL_LPORT_NORMAL_PORT:
    case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
        is_trunk = FALSE;
        break;
    case SWCTRL_LPORT_TRUNK_PORT:
        is_trunk = TRUE;
        break;
    default:
        return JUMP_OUT_MORE;
    }

    system_entry.rem_time_mark = 0;
    system_entry.rem_local_port_num = lport;
    system_entry.rem_index = 0;

    PROCESS_MORE_FUNC("LLDP Remote Devices Information Detail\r\n");

    while (LLDP_PMGR_GetNextRemoteSystemDataByPort(&system_entry) == LLDP_TYPE_RETURN_OK)
    {
        PROCESS_MORE_FUNC("------------------------------------------------------------------------------\r\n");

        left_width = strlen("Enabled Capabilities");
        right_width = 78 - 1 - left_width - 3;

        /* Index */
        CLI_LIB_PrintStr_N(" %-*s : %lu", left_width, "Index", (unsigned long)system_entry.rem_index);
        PROCESS_MORE_FUNC("\r\n");

        /* Chassis ID Type */
        {
            char *ch_type_str[] = {
                    [0] = "Reserved",
                    [1] = "Chassis Component",
                    [2] = "Interface Alias",
                    [3] = "Port Component",
                    [4] = "MAC Address",
                    [5] = "Network Address",
                    [6] = "Interface Name",
                    [7] = "Locally Assigned"};

            char *ch_type_str_p = ch_type_str[0];

            if (system_entry.rem_chassis_id_subtype <= 7)
                ch_type_str_p = ch_type_str[system_entry.rem_chassis_id_subtype];

            CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Chassis Type", ch_type_str_p);
            PROCESS_MORE_FUNC("\r\n");
        }

        /* Chassis ID */
        CLI_LIB_PrintStr_N(" %-*s : ", left_width, "Chassis ID");
        if (system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceAlias ||
            system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceName)
        {
            CLI_LIB_PrintStr_N("%.*s", (int)system_entry.rem_chassis_id_len, system_entry.rem_chassis_id);
        }
        else
        {
            CLI_API_Lldp_PrintHex(system_entry.rem_chassis_id, system_entry.rem_chassis_id_len);
        }
        PROCESS_MORE_FUNC("\r\n");

        /* Port ID Type */
        {
            char *port_type_str[] = {
            [0]                                          = "Reserved",
            [LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS]          = "Interface Alias",
            [LLDP_TYPE_PORT_ID_SUBTYPE_PORT]             = "Port Component",
            [LLDP_TYPE_PORT_ID_SUBTYPE_MAC_ADDR]         = "MAC Address",
            [LLDP_TYPE_PORT_ID_SUBTYPE_NETWORK_ADDR]     = "Network Address",
            [LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME]           = "Interface Name",
            [LLDP_TYPE_PORT_ID_SUBTYPE_AGENT_CIRCUIT_ID] = "Agent Circuit ID",
            [LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL]            = "Locally Assigned"};

            char *port_type_str_p = port_type_str[0];

            if (system_entry.rem_port_id_subtype <= LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
                port_type_str_p = port_type_str[system_entry.rem_port_id_subtype];

            CLI_LIB_PrintStr_N(" %-*s : %s", left_width, "Port ID Type", port_type_str_p);
            PROCESS_MORE_FUNC("\r\n");
        }

        /* Port ID */
        CLI_LIB_PrintStr_N(" %-*s : ", left_width, "Port ID");
        if (    (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS)
             || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME)
             || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
           )
        {
            CLI_LIB_PrintStr_N("%.*s", (int)system_entry.rem_port_id_len, system_entry.rem_port_id);
        }
        else
        {
            CLI_API_Lldp_PrintHex(system_entry.rem_port_id, system_entry.rem_port_id_len);
        }
        PROCESS_MORE_FUNC("\r\n");

        /* TTL */
        CLI_LIB_PrintStr_N(" %-*s : %lu seconds", left_width, "Time To Live", (unsigned long)system_entry.rem_ttl);
        PROCESS_MORE_FUNC("\r\n");

        /* Port Description */
        tmp_len = system_entry.rem_port_desc_len;
        if (tmp_len > 0)
        {
            CLI_LIB_PrintStr_N(" %-*s : ", left_width, "Port Description");
            for(i = 0, j = 0; j < tmp_len; i++, j++)
            {
                if ((system_entry.rem_port_desc[j] == '\r') &&
                    (system_entry.rem_port_desc[j+1] == '\n'))
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                    i = 0;
                    j++;
                    continue;
                }
                if(i != 0 && (i % right_width) == 0)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                }
                CLI_LIB_PutChar(system_entry.rem_port_desc[j]);
            }
            PROCESS_MORE_FUNC("\r\n");
        }

        /* System Name */
        tmp_len = system_entry.rem_sys_name_len;
        if (tmp_len > 0)
        {
            CLI_LIB_PrintStr_N(" %-*s : ", left_width, "System Name");
            for(i = 0, j = 0; j < tmp_len; i++, j++)
            {
                if ((system_entry.rem_sys_name[j] == '\r') &&
                    (system_entry.rem_sys_name[j+1] == '\n'))
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                    i = 0;
                    j++;
                    continue;
                }
                if(i != 0 && (i % right_width) == 0)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                }
                CLI_LIB_PutChar(system_entry.rem_sys_name[j]);
            }
            PROCESS_MORE_FUNC("\r\n");
        }

        /* System Description */
        tmp_len = system_entry.rem_sys_desc_len;
        if (tmp_len > 0)
        {
            CLI_LIB_PrintStr_N(" %-*s : ", left_width, "System Description");
            for(i = 0, j = 0; j < tmp_len; i++, j++)
            {
                if ((system_entry.rem_sys_desc[j] == '\r') &&
                    (system_entry.rem_sys_desc[j+1] == '\n'))
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                    i = 0;
                    j++;
                    continue;
                }
                if(i != 0 && (i % right_width) == 0)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %-*s", left_width+3, "");
                }
                CLI_LIB_PutChar(system_entry.rem_sys_desc[j]);
            }
            PROCESS_MORE_FUNC("\r\n");
        }

        {
#define CAP_CHK_MAX_NUM     (sizeof(sup_cap_rec_ar)/sizeof(struct cli_lldp_cap_rec))

            struct cli_lldp_cap_rec{
                char    *cap_str_p;
                UI32_T  cap_bit;
            };

            struct cli_lldp_cap_rec  sup_cap_rec_ar[] = {
                {"Other, ",                 LLDP_TYPE_REM_SYS_CAP_SUP_OTHER},
                {"Repeater, ",              LLDP_TYPE_REM_SYS_CAP_SUP_REPEATER},
                {"Bridge, ",                LLDP_TYPE_REM_SYS_CAP_SUP_BRIDGE},
                {"WLAN Access Point, ",     LLDP_TYPE_REM_SYS_CAP_SUP_WLAN_ACCESS_POINT},
                {"Router, ",                LLDP_TYPE_REM_SYS_CAP_SUP_ROUTER},
                {"Telephone, ",             LLDP_TYPE_REM_SYS_CAP_SUP_TELEPHONE},
                {"DOCSIS Cable Device, ",   LLDP_TYPE_REM_SYS_CAP_SUP_DOCSIS_CABLE_DEVICE},
                {"Station Only, ",          LLDP_TYPE_REM_SYS_CAP_SUP_STATION_ONLY}};

            /* System Capabilities */
            tmp_len = 0;
            first_line = TRUE;
            for (i =0; i < CAP_CHK_MAX_NUM; i++)
            {
                if (system_entry.rem_sys_cap_supported & sup_cap_rec_ar[i].cap_bit)
                {
                    if (first_line == TRUE)
                    {
                        CLI_LIB_PrintStr_N(" %-*s : ", left_width,
                            "System Capabilities");
                        first_line = FALSE;
                    }

                    if ((tmp_len + strlen(sup_cap_rec_ar[i].cap_str_p)) > right_width)
                    {
                        CLI_LIB_PrintStr_N("%.*s", tmp_len, buff);
                        PROCESS_MORE_FUNC("\r\n");
                        tmp_len = sprintf(buff, "%*s", left_width+4, "");
                    }
                    tmp_len += sprintf(&buff[tmp_len], "%s", sup_cap_rec_ar[i].cap_str_p);
                }
            }

            if ((tmp_len > 2) && (buff[tmp_len-2] == ','))
            {
                buff[tmp_len-2] = '\0';
                CLI_LIB_PrintStr_N("%s", buff);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Enabled Capabilities */
            tmp_len = 0;
            first_line = TRUE;
            for (i =0; i < CAP_CHK_MAX_NUM; i++)
            {
                if (system_entry.rem_sys_cap_enabled & sup_cap_rec_ar[i].cap_bit)
                {
                    if (first_line == TRUE)
                    {
                        CLI_LIB_PrintStr_N(" %-*s : ", left_width,
                            "Enabled Capabilities");
                        first_line = FALSE;
                    }

                    if ((tmp_len + strlen(sup_cap_rec_ar[i].cap_str_p)) > right_width)
                    {
                        CLI_LIB_PrintStr_N("%.*s", tmp_len, buff);
                        PROCESS_MORE_FUNC("\r\n");
                        tmp_len = sprintf(buff, "%*s", left_width+4, "");
                    }
                    tmp_len += sprintf(&buff[tmp_len], "%s", sup_cap_rec_ar[i].cap_str_p);
                }
            }

            if ((tmp_len > 2) && (buff[tmp_len-2] == ','))
            {
                buff[tmp_len-2] = '\0';
                CLI_LIB_PrintStr_N("%s", buff);
                PROCESS_MORE_FUNC("\r\n");
            }
        }

        /* Management Address */
        {
            LLDP_MGR_RemoteManagementAddrEntry_T    man_addr_entry;

            memset(&man_addr_entry, 0, sizeof(LLDP_MGR_RemoteManagementAddrEntry_T));
            man_addr_entry.rem_local_port_num = system_entry.rem_local_port_num;
            man_addr_entry.rem_index = system_entry.rem_index;

            first_line = TRUE;
            while (LLDP_POM_GetNextRemManAddrByIndex(&man_addr_entry) == LLDP_TYPE_RETURN_OK)
            {
                if (first_line == TRUE)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %s : ", "Management Address");
                    first_line = FALSE;
                }
                else
                {
                    CLI_LIB_PrintStr_N(" %*s", (int)strlen("Management Address")+3, "");
                }

                if (man_addr_entry.rem_man_addr_subtype == 1)
                {

                    CLI_LIB_PrintStr_N("%hu.%hu.%hu.%hu (IPv4)",
                        man_addr_entry.rem_man_addr[0],man_addr_entry.rem_man_addr[1],
                        man_addr_entry.rem_man_addr[2],man_addr_entry.rem_man_addr[3]);
                    PROCESS_MORE_FUNC("\r\n");
                }
                else if (man_addr_entry.rem_man_addr_subtype == 2)
                {
                    char    addr_buf[46] = {0};

                    L_INET_Ntop(L_INET_AF_INET6,
                        man_addr_entry.rem_man_addr, addr_buf, sizeof(addr_buf));
                    CLI_LIB_PrintStr_N("%s (IPv6)", addr_buf);
                    PROCESS_MORE_FUNC("\r\n");
                }
                else
                {
                    CLI_LIB_PrintStr_N("%02X-%02X-%02X-%02X-%02X-%02X (MAC Address)",
                        man_addr_entry.rem_man_addr[0], man_addr_entry.rem_man_addr[1],
                        man_addr_entry.rem_man_addr[2], man_addr_entry.rem_man_addr[3],
                        man_addr_entry.rem_man_addr[4], man_addr_entry.rem_man_addr[5]);
                    PROCESS_MORE_FUNC("\r\n");
                }
            }
        }
#if (SYS_CPNT_LLDP_EXT==TRUE)
        /* 802.1 extensions */
        {
            LLDP_MGR_Xdot1RemEntry_T            xdot1_rem_entry;
            LLDP_MGR_Xdot1RemProtoVlanEntry_T   xdot1_rem_proto_vlan_entry;
            LLDP_MGR_Xdot1RemVlanNameEntry_T    xdot1_rem_vlan_name_entry;
            LLDP_MGR_Xdot1RemProtocolEntry_T    xdot1_rem_protocol_entry;
            LLDP_MGR_Xdot1RemCnEntry_T          xdot1_rem_cn_entry;

            memset(&xdot1_rem_entry, 0, sizeof(LLDP_MGR_Xdot1RemEntry_T));
            memset(&xdot1_rem_proto_vlan_entry, 0, sizeof(LLDP_MGR_Xdot1RemProtoVlanEntry_T));
            memset(&xdot1_rem_vlan_name_entry, 0, sizeof(LLDP_MGR_Xdot1RemVlanNameEntry_T));
            memset(&xdot1_rem_protocol_entry, 0, sizeof(LLDP_MGR_Xdot1RemProtocolEntry_T));
            memset(&xdot1_rem_cn_entry, 0, sizeof(LLDP_MGR_Xdot1RemCnEntry_T));

            /* Port VLAN ID */
            xdot1_rem_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot1_rem_entry.rem_index = system_entry.rem_index;
            if (LLDP_POM_GetXdot1RemEntry(&xdot1_rem_entry))
            {

                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N(" %s : %lu", "Port VLAN ID", (unsigned long)xdot1_rem_entry.rem_port_vlan_id);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Port and Protocol VLAN ID */
            xdot1_rem_proto_vlan_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot1_rem_proto_vlan_entry.rem_index = system_entry.rem_index;
            xdot1_rem_proto_vlan_entry.rem_proto_vlan_id = LLDP_TYPE_FIRST_PPVID;
            first_line = TRUE;
            while (LLDP_POM_GetNextXdot1RemProtoVlanEntryByIndex(&xdot1_rem_proto_vlan_entry))
            {
                char    *sup_str_p, *en_str_p;

                if (first_line)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %s : ", "Port and Protocol VLAN ID");
                    first_line = FALSE;
                }
                else
                {
                    CLI_LIB_PrintStr_N(" %*s",
                        (int)strlen("Port and Protocol VLAN ID")+3, "");
                }

                if (xdot1_rem_proto_vlan_entry.rem_proto_vlan_supported == VAL_lldpXdot1RemProtoVlanSupported_true)
                {
                    sup_str_p = "supported";

                    if (xdot1_rem_proto_vlan_entry.rem_proto_vlan_enabled == VAL_lldpXdot1RemProtoVlanEnabled_true)
                        en_str_p = ", enabled";
                    else
                        en_str_p = ", disabled";
                }
                else
                {
                    sup_str_p = "not supported";
                    en_str_p  = "";
                }

                if (xdot1_rem_proto_vlan_entry.rem_proto_vlan_id == 0)
                {
                    CLI_LIB_PrintStr_N("%s%s", sup_str_p, en_str_p);
                }
                else
                {
                    CLI_LIB_PrintStr_N("VLAN %4lu - %s%s",
                        (unsigned long)xdot1_rem_proto_vlan_entry.rem_proto_vlan_id,
                        sup_str_p, en_str_p);
                }
                PROCESS_MORE_FUNC("\r\n");
            }

            /* VLAN Name */
            xdot1_rem_vlan_name_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot1_rem_vlan_name_entry.rem_index = system_entry.rem_index;
            first_line = TRUE;
            while (LLDP_POM_GetNextXdot1RemVlanNameEntryByIndex(&xdot1_rem_vlan_name_entry))
            {
                if (first_line)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %s : ", "VLAN Name");
                    first_line = FALSE;
                }
                else
                {
                    CLI_LIB_PrintStr_N(" %*s",
                        (int)strlen("VLAN Name")+3, "");
                }

                CLI_LIB_PrintStr_N("VLAN %4lu - %s",
                    (unsigned long)xdot1_rem_vlan_name_entry.rem_vlan_id,
                    xdot1_rem_vlan_name_entry.rem_vlan_name);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Protocol Identity */
            xdot1_rem_protocol_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot1_rem_protocol_entry.rem_index = system_entry.rem_index;
            first_line = TRUE;
            while (LLDP_POM_GetNextXdot1RemProtocolEntryByIndex(&xdot1_rem_protocol_entry))
            {
                if (first_line)
                {
                    PROCESS_MORE_FUNC("\r\n");
                    CLI_LIB_PrintStr_N(" %s : ", "Protocol Identity (Hex)");
                    first_line = FALSE;
                }
                else
                {
                    CLI_LIB_PrintStr_N(" %*s",
                        (int)strlen("Protocol Identity (Hex)")+3, "");
                }

                tmp_len = 0;
                for (i = 0; i < xdot1_rem_protocol_entry.rem_protocol_id_len; i++)
                {
                    tmp_len += sprintf(&buff[tmp_len], "%02X-",
                        xdot1_rem_protocol_entry.rem_protocol_id[i]);
                }

                if (tmp_len > 1)
                    buff[tmp_len-1] = '\0';

                CLI_LIB_PrintStr_N("%s", buff);
                PROCESS_MORE_FUNC("\r\n");
            }

#if (SYS_CPNT_CN == TRUE)
            /* Congestion Notification */
            xdot1_rem_cn_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot1_rem_cn_entry.rem_index = system_entry.rem_index;
            if (LLDP_POM_GetXdot1RemCnEntry(&xdot1_rem_cn_entry) == LLDP_TYPE_RETURN_OK)
            {
                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Congestion Notification\r\n");
                CLI_LIB_PrintStr_N("  %s : 0x%02X",
                    "Per-priority CNPV Indicators ",
                    xdot1_rem_cn_entry.rem_cnpv_indicators);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %s : 0x%02X",
                    "Per-priority Ready Indicators",
                    xdot1_rem_cn_entry.rem_ready_indicators);
                PROCESS_MORE_FUNC("\r\n");
            }
#endif /* #if (SYS_CPNT_CN == TRUE) */
        }

        /* 802.3 extensions */
        {
            LLDP_MGR_Xdot3RemPortEntry_T            xdot3_rem_port_entry;
            LLDP_MGR_Xdot3RemPowerEntry_T           xdot3_rem_power_entry;
            LLDP_MGR_Xdot3RemLinkAggEntry_T         xdot3_rem_link_agg_entry;
            LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T    xdot3_rem_max_frame_size_entry;

            memset(&xdot3_rem_port_entry, 0, sizeof(LLDP_MGR_Xdot3RemPortEntry_T));
            memset(&xdot3_rem_power_entry, 0, sizeof(LLDP_MGR_Xdot3RemPowerEntry_T));
            memset(&xdot3_rem_link_agg_entry, 0, sizeof(LLDP_MGR_Xdot3RemLinkAggEntry_T));
            memset(&xdot3_rem_max_frame_size_entry, 0, sizeof(LLDP_MGR_Xdot3RemMaxFrameSizeEntry_T));

            xdot3_rem_port_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot3_rem_port_entry.rem_index = system_entry.rem_index;
            xdot3_rem_power_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot3_rem_power_entry.rem_index = system_entry.rem_index;
            xdot3_rem_link_agg_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot3_rem_link_agg_entry.rem_index = system_entry.rem_index;
            xdot3_rem_max_frame_size_entry.rem_local_port_num = system_entry.rem_local_port_num;
            xdot3_rem_max_frame_size_entry.rem_index = system_entry.rem_index;

            /* MAC/PHY Configuration/Status */
            if (LLDP_POM_GetXdot3RemPortEntry(&xdot3_rem_port_entry))
            {
                char    *sts_str_p;

                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" MAC/PHY Configuration/Status\r\n");

                /* longest of left text in this group, not counting space
                 */
                left_width = strlen("Port Auto-neg Advertised Cap (Hex)");

                if (xdot3_rem_port_entry.rem_port_auto_neg_supported == 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Port Auto-neg Supported", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_port_entry.rem_port_auto_neg_enable == 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Port Auto-neg Enabled", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                CLI_LIB_PrintStr_N("  %-*s : %02X%02X", left_width,
                    "Port Auto-neg Advertised Cap (Hex)",
                    ((UI8_T *)&xdot3_rem_port_entry.rem_port_auto_neg_adv_cap)[0],
                    ((UI8_T *)&xdot3_rem_port_entry.rem_port_auto_neg_adv_cap)[1]
                    );
                PROCESS_MORE_FUNC("\r\n");

                CLI_LIB_PrintStr_N("  %-*s : %u", left_width, "Port MAU Type",
                    xdot3_rem_port_entry.rem_port_oper_mau_type);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Power Via MDI */
            if (LLDP_POM_GetXdot3RemPowerEntry(&xdot3_rem_power_entry))
            {
                char    *sts_str_p;

                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Power via MDI\r\n");

                /* longest of left text in this group, not counting space
                 */
                left_width = strlen("Power Pair Controllable");

                if (xdot3_rem_power_entry.rem_power_port_class == 1)
                    sts_str_p = "PSE";
                else
                    sts_str_p = "PD";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Power Class", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_power_entry.rem_power_mdi_supported == 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Power MDI Supported", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_power_entry.rem_power_mdi_enabled == 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Power MDI Enabled", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_power_entry.rem_power_pair_controlable == 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Power Pair Controllable", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_power_entry.rem_power_pair_controlable == 1)
                    sts_str_p = "Signal";
                else
                    sts_str_p = "Spare";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Power Pairs", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                CLI_LIB_PrintStr_N("  %-*s : Class %u", left_width,
                    "Power Classification", xdot3_rem_power_entry.rem_power_class);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Link Aggregation */
            if (LLDP_POM_GetXdot3RemLinkAggEntry(&xdot3_rem_link_agg_entry))
            {
                char    *sts_str_p;

                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Link Aggregation\r\n");

                /* longest of left text in this group, not counting space
                 */
                left_width = strlen("Link Aggregation Capable");

                if (xdot3_rem_link_agg_entry.rem_link_agg_status & 1)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Link Aggregation Capable", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                if (xdot3_rem_link_agg_entry.rem_link_agg_status & 2)
                    sts_str_p = "Yes";
                else
                    sts_str_p = "No";

                CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                    "Link Aggregation Enable", sts_str_p);
                PROCESS_MORE_FUNC("\r\n");

                CLI_LIB_PrintStr_N("  %-*s : %lu", left_width,
                    "Link Aggregation Port ID", (unsigned long)xdot3_rem_link_agg_entry.rem_link_agg_port_id);
                PROCESS_MORE_FUNC("\r\n");
            }

            /* Max Frame Size */
            if (LLDP_POM_GetXdot3RemMaxFrameSizeEntry(&xdot3_rem_max_frame_size_entry))
            {
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N(" Max Frame Size : %lu",
                    (unsigned long)xdot3_rem_max_frame_size_entry.rem_max_frame_size);
                PROCESS_MORE_FUNC("\r\n");
            }
        }
#endif  /* #if (SYS_CPNT_LLDP_EXT==TRUE) */
#if (SYS_CPNT_LLDP_MED == TRUE)
        /* tia lldp-med */
        {
            LLDP_MGR_XMedRemCapEntry_T rem_cap_entry;
            LLDP_MGR_XMedRemMediaPolicyEntry_T rem_med_policy_entry;
            LLDP_MGR_XMedRemLocationEntry_T rem_location_entry;
            LLDP_MGR_XMedRemPoeEntry_T rem_poe_entry;
            LLDP_MGR_XMedRemInventoryEntry_T rem_inventory_entry;
            UI8_T   app_type;
            char   *app_type_str[LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE] = {\
                                 "Voice", "Voice Signaling", "Guest Voice",\
                                 "Guest Voice Signaling", "Softphone Voice",\
                                 "Video Conferencing", "Stream Video" ,"Video Signaling"};

            memset(&rem_cap_entry, 0, sizeof(rem_cap_entry));
            memset(&rem_med_policy_entry, 0, sizeof(rem_med_policy_entry));
            memset(&rem_location_entry, 0, sizeof(rem_location_entry));
            memset(&rem_poe_entry, 0, sizeof(rem_poe_entry));
            memset(&rem_inventory_entry, 0, sizeof(rem_inventory_entry));

            rem_cap_entry.rem_time_mark = 0;
            rem_cap_entry.rem_local_port_num = system_entry.rem_local_port_num;
            rem_cap_entry.rem_index = system_entry.rem_index;
            rem_med_policy_entry.rem_time_mark = 0;
            rem_med_policy_entry.rem_local_port_num = system_entry.rem_local_port_num;
            rem_med_policy_entry.rem_index = system_entry.rem_index;
            rem_location_entry.rem_time_mark = 0;
            rem_location_entry.rem_local_port_num = system_entry.rem_local_port_num;
            rem_location_entry.rem_index = system_entry.rem_index;
            rem_poe_entry.rem_time_mark = 0;
            rem_poe_entry.rem_local_port_num = system_entry.rem_local_port_num;
            rem_poe_entry.rem_index = system_entry.rem_index;
            rem_inventory_entry.rem_time_mark = 0;
            rem_inventory_entry.rem_local_port_num = system_entry.rem_local_port_num;
            rem_inventory_entry.rem_index = system_entry.rem_index;

            /* LLDP-MED Capabilities */
            if (LLDP_POM_GetXMedRemCapEntry(&rem_cap_entry))
            {
                char sup_cap_str[] = "  Supported Capabilities : ";
                char ena_cap_str[] = "  Enabled Capabilities   : ";
                char padding_str[] = "                           ";
                char *caption_str;
                int count;

                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" LLDP-MED Capabilities\r\n");

                /* longest of left text in this group, not counting space
                 */
                left_width = strlen("Supported Capabilities");

                CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Device Class");
                switch (rem_cap_entry.rem_device_class)
                {
                    case VAL_lldpXMedRemDeviceClass_endpointClass1:
                        CLI_LIB_PrintStr_N("Endpoint Class I");
                        break;
                    case VAL_lldpXMedRemDeviceClass_endpointClass2:
                        CLI_LIB_PrintStr_N("Endpoint Class II");
                        break;
                    case VAL_lldpXMedRemDeviceClass_endpointClass3:
                        CLI_LIB_PrintStr_N("Endpoint Class III");
                        break;
                    case VAL_lldpXMedRemDeviceClass_networkConnectivity:
                        CLI_LIB_PrintStr_N("Network Connectivity");
                        break;
                    default:
                        CLI_LIB_PrintStr_N("Type Not Defined");
                        break;
                }
                PROCESS_MORE_FUNC("\r\n");

                caption_str = sup_cap_str;
                count = 0;
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_capabilities))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("LLDP-MED Capabilities\r\n");
                }
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_networkPolicy))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Network Policy\r\n");
                }
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_location))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Location Identification\r\n");
                }
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_extendedPSE))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PSE\r\n");
                }
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_extendedPD))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PD\r\n");
                }
                if (rem_cap_entry.rem_cap_supported & BIT_VALUE(VAL_lldpXMedRemCapSupported_inventory))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Inventory\r\n");
                }

                caption_str = ena_cap_str;
                count = 0;
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_capabilities))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("LLDP-MED Capabilities\r\n");
                }
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_networkPolicy))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Network Policy\r\n");
                }
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_location))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Location Identification\r\n");
                }
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_extendedPSE))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PSE\r\n");
                }
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_extendedPD))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Extended Power via MDI - PD\r\n");
                }
                if (rem_cap_entry.rem_cap_current & BIT_VALUE(VAL_lldpXMedRemCapCurrent_inventory))
                {
                    CLI_LIB_PrintStr_N("%s", count++ ? padding_str : caption_str);
                    PROCESS_MORE_FUNC("Inventory\r\n");
                }
            }

            /* Network Policy */
            left_width = strlen("Unknown Policy Flag");
            for (app_type = VAL_lldpXMedRemMediaPolicyAppType_voice;
                 app_type <= LLDP_TYPE_MED_MAX_NETWORK_POLITY_TYPE;
                 app_type++)
            {
                rem_med_policy_entry.rem_app_type = app_type;
                if (LLDP_PMGR_GetXMedRemMediaPolicyEntryByIndex(&rem_med_policy_entry))
                {
                    PROCESS_MORE_FUNC("\r\n");
                    PROCESS_MORE_FUNC(" Network Policy\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : %s", left_width,
                        "Application Type", app_type_str[app_type-1]);
                    PROCESS_MORE_FUNC("\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Unknown Policy Flag");
                    if (rem_med_policy_entry.rem_unknown == VAL_lldpXMedRemMediaPolicyUnknown_true)
                    {
                        PROCESS_MORE_FUNC("Enabled\r\n");
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("Disabled\r\n");
                    }

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Tagged Flag");
                    if (rem_med_policy_entry.rem_tagged == VAL_lldpXMedRemMediaPolicyTagged_true)
                    {
                        PROCESS_MORE_FUNC("Enabled\r\n");
                    }
                    else
                    {
                        PROCESS_MORE_FUNC("Disabled\r\n");
                    }

                    if (rem_med_policy_entry.rem_unknown != VAL_lldpXMedRemMediaPolicyUnknown_true)
                    {
                        if (rem_med_policy_entry.rem_tagged == VAL_lldpXMedRemMediaPolicyTagged_true)
                        {
                            CLI_LIB_PrintStr_N("  %-*s : %u", left_width,
                                "VLAN ID", rem_med_policy_entry.rem_vid);
                            PROCESS_MORE_FUNC("\r\n");

                            CLI_LIB_PrintStr_N("  %-*s : %lu", left_width,
                                "Layer 2 Priority", (unsigned long)rem_med_policy_entry.rem_priority);
                            PROCESS_MORE_FUNC("\r\n");
                        }

                        CLI_LIB_PrintStr_N("  %-*s : %lu", left_width,
                            "DSCP Value", (unsigned long)rem_med_policy_entry.rem_dscp);
                        PROCESS_MORE_FUNC("\r\n");
                    }
                }
            }

            /* Location Identification */
            left_width = strlen("Location Data Format");
            while (LLDP_PMGR_GetNextXMedRemLocationEntryByIndex(&rem_location_entry))
            {
                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Location Identification\r\n");

                if (rem_location_entry.rem_location_subtype == VAL_lldpXMedRemLocationSubtype_coordinateBased)
                {
#if 0 // wakka: not implelemt
                        PROCESS_MORE_FUNC("  Location Data Format : Coordinate-based LCI\r\n");
                        PROCESS_MORE_FUNC("  Latitude             : 24.46 degrees\r\n");
                        PROCESS_MORE_FUNC("  Longitude            : 120.59 degrees\r\n");
                        PROCESS_MORE_FUNC("  Altitude             : 15 meters\r\n");
#endif
                }
                else if (rem_location_entry.rem_location_subtype == VAL_lldpXMedRemLocationSubtype_civicAddress)
                {
                        LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;
                        UI8_T country_code[2], what;
                        BOOL_T ret;

                        CLI_LIB_PrintStr_N("  %-*s : Civic Address LCI",
                            left_width, "Location Data Format")
                        PROCESS_MORE_FUNC("\r\n");

                        if (LLDP_PMGR_GetXMedRemLocationCivicAddrCountryCode(rem_location_entry.rem_local_port_num, rem_location_entry.rem_index, country_code))
                        {
                            CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width,
                                "Country Name", 2, country_code);
                            PROCESS_MORE_FUNC("\r\n");
                        }

                        if (LLDP_PMGR_GetXMedRemLocationCivicAddrWhat(rem_location_entry.rem_local_port_num, rem_location_entry.rem_index, &what))
                        {
                            CLI_LIB_PrintStr_N("  %-*s : %u", left_width, "What", what);
                            PROCESS_MORE_FUNC("\r\n");
                        }

                        {
                            char ca_value[sizeof(ca_entry.ca_value) + 1] = {0};

                            for (ret = LLDP_PMGR_Get1stXMedRemLocationCivicAddrCaEntry(rem_location_entry.rem_local_port_num, rem_location_entry.rem_index, &ca_entry);
                                 ret;
                                 ret = LLDP_PMGR_GetNextXMedRemLocationCivicAddrCaEntry(rem_location_entry.rem_local_port_num, rem_location_entry.rem_index, &ca_entry))
                            {
                                replace_nonprintable_char_with_period(ca_entry.ca_value, ca_entry.ca_length, ca_value, sizeof(ca_value));

                                CLI_LIB_PrintStr_N("  CA Type %-*u : %.*s",
                                    left_width-(int)strlen("CA Type "), ca_entry.ca_type,
                                    (int)ca_entry.ca_length, ca_value);
                                PROCESS_MORE_FUNC("\r\n");
                            }
                        }
                }
                else if (rem_location_entry.rem_location_subtype == VAL_lldpXMedRemLocationSubtype_elin)
                {
                        CLI_LIB_PrintStr_N("  %-*s : ECS ELIN",
                            left_width, "Location Data Format");
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : %*s", left_width, "ELIN",
                            (int)rem_location_entry.rem_location_info_len,
                            rem_location_entry.rem_location_info);
                        PROCESS_MORE_FUNC("\r\n");
                }
                else
                {
                        CLI_LIB_PrintStr_N("  %-*s : Unknown",
                            left_width, "Location Data Format");
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Location ID",
                            (int)rem_location_entry.rem_location_info_len,
                            rem_location_entry.rem_location_info);
                        PROCESS_MORE_FUNC("\r\n");
                }
            }

            /* Extended Power-Via-MDI */
            if (LLDP_POM_GetXMedRemPoeEntry(&rem_poe_entry))
            {
                LLDP_MGR_XMedRemPoePseEntry_T rem_pse_entry;
                LLDP_MGR_XMedRemPoePdEntry_T rem_pd_entry;

                rem_pse_entry.rem_time_mark = rem_pd_entry.rem_time_mark = 0;
                rem_pse_entry.rem_local_port_num = rem_pd_entry.rem_local_port_num = rem_poe_entry.rem_local_port_num;
                rem_pse_entry.rem_index = rem_pd_entry.rem_index = rem_poe_entry.rem_index;

                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Extended Power-Via-MDI\r\n");

                left_width = strlen("Power Priority");
                if (rem_poe_entry.rem_poe_device_type == VAL_lldpXMedRemXPoEDeviceType_pseDevice &&
                    LLDP_POM_GetXMedRemPoePseEntry(&rem_pse_entry))
                {
                    CLI_LIB_PrintStr_N("  %-*s : PSE Device", left_width, "Power Type");
                    PROCESS_MORE_FUNC("\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Power Source");
                    switch (rem_pse_entry.rem_pse_power_source)
                    {
                        case VAL_lldpXMedRemXPoEPSEPowerSource_primary:
                            PROCESS_MORE_FUNC("Primary Power Source\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPSEPowerSource_backup:
                            PROCESS_MORE_FUNC("Backup Power Source\r\n");
                            break;
                        default:
                            PROCESS_MORE_FUNC("Unknown\r\n");
                            break;
                    }

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Power Priority");
                    switch (rem_pse_entry.rem_pse_power_priority)
                    {
                        case VAL_lldpXMedRemXPoEPSEPowerPriority_critical:
                            PROCESS_MORE_FUNC("Critical\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPSEPowerPriority_high:
                            PROCESS_MORE_FUNC("High\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPSEPowerPriority_low:
                            PROCESS_MORE_FUNC("Low\r\n");
                            break;
                        default:
                            PROCESS_MORE_FUNC("Unknown\r\n");
                            break;
                    }

                    CLI_LIB_PrintStr_N("  %-*s : %lu.%lu Watts", left_width,
                        "Power Value", (unsigned long)rem_pse_entry.rem_pse_power_av/10,
                        (unsigned long)rem_pse_entry.rem_pse_power_av%10);
                    PROCESS_MORE_FUNC("\r\n");
                }
                else if (rem_poe_entry.rem_poe_device_type == VAL_lldpXMedRemXPoEDeviceType_pdDevice &&
                    LLDP_POM_GetXMedRemPoePdEntry(&rem_pd_entry))
                {
                    CLI_LIB_PrintStr_N("  %-*s : PD Device", left_width, "Power Type");
                    PROCESS_MORE_FUNC("\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Power Source");
                    switch (rem_pd_entry.rem_pd_power_source)
                    {
                        case VAL_lldpXMedRemXPoEPDPowerSource_fromPSE:
                            PROCESS_MORE_FUNC("PSE\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPDPowerSource_local:
                            PROCESS_MORE_FUNC("Local\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPDPowerSource_localAndPSE:
                            PROCESS_MORE_FUNC("PSE and Local\r\n");
                            break;
                        default:
                            PROCESS_MORE_FUNC("Unknown\r\n");
                            break;
                    }

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width, "Power Priority");
                    switch (rem_pd_entry.rem_pd_power_priority)
                    {
                        case VAL_lldpXMedRemXPoEPDPowerPriority_critical:
                            PROCESS_MORE_FUNC("Critical\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPDPowerPriority_high:
                            PROCESS_MORE_FUNC("High\r\n");
                            break;
                        case VAL_lldpXMedRemXPoEPDPowerPriority_low:
                            PROCESS_MORE_FUNC("Low\r\n");
                            break;
                        default:
                            PROCESS_MORE_FUNC("Unknown\r\n");
                            break;
                    }

                    CLI_LIB_PrintStr_N("  %-*s : %lu.%lu Watts", left_width,
                        "Power Value", (unsigned long)rem_pd_entry.rem_pd_power_req/10,
                        (unsigned long)rem_pd_entry.rem_pd_power_req%10);
                    PROCESS_MORE_FUNC("\r\n");
                }
            }

            /* inventory */
            if (LLDP_POM_GetXMedRemInventoryEntry(&rem_inventory_entry))
            {
                PROCESS_MORE_FUNC("\r\n");
                PROCESS_MORE_FUNC(" Inventory\r\n");

                left_width = strlen("Hardware Revision");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Hardware Revision", (int)rem_inventory_entry.rem_hardware_rev_len, rem_inventory_entry.rem_hardware_rev);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Firmware Revision", (int)rem_inventory_entry.rem_firmware_rev_len, rem_inventory_entry.rem_firmware_rev);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Software Revision", (int)rem_inventory_entry.rem_software_rev_len, rem_inventory_entry.rem_software_rev);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Serial Number", (int)rem_inventory_entry.rem_serial_num_len, rem_inventory_entry.rem_serial_num);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Manufacture Name", (int)rem_inventory_entry.rem_mfg_name_len, rem_inventory_entry.rem_mfg_name);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Model Name", (int)rem_inventory_entry.rem_model_name_len, rem_inventory_entry.rem_model_name);
                PROCESS_MORE_FUNC("\r\n");
                CLI_LIB_PrintStr_N("  %-*s : %.*s", left_width, "Asset ID", (int)rem_inventory_entry.rem_asset_id_len, rem_inventory_entry.rem_asset_id);
                PROCESS_MORE_FUNC("\r\n");
            }
        }
#endif /* if (SYS_CPNT_LLDP_MED == TRUE) */

#if(SYS_CPNT_DCBX == TRUE)
        {
            /* application priority */
            {
                LLDP_TYPE_DcbxRemAppPriEntry_T rem_app_pri_entry;

                memset(&rem_app_pri_entry, 0, sizeof(LLDP_TYPE_DcbxRemAppPriEntry_T));

                rem_app_pri_entry.rem_local_port_num = system_entry.rem_local_port_num;
                rem_app_pri_entry.rem_index = system_entry.rem_index;
                rem_app_pri_entry.rem_protocol_id = 0;
                first_line = TRUE;
                while (LLDP_PMGR_GetNextDcbxAppPriorityRemoteData(&rem_app_pri_entry))
                {
                    if (first_line)
                    {
                        PROCESS_MORE_FUNC("\r\n");
                        PROCESS_MORE_FUNC("Application Priority\r\n");
                        PROCESS_MORE_FUNC(" Protocol ID Sel Priority\r\n");
                        PROCESS_MORE_FUNC(" ----------- --- --------\r\n");
                        first_line = FALSE;
                    }

                    CLI_LIB_PrintStr_N(" %11d %3d %8d",
                        rem_app_pri_entry.rem_protocol_id,
                        rem_app_pri_entry.rem_sel,
                        rem_app_pri_entry.rem_priority);
                    PROCESS_MORE_FUNC("\r\n");
                }
            }

            /* ETS */
            {
                LLDP_TYPE_DcbxRemEtsEntry_T rem_ets_entry;
                char    *sts_str_p;
                UI8_T index = 0;

                memset(&rem_ets_entry, 0, sizeof(LLDP_TYPE_DcbxRemEtsEntry_T));
                rem_ets_entry.rem_local_port_num = system_entry.rem_local_port_num;
                rem_ets_entry.rem_index = system_entry.rem_index;
                left_width = strlen("Traffic Class Bandwidth(Hex)");
                if(LLDP_TYPE_RETURN_OK == LLDP_PMGR_GetDcbxEtsRemoteData(&rem_ets_entry))
                {
                    /* ETS configuration */
                    if(rem_ets_entry.rem_config_rcvd)
                    {
                        PROCESS_MORE_FUNC("\r\n");
                        PROCESS_MORE_FUNC("ETS Configuration\r\n");
                        if (rem_ets_entry.rem_config_willing == TRUE)
                            sts_str_p = "True";
                        else
                            sts_str_p = "False";

                        CLI_LIB_PrintStr_N("  %-*s : %s", left_width, "Willing", sts_str_p);
                        PROCESS_MORE_FUNC("\r\n");

                        if (rem_ets_entry.rem_config_cbs == TRUE)
                            sts_str_p = "True";
                        else
                            sts_str_p = "False";

                        CLI_LIB_PrintStr_N("  %-*s : %s", left_width, "CBS", sts_str_p);
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : %d", left_width,
                            "Number of TCs supported", rem_ets_entry.rem_config_max_tc);
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Priority Assignment Table");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02d  ", index, (UI8_T)(0xf &(rem_ets_entry.rem_config_pri_assign_table[index/2]>>(((index+1)%2)*4))));
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s   ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02d  ", index, (UI8_T)(0xf &(rem_ets_entry.rem_config_pri_assign_table[index/2]>>(((index+1)%2)*4))));
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Traffic Class Bandwidth(Hex)");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02x  ", index, rem_ets_entry.rem_config_tc_bandwidth_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02x  ", index, rem_ets_entry.rem_config_tc_bandwidth_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Traffic Selection Algorithm");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%-3d ", index, rem_ets_entry.rem_config_tsa_assign_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%-3d ", index, rem_ets_entry.rem_config_tsa_assign_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");
                    }

                    /* ETS recommendation */
                    if(rem_ets_entry.rem_recommend_rcvd)
                    {
                        PROCESS_MORE_FUNC("\r\n");
                        PROCESS_MORE_FUNC("ETS Recommendation\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Priority Assignment Table");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02d  ", index, (UI8_T)(0xf &(rem_ets_entry.rem_recommend_pri_assign_table[index/2]>>(((index+1)%2)*4))));
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s   ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02d  ", index, (UI8_T)(0xf &(rem_ets_entry.rem_recommend_pri_assign_table[index/2]>>(((index+1)%2)*4))));
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Traffic Class Bandwidth(Hex)");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02x  ", index, rem_ets_entry.rem_recommend_tc_bandwidth_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%02x  ", index, rem_ets_entry.rem_recommend_tc_bandwidth_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                            "Traffic Selection Algorithm");
                        for(index = 0; index < 4; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%-3d ", index, rem_ets_entry.rem_recommend_tsa_assign_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");

                        CLI_LIB_PrintStr_N("  %-*s : ", left_width, "");
                        for(index = 4; index < 8; index++)
                        {
                            CLI_LIB_PrintStr_N("[%d]%-3d ", index, rem_ets_entry.rem_recommend_tsa_assign_table[index]);
                        }
                        PROCESS_MORE_FUNC("\r\n");
                    }
                }
            }
            /* PFC configuration */
            {
                LLDP_TYPE_DcbxRemPfcEntry_T rem_pfc_entry;
                char    *sts_str_p;
                UI8_T index = 0;

                memset(&rem_pfc_entry, 0, sizeof(LLDP_TYPE_DcbxRemPfcEntry_T));
                rem_pfc_entry.rem_local_port_num = system_entry.rem_local_port_num;
                rem_pfc_entry.rem_index = system_entry.rem_index;
                left_width = strlen("Max PFC classes supported");
                if(LLDP_TYPE_RETURN_OK == LLDP_PMGR_GetDcbxPfcRemoteData(&rem_pfc_entry))
                {
                    PROCESS_MORE_FUNC("\r\n");
                    PROCESS_MORE_FUNC("PFC Configuration\r\n");
                    if (rem_pfc_entry.rem_willing == TRUE)
                        sts_str_p = "True";
                    else
                        sts_str_p = "False";

                    CLI_LIB_PrintStr_N("  %-*s : %s", left_width, "Willing", sts_str_p);
                    PROCESS_MORE_FUNC("\r\n");

                    if (rem_pfc_entry.rem_mbc == TRUE)
                        sts_str_p = "True";
                    else
                        sts_str_p = "False";

                    CLI_LIB_PrintStr_N("  %-*s : %s", left_width, "MBC", sts_str_p);
                    PROCESS_MORE_FUNC("\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : %u", left_width,
                        "Max PFC classes supported", rem_pfc_entry.rem_cap);
                    PROCESS_MORE_FUNC("\r\n");

                    CLI_LIB_PrintStr_N("  %-*s : ", left_width,
                        "PFC Enable Vector");
                    for(index = 0; index < 8; index++)
                    {
                        CLI_LIB_PrintStr_N("[%d]%d ", index, (UI8_T)((1<<index)&rem_pfc_entry.rem_enable)?1:0);
                    }
                    PROCESS_MORE_FUNC("\r\n");

                }
            }
        }
#endif
        PROCESS_MORE_FUNC("\r\n");
    } /* end of while */

    return line_num;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_LldpRemote
 *-------------------------------------------------------------------------
 * PURPOSE  : Show the information of LLDP remote devices (global and interface)
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_LldpRemote(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T unit, i, trunk_id;
    UI32_T  err_idx;
    UI32_T lport;
    CLI_API_EthStatus_T verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char *s;
    char delemiters[2] = {0};

    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W5_SHOW_LLDP_INFO_REMOTEDEVICE_DETAIL:
        {
            if (arg[0][0] == 'e' || arg[0][0] == 'E')
            {
                s = arg[1];

                /*get the unit*/
                unit = atoi(s);

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

                        for(i=lower_val; i<=upper_val; i++)
                        {
                            verify_ret_e = verify_ethernet(unit, i, &lport);

                            if ((verify_ret_e == CLI_API_ETH_NOT_PRESENT) || (verify_ret_e == CLI_API_ETH_UNKNOWN_PORT))
                            {
                                display_ethernet_msg(verify_ret_e, unit, i);
                                continue;
                            }

                            if ((line_num = show_one_lldp_remote(lport, line_num)) ==  JUMP_OUT_MORE)
                            {
                                return CLI_NO_ERROR;
                            }
                            else if (line_num == EXIT_SESSION_MORE)
                            {
                                return CLI_EXIT_SESSION;
                            }
                        }
                    }
                    if(s == 0)
                        break;
                    else
                        memset(Token, 0, sizeof(Token));
                }
            }
            else if (arg[0][0] == 'p' || arg[0][0] == 'P')
            {
                trunk_id = atoi(arg[1]);
                if ((verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, trunk_id);
                    return CLI_NO_ERROR;
                }

                if((line_num = show_one_lldp_remote(lport, line_num)) ==  JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
            break;
        }

        case PRIVILEGE_EXEC_CMD_W4_SHOW_LLDP_INFO_REMOTEDEVICE:
        {
            UI32_T  unit, port, trunk_id;
            LLDP_MGR_RemoteSystemData_T system_entry;

            memset(&system_entry, 0, sizeof(LLDP_MGR_RemoteSystemData_T));
            PROCESS_MORE("LLDP Remote Devices Information\r\n");
            PROCESS_MORE(" Local Port Chassis ID        Port ID           System Name\r\n");
            PROCESS_MORE(" ---------- ----------------- ----------------- ------------------------------\r\n");
            while (LLDP_PMGR_GetNextRemoteSystemDataByIndex(&system_entry) == LLDP_TYPE_RETURN_OK)
            {
                SWCTRL_Lport_Type_T port_type = SWCTRL_POM_LogicalPortToUserPort(system_entry.rem_local_port_num, &unit, &port, &trunk_id);

                if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                {
                    CLI_LIB_PrintStr_N(" Eth %lu/%-2lu   ", (unsigned long)unit, (unsigned long)port);
                }
                else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    CLI_LIB_PrintStr_N(" Trunk %-2lu   ", (unsigned long)trunk_id);
                }

                if (system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceAlias ||
                    system_entry.rem_chassis_id_subtype == VAL_lldpRemChassisIdSubtype_interfaceName)
                {
                    if (system_entry.rem_chassis_id_len > 17)
                    {
                        CLI_LIB_PrintStr_N("%.*s", 14, system_entry.rem_chassis_id);
                        CLI_LIB_PrintStr_N("... ");
                    }
                    else
                    {
                        CLI_LIB_PrintStr_N("%-17.*s ", (int)system_entry.rem_chassis_id_len, system_entry.rem_chassis_id);
                    }
                }
                else
                {
                    if (system_entry.rem_chassis_id_len > 6)
                    {
                        CLI_API_Lldp_PrintHex(system_entry.rem_chassis_id, 5);
                        CLI_LIB_PrintStr_N("... ");
                    }
                    else
                    {
                        CLI_API_Lldp_PrintHex(system_entry.rem_chassis_id, system_entry.rem_chassis_id_len);
                        CLI_LIB_PrintStr_N("%*s ", (int)(18 - system_entry.rem_chassis_id_len * 3), "");
                    }
                }

                if (    (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFALIAS)
                     || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_IFNAME)
                     || (system_entry.rem_port_id_subtype == LLDP_TYPE_PORT_ID_SUBTYPE_LOCAL)
                   )
                {
                    if (system_entry.rem_port_id_len > 17)
                    {
                        CLI_LIB_PrintStr_N("%.*s", 14, system_entry.rem_port_id);
                        CLI_LIB_PrintStr_N("... ");
                    }
                    else
                    {
                        CLI_LIB_PrintStr_N("%-17.*s ", (int)system_entry.rem_port_id_len, system_entry.rem_port_id);
                    }
                }
                else
                {
                    if (system_entry.rem_port_id_len > 6)
                    {
                        CLI_API_Lldp_PrintHex(system_entry.rem_port_id, 5);
                        CLI_LIB_PrintStr_N("... ");
                    }
                    else
                    {
                        CLI_API_Lldp_PrintHex(system_entry.rem_port_id, system_entry.rem_port_id_len);
                        CLI_LIB_PrintStr_N("%*s ", (int)(18 - system_entry.rem_port_id_len * 3), "");
                    }
                }

                if (system_entry.rem_sys_name_len > 30)
                {
                    int i;

                    for(i = 0; i < 27; i++)
                    {
                        CLI_LIB_PutChar(system_entry.rem_sys_name[i]);
                    }
                    CLI_LIB_PrintStr_N("...");
                }
                else
                {
                    CLI_LIB_PrintStr_N("%s", system_entry.rem_sys_name);
                }
                PROCESS_MORE("\r\n");
            } /* end of while */

            PROCESS_MORE("\r\n");
            break;
        }
    }

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_LldpStatistics
 *-------------------------------------------------------------------------
 * PURPOSE  : Show the information of LLDP statistics (global and interface)
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_LldpStatistics(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char delemiters[2] = {0};
    char Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char *s;
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx, trunk_id, unit, port;
    UI8_T  i;
    UI32_T lport;
    CLI_API_EthStatus_T verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    LLDP_MGR_Statistics_T statistics_entry;
    LLDP_MGR_PortRxStatistics_T rx_port_statistics_entry;
    LLDP_MGR_PortTxStatistics_T tx_port_statistics_entry;

    rx_port_statistics_entry.port_num = 0;
    tx_port_statistics_entry.port_num = 0;

    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W5_SHOW_LLDP_INFO_STATISTICS_DETAIL:

            if((arg[0]==NULL)||(arg[1]==NULL))
                return CLI_ERR_INTERNAL;

            if (arg[0][0] == 'e' || arg[0][0] == 'E')
            {
                s = arg[1];

                /*get the unit*/
                unit = atoi((char*)s);

                /*move the ptr to just after the slash*/
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
                        for(i=lower_val; i<=upper_val; i++)
                        {
                            verify_ret_e = verify_ethernet(unit, i, &lport);

                            if ((verify_ret_e == CLI_API_ETH_NOT_PRESENT) || (verify_ret_e == CLI_API_ETH_UNKNOWN_PORT))
                            {
                                display_ethernet_msg(verify_ret_e, unit, i);
                                continue;
                            }
                            else
                            {
                                rx_port_statistics_entry.port_num = lport;
                                tx_port_statistics_entry.port_num = lport;
                                LLDP_PMGR_GetPortTxStatisticsEntry(&tx_port_statistics_entry);

                                if (LLDP_PMGR_GetPortRxStatisticsEntry(&rx_port_statistics_entry) == LLDP_TYPE_RETURN_OK)
                                {
                                    PROCESS_MORE("LLDP Port Statistics Detail\r\n");
                                    CLI_LIB_PrintStr_N(" Port Name         : Eth %lu/%d", (unsigned long)unit, i);
                                    PROCESS_MORE("\r\n");
                                    PROCESS_MORE_1(" Frames Discarded  : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_discarded_total);
                                    PROCESS_MORE_1(" Frames Invalid    : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_errors);
                                    PROCESS_MORE_1(" Frames Received   : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_total);
                                    PROCESS_MORE_1(" Frames Sent       : %lu\r\n", (unsigned long)tx_port_statistics_entry.tx_frames_total);
                                    PROCESS_MORE_1(" TLVs Unrecognized : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_tlvs_unrecognized_total);
                                    PROCESS_MORE_1(" TLVs Discarded    : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_tlvs_discarded_total);
                                    PROCESS_MORE_1(" Neighbor Ageouts  : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_ageouts_total);
                                    PROCESS_MORE("\r\n");
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
            else if (arg[0][0] == 'p' || arg[0][0] == 'P')
            {
                trunk_id = atoi(arg[1]);
                if ((verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, trunk_id);
                    return CLI_NO_ERROR;
                }

                rx_port_statistics_entry.port_num = lport;
                tx_port_statistics_entry.port_num = lport;
                LLDP_PMGR_GetPortTxStatisticsEntry(&tx_port_statistics_entry);

                if (LLDP_PMGR_GetPortRxStatisticsEntry(&rx_port_statistics_entry) == LLDP_TYPE_RETURN_OK)
                {
                    PROCESS_MORE("LLDP Port Statistics Detail\r\n");
                    CLI_LIB_PrintStr_N(" Port Name         : Trunk %lu", (unsigned long)trunk_id);
                    PROCESS_MORE("\r\n");
                    PROCESS_MORE_1(" Frames Discarded  : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_discarded_total);
                    PROCESS_MORE_1(" Frames Invalid    : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_errors);
                    PROCESS_MORE_1(" Frames Received   : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_frames_total);
                    PROCESS_MORE_1(" Frames Sent       : %lu\r\n", (unsigned long)tx_port_statistics_entry.tx_frames_total);
                    PROCESS_MORE_1(" TLVs Unrecognized : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_tlvs_unrecognized_total);
                    PROCESS_MORE_1(" TLVs Discarded    : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_tlvs_discarded_total);
                    PROCESS_MORE_1(" Neighbor Ageouts  : %lu\r\n", (unsigned long)rx_port_statistics_entry.rx_ageouts_total);
                    PROCESS_MORE("\r\n");
                }
            }
            break;

        case PRIVILEGE_EXEC_CMD_W4_SHOW_LLDP_INFO_STATISTICS:
        {
            PROCESS_MORE("LLDP Global Statistics\r\n");
            LLDP_PMGR_GetSysStatisticsEntry(&statistics_entry);
            statistics_entry.rem_tables_last_change_time = statistics_entry.rem_tables_last_change_time/SYS_BLD_TICKS_PER_SECOND;
            PROCESS_MORE_1(" Neighbor Entries List Last Updated : %lu seconds\r\n", (unsigned long)statistics_entry.rem_tables_last_change_time);
            PROCESS_MORE_1(" New Neighbor Entries Count         : %lu\r\n", (unsigned long)statistics_entry.rem_tables_inserts);
            PROCESS_MORE_1(" Neighbor Entries Deleted Count     : %lu\r\n", (unsigned long)statistics_entry.rem_tables_deletes);
            PROCESS_MORE_1(" Neighbor Entries Dropped Count     : %lu\r\n", (unsigned long)statistics_entry.rem_tables_drops);
            PROCESS_MORE_1(" Neighbor Entries Ageout Count      : %lu\r\n", (unsigned long)statistics_entry.rem_tables_ageouts);
            PROCESS_MORE("\r\n");
            PROCESS_MORE("LLDP Port Statistics\r\n");
            PROCESS_MORE(" Port     NumFramesRecvd NumFramesSent NumFramesDiscarded\r\n");
            PROCESS_MORE(" -------- -------------- ------------- ------------------\r\n");

            while(LLDP_PMGR_GetNextPortRxStatisticsEntry(&rx_port_statistics_entry) == LLDP_TYPE_RETURN_OK)
            {
                SWCTRL_Lport_Type_T port_type = SWCTRL_POM_LogicalPortToUserPort(rx_port_statistics_entry.port_num, &unit, &port, &trunk_id);

                if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                {
                    CLI_LIB_PrintStr_N(" Eth %lu/%-2lu ", (unsigned long)unit, (unsigned long)port);
                }
                else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                {
                    CLI_LIB_PrintStr_N(" Trunk %-2lu ", (unsigned long)trunk_id);
                }

                tx_port_statistics_entry.port_num = rx_port_statistics_entry.port_num;
                LLDP_PMGR_GetPortTxStatisticsEntry(&tx_port_statistics_entry);

                CLI_LIB_PrintStr_N("%14lu %13lu %18lu", (unsigned long)rx_port_statistics_entry.rx_frames_total,
                    (unsigned long)tx_port_statistics_entry.tx_frames_total, (unsigned long)rx_port_statistics_entry.rx_frames_discarded_total);
                PROCESS_MORE("\r\n");
            }

            PROCESS_MORE("\r\n");
            break;
        }
    }

    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_medFastStartCount
 *-------------------------------------------------------------------------
 * PURPOSE  : To configure the LLDP packet send counts for fast start
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_medFastStartCount(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_LLDP_MEDFASTSTARTCOUNT:
        {
            if(LLDP_PMGR_SetXMedFastStartRepeatCount(atoi(arg[0])) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to configure the fast start counts.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LLDP_MEDFASTSTARTCOUNT:
        {
            if(LLDP_PMGR_SetXMedFastStartRepeatCount(LLDP_TYPE_DEFAULT_FAST_START_REPEAT_COUNT) != TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_N("Failed to configure the fast start counts to default value.\r\n");
#endif
                return CLI_NO_ERROR;
            }
            break;
        }

        default:
            return CLI_ERR_INTERNAL;

    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
   return CLI_NO_ERROR;
}/*End of CLI_API_LLDP_medFastStartCount*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_medTlv_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : To configure the LLDP packet send which type TLVs
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_medTlv_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI16_T  tlvs_tx_enabled=0;
    UI32_T  lport, i, verify_port, verify_ret;
    UI32_T  verify_unit = ctrl_P->sys_info.my_unit_id;
    LLDP_MGR_XMedPortConfigEntry_T port_config_entry;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            port_config_entry.lport=lport;

            if (FALSE == LLDP_PMGR_GetXMedPortConfigEntry(&port_config_entry))
            {
                continue;
            }

            tlvs_tx_enabled = port_config_entry.lldp_xmed_port_tlvs_tx_enabled;

            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_MEDTLV:
                    switch(arg[0][0])
                    {
                        case 'm':
                        case 'M':
                           tlvs_tx_enabled |= LLDP_TYPE_MED_CAP_TX;
                           break;

                        case 'n':
                        case 'N':
                           tlvs_tx_enabled |= LLDP_TYPE_MED_NETWORK_POLICY_TX;
                           break;

                        case 'l':
                        case 'L':
                           tlvs_tx_enabled |= LLDP_TYPE_MED_LOCATION_IDENT_TX;
                           break;

                        case 'e':
                        case 'E':
                           tlvs_tx_enabled |= (LLDP_TYPE_MED_EXT_PSE_TX | LLDP_TYPE_MED_EXT_PD_TX);
                           break;

                        case 'i':
                        case 'I':
                           tlvs_tx_enabled |= LLDP_TYPE_MED_INVENTORY_TX;
                           break;

                        default:
                           return CLI_ERR_INTERNAL;

                    }

                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_MEDTLV:
                    {
                        if(arg[0] != NULL)
                        {
                            switch(arg[0][0])
                            {
                                case 'm':
                                case 'M':
                                   tlvs_tx_enabled &= ~LLDP_TYPE_MED_CAP_TX;
                                   break;

                                case 'n':
                                case 'N':
                                   tlvs_tx_enabled &= ~LLDP_TYPE_MED_NETWORK_POLICY_TX;
                                   break;

                                case 'l':
                                case 'L':
                                   tlvs_tx_enabled &= ~LLDP_TYPE_MED_LOCATION_IDENT_TX;
                                   break;

                                case 'e':
                                case 'E':
                                   tlvs_tx_enabled &= ~(LLDP_TYPE_MED_EXT_PSE_TX | LLDP_TYPE_MED_EXT_PD_TX);
                                   break;

                                case 'i':
                                case 'I':
                                   tlvs_tx_enabled &= ~LLDP_TYPE_MED_INVENTORY_TX;
                                   break;

                                 default:
                                     return CLI_ERR_INTERNAL;
                             }
                        }
                        else
                        {
                            tlvs_tx_enabled = LLDP_TYPE_DEFAULT_MED_TX;
                        }
                    }
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (LLDP_PMGR_SetXMedPortConfigTlvsTx(lport,tlvs_tx_enabled)!=TRUE)
            {
                CLI_LIB_PrintStr_N("Failed to configure the MED TLVs.\r\n");
            }
        }
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
   return CLI_NO_ERROR;
} /*End of CLI_API_LLDP_medTlv_eth*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_medTlv_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : To configure the LLDP packet send which type TLVs
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_medTlv_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T lport, verify_ret;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI16_T tlvs_tx_enabled=0;
    LLDP_MGR_XMedPortConfigEntry_T port_config_entry;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    port_config_entry.lport=lport;

    if (FALSE == LLDP_PMGR_GetXMedPortConfigEntry(&port_config_entry))
    {
        return CLI_NO_ERROR;
    }

    tlvs_tx_enabled = port_config_entry.lldp_xmed_port_tlvs_tx_enabled;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_MEDTLV:
            switch(arg[0][0])
            {
                case 'm':
                case 'M':
                   tlvs_tx_enabled |= LLDP_TYPE_MED_CAP_TX;
                   break;

                case 'n':
                case 'N':
                   tlvs_tx_enabled |= LLDP_TYPE_MED_NETWORK_POLICY_TX;
                   break;

                case 'l':
                case 'L':
                   tlvs_tx_enabled |= LLDP_TYPE_MED_LOCATION_IDENT_TX;
                   break;

                case 'e':
                case 'E':
                   tlvs_tx_enabled |= (LLDP_TYPE_MED_EXT_PSE_TX | LLDP_TYPE_MED_EXT_PD_TX);
                   break;

                case 'i':
                case 'I':
                   tlvs_tx_enabled |= LLDP_TYPE_MED_INVENTORY_TX;
                   break;

                default:
                   return CLI_ERR_INTERNAL;

            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_MEDTLV:
            {
                if(arg[0] != NULL)
                {
                    switch(arg[0][0])
                    {
                        case 'm':
                        case 'M':
                           tlvs_tx_enabled &= ~LLDP_TYPE_MED_CAP_TX;
                           break;

                        case 'n':
                        case 'N':
                           tlvs_tx_enabled &= ~LLDP_TYPE_MED_NETWORK_POLICY_TX;
                           break;

                        case 'l':
                        case 'L':
                           tlvs_tx_enabled &= ~LLDP_TYPE_MED_LOCATION_IDENT_TX;
                           break;

                        case 'e':
                        case 'E':
                           tlvs_tx_enabled &= ~(LLDP_TYPE_MED_EXT_PSE_TX | LLDP_TYPE_MED_EXT_PD_TX);
                           break;

                        case 'i':
                        case 'I':
                           tlvs_tx_enabled &= ~LLDP_TYPE_MED_INVENTORY_TX;
                           break;

                        default:
                           return CLI_ERR_INTERNAL;

                    }
                }
                else
                {
                    tlvs_tx_enabled = LLDP_TYPE_DEFAULT_MED_TX;
                }
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (LLDP_PMGR_SetXMedPortConfigTlvsTx(lport,tlvs_tx_enabled)!=TRUE)
    {
        CLI_LIB_PrintStr_N("Failed to configure the MED TLVs.\r\n");
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}/*End of CLI_API_LLDP_medTlv_pch*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_medNotification_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : To configure the ethernet LLDP MED notification status
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_medNotification_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T  lport, i, verify_port, verify_ret;
    UI32_T  verify_unit = ctrl_P->sys_info.my_unit_id;
    UI8_T notif_enabled;


    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_MEDNOTIFICATION:
                    notif_enabled = VAL_lldpXMedPortConfigNotifEnable_true;
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_MEDNOTIFICATION:
                    notif_enabled = VAL_lldpXMedPortConfigNotifEnable_false;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if (LLDP_PMGR_SetXMedPortConfigNotifEnabled(lport,notif_enabled)!=TRUE)
            {
                CLI_LIB_PrintStr_N("Failed to configure the LLDP MED notification.\r\n");
            }

        }
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;

}/*End of CLI_API_LLDP_medNotification_eth*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_medNotification_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : To configure the port-channel LLDP MED  packet send which type TLVs
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_medNotification_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T lport, verify_ret;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI8_T notif_enabled;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_MEDNOTIFICATION:
            notif_enabled = VAL_lldpXMedPortConfigNotifEnable_true;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_MEDNOTIFICATION:
            notif_enabled = VAL_lldpXMedPortConfigNotifEnable_false;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (LLDP_PMGR_SetXMedPortConfigNotifEnabled(lport,notif_enabled)!=TRUE)
    {
        CLI_LIB_PrintStr_N("Failed to configure the LLDP MED notification.\r\n");
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}   /*End of CLI_API_LLDP_medNotification_pch*/

UI32_T CLI_API_Lldp_PortMedLocationCivicAddr_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;
    BOOL_T set_or_unset, set_ca_entry = FALSE;
    BOOL_T ret = FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LLDP_MEDLOCATION_CIVICADDR:
            set_or_unset = TRUE;
            if (arg[0])
            {
                ca_entry.ca_type = atoi(arg[0]);
                ca_entry.ca_length = strlen(arg[1]);
                memcpy(ca_entry.ca_value, arg[1], ca_entry.ca_length);
                set_ca_entry = TRUE;
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LLDP_MEDLOCATION_CIVICADDR:
            set_or_unset = FALSE;
            if (arg[0])
            {
                ca_entry.ca_type = atoi(arg[0]);
                set_ca_entry = TRUE;
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            if (set_ca_entry)
            {
                ret = LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry(lport, &ca_entry, set_or_unset);
            }
            else
            {
                if (!set_or_unset)
                    LLDP_PMGR_SetXMedLocLocationStatus(lport, VAL_lldpXMedLocLocationSubtype_civicAddress, FALSE);
                    ret = LLDP_PMGR_SetXMedLocLocationStatus(lport, VAL_lldpXMedLocLocationSubtype_civicAddress, TRUE);
            }

            if(!ret)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address on ethernet %s.\r\n", name);
               }
#else
               CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortMedLocationCivicAddr_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;


    LLDP_MGR_XMedLocationCivicAddrCaEntry_T ca_entry;
    BOOL_T set_or_unset=FALSE, set_ca_entry = FALSE;
    BOOL_T ret = FALSE;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_LLDP_MEDLOCATION_CIVICADDR:
            if (arg[0])
            {
                ca_entry.ca_type = atoi(arg[0]);
                ca_entry.ca_length = strlen(arg[1]);
                memcpy(ca_entry.ca_value, arg[1], ca_entry.ca_length);
                set_or_unset = TRUE;
                set_ca_entry = TRUE;
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_LLDP_MEDLOCATION_CIVICADDR:
            if (arg[0])
            {
                ca_entry.ca_type = atoi(arg[0]);
                set_or_unset = FALSE;
                set_ca_entry = TRUE;
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    if (set_ca_entry)
    {
        ret = LLDP_PMGR_SetXMedLocLocationCivicAddrCaEntry(lport, &ca_entry, set_or_unset);
    }
    else
    {
        if (!set_or_unset)
            LLDP_PMGR_SetXMedLocLocationStatus(lport, VAL_lldpXMedLocLocationSubtype_civicAddress, FALSE);
        ret = LLDP_PMGR_SetXMedLocLocationStatus(lport, VAL_lldpXMedLocLocationSubtype_civicAddress, TRUE);
    }

    if(!ret)
    {
       CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address on trunk %lu.\r\n", (unsigned long)verify_trunk_id);
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortMedLocationCivicAddrCountry_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    UI8_T country_code[2];
    BOOL_T ret = FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_LLDP_MEDLOCATION_CIVICADDR_COUNTRY:
            memcpy(country_code, arg[0], 2);
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_LLDP_MEDLOCATION_CIVICADDR_COUNTRY:
            memcpy(country_code, LLDP_TYPE_DEFAULT_MED_LOCATION_CA_CONNTRY, 2);
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            ret = LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode(lport, country_code);

            if(!ret)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address country code on ethernet %s.\r\n", name);
               }
#else
               CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address country code on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortMedLocationCivicAddrCountry_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    UI8_T country_code[2];
    BOOL_T ret = FALSE;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_LLDP_MEDLOCATION_CIVICADDR_COUNTRY:
            memcpy(country_code, arg[0], 2);
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_LLDP_MEDLOCATION_CIVICADDR_COUNTRY:
            memcpy(country_code, LLDP_TYPE_DEFAULT_MED_LOCATION_CA_CONNTRY, 2);
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    ret = LLDP_PMGR_SetXMedLocLocationCivicAddrCoutryCode(lport, country_code);

    if(!ret)
    {
       CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address country code on trunk %lu.\r\n", (unsigned long)verify_trunk_id);
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortMedLocationCivicAddrWhat_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;

    UI8_T what;
    BOOL_T ret = FALSE;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_LLDP_MEDLOCATION_CIVICADDR_WHAT:
            what = atoi(arg[0]);
            break;
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_LLDP_MEDLOCATION_CIVICADDR_WHAT:
            what = LLDP_TYPE_DEFAULT_MED_LOCATION_CA_WHAT;
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }

            ret = LLDP_PMGR_SetXMedLocLocationCivicAddrWhat(lport, what);

            if(!ret)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address what value on ethernet %s.\r\n", name);
               }
#else
               CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address what value on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Lldp_PortMedLocationCivicAddrWhat_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LLDP_MED == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;


    UI8_T what;
    BOOL_T ret = FALSE;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_LLDP_MEDLOCATION_CIVICADDR_WHAT:
            what = atoi(arg[0]);
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_LLDP_MEDLOCATION_CIVICADDR_WHAT:
            what = LLDP_TYPE_DEFAULT_MED_LOCATION_CA_WHAT;
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    ret = LLDP_PMGR_SetXMedLocLocationCivicAddrWhat(lport, what);

    if(!ret)
    {
       CLI_LIB_PrintStr_N("Failed to configure the LLDP MED civic address what value on trunk %lu.\r\n", (unsigned long)verify_trunk_id);
    }
#endif /* #if (SYS_CPNT_LLDP_MED == TRUE) */
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_DCBX == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_DcbxTlv_eth
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp dcbx-tlv and no lldp dcbx-tlv
 *            config (DCBX TLVs) in interface eth.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_DcbxTlv_eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  lport   = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    UI32_T  verify_ret;
    UI32_T  i = 0;
    LLDP_TYPE_XdcbxPortConfigEntry_T  port_config_entry;

    memset(&port_config_entry, 0, sizeof(LLDP_TYPE_XdcbxPortConfigEntry_T));
    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                port_config_entry.lport = lport;
                if(!LLDP_PMGR_GetXdcbxPortConfig(&port_config_entry))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set DCBX TLV on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }

                if(arg[0]!=NULL)
                {
                    switch(cmd_idx)
                    {
                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_LLDP_DCBXTLV:
                            if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                                (*(arg[0]+4) == 'C' || *(arg[0]+4) == 'c'))
                            {
                                /** ets-config **/
                                port_config_entry.ets_config_tlv_enable = TRUE;
                            }
                            else if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                                (*(arg[0]+4) == 'R' || *(arg[0]+4) == 'r'))
                            {
                                /** ets-recommend **/
                                    port_config_entry.ets_recommend_tlv_enable = TRUE;
                            }
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                /** pfc-config **/
                                port_config_entry.pfc_config_tlv_enable = TRUE;
                            }
                            break;

                        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_LLDP_DCBXTLV:
                            if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                                (*(arg[0]+4) == 'C' || *(arg[0]+4) == 'c'))
                            {
                                /** ets-config **/
                                port_config_entry.ets_config_tlv_enable = FALSE;
                            }
                            else if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                                (*(arg[0]+4) == 'R' || *(arg[0]+4) == 'r'))
                            {
                                /** ets-recommend **/
                                    port_config_entry.ets_recommend_tlv_enable = FALSE;
                            }
                            else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                            {
                                /** pfc-config **/
                                port_config_entry.pfc_config_tlv_enable = FALSE;
                            }
                            break;

                        default:
                            return CLI_ERR_INTERNAL;
                    }
                }
                else
                    return CLI_ERR_INTERNAL;

                if (LLDP_PMGR_SetXdcbxPortConfig(&port_config_entry)!=LLDP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set DCBX TLV on ethernet %s.\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_N("Failed to set DCBX TLV on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                }
            }
        }
    }
    return CLI_NO_ERROR;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_LLDP_DcbxTlv_pch
 *-------------------------------------------------------------------------
 * PURPOSE  : API for lldp dcbx-tlv and no lldp dcbx-tlv
 *            config (DCBX TLVs) in interface pch.
 * INPUT    :
 * OUTPUT   :
 * RETURN   : CLI_NO_ERROR       -- success
 *
 * Note     : None
 *-------------------------------------------------------------------------
 */
UI32_T CLI_API_LLDP_DcbxTlv_pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  lport   = 0;
    LLDP_TYPE_XdcbxPortConfigEntry_T  port_config_entry;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T verify_ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    port_config_entry.lport = lport;

    if(!LLDP_PMGR_GetXdcbxPortConfig(&port_config_entry))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set DCBX TLV on Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
        return CLI_NO_ERROR;
    }

    if(arg[0]!=NULL)
    {
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_LLDP_DCBXTLV:
                if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                    (*(arg[0]+4) == 'C' || *(arg[0]+4) == 'c'))
                {
                    /** ets-config **/
                    port_config_entry.ets_config_tlv_enable = TRUE;
                }
                else if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                    (*(arg[0]+4) == 'R' || *(arg[0]+4) == 'r'))
                {
                    /** ets-recommend **/
                    port_config_entry.ets_recommend_tlv_enable = TRUE;
                }
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    /** pfc-config **/
                    port_config_entry.pfc_config_tlv_enable = TRUE;
                }
                break;

            case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_LLDP_DCBXTLV:
                if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                    (*(arg[0]+4) == 'C' || *(arg[0]+4) == 'c'))
                {
                    /** ets-config **/
                    port_config_entry.ets_config_tlv_enable = FALSE;
                }
                else if ((*(arg[0]) == 'E' || *(arg[0]) == 'e') &&
                    (*(arg[0]+4) == 'R' || *(arg[0]+4) == 'r'))
                {
                    /** ets-recommend **/
                    port_config_entry.ets_recommend_tlv_enable = FALSE;
                }
                else if (*(arg[0]) == 'P' || *(arg[0]) == 'p')
                {
                    /** pfc-config **/
                    port_config_entry.pfc_config_tlv_enable = FALSE;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }
    else
        return CLI_ERR_INTERNAL;

    if (LLDP_PMGR_SetXdcbxPortConfig(&port_config_entry) != LLDP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_N("Failed to set DCBX TLV on Trunk %lu.\r\n", (unsigned long)verify_trunk_id);
#endif
    }
    return CLI_NO_ERROR;
}

#endif

#endif
