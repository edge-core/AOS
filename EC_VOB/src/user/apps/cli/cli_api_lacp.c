#include "cli_api.h"
#include "cli_api_lacp.h"
#include "sys_cpnt.h"
#include <stdio.h>
/************************************<<LACP>>*****************************/
static UI32_T CLI_API_Show_Lacp_Entry_By_One_Counters(LACP_MGR_Dot3adAggPortStatsEntry_T data, UI32_T line_num);
static UI32_T CLI_API_Show_Lacp_Entry_By_One_Internal(LACP_MGR_Dot3adAggPortEntry_T data, UI32_T line_num);
static UI32_T CLI_API_Show_Lacp_Entry_By_One_Neighbors(LACP_MGR_Dot3adAggPortEntry_T data, UI32_T line_num);

UI32_T CLI_API_Lacp_Actor_System_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI16_T priority   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_LACP_SYSTEMPRIORITY:
      priority = atoi(arg[0]);
      if (LACP_PMGR_SetDot3adAggActorSystemPriorityForAll(priority)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr_1("Failed to set LACP actor system priority %d\r\n", priority);
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LACP_SYSTEMPRIORITY:
      priority = 0x8000;/*LACP_SYSTEM_DEFAULT_PRIORITY*/
      if (LACP_PMGR_SetDot3adAggActorSystemPriorityForAll(priority)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to set default LACP actor system priority\r\n");
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Actor_Admin_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI16_T admin_key   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_LACP_ADMINKEY:
      admin_key = atoi(arg[0]);
      if (LACP_PMGR_SetDot3adAggActorAdminKeyForAll(admin_key)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr_1("Failed to set LACP actor admin key %d\r\n", admin_key);
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LACP_ADMINKEY:
      admin_key = 1;/*LACP_DEFAULT_KEY*/
      if (LACP_PMGR_SetDot3adAggActorAdminKeyForAll(admin_key)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to set default LACP actor admin key\r\n");
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Collector_Max_Delay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI16_T delay_time   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_LACP_COLLECTORMAXDELAY:
      delay_time = atoi(arg[0]);
      if (LACP_PMGR_SetDot3adAggCollectorMaxDelayForAll(delay_time)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr_1("Failed to set LACP collector max delay %d\r\n", delay_time);
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_LACP_COLLECTORMAXDELAY:
      delay_time = 0;/*LACP_COLLECTOR_MAX_DELAY*/
      if (LACP_PMGR_SetDot3adAggCollectorMaxDelayForAll(delay_time)!= LACP_RETURN_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to set default LACP collector max delay\r\n");
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Actor_System_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T priority   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_ACTOR_SYSTEMPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_ACTOR_SYSTEMPRIORITY:
      priority = 0x8000;/*LACP_SYSTEM_DEFAULT_PRIORITY*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortActorSystemPriority(ifindex, priority)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port actor system priority %d on ethernet %s\r\n", priority, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port actor system priority %d on ethernet %lu/%lu\r\n", priority, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
          }
       }
    }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Actor_Admin_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T admin_key   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_ACTOR_ADMINKEY:
      admin_key = atoi((char*)arg[0]);

      for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
      {
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            verify_port = i;

            verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);

            /*do not check if this port is trunk member or not   */
            /*and semantic check must be completed in LACP itself*/
            if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else if (LACP_PMGR_SetDot3adAggPortActorAdminKey(ifindex, admin_key)!= LACP_RETURN_SUCCESS)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(ifindex,name);
                  CLI_LIB_PrintStr_2("Failed to set LACP port actor admin key %d on ethernet %s\r\n", admin_key, name);
               }
#else
                  CLI_LIB_PrintStr_3("Failed to set LACP port actor admin key %d on ethernet %lu/%lu\r\n", admin_key, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
             }
         }
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_ACTOR_ADMINKEY:
      //admin_key = 1;/*LACP_DEFAULT_KEY*/
      for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
      {
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            verify_port = i;

            verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);

            /*do not check if this port is trunk member or not   */
            /*and semantic check must be completed in LACP itself*/
            if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else if (LACP_PMGR_SetDefaultDot3adAggPortActorAdminKey(ifindex)!= LACP_RETURN_SUCCESS)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(ifindex,name);
                  CLI_LIB_PrintStr_2("Failed to set LACP port actor admin key %d on ethernet %s\r\n", admin_key, name);
               }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port actor admin key %d on ethernet %lu/%lu\r\n", admin_key, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
             }
         }
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Actor_Port_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T priority   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_ACTOR_PORTPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_ACTOR_PORTPRIORITY:
      priority = 0x8000;/*LACP_PORT_DEFAULT_PRIORITY*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortActorPortPriority(ifindex, priority)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port actor port priority %d on ethernet %s\r\n", priority, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port actor port priority %d on ethernet %lu/%lu\r\n", priority, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
          }
       }
    }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Actor_Admin_State(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI8_T admin_state   = 0;
   UI8_T set_state   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;
   UI8_T state[] = { "LACP-activity",/*0*/
               "LACP-timeout",
               "aggregation",
               "synchronization",
               "collecting",
               "distributing",
               "defaulted",
               "expired"};

   if (*arg[0] == 'a' || *arg[0] == 'A')/*aggregation*/
      admin_state = 0x04;
   else if (*arg[0] == 'c' || *arg[0] == 'C')/*collecting*/
      admin_state = 0x10;
   else if (*arg[0] == 'e' || *arg[0] == 'E')/*expired*/
      admin_state = 0x80;
   else if (*arg[0] == 's' || *arg[0] == 'S')/*synchronization*/
      admin_state = 0x08;
   else if (*(arg[0]+1) == 'e' || *(arg[0]+1) == 'E')/*defaulted*/
      admin_state = 0x40;
   else if (*(arg[0]+1) == 'i' || *(arg[0]+1) == 'I')/*distributing*/
      admin_state = 0x20;
   else if (*(arg[0]+5) == 'a' || *(arg[0]+5) == 'A')/*lacp-Activity*/
      admin_state = 0x01;
   else if (*(arg[0]+5) == 't' || *(arg[0]+5) == 'T')/*lacp-Timeout*/
      admin_state = 0x02;
   else
      return CLI_ERR_INTERNAL;

   for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
   {
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }

         set_state = 0;
         if(LACP_PMGR_GetRunningDot3adAggPortActorAdminState(ifindex,&set_state) == SYS_TYPE_GET_RUNNING_CFG_FAIL)
         {
            CLI_LIB_PrintStr_2("Failed to get actor admin state on ethernet %lu/%lu.\r\n", verify_unit, verify_port);
         }
         else
         {
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_ACTOR_ADMINSTATE:
                   set_state |= admin_state;
                   break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_ACTOR_ADMINSTATE:
                   set_state &= ~admin_state;
                   break;

                default:
                   return CLI_ERR_INTERNAL;
            }

            if (LACP_PMGR_SetDot3adAggPortActorAdminState(ifindex, set_state)!= LACP_RETURN_SUCCESS)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(ifindex,name);
                  CLI_LIB_PrintStr_2("Failed to set LACP port actor admin state %s on ethernet %s\r\n", state[admin_state], name);
               }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port actor admin state %s on ethernet %lu/%lu\r\n", state[admin_state], verify_unit, verify_port);
#endif
            }
         }
       }
    }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_System_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T priority   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_SYSTEMPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_SYSTEMPRIORITY:
      priority = 0x8000;/*LACP_SYSTEM_DEFAULT_PRIORITY*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortPartnerAdminSystemPriority(ifindex, priority)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port partner system priority %d on ethernet %s\r\n", priority, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port partner system priority %d on ethernet %lu/%lu\r\n", priority, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
          }
       }
    }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_Admin_Key(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T admin_key   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_ADMINKEY:
      admin_key = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_ADMINKEY:
      admin_key = SYS_DFLT_LACP_KEY_NULL;
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortPartnerAdminKey(ifindex, admin_key)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port partner admin key %d on ethernet %s\r\n", admin_key, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port partner admin key %d on ethernet %lu/%lu\r\n", admin_key, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
          }
       }
    }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_Admin_Port_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
   UI16_T priority   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_PORTPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_PORTPRIORITY:
      priority = 0x8000;/*LACP_PORT_DEFAULT_PRIORITY*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortPartnerAdminPortPriority(ifindex, priority)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port partner port priority %d on ethernet %s\r\n", priority, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port partner port priority %d on ethernet %lu/%lu\r\n", priority, (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
          }
       }
    }

#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_Admin_State(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI8_T admin_state   = 0;
   UI8_T set_state   = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;
   UI8_T state[] = { "LACP-activity",/*0*/
               "LACP-timeout",
               "aggregation",
               "synchronization",
               "collecting",
               "distributing",
               "defaulted",
               "expired"};

   if (*arg[0] == 'a' || *arg[0] == 'A')/*aggregation*/
      admin_state = 0x04;
   else if (*arg[0] == 'c' || *arg[0] == 'C')/*collecting*/
      admin_state = 0x10;
   else if (*arg[0] == 'e' || *arg[0] == 'E')/*expired*/
      admin_state = 0x80;
   else if (*arg[0] == 's' || *arg[0] == 'S')/*synchronization*/
      admin_state = 0x08;
   else if (*(arg[0]+1) == 'e' || *(arg[0]+1) == 'E')/*defaulted*/
      admin_state = 0x40;
   else if (*(arg[0]+1) == 'i' || *(arg[0]+1) == 'I')/*distributing*/
      admin_state = 0x20;
   else if (*(arg[0]+5) == 'a' || *(arg[0]+5) == 'A')/*lacp-Activity*/
      admin_state = 0x01;
   else if (*(arg[0]+5) == 't' || *(arg[0]+5) == 'T')/*lacp-Timeout*/
      admin_state = 0x02;
   else
      return CLI_ERR_INTERNAL;

   for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
   {
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex);

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }

         set_state = 0;
         if(LACP_PMGR_GetRunningDot3adAggPortPartnerAdminState(ifindex,&set_state) == SYS_TYPE_GET_RUNNING_CFG_FAIL)
         {
            CLI_LIB_PrintStr_2("Failed to get actor admin state on ethernet %lu/%lu.\r\n", verify_unit, verify_port);
         }
         else
         {
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_ADMINSTATE:
                   set_state |= admin_state;
                   break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_ADMINSTATE:
                   set_state &= ~admin_state;
                   break;

                default:
                   return CLI_ERR_INTERNAL;
            }
            if (LACP_PMGR_SetDot3adAggPortPartnerAdminState(ifindex, set_state)!= LACP_RETURN_SUCCESS)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(ifindex,name);
                  CLI_LIB_PrintStr_2("Failed to set LACP port actor admin state %s on ethernet %s\r\n", state[admin_state], name);
               }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port actor admin state %s on ethernet %lu/%lu\r\n", state[admin_state], verify_unit, verify_port);
#endif
            }
          }
       }
    }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_Admin_System_Id(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI8_T sys_id[6]   = {0};
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_ADMINSYSTEMID:
      CLI_LIB_ValsInMac(arg[0], sys_id);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_ADMINSYSTEMID:
      CLI_LIB_ValsInMac(0, sys_id);/*????*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortPartnerAdminSystemId(ifindex, sys_id)!= LACP_RETURN_SUCCESS)
         {
            char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};

            sprintf((char*)buff, " %02X-%02X-%02X-%02X-%02X-%02X", sys_id[0], sys_id[1], sys_id[2],
                                                               sys_id[3], sys_id[4], sys_id[5]);
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port partner admin system id %s on ethernet %s\r\n", buff, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port partner admin system id %s on ethernet %lu/%lu\r\n", buff, verify_unit, verify_port);
#endif
          }
       }
    }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Lacp_Port_Partner_Admin_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
#if 0
   UI16_T admin_port  = 0;
   UI32_T i       = 0;
   UI32_T ifindex = 0;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;

   CLI_API_EthStatus_T verify_ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_LACP_PARTNER_ADMINPORT:
      admin_port = atoi(arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_LACP_PARTNER_ADMINPORT:
      admin_port = SYS_DFLT_PORT_LACP_PORT_STATUS;/*LACP_DEFAULT_PORT_ADMIN*/
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

         /*do not check if this port is trunk member or not   */
         /*and semantic check must be completed in LACP itself*/
         if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (LACP_PMGR_SetDot3adAggPortPartnerAdminPort(ifindex, admin_port)!= LACP_RETURN_SUCCESS)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(ifindex,name);
               CLI_LIB_PrintStr_2("Failed to set LACP port partner admin port %d on ethernet %s\r\n", admin_port, name);
            }
#else
               CLI_LIB_PrintStr_3("Failed to set LACP port partner admin port %d on ethernet %lu/%lu\r\n", admin_port, verify_unit, verify_port);
#endif
          }
       }
    }
#endif
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Lacp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)
    UI32_T group = 0;
    UI32_T ifindex = 0;
    UI32_T line_num = 0;
    LACP_MGR_Dot3adAggPortListEntry_T agg_port_list_entry;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T count, i, j;

    switch (cmd_idx)
    {
    case PRIVILEGE_EXEC_CMD_W3_SHOW_LACP_COUNTERS:
    case NORMAL_EXEC_CMD_W3_SHOW_LACP_COUNTERS:
    {
        LACP_MGR_Dot3adAggPortStatsEntry_T data;

        count = 0;
        for (group = 1; group <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; group++)
        {
            SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);
            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);

                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortStatsEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Counters(data, line_num);
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
                }
                
                PROCESS_MORE("\r\n");
                count++;
            }
        }

        if (count == 0)
        {
            PROCESS_MORE("There is no active port channel.\r\n");
            PROCESS_MORE("\r\n");
        }
    }
        break;

    case PRIVILEGE_EXEC_CMD_W3_SHOW_LACP_INTERNAL:
    case NORMAL_EXEC_CMD_W3_SHOW_LACP_INTERNAL:
    {
        LACP_MGR_Dot3adAggPortEntry_T data;

        for (group = 1; group <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; group++)
        {
            LACP_MGR_Dot3adAggEntry_T agg_entry;

            SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);
            agg_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggEntry(&agg_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);
                PROCESS_MORE_F("Admin Key    : %lu\r\n", (unsigned long)agg_entry.dot3ad_agg_actor_admin_key);
                PROCESS_MORE_F("Oper Key     : %lu\r\n", (unsigned long)agg_entry.dot3ad_agg_actor_oper_key);
                if (agg_entry.dot3ad_agg_actor_timeout == LACP_LONG_TIMEOUT)
                {
                    PROCESS_MORE_F("Timeout      : %s\r\n", "Long");
                }
                else if (agg_entry.dot3ad_agg_actor_timeout == LACP_SHORT_TIMEOUT)
                {
                    PROCESS_MORE_F("Timeout      : %s\r\n", "Short");
                }
            }
            else
            {
                CLI_LIB_PrintStr_N("Failed to get information for port channel %lu.\r\n", (unsigned long)group);
                return CLI_NO_ERROR;
            }

            SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);
            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Internal(data, line_num);
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
                }
            }
            
            PROCESS_MORE("\r\n");
        }
    }
        break;

    case PRIVILEGE_EXEC_CMD_W3_SHOW_LACP_NEIGHBORS:
    case NORMAL_EXEC_CMD_W3_SHOW_LACP_NEIGHBORS:
    {
        LACP_MGR_Dot3adAggPortEntry_T data;

        count = 0;
        for (group = 1; group <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; group++)
        {
            SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);
            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);

                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Neighbors(data, line_num);
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
                }
                
                PROCESS_MORE("\r\n");
                count++;
            }
        }

        if (count == 0)
        {
            PROCESS_MORE("There is no active port channel.\r\n");
            PROCESS_MORE("\r\n");
        }
    }
        break;

    case PRIVILEGE_EXEC_CMD_W3_SHOW_LACP_SYSID:
    case NORMAL_EXEC_CMD_W3_SHOW_LACP_SYSID:
    {
        LACP_MGR_Dot3adAggEntry_T agg_entry;

        PROCESS_MORE("Port Channel  System Priority  System MAC Address\r\n");
        PROCESS_MORE("------------  ---------------  ------------------\r\n");

        for (group = 1; group <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; group++)
        {
            SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);
            agg_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggEntry(&agg_entry) == TRUE)
            {
                PROCESS_MORE_F("%12lu  %15u  %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    (unsigned long)group, agg_entry.dot3ad_agg_actor_system_priority,
                    agg_entry.dot3ad_agg_actor_system_id[0],
                    agg_entry.dot3ad_agg_actor_system_id[1],
                    agg_entry.dot3ad_agg_actor_system_id[2],
                    agg_entry.dot3ad_agg_actor_system_id[3],
                    agg_entry.dot3ad_agg_actor_system_id[4],
                    agg_entry.dot3ad_agg_actor_system_id[5]);
            }
        }
    }
        PROCESS_MORE("\r\n");
        break;

    case PRIVILEGE_EXEC_CMD_W2_SHOW_LACP:
    case NORMAL_EXEC_CMD_W2_SHOW_LACP:
    {
        group = atoi(arg[0]);
        SWCTRL_POM_TrunkIDToLogicalPort(group, &ifindex);

        switch (arg[1][0])
        {
        case 'c':
        case 'C':
        {
            LACP_MGR_Dot3adAggPortStatsEntry_T data;

            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);

                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortStatsEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Counters(data, line_num);
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
                }
            }
            else
                PROCESS_MORE("There is no active port channel.\r\n");
        }
            break;

        case 'i':
        case 'I':
        {
            LACP_MGR_Dot3adAggPortEntry_T data;
            LACP_MGR_Dot3adAggEntry_T agg_entry;

            agg_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggEntry(&agg_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);
                PROCESS_MORE_F("Admin Key    : %lu\r\n", (unsigned long)agg_entry.dot3ad_agg_actor_admin_key);
                PROCESS_MORE_F("Oper Key     : %lu\r\n", (unsigned long)agg_entry.dot3ad_agg_actor_oper_key);
                if (agg_entry.dot3ad_agg_actor_timeout == LACP_LONG_TIMEOUT)
                {
                    PROCESS_MORE("Timeout      : Long\r\n");
                }
                else if (agg_entry.dot3ad_agg_actor_timeout == LACP_SHORT_TIMEOUT)
                {
                    PROCESS_MORE("Timeout      : Short\r\n");
                }
            }
            else
            {
                CLI_LIB_PrintStr_N("Failed to get information for port channel %lu.\r\n", (unsigned long)group);
                return CLI_NO_ERROR;
            }

            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Internal(data, line_num);
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
                }
            }
        }
            break;

        case 'n':
        case 'N':
        {
            LACP_MGR_Dot3adAggPortEntry_T data;

            agg_port_list_entry.dot3ad_agg_index = ifindex;
            if (LACP_PMGR_GetDot3adAggPortListEntry(&agg_port_list_entry) == TRUE)
            {
                PROCESS_MORE_F("Port Channel : %lu\r\n", (unsigned long)group);

                for (i = 0; i < LACP_PORT_LIST_OCTETS; i++)
                {
                    for (j = 8; j >= 1; j--)
                    {
                        if ((0x01 << (j-1)) & (agg_port_list_entry.dot3ad_agg_port_list_ports[i]))
                        {
                            data.dot3ad_agg_port_index = i * 8 + (8 - j) + 1;
                            if (LACP_PMGR_GetDot3adAggPortEntry(&data) == TRUE)
                            {
                                line_num = CLI_API_Show_Lacp_Entry_By_One_Neighbors(data, line_num);
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
                }
            }
            else
                PROCESS_MORE("There is no active port channel.\r\n");
        }
            break;

        default:
            return CLI_ERR_INTERNAL;
        } /* end switch */
    }
        PROCESS_MORE("\r\n");
        break;

    default:
        return CLI_ERR_INTERNAL;
    } /* end switch */
#endif /* #if (SYS_CPNT_LACP == TRUE) */
   return CLI_NO_ERROR;
}

static UI32_T CLI_API_Show_Lacp_Entry_By_One_Counters(LACP_MGR_Dot3adAggPortStatsEntry_T data, UI32_T line_num)
{
    UI32_T   unit;
    UI32_T   port;
    UI32_T   trunk_id;
    char     buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE(" ------------------------------------------------------------\r\n");

    if (SWCTRL_POM_LogicalPortToUserPort(data.dot3ad_agg_port_index, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        PROCESS_MORE_F(" Member Port                : Trunk %lu\r\n", (unsigned long)trunk_id);
    else
#if (CLI_SUPPORT_PORT_NAME == 1)
    {
        UI8_T name[MAXSIZE_ifName+1] = {0};

        CLI_LIB_Ifindex_To_Name(addr_entry.l_port,name);
        if (strlen(name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
        {                                        /*pttch 2002.07.10*/
            name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
        }
        PROCESS_MORE_F(" Member Port                : %s\r\n", name);
    }
#else
        PROCESS_MORE_F(" Member Port                : Eth %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
#endif

    PROCESS_MORE_F(" LACPDU Sent                : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_lac_pdus_tx);
    PROCESS_MORE_F(" LACPDU Received            : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_lac_pdus_rx);
    PROCESS_MORE_F(" MarkerPDU Sent             : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_marker_pdus_tx);
    PROCESS_MORE_F(" MarkerPDU Received         : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_marker_pdus_rx);
    PROCESS_MORE_F(" MarkerResponsePDU Sent     : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_marker_response_pdus_tx);
    PROCESS_MORE_F(" MarkerResponsePDU Received : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_marker_response_pdus_rx);
    PROCESS_MORE_F(" Unknown Packet Received    : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_unknown_rx);
    PROCESS_MORE_F(" Illegal Packet Received    : %lu\r\n", (unsigned long)data.dot3ad_agg_port_stats_illegal_rx);
    return line_num;
}

static UI32_T CLI_API_Show_Lacp_Entry_By_One_Internal(LACP_MGR_Dot3adAggPortEntry_T data, UI32_T line_num)
{
    UI32_T   unit;
    UI32_T   port;
    UI32_T   trunk_id;
    char     buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE(" ------------------------------------------------------------\r\n");

    if (SWCTRL_POM_LogicalPortToUserPort(data.dot3ad_agg_port_index, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        PROCESS_MORE_F(" Member Port     : Trunk %lu\r\n", (unsigned long)trunk_id);
    else
#if (CLI_SUPPORT_PORT_NAME == 1)
    {
        UI8_T name[MAXSIZE_ifName+1] = {0};

        CLI_LIB_Ifindex_To_Name(addr_entry.l_port,name);
        if (strlen(name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
        {                                        /*pttch 2002.07.10*/
            name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
        }
        PROCESS_MORE_F(" Member Port     : %s\r\n", name);
    }
#else
        PROCESS_MORE_F(" Member Port     : Eth %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
#endif

    if (data.dot3ad_agg_port_actor_oper_state & 0x02)
    {
        /* Short Timeout fix 1sec */
        PROCESS_MORE(" Periodic Time   : 1 seconds\r\n");
    }
    else
    {
        /* Long Timeout fix 30sec */
        PROCESS_MORE(" Periodic Time   : 30 seconds\r\n");
    }
    PROCESS_MORE_F(" System Priority : %u\r\n", data.dot3ad_agg_port_actor_system_priority);
    PROCESS_MORE_F(" Port Priority   : %u\r\n", data.dot3ad_agg_port_actor_port_priority);
    PROCESS_MORE_F(" Admin Key       : %u\r\n", data.dot3ad_agg_port_actor_admin_key);
    PROCESS_MORE_F(" Oper Key        : %u\r\n", data.dot3ad_agg_port_actor_oper_key);

    strcat(buff, " Admin State     : ");
    if (data.dot3ad_agg_port_actor_admin_state & 0x80)
    {
        strcat(buff, "Expired, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x40)
    {
        strcat(buff, "Defaulted, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x20)
    {
        strcat(buff, "Distributing, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x10)
    {
        strcat(buff, "Collecting, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x08)
    {
        strcat(buff, "Synchronization, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x04)
    {
        if ((strlen(buff) + strlen("Aggregatable, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Aggregatable, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x02)
    {
        if ((strlen(buff) + strlen("Short Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Short Timeout, ");
    }
    else
    {
        if ((strlen(buff) + strlen("Long Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Long Timeout, ");
    }
    if (data.dot3ad_agg_port_actor_admin_state & 0x01)
    {
        if ((strlen(buff) + strlen("Actvie LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Actvie LACP\r\n");
    }
    else
    {
        if ((strlen(buff) + strlen("Passive LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Passive LACP\r\n");
    }
    PROCESS_MORE(buff);

    strcat(buff, " Oper State      : ");
    if (data.dot3ad_agg_port_actor_oper_state & 0x80)
    {
        strcat(buff, "Expired, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x40)
    {
        strcat(buff, "Defaulted, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x20)
    {
        strcat(buff, "Distributing, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x10)
    {
        strcat(buff, "Collecting, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x08)
    {
        strcat(buff, "Synchronization, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x04)
    {
        if ((strlen(buff) + strlen("Aggregatable, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Aggregatable, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x02)
    {
        if ((strlen(buff) + strlen("Short Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Short Timeout, ");
    }
    else
    {
        if ((strlen(buff) + strlen("Long Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Long Timeout, ");
    }
    if (data.dot3ad_agg_port_actor_oper_state & 0x01)
    {
        if ((strlen(buff) + strlen("Actvie LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Actvie LACP\r\n");
    }
    else
    {
        if ((strlen(buff) + strlen("Passive LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                   ");
        }
        strcat(buff, "Passive LACP\r\n");
    }
    PROCESS_MORE(buff);

    return line_num;
}

static UI32_T CLI_API_Show_Lacp_Entry_By_One_Neighbors(LACP_MGR_Dot3adAggPortEntry_T data, UI32_T line_num)
{
    UI32_T   unit;
    UI32_T   port;
    UI32_T   trunk_id;
    char     buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE(" ------------------------------------------------------------\r\n");

    if (SWCTRL_POM_LogicalPortToUserPort(data.dot3ad_agg_port_index, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
        PROCESS_MORE_F(" Member Port             : Trunk %lu\r\n", (unsigned long)trunk_id);
    else
#if (CLI_SUPPORT_PORT_NAME == 1)
    {
        UI8_T name[MAXSIZE_ifName+1] = {0};

        CLI_LIB_Ifindex_To_Name(addr_entry.l_port,name);
        if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
        {                                        /*pttch 2002.07.10*/
           name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
        }
        PROCESS_MORE_F(" Member Port             : %s\r\n", name);
    }
#else
        PROCESS_MORE_F(" Member Port             : Eth %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
#endif

    PROCESS_MORE_F(" Partner Admin System ID : %u, %02X-%02X-%02X-%02X-%02X-%02X\r\n",
        data.dot3ad_agg_port_partner_admin_system_priority,
        data.dot3ad_agg_port_partner_admin_system_id[0],
        data.dot3ad_agg_port_partner_admin_system_id[1],
        data.dot3ad_agg_port_partner_admin_system_id[2],
        data.dot3ad_agg_port_partner_admin_system_id[3],
        data.dot3ad_agg_port_partner_admin_system_id[4],
        data.dot3ad_agg_port_partner_admin_system_id[5]);
    PROCESS_MORE_F(" Partner Oper System ID  : %u, %02X-%02X-%02X-%02X-%02X-%02X\r\n",
        data.dot3ad_agg_port_partner_oper_system_priority,
        data.dot3ad_agg_port_partner_oper_system_id[0],
        data.dot3ad_agg_port_partner_oper_system_id[1],
        data.dot3ad_agg_port_partner_oper_system_id[2],
        data.dot3ad_agg_port_partner_oper_system_id[3],
        data.dot3ad_agg_port_partner_oper_system_id[4],
        data.dot3ad_agg_port_partner_oper_system_id[5]);
    PROCESS_MORE_F(" Partner Admin Port ID   : %u, %u\r\n",
        data.dot3ad_agg_port_partner_admin_port_priority,
        data.dot3ad_agg_port_partner_admin_port);
    PROCESS_MORE_F(" Partner Oper Port ID    : %u, %u\r\n",
        data.dot3ad_agg_port_partner_oper_port_priority,
        data.dot3ad_agg_port_partner_oper_port);
    PROCESS_MORE_F(" Partner Admin Key       : %u\r\n", data.dot3ad_agg_port_partner_admin_key);
    PROCESS_MORE_F(" Partner Oper Key        : %u\r\n", data.dot3ad_agg_port_partner_oper_key);

    strcat(buff, " Partner Admin State     : ");
    if (data.dot3ad_agg_port_partner_admin_state & 0x80)
    {
        strcat(buff, "Expired, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x40)
    {
        strcat(buff, "Defaulted, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x20)
    {
        strcat(buff, "Distributing, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x10)
    {
        strcat(buff, "Collecting, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x08)
    {
        if ((strlen(buff) + strlen("Synchronization, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Synchronization, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x04)
    {
        if ((strlen(buff) + strlen("Aggregatable, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Aggregatable, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x02)
    {
        if ((strlen(buff) + strlen("Short Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Short Timeout, ");
    }
    else
    {
        if ((strlen(buff) + strlen("Long Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Long Timeout, ");
    }
    if (data.dot3ad_agg_port_partner_admin_state & 0x01)
    {
        if ((strlen(buff) + strlen("Actvie LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Actvie LACP\r\n");
    }
    else
    {
        if ((strlen(buff) + strlen("Passive LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Passive LACP\r\n");
    }
    PROCESS_MORE(buff);

    strcat(buff, " Partner Oper State      : ");
    if (data.dot3ad_agg_port_partner_oper_state & 0x80)
    {
        strcat(buff, "Expired, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x40)
    {
        strcat(buff, "Defaulted, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x20)
    {
        strcat(buff, "Distributing, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x10)
    {
        strcat(buff, "Collecting, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x08)
    {
        if ((strlen(buff) + strlen("Synchronization, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Synchronization, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x04)
    {
        if ((strlen(buff) + strlen("Aggregatable, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Aggregatable, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x02)
    {
        if ((strlen(buff) + strlen("Short Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Short Timeout, ");
    }
    else
    {
        if ((strlen(buff) + strlen("Long Timeout, ")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Long Timeout, ");
    }
    if (data.dot3ad_agg_port_partner_oper_state & 0x01)
    {
        if ((strlen(buff) + strlen("Actvie LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Actvie LACP\r\n");
    }
    else
    {
        if ((strlen(buff) + strlen("Passive LACP")) > 80)
        {
            strcat(buff, "\r\n");
            PROCESS_MORE(buff)
            strcat(buff, "                           ");
        }
        strcat(buff, "Passive LACP\r\n");
    }
    PROCESS_MORE(buff);

    return line_num;
}

/* set port-channel lacp admin-key */
UI32_T CLI_API_Lacp_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_LACP == TRUE)

   UI16_T admin_key   = 0;
   UI32_T ifindex = 0;

   SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_LACP:
        if (*arg[0] == 'a' || *arg[0] == 'A')/* admin key */
        {
            admin_key = atoi((char*)arg[1]);
            if (LACP_PMGR_SetDot3adAggActorAdminKey(ifindex,admin_key)!= LACP_RETURN_SUCCESS)
            {
                CLI_LIB_PrintStr_1("Failed to set LACP admin key %d\r\n", admin_key);
            }
        }
        else if (*arg[0] == 't' || *arg[0] == 'T')/* timeout */
        {
            UI32_T timeout;
            if (*arg[1] == 'l' || *arg[1] == 'L')/* long */
            {
                timeout = LACP_LONG_TIMEOUT;
            }
            else if (*arg[1] == 's' || *arg[1] == 'S')/* short */
            {
                timeout = LACP_SHORT_TIMEOUT;
            }
            else
                return CLI_ERR_INTERNAL;
            if(LACP_PMGR_SetDot3adAggActorLACP_Timeout(ifindex, timeout) != LACP_RETURN_SUCCESS)
            {
                if(timeout==LACP_LONG_TIMEOUT)
                {
                    CLI_LIB_PrintStr("Failed to set LACP long timeout\r\n");
                }
                else
                {
                    CLI_LIB_PrintStr("Failed to set LACP short timeout\r\n");
                }
            }

        }
        else
            return CLI_ERR_INTERNAL;
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_LACP:
        if (*arg[0] == 'a' || *arg[0] == 'A')/* admin key */
        {
            if (LACP_PMGR_SetDefaultDot3adAggActorAdminKey(ifindex)!= LACP_RETURN_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to restore default LACP actor admin key\r\n");
            }
        }
        else if (*arg[0] == 't' || *arg[0] == 'T')/* timeout */
        {
            UI32_T timeout;

            timeout = LACP_DEFAULT_SYSTEM_TIMEOUT;

            if(LACP_PMGR_SetDot3adAggActorLACP_Timeout(ifindex, timeout) != LACP_RETURN_SUCCESS)
            {
                CLI_LIB_PrintStr("Failed to restore default LACP timeout\r\n");
            }

        }
        else
            return CLI_ERR_INTERNAL;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

#endif
   return CLI_NO_ERROR;
}

