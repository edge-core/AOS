#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <stdio.h>
#include "sysfun.h"
#include "sys_dflt.h"
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_msg.h"
#include "cli_io.h"
#include "cli_task.h"
#include "cli_lib.h"
#include "cli_pars.h"
#include "cli_cmd.h"
#include "cli_api.h"
#include "cli_tbl.h"
#include "cli_api_1x.h"
#include "1x_pmgr.h"
#include "1x_pom.h"
#include "1x_mgr.h"
#include "sys_cpnt.h"
#include "swctrl.h"
#include "leaf_ieee8021x.h"

#if(SYS_CPNT_NETACCESS == TRUE)
    #include "netaccess_pmgr.h"
#endif

typedef enum {
    EAPOL_PASS_THRU_UNKNOWN = 0,
    EAPOL_PASS_THRU_ENABLE,
    EAPOL_PASS_THRU_DISABLE
}CLI_API_1X_Dot1XEapolPassThru_T;

typedef enum {
    PORT_OPER_MODE_UNKNOWN = 0, /* error happened */
    PORT_OPER_MODE_SINGLE_HOST,
    PORT_OPER_MODE_MULTI_HOST,
    PORT_OPER_MODE_MAC_BASED,
    PORT_OPER_MODE_MAX
}CLI_API_1X_Dot1XPortOperMode_T;

/* Local function
 */
#if (SYS_CPNT_DOT1X == TRUE)
static char *port_control_str(UI32_T ctrl_mode);
static char *operation_mode_str(UI32_T oper_mode);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
static CLI_API_1X_Dot1XEapolPassThru_T get_eapol_pass_thru();
#endif

enum {
    DOT1X_SUMMARY_TBL_PORT = 0,
    DOT1X_SUMMARY_TBL_TYPE,
    DOT1X_SUMMARY_TBL_OPER_MODE,
    DOT1X_SUMMARY_TBL_CTRL_MODE,
    DOT1X_SUMMARY_TBL_AUTHOR
};

UI32_T CLI_API_Authentication_Dot1x(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_AUTHENTICATION_DOT1X:

      if(DOT1X_PMGR_Set_Authen_do1x()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to enable 802.1x authentication and use RADIUS method.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_AUTHENTICATION_DOT1X:

      if(DOT1X_PMGR_No_Authen_do1x()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to disable 802.1x authentication.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Default(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
#if (SYS_CPNT_NETACCESS == TRUE)
    if(NETACCESS_PMGR_SetDot1xConfigSettingToDefault()!= TRUE)
#else
    if(DOT1X_PMGR_SetConfigSettingToDefault()!=TRUE)
#endif
    {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set default value of 1X configuration.\r\n");
#endif
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Eapol_Pass_Through(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
    UI32_T status;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_DOT1X_EAPOLPASSTHROUGH:
        status = VAL_dot1xEapolPassThrough_enabled;
        break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_DOT1X_EAPOLPASSTHROUGH:
        status = VAL_dot1xEapolPassThrough_disabled;
        break;

        default:
        return CLI_ERR_INTERNAL;
    }

    if (FALSE == NETACCESS_PMGR_SetDot1xEapolPassThrough(status))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set EAPOL frames pass through\r\n");
#endif
    }
#endif /*#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Re_authentication(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_DOT1X_REAUTHENTICATION:

      if(DOT1X_PMGR_Set_ReAuthenticationMode(TRUE)!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to enable global period re-authentication of the client.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_DOT1X_REAUTHENTICATION:

      if(DOT1X_PMGR_No_ReAuthenticationMode()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set global period re-authentication of the client to default.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Quietperiod(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_DOT1X_TIMEOUT_QUIETPERIOD:

      if(DOT1X_PMGR_Set_QuietPeriod(atoi(arg[0]))!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set quiet period.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_DOT1X_TIMEOUT_QUIETPERIOD:

      if(DOT1X_PMGR_No_QuietPeriod()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set quiet period to default.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Re_authperiod(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_DOT1X_TIMEOUT_REAUTHPERIOD:

      if(DOT1X_PMGR_Set_ReAuthPeriod(atoi(arg[0]))!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set re-authentication period.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_DOT1X_TIMEOUT_REAUTHPERIOD:

      if(DOT1X_PMGR_No_ReAuthPeriod()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set re-authentication period to default.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Tx_period(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_DOT1X_TIMEOUT_TXPERIOD:

      if(DOT1X_PMGR_Set_TxPeriod(atoi(arg[0]))!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set TxPeriod.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_DOT1X_TIMEOUT_TXPERIOD:

      if(DOT1X_PMGR_No_TxPeriod()!=TRUE)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set TxPeriod to default.\r\n");
#endif
         return CLI_NO_ERROR;
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Port_MaxReauthReq_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i;
    UI32_T  max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_MAXREAUTHREQ:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

                if(NETACCESS_PMGR_SetDot1xPortMaxReAuthReq(l_port, (UI32_T)atoi(arg[0])) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set port max request time on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_MAXREAUTHREQ:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

                if(NETACCESS_PMGR_SetDot1xPortMaxReAuthReq(l_port, SYS_DFLT_DOT1X_AUTH_MAX_REQ) != TRUE)
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore port max request time on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Port_MaxRequest_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i;
    UI32_T  max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_MAXREQ:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortMaxReq(l_port, (UI32_T)atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortMaxReq(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set port max request time on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_MAXREQ:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortMaxReq(l_port, SYS_DFLT_DOT1X_AUTH_MAX_REQ) != TRUE)
#else
                if(DOT1X_PMGR_No_PortMaxReq(l_port)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore port max request time on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Port_Re_authentication_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i;
    UI32_T  max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_REAUTHENTICATION:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortReAuthEnabled(l_port, VAL_dot1xPaePortReauthenticate_true) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortReAuthEnabled(l_port, VAL_dot1xPaePortReauthenticate_true)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set port reauthentication on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_REAUTHENTICATION:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortReAuthEnabled(l_port, SYS_DFLT_DOT1X_AUTH_REAUTH_ENABLED) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortReAuthEnabled(l_port, SYS_DFLT_DOT1X_AUTH_REAUTH_ENABLED)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore port reauthentication on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Dot1x_Multiple_Hosts_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   UI32_T i;
   UI32_T max_port_num = 0;

   max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_MULTIPLEHOSTS:
      for (i = 1; i <= max_port_num; i++)
      {
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            UI32_T l_port;
            BOOL_T is_inherit = TRUE;

            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
            {
                display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                continue;
            }

            if(DOT1X_PMGR_Set_MultiHostMode(l_port, TRUE)!=TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_2("Failed to set multiple hosts mode on port %lu/%lu.\r\n", ctrl_P->CMenu.unit_id, i);
#endif
            }
         }
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_MULTIPLEHOSTS:

      for (i = 1; i <= max_port_num; i++)
      {
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            UI32_T l_port;
            BOOL_T is_inherit = TRUE;

            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
            {
                display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                continue;
            }

            if(DOT1X_MGR_No_MultiHostMode(l_port)!=TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_2("Failed to set default multiple hosts mode on port %lu/%lu.\r\n", ctrl_P->CMenu.unit_id, i);
#endif
            }
         }
      }
      break;
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Port_Control_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T i;
    UI32_T type;
    UI32_T max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_PORTCONTROL:
            if (arg[0][0] == 'a' || arg[0][0] == 'A')
            {
                type = VAL_dot1xAuthAuthControlledPortControl_auto;
            }
            else if (arg[0][6] == 'a' || arg[0][6] == 'A')
            {
                type = VAL_dot1xAuthAuthControlledPortControl_forceAuthorized;
            }
            else if (arg[0][6] == 'u' || arg[0][6] == 'U')
            {
                type = VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized;
            }
            else
            {
                return CLI_ERR_INTERNAL;
            }

            for (i = 1; i <= max_port_num; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    UI32_T l_port;
                    BOOL_T is_inherit = TRUE;

                    if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                    {
                        display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                        continue;
                    }

#if(SYS_CPNT_NETACCESS == TRUE)
                    if(NETACCESS_PMGR_SetDot1xPortControlMode(l_port, type) != TRUE)
#else
                    if(DOT1X_PMGR_Set_PortControlMode(l_port, type)!=TRUE)
#endif
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to set port control mode on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                    }
                }
            }
            break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_PORTCONTROL:

        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortControlMode(l_port, SYS_DFLT_DOT1X_AUTH_CONTROLLED_PORT_CONTROL) != TRUE)
#else
                if(DOT1X_PMGR_No_PortControlMode(l_port)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to disable port control mode on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Port_Quietperiod_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_DOT1X_TIMEOUT_QUIETPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortQuietPeriod(l_port, atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortQuietPeriod(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set per-port quiet period on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_DOT1X_TIMEOUT_QUIETPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortQuietPeriod(l_port, SYS_DFLT_DOT1X_AUTH_QUIET_PERIOD) != TRUE)
#else
                if(DOT1X_PMGR_No_PortQuietPeriod(l_port)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore per-port quiet period on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Port_Reauthperiod_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_DOT1X_TIMEOUT_REAUTHPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortReAuthPeriod(l_port, atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortReAuthPeriod(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set per-port re-authentication period on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_DOT1X_TIMEOUT_REAUTHPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortReAuthPeriod(l_port, SYS_DFLT_DOT1X_AUTH_REAUTH_PERIOD) != TRUE)
#else
                if(DOT1X_PMGR_No_PortReAuthPeriod(l_port)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore per-port re-authentication period on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Timeout_Port_Txperiod_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_DOT1X_TIMEOUT_TXPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortTxPeriod(l_port, atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortTxPeriod(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set per-port TxPeriod on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_DOT1X_TIMEOUT_TXPERIOD:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortTxPeriod(l_port, SYS_DFLT_DOT1X_AUTH_TX_PERIOD) != TRUE)
#else
                if(DOT1X_PMGR_No_PortTxPeriod(l_port)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore per-port TxPeriod on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

/*xiongyu add 20081225*/
UI32_T CLI_API_Dot1x_Timeout_Port_AuthServerTimeout_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 /* server-timeout command is removed */
    UI32_T  i, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_DOT1X_TIMEOUT_SERVERTIMEOUT:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortAuthServerTimeout(l_port, atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_AuthServerTimeout(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set per-port auth server timeout on port %lu/%lu.\r\n", ctrl_P->CMenu.unit_id, i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_DOT1X_TIMEOUT_SERVERTIMEOUT:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortAuthServerTimeout(l_port, SYS_DFLT_DOT1X_AUTH_SERVER_TIMEOUT) != TRUE)
#else
                if(DOT1X_PMGR_Set_AuthServerTimeout(l_port, SYS_DFLT_DOT1X_AUTH_SERVER_TIMEOUT)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore per-port auth server timeout on port %lu/%lu.\r\n", ctrl_P->CMenu.unit_id, i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if 0 */
    return CLI_NO_ERROR;
}

/*xiongyu add 20081225*/
UI32_T CLI_API_Dot1x_Timeout_Port_AuthSuppTimeout_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_DOT1X_TIMEOUT_SUPPTIMEOUT:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout(l_port, atoi((char*)arg[0])) != TRUE)
#else
                if(DOT1X_PMGR_Set_AuthSuppTimeout(l_port, atoi(arg[0]))!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set per-port auth supplicant timeout on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_DOT1X_TIMEOUT_SUPPTIMEOUT:
        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortAuthSuppTimeout(l_port, SYS_DFLT_DOT1X_AUTH_SUPP_TIMEOUT) != TRUE)
#else
                if(DOT1X_PMGR_Set_AuthSuppTimeout(l_port, SYS_DFLT_DOT1X_AUTH_SUPP_TIMEOUT)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to restore per-port auth supplicant timeout on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
                }
            }
        }
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}
UI32_T CLI_API_Dot1x_Re_authenticate_Exec(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i ,j;
    UI32_T  max_port_num = 0;

    if(arg[0]!=0)
    {
        UI32_T unit;
        UI32_T port;
        UI32_T l_port;
        BOOL_T is_inherit = TRUE;

        unit = atoi((char*)arg[2]);
        port = atoi(strchr((char*)arg[2], '/')+1);

        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(unit, port, &l_port, &is_inherit))
        {
            display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, unit, port);
            return CLI_NO_ERROR;
        }

#if(SYS_CPNT_NETACCESS == TRUE)
        if(NETACCESS_PMGR_DoDot1xReAuthenticate(l_port) != TRUE)
#else
        if(DOT1X_PMGR_Do_ReAuthenticate(l_port, NULL, 0)!= TRUE)
#endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to enable port period re-authentication of the client on port %lu/%lu.\r\n", (unsigned long)unit, (unsigned long)port);
#endif
        }
    }
    else
    {
        for (i=0; STKTPLG_POM_GetNextUnit(&i); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(i);

            for (j = 1; j <= max_port_num; j++)
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(i, j, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, i, j);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_DoDot1xReAuthenticate(l_port) != TRUE)
#else
                if(DOT1X_PMGR_Do_ReAuthenticate(l_port, NULL,0)!=TRUE)
#endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to enable port period re-authentication of the client on port %lu/%lu.\r\n", (unsigned long)i, (unsigned long)j);
#endif
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_Intrusionaction(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    UI32_T  i, action_state, max_port_num = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_INTRUSIONACTION:
        switch(arg[0][0])
        {
        case 'b':
        case 'B':
            action_state = VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic;
            break;
        case 'g':
        case 'G':
            action_state = VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan;
            break;
        default:
            return CLI_ERR_INTERNAL;
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_INTRUSIONACTION:
        action_state = DOT1X_DEFAULT_ACTION;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    for (i = 1; i <= max_port_num; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            UI32_T l_port;
            BOOL_T is_inherit = TRUE;

            if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
            {
                display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                continue;
            }

            if(NETACCESS_PMGR_SetDot1xPortIntrusionAction(l_port, action_state)!=TRUE)
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Fail to set the intrusion action on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
#endif
            }
        }
    }
#endif /* SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_DOT1X == TRUE)
static UI32_T show_one_port_dot1x(UI32_T line_num, UI32_T unit, UI32_T port, UI32_T l_port);
static UI32_T show_one_port_dot1x_statistics(UI32_T line_num, UI32_T unit, UI32_T port, UI32_T l_port);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

UI32_T CLI_API_Show_Dot1x(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T  line_num = 0;

#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  sys_ctrl_status;
    UI32_T  ctrl_mode;
    NETACCESS_MGR_Dot1XAuthControlledPortStatus_T author_status = NETACCESS_MGR_DOT1X_AUTH_CONTROLLED_PORT_STATUS_ERR;
    UI32_T  i, j, oper_mode;
    UI32_T  max_port_num = 0;
    char    port_status[15] = {0},auth[10] ={0},eth_name[10]={0};
    CLI_TBL_Object_T tb;
    int     rc;
    CLI_TBL_Temp_T summary_tbl[] =
    {
        {DOT1X_SUMMARY_TBL_PORT,       8, CLI_TBL_ALIGN_LEFT},
        {DOT1X_SUMMARY_TBL_TYPE,      13, CLI_TBL_ALIGN_LEFT},
        {DOT1X_SUMMARY_TBL_OPER_MODE, 14, CLI_TBL_ALIGN_LEFT},
        {DOT1X_SUMMARY_TBL_CTRL_MODE, 18, CLI_TBL_ALIGN_LEFT},
        {DOT1X_SUMMARY_TBL_AUTHOR,    10, CLI_TBL_ALIGN_LEFT},
    };
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    if(arg[0]==0)
    {
#if (SYS_CPNT_DOT1X == TRUE)
        sprintf(buff, "Global 802.1X Parameters:\r\n");
        PROCESS_MORE(buff);

        NETACCESS_PMGR_GetDot1xSystemAuthControl(&sys_ctrl_status);
        sprintf(buff, " System Auth Control       : %s\r\n",sys_ctrl_status == VAL_dot1xPaeSystemAuthControl_enabled ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);

        sprintf(buff, "\r\n");
        PROCESS_MORE(buff);

        sprintf(buff, "Authenticator Parameters:\r\n");
        PROCESS_MORE(buff);
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
        sprintf(buff, " EAPOL Pass Through        : %s\r\n", get_eapol_pass_thru() == EAPOL_PASS_THRU_ENABLE ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);
#endif /* SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH */

#if (SYS_CPNT_DOT1X == TRUE)
        sprintf(buff, "\r\n");
        PROCESS_MORE(buff);

        sprintf((char*)buff, "802.1X Port Summary\r\n");
        PROCESS_MORE(buff);

        sprintf(buff, "\r\n");
        PROCESS_MORE(buff);

        CLI_TBL_InitWithBuf(&tb, buff, sizeof(buff));
        CLI_TBL_SetColIndirect(&tb, summary_tbl, sizeof(summary_tbl)/sizeof(summary_tbl[0]));
        CLI_TBL_SetLineNum(&tb, line_num);

        CLI_TBL_SetColTitle(&tb, DOT1X_SUMMARY_TBL_PORT,        "Port");
        CLI_TBL_SetColTitle(&tb, DOT1X_SUMMARY_TBL_TYPE,        "Type");
        CLI_TBL_SetColTitle(&tb, DOT1X_SUMMARY_TBL_OPER_MODE,   "Operation Mode");
        CLI_TBL_SetColTitle(&tb, DOT1X_SUMMARY_TBL_CTRL_MODE,   "Control Mode");
        CLI_TBL_SetColTitle(&tb, DOT1X_SUMMARY_TBL_AUTHOR,      "Authorized");
        CLI_TBL_Print(&tb);

        CLI_TBL_SetLine(&tb);
        CLI_TBL_Print(&tb);

        for (i=0; STKTPLG_POM_GetNextUnit(&i); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(i);
            for (j = 1; j <= max_port_num; j++)
            {
                UI32_T l_port;
                UI32_T mode;
                BOOL_T is_inherit = TRUE;

                if (   (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(i, j, &l_port, &is_inherit))
                    || (FALSE == NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &mode))
                    )
                {
                    continue;
                }

                NETACCESS_PMGR_GetDot1xPortControlMode(l_port, &ctrl_mode);
                if ((sys_ctrl_status == VAL_dot1xPaeSystemAuthControl_enabled) &&
                    (ctrl_mode != VAL_dot1xAuthAuthControlledPortControl_forceAuthorized))
                {
                    strcpy(port_status,"Authenticator");
                }
                else
                {
                    strcpy(port_status,"Disabled");
                }
                NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &oper_mode);

                if(SWCTRL_POM_isPortLinkUp(l_port)==TRUE)
                {
                    NETACCESS_PMGR_GetDot1xPortAuthorized(l_port, &author_status);

                    if(author_status == VAL_dot1xAuthAuthControlledPortStatus_unauthorized)
                    {
                        strcpy((char*)auth,"No");
                    }
                    else if(author_status == VAL_dot1xAuthAuthControlledPortStatus_authorized)
                    {
                        strcpy((char*)auth,"Yes");
                    }
                }
                else
                {
                    strcpy((char*)auth,"N/A");
                }

                sprintf(eth_name,"Eth %1lu/%2lu",(unsigned long)i,(unsigned long)j);

                CLI_TBL_SetColText(&tb, DOT1X_SUMMARY_TBL_PORT, eth_name);
                CLI_TBL_SetColText(&tb, DOT1X_SUMMARY_TBL_TYPE, port_status);
                CLI_TBL_SetColText(&tb, DOT1X_SUMMARY_TBL_OPER_MODE, operation_mode_str(oper_mode));
                CLI_TBL_SetColText(&tb, DOT1X_SUMMARY_TBL_CTRL_MODE, port_control_str(ctrl_mode));
                CLI_TBL_SetColText(&tb, DOT1X_SUMMARY_TBL_AUTHOR, auth);

                rc = CLI_TBL_Print(&tb);
                if (CLI_TBL_PRINT_RC_SUCCESS != rc)
                {
                    return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
                }
            }
        }

        line_num = CLI_TBL_GetLineNum(&tb);

        sprintf(buff, "\r\n");
        PROCESS_MORE(buff);

        sprintf((char*)buff, "802.1X Port Details\r\n");
        PROCESS_MORE(buff);

        for (i=0; STKTPLG_POM_GetNextUnit(&i); )
        {
            for (j = 1; j <= max_port_num; j++)
            {
                UI32_T l_port;
                UI32_T mode;
                BOOL_T is_inherit = TRUE;

                if (   (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(i, j, &l_port, &is_inherit))
                    || (FALSE == NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &mode))
                    )
                {
                    continue;
                }

                line_num = show_one_port_dot1x(line_num, i, j, l_port);
                if (JUMP_OUT_MORE == line_num)
                {
                    return CLI_NO_ERROR;
                }
                else if (EXIT_SESSION_MORE == line_num)
                {
                    return CLI_EXIT_SESSION;
                }
            }
        }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */
    }
#if (SYS_CPNT_DOT1X == TRUE)
    else if(arg[0][0]=='i' || arg[0][0]=='I')
    {
        UI32_T unit;
        UI32_T port;
        UI32_T l_port;
        UI32_T mode;
        BOOL_T is_inherit = TRUE;

        unit = atoi((char*)arg[2]);
        port = atoi(strchr((char*)arg[2], '/') + 1);

        if (   (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(unit, port, &l_port, &is_inherit))
            || (FALSE == NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &mode))
            )
        {
            display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, unit, port);
            return CLI_NO_ERROR;
        }

        line_num = show_one_port_dot1x(line_num, unit, port, l_port);
        if (JUMP_OUT_MORE == line_num)
        {
            return CLI_NO_ERROR;
        }
        else if (EXIT_SESSION_MORE == line_num)
        {
            return CLI_EXIT_SESSION;
        }
    }
    else if(arg[0][0]=='s' || arg[0][0]=='S')
    {
        if(arg[1]==0)
        {
            for (i=0; STKTPLG_POM_GetNextUnit(&i); )
            {
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(i);
                for (j = 1; j <= max_port_num; j++)
                {
                    UI32_T l_port;
                    UI32_T mode;
                    BOOL_T is_inherit = TRUE;

                    if (   (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(i, j, &l_port, &is_inherit))
                        || (FALSE == NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &mode))
                        )
                    {
                        continue;
                    }

                    line_num = show_one_port_dot1x_statistics(line_num, i, j, l_port);
                    if (JUMP_OUT_MORE == line_num)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (EXIT_SESSION_MORE == line_num)
                    {
                        return CLI_EXIT_SESSION;
                    }
                }
            }
        }
        else
        {
            UI32_T unit;
            UI32_T port;
            UI32_T l_port;
            UI32_T mode;
            BOOL_T is_inherit = TRUE;

            unit = atoi((char*)arg[3]);
            port = atoi(strchr((char*)arg[3], '/') + 1);

            if (   (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(unit, port, &l_port, &is_inherit))
                || (FALSE == NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &mode))
                )
            {
                display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, unit, port);
                return CLI_NO_ERROR;
            }

            line_num = show_one_port_dot1x_statistics(line_num, unit, port, l_port);
            if(JUMP_OUT_MORE == line_num)
            {
                return CLI_NO_ERROR;
            }
            else if (EXIT_SESSION_MORE == line_num)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_DOT1X == TRUE)
static char *apsm_state_str(UI32_T apsm_state)
{
    static char *string_array[] =
        {
        "Unknown",
        "Initialize",
        "Disconnected",
        "Connecting",
        "Authenticating",
        "Authenticated",
        "Aborting",
        "Held",
        "Force-Auth",
        "Force-Unauth"
        }
        ;

    if (VAL_dot1xAuthPaeState_initialize <= apsm_state
        && apsm_state <= VAL_dot1xAuthPaeState_forceUnauth
        )
    {
        return string_array[apsm_state];
    }

    return string_array[0];
}

static char *basm_state_str(UI32_T basm_state)
{
    static char *string_array[] =
        {
        "Unknown",
        "Request",
        "Response",
        "Success",
        "Fail",
        "Timeout",
        "Idle",
        "Initialize",
        }
        ;

    if (VAL_dot1xAuthBackendAuthState_request <= basm_state
        && basm_state <= VAL_dot1xAuthBackendAuthState_initialize
        )
    {
        return string_array[basm_state];
    }

    return string_array[0];
}

static char *resm_state_str(UI32_T resm_state)
{
    static char *string_array[] =
        {
        "Initialize",
        "Reauthenticate",
        }
        ;

    if (resm_Initialize <= resm_state
        && resm_state <= resm_Reauthenticate
        )
    {
        return string_array[resm_state];
    }

    return string_array[0];
}

static char *operation_mode_str(UI32_T oper_mode)
{
    static char *string_array[] =
    {
        "Unknown",
        "Single-Host",
        "Multi-Host",
        "MAC-Based"
    };

    if (DOT1X_PORT_OPERATION_MODE_ONEPASS <= oper_mode
    #if (SYS_CPNT_DOT1X_MACBASED_AUTH == TRUE)
        && oper_mode <= DOT1X_PORT_OPERATION_MODE_MACBASED
    #else
        && oper_mode <= DOT1X_PORT_OPERATION_MODE_MULTIPASS
    #endif
        )
    {
        return string_array[oper_mode];
    }

    return string_array[0];
}

static char *port_control_str(UI32_T ctrl_mode)
{
    static char *string_array[] =
    {
        "Unknown",
        "Force-Unauthorized",
        "Auto",
        "Force-Authorized"
    };

    if (VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized <= ctrl_mode
        && ctrl_mode <= VAL_dot1xAuthAuthControlledPortControl_forceAuthorized
        )
    {
        return string_array[ctrl_mode];
    }

    return string_array[0];
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
static char *intrusion_action_str(UI32_T action)
{
    static char *string_array[] =
    {
        "Unknown",
        "Block traffic",
        "Guest VLAN",
    };

    if (VAL_dot1xAuthConfigExtPortIntrusionAction_block_traffic <= action
        && action <= VAL_dot1xAuthConfigExtPortIntrusionAction_guest_vlan
        )
    {
        return string_array[action];
    }

    return string_array[0];
}
#endif  /* #if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE) */

#if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE)
static CLI_API_1X_Dot1XEapolPassThru_T get_eapol_pass_thru()
{
    UI32_T status = EAPOL_PASS_THRU_UNKNOWN;

#if (SYS_CPNT_NETACCESS == TRUE)
    if (NETACCESS_PMGR_GetDot1xEapolPassThrough(&status) == FALSE)
    {
        return EAPOL_PASS_THRU_UNKNOWN;
    }
#endif

    return (status == VAL_dot1xEapolPassThrough_enabled) ? EAPOL_PASS_THRU_ENABLE :
        (status == VAL_dot1xEapolPassThrough_disabled) ? EAPOL_PASS_THRU_DISABLE :
        EAPOL_PASS_THRU_UNKNOWN;
}
#endif /* #if (SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
static UI32_T show_one_port_dot1x(UI32_T line_num, UI32_T unit, UI32_T port, UI32_T l_port)
{
#if(SYS_CPNT_NETACCESS == TRUE)

    #if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
    UI32_T  action_state;
    #endif

    UI32_T  ctrl_mode, sys_ctrl_status;
#endif
    UI32_T  index;
    DOT1X_PortDetails_T port_details;

    UI32_T  Max_Req=0,Quiet_Period=0,ReAuth_Period=0,Tx_Period=0;
    UI32_T  Re_Auth = 0;
    UI32_T  ReAuth_Max = 0;
    UI32_T  Supp_timeout = 0;
    UI32_T  Server_timeout = 0;
    UI32_T  multihost_count = 0;
    UI32_T  oper_mode;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    NETACCESS_PMGR_GetDot1xSystemAuthControl(&sys_ctrl_status);
    NETACCESS_PMGR_GetDot1xPortControlMode(l_port, &ctrl_mode);
    if ((sys_ctrl_status == VAL_dot1xPaeSystemAuthControl_disabled) ||
        (ctrl_mode == VAL_dot1xAuthAuthControlledPortControl_forceAuthorized))
    {
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "802.1X Authenticator is disabled on port %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "802.1X Authenticator is enabled on port %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
        PROCESS_MORE_FUNC(buff);

        NETACCESS_PMGR_GetDot1xPortOperationMode(l_port, &oper_mode);
        NETACCESS_PMGR_GetDot1xPortMultiHostMacCount(l_port, &multihost_count);
        NETACCESS_PMGR_GetDot1xPortMaxReq(l_port, &Max_Req);
        NETACCESS_PMGR_GetDot1xPortQuietPeriod(l_port, &Quiet_Period);
        NETACCESS_PMGR_GetDot1xPortReAuthPeriod(l_port, &ReAuth_Period);
        NETACCESS_PMGR_GetDot1xPortTxPeriod(l_port, &Tx_Period);
        NETACCESS_PMGR_GetDot1xPortReAuthEnabled(l_port, &Re_Auth);
        NETACCESS_PMGR_GetDot1xPortAuthSuppTimeout(l_port, &Supp_timeout);
        NETACCESS_PMGR_GetDot1xPortAuthServerTimeout(l_port, &Server_timeout);
        NETACCESS_PMGR_GetDot1xPortReAuthMax(l_port, &ReAuth_Max);

        sprintf((char*)buff, " Reauthentication     : %s\r\n", Re_Auth ==  VAL_dot1xPaePortReauthenticate_true ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Reauth Period        : %lu seconds\r\n", (unsigned long)ReAuth_Period);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Quiet Period         : %lu seconds\r\n", (unsigned long)Quiet_Period);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " TX Period            : %lu seconds\r\n", (unsigned long)Tx_Period);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Supplicant Timeout   : %lu seconds\r\n", (unsigned long)Supp_timeout);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Server Timeout       : %lu seconds\r\n", (unsigned long)Server_timeout);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Reauth Max Retries   : %lu\r\n", (unsigned long)ReAuth_Max);
        PROCESS_MORE(buff);
        sprintf((char*)buff, " Max Request          : %lu\r\n",(unsigned long)Max_Req);
        PROCESS_MORE(buff);
        sprintf(buff, " Operation Mode       : %s\r\n", operation_mode_str(oper_mode));
        PROCESS_MORE(buff);
        sprintf(buff, " Port Control         : %s\r\n", port_control_str(ctrl_mode));
        PROCESS_MORE(buff);

        if(DOT1X_PORT_OPERATION_MODE_MULTIPASS == oper_mode)
        {
            sprintf(buff, " Maximum MAC Count    : %lu\r\n", (unsigned long)multihost_count);
            PROCESS_MORE(buff);
        }
#if (SYS_CPNT_NETACCESS_GUEST_VLAN == TRUE)
        NETACCESS_PMGR_GetDot1xPortIntrusionAction(l_port, &action_state);
        sprintf(buff, " Intrusion Action     : %s\r\n", intrusion_action_str(action_state));
        PROCESS_MORE(buff);
#endif

        index = 0;
        while (DOT1X_POM_GetNextPortDetails(l_port, &index, &port_details))
        {
            sprintf(buff, "\r\n");
            PROCESS_MORE_FUNC(buff);

            if (DOT1X_PORT_OPERATION_MODE_ONEPASS == oper_mode
                || DOT1X_PORT_OPERATION_MODE_MULTIPASS == oper_mode
                )
            {
                sprintf(buff, "Supplicant            : %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    port_details.supplicant[0],
                    port_details.supplicant[1],
                    port_details.supplicant[2],
                    port_details.supplicant[3],
                    port_details.supplicant[4],
                    port_details.supplicant[5]
                    );
                PROCESS_MORE_FUNC(buff);
                sprintf(buff, "\r\n");
            }
            else
            {
                sprintf(buff, "Supplicant %-2lu         : %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    (unsigned long)index,
                    port_details.supplicant[0],
                    port_details.supplicant[1],
                    port_details.supplicant[2],
                    port_details.supplicant[3],
                    port_details.supplicant[4],
                    port_details.supplicant[5]
                    );
            }
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, " Authenticator PAE State Machine\r\n");
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  State               : %s\r\n", apsm_state_str(port_details.apsm_state));
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  Reauth Count        : %lu\r\n", (unsigned long)port_details.reauth_count);
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  Current Identifier  : %lu\r\n", (unsigned long)port_details.current_id);
            PROCESS_MORE_FUNC(buff);

            PROCESS_MORE_FUNC("\r\n");

            sprintf(buff, " Backend State Machine\r\n");
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  State               : %s\r\n", basm_state_str(port_details.basm_state));
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  Request Count       : %lu\r\n", (unsigned long)port_details.request_count);
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  Identifier (Server) : %lu\r\n", (unsigned long)port_details.identifier);
            PROCESS_MORE_FUNC(buff);


            PROCESS_MORE_FUNC("\r\n");

            sprintf(buff, " Reauthentication State Machine\r\n");
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "  State               : %s\r\n", resm_state_str(port_details.resm_state));
            PROCESS_MORE_FUNC(buff);
        }

    }

    return line_num;
}

static UI32_T show_one_port_dot1x_statistics(UI32_T line_num, UI32_T unit, UI32_T port, UI32_T l_port)
{
#if(SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PortMode_T                 secure_port_mode;
#endif
    DOT1X_AuthStatsEntry_T AuthStateEntry;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

#if(SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PMGR_GetSecurePortMode(l_port, &secure_port_mode);
    if(secure_port_mode != NETACCESS_PORTMODE_DOT1X)
#else
    if(DOT1X_POM_Get_Port_Status(l_port) == FALSE)
#endif
    {
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "802.1X is disabled on port %1lu/%2lu\r\n", (unsigned long)unit, (unsigned long)port);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "Eth %lu/%lu\r\n",(unsigned long)unit,(unsigned long)port);
        PROCESS_MORE_FUNC(buff);

#if(SYS_CPNT_NETACCESS == TRUE)
        NETACCESS_PMGR_GetDot1xAuthStatsEntry(l_port, &AuthStateEntry);
#else
        DOT1X_POM_Auth_Stats_Table(l_port,&AuthStateEntry);
#endif

        sprintf((char*)buff, "Rx: EAPOL      EAPOL      EAPOL      EAPOL      EAP      EAP      EAP\r\n");
        PROCESS_MORE_FUNC(buff);
        sprintf((char*)buff, "    Start      Logoff    Invalid     Total    Resp/ID  Resp/Oth LenError\r\n");
        PROCESS_MORE_FUNC(buff);

        sprintf((char*)buff, "%9lu%11lu%11lu%11lu%9lu%9lu%9lu\r\n",(unsigned long)AuthStateEntry.dot1xAuthEapolStartFramesRx,
                                                            (unsigned long)AuthStateEntry.dot1xAuthEapolLogoffFramesRx,
                                                            (unsigned long)AuthStateEntry.dot1xAuthInvalidEapolFramesRx,
                                                            (unsigned long)AuthStateEntry.dot1xAuthEapolFramesRx,
                                                            (unsigned long)AuthStateEntry.dot1xAuthEapolRespIdFramesRx,
                                                            (unsigned long)AuthStateEntry.dot1xAuthEapolRepsFramesRx,
                                                           (unsigned long) AuthStateEntry.dot1xAuthEapLengthErrorFramesRx);
        PROCESS_MORE_FUNC(buff);
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "    Last      Last\r\n");
        PROCESS_MORE_FUNC(buff);
        sprintf((char*)buff, "EAPOLVer     EAPOLSrc\r\n");
        PROCESS_MORE_FUNC(buff);
        sprintf((char*)buff, "%8lu     %02X-%02X-%02X-%02X-%02X-%02X\r\n",(unsigned long)AuthStateEntry.dot1xAuthLastEapolFrameVersion,
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[0],
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[1],
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[2],
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[3],
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[4],
                                    AuthStateEntry.dot1xAuthLastEapolFramesSource[5]);
        PROCESS_MORE_FUNC(buff);
        PROCESS_MORE_FUNC(("\r\n"));
        sprintf((char*)buff, "Tx: EAPOL      EAP      EAP\r\n");
        PROCESS_MORE_FUNC(buff);
        sprintf((char*)buff, "    Total     Req/ID   Req/Oth\r\n");
        PROCESS_MORE_FUNC(buff);
        sprintf((char*)buff, "%9lu%9lu%9lu\r\n",(unsigned long)AuthStateEntry.dot1xAuthEapolFramesTx,(unsigned long)AuthStateEntry.dot1xAuthEapolReqIdFramesTx,(unsigned long)AuthStateEntry.dot1xAuthEapolReqFramesTx);
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

UI32_T CLI_API_Dot1x_Operation_Mode_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T  i, set_mode = 0, counter = 0;
    UI32_T  max_port_num = 0, counter_flag = 0;

    max_port_num = SWCTRL_POM_UIGetUnitPortNumber(ctrl_P->CMenu.unit_id);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_DOT1X_OPERATIONMODE:
        if((arg[0][0] == 's') || (arg[0][0] == 'S'))
        {
            /* single mode */
            set_mode = DOT1X_PORT_OPERATION_MODE_ONEPASS;
        }
        else if((arg[0][0] == 'm') || (arg[0][0] == 'M'))
        {

#if (SYS_CPNT_DOT1X_MACBASED_AUTH == TRUE)
            if((arg[0][1] == 'u') || (arg[0][1] == 'U'))
            {
                    counter_flag = 1;
                /* multi mode */
                    if(arg[1] != NULL)
                    {
                        counter = (UI32_T)atoi((char*)arg[2]);
                    }
                    else
                    {
                        counter = DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT;
                    }
                set_mode = DOT1X_PORT_OPERATION_MODE_MULTIPASS;
            }
            else if ((arg[0][1] == 'a') || (arg[0][1] == 'A'))
            {
                /* MAC based mode */
                set_mode = DOT1X_PORT_OPERATION_MODE_MACBASED;
            }
#else
            /* multi mode */
                /* with max-count */
                counter_flag = 1;
                if(arg[1] != NULL)
                {
                    counter = (UI32_T)atoi((char*)arg[2]);
                }
                else
                {
                    counter = DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT;
                }

                set_mode = DOT1X_PORT_OPERATION_MODE_MULTIPASS;
#endif
        }
        else
        {
             CLI_LIB_PrintStr("Failed to set operation mode on ports.\r\n");
            return CLI_ERR_CMD_INVALID;
        }

        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortOperationMode(l_port, set_mode) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortOperationMode(l_port, set_mode) != TRUE)
#endif
                {
                    CLI_LIB_PrintStr_2("Failed to set operation mode on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
                }
                if(counter_flag == 1)
                {
#if(SYS_CPNT_NETACCESS == TRUE)
                    if(NETACCESS_PMGR_SetDot1xPortMultiHostMacCount(l_port, counter) != TRUE)
#else
                    if(DOT1X_PMGR_Set_PortMultiHostMacCount(l_port, counter) != TRUE)
#endif
                    {
                            CLI_LIB_PrintStr_2("Failed to set max MAC number on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
                    }
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_DOT1X_OPERATIONMODE:
        /* set to default value */
        set_mode = DOT1X_DEFAULT_PORT_OPERATION_MODE;
        if(arg[1] != NULL)
        {
            counter_flag = 1;
            counter = DOT1X_DEFAULT_MULTI_HOST_MAC_COUNT;
        }

        for (i = 1; i <= max_port_num; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                UI32_T l_port;
                BOOL_T is_inherit = TRUE;

                if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToIfindex(ctrl_P->CMenu.unit_id, i, &l_port, &is_inherit))
                {
                    display_ethernet_msg(CLI_API_ETH_UNKNOWN_PORT, ctrl_P->CMenu.unit_id, i);
                    continue;
                }

#if(SYS_CPNT_NETACCESS == TRUE)
                if(NETACCESS_PMGR_SetDot1xPortOperationMode(l_port, set_mode) != TRUE)
#else
                if(DOT1X_PMGR_Set_PortOperationMode(l_port, set_mode) != TRUE)
#endif
                {
                    CLI_LIB_PrintStr_2("Failed to restore operation mode on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
                }
                if(counter_flag == 1)
                {
#if(SYS_CPNT_NETACCESS == TRUE)
                    if(NETACCESS_PMGR_SetDot1xPortMultiHostMacCount(l_port, counter) != TRUE)
#else
                    if(DOT1X_PMGR_Set_PortMultiHostMacCount(l_port, counter) != TRUE)
#endif
                    {
                            CLI_LIB_PrintStr_2("Failed to set max MAC number on port %lu/%lu.\r\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)i);
                    }
                }
            }
        }
        break;

   default:
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Dot1x_SYSTEMTAUTHCTRL(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_API_Dot1x_SystemAuthControl(cmd_idx, arg, ctrl_P);
}

UI32_T CLI_API_Dot1x_SystemAuthControl(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DOT1X == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_DOT1X_SYSTEMAUTHCONTROL:
#if(SYS_CPNT_NETACCESS == TRUE)
        if(NETACCESS_PMGR_SetDot1xSystemAuthControl(VAL_dot1xPaeSystemAuthControl_enabled)!=TRUE)
#else
        if(DOT1X_PMGR_Set_SystemAuthControl(VAL_dot1xPaeSystemAuthControl_enabled)!=TRUE)
#endif
             CLI_LIB_PrintStr("Failed to enable 802.1x globally.\r\n");
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_DOT1X_SYSTEMAUTHCONTROL:
#if(SYS_CPNT_NETACCESS == TRUE)
        if(NETACCESS_PMGR_SetDot1xSystemAuthControl(VAL_dot1xPaeSystemAuthControl_disabled)!=TRUE)
#else
        if(DOT1X_PMGR_Set_SystemAuthControl(VAL_dot1xPaeSystemAuthControl_disabled)!=TRUE)
#endif
             CLI_LIB_PrintStr("Failed to disable 802.1x globally.\r\n");
        break;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return CLI_NO_ERROR;
}

