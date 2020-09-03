#include "cli_api.h"
#include "cli_api_pvlan_isol.h"

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION)
static BOOL_T fill_eth_lport_list(char *unit_port_str, UI8_T *port_list,UI8_T *com_port_list,BOOL_T if_disable);
static BOOL_T fill_trunk_lport_list(char *trunk_str, UI8_T *port_list,UI8_T *com_port_list,BOOL_T if_disable);
#endif
#endif

UI32_T CLI_API_Pvlan_Isol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION)

   UI8_T uplink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
   UI8_T downlink_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

   /*set up-link and down-link group of pvlan*/
   memset(uplink_port_list, 0, sizeof(uplink_port_list));
   memset(downlink_port_list, 0, sizeof(downlink_port_list));

   /*get now port_list*/
   memcpy(uplink_port_list,ctrl_P->CMenu.uplink_port_list,sizeof(uplink_port_list));
   memcpy(downlink_port_list,ctrl_P->CMenu.downlink_port_list,sizeof(downlink_port_list));

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W1_TRAFFICSEGMENTATION:
      if(arg[0] == NULL)
      {
         /*enable pvlan*/
         if(!SWCTRL_PMGR_EnablePrivateVlan())
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to enable traffic segmentation.\r\n");
#endif
         }
      }
      else
      {
         UI8_T downlink_pos;
         BOOL_T if_disable=0;

         /*up-link*/
         if(arg[1][0] == 'e' || arg[1][0] == 'E')
         {
            fill_eth_lport_list(arg[2], uplink_port_list,downlink_port_list,if_disable);

            if(arg[3] == 0)
                break;

            if(arg[3][0] == 'p' || arg[3][0] == 'P')
            {
               fill_trunk_lport_list(arg[4], uplink_port_list,downlink_port_list,if_disable);
               downlink_pos = 5;
            }
            else
            {
               downlink_pos = 3;
            }
         }
         else
         {
            fill_trunk_lport_list(arg[2], uplink_port_list,downlink_port_list,if_disable);
            downlink_pos = 3;
         }

         if (arg[downlink_pos]== 0)
            break;

         /*down-link*/
         if(arg[downlink_pos+1][0] == 'e' || arg[downlink_pos+1][0] == 'E')
         {
            fill_eth_lport_list(arg[downlink_pos+2], downlink_port_list,uplink_port_list,if_disable);

            if(arg[downlink_pos + 3])
            {
               fill_trunk_lport_list(arg[downlink_pos+4], downlink_port_list,uplink_port_list,if_disable);
            }
         }
         else
         {
            fill_trunk_lport_list(arg[downlink_pos+2], downlink_port_list,uplink_port_list,if_disable);
         }
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_TRAFFICSEGMENTATION:
      if(arg[0] == NULL)
      {
          /*disable pvlan*/
          if(!SWCTRL_PMGR_DisablePrivateVlan())
          {
#if (SYS_CPNT_EH == TRUE)
              CLI_API_Show_Exception_Handeler_Msg();
              return CLI_ERR_INTERNAL;
#else
              CLI_LIB_PrintStr("Failed to disable traffic segmentation.\r\n");
              return CLI_ERR_INTERNAL;
#endif
          }
          else
          {
              return CLI_NO_ERROR;
          }    
      }
      else
      {
         UI8_T downlink_pos;
         BOOL_T if_disable=1;

         /*up-link*/
         if(arg[1][0] == 'e' || arg[1][0] == 'E')
         {
            fill_eth_lport_list(arg[2], uplink_port_list,downlink_port_list,if_disable);

            if(arg[3] == 0)
                break;

            if(arg[3][0] == 'p' || arg[3][0] == 'P')
            {
               fill_trunk_lport_list(arg[4], uplink_port_list,downlink_port_list,if_disable);
               downlink_pos = 5;
            }
            else
            {
               downlink_pos = 3;
            }
         }
         else
         {
            fill_trunk_lport_list(arg[2], uplink_port_list,downlink_port_list,if_disable);
            downlink_pos = 3;
         }

         if (arg[downlink_pos] == 0)
            break;

         /*down-link*/
         if(arg[downlink_pos+1][0] == 'e' || arg[downlink_pos+1][0] == 'E')
         {
            fill_eth_lport_list(arg[downlink_pos+2], downlink_port_list,uplink_port_list,if_disable);

            if(arg[downlink_pos + 3])
            {
               fill_trunk_lport_list(arg[downlink_pos+4], downlink_port_list,uplink_port_list,if_disable);
            }
         }
         else
         {
            fill_trunk_lport_list(arg[downlink_pos+2], downlink_port_list,uplink_port_list,if_disable);
         }
      }
      break;
    }

    /*action*/
    if(!SWCTRL_PMGR_SetPrivateVlan(uplink_port_list, downlink_port_list))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set traffic segmentation.\r\n");
#endif
    }
    else
    {
        /*keep the port_list in working area*/
        memcpy(ctrl_P->CMenu.uplink_port_list,uplink_port_list,sizeof(uplink_port_list));
        memcpy(ctrl_P->CMenu.downlink_port_list,downlink_port_list,sizeof(downlink_port_list));
    }

#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION) */
#endif
   return CLI_NO_ERROR;
}

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION)

static BOOL_T fill_eth_lport_list(char *unit_port_str, UI8_T *port_list,UI8_T *com_port_list,BOOL_T if_disable)
{
   UI32_T unit = atoi(unit_port_str);
   UI32_T port;
   char  Token[CLI_DEF_MAX_BUFSIZE];
   char  *op_ptr;
   char  delemiters[2] = {0};
   UI32_T lower_val;
   UI32_T upper_val;
   UI32_T err_idx;
   UI32_T lport;
   BOOL_T is_fail = FALSE;

   op_ptr = strchr(unit_port_str, '/') + 1;
   delemiters[0] = ',';

   do
   {
      memset(Token, 0, sizeof(Token));
      op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

      if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
         return FALSE;
      else
      {
         for(port = lower_val; port<=upper_val; port++)
         {
            CLI_API_EthStatus_T verify_ret;

            if( (verify_ret = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, unit, port);
               is_fail = TRUE;
            }
            else
            {
                /*enable pvlan_list*/
                if(if_disable==0)
                {
                    /*set the port was in the com_port_list, we have to take it away from com_port_list.*/
                    if( (port_list[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8)))) &&  ( com_port_list[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8)))) == TRUE )
                    {
                        com_port_list[(UI32_T)((lport-1)/8)]  &= (~ (1 << ( 7 - ((lport-1)%8))));
                    }
                    /*add port we want to set to port_list*/
                    port_list[(UI32_T)((lport-1)/8)]  |= (1 << ( 7 - ((lport-1)%8)));
                }
                /*disable pvlan_list,and we don't care that the port was in the com_port_list*/
                else
                {
                    port_list[(UI32_T)((lport-1)/8)]  &= (~ (1 << ( 7 - ((lport-1)%8))));
                }
            }
         }/*end for*/
      }
   } while(op_ptr != 0 && !isspace(*op_ptr));

   if(is_fail)
      return FALSE;
   else
      return TRUE;
}

static BOOL_T fill_trunk_lport_list(char *trunk_str, UI8_T *port_list,UI8_T *com_port_list,BOOL_T if_disable)
{
   UI32_T trunk_id;

   char  Token[CLI_DEF_MAX_BUFSIZE];
   char  *op_ptr;
   char  delemiters[2] = {0};
   UI32_T lower_val;
   UI32_T upper_val;
   UI32_T err_idx;
   UI32_T lport;
   BOOL_T is_fail = FALSE;

   op_ptr = trunk_str;
   delemiters[0] = ',';

   do
   {
      memset(Token, 0, sizeof(Token));
      op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

      if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
         return FALSE;
      else
      {
         for(trunk_id = lower_val; trunk_id<=upper_val; trunk_id++)
         {
            CLI_API_TrunkStatus_T verify_ret;

            if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
               display_trunk_msg(verify_ret, trunk_id);
               is_fail = TRUE;
            }
            else
            {
                /*enable pvlan_list*/
                if(if_disable==0)
                {
                    if( (port_list[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8)))) &&  ( com_port_list[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8)))) == TRUE )
                    /*set the port was in the com_port_list, we have to take it away from com_port_list.*/
                    {
                        com_port_list[(UI32_T)((lport-1)/8)]  &= (~ (1 << ( 7 - ((lport-1)%8))));
                    }
                    /*add port we want to set to port_list*/
                    port_list[(UI32_T)((lport-1)/8)]  |= (1 << ( 7 - ((lport-1)%8)));
                }
                /*disable pvlan_list,and we don't care that the port was in the com_port_list*/
                else
                {
                    port_list[(UI32_T)((lport-1)/8)]  &= (~ (1 << ( 7 - ((lport-1)%8))));
                }
            }
         }/*end for*/
      }
   } while(op_ptr != 0 && !isspace(*op_ptr));

   if(is_fail)
      return FALSE;
   else
      return TRUE;
}
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION) */
#endif
/*
typedef struct
{
    UI32_T  vlan_status;
    UI8_T   uplink_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   downlink_ports[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} SWCTRL_PrivateVlan_T;
 */

UI32_T CLI_API_Show_Pvlan_Isol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION)
   UI32_T lport;
   UI32_T unit;
   UI32_T port;
   UI32_T trunk_id;


   SWCTRL_PrivateVlan_T private_vlan;

   memset(&private_vlan, 0, sizeof(SWCTRL_PrivateVlan_T));

   if(!SWCTRL_POM_GetPrivateVlan(&private_vlan))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get traffic segmentation status.\r\n");
#endif
      return CLI_NO_ERROR;
   }

   /*private-VLAN status*/
   CLI_LIB_PrintStr_1("Private VLAN status: %s\r\n", (private_vlan.vlan_status == VAL_privateVlanStatus_enabled) ? "Enabled" : "Disabled");

   /*up-link*/
     CLI_LIB_PrintStr("Up-link Port:        \r\n");
   for(lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
   {
      if (private_vlan.uplink_ports[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8))) )
      {
         SWCTRL_Lport_Type_T ret;

         ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

         switch(ret)
         {
         case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
               CLI_LIB_PrintStr_1(" Ethernet %s\r\n", name);
            }
#else
            CLI_LIB_PrintStr_2(" Ethernet %lu/%lu\r\n", unit, port);
#endif
            break;

         case SWCTRL_LPORT_TRUNK_PORT:
            CLI_LIB_PrintStr_1(" Trunk %lu\r\n", trunk_id);
            break;

         default:
           CLI_LIB_PrintStr(" Invalid lport got from SWCTRL.\r\n");
         }
      }
   }

   /*down-link*/
   CLI_LIB_PrintStr("Down-link Port:      \r\n");
   for(lport = SYS_ADPT_ETHER_1_IF_INDEX_NUMBER; lport<= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
   {
      if (private_vlan.downlink_ports[(UI32_T)((lport-1)/8)]  & (1 << ( 7 - ((lport-1)%8))) )
      {
         SWCTRL_Lport_Type_T ret;

         ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

         switch(ret)
         {
         case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
               CLI_LIB_PrintStr_1(" Ethernet %s\r\n", name);
            }
#else
            CLI_LIB_PrintStr_2(" Ethernet %lu/%lu\r\n", unit, port);
#endif
            break;

         case SWCTRL_LPORT_TRUNK_PORT:
            CLI_LIB_PrintStr_1(" Trunk %lu\r\n", trunk_id);
            break;

         default:
            CLI_LIB_PrintStr(" Invalid lport got from SWCTRL.\r\n");
         }
      }
   }
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE == SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION) */
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Protected_Isol(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)

    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T port_private_mode;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_PROTECTED:
        port_private_mode = VAL_pvePortStatus_enabled;
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_PROTECTED:
        port_private_mode = VAL_pvePortStatus_disabled;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!SWCTRL_PMGR_SetPortPrivateMode(ifindex, port_private_mode))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set protected port on ethernet %s\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set protected port on ethernet %lu/%lu\r\n", verify_unit, verify_port);
#endif
#endif
            }
        }
    }

#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */


    return CLI_NO_ERROR;
    
}

