/* -------------------------------------------------------------------------------------
 * FILE NAME:  CLI_API_STA.C
 * -------------------------------------------------------------------------------------
 * PURPOSE:This file is the action function of 802.1w command
 * NOTE:
 *
 *
 *
 * HISTORY:
 * Modifier         Date                Description
 * -------------------------------------------------------------------------------------
 * pttch           6-21-2002             First Created
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2002
 * -------------------------------------------------------------------------------------*/
/* INCLUDE FILE DECLARATIONS
 */
/*system*/
#include "sysfun.h"
#include "sys_cpnt.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>
#include <ctype.h>
#include "sys_dflt.h"
#include "l_bitmap.h"
/*cli internal*/
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
//#include "cli_type.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"
#include "cli_msg.h"
#include "cli_cmd.h"
#include "cli_runcfg.h"
#include "cli_api.h"
#include "swctrl.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "amtrdrv_pom.h"
#include "amtrdrv_mgr.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#include "xstp_type.h"

#include "swctrl_pmgr.h"
#include "trk_pmgr.h"
#include "vlan_pmgr.h"
#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_pmgr.h"
#endif

#if (SYS_CPNT_MSTP_SUPPORT_PVST==TRUE)
#include "cli_api_vlan.h"
#endif

#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
#include "stktplg_pom.h"
#endif

#if(SYS_CPNT_VXLAN == TRUE)
#include "vxlan_type.h"
#include "vxlan_pom.h"
#include "amtrl3_pom.h"
#endif

#if(SYS_CPNT_OVSVTEP == TRUE)
#include "ovsvtep_om.h"
#endif

#define CLI_API_STA_CHK_LINE_NUM(ln_num)         \
                    switch (ln_num)              \
                    {                            \
                    case EXIT_SESSION_MORE:      \
                        return CLI_EXIT_SESSION; \
                    case JUMP_OUT_MORE:          \
                        return CLI_NO_ERROR;     \
                    default:                     \
                        break;                   \
                    }

static BOOL_T IS_MAC_ZERO(UI8_T *mac);
static void   DEC_MAC(UI8_T *mac);
static BOOL_T MacFilter(UI8_T got_mac[6], UI8_T keyin_mac[6], UI8_T ignore_mask[6]);
static void   show_mac_count_by_lport(UI32_T lport);

static UI32_T cli_api_show_spanningtree_brief(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);


#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
static BOOL_T cli_api_spanningtree_eth_port_to_bmap(char *unit_port_str, UI8_T *lport_pbmp);
static UI32_T cli_api_show_spanningtree_one_group(UI32_T group_id, UI8_T portbitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T line_num);
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MaxHops
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "max-hops" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MaxHops(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI32_T maxhops = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_MSTP_CMD_W1_MAXHOPS:
      maxhops = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_MSTP_CMD_W2_NO_MAXHOPS:
      maxhops = XSTP_TYPE_DEFAULT_BRIDGE_MAXHOP;
      break;

   default:
      break;
   }

   if (XSTP_PMGR_SetMstpMaxHop(maxhops)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set MST maxhops\r\n");
#endif
   }
#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Mst
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mst" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Mst(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI32_T instance_id = 0, priority = 0;
   UI32_T line_num = 0;
   char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};


   instance_id = atoi((char*)arg[0]);
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_MSTP_CMD_W1_MST:
      switch(arg[1][0])
      {
      case 'p':
      case 'P':

         if (!XSTP_POM_IsMstInstanceExistingInMstConfigTable(instance_id))
         {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr_1("The MSTP %ld does not exist\r\n", (unsigned long)instance_id);
#endif
             return CLI_NO_ERROR;
         }

         priority = atoi((char*)arg[2]);
         if (XSTP_PMGR_SetMstPriority(instance_id,priority)!=XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set MST priority\r\n");
#endif
         }
         break;

      case 'v':
      case 'V':
         {
            UI8_T  vlan_list[512] = {0};
            char   Token[CLI_DEF_MAX_BUFSIZE];
            char   *op_ptr;
            char   delemiters[2] = {0};
            UI32_T lower_val;
            UI32_T upper_val;
            UI32_T err_idx;
            UI32_T i;

            delemiters[0] = ',';
            op_ptr = arg[2];
            do
            {
               memset(Token, 0, sizeof(Token));
               op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

               if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                  return FALSE;
               else
               {
                  for(i = lower_val; i<=upper_val; i++)
                  {
                     vlan_list[(UI32_T)((i-1)/8)]  |= (1 << ( 7 - ((i-1)%8)));
                  }
               }
            } while(op_ptr != 0 && !isspace(*op_ptr));

            for (i = 1; i < SYS_DFLT_DOT1QMAXVLANID+1 ; i++)//pttch ???? vlan id need to use naming constant
            {
               if (vlan_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))))
               {
                  if (XSTP_PMGR_SetVlanToMstConfigTable(instance_id,i) != XSTP_TYPE_RETURN_OK)
                  {
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_N("Failed to add VLAN %lu on MST %lu\r\n",(unsigned long)i,(unsigned long)instance_id);
#endif
                  }
               }
            }
         }
         break;

      default:
         return CLI_ERR_INTERNAL;
      }
      break;

   case PRIVILEGE_CFG_MSTP_CMD_W2_NO_MST:
      if (arg[1] == NULL)
      {  /*vlan id = 0 means remove all vlans*/
         if (XSTP_PMGR_RemoveVlanFromMstConfigTable(instance_id,0)!=XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_N("Failed to remove VLAN on MST %lu\r\n", (unsigned long)instance_id);
#endif
         }
      }
      else
      {
         switch(arg[1][0])
         {
         case 'p':
         case 'P':
            if (!XSTP_POM_IsMstInstanceExisting(instance_id))
            {
               if (XSTP_POM_IsMstInstanceExistingInMstConfigTable(instance_id))
               {
#if (SYS_CPNT_EH == TRUE)
                  CLI_API_Show_Exception_Handeler_Msg();
#else
                  CLI_LIB_PrintStr_1("The MSTP %ld is inactive\r\n", (unsigned long)instance_id);
#endif
                  return CLI_NO_ERROR;
               }
               CLI_LIB_PrintStr_1("The MSTP %ld does not exist\r\n", (unsigned long)instance_id);
               return CLI_NO_ERROR;
            }
            if (XSTP_PMGR_SetMstPriority(instance_id,SYS_DFLT_STP_BRIDGE_PRIORITY)!=XSTP_TYPE_RETURN_OK)
            {
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr("Failed to set MST priority\r\n");
#endif
            }
            break;

         case 'v':
         case 'V':
            {
               UI8_T  vlan_list[512] = {0};
               char   Token[CLI_DEF_MAX_BUFSIZE];
               char   *op_ptr;
               char   delemiters[2] = {0};
               UI32_T lower_val;
               UI32_T upper_val;
               UI32_T err_idx;
               UI32_T i;

               delemiters[0] = ',';
               op_ptr = arg[2];
               do
               {
                  memset(Token, 0, sizeof(Token));
                  op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                  if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                     return FALSE;
                  else
                  {
                     for(i = lower_val; i<=upper_val; i++)
                     {
                        vlan_list[(UI32_T)((i-1)/8)]  |= (1 << ( 7 - ((i-1)%8)));
                     }
                  }
               } while(op_ptr != 0 && !isspace(*op_ptr));

               for (i = 1; i < SYS_DFLT_DOT1QMAXVLANID+1 ; i++)//pttch ???? vlan id need to use naming constant
               {
                  if (vlan_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))))
                  {
                     if (XSTP_PMGR_RemoveVlanFromMstConfigTable(instance_id,i)!=XSTP_TYPE_RETURN_OK)
                     {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_N("Failed to remove VLAN %lu on MST %lu\r\n",(unsigned long)i,(unsigned long)instance_id);
#endif
                     }
                  }
               }
            }
            break;
         default:
            return CLI_ERR_INTERNAL;
         }
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Name
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "name" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_MSTP_SUPPORT_PVST == FALSE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI8_T SysNameBuf[XSTP_TYPE_REGION_NAME_MAX_LENGTH + 1] = {0};
   UI8_T                   mac_addr[6] = {0};

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_MSTP_CMD_W1_NAME:
        if (NULL != arg[0])
        {
            strcpy((char*)SysNameBuf,(char*)arg[0]);
        }
        else
        {
            return CLI_NO_ERROR;
        }
        break;

   case PRIVILEGE_CFG_MSTP_CMD_W2_NO_NAME:
        if (SWCTRL_POM_GetCpuMac(mac_addr)==TRUE)
        {
           SYSFUN_Sprintf((char*)SysNameBuf, "%02X %02X %02X %02X %02X %02X", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        }
        break;

   default:
      break;
   }

    if(XSTP_PMGR_SetMstpConfigurationName(SysNameBuf)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set name\r\n");
#endif
    }

#endif

#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Revision
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "revision" in MSTP configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Revision(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_MSTP_SUPPORT_PVST == FALSE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI32_T revision = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_MSTP_CMD_W1_REVISION:
      revision = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_MSTP_CMD_W2_NO_REVISION:
      revision = XSTP_TYPE_DEFAULT_CONFIG_REVISION;
      break;

   default:
      break;
   }

   if (XSTP_PMGR_SetMstpRevisionLevel(revision) !=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set MST revision\r\n");
#endif
   }

#endif
#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T status = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W1_SPANNINGTREE:
      status = VAL_staSystemStatus_enabled;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_SPANNINGTREE:
      status = VAL_staSystemStatus_disabled;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetSystemSpanningTreeStatus(status) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set spanning-tree\r\n");
#endif
   }

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ForwartTime
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree forward-time"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ForwartTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T forward_delay = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_FORWARDTIME:
      forward_delay = atoi((char*)arg[0])*100;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_FORWARDTIME:
      forward_delay = SYS_DFLT_STP_BRIDGE_FORWARD_DELAY*100;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetForwardDelay(forward_delay)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set forward-time\r\n");
#endif
   }

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_HelloTime
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree hello-time" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_HelloTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T hello_time = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_HELLOTIME:
      hello_time = atoi((char*)arg[0])*100;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_HELLOTIME:
      hello_time = SYS_DFLT_XSTP_BRIDGE_HELLO_TIME*100;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetHelloTime(hello_time)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set hello-time\r\n");
#endif
   }
   return CLI_NO_ERROR;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_MaxAge
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree max-age" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_MaxAge(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T max_age = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_MAXAGE:
      max_age = atoi((char*)arg[0])*100;
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_MAXAGE:
      max_age = SYS_DFLT_STP_BRIDGE_MAX_AGE * 100;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetMaxAge(max_age)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set max-age\r\n");
#endif
   }

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mode
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mode" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_MSTP_SUPPORT_PVST == FALSE)
   UI32_T mode = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_MODE:
      switch(arg[0][0])
      {
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      case 'm':
      case 'M':
         mode = XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID;
         break;
#endif

      case 'r':
      case 'R':
         mode = XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID;
         break;

      case 's':
      case 'S':
         mode = XSTP_TYPE_STP_PROTOCOL_VERSION_ID;
         break;
      default:
         return CLI_ERR_INTERNAL;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_MODE:
      mode = SYS_DFLT_STP_PROTOCOL_TYPE;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetSystemSpanningTreeVersion(mode)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set spanning-tree mode\r\n");
#endif
   }

#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_MstConfiguration
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst-configuration"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_MstConfiguration(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_MSTP_MODE;
#endif
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Pathcost_Method
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree pathcost method" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Pathcost_Method(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T pathcost_method = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_SPANNINGTREE_PATHCOST_METHOD:
      switch(*arg[0])
      {
      case 'l':
      case 'L':
         pathcost_method = VAL_staPathCostMethod_long;
         break;

      case 's':
      case 'S':
         pathcost_method = VAL_staPathCostMethod_short;
         break;

      default:
         return CLI_ERR_INTERNAL;
      }

      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SPANNINGTREE_PATHCOST_METHOD:
      pathcost_method = XSTP_TYPE_DEFAULT_PATH_COST_METHOD;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
   if (XSTP_PMGR_SetPathCostMethod(pathcost_method)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set spanning-tree pathcost-method\r\n");
#endif
   }


   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Priority
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree priority" in global
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T priority = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_PRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_PRIORITY:
      priority = XSTP_TYPE_DEFAULT_BRIDGE_PRIORITY;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetSystemBridgePriority(priority)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set spanning-tree priority\r\n");
#endif
   }

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_TransmissionLimit
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree transmission-limit"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_TransmissionLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T tx_hold_count = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_TRANSMISSIONLIMIT:
      tx_hold_count = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_TRANSMISSIONLIMIT:
      tx_hold_count = XSTP_TYPE_DEFAULT_TX_HOLD_COUNT;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (XSTP_PMGR_SetTransmissionLimit(tx_hold_count)!=XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to set spanning-tree transmission limit\r\n");
#endif
   }

   return CLI_NO_ERROR;
}




/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Cost_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree cost"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Cost_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T cost = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_COST:
      cost = atoi((char*)arg[0]);
      for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
      {
         UI32_T lport = 0;
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else if(XSTP_PMGR_SetPortPathCost(lport, cost) != XSTP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                  CLI_API_Show_Exception_Handeler_Msg();
#else
                  CLI_LIB_PrintStr_1("Failed to set path cost on ethernet %s.\r\n", name);
#endif
               }
#else
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_2("Failed to set path cost on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
         }
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_COST:
      for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
      {
         UI32_T lport = 0;
         if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
         {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
              if(XSTP_PMGR_SetPortAutoPathCost(lport) != XSTP_TYPE_RETURN_OK)
              {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                  CLI_API_Show_Exception_Handeler_Msg();
#else
                  CLI_LIB_PrintStr_1("Failed to set path cost on ethernet %s\r\n", name);
#endif
               }
#else
#if (SYS_CPNT_EH == TRUE)
                 CLI_API_Show_Exception_Handeler_Msg();
#else
                 CLI_LIB_PrintStr_2("Failed to set path cost on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
              }
            }
         }
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_EdgePort_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree edgeport"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_EdgePort_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T status = 0;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_EDGEPORT:
      if (arg[0] == NULL)
      {
         status = VAL_staPortAdminEdgePortWithAuto_true;
      }
      else
      {
         status = VAL_staPortAdminEdgePortWithAuto_auto;
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_EDGEPORT:
      status = VAL_staPortAdminEdgePortWithAuto_false;
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

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if(XSTP_PMGR_SetPortAdminEdgePort(lport, status) != XSTP_TYPE_RETURN_OK)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_1("Failed to set edge-port status on ethernet %s\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set edge-port status on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
         }
      }
   }

   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_LinkType_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree link-type"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_LinkType_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T type = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_LINKTYPE:
      switch(arg[0][0])
      {
      case 'a':
      case 'A':
         type = VAL_staPortAdminPointToPoint_auto;
         break;

      case 'p':
      case 'P'://pttch ???????????
         type = VAL_staPortAdminPointToPoint_forceTrue;
         break;

      case 's':
      case 'S':
         type = VAL_staPortAdminPointToPoint_forceFalse;
         break;

      default:
         return CLI_ERR_INTERNAL;

      }
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_LINKTYPE:
      type = VAL_staPortAdminPointToPoint_auto;//pttch?????
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

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if(XSTP_PMGR_SetPortLinkTypeMode(lport, type) != XSTP_TYPE_RETURN_OK)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
              CLI_API_Show_Exception_Handeler_Msg();
#else
              CLI_LIB_PrintStr_1("Failed to set link type on ethernet %s\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set link type on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
         }
      }
   }

   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mst_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mst_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T instance_id = 0;
   UI32_T cost = 0;
   UI32_T priority = 0;
   instance_id = atoi((char*)arg[0]);

   if (!XSTP_POM_IsMstInstanceExisting(instance_id))
   {
      if (XSTP_POM_IsMstInstanceExistingInMstConfigTable(instance_id))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_1("The MSTP %ld is inactive\r\n", (unsigned long)instance_id);
#endif
         return CLI_NO_ERROR;
      }
      CLI_LIB_PrintStr_1("The MSTP %ld does not exist\r\n", (unsigned long)instance_id);
      return CLI_NO_ERROR;
   }

   for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
   {
      UI32_T lport = 0;
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else
         {
            switch(cmd_idx)
            {
            case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_MST:
               switch(arg[1][0])
               {
               case 'c':
               case 'C':
                  cost = atoi((char*)arg[2]);
                  if(XSTP_PMGR_SetMstPortPathCost(lport, instance_id, cost) != XSTP_TYPE_RETURN_OK)
                  {
#if (CLI_SUPPORT_PORT_NAME == 1)
                     {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set mstp cost on ethernet %s\r\n", name);
#endif
                     }
#else
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to set MSTP cost on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                  }
                  break;

               case 'p':
               case 'P':
                  priority = atoi((char*)arg[2]);
                  if(XSTP_PMGR_SetMstPortPriority(lport, instance_id, priority) != XSTP_TYPE_RETURN_OK)
                  {
#if (CLI_SUPPORT_PORT_NAME == 1)
                     {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set MSTP priority on ethernet %s\r\n", name);
#endif
                     }
#else
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to set MSTP priority on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                  }
                  break;

               default:
                  return CLI_ERR_INTERNAL;
               }
               break;

            case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_MST:
               switch(arg[1][0])
               {
               case 'c':
               case 'C':
                  if(XSTP_PMGR_SetMstPortAutoPathCost(lport, instance_id) != XSTP_TYPE_RETURN_OK)
                  {
#if (CLI_SUPPORT_PORT_NAME == 1)
                     {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set MSTP cost on ethernet %s\r\n", name);
#endif
                     }
#else
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_2("Failed to set MSTP cost on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                  }
                  break;

               case 'p':
               case 'P':
                  priority = 128;//pttch
                  if(XSTP_PMGR_SetMstPortPriority(lport, instance_id, priority) != XSTP_TYPE_RETURN_OK)
                  {
#if (CLI_SUPPORT_PORT_NAME == 1)
                     {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set MSTP priority on ethernet %s\r\n", name);
#endif
                     }
#else
#if (SYS_CPNT_EH == TRUE)
                     CLI_API_Show_Exception_Handeler_Msg();
#else
                     CLI_LIB_PrintStr_2("Failed to set MSTP priority on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                  }
                  break;

               default:
                  return CLI_ERR_INTERNAL;
               }
               break;

            default:
               return CLI_ERR_INTERNAL;
            }
         }
      }
   }
#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_PortPriority_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree port-priority"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_PortPriority_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T priority = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_PORTPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_PORTPRIORITY:
      priority = SYS_DFLT_STP_PORT_PRIORITY;
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

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if(XSTP_PMGR_SetPortPriority(lport,priority) != XSTP_TYPE_RETURN_OK)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_1("Failed to set port priority on ethernet %s\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set port priority on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
         }
      }
   }
   return CLI_NO_ERROR;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Portfast_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree portfast"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Portfast_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 // removed, said by macauley
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T status = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_PORTFAST:
      status = VAL_staPortAdminEdgePort_true;
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_PORTFAST:
      status = VAL_staPortAdminEdgePort_false;
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

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if(XSTP_PMGR_SetPortAdminEdgePort(lport,status) != XSTP_TYPE_RETURN_OK)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
               UI8_T name[MAXSIZE_ifName+1] = {0};
               CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
               CLI_API_Show_Exception_Handeler_Msg();
#else
               CLI_LIB_PrintStr_1("Failed to set port fast on ethernet %s\r\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set port fast on ethernet %lu/%lu\r\n", verify_unit, verify_port);
#endif
#endif
         }
      }
   }
#endif // #if 0

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ProtocolMigration
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree prototol migration"
 *            in privilege exec mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ProtocolMigration(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP)
   UI32_T lport;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_trunk_id = 0;
   CLI_API_TrunkStatus_T verify_trunk_ret;
   UI32_T verify_unit;
   UI32_T verify_port;

   switch(arg[0][0])
   {
   case 'e':
   case 'E':
      verify_unit = atoi((char*)arg[1]);
      verify_port = atoi( strchr((char*)arg[1], '/')+1 );
#if (CLI_SUPPORT_PORT_NAME == 1)
      if (isdigit(arg[1][0]))
      {
         verify_unit = atoi((char*)arg[1]);
         verify_port = atoi(strchr((char*)arg[1],'/')+1);
      }
      else/*port name*/
      {
         UI32_T trunk_id = 0;
         if (!IF_PMGR_IfnameToIfindex(arg[1], &lport))
         {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
#endif
             return CLI_NO_ERROR;
         }
         SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
      }
#endif

      if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
      {
         display_ethernet_msg(verify_ret, verify_unit, verify_port);
         return CLI_NO_ERROR;
      }

      if(XSTP_PMGR_SetPortProtocolMigration(lport,VAL_staPortProtocolMigration_true) != XSTP_TYPE_RETURN_OK)
      {
#if (CLI_SUPPORT_PORT_NAME == 1)
         {
            UI8_T name[MAXSIZE_ifName+1] = {0};
            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set protocol-migration on ethernet %s\r\n", name);
#endif
         }
#else
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_2("Failed to set protocol-migration on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
      }
      break;

   case 'p':
   case 'P':
      verify_trunk_id = atoi((char*)arg[1]);
      if( (verify_trunk_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
      {
         display_trunk_msg(verify_trunk_ret, verify_trunk_id);
         return CLI_NO_ERROR;
      }
      if(XSTP_PMGR_SetPortProtocolMigration(lport, VAL_staPortProtocolMigration_true) != XSTP_TYPE_RETURN_OK)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_1("Failed to set protocol-migration on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
      }
      break;

   default:
      break;
   }
#endif
   return CLI_NO_ERROR;

}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Cost_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree cost"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Cost_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T cost;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_COST:
      cost = atoi((char*)arg[0]);
      if(XSTP_PMGR_SetPortPathCost(lport, cost) != XSTP_TYPE_RETURN_OK)
      {
#if (SYS_CPNT_EH == TRUE)
          CLI_API_Show_Exception_Handeler_Msg();
#else
          CLI_LIB_PrintStr_1("Failed to set path cost on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_COST:
      if(XSTP_PMGR_SetPortAutoPathCost(lport) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set path cost on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
   }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;

}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_EdgePort_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree edgeport"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_EdgePort_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T status = 0;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_EDGEPORT:
      if (arg[0] == NULL)
      {
         status = VAL_staPortAdminEdgePortWithAuto_true;
      }
      else
      {
         status = VAL_staPortAdminEdgePortWithAuto_auto;
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_EDGEPORT:
      status = VAL_staPortAdminEdgePortWithAuto_false;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(XSTP_PMGR_SetPortAdminEdgePort(lport, status) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set edge-port status on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
   }
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_LinkType_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree link-type"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_LinkType_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T type = 0;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_LINKTYPE:
      switch(arg[0][0])
      {
      case 'a':
      case 'A':
         type = VAL_staPortAdminPointToPoint_auto;
         break;

      case 'p':
      case 'P'://pttch ???????????
         type = VAL_staPortAdminPointToPoint_forceTrue;
         break;

      case 's':
      case 'S':
         type = VAL_staPortAdminPointToPoint_forceFalse;
         break;

      default:
         return CLI_ERR_INTERNAL;

      }
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_LINKTYPE:
      type = VAL_staPortAdminPointToPoint_auto;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(XSTP_PMGR_SetPortLinkTypeMode(lport, type) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set link type on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
   }
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Mst_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree mst"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Mst_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T instance_id = 0;
   UI32_T cost = 0;
   UI32_T priority = 0;
   instance_id = atoi((char*)arg[0]);

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_MST:
      switch(arg[1][0])
      {
      case 'c':
      case 'C':
         cost = atoi((char*)arg[2]);
         if(XSTP_PMGR_SetMstPortPathCost(lport, instance_id, cost) != XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set MSTP cost on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
         }
         break;

      case 'p':
      case 'P':
         priority = atoi((char*)arg[2]);
         if(XSTP_PMGR_SetMstPortPriority(lport, instance_id, priority) != XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set MSTP priority on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
         }
         break;

      default:
         return CLI_ERR_INTERNAL;
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_MST:
      switch(arg[1][0])
      {
      case 'c':
      case 'C':
         if(XSTP_PMGR_SetMstPortAutoPathCost(lport, instance_id) != XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set MSTP cost on ethernet %lu\r\n", (unsigned long)verify_trunk_id);
#endif
         }
         break;

      case 'p':
      case 'P':
         priority = 128;//pttch
         if(XSTP_PMGR_SetMstPortPriority(lport, instance_id, priority) != XSTP_TYPE_RETURN_OK)
         {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set MSTP priority on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
         }
         break;

      default:
         return CLI_ERR_INTERNAL;
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_PortPriority_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree port-priority"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_PortPriority_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T priority = 0;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_PORTPRIORITY:
      priority = atoi((char*)arg[0]);
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_PORTPRIORITY:
      priority = SYS_DFLT_XSTP_PORT_PRIORITY;//pttch
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(XSTP_PMGR_SetPortPriority(lport,priority) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set port priority on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
   }
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
   return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_Portfast_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree portfast"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_Portfast_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 // removed, said by macauley

   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T status = 0;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_PORTFAST:
      status = VAL_staPortAdminEdgePort_true;
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_PORTFAST:
      status = VAL_staPortAdminEdgePort_false;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(XSTP_PMGR_SetPortAdminEdgePort(lport,status) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set port fast on trunk %lu\r\n", verify_trunk_id);
#endif
   }
#endif //#if 0

   return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_ProtocolMigration_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree prototol migration"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_ProtocolMigration_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T status;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_PROTOCOLMIGRATION:
      status = VAL_dot1dStpPortProtocolMigration_true;//pttch
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_PROTOCOLMIGRATION:
      status = VAL_dot1dStpPortProtocolMigration_false;//pttch
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if(XSTP_PMGR_SetPortProtocolMigration(lport,status) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set port priority on trunk %lu\r\n", verify_trunk_id);
#endif
   }
#endif
   return CLI_NO_ERROR;
}

static UI32_T Show_XSTP_One_Port(UI32_T mstid,UI32_T lport,UI32_T line_num);
   static UI32_T Show_XSTP_One_Instance(UI32_T mstid, BOOL_T is_mstp, UI32_T line_num);

/* 2006/06/28
 * ES4649-38-00189: free memory before return
 */
UI32_T cli_api_show_spanningtree_macro(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P, char *str_list)
{

   UI32_T line_num = 0;
   UI32_T mode = 0;
   UI32_T current_max_unit,max_port_num;
   UI32_T i,j;
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
#if (SYS_CPNT_STACKING == TRUE)
      //STKTPLG_PMGR_GetNumberOfUnit(&current_max_unit);
      current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
      current_max_unit = 1;
#endif

   XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);

/*For Foundry, it does not support "show spanning tree <cr>"*/
#if (SYS_CPNT_MSTP_SUPPORT_PVST == FALSE)
   if (arg[0] == NULL)
   {
      UI32_T trunk_id = 0;
      UI32_T mstid    = 0;
      UI32_T lport    = 0;

      if( mode == XSTP_TYPE_STP_PROTOCOL_VERSION_ID || mode == XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
      {
         if((line_num = Show_XSTP_One_Instance(mstid, TRUE,line_num)) == JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
            return CLI_EXIT_SESSION;
         }

         /*for(j = 1; j <= current_max_unit; j++)*/
         for (j=0; STKTPLG_POM_GetNextUnit(&j); )
         {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            /*eth*/
            for (i = 1 ; i <= max_port_num;i++)
            {
               UI32_T verify_unit = j;
               UI32_T verify_port = i;
               CLI_API_EthStatus_T verify_ret;

               if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                  continue; /*1. not present; 2. trunk member; 3. unknown*/
               else
               {
                  XSTP_MGR_Dot1dStpExtPortEntry_T ext_port_entry;
                  /*pttch check if it is in the mst vlan port list*/
                  if (!XSTP_POM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port_entry))
                     continue;
#if (CLI_SUPPORT_PORT_NAME == 1)
                  {
                     UI8_T name[MAXSIZE_ifName+1] = {0};
                     CLI_LIB_Ifindex_To_Name(lport,name);
                     SYSFUN_Sprintf((char*)buff, "%s Information\r\n", name);
                     PROCESS_MORE(buff);
                  }
#else
                  SYSFUN_Sprintf((char*)buff, "Eth %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                  PROCESS_MORE(buff);
#endif
                  PROCESS_MORE("---------------------------------------------------------------\r\n");
                  if((line_num = Show_XSTP_One_Port(mstid, lport, line_num)) == JUMP_OUT_MORE)
                  {
                     return CLI_NO_ERROR;
                  }
                  else if (line_num == EXIT_SESSION_MORE)
                  {
                     return CLI_EXIT_SESSION;
                  }
               }
            }
         }/*end of unit*/
         /*trunk*/
         while(TRK_PMGR_GetNextTrunkId(&trunk_id))
         {
            UI32_T trunk_ifindex = 0;
            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

            if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
            {
               SYSFUN_Sprintf((char*)buff, "Trunk %lu Information\r\n",(unsigned long)trunk_id);
               PROCESS_MORE(buff);
               PROCESS_MORE("---------------------------------------------------------------\r\n");
               if((line_num = Show_XSTP_One_Port(mstid, trunk_ifindex,line_num)) == JUMP_OUT_MORE)
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
      else
      {
         do
         {
            if((line_num = Show_XSTP_One_Instance(mstid, TRUE,line_num)) == JUMP_OUT_MORE)
            {
               return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
               return CLI_EXIT_SESSION;
            }

            /*for(j = 1; j <= current_max_unit; j++)*/
            for (j=0; STKTPLG_POM_GetNextUnit(&j); )
            {
               max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
               for (i = 1 ; i <= max_port_num;i++)
               {
                  UI32_T verify_unit = j;
                  UI32_T verify_port = i;
                  CLI_API_EthStatus_T verify_ret;

                  if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                     continue; /*1. not present; 2. trunk member; 3. unknown*/
                  else
                  {
                     XSTP_MGR_Dot1dStpExtPortEntry_T ext_port;

                     /*pttch check if it is in the mst vlan port list*/
                     if (!XSTP_POM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port))
                        continue;
#if (CLI_SUPPORT_PORT_NAME == 1)
                     {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        SYSFUN_Sprintf((char*)buff, "%s Information\r\n", name);
                        PROCESS_MORE(buff);
                     }
#else
                     SYSFUN_Sprintf((char*)buff, "Eth %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                     PROCESS_MORE(buff);
#endif
                     PROCESS_MORE("---------------------------------------------------------------\r\n");
                     if((line_num = Show_XSTP_One_Port(mstid, lport, line_num)) == JUMP_OUT_MORE)
                     {
                        return CLI_NO_ERROR;
                     }
                     else if (line_num == EXIT_SESSION_MORE)
                     {
                        return CLI_EXIT_SESSION;
                     }
                  }
               }
            }/*end of unit*/
            /*trunk*/
            while(TRK_PMGR_GetNextTrunkId(&trunk_id))
            {
               UI32_T trunk_ifindex = 0;
               SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

               if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
               {
                  SYSFUN_Sprintf((char*)buff, "Trunk %lu Information\r\n",(unsigned long)trunk_id);
                  PROCESS_MORE(buff);
                  PROCESS_MORE("---------------------------------------------------------------\r\n");
                  if((line_num = Show_XSTP_One_Port(mstid, trunk_ifindex,line_num)) == JUMP_OUT_MORE)
                  {
                     return CLI_NO_ERROR;
                  }
                  else if (line_num == EXIT_SESSION_MORE)
                  {
                     return CLI_EXIT_SESSION;
                  }
               }
            }
         }while(XSTP_POM_GetNextExistedInstance(&mstid));
      }
   }
   else
#endif

   {
      switch(arg[0][0])
      {
      case 'b': /* show spanning-tree brief */
      case 'B':
        return cli_api_show_spanningtree_brief(cmd_idx, NULL, ctrl_P);
        break;

      case 'e':
      case 'E':
         {
            UI32_T lport    = 0;
            UI32_T mstid = 0;
            UI32_T verify_unit = atoi((char*)arg[1]);
            UI32_T verify_port = atoi(strchr((char*)arg[1],'/')+1);
            CLI_API_EthStatus_T verify_ret;

#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[1][0]))
            {
               verify_unit = atoi(arg[1]);
               verify_port = atoi(strchr((char*)arg[1],'/')+1);
            }
            else/*port name*/
            {
               UI32_T trunk_id = 0;
               if (!IF_MGR_IfnameToIfindex(arg[1], &lport))
               {
#if (SYS_CPNT_EH == TRUE)
                   CLI_API_Show_Exception_Handeler_Msg();
#else
                   CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
#endif
                   return CLI_NO_ERROR;
               }
               SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
            }
#endif
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               return CLI_NO_ERROR;
            }
            else
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  SYSFUN_Sprintf((char*)buff, "%s Information\r\n", name);
                  PROCESS_MORE(buff);
               }
#else
               SYSFUN_Sprintf((char*)buff, "Eth %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
               PROCESS_MORE(buff);
#endif
               PROCESS_MORE("--------------------------------------------------------------\r\n");
               Show_XSTP_One_Port(mstid,lport,line_num);
            }
         }
         break;

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      case 'm':
      case 'M':
         if(arg[1][0] == 'c' || arg[1][0] == 'C')
         {
            UI8_T SysNameBuf[XSTP_TYPE_REGION_NAME_MAX_LENGTH + 1];
            UI32_T revision = 0;
            UI32_T mstid = 0;
            XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;
            char  vlan_list[512] = {0};

            /* do not have get function, so use get running*/
            XSTP_PMGR_GetRunningMstpConfigurationName(SysNameBuf);
            XSTP_POM_GetRunningMstpRevisionLevel(&revision);

            SYSFUN_Sprintf((char*)buff, "MSTP Configuration Information\r\n");
            PROCESS_MORE(buff);
            PROCESS_MORE("--------------------------------------------------------------\r\n");
#if (SYS_CPNT_MSTP_SUPPORT_PVST == FALSE)
            SYSFUN_Sprintf((char*)buff, " Configuration Name : %s\r\n",SysNameBuf);
            PROCESS_MORE(buff);
            SYSFUN_Sprintf((char*)buff, " Revision Level     : %lu\r\n",(unsigned long)revision);
            PROCESS_MORE(buff);
            PROCESS_MORE("\r\n")
#endif
            SYSFUN_Sprintf((char*)buff, " Instance VLANs\r\n");
            PROCESS_MORE(buff);
            PROCESS_MORE("--------------------------------------------------------------\r\n");

            do
            {
               if (XSTP_POM_GetMstpInstanceVlanConfiguration(mstid,&mstp_instance_entry))
               {
#if (SYS_CPNT_MSTP_SUPPORT_PVST == TRUE)
                  if(mstid==0)
                      continue;
#endif
                  memcpy(vlan_list,mstp_instance_entry.mstp_instance_vlans_mapped,sizeof(UI8_T)*128);
                  memcpy(vlan_list+128,mstp_instance_entry.mstp_instance_vlans_mapped2k,sizeof(UI8_T)*128);
                  memcpy(vlan_list+256,mstp_instance_entry.mstp_instance_vlans_mapped3k,sizeof(UI8_T)*128);
                  memcpy(vlan_list+384,mstp_instance_entry.mstp_instance_vlans_mapped4k,sizeof(UI8_T)*128);
                  CLI_LIB_MSB_To_LSB(vlan_list,512);
                  CLI_LIB_Bitmap_To_List(vlan_list,str_list,512*4,SYS_DFLT_DOT1QMAXVLANID,FALSE);

                  {  /*because must show per line, so spilt the str_list to show */
                     UI32_T str_len = strlen((char*)str_list);
                     UI32_T i = 0;
                     UI8_T  print_str[68+1] = {0};

                     for (i = 0; i < ((str_len + 67) / 68); i++)
                     {
                        memcpy(print_str, str_list+i*68, sizeof(UI8_T)*68);
                        if (i == 0)
                        {
                           SYSFUN_Sprintf((char*)buff, "%6lu    %s\r\n",(unsigned long)mstid,print_str);
                           PROCESS_MORE(buff);
                        }
                        else
                        {
                           SYSFUN_Sprintf((char*)buff, "              %s\r\n",print_str);
                           PROCESS_MORE(buff);
                        }
                     }
                  }
               }
            }while(XSTP_POM_GetNextExistedInstanceForMstConfigTable(&mstid));
         }
         else
         {
            XSTP_MGR_Dot1dStpExtPortEntry_T ext_port_entry;
            UI32_T mstid = atoi((char*)arg[1]);
            UI32_T lport = 0;

            if (!XSTP_POM_IsMstInstanceExisting(mstid))
            {
               if (XSTP_POM_IsMstInstanceExistingInMstConfigTable(mstid))
               {
#if (SYS_CPNT_EH == TRUE)
                  CLI_API_Show_Exception_Handeler_Msg();
#else
                  CLI_LIB_PrintStr_1("The MSTP %ld is inactive\r\n", (long)mstid);
#endif
                  return CLI_NO_ERROR;
               }
               CLI_LIB_PrintStr_1("The MSTP %ld does not exist\r\n", (long)mstid);
               return CLI_NO_ERROR;
            }

            if (arg[2] == NULL)
            {
               UI32_T trunk_id = 0;

               if( mode == XSTP_TYPE_STP_PROTOCOL_VERSION_ID || mode == XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID)
               {
                  if((line_num = Show_XSTP_One_Instance(mstid, TRUE,line_num)) == JUMP_OUT_MORE)
                  {
                     return CLI_NO_ERROR;
                  }
                  else if (line_num == EXIT_SESSION_MORE)
                  {
                     return CLI_EXIT_SESSION;
                  }
               }
               else
               {
                  if((line_num = Show_XSTP_One_Instance(mstid, TRUE,line_num)) == JUMP_OUT_MORE)
                  {
                     return CLI_NO_ERROR;
                  }
                  else if (line_num == EXIT_SESSION_MORE)
                  {
                     return CLI_EXIT_SESSION;
                  }
               }
               /*for(j = 1; j <= current_max_unit; j++)*/
               for (j=0; STKTPLG_POM_GetNextUnit(&j); )
               {
                  max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
                  for (i = 1 ; i <= max_port_num;i++)
                  {
                     UI32_T verify_unit = j;
                     UI32_T verify_port = i;
                     CLI_API_EthStatus_T verify_ret;

                     if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                        continue; /*1. not present; 2. trunk member; 3. unknown*/
                     else
                     {
                        /*pttch check if it is in the mst vlan port list*/
                        if (!XSTP_POM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port_entry))
                           continue;
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                           UI8_T name[MAXSIZE_ifName+1] = {0};
                           CLI_LIB_Ifindex_To_Name(lport,name);
                           SYSFUN_Sprintf((char*)buff, "%s Information\r\n", name);
                           PROCESS_MORE(buff);
                        }
#else
                        SYSFUN_Sprintf((char*)buff, "Eth %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                        PROCESS_MORE(buff);
#endif
                        PROCESS_MORE("---------------------------------------------------------------\r\n");
                        if((line_num = Show_XSTP_One_Port(mstid, lport, line_num)) == JUMP_OUT_MORE)
                        {
                           return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                           return CLI_EXIT_SESSION;
                        }
                     }
                  }
               }/*end of unit*/
               /*trunk*/
               while(TRK_PMGR_GetNextTrunkId(&trunk_id))
               {
                  UI32_T trunk_ifindex = 0;
                  SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

                  if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
                  {
                     /*pttch check if it is in the mst vlan port list*/
                     if (!XSTP_POM_GetDot1dMstExtPortEntry(mstid, trunk_ifindex, &ext_port_entry))
                        continue;

                     SYSFUN_Sprintf((char*)buff, "Trunk %lu Information\r\n",(unsigned long)trunk_id);
                     PROCESS_MORE(buff);
                     PROCESS_MORE("---------------------------------------------------------------\r\n");
                     if((line_num = Show_XSTP_One_Port(mstid, trunk_ifindex,line_num)) == JUMP_OUT_MORE)
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
            else
            {
               switch(arg[2][0])
               {
               case 'b': /* show spanning-tree mst instance_id brief */
               case 'B':
                  return cli_api_show_spanningtree_brief(cmd_idx, &arg[1], ctrl_P);
                  break;

               case 'e':
               case 'E':
                  {
                     UI32_T verify_unit = atoi((char*)arg[3]);
                     UI32_T verify_port = atoi(strchr((char*)arg[3],'/')+1);
                     CLI_API_EthStatus_T verify_ret;
                     line_num = 2;

#if (CLI_SUPPORT_PORT_NAME == 1)
                     if (isdigit(arg[3][0]))
                     {
                        verify_unit = atoi((char*)arg[3]);
                        verify_port = atoi(strchr((char*)arg[3],'/')+1);
                     }
                     else/*port name*/
                     {
                        UI32_T trunk_id = 0;
                        if (!IF_PMGR_IfnameToIfindex(arg[3], &lport))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[3]);
#endif
                            return CLI_NO_ERROR;
                        }
                        SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                     }
#endif
                     if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                     {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        return CLI_NO_ERROR;
                     }
                     else
                     {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                           UI8_T name[MAXSIZE_ifName+1] = {0};
                           CLI_LIB_Ifindex_To_Name(lport,name);
                           SYSFUN_Sprintf((char*)buff, "%s Information\r\n", name);
                           PROCESS_MORE(buff);
                        }
#else
                        SYSFUN_Sprintf((char*)buff, "Eth %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                        PROCESS_MORE(buff);
#endif
                        PROCESS_MORE("---------------------------------------------------------------\r\n");
                        if((line_num = Show_XSTP_One_Port(mstid, lport,line_num)) == JUMP_OUT_MORE)
                        {
                           return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                           return CLI_EXIT_SESSION;
                        }
                     }
                  }
                  break;

               case 'p':
               case 'P':
                  {

                     UI32_T verify_trunk_id = atoi((char*)arg[3]);
                     CLI_API_TrunkStatus_T verify_ret;

                     if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_ETH_OK)
                     {
                        display_trunk_msg(verify_ret, verify_trunk_id);
                        return CLI_NO_ERROR;
                     }
                     else
                     {
                        SYSFUN_Sprintf((char*)buff, "Trunk %d Information\r\n", atoi((char*)arg[3]));
                        PROCESS_MORE(buff);
                        PROCESS_MORE("---------------------------------------------------------------\r\n");
                        if((line_num = Show_XSTP_One_Port(mstid, lport,line_num)) == JUMP_OUT_MORE)
                        {
                           return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                           return CLI_EXIT_SESSION;
                        }
                     }
                  }
                  break;

               default:
                  break;
               }
            }
         }
         break;
#endif

      case 'p':
      case 'P':
         {
            UI32_T lport    = 0;
            UI32_T mstid = 0;
            UI32_T verify_trunk_id = atoi((char*)arg[1]);
            CLI_API_TrunkStatus_T verify_ret;

            if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_ETH_OK)
            {
               display_trunk_msg(verify_ret, verify_trunk_id);
               return CLI_NO_ERROR;
            }
            else
            {
               SYSFUN_Sprintf((char*)buff, "Trunk %d Information\r\n", atoi((char*)arg[1]));
               PROCESS_MORE(buff);
               PROCESS_MORE("---------------------------------------------------------------\r\n");
               Show_XSTP_One_Port(mstid, lport, line_num);
            }
         }
         break;

      case 's':
      case 'S':
      {
         UI32_T trunk_id = 0;
         UI32_T mstid    = 0;
         UI32_T lport    = 0;

         if((line_num = Show_XSTP_One_Instance(mstid, TRUE,line_num)) == JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
            return CLI_EXIT_SESSION;
         }

         /*for(j = 1; j <= current_max_unit; j++)*/
         for (j = 0; STKTPLG_POM_GetNextUnit(&j);)
         {
            /*eth*/
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            for (i = 1 ; i <= max_port_num;i++)
            {
               UI32_T verify_unit = j;
               UI32_T verify_port = i;
               CLI_API_EthStatus_T verify_ret;
               UI32_T status;

               if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                  continue; /*1. not present; 2. trunk member; 3. unknown*/
               else
               {
                  if (!XSTP_POM_GetPortSpanningTreeStatus(lport, &status))
                     continue;
                  if (status == VAL_staPortSystemStatus_disabled)
                     continue;

#if (CLI_SUPPORT_PORT_NAME == 1)
                  {
                     UI8_T name[MAXSIZE_ifName+1] = {0};
                     CLI_LIB_Ifindex_To_Name(lport,name);
                     SYSFUN_Sprintf(buff, "%s Information\r\n", name);
                     PROCESS_MORE(buff);
                  }
#else
                  SYSFUN_Sprintf(buff, "Eth  %lu/%2lu Information\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                  PROCESS_MORE(buff);
#endif

                  PROCESS_MORE("---------------------------------------------------------------\r\n");
                  if ((line_num = Show_XSTP_One_Port(mstid, lport, line_num)) == JUMP_OUT_MORE)
                  {
                     return CLI_NO_ERROR;
                  }
                  else if (line_num == EXIT_SESSION_MORE)
                  {
                     return CLI_EXIT_SESSION;
                  }
               }
            }
         }/*end of unit*/

         /*trunk*/
         while (TRK_PMGR_GetNextTrunkId(&trunk_id))
         {
            UI32_T trunk_ifindex = 0;
            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

            if (TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
            {
               UI32_T status;

               if (!XSTP_POM_GetPortSpanningTreeStatus(trunk_ifindex, &status))
                  continue;
               if (status == VAL_staPortSystemStatus_disabled)
                  continue;

               SYSFUN_Sprintf(buff, "Trunk%lu   Information\r\n",(unsigned long)trunk_id);
               PROCESS_MORE(buff);
               PROCESS_MORE("---------------------------------------------------------------\r\n");
               if((line_num = Show_XSTP_One_Port(mstid, trunk_ifindex, line_num)) == JUMP_OUT_MORE)
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
         break;


      default:
         return CLI_ERR_INTERNAL;
      } /* switch(arg[0][0]) */
   }
   return CLI_NO_ERROR;
}

static char *Show_XSTP_Brief_GetPortStateStr(
    UI32_T  port_state)
{
    char    *ret_str_p;
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    switch(port_state)
    {
    case XSTP_TYPE_PORT_STATE_DISABLED:
        ret_str_p = "DSB";
        break;
    case XSTP_TYPE_PORT_STATE_BLOCKING:
        ret_str_p = "BLK";
        break;
    case XSTP_TYPE_PORT_STATE_LISTENING:
        ret_str_p = "LSN";
        break;
    case XSTP_TYPE_PORT_STATE_LEARNING:
        ret_str_p = "LRN";
        break;
    case XSTP_TYPE_PORT_STATE_FORWARDING:
        ret_str_p = "FWD";
        break;
    case XSTP_TYPE_PORT_STATE_BROKEN:
        ret_str_p = "BKN";
        break;
    default:
        ret_str_p = "UKN";
        break;
    }
#else
    switch(port_state)
    {
    case VAL_dot1dStpPortState_disabled:
        ret_str_p = "DSB";
        break;
    case VAL_dot1dStpPortState_blocking:
        ret_str_p = "BLK";
        break;
    case VAL_dot1dStpPortState_listening:
        ret_str_p = "LSN";
        break;
    case VAL_dot1dStpPortState_learning:
        ret_str_p = "LRN";
        break;
    case VAL_dot1dStpPortState_forwarding:
        ret_str_p = "FWD";
        break;
    case VAL_dot1dStpPortState_broken:
        ret_str_p = "BKN";
        break;
    default:
        ret_str_p = "UKN";
        break;
    }
#endif /* #if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP) */

    return ret_str_p;
}

static char *Show_XSTP_Brief_GetPortRoleStr(
    UI32_T  port_role)
{
    char    *ret_str_p;

    switch(port_role)
    {
    case XSTP_TYPE_PORT_ROLE_DISABLED:
        ret_str_p = "DISB";
        break;
    case XSTP_TYPE_PORT_ROLE_ROOT:
        ret_str_p = "ROOT";
        break;
    case XSTP_TYPE_PORT_ROLE_DESIGNATED:
        ret_str_p = "DESG";
        break;
    case XSTP_TYPE_PORT_ROLE_ALTERNATE:
        ret_str_p = "ALTN";
        break;
    case XSTP_TYPE_PORT_ROLE_BACKUP:
        ret_str_p = "BKUP";
        break;
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    case XSTP_TYPE_PORT_ROLE_MASTER:
        ret_str_p = "MAST";
        break;
#endif
    default:
        ret_str_p = "UNKN";
        break;
    }

    return ret_str_p;
}

#if 0
static UI32_T Show_XSTP_Brief_One_Instance_Help(
    UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    sprintf(buff, "%-9s : %s\r\n",
        "State", "Blocking (BLK), Broken (BKN), Disabled (DSB), Discarding (DSC),");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s   %s\r\n",
        "", "Forwarding (FWD), Learning (LRN), Listening (LSN), Unknown (UKN).");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s : %s\r\n",
        "Role", "Alternate (ALTN), Backup (BKUP), Designated (DESG),");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s   %s\r\n",
        "","Disabled (DISB), Master (MAST), Root (ROOT), Unknown (UNKN).");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-9s : %s\r\n",
        "Oper Edge", "Disabled (DIS), Enabled (EN).");
    PROCESS_MORE_FUNC(buff);

    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}
#endif

/* Sample:
 *
 * Spanning tree brief for instance 0*
 * Spanning Tree Mode             : MSTP
 * Spanning Tree Enabled/Disabled : Enabled
 * Designated Root                : 32768.00.0030F1BD71C4
 * Current Root Port(Eth)         : 0/0
 * Current Root Cost              : 0
 *
 * *"Spanning tree interface..." appears when MSTP mode is configured.
 */
static UI32_T Show_XSTP_Brief_One_Instance_GlobalInfo(
    BOOL_T is_mstp, UI32_T  mode, UI32_T  mstid, UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (TRUE == is_mstp)
    {
        sprintf(buff, "Spanning tree brief for instance %ld\r\n", (long)mstid);
        PROCESS_MORE_FUNC(buff);
    }

    {
        UI32_T  stp_status = 0;
        char    *stp_mode_str_p;
        char    *stp_sts_str_p = "Disabled";

        switch(mode)
        {
        case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
           stp_mode_str_p = "STP";
           break;
        case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
           stp_mode_str_p = "RSTP";
           break;
        case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
           stp_mode_str_p = "MSTP";
           break;
        default:
           stp_mode_str_p = "None";
           break;
        }

        SYSFUN_Sprintf(buff, "%-30s %c %s\r\n",
            "Spanning Tree Mode", ':', stp_mode_str_p);
        PROCESS_MORE_FUNC(buff);

        if (SYS_TYPE_GET_RUNNING_CFG_FAIL !=
                XSTP_POM_GetRunningSystemSpanningTreeStatus(&stp_status))
        {
            if (VAL_ifAdminStatus_up == stp_status)
                stp_sts_str_p = "Enabled";

            SYSFUN_Sprintf(buff, "%-30s %c %s\r\n",
                "Spanning Tree Enabled/Disabled", ':', stp_sts_str_p);
            PROCESS_MORE_FUNC(buff);
        }
    }

    {
        XSTP_MGR_BridgeIdComponent_T    designated_root;

        XSTP_POM_GetDesignatedRoot(mstid, &designated_root);
        switch(mode)
        {
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
        case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
            SYSFUN_Sprintf(buff, "%-30s %c %d.%02d.%02X%02X%02X%02X%02X%02X\r\n",
                "Designated Root", ':',
                designated_root.priority, designated_root.system_id_ext,
                designated_root.addr[0], designated_root.addr[1],
                designated_root.addr[2], designated_root.addr[3],
                designated_root.addr[4], designated_root.addr[5]);
            PROCESS_MORE_FUNC(buff);
            break;
#endif /* #if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP) */
        case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
        case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
            SYSFUN_Sprintf(buff, "%-30s %c %d.%02X%02X%02X%02X%02X%02X\r\n",
                "Designated Root", ':',
                designated_root.priority,
                designated_root.addr[0], designated_root.addr[1],
                designated_root.addr[2], designated_root.addr[3],
                designated_root.addr[4], designated_root.addr[5]);
            PROCESS_MORE_FUNC(buff);
            break;

        default:
            break;
        }
    }

    {
        XSTP_MGR_Dot1dStpEntry_T  entry;
        UI32_T unit=0, port=0, trunk_id=0;

        if (TRUE == XSTP_PMGR_GetDot1dMstEntry(mstid, &entry))
        {
            if(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_LogicalPortToUserPort(entry.dot1d_stp_root_port, &unit, &port, &trunk_id))
            {
              if(trunk_id==0)
              {
                #if(SYS_CPNT_STACKING == TRUE)
                SYSFUN_Sprintf(buff, "%-30s %c %ld/%ld\r\n",
                    "Current Root Port (Eth)", ':', (long)unit, (long)port);
                #else
                SYSFUN_Sprintf(buff, "%-30s %c %d\r\n",
                    "Current Root Port", ':', entry.dot1d_stp_root_port);
                #endif
              }
              else
              {
                SYSFUN_Sprintf(buff, "%-30s %c %ld\r\n",
                    "Current Root Port (Pch)", ':', (long)trunk_id);
              }
            }
            else
            {
                SYSFUN_Sprintf(buff, "%-30s %c %d\r\n",
                               "Current Root Port", ':', entry.dot1d_stp_root_port);
            }
            PROCESS_MORE_FUNC(buff);
            SYSFUN_Sprintf(buff, "%-30s %c %ld\r\n",
                "Current Root Cost", ':', (long)entry.dot1d_stp_root_cost);
            PROCESS_MORE_FUNC(buff);
        }
    }
    PROCESS_MORE_FUNC("\r\n");

    return line_num;
}


/* Sample:
 *
 * Interface Pri Designated            Designated Oper     STP    Role State Oper
 *               Bridge ID             Port ID    Cost     Status            Edge
 * --------- --- --------------------- ---------- -------- ------ ---- ----- ----
 * Eth 1/ 1  128 32768.00.001122334455 128.1      10000000 EN     ROOT FWD   Yes
 * Eth 1/ 2  128 32768.00.001122334455 128.2      10000000 DIS    DISB FWD   No
 * Eth 1/ 3  128 32768.00.001122334455 128.3      10000000 EN     ALTN BLK   Yes
 * Trunk 1   128 32768.00.001122334455 128.5       1000000 DIS    DESG FWD   No
 */
static UI32_T Show_XSTP_Brief_One_Instance_Titile(
    BOOL_T is_mstp, UI32_T  mstid, UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};

    sprintf(buff, "%-9s %-3s %-21s %-10s %-8s %-6s %-4s %-5s %-4s\r\n",
        "Interface", "Pri", "Designated", "Designated",
        "Oper", "STP", "Role", "State", "Oper");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-3s %-21s %-10s %-8s %-6s %-4s %-5s %-4s\r\n",
        "", "", "Bridge ID", "Port ID", "Cost", "Status", "",  "", "Edge");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-9s %-3s %-21s %-10s %-8s %-6s %-4s %-5s %-4s\r\n",
        "---------", "---", "---------------------", "----------",
        "--------", "------", "----", "-----", "----");
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_XSTP_Brief_One_Instance_One_Port(
    UI32_T mode, UI32_T  mstid, UI32_T lport, UI32_T line_num, char *buff)
{
    UI32_T                          unit, port, trunk_id;
    UI32_T                          port_state, port_role,
                                    port_stp_status =0, oper_cost = 0;
    char                            *opr_edge_str_p, *state_str_p,
                                    *role_str_p, *port_stp_sts_p;
    SWCTRL_Lport_Type_T             port_type;
    XSTP_MGR_Dot1dStpPortEntry_T    stp_port_entry;
    XSTP_MGR_Dot1dStpExtPortEntry_T ext_port_entry;
    XSTP_MGR_PortIdComponent_T      designated_port;
    XSTP_MGR_BridgeIdComponent_T    designated_bridge;
    char                            inf_str[10], des_port_id[12],
                                    des_brg_id[22];

    port_type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    if (  (port_type == SWCTRL_LPORT_TRUNK_PORT)
        ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
       )
    {
        stp_port_entry.dot1d_stp_port = (UI16_T)lport;

        if (  (FALSE == XSTP_POM_GetDot1dMstPortEntry(mstid, &stp_port_entry))
            ||(FALSE == XSTP_POM_GetPortDesignatedPort(lport, mstid, &designated_port))
            ||(FALSE == XSTP_POM_GetPortDesignatedBridge(lport, mstid, &designated_bridge))
            ||(FALSE == XSTP_POM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port_entry))
            ||(XSTP_TYPE_RETURN_OK != XSTP_POM_GetMstPortState(lport, mstid, &port_state))
            ||(XSTP_TYPE_RETURN_OK != XSTP_POM_GetMstPortRole(lport, mstid, &port_role))
           )
        {
            return line_num;
      }
   }
    else
    {
        return line_num;
    }

    if (port_type == SWCTRL_LPORT_NORMAL_PORT)
    {
        sprintf(inf_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
    }
    else
    {
        sprintf(inf_str, "Trunk %lu", (unsigned long)trunk_id);
    }

    sprintf(des_port_id, "%d.%d", designated_port.priority,
                                  designated_port.port_num);
    switch(mode)
    {
    case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
        sprintf(des_brg_id,"%d.%02d.%02X%02X%02X%02X%02X%02X",
        designated_bridge.priority, designated_bridge.system_id_ext,
        designated_bridge.addr[0],  designated_bridge.addr[1],
        designated_bridge.addr[2],  designated_bridge.addr[3],
        designated_bridge.addr[4],  designated_bridge.addr[5]);
        break;

    case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
    case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
        sprintf(des_brg_id,"%d.%02X%02X%02X%02X%02X%02X",
        designated_bridge.priority,
        designated_bridge.addr[0],  designated_bridge.addr[1],
        designated_bridge.addr[2],  designated_bridge.addr[3],
        designated_bridge.addr[4],  designated_bridge.addr[5]);
        break;

    default:
        break;
    }

    /* port cost will shows external oper cost for mst id == 0
     *                      internal oper cost for mst id != 0
     */
    if (0 == mstid)
    {
        XSTP_POM_GetPortOperPathCost(lport, &oper_cost);
    }
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    else
    {
        XSTP_POM_GetMstPortOperPathCost(lport, mstid, &oper_cost);
    }
#endif

    if (  (TRUE == XSTP_POM_GetPortSpanningTreeStatus(lport, &port_stp_status))
        &&(VAL_staPortSystemStatus_enabled == port_stp_status)
       )
    {
        port_stp_sts_p = "EN";
    }
    else
    {
        port_stp_sts_p = "DIS";
    }

    role_str_p  = Show_XSTP_Brief_GetPortRoleStr(port_role);
    state_str_p = Show_XSTP_Brief_GetPortStateStr(port_state);

    if (VAL_staPortOperEdgePort_true == ext_port_entry.dot1d_stp_port_oper_edge_port)
        opr_edge_str_p = "Yes";
    else
        opr_edge_str_p = "No";

    SYSFUN_Sprintf(buff, "%-9s %3d %-21s %-10s %8ld %-6s %-4s %-5s %-4s\r\n",
        inf_str,        stp_port_entry.dot1d_stp_port_priority,
        des_brg_id,     des_port_id,    (long)oper_cost,      port_stp_sts_p,
        role_str_p,     state_str_p,    opr_edge_str_p
        );
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_XSTP_Brief_One_Instance(
    UI32_T mode, UI32_T mstid, UI32_T line_num)
{
    UI32_T  j, i, max_port_num, lport, trunk_id =0;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    for (j=0; STKTPLG_POM_GetNextUnit(&j); )
    {
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
        /*eth*/
        for (i = 1 ; i <= max_port_num;i++)
        {
           UI32_T               verify_unit = j;
           UI32_T               verify_port = i;
           CLI_API_EthStatus_T  verify_ret;

           if ( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                continue; /*1. not present; 2. trunk member; 3. unknown*/

           line_num = Show_XSTP_Brief_One_Instance_One_Port(
                        mode, mstid, lport, line_num, buff);
           if (JUMP_OUT_MORE == line_num)
           {
               return line_num;
           }
           else if (EXIT_SESSION_MORE == line_num)
           {
               return line_num;
           }
        } /* for (i = 1 ; i <= max_port_num;i++) */
    }/* for (j=0; STKTPLG_POM_GetNextUnit(&j); ) */

    /*trunk*/
    while (TRK_PMGR_GetNextTrunkId(&trunk_id))
    {
        UI32_T  trunk_ifindex = 0;

        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

        if (TRK_PMGR_GetTrunkMemberCounts(trunk_id) != 0)
        {
            line_num = Show_XSTP_Brief_One_Instance_One_Port(
                            mode, mstid, trunk_ifindex, line_num, buff);
            if (JUMP_OUT_MORE == line_num)
            {
                return line_num;
            }
            else if (EXIT_SESSION_MORE == line_num)
            {
                return line_num;
            }
        }
    }

    PROCESS_MORE_FUNC("\r\n");
    return line_num;
}

/* command: show spanning-tree brief
 *          show spanning-tree mst instance_id brief
 *
 *          arg pointer to instance_id or NULL
 */
static UI32_T cli_api_show_spanningtree_brief(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#define _CHECK_LINE_NUM(l_num)              \
    if (JUMP_OUT_MORE == l_num)             \
        return CLI_NO_ERROR;                \
    else if (EXIT_SESSION_MORE == l_num)    \
        return CLI_EXIT_SESSION;

    UI32_T  mst_id =0, line_num=0, mode;
    BOOL_T  is_mst = TRUE, is_show_all = TRUE;

    if ((NULL != arg) && (NULL != arg[0]))
    {
        mst_id = atoi(arg[0]);

        /* skip the test for mst 0,
         * bcz mst 0 test failed when spanning tree is disabled
         */
        if (  (0 != mst_id)
            &&(FALSE == XSTP_POM_IsMstInstanceExisting(mst_id))
           )
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("MSTP instance %ld does not exist.\r\n", (long)mst_id);
#endif
            return CLI_NO_ERROR;
        }

        is_show_all = FALSE;
    }

    XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);

    if (  (XSTP_TYPE_STP_PROTOCOL_VERSION_ID == mode)
        ||(XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID== mode)
       )
    {
        is_mst = FALSE;
    }

#if 0
    line_num = Show_XSTP_Brief_One_Instance_Help(line_num);
    _CHECK_LINE_NUM(line_num);
#endif

    do
    {
        line_num = Show_XSTP_Brief_One_Instance_GlobalInfo(
                        is_mst, mode, mst_id, line_num);
        _CHECK_LINE_NUM(line_num);

        line_num = Show_XSTP_Brief_One_Instance_Titile(
                        is_mst, mst_id, line_num);
        _CHECK_LINE_NUM(line_num);

        line_num = Show_XSTP_Brief_One_Instance(mode, mst_id, line_num);
        _CHECK_LINE_NUM(line_num);

    } while (  (TRUE == is_mst) && (TRUE == is_show_all)
             &&(XSTP_POM_GetNextExistedInstance(&mst_id)));

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  return_vlaue;
    char  *str_list;

    if( (str_list = calloc(512*4, sizeof(UI8_T))) == NULL )
    {
       return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    /* 2006/06/28
     * ES4649-38-00189: free memory before return
     */
    return_vlaue = cli_api_show_spanningtree_macro(cmd_idx, arg, ctrl_P, str_list);

    free(str_list);
    return return_vlaue;
}

/* 2006/06/28
 * ES4649-38-00189: free memory before return
 */
static UI32_T show_xstp_one_instance_macro(UI32_T mstid, BOOL_T is_mstp, UI32_T line_num, char *str_list)
{
   UI8_T  stp_mode[5] = {0};
   UI32_T status = 0;
   UI32_T mode = 0;
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   XSTP_MGR_Dot1dStpEntry_T  entry;
   XSTP_MGR_BridgeIdComponent_T designated_root;
   memset(&entry , 0 , sizeof(XSTP_MGR_Dot1dStpEntry_T));
   XSTP_POM_GetRunningSystemSpanningTreeStatus(&status);
   XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);

   switch(mode)
   {
   case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
      strcpy((char*)stp_mode,"STP");
      break;

   case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
      strcpy((char*)stp_mode,"RSTP");
      break;

   case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
      strcpy((char*)stp_mode,"MSTP");
      break;

   default:
      strcpy((char*)stp_mode,"None");
      break;
   }
   if (!XSTP_PMGR_GetDot1dMstEntry(mstid, &entry))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get spanning-tree attribs\r\n");
#endif
      return CLI_NO_ERROR;
   }


   PROCESS_MORE_FUNC(("Spanning Tree Information\r\n"));
   PROCESS_MORE_FUNC(("---------------------------------------------------------------\r\n"));
   SYSFUN_Sprintf((char*)buff, " Spanning Tree Mode              : %s\r\n",stp_mode);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Spanning Tree Enabled/Disabled  : %s\r\n",status == VAL_ifAdminStatus_up ? "Enabled" : "Disabled");
   PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   if (is_mstp)
   {
      XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;
      char  vlan_list[512] = {0};

      if (XSTP_POM_GetMstpInstanceVlanConfiguration(mstid,&mstp_instance_entry))
      {
         memcpy(vlan_list,mstp_instance_entry.mstp_instance_vlans_mapped,sizeof(UI8_T)*128);
         memcpy(vlan_list+128,mstp_instance_entry.mstp_instance_vlans_mapped2k,sizeof(UI8_T)*128);
         memcpy(vlan_list+256,mstp_instance_entry.mstp_instance_vlans_mapped3k,sizeof(UI8_T)*128);
         memcpy(vlan_list+384,mstp_instance_entry.mstp_instance_vlans_mapped4k,sizeof(UI8_T)*128);
         CLI_LIB_MSB_To_LSB(vlan_list,512);
         CLI_LIB_Bitmap_To_List(vlan_list,str_list,512*4,SYS_DFLT_DOT1QMAXVLANID,FALSE);
      }

      SYSFUN_Sprintf((char*)buff, " Instance                        : %lu\r\n",(unsigned long)mstid);
      PROCESS_MORE_FUNC(buff);
      {  /*because must show per line, so spilt the str_list to show */
         UI32_T str_len = strlen((char*)str_list);
         UI32_T i = 0;
         UI8_T  print_str[44+1] = {0};

         for (i = 0; i < ((str_len+43) / 44); i++)
         {
            memcpy(print_str, str_list+i*44, sizeof(UI8_T)*44);
            if (i == 0)
            {
               SYSFUN_Sprintf((char*)buff, " VLANs Configured                : %s\r\n",print_str);
               PROCESS_MORE_FUNC(buff);
            }
            else
            {
               SYSFUN_Sprintf((char*)buff, "                                   %s\r\n",print_str);
               PROCESS_MORE_FUNC(buff);
            }
         }
      }
   }
#endif
   SYSFUN_Sprintf((char*)buff, " Priority                        : %lu\r\n",(unsigned long)entry.dot1d_stp_priority);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Bridge Hello Time (sec.)        : %d\r\n",entry.dot1d_stp_bridge_hello_time/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Bridge Max. Age (sec.)          : %d\r\n",entry.dot1d_stp_bridge_max_age/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Bridge Forward Delay (sec.)     : %d\r\n",entry.dot1d_stp_bridge_forward_delay/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Root Hello Time (sec.)          : %d\r\n",entry.dot1d_stp_hello_time/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Root Max. Age (sec.)            : %d\r\n",entry.dot1d_stp_max_age/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Root Forward Delay (sec.)       : %d\r\n",entry.dot1d_stp_forward_delay/100);
   PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   if (is_mstp)
   {
      UI32_T max_hop;
      XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;

      memset(&mstp_instance_entry, 0, sizeof(mstp_instance_entry));
      if (XSTP_POM_GetMstpMaxHop(&max_hop))
      {
         SYSFUN_Sprintf((char*)buff, " Max. Hops                       : %lu\r\n",(unsigned long)max_hop);
         PROCESS_MORE_FUNC(buff);
      }

      if (XSTP_POM_GetMstpInstanceVlanConfiguration(mstid, &mstp_instance_entry))
      {
         SYSFUN_Sprintf((char*)buff, " Remaining Hops                  : %lu\r\n",(unsigned long)mstp_instance_entry.mstp_instance_remaining_hop_count);
         PROCESS_MORE_FUNC(buff);
      }
   }
#endif
   {
      UI32_T mode;
      XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);
      XSTP_POM_GetDesignatedRoot(mstid, &designated_root);

      switch(mode)
      {
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
         SYSFUN_Sprintf((char*)buff," Designated Root                 : %d.%d.%02X%02X%02X%02X%02X%02X\r\n",designated_root.priority,designated_root.system_id_ext,
         designated_root.addr[0],designated_root.addr[1],designated_root.addr[2],
         designated_root.addr[3],designated_root.addr[4],designated_root.addr[5]);
         PROCESS_MORE_FUNC(buff);
         break;
#endif
      case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
      case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
         SYSFUN_Sprintf((char*)buff," Designated Root                 : %d.%02X%02X%02X%02X%02X%02X\r\n",designated_root.priority,
         designated_root.addr[0],designated_root.addr[1],designated_root.addr[2],
         designated_root.addr[3],designated_root.addr[4],designated_root.addr[5]);
         PROCESS_MORE_FUNC(buff);
         break;

      default:
         break;
      }
   }
   {
       UI32_T unit=0, port=0, trunk_id=0;

       if(SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_POM_LogicalPortToUserPort(entry.dot1d_stp_root_port, &unit, &port, &trunk_id))
       {
         if(trunk_id==0)
         {
           #if(SYS_CPNT_STACKING == TRUE)
           SYSFUN_Sprintf(buff, " Current Root Port(Eth)          : %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
           #else
           SYSFUN_Sprintf(buff, " Current Root Port(Eth)          : %lu\r\n", (unsigned long)entry.dot1d_stp_root_port);
           #endif
         }
         else
         {
           SYSFUN_Sprintf(buff, " Current Root Port(Pch)          : %lu\r\n", (unsigned long)trunk_id);
         }
     }
     else
         SYSFUN_Sprintf((char*)buff, " Current Root Port               : %d\r\n",entry.dot1d_stp_root_port);
   }
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Current Root Cost               : %lu\r\n",(unsigned long)entry.dot1d_stp_root_cost);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Number of Topology Changes      : %lu\r\n",(unsigned long)entry.dot1d_stp_top_changes);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Last Topology Change Time (sec.): %lu\r\n",(unsigned long)entry.dot1d_stp_time_since_topology_change/100);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Transmission Limit              : %d\r\n",entry.dot1d_stp_tx_hold_count);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff, " Path Cost Method                : %s\r\n",(entry.dot1d_stp_path_cost_default == VAL_staPathCostMethod_long)?("Long"):("Short"));
   PROCESS_MORE_FUNC(buff);
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
   {
       UI32_T cisco_prestandard_status;

       XSTP_POM_GetCiscoPrestandardCompatibility(&cisco_prestandard_status);
       SYSFUN_Sprintf(buff, " Cisco Prestandard               : %s\r\n",(cisco_prestandard_status== XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED)?("Enabled"):("Disabled"));
       PROCESS_MORE_FUNC(buff);
   }
#endif

   PROCESS_MORE_FUNC(("---------------------------------------------------------------\r\n"));

   return line_num;
}

static UI32_T Show_XSTP_One_Instance(UI32_T mstid, BOOL_T is_mstp, UI32_T line_num)
{
    char   *str_list;
    UI32_T return_value;

    if( (str_list = calloc(512*4, sizeof(UI8_T))) == NULL )
    {
     return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    return_value = show_xstp_one_instance_macro(mstid, is_mstp, line_num, str_list);

    free(str_list);

    return return_value;
}



static UI32_T Show_XSTP_One_Port(UI32_T mstid,UI32_T lport,UI32_T line_num)
{
   enum SHOW_STATUS
   {
      SHOW_STATUS_DISABLE = 0,
      SHOW_STATUS_ENABLE,
      SHOW_STATUS_BLOCKING,
      SHOW_STATUS_LISTENING,
      SHOW_STATUS_LEARNING,
      SHOW_STATUS_FORWARDING,
      SHOW_STATUS_BROKEN,
      SHOW_MST_PORT_POINT,
      SHOW_MST_PORT_SHARED,
      SHOW_MST_PORT_AUTO,
      SHOW_MST_ROLE_ROOT,
      SHOW_MST_ROLE_DESIGNATE,
      SHOW_MST_ROLE_ALTERNATE,
      SHOW_MST_ROLE_BACKUP,
      SHOW_MST_ROLE_MASTER,
      SHOW_STATUS_DISCARDING
   };


   char *str[] = {"Disabled",/*0*/  //to remove warning, change type from UI8_T* to char *
                   "Enabled",
                   "Blocking",
                   "Listening",
                   "Learning",
                   "Forwarding",
                   "Broken",
                   "Point-to-point",
                   "Shared",
                   "Auto",
                   "Root",
                   "Designate",
                   "Alternate",
                   "Backup",
                   "Master",
                   "Discarding"
                  };

   XSTP_MGR_Dot1dStpPortEntry_T  stp_port_entry;
   XSTP_MGR_Dot1dStpExtPortEntry_T ext_port_entry;
   XSTP_MGR_BridgeIdComponent_T designated_root, designated_bridge;
   XSTP_MGR_PortIdComponent_T designated_port;
   UI32_T state = 0;
   UI32_T link_type_state = 0;
   UI32_T role = 0;
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T spanning_status = 0;
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
   UI32_T rg_status = 0;
#endif
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
   UI32_T bpdu_guard_status = 0;
   UI32_T bpdu_guard_auto_recovery;
   UI32_T bpdu_guard_auto_recovery_interval;
#endif
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
   UI32_T bpdu_filter_status = 0;
#endif

   stp_port_entry.dot1d_stp_port = (UI16_T)lport;

   if (!XSTP_POM_GetPortSpanningTreeStatus(lport,&spanning_status))
   {
      CLI_LIB_PrintStr("Failed to get specific port attrib\r\n");
      return JUMP_OUT_MORE;
   }

   if (!XSTP_POM_GetDot1dMstPortEntry(mstid, &stp_port_entry))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get specific port attrib\r\n");
#endif
      return JUMP_OUT_MORE;
   }

   if (!XSTP_POM_GetDot1dMstExtPortEntry(mstid, lport, &ext_port_entry))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get specific MST port entry info\r\n");
#endif
      return JUMP_OUT_MORE;
   }

   if (XSTP_POM_GetMstPortRole(lport, mstid, &role) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get specific MST port role\r\n");
#endif
      return JUMP_OUT_MORE;
   }

   switch(role)
   {
   case XSTP_TYPE_PORT_ROLE_DISABLED:
      role = SHOW_STATUS_DISABLE;
      break;

   case XSTP_TYPE_PORT_ROLE_ROOT:
      role = SHOW_MST_ROLE_ROOT;
      break;

   case XSTP_TYPE_PORT_ROLE_DESIGNATED:
      role = SHOW_MST_ROLE_DESIGNATE;
      break;

   case XSTP_TYPE_PORT_ROLE_ALTERNATE:
      role = SHOW_MST_ROLE_ALTERNATE;
      break;

   case XSTP_TYPE_PORT_ROLE_BACKUP:
      role = SHOW_MST_ROLE_BACKUP;
      break;

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   case XSTP_TYPE_PORT_ROLE_MASTER:
      role = SHOW_MST_ROLE_MASTER;
      break;
#endif
    default:
       role = SHOW_STATUS_DISABLE;
       break;
   }

   if (XSTP_POM_GetMstPortState(lport, mstid, &state) != XSTP_TYPE_RETURN_OK)
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get specific MST port state\r\n");
#endif
      return JUMP_OUT_MORE;
   }

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
   switch(state)
   {
   case XSTP_TYPE_PORT_STATE_LISTENING:
      state = SHOW_STATUS_LISTENING;
      break;

   case XSTP_TYPE_PORT_STATE_LEARNING:
      state = SHOW_STATUS_LEARNING;
      break;

   case XSTP_TYPE_PORT_STATE_DISCARDING:
      state = SHOW_STATUS_DISCARDING;
      break;

   default:
      break;
   }

#else

   switch(state)
   {
   case VAL_dot1dStpPortState_disabled:
      state = SHOW_STATUS_DISABLE;
      break;

   case VAL_dot1dStpPortState_blocking:
      state = SHOW_STATUS_BLOCKING;
      break;

   case VAL_dot1dStpPortState_listening:
      state = SHOW_STATUS_LISTENING;
      break;

   case VAL_dot1dStpPortState_learning:
      state = SHOW_STATUS_LEARNING;
      break;

   case VAL_dot1dStpPortState_forwarding:
      state = SHOW_STATUS_FORWARDING;
      break;

   case VAL_dot1dStpPortState_broken:
      state = SHOW_STATUS_BROKEN;
      break;

   default:
      break;
   }
#endif

   switch(ext_port_entry.dot1d_stp_port_admin_point_to_point)
   {
   case VAL_staPortAdminPointToPoint_forceTrue:
      link_type_state = SHOW_MST_PORT_POINT;
      break;

   case VAL_staPortAdminPointToPoint_forceFalse:
      link_type_state = SHOW_MST_PORT_SHARED;
      break;

   case VAL_staPortAdminPointToPoint_auto:
      link_type_state = SHOW_MST_PORT_AUTO;
      break;

   default:
      break;
   }

   SYSFUN_Sprintf((char*)buff," Admin Status                      : %s\r\n",
       (stp_port_entry.dot1d_stp_port_enable == VAL_dot1dStpPortEnable_enabled)?("Enabled"):("Disabled"));
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff," Role                              : %s\r\n",str[role]);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff," State                             : %s\r\n",str[state]);
   PROCESS_MORE_FUNC(buff);
   {
      UI32_T mode;
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      UI32_T i_oper_cost = 0;
      UI32_T i_admin_cost = 0;
#endif
      UI32_T e_oper_cost = 0;
      UI32_T e_admin_cost = 0;

      XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);
      XSTP_POM_GetPortAdminPathCost(lport, &e_admin_cost);
      XSTP_POM_GetPortOperPathCost(lport, &e_oper_cost);
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      XSTP_POM_GetMstPortAdminPathCost(lport, mstid, &i_admin_cost);
      XSTP_POM_GetMstPortOperPathCost(lport, mstid, &i_oper_cost);
#endif

      switch(mode)
      {
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
      case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
           SYSFUN_Sprintf((char*)buff," External Admin Path Cost          : %lu\r\n",(unsigned long)e_admin_cost);
           PROCESS_MORE_FUNC(buff);
           SYSFUN_Sprintf((char*)buff," Internal Admin Path Cost          : %lu\r\n",(unsigned long)i_admin_cost);
           PROCESS_MORE_FUNC(buff);
           SYSFUN_Sprintf((char*)buff," External Oper Path Cost           : %lu\r\n",(unsigned long)e_oper_cost);
           PROCESS_MORE_FUNC(buff);
           SYSFUN_Sprintf((char*)buff," Internal Oper Path Cost           : %lu\r\n",(unsigned long)i_oper_cost);
           PROCESS_MORE_FUNC(buff);

         break;
#endif
      case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
      case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
         SYSFUN_Sprintf((char*)buff," Admin Path Cost                   : %lu\r\n",(unsigned long)e_admin_cost);
         PROCESS_MORE_FUNC(buff);

         SYSFUN_Sprintf((char*)buff," Oper Path Cost                    : %lu\r\n",(unsigned long)e_oper_cost);
         PROCESS_MORE_FUNC(buff);
         break;

      default:
         break;
      }
   }
   SYSFUN_Sprintf((char*)buff," Priority                          : %d\r\n",stp_port_entry.dot1d_stp_port_priority);
   PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff," Designated Cost                   : %lu\r\n",(unsigned long)stp_port_entry.dot1d_stp_port_designated_cost);
   PROCESS_MORE_FUNC(buff);
   if (!XSTP_POM_GetPortDesignatedPort(lport, mstid, &designated_port))
   {
      CLI_LIB_PrintStr("Failed to get specific designated port\r\n");
      return JUMP_OUT_MORE;
   }
   SYSFUN_Sprintf((char*)buff," Designated Port                   : %d.%d\r\n",designated_port.priority,designated_port.port_num);
   PROCESS_MORE_FUNC(buff);
    {
      UI32_T mode;
      XSTP_POM_GetRunningSystemSpanningTreeVersion(&mode);

      if (!XSTP_POM_GetDesignatedRoot(mstid, &designated_root))
      {
         CLI_LIB_PrintStr("Failed to get specific designated root\r\n");
         return JUMP_OUT_MORE;
      }

      if (!XSTP_POM_GetPortDesignatedBridge(lport, mstid, &designated_bridge))
      {
         CLI_LIB_PrintStr("Failed to get specific designated bridge\r\n");
         return JUMP_OUT_MORE;
      }

      switch(mode)
      {
      case XSTP_TYPE_MSTP_PROTOCOL_VERSION_ID:
         SYSFUN_Sprintf((char*)buff," Designated Root                   : %d.%d.%02X%02X%02X%02X%02X%02X\r\n",
            designated_root.priority,designated_root.system_id_ext,
            designated_root.addr[0],designated_root.addr[1],designated_root.addr[2],
            designated_root.addr[3],designated_root.addr[4],designated_root.addr[5]);
         PROCESS_MORE_FUNC(buff);
         SYSFUN_Sprintf((char*)buff," Designated Bridge                 : %d.%d.%02X%02X%02X%02X%02X%02X\r\n",
            designated_bridge.priority,designated_bridge.system_id_ext,
            designated_bridge.addr[0],designated_bridge.addr[1],designated_bridge.addr[2],
            designated_bridge.addr[3],designated_bridge.addr[4],designated_bridge.addr[5]);
         PROCESS_MORE_FUNC(buff);
         break;

      case XSTP_TYPE_RSTP_PROTOCOL_VERSION_ID:
      case XSTP_TYPE_STP_PROTOCOL_VERSION_ID:
         SYSFUN_Sprintf((char*)buff," Designated Root                   : %d.%02X%02X%02X%02X%02X%02X\r\n",designated_root.priority,
            designated_root.addr[0],designated_root.addr[1],designated_root.addr[2],
            designated_root.addr[3],designated_root.addr[4],designated_root.addr[5]);
         PROCESS_MORE_FUNC(buff);
         SYSFUN_Sprintf((char*)buff," Designated Bridge                 : %d.%02X%02X%02X%02X%02X%02X\r\n",designated_bridge.priority,
            designated_bridge.addr[0],designated_bridge.addr[1],designated_bridge.addr[2],
            designated_bridge.addr[3],designated_bridge.addr[4],designated_bridge.addr[5]);
         PROCESS_MORE_FUNC(buff);
         break;

      default:
         break;
      }
   }

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_STA)
    SYSFUN_Sprintf((char*)buff," Fast Forwarding                   : %s\r\n",(ext_port_entry.dot1d_stp_port_oper_edge_port == VAL_staPortAdminEdgePort_true)?("Enabled"):("Disabled"));
    PROCESS_MORE_FUNC(buff);
#endif
    SYSFUN_Sprintf((char*)buff," Forward Transitions               : %d\r\n",stp_port_entry.dot1d_stp_port_forward_transitions);
    PROCESS_MORE_FUNC(buff);
#if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))
    #if (SYS_CPNT_STP_AUTO_EDGE_PORT == TRUE)
    if (ext_port_entry.dot1d_stp_port_admin_edge_port == VAL_staPortAdminEdgePortWithAuto_auto)
        SYSFUN_Sprintf(buff," Admin Edge Port                   : %s\r\n", "Auto");
    else
    #endif
    SYSFUN_Sprintf((char*)buff," Admin Edge Port                   : %s\r\n",(ext_port_entry.dot1d_stp_port_admin_edge_port == VAL_staPortAdminEdgePort_true)?("Enabled"):("Disabled"));
    PROCESS_MORE_FUNC(buff);
    SYSFUN_Sprintf((char*)buff," Oper Edge Port                    : %s\r\n",(ext_port_entry.dot1d_stp_port_oper_edge_port == VAL_staPortOperEdgePort_true)?("Enabled"):("Disabled"));
    PROCESS_MORE_FUNC(buff);
#endif /* #if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)) */
    SYSFUN_Sprintf((char*)buff," Admin Link Type                   : %s\r\n",str[link_type_state]);
    PROCESS_MORE_FUNC(buff);
    SYSFUN_Sprintf((char*)buff," Oper Link Type                    : %s\r\n",(ext_port_entry.dot1d_stp_port_oper_point_to_point == TRUE)?("Point-to-point"):("Shared"));
    PROCESS_MORE_FUNC(buff);
   SYSFUN_Sprintf((char*)buff," Spanning-Tree Status              : %s\r\n",(spanning_status == VAL_staPortSystemStatus_enabled)?("Enabled"):("Disabled"));
   PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    if (XSTP_POM_GetPortRootGuardStatus(lport, &rg_status) != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get root guard status\r\n");
        return JUMP_OUT_MORE;
    }

    SYSFUN_Sprintf((char*)buff, " Root Guard Status                 : %s\r\n",
        (rg_status == VAL_staPortRootGuardAdminStatus_enabled) ? ("Enabled") : ("Disabled"));
    PROCESS_MORE_FUNC(buff);
#endif

#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    if (XSTP_POM_GetPortBpduGuardStatus(lport, &bpdu_guard_status) != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get BPDU Guard status\r\n");
        return JUMP_OUT_MORE;
    }
    SYSFUN_Sprintf(buff, " BPDU Guard Status                 : %s\r\n",
        (bpdu_guard_status == XSTP_TYPE_PORT_BPDU_GUARD_ENABLED) ? ("Enabled") : ("Disabled"));
    PROCESS_MORE_FUNC(buff);
    if (XSTP_POM_GetPortBPDUGuardAutoRecovery(lport, &bpdu_guard_auto_recovery) != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get BPDU Guard auto recovery\r\n");
        return JUMP_OUT_MORE;
    }
    SYSFUN_Sprintf(buff, " BPDU Guard Auto Recovery          : %s\r\n",
        (bpdu_guard_auto_recovery == XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED) ? ("Enabled") : ("Disabled"));
    PROCESS_MORE_FUNC(buff);
    if (XSTP_POM_GetPortBPDUGuardAutoRecoveryInterval(lport, &bpdu_guard_auto_recovery_interval) != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get BPDU Guard auto recovery interval\r\n");
        return JUMP_OUT_MORE;
    }
    SYSFUN_Sprintf(buff, " BPDU Guard Auto Recovery Interval : %lu\r\n", (unsigned long)bpdu_guard_auto_recovery_interval);
    PROCESS_MORE_FUNC(buff);
#endif

#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    if (XSTP_POM_GetPortBpduFilterStatus(lport, &bpdu_filter_status) != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr("Failed to get BPDU Filter status\r\n");
        return JUMP_OUT_MORE;
    }
    SYSFUN_Sprintf(buff, " BPDU Filter Status                : %s\r\n",
        (bpdu_filter_status == XSTP_TYPE_PORT_BPDU_FILTER_ENABLED) ? ("Enabled") : ("Disabled"));
    PROCESS_MORE_FUNC(buff);
#endif
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
   SYSFUN_Sprintf(buff, " TC Propagate Stop                 : %s\r\n",
      (XSTP_POM_GetPortTcPropStop(lport)) ? ("Enabled") : ("Disabled"));
   PROCESS_MORE_FUNC(buff);
#endif
   PROCESS_MORE_FUNC(("\r\n"));
   return line_num;
}

/*
*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Port_MacAddr_Learning_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mac-learning"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Port_MacAddr_Learning_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_UI == TRUE)
#if (SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)
   UI32_T i;
   CLI_API_EthStatus_T verify_ret;
   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   UI32_T status1,status = 0;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_MACLEARNING:
      status = VAL_staPortMacAddrLearning_true;
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_MACLEARNING:
      status = VAL_staPortMacAddrLearning_false;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (status == VAL_staPortMacAddrLearning_true)
      status1 = TRUE;
   else
      status1 = FALSE;
   for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
   {
      UI32_T lport = 0;
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if(SWCTRL_PMGR_SetPortMACLearningStatus(lport, status1) != TRUE)
         {
#if (CLI_SUPPORT_PORT_NAME == 1)
           {
            UI8_T name[MAXSIZE_ifName+1] = {0};
            CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set macaddress learning status on ethernet %s\n", name);
#endif
            }
#else
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to set macaddress learning on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
          }
      }
    }

#endif /* (SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE) */
#endif /* #if (SYS_CPNT_AMTR_UI == TRUE) */
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Port_MacAddr_Learning_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "mac-learning"
 *            in interface port-channel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Port_MacAddr_Learning_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_UI == TRUE)
#if (SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T status1,status = 0;
   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
       case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_MACLEARNING:
          status = VAL_staPortMacAddrLearning_true;
          break;

       case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_MACLEARNING:
          status = VAL_staPortMacAddrLearning_false;
          break;

       default:
          return CLI_ERR_INTERNAL;
   }

   if (status == VAL_staPortMacAddrLearning_true)
     status1 = TRUE;
   else
     status1 = FALSE;

   if(SWCTRL_PMGR_SetPortMACLearningStatus(lport, status1) != TRUE)
   {
#if (SYS_CPNT_EH == TRUE)
       CLI_API_Show_Exception_Handeler_Msg();
#else
       CLI_LIB_PrintStr_1("Failed to set mac_learning status on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
   }

#endif /* (SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE) */
#endif /* #if (SYS_CPNT_AMTR_UI == TRUE) */
   return CLI_NO_ERROR;

}

/* ahten */

UI32_T CLI_API_MacAddressTable_AgingTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_UI == TRUE)
   UI32_T aging_time = 0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACADDRESSTABLE_AGINGTIME:
      aging_time = atoi((char*)arg[0]);

      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACADDRESSTABLE_AGINGTIME:
      aging_time = SYS_DFLT_L2_DYNAMIC_ADDR_AGING_TIME;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   if (aging_time != 0)
   {
      if (!AMTR_PMGR_SetDot1dTpAgingTime(aging_time))
      {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set aging time\r\n");
#endif
      }
      if (!AMTR_PMGR_SetAgingStatus(VAL_amtrMacAddrAgingStatus_enabled))
      {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else

         CLI_LIB_PrintStr("Failed to enable aging time\r\n");
#endif
      }
   }
   else
   {
      if (!AMTR_PMGR_SetAgingStatus(VAL_amtrMacAddrAgingStatus_disabled))
      {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else

         CLI_LIB_PrintStr("Failed to disable aging time\r\n");
#endif
      }
   }
#endif /* #if (SYS_CPNT_AMTR_UI == TRUE) */
   return CLI_NO_ERROR;
}

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
UI32_T CLI_API_MacAddressTable_HashLookupDepth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T lookup_depth = 0;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACADDRESSTABLE_HASHLOOKUPDEPTH:
        lookup_depth = atoi((char*)arg[0]);
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACADDRESSTABLE_HASHLOOKUPDEPTH:
        lookup_depth = SYS_DFLT_L2_MAC_TABLE_HASH_LOOKUP_DEPTH;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    if (lookup_depth != 0)
    {
        if (!AMTR_PMGR_SetHashLookupDepth(lookup_depth))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to set hash lookup depth.\r\n");
#endif
            return CLI_NO_ERROR;
        }
    }
    else
    {
        if (!AMTR_PMGR_SetHashLookupDepth(SYS_DFLT_L2_MAC_TABLE_HASH_LOOKUP_DEPTH))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to unset hash lookup depth.\r\n");
#endif
            return CLI_NO_ERROR;
        }
    }

    CLI_LIB_PrintStr("Success. New hash lookup depth will be activated after reboot.\r\n");

    return CLI_NO_ERROR;
}
#endif

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
UI32_T CLI_API_MacAddressTable_MacLearning(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T vid = 0;
    BOOL_T learning = TRUE;

    vid = atoi((char*)arg[1]);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACADDRESSTABLE_MACLEARNING:
            learning = TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACADDRESSTABLE_MACLEARNING:
            learning = FALSE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (!AMTR_PMGR_SetVlanLearningStatus(vid, learning))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set vlan learning on vlan %ld.\r\n", (unsigned long)vid);
#endif
        return CLI_NO_ERROR;
    }

    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_MacAddressTable_Secure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0/*(SYS_CPNT_PORT_SECURITY == TRUE)*/
   UI32_T lport;
   PSEC_MGR_PortSecAddrEntry_T port_sec_addr_entry;

   CLI_LIB_ValsInMac(arg[0], port_sec_addr_entry.port_sec_addr_address);

   switch (cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACADDRESSTABLE_SECURE:
      switch(arg[1][0])
      {
      case 'e':
      case 'E':
         {
            UI32_T verify_unit = atoi(arg[2]);
            UI32_T verify_port = atoi(strchr((char*)arg[2],'/')+1);
            CLI_API_EthStatus_T verify_ret;
#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[2][0]))
            {
               verify_unit = atoi(arg[2]);
               verify_port = atoi(strchr((char*)arg[2],'/')+1);
            }
            else/*port name*/
            {
               UI32_T trunk_id = 0;
               if (!IF_PMGR_IfnameToIfindex(arg[2], &lport))
               {
#if (SYS_CPNT_EH == TRUE)
                   CLI_API_Show_Exception_Handeler_Msg();
#else
                   CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[2]);
#endif
                   return CLI_NO_ERROR;
               }
               SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
            }
#endif
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               return CLI_NO_ERROR;
            }
         }
         break;

      case 'p':
      case 'P':
         {
            UI32_T verify_trunk_id = atoi(arg[2]);
            CLI_API_TrunkStatus_T verify_ret;

            if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_ETH_OK)
            {
               display_trunk_msg(verify_ret, verify_trunk_id);
               return CLI_NO_ERROR;
            }
         }
         break;

      default:
         return CLI_NO_ERROR;
      }

      if(!VLAN_POM_IsVlanExisted(atoi(arg[4])))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_1("No VLAN %u\r\n", atoi(arg[4]));
#endif
         return CLI_NO_ERROR;
      }
      port_sec_addr_entry.port_sec_addr_fdb_id = atoi(arg[4]);
      port_sec_addr_entry.port_sec_addr_port = lport;

      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACADDRESSTABLE_SECURE:
      if(!VLAN_POM_IsVlanExisted(atoi(arg[4])))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_1("No VLAN %u\r\n", atoi(arg[4]));
#endif
         return CLI_NO_ERROR;
      }
      port_sec_addr_entry.port_sec_addr_fdb_id = atoi(arg[2]);
      port_sec_addr_entry.port_sec_addr_port = 0;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
   if (!PSEC_PMGR_SetPortSecAddrEntry(&port_sec_addr_entry))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to set the MAC address table on VLAN %lu\r\n", (unsigned long)port_sec_addr_entry.port_sec_addr_fdb_id);
#endif
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_MacAddressTable_Static(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_UI == TRUE)
   AMTR_TYPE_AddrEntry_T addr_entry;
    char ch;
   memset(addr_entry.mac,0,sizeof(addr_entry.mac));
   addr_entry.vid=1;
    
#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_OM_VNI_T vni_entry;
    UI16_T l_vxlan_port;
    UI32_T r_vxlan_port;
    UI16_T l_vfi;
    
    memset(addr_entry.r_vtep_ip, 0, SYS_ADPT_IPV4_ADDR_LEN);
#endif
   
   /*MAC address*/
   CLI_LIB_ValsInMac(arg[0], addr_entry.mac);

   switch (cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACADDRESSTABLE_STATIC:
#if (SYS_CPNT_VXLAN == TRUE)
        memset(&vni_entry, 0, sizeof(VXLAN_OM_VNI_T));
        if (arg[1][0] == 'v' || arg[1][0] == 'V')
        {
            vni_entry.vni = atoi((char*)arg[2]);

            if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetVniEntry(&vni_entry))
            {
                CLI_LIB_PrintStr_1("No VNI %d entry.\n", vni_entry.vni);
                return CLI_NO_ERROR;
            }
            addr_entry.vid = vni_entry.vfi;
        }
      
        if (arg[3][0] == 'r' || arg[3][0] == 'R')
        {
            AMTRL3_OM_VxlanTunnelEntry_T vxlan_tunnel;
            VXLAN_OM_RVtep_T rvtep_entry;
            L_INET_AddrIp_T ip_p;

            if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                               arg[4],
                                                               (L_INET_Addr_T *) &ip_p,
                                                               sizeof(ip_p)))
            {
                CLI_LIB_PrintStr("Invalid address format.\r\n");
                return CLI_NO_ERROR;
            }
            
            memcpy(addr_entry.r_vtep_ip, ip_p.addr, SYS_ADPT_IPV4_ADDR_LEN);
            memset(&rvtep_entry, 0, sizeof(VXLAN_OM_RVtep_T));
            memset(&vxlan_tunnel, 0, sizeof(AMTRL3_OM_VxlanTunnelEntry_T));

            if (VXLAN_POM_GetNextFloodRVtep(&rvtep_entry) == VXLAN_TYPE_RETURN_OK)
            {
                vxlan_tunnel.local_vtep = rvtep_entry.s_ip;
                vxlan_tunnel.vfi_id = vni_entry.vfi;
                vxlan_tunnel.remote_vtep = ip_p;
                
                if ((AMTRL3_POM_GetVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel) == FALSE) ||
                    (vxlan_tunnel.uc_vxlan_port == 0))
                {
                    /* If vxlan tunnel entry is not established, network vxlan port cannot be obtained,
                       so just fill the field with any valid value. Until host route with the r-vtep is ready,
                       the network vxlan port will be fixed with correct value.
                    */
                    addr_entry.ifindex = SYS_ADPT_VXLAN_MAX_LOGICAL_PORT_ID - 1;      
                }
                else
                {    
                    VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port); 
                    addr_entry.ifindex = l_vxlan_port;
                }             
            }
            else
            {
                CLI_LIB_PrintStr("Failed to set static MAC of network port.\r\n");
                return CLI_NO_ERROR;    
            }    
        }
        else
#endif   
      {
            if ((arg[2][0] == 'e' || arg[2][0] == 'E')
#if (SYS_CPNT_VXLAN == TRUE)
                || (arg[4][0] == 'e' || arg[4][0] == 'E')
#endif
               )
      {
                UI32_T verify_unit;
                UI32_T verify_port;
         UI32_T verify_ifindex = 0;
         CLI_API_EthStatus_T verify_ret;

#if (SYS_CPNT_VXLAN == TRUE)         
                if (vni_entry.vni != 0)
                {
                    verify_unit = atoi((char*)arg[5]);
                    verify_port = atoi(strchr((char*)arg[5],'/')+1);
                }
                else
#endif   
                {
                    verify_unit = atoi((char*)arg[3]);
                    verify_port = atoi(strchr((char*)arg[3],'/')+1);
                }
         
/*2004/5/18 05:17 pttch add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
         if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
         {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
            //UI32_T ifindex;  //not used ,to remove warning
            UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

            UI32_T  master_id = 0;

            STKTPLG_POM_GetMasterUnitId((UI8_T *) & master_id );

            /*if the port is module port, save command in buffer*/
            if ( TRUE == STKTPLG_POM_IsModulePort( verify_unit, verify_port ) )
            {
                sprintf((char*)cmd_buff,"mac-address-table static %s interface ethernet %s vlan %s %s\n!\n", arg[0], arg[3], arg[5], arg[6]);
                CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                return CLI_NO_ERROR;
            }
#else
            UI32_T ifindex;
            BOOL_T is_inherit        =TRUE;
            UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

            SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &ifindex, &is_inherit);

            /*if the port is module port, save command in buffer*/
            if (TRUE == CLI_MGR_IsModulePort(ifindex))
            {
                sprintf((char*)cmd_buff,"mac-address-table static %s interface ethernet %s vlan %s %s\n!\n", arg[0], arg[3], arg[5], arg[6]);
                CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                return CLI_NO_ERROR;
            }
#endif
         }
#endif

#if (CLI_SUPPORT_PORT_NAME == 1)
         if (isdigit(arg[3][0]))
         {
            verify_unit = atoi(arg[3]);
            verify_port = atoi(strchr((char*)arg[3],'/')+1);
         }
         else/*port name*/
         {
            UI32_T trunk_id = 0;
            if (!IF_PMGR_IfnameToIfindex(arg[3], &verify_ifindex))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[3]);
#endif
                return CLI_NO_ERROR;
            }
            SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &verify_unit, &verify_port, &trunk_id);
         }
#endif
         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &verify_ifindex )) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            return CLI_NO_ERROR;
         }
         addr_entry.ifindex = (UI16_T)verify_ifindex;
      }
            else if (arg[2][0] == 'p' || arg[2][0] == 'P')
      {
         UI32_T trunk_id = atoi((char*)arg[3]);
         UI32_T verify_ifindex = 0;
         CLI_API_TrunkStatus_T verify_ret;

         if( (verify_ret = verify_trunk(trunk_id, &verify_ifindex)) != CLI_API_TRUNK_OK)
         {
            display_trunk_msg(verify_ret, trunk_id);
            return CLI_NO_ERROR;
         }
         addr_entry.ifindex = (UI16_T)verify_ifindex;
      }
            else
            {  
                return CLI_ERR_INTERNAL;
            }
            /*VLAN*/
#if (SYS_CPNT_VXLAN == TRUE)
            if (vni_entry.vni != 0)
            {
                UI16_T vid;
                UI32_T vni;
                
                vid = atoi((char*)arg[7]);
                if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetPortVlanVniMapEntry(addr_entry.ifindex, vid, &vni))
                {
                    CLI_LIB_PrintStr_2("Port %u & VLAN %u are not associated with VNI.\n", addr_entry.ifindex, vid);
                    return CLI_NO_ERROR;
                }
                
                if (vni_entry.vni != vni)
                {
                    CLI_LIB_PrintStr("Please check VNI.\n");
                    return CLI_NO_ERROR;
                }

                r_vxlan_port = VXLAN_POM_GetAccessVxlanPort(vid, addr_entry.ifindex);  
                if (r_vxlan_port == 0)
                {
                    CLI_LIB_PrintStr("Failed to set access port static MAC.\n");
                    return CLI_NO_ERROR;
                }

                VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(r_vxlan_port, l_vxlan_port);    
                addr_entry.ifindex = l_vxlan_port;    
      }
            else  
#endif 
            {
      addr_entry.vid = atoi((char*)arg[5]);
      if(!VLAN_POM_IsVlanExisted(addr_entry.vid))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr_1("No VLAN %u\r\n", addr_entry.vid);
#endif
         return CLI_NO_ERROR;
      }
            }   
        }

      /*Life_Time*/
        ch = 'p';
#if (SYS_CPNT_VXLAN == TRUE)         
        if (vni_entry.vni != 0)
        {
            if (arg[3][0] == 'r' || arg[3][0] == 'R')
            {
                if (arg[5] != NULL)
                    ch = arg[5][0];    
            }
            else
            {
                if (arg[8] != NULL)
                    ch = arg[8][0];
            }    
        }
        else
#endif            
        {
            if (arg[6] != NULL)
                ch = arg[6][0];
        }    
        
        if (ch == 'p' || ch == 'P')
      {
         addr_entry.life_time= AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;         /*permanent*/
      }
      else
      {
         addr_entry.life_time= AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;   /*delete-on-reset*/
      }
      addr_entry.source= AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
      addr_entry.action= AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
      if (!AMTR_PMGR_SetAddrEntry(&addr_entry))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set static MAC address table.\r\n");
#endif
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACADDRESSTABLE_STATIC:
#if (SYS_CPNT_VXLAN == TRUE)      
      if (arg[1][1] == 'n' || arg[1][1] == 'N')
      {
            vni_entry.vni = atoi((char*)arg[2]);
          if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetVniEntry(&vni_entry))
          {
              CLI_LIB_PrintStr_1("No VNI %d entry.\n", vni_entry.vni);
              return CLI_NO_ERROR;
          }
          addr_entry.vid = vni_entry.vfi;
      }
      else    
#endif        
      {  
            addr_entry.vid = atoi((char*)arg[2]);
            if(!VLAN_POM_IsVlanExisted(addr_entry.vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("No VLAN %u\r\n", addr_entry.vid);
#endif
                return CLI_NO_ERROR;
            }
        }

      if (!AMTR_PMGR_DeleteAddr(addr_entry.vid, addr_entry.mac))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to remove entry from static address table.\r\n");
#endif
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif /* #if (SYS_CPNT_AMTR_UI == TRUE) */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Clear_MacAddressTable_Dynamic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    #define NOT_EXIST_POSITION 0xffffffff
    #define CLEAR_METHOD_ALL 0
    #define CLEAR_METHOD_ADDRESS 1
    #define CLEAR_METHOD_ETHERNET 2
    #define CLEAR_METHOD_PORT_CHANNEL 3
    #define CLEAR_METHOD_VLAN 4

    UI32_T to_detect_position   = 0;
    UI32_T interface_position   = NOT_EXIST_POSITION;
    UI32_T vid_position         = NOT_EXIST_POSITION;
    UI32_T lport       =  0;
    UI32_T vid         =  0;
    UI32_T trunk_id=0;
    UI32_T verify_port=0, verify_unit=0, ifindex=0;
    UI8_T  mac_addr[6] = {0};
    UI8_T  clear_method = CLEAR_METHOD_ALL;
    AMTR_TYPE_AddrEntry_T addr_entry;
    CLI_API_EthStatus_T verify_ret;

    if(arg[to_detect_position] == NULL) /* no arguments : clear all */
    {
        clear_method = CLEAR_METHOD_ALL;
    }
    else
    {
        switch (arg[to_detect_position][0])
        {
            case 'a': /* clear by specific address or all */
            case 'A':
                clear_method = CLEAR_METHOD_ADDRESS;
                to_detect_position += 1;

                if(arg[to_detect_position]) /* clear by specific MAC address */
                {
                    if(FALSE == CLI_LIB_ValsInMac(arg[to_detect_position], mac_addr)) /*check MAC format*/
                    {
                    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
                    #else
                        CLI_LIB_PrintStr_1("%s is invalid\r\n",arg[to_detect_position]);
                    #endif
                        return CLI_NO_ERROR;
                    }
                }
                else /* clear all */
                {
                    clear_method = CLEAR_METHOD_ALL;
                }
            break;

            case 'i': /* clear by interface (ethernet or port channel) */
            case 'I':
                if(((arg[to_detect_position+1][0] == 'e' || arg[to_detect_position+1][0] == 'E') )||
                    ( arg[to_detect_position+1][0] == 'p' || arg[to_detect_position+1][0] == 'P') )
                {
                    to_detect_position++;
                    interface_position  = to_detect_position;
                    to_detect_position += 2;

                    if(arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E') /*ethernet*/
                    {
                        clear_method = CLEAR_METHOD_ETHERNET;

                        verify_unit = atoi(arg[interface_position+1]);
                        verify_port = atoi(strchr(arg[interface_position+1],'/')+1);

                    #if (CLI_SUPPORT_PORT_NAME == 1)
                        if (isdigit(arg[interface_position+1][0]))
                        {
                            verify_unit = atoi(arg[interface_position+1]);
                            verify_port = atoi(strchr(arg[interface_position+1],'/')+1);
                        }
                        else/*port name*/
                        {
                            if (!IF_PMGR_IfnameToIfindex(arg[interface_position+1], &lport))
                            {
                            #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
                            #else
                                CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[interface_position+1]);
                            #endif
                                return CLI_NO_ERROR;
                            }
                        SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                        }
                            #endif

                    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        return CLI_NO_ERROR;
                    }
                }
                else    /*port-channel*/
                {
                    clear_method = CLEAR_METHOD_PORT_CHANNEL;
                    trunk_id = atoi(arg[interface_position+1]);

                    if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                    {
                        display_trunk_msg(verify_ret, trunk_id);
                        return CLI_NO_ERROR;
                    }
                }
            }
            break;
            case 'v': /* clear by vlan */
            case 'V':
                clear_method = CLEAR_METHOD_VLAN;
                vid_position = to_detect_position;
                vid = atoi(arg[to_detect_position + 1]);
            break;

            default:
                clear_method = CLEAR_METHOD_ALL;
            break;

        }/* end of switch */
    }   /*end of if(arg[to_detect_position] == NULL) */

    switch (clear_method)
    {
        case CLEAR_METHOD_ALL:
            if (FALSE == AMTR_PMGR_DeleteAddrByLifeTime(AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            {
            #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
            #else
                CLI_LIB_PrintStr("Clear all dynamic address error\r\n");
            #endif
            }
        break;

        case CLEAR_METHOD_ADDRESS:
            memset (&addr_entry, 0, sizeof (AMTR_TYPE_AddrEntry_T));
            memcpy(addr_entry.mac, mac_addr, SYS_ADPT_MAC_ADDR_LEN);
            addr_entry.vid=0;
            while  ( AMTR_PMGR_GetNextMVAddrEntry (&addr_entry, AMTR_MGR_GET_DYNAMIC_ADDRESS))
            {
                if (memcmp(addr_entry.mac, mac_addr, SYS_ADPT_MAC_ADDR_LEN) == 0 )
                {
                    if (FALSE == AMTR_PMGR_DeleteAddr(addr_entry.vid, addr_entry.mac))
                    {
                    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
                    #else
                        CLI_LIB_PrintStr("Clear dynamic address error\r\n");
                    #endif
                    }
                }
                else
                    break;
            }
        break;

        case CLEAR_METHOD_ETHERNET:
            SWCTRL_POM_UserPortToIfindex( verify_unit, verify_port, &ifindex);
            if(FALSE== AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            {
            #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
            #else
                CLI_LIB_PrintStr("Clear dynamic address by port error\r\n");
            #endif
            }

        break;

        case CLEAR_METHOD_PORT_CHANNEL:
            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &ifindex);
            if(FALSE== AMTR_PMGR_DeleteAddrByLifeTimeAndLPort(ifindex, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            {
            #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
            #else
                CLI_LIB_PrintStr("Clear dynamic address by trunk id error\r\n");
            #endif
            }
        break;

        case CLEAR_METHOD_VLAN:
            if(FALSE== AMTR_PMGR_DeleteAddrByVidAndLifeTime(vid, AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT))
            {
            #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
            #else
                CLI_LIB_PrintStr("Clear dynamic address by trunk id error\r\n");
            #endif
            }
        break;
    }
   return CLI_NO_ERROR;
}/* end of CLI_API_Clear_MacAddressTable_Dynamic */

static void show_mac_count_by_lport(UI32_T lport)
{
    UI32_T count, static_count, unit, port, trunk_id;
    char   buf[64];

    count = AMTR_OM_GetDynCounterByPort(lport);

    static_count = AMTR_OM_GetStaticCounterByPort(lport);

    CLI_LIB_PrintStr("\r\n");

    if(SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
    {
        SYSFUN_Snprintf(buf, sizeof(buf), "MAC Entries for Trunk ID %ld\r\n", (long)trunk_id);
        buf[sizeof(buf)-1]=0;
        CLI_LIB_PrintStr(buf);
    }
    else
    {
        SYSFUN_Snprintf(buf, sizeof(buf), "MAC Entries for Eth %ld/%ld\r\n", (long)unit, (long)port);
        buf[sizeof(buf)-1]=0;
        CLI_LIB_PrintStr(buf);
    }

    SYSFUN_Snprintf(buf, sizeof(buf), "Total Address Count      :%ld\r\n", (long)count+static_count);
    buf[sizeof(buf)-1]=0;
    CLI_LIB_PrintStr(buf);
    SYSFUN_Snprintf(buf, sizeof(buf), "Static Address Count     :%ld\r\n", (long)static_count);
    buf[sizeof(buf)-1]=0;
    CLI_LIB_PrintStr(buf);
    SYSFUN_Snprintf(buf, sizeof(buf), "Dynamic Address Count    :%ld\r\n", (long)count);
    buf[sizeof(buf)-1]=0;
    CLI_LIB_PrintStr(buf);
    return ;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_MacAddressTable
 *------------------------------------------------------------------------------
 * PURPOSE  : show mac address table according to the way user specified
 *            configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    : pttch 92.01.08
 *            if user specify mac first, default sort method will be MAC.
 *            if user specify vlan first, default sort method will be vlan.
 *            if user specify interface first, default sort method will be interface.
 *            if user specify sort method by interface and specify mac address
 *            may be take long time to search.
 *------------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_MacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#define NOT_EXIST_POSITION 0xffffffff
#define SORT_METHOD_ADDR      0
#define SORT_METHOD_INTERFACE 1
#define SORT_METHOD_VLAN      2

    UI32_T to_detect_position   = 0;
    UI32_T interface_position   = NOT_EXIST_POSITION;
    UI32_T mac_addr_position    = NOT_EXIST_POSITION;
    UI32_T mac_netmask_position = NOT_EXIST_POSITION;
    UI32_T vid_position         = NOT_EXIST_POSITION;
    UI32_T sort_method_position = NOT_EXIST_POSITION;
    UI32_T lport       =  0;
    UI32_T vid         =  0;
    UI8_T  mac_addr[6] = {0};
    UI8_T  true_mac_addr[6] = {0};
    UI8_T  mac_mask[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; /*didn't specify mac addr, mac mask => display all*/
    UI8_T  sort_method = SORT_METHOD_INTERFACE;
    BOOL_T is_specify_mac_addr = FALSE;
    BOOL_T is_trunk_member;

    if(arg[to_detect_position])
    {
        /*specify address*/
        if (arg[to_detect_position][0] == 'a' || arg[to_detect_position][0] == 'A')
        {
            /*pttch 92.01.08 use the first specify with primary key*/
            to_detect_position += 1;
            sort_method = SORT_METHOD_ADDR;
        }
        else if(arg[to_detect_position][0] == 'c' || arg[to_detect_position][0] == 'C')/*mac-learning count*/
        {
            if(arg[to_detect_position+1])
            {
                /* argument behind 'count' exists
                 */
                interface_position = to_detect_position + 2;

                if((arg[interface_position] != NULL) &&
                   (((arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E') && strchr((char*)arg[interface_position], '-') == 0) ||
                    ( arg[interface_position][0] == 'p' || arg[interface_position][0] == 'P')))
                {
                    CLI_API_TrunkStatus_T verify_ret_trunk;
                    CLI_API_EthStatus_T   verify_ret_eth;

                    if((arg[interface_position] != NULL)&&(arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E'))
                    {
                        UI32_T verify_unit ;
                        UI32_T verify_port ;
                        char * port_off_p;

                        if(!(port_off_p = strchr((char*)arg[interface_position+1],'/')))
                            return CLI_NO_ERROR;

                        verify_unit = atoi((char*)arg[interface_position+1]);
                        verify_port = atoi((char*)(port_off_p+1));

                        if((verify_ret_eth = verify_ethernet(verify_unit,verify_port,&lport)) != CLI_API_ETH_OK)
                        {
                            display_ethernet_msg(verify_ret_eth,verify_unit,verify_port);
                            return CLI_NO_ERROR;
                        }

                        is_trunk_member = FALSE;
                    }
                    else
                    {
                        UI32_T trunk_id = atoi((char*)arg[interface_position+1]);

                        if( (verify_ret_trunk = verify_trunk(trunk_id,&lport)) != CLI_API_TRUNK_OK)
                        {
                            display_trunk_msg(verify_ret_trunk, trunk_id);
                            return CLI_NO_ERROR;
                        }

                        is_trunk_member = TRUE;
                    }

                    show_mac_count_by_lport(lport);

                    return CLI_NO_ERROR;
                }
            }
            else
            {
                /* no argument behind 'count' */
                AMTR_OM_CountersPerSystem_T  counter;
                UI32_T  line_num = 0;
                char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

                AMTR_OM_GetCountersPerSystem(&counter);

                PROCESS_MORE("\r\n");
                PROCESS_MORE("Compute the number of MAC Address...\r\n");
                PROCESS_MORE("\r\n");
                PROCESS_MORE("Maximum number of MAC Address which can be created in the system:\r\n");
                PROCESS_MORE_1("Total Number of MAC Address      : %u\r\n", SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY);
                PROCESS_MORE_1("Number of Static MAC Address     : %u\r\n", SYS_ADPT_MAX_NBR_TOTAL_STATIC_MAC);
                PROCESS_MORE("\r\n");
                PROCESS_MORE("Current number of entries which have been created in the system:\r\n");
                PROCESS_MORE_1("Total Number of MAC Address      : %lu\r\n", (unsigned long)counter.total_count);
                PROCESS_MORE_1("Number of Static MAC Address     : %lu\r\n", (unsigned long)counter.static_count);
                PROCESS_MORE_1("Number of Dynamic MAC Address    : %lu\r\n", (unsigned long)counter.dynamic_count);

                return CLI_NO_ERROR;
            }
        }
    }

    /*MAC address and mask*/
    if(arg[to_detect_position])
    {
        if(CLI_LIB_ValsInMac(arg[to_detect_position], mac_addr)) /*check MAC format*/
        {
            mac_addr_position      =  to_detect_position;
            to_detect_position    += 1;
            is_specify_mac_addr = TRUE;
            if( (arg[to_detect_position] != NULL) && CLI_LIB_ValsInMac(arg[to_detect_position], mac_mask)) /*check MAC format*/
            {
                mac_netmask_position      = to_detect_position;
                to_detect_position       += 1;
            }
            else
                memset(mac_mask, 0, sizeof(mac_mask));/*specify MAC address, check all => don't by pass any one*/
        }
        else
            memset(mac_mask, 0xff, sizeof(mac_mask)); /*no specified MAC address, ignore all => by pass all*/
    }
    else
        goto PARSING_END;

    /*interface*/
    if(arg[to_detect_position])
    {
        if((arg[to_detect_position+1] != NULL) &&
                (((arg[to_detect_position+1][0] == 'e' || arg[to_detect_position+1][0] == 'E') && strchr((char*)arg[to_detect_position+1], '-') == 0 ) ||
                 ( arg[to_detect_position+1][0] == 'p' || arg[to_detect_position+1][0] == 'P') ))
        {
            to_detect_position++;
            interface_position  = to_detect_position;
            to_detect_position += 2;

            if((arg[interface_position] != NULL)&&(arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E')) /*ethernet*/
            {
                UI32_T verify_unit = atoi((char*)arg[interface_position+1]);
                UI32_T verify_port = atoi(strchr((char*)arg[interface_position+1],'/')+1);
                CLI_API_EthStatus_T verify_ret;
#if (CLI_SUPPORT_PORT_NAME == 1)
                if ((arg[interface_position+1] != NULL) && (isdigit(arg[interface_position+1][0])))
                {
                    verify_unit = atoi((char*)arg[interface_position+1]);
                    verify_port = atoi(strchr((char*)arg[interface_position+1],'/')+1);
                }
                else/*port name*/
                {
                    UI32_T trunk_id = 0;

                    if (!IF_PMGR_IfnameToIfindex(arg[interface_position+1], &lport))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[interface_position+1]);
#endif
                        return CLI_NO_ERROR;
                    }

                    SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                }
#endif
                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    return CLI_NO_ERROR;
                }
            }
            else                                                                       /*port-channel*/
            {
                UI32_T trunk_id = atoi((char*)arg[interface_position+1]);
                CLI_API_TrunkStatus_T verify_ret;

                if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret, trunk_id);
                    return CLI_NO_ERROR;
                }
            }
        }
    }
    else
        goto PARSING_END;

    /*VLAN id*/
    if(arg[to_detect_position])
    {
        /*pttch 92.01.08 use the first specify with primary key*/
        if (to_detect_position == 0)
            sort_method = SORT_METHOD_VLAN;

        if(arg[to_detect_position][0] == 'v' || arg[to_detect_position][0] == 'V')
        {
            vid_position = to_detect_position;
            vid = atoi((char*)arg[to_detect_position + 1]);
            to_detect_position += 2;
        }
    }
    else
        goto PARSING_END;

    /*sort method*/
    if(arg[to_detect_position]) /*if the argument exists, this must be "sort xxxx"*/
    {
        sort_method_position = to_detect_position + 1;

        switch(arg[sort_method_position][0])
        {
            case 'a':
            case 'A':
                sort_method = SORT_METHOD_ADDR;
                break;

            case 'i':
            case 'I':
                sort_method = SORT_METHOD_INTERFACE;
                break;

            case 'v':
            case 'V':
                sort_method = SORT_METHOD_VLAN;
                break;
        }
    }
    else
        goto PARSING_END;

PARSING_END:
    {
        AMTR_TYPE_AddrEntry_T addr_entry;
        UI32_T               unit;
        UI32_T               port;
        UI32_T               trunk_id;

        char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
        UI8_T  tmp_buff[CLI_DEF_MAX_BUFSIZE/2] = {0};
        UI32_T line_num = 0;
        UI32_T check_vid = 0;
        UI32_T check_lport = 0;
        SWCTRL_Lport_Type_T lport_type;
#if(SYS_CPNT_VXLAN == TRUE)
        AMTRL3_TYPE_VxlanTunnelEntry_T tunnel_entry;
        UI32_T r_vxlan_port = 0;
        SWCTRL_Lport_Type_T access_type;
        UI32_T access_lport = 0;
        UI32_T access_unit = 0;
        UI32_T access_port = 0;
        UI32_T access_trunk = 0;
        UI32_T vxlan_vni =0;
        UI16_T vxlan_vid = 0;
        I32_T  vni;
        char ip_ar[20] = {0};
#endif
        char vid_ar[20] = {0};

        memset(&addr_entry, 0, sizeof(AMTR_TYPE_AddrEntry_T));
#if 0
#if (SYS_CPNT_SWCTRL_SHOW_ADDRESS_TABLE_TOTAL_ENTRY == TRUE)
        {
            UI32_T total_entry;

            total_entry = AMTR_OM_GetTotalCounter();
            SYSFUN_Sprintf(buff, "\n Total entry in system: %lu\r\n", total_entry);
            PROCESS_MORE(buff);
        }
#endif /* #if (SYS_CPNT_SWCTRL_SHOW_ADDRESS_TABLE_TOTAL_ENTRY == TRUE) */
#endif

        if (is_specify_mac_addr == TRUE)/*pttch 92.01.08 specify mac address to check*/
        {
            memcpy(true_mac_addr, mac_addr, sizeof(mac_addr));
            memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
        }
        switch(sort_method)
        {
            case SORT_METHOD_ADDR:
#if(SYS_CPNT_VXLAN == TRUE)
                PROCESS_MORE(" Flag: * - VxLAN VNID\r\n");
                PROCESS_MORE(" MAC Address       VLAN/VxLAN  Interface           Type     Life Time\r\n");
                PROCESS_MORE(" ----------------- ----------- ------------------- -------- -----------------\r\n");
#else
                PROCESS_MORE(" MAC Address       VLAN Interface           Type     Life Time\r\n");
                PROCESS_MORE(" ----------------- ---- ------------------- -------- -----------------\r\n");
#endif

                while(AMTR_PMGR_GetNextMVAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
                {
                    if(interface_position != NOT_EXIST_POSITION && addr_entry.ifindex != lport)
                        continue; /*charles, Mar 18, 2002*/

                    if(vid_position != NOT_EXIST_POSITION && addr_entry.vid != vid)
                        continue;

                    if(MacFilter(addr_entry.mac, true_mac_addr, mac_mask))
                    {

                        /*MAC*/
                        SYSFUN_Sprintf((char*)buff, " %02X-%02X-%02X-%02X-%02X-%02X",
                                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);

                        /*vlan*/
#if(SYS_CPNT_VXLAN == TRUE)
                        if(addr_entry.vid <= SYS_ADPT_MAX_VLAN_ID)
                        {
                          SYSFUN_Sprintf((char*)tmp_buff, "  %10u", addr_entry.vid);
                          strcat((char*)buff, (char*)tmp_buff);
                        }
                        else
                        {
                            vni = VXLAN_POM_GetVniByVfi(addr_entry.vid);
                            
                            if (vni > 0)
                            {
                                SYSFUN_Sprintf((char*)tmp_buff, " *%10u", vni);
                            }
                            else
                            {
                                SYSFUN_Sprintf((char*)tmp_buff, " *%10s", "");
                            }
                            strcat((char*)buff, (char*)tmp_buff);
                        }
#else
                        SYSFUN_Sprintf((char*)tmp_buff, " %4u", addr_entry.vid);
                        strcat((char*)buff, (char*)tmp_buff);
#endif
                        /*interface*/
                        if(addr_entry.source==AMTR_TYPE_ADDRESS_SOURCE_SELF)
                        {
                            SYSFUN_Sprintf((char*)tmp_buff, " CPU                ");
                        }
                        else if(addr_entry.ifindex==0)
                        {
                            SYSFUN_Sprintf((char*)tmp_buff, " NA                 ");
                        }
                        else
                        {
                            lport_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &unit, &port, &trunk_id);
                            if(lport_type==SWCTRL_LPORT_TRUNK_PORT)
                                SYSFUN_Sprintf((char*)tmp_buff, " Trunk %2lu           ", (unsigned long)trunk_id);
#if(SYS_CPNT_VXLAN == TRUE)
                            else if (lport_type == SWCTRL_LPORT_VXLAN_PORT)
                            {
                                VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(addr_entry.ifindex, r_vxlan_port);
                                if (r_vxlan_port != 0)
                                {
                                    memset(&tunnel_entry, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
                                    tunnel_entry.vxlan_port = r_vxlan_port;
                                    
                                    if (addr_entry.r_vtep_ip[0]) 
                                    {   /* entry for static MAC of network port */
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            addr_entry.r_vtep_ip[0], addr_entry.r_vtep_ip[1],
                                            addr_entry.r_vtep_ip[2], addr_entry.r_vtep_ip[3]);

                                        SYSFUN_Sprintf((char*)tmp_buff, " %-19s", ip_ar);
                                    }
                                    else if (AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(SYS_ADPT_DEFAULT_FIB, &tunnel_entry) == TRUE)
                                    {
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            tunnel_entry.remote_vtep.addr[0], tunnel_entry.remote_vtep.addr[1],
                                            tunnel_entry.remote_vtep.addr[2], tunnel_entry.remote_vtep.addr[3]);

                                        SYSFUN_Sprintf((char*)tmp_buff, " %-19s", ip_ar);
                                    }
                                    else if (VXLAN_POM_GetVlanNlportOfAccessPort(r_vxlan_port, &vxlan_vid, &access_lport))
                                    {
                                        access_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &access_unit, &access_port, &access_trunk);
                                        if(access_type==SWCTRL_LPORT_TRUNK_PORT)
                                        {
                                            SYSFUN_Sprintf((char*)tmp_buff, " Trunk %2lu           ", (unsigned long)access_trunk);
                                        }
                                        else
                                        {
                                            SYSFUN_Sprintf((char*)tmp_buff, " Eth %lu/%2lu           ", (unsigned long)access_unit, (unsigned long)access_lport);
                                        }
                                    }
                                    else
                                    {

                                        SYSFUN_Sprintf((char*)tmp_buff, " %19s", "");
                                    }
                                }
                                else
                                {
                                    SYSFUN_Sprintf((char*)tmp_buff, " %19s", "");
                                }
                            }
#endif
                            else
#if (CLI_SUPPORT_PORT_NAME == 1)
                            {
                                UI8_T name[MAXSIZE_ifName+1] = {0};
                                CLI_LIB_Ifindex_To_Name(addr_entry.ifindex,name);
                                if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                                {                                        /*pttch 2002.07.10*/
                                    name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                                }
                                SYSFUN_Sprintf((char*)tmp_buff, " %19s", name);
                            }
#else
                                SYSFUN_Sprintf((char*)tmp_buff, " Eth %lu/%2lu           ", (unsigned long)unit, (unsigned long)port);
#endif
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        /*type*/
                        switch(addr_entry.source)
                        {
                            case AMTR_TYPE_ADDRESS_SOURCE_CONFIG:
                                SYSFUN_Sprintf((char*)tmp_buff, " Config  ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_INTERNAL:
                                SYSFUN_Sprintf((char*)tmp_buff, " Internal");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SELF:
                                SYSFUN_Sprintf((char*)tmp_buff, " CPU     ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SECURITY:
                                SYSFUN_Sprintf((char*)tmp_buff, " Security");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_MLAG:
                                SYSFUN_Sprintf((char*)tmp_buff, " MLAG    ");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Learn   ");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        switch(addr_entry.life_time)
                        {
                            case AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Permanent\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Reset\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Timeout\r\n");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Other\r\n");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        /*print out*/
                        PROCESS_MORE(buff);
                    }
                }
                break;

            case SORT_METHOD_INTERFACE:
#if(SYS_CPNT_VXLAN == TRUE)
                PROCESS_MORE(" Flag: * - VxLAN VNID\r\n");
                PROCESS_MORE(" Interface           MAC Address       VLAN/VxLAN  Type     Life Time\r\n");
                PROCESS_MORE(" ------------------- ----------------- ----------- -------- -----------------\r\n");
#else
                PROCESS_MORE(" Interface           MAC Address       VLAN Type     Life Time\r\n");
                PROCESS_MORE(" ------------------- ----------------- ---- -------- -----------------\r\n");
#endif

                if (is_specify_mac_addr == TRUE)/*pttch 92.01.08 specify mac address to check*/
                {
                    if (IS_MAC_ZERO(mac_mask) == TRUE)
                    {
                        DEC_MAC(mac_addr);
                        memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
                    }
                    addr_entry.ifindex = 1;
                    addr_entry.vid = 1;
                    check_lport = 1;
                }
                else
                {
                    addr_entry.ifindex = lport;
                }
                while(AMTR_PMGR_GetNextIfIndexAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
                {
                    /*
                       because primary key is lport so if user specify macaddress, need
                       add the search key if mac address search finish.
                     */
                    if (is_specify_mac_addr == TRUE)/*pttch 92.01.08 specify mac address to check*/
                    {
                        /*because mac dec 1, get next may get vlan x but mac is mac dec 1*/
                        if (memcmp(addr_entry.mac, mac_addr, sizeof(mac_addr)) == 0)
                            continue;

                        if (addr_entry.ifindex != check_lport)
                        {
                            memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
                            check_lport = addr_entry.ifindex;
                            addr_entry.vid = 1;
                            check_vid = 1;
                            continue;
                        }
                        if (addr_entry.vid != check_vid)
                        {
                            memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
                            check_vid = addr_entry.vid;
                            continue;
                        }
                    }

                    if(interface_position != NOT_EXIST_POSITION && addr_entry.ifindex > lport) /*user specify and the next one is not the specified interface*/
                        break;

                    if(vid_position != NOT_EXIST_POSITION && addr_entry.vid != vid)
                        continue;

                    if(MacFilter(addr_entry.mac, true_mac_addr, mac_mask))
                    {
                        /* First parsing entry's vlan to get vid
                         */
                        memset(vid_ar, 0, sizeof(vid_ar));
#if(SYS_CPNT_VXLAN == TRUE)
                        if(addr_entry.vid <= SYS_ADPT_MAX_VLAN_ID)
                        {
                          SYSFUN_Sprintf((char*)vid_ar, "  %10u", addr_entry.vid);
                        }
                        else
                        {
                            vni = VXLAN_POM_GetVniByVfi(addr_entry.vid);
                            
                            if (vni > 0)
                            {
                                SYSFUN_Sprintf((char*)vid_ar, " *%10u", vni);
                            }
                            else
                            {
                                SYSFUN_Sprintf((char*)vid_ar, " *%10s", "");
                            }
                        }
#else
                        SYSFUN_Sprintf((char*)vid_ar, " %4u", addr_entry.vid);
#endif
                        /*interface*/
                        if(addr_entry.source==AMTR_TYPE_ADDRESS_SOURCE_SELF)
                        {
                            SYSFUN_Sprintf((char*)buff, " CPU                ");
                        }
                        else if( addr_entry.ifindex==0)
                        {
                            SYSFUN_Sprintf((char*)buff, " NA                 ");
                        }
                        else
                        {
                            lport_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &unit, &port, &trunk_id);
                            if(lport_type==SWCTRL_LPORT_TRUNK_PORT)
                                SYSFUN_Sprintf((char*)buff, " Trunk %2lu           ", (unsigned long)trunk_id);
#if(SYS_CPNT_VXLAN == TRUE)
                            else if (lport_type == SWCTRL_LPORT_VXLAN_PORT)
                            {
                                VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(addr_entry.ifindex, r_vxlan_port);
                                if (r_vxlan_port != 0)
                                {
                                    memset(&tunnel_entry, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
                                    tunnel_entry.vxlan_port = r_vxlan_port;

                                    if (addr_entry.r_vtep_ip[0])
                                    {   /* entry for static MAC of network port */
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            addr_entry.r_vtep_ip[0], addr_entry.r_vtep_ip[1],
                                            addr_entry.r_vtep_ip[2], addr_entry.r_vtep_ip[3]);

                                        SYSFUN_Sprintf((char*)buff, " %-19s", ip_ar);
                                    }
                                    else if (AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(SYS_ADPT_DEFAULT_FIB, &tunnel_entry) == TRUE)
                                    {
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            tunnel_entry.remote_vtep.addr[0], tunnel_entry.remote_vtep.addr[1],
                                            tunnel_entry.remote_vtep.addr[2], tunnel_entry.remote_vtep.addr[3]);

                                        SYSFUN_Sprintf((char*)buff, " %-19s", ip_ar);
                                    }
                                    else if (VXLAN_POM_GetVlanNlportOfAccessPort(r_vxlan_port, &vxlan_vid, &access_lport))
                                    {
                                        access_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &access_unit, &access_port, &access_trunk);
                                        if(access_type==SWCTRL_LPORT_TRUNK_PORT)
                                        {
                                            SYSFUN_Sprintf((char*)buff, " Trunk %2lu           ", (unsigned long)access_trunk);
                                        }
                                        else
                                        {
                                            SYSFUN_Sprintf((char*)buff, " Eth %lu/%2lu           ", (unsigned long)access_unit, (unsigned long)access_lport);
                                        }
                                    }
                                    else
                                    {
                                        SYSFUN_Sprintf((char*)buff, " %19s", "");
                                    }
                                }
                                else
                                {
                                    SYSFUN_Sprintf((char*)buff, " %19s", "");
                                }
                            }
#endif
                            else
#if (CLI_SUPPORT_PORT_NAME == 1)
                            {
                                UI8_T name[MAXSIZE_ifName+1] = {0};
                                CLI_LIB_Ifindex_To_Name(addr_entry.ifindex,name);
                                if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                                {                                        /*pttch 2002.07.10*/
                                    name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                                }
                                SYSFUN_Sprintf((char*)buff, " %19s", name);
                            }
#else
                                SYSFUN_Sprintf((char*)buff, " Eth %lu/%2lu           ", (unsigned long)unit, (unsigned long)port);
#endif
                        }

                        /*MAC*/
                        SYSFUN_Sprintf((char*)tmp_buff, " %02X-%02X-%02X-%02X-%02X-%02X", addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
                        strcat((char*)buff, (char*)tmp_buff);

                        /*vlan*/
                        strcat((char*)buff, (char*)vid_ar);

                        /*type*/
                        switch(addr_entry.source)
                        {
                            case AMTR_TYPE_ADDRESS_SOURCE_CONFIG:
                                SYSFUN_Sprintf((char*)tmp_buff, " Config  ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_INTERNAL:
                                SYSFUN_Sprintf((char*)tmp_buff, " Internal");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SELF:
                                SYSFUN_Sprintf((char*)tmp_buff, " CPU     ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SECURITY:
                                SYSFUN_Sprintf((char*)tmp_buff, " Security");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_MLAG:
                                SYSFUN_Sprintf((char*)tmp_buff, " MLAG    ");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Learn   ");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        switch(addr_entry.life_time)
                        {
                            case AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Permanent\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Reset\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Timeout\r\n");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Other\r\n");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        /*print out*/
                        PROCESS_MORE(buff);
                    }
                }
                break;

            case SORT_METHOD_VLAN:
#if(SYS_CPNT_VXLAN == TRUE)
                PROCESS_MORE(" Flag: * - VxLAN VNID\r\n");
                PROCESS_MORE(" VLAN/VxLAN  MAC Address       Interface           Type     Life Time\r\n");
                PROCESS_MORE(" ----------- ----------------- ------------------- -------- -----------------\r\n");
#else
                PROCESS_MORE(" VLAN MAC Address       Interface           Type     Life Time\r\n");
                PROCESS_MORE(" ---- ----------------- ------------------- -------- -----------------\r\n");
#endif

                if (is_specify_mac_addr == TRUE)/*pttch 92.01.08 specify mac address to check*/
                {
                    if (IS_MAC_ZERO(mac_mask) == TRUE)
                    {
                        DEC_MAC(mac_addr);
                        memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
                    }
                    addr_entry.vid = 1;
                    check_vid = 1;
                }
                while(AMTR_PMGR_GetNextVMAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
                {
                    /*
                     * because primary key is lport so if user specify macaddress, need
                     * add the search key if mac address search finish.
                     */
                    if (is_specify_mac_addr == TRUE)/*pttch 92.01.08 specify mac address to check*/
                    {
                        /*because mac dec 1, get next may get vlan x but mac is mac dec 1*/
                        if (memcmp(addr_entry.mac, mac_addr, sizeof(mac_addr)) == 0)
                            continue;

                        if (addr_entry.vid != check_vid)
                        {
                            memcpy(addr_entry.mac, mac_addr, sizeof(mac_addr));
                            check_vid = addr_entry.vid;
                            continue;
                        }
                    }
                    if(interface_position != NOT_EXIST_POSITION && addr_entry.ifindex != lport)
                        continue; /*charles, Mar 18, 2002*/

                    if (vid_position != NOT_EXIST_POSITION)
                    {
                        if (vid == 0)/*do not specify vlan id*/
                        {
                            if(addr_entry.vid > vid)
                                break;
                        }
                        else
                        {
                            if(addr_entry.vid < vid)/*if entry vlan id less than specify vlan id need to get next*/
                                continue;
                            else if (addr_entry.vid > vid)
                                break;
                        }
                    }

                    if(MacFilter(addr_entry.mac, true_mac_addr, mac_mask))
                    {
                        /*vlan*/
#if(SYS_CPNT_VXLAN == TRUE)
                        if(addr_entry.vid <= SYS_ADPT_MAX_VLAN_ID)
                        {
                            SYSFUN_Sprintf((char*)buff, "  %10u", addr_entry.vid);
                        }
                        else
                        {
                            vni = VXLAN_POM_GetVniByVfi(addr_entry.vid);
                            
                            if (vni > 0)
                            {
                                SYSFUN_Sprintf((char*)tmp_buff, " *%10u", vni);
                            }
                            else
                            {
                                SYSFUN_Sprintf((char*)tmp_buff, " *%10s", "");
                            }
                            strcat((char*)buff, (char*)tmp_buff);
                        }
#else
                        SYSFUN_Sprintf((char*)buff, " %4u", addr_entry.vid);
#endif

                        /*MAC*/
                        SYSFUN_Sprintf((char*)tmp_buff, " %02X-%02X-%02X-%02X-%02X-%02X", addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
                        strcat((char*)buff, (char*)tmp_buff);

                        /*interface*/
                        if(addr_entry.source==AMTR_TYPE_ADDRESS_SOURCE_SELF)
                        {
                            SYSFUN_Sprintf((char*)tmp_buff, " CPU                ");
                        }
                        else if(addr_entry.ifindex==0)
                        {
                            SYSFUN_Sprintf((char*)tmp_buff, " NA                 ");
                        }
                        else
                        {
                            lport_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &unit, &port, &trunk_id);
                            if(lport_type==SWCTRL_LPORT_TRUNK_PORT)
                                SYSFUN_Sprintf((char*)tmp_buff, " Trunk %2lu           ", (unsigned long)trunk_id);
#if(SYS_CPNT_VXLAN == TRUE)
                            else if (lport_type == SWCTRL_LPORT_VXLAN_PORT)
                            {
                                VXLAN_TYPE_L_PORT_CONVERTTO_R_PORT(addr_entry.ifindex, r_vxlan_port);
                                if (r_vxlan_port != 0)
                                {
                                    memset(&tunnel_entry, 0, sizeof(AMTRL3_TYPE_VxlanTunnelEntry_T));
                                    tunnel_entry.vxlan_port = r_vxlan_port;
                                    
                                    if (addr_entry.r_vtep_ip[0])
                                    {   /* entry for static MAC of network port */
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            addr_entry.r_vtep_ip[0], addr_entry.r_vtep_ip[1],
                                            addr_entry.r_vtep_ip[2], addr_entry.r_vtep_ip[3]);

                                        SYSFUN_Sprintf((char*)tmp_buff, " %-19s", ip_ar);
                                    }
                                    else if (AMTRL3_POM_GetVxlanTunnelEntryByVxlanPort(SYS_ADPT_DEFAULT_FIB, &tunnel_entry) == TRUE)
                                    {
                                        memset(ip_ar, 0, sizeof(ip_ar));
                                        SYSFUN_Snprintf(ip_ar, sizeof(ip_ar), "Vx(%d.%d.%d.%d)",
                                            tunnel_entry.remote_vtep.addr[0], tunnel_entry.remote_vtep.addr[1],
                                            tunnel_entry.remote_vtep.addr[2], tunnel_entry.remote_vtep.addr[3]);

                                        SYSFUN_Sprintf((char*)tmp_buff, " %-19s", ip_ar);
                                    }
                                    else if (VXLAN_POM_GetVlanNlportOfAccessPort(r_vxlan_port, &vxlan_vid, &access_lport))
                                    {
                                        access_type = SWCTRL_POM_LogicalPortToUserPort(addr_entry.ifindex, &access_unit, &access_port, &access_trunk);
                                        if(access_type==SWCTRL_LPORT_TRUNK_PORT)
                                        {
                                            SYSFUN_Sprintf((char*)tmp_buff, " Trunk %2lu           ", (unsigned long)access_trunk);
                                        }
                                        else
                                        {
                                            SYSFUN_Sprintf((char*)tmp_buff, " Eth %lu/%2lu           ", (unsigned long)access_unit, (unsigned long)access_lport);
                                        }
                                    }
                                    else
                                    {
                                        SYSFUN_Sprintf((char*)tmp_buff, " %19s", "");
                                    }
                                }
                                else
                                {
                                    SYSFUN_Sprintf((char*)tmp_buff, " %19s", "");
                                }
                            }
#endif
                            else
#if (CLI_SUPPORT_PORT_NAME == 1)
                            {
                                UI8_T name[MAXSIZE_ifName+1] = {0};
                                CLI_LIB_Ifindex_To_Name(addr_entry.ifindex,name);
                                if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                                {                                        /*pttch 2002.07.10*/
                                    name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                                }
                                SYSFUN_Sprintf((char*)tmp_buff, " %19s", name);
                            }
#else
                                SYSFUN_Sprintf((char*)tmp_buff, " Eth %lu/%2lu           ", (unsigned long)unit, (unsigned long)port);
#endif
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        /*type*/
                        switch(addr_entry.source)
                        {
                            case AMTR_TYPE_ADDRESS_SOURCE_CONFIG:
                                SYSFUN_Sprintf((char*)tmp_buff, " Config  ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_INTERNAL:
                                SYSFUN_Sprintf((char*)tmp_buff, " Internal");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SELF:
                                SYSFUN_Sprintf((char*)tmp_buff, " CPU     ");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_SECURITY:
                                SYSFUN_Sprintf((char*)tmp_buff, " Security");
                                break;

                            case AMTR_TYPE_ADDRESS_SOURCE_MLAG:
                                SYSFUN_Sprintf((char*)tmp_buff, " MLAG    ");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Learn   ");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);


                        switch(addr_entry.life_time)
                        {
                            case AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Permanent\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Reset\r\n");
                                break;

                            case AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT:
                                SYSFUN_Sprintf((char*)tmp_buff, " Delete on Timeout\r\n");
                                break;

                            default:
                                SYSFUN_Sprintf((char*)tmp_buff, " Other\r\n");
                                break;
                        }
                        strcat((char*)buff, (char*)tmp_buff);

                        /*print out*/
                        PROCESS_MORE(buff);
                    }
                }
                break;
        }
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_CollisionMacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
    UI32_T line_num = 0;
    UI8_T  idx, num_of_entry;
    UI16_T vlan_id, collision_count;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
    char  buff[CLI_DEF_MAX_BUFSIZE]={0};

    idx=AMTRDRV_MGR_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX;
    num_of_entry=0;

    CLI_LIB_PrintStr("MAC Address        VLAN  Collision Count\r\n");
    CLI_LIB_PrintStr("----------------- ----- ----------------\r\n");
    while(AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable(&idx, &vlan_id, mac, &collision_count)==TRUE)
    {
        SYSFUN_Sprintf(buff,"%02X-%02X-%02X-%02X-%02X-%02X  %4hu  %15lu\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], vlan_id, (UI32_T)collision_count);
        PROCESS_MORE(buff);
        num_of_entry++;
    }
    PROCESS_MORE("\r\n");
    SYSFUN_Sprintf((char*)buff,"Total collision mac number: %hu\r\n", num_of_entry);
    PROCESS_MORE(buff);
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Clear_CollisionMacAddressTable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
    AMTR_PMGR_ClearCollisionVlanMacTable();
#endif
    return CLI_NO_ERROR;
}

static BOOL_T IS_MAC_ZERO(UI8_T *mac)
{
   UI8_T i = 0;

   for( i = 0; i < 6 ; i ++)
   {
      if (mac[i] != 0)
         return FALSE;
   }
   return TRUE;
}
static void DEC_MAC(UI8_T *mac)
{
   int i;
   if (IS_MAC_ZERO(mac) == TRUE)
      return;

   for (i = 5; i >= 0; i--)
   {
      if ((UI8_T)(mac[i] - 1) == 0xff)
      {
         mac[i] = 0xff;
      }
      else
      {
         mac[i] = mac[i] - 1;
         return;
      }
   }


   return;
}

static BOOL_T MacFilter(UI8_T got_mac[6], UI8_T keyin_mac[6], UI8_T ignore_mask[6])
{
   UI32_T n;

   for(n=0; n<6; n++)
   {
      if( (got_mac[n] & (~ignore_mask[n])) != (keyin_mac[n] & (~ignore_mask[n])))
         return FALSE;
   }
   return TRUE;
}

UI32_T CLI_API_Show_MacAddressTable_Secure(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0/*(SYS_CPNT_PORT_SECURITY == TRUE)*/
   #define NOT_EXIST_POSITION 0xffffffff
   #define SORT_METHOD_ADDR      0
   #define SORT_METHOD_INTERFACE 1
   #define SORT_METHOD_VLAN      2

   static BOOL_T MacFilter(UI8_T got_mac[6], UI8_T keyin_mac[6], UI8_T ignore_mask[6]);

   UI32_T to_detect_position   = 0;
   UI32_T interface_position   = NOT_EXIST_POSITION;
   UI32_T mac_addr_position    = NOT_EXIST_POSITION;
   UI32_T mac_netmask_position = NOT_EXIST_POSITION;
   UI32_T vid_position         = NOT_EXIST_POSITION;
   UI32_T sort_method_position = NOT_EXIST_POSITION;
   UI32_T lport       =  0;
   UI32_T vid         =  0;
   UI8_T  mac_addr[6] = {0};
   UI8_T  mac_mask[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; /*didn't specify mac addr, mac mask => display all*/
   UI8_T  sort_method = SORT_METHOD_INTERFACE;

   /*specify address*/
   if (arg[to_detect_position][0] == 'a' || arg[to_detect_position][0] == 'a')
      to_detect_position += 1;
   /*MAC address and mask*/
   if(arg[to_detect_position])
   {
      if(CLI_LIB_ValsInMac(arg[to_detect_position], mac_addr)) /*check MAC format*/
      {
         mac_addr_position      =  to_detect_position;
         to_detect_position    += 1;

         if(CLI_LIB_ValsInMac(arg[to_detect_position], mac_mask)) /*check MAC format*/
         {
            mac_netmask_position      = to_detect_position;
            to_detect_position       += 1;
         }
         else
            memset(mac_mask, 0, sizeof(mac_mask));/*specify MAC address, check all => don't by pass any one*/
      }
      else
         memset(mac_mask, 0xff, sizeof(mac_mask)); /*no specified MAC address, ignore all => by pass all*/
   }
   else
      goto PARSING_END;

   /*interface*/
   if(arg[to_detect_position])
   {
       if(((arg[to_detect_position][0] == 'e' || arg[to_detect_position][0] == 'E') && strchr((char*)arg[to_detect_position], '-') == 0 )||
          ( arg[to_detect_position][0] == 'p' || arg[to_detect_position][0] == 'P') )
      {
         interface_position  = to_detect_position;
         to_detect_position += 2;

         if(arg[interface_position][0] == 'e' || arg[interface_position][0] == 'E') /*ethernet*/
         {
            UI32_T verify_unit = atoi(arg[interface_position+1]);
            UI32_T verify_port = atoi(strchr((char*)arg[interface_position+1],'/')+1);
            CLI_API_EthStatus_T verify_ret;
#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[interface_position+1][0]))
            {
               verify_unit = atoi(arg[interface_position+1]);
               verify_port = atoi(strchr((char*)arg[interface_position+1],'/')+1);
            }
            else/*port name*/
            {
               UI32_T trunk_id = 0;
               if (!IF_PMGR_IfnameToIfindex(arg[interface_position+1], &lport))
               {
#if (SYS_CPNT_EH == TRUE)
                  CLI_API_Show_Exception_Handeler_Msg();
#else
                  CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[interface_position+1]);
#endif
                  return CLI_NO_ERROR;
               }
               SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
            }
#endif

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               return CLI_NO_ERROR;
            }
         }
         else                                                                       /*port-channel*/
         {
            UI32_T trunk_id = atoi(arg[interface_position+1]);
            CLI_API_TrunkStatus_T verify_ret;

            if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
               display_trunk_msg(verify_ret, trunk_id);
               return CLI_NO_ERROR;
            }
         }
      }
   }
   else
      goto PARSING_END;

   /*VLAN id*/
   if(arg[to_detect_position])
   {
      if(arg[to_detect_position][0] == 'v' || arg[to_detect_position][0] == 'V')
      {
         vid_position = to_detect_position;
         vid = atoi(arg[to_detect_position + 1]);
         to_detect_position += 2;
      }
   }
   else
      goto PARSING_END;

   /*sort method*/
   if(arg[to_detect_position]) /*if the argument exists, this must be "sort xxxx"*/
   {
      sort_method_position = to_detect_position + 1;

      switch(arg[sort_method_position][0])
      {
      case 'a':
      case 'A':
         sort_method = SORT_METHOD_ADDR;
         break;

      case 'i':
      case 'I':
         sort_method = SORT_METHOD_INTERFACE;
         break;

      case 'v':
      case 'V':
         sort_method = SORT_METHOD_VLAN;
         break;
      }
   }
   else
      goto PARSING_END;

   PARSING_END:
   {
      PSEC_MGR_PortSecAddrEntry_T port_sec_addr_entry;
      UI32_T               unit;
      UI32_T               port;
      UI32_T               trunk_id;

      char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
      UI8_T  tmp_buff[CLI_DEF_MAX_BUFSIZE/2] = {0};
      UI32_T line_num = 0;

      memset(&port_sec_addr_entry, 0, sizeof(PSEC_MGR_PortSecAddrEntry_T));

      switch(sort_method)
      {
      case SORT_METHOD_ADDR:
         PROCESS_MORE(" MAC Address       VLAN Interface Type\r\n");
         PROCESS_MORE(" ----------------- ---- --------- -----------------\r\n");

         while(PSEC_PMGR_GetNextPortSecAddrEntry(&port_sec_addr_entry))
         {
            if(interface_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_port != lport)
               continue; /*charles, Mar 18, 2002*/

            if(vid_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_fdb_id != vid)
               continue;

            if(MacFilter(port_sec_addr_entry.port_sec_addr_address, mac_addr, mac_mask))
            {
               /*MAC*/
               SYSFUN_Sprintf((char*)buff, " %02X-%02X-%02X-%02X-%02X-%02X", port_sec_addr_entry.port_sec_addr_address[0], port_sec_addr_entry.port_sec_addr_address[1], port_sec_addr_entry.port_sec_addr_address[2],
                                                               port_sec_addr_entry.port_sec_addr_address[3], port_sec_addr_entry.port_sec_addr_address[4], port_sec_addr_entry.port_sec_addr_address[5]);

               /*vlan*/
               SYSFUN_Sprintf((char*)tmp_buff, " %4lu", port_sec_addr_entry.port_sec_addr_fdb_id);
               strcat((char*)buff, (char*)tmp_buff);

               /*interface*/
               if(SWCTRL_POM_LogicalPortToUserPort(port_sec_addr_entry.port_sec_addr_port, &unit, &port, &trunk_id)==SWCTRL_LPORT_TRUNK_PORT)
                  SYSFUN_Sprintf((char*)tmp_buff, "   Trunk %lu", trunk_id);
               else
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(port_sec_addr_entry.port_sec_addr_port,name);
                  if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                  {                                        /*pttch 2002.07.10*/
                     name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                  }
                  SYSFUN_Sprintf((char*)tmp_buff, "  %8s", name);
               }
#else
                  SYSFUN_Sprintf((char*)tmp_buff, "  Eth %lu/%2lu", unit, port);
#endif

               strcat((char*)buff, (char*)tmp_buff);

               SYSFUN_Sprintf((char*)tmp_buff, " Security\r\n");
               strcat((char*)buff, (char*)tmp_buff);

               /*print out*/
               PROCESS_MORE(buff);
            }
         }
         break;

      case SORT_METHOD_INTERFACE:
         PROCESS_MORE(" Interface MAC Address       VLAN Type\r\n");
         PROCESS_MORE(" --------- ----------------- ---- -----------------\r\n");

         port_sec_addr_entry.port_sec_addr_port = lport;
         while(PSEC_PMGR_GetNextPortSecAddrEntry(&port_sec_addr_entry))
         {
            if(interface_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_port > lport) /*user specify and the next one is not the specified interface*/
               break;

            if(vid_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_fdb_id != vid)
               continue;

            if(MacFilter(port_sec_addr_entry.port_sec_addr_address, mac_addr, mac_mask))
            {
               /*interface*/
               if(SWCTRL_POM_LogicalPortToUserPort(port_sec_addr_entry.port_sec_addr_port, &unit, &port, &trunk_id)==SWCTRL_LPORT_TRUNK_PORT)
                  SYSFUN_Sprintf((char*)buff, "   Trunk %lu", trunk_id);
               else
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(port_sec_addr_entry.port_sec_addr_port,name);
                  if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                  {                                        /*pttch 2002.07.10*/
                     name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                  }
                  SYSFUN_Sprintf((char*)buff, "  %8s", name);
               }
#else
                  SYSFUN_Sprintf((char*)buff, "  Eth %lu/%2lu", unit, port);
#endif

               /*MAC*/
               SYSFUN_Sprintf((char*)tmp_buff, " %02X-%02X-%02X-%02X-%02X-%02X", port_sec_addr_entry.port_sec_addr_address[0], port_sec_addr_entry.port_sec_addr_address[1], port_sec_addr_entry.port_sec_addr_address[2],
                                                                   port_sec_addr_entry.port_sec_addr_address[3], port_sec_addr_entry.port_sec_addr_address[4], port_sec_addr_entry.port_sec_addr_address[5]);
               strcat((char*)buff,(char*)tmp_buff);

               /*vlan*/
               SYSFUN_Sprintf((char*)tmp_buff, " %4lu", port_sec_addr_entry.port_sec_addr_fdb_id);
               strcat((char*)buff, (char*)tmp_buff);

               SYSFUN_Sprintf((char*)tmp_buff, " Security\r\n");
               strcat((char*)buff, (char*)tmp_buff);

               PROCESS_MORE(buff);
            }
         }
         break;

      case SORT_METHOD_VLAN:
         PROCESS_MORE(" VLAN MAC Address       Interface Type\r\n");
         PROCESS_MORE(" ---- ----------------- --------- -----------------\r\n");

         while(PSEC_PMGR_GetNextPortSecAddrEntry(&port_sec_addr_entry))
         {
            if(interface_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_port != lport)
               continue; /*charles, Mar 18, 2002*/

            if(vid_position != NOT_EXIST_POSITION && port_sec_addr_entry.port_sec_addr_fdb_id > vid)
               break;

            if(MacFilter(port_sec_addr_entry.port_sec_addr_address, mac_addr, mac_mask))
            {
               /*vlan*/
               SYSFUN_Sprintf((char*)buff, " %4lu", port_sec_addr_entry.port_sec_addr_fdb_id);

               /*MAC*/
               SYSFUN_Sprintf((char*)tmp_buff, " %02X-%02X-%02X-%02X-%02X-%02X", port_sec_addr_entry.port_sec_addr_address[0], port_sec_addr_entry.port_sec_addr_address[1], port_sec_addr_entry.port_sec_addr_address[2],
                                                                   port_sec_addr_entry.port_sec_addr_address[3], port_sec_addr_entry.port_sec_addr_address[4], port_sec_addr_entry.port_sec_addr_address[5]);
               strcat((char*)buff, (char*)tmp_buff);

               /*interface*/
               if(SWCTRL_POM_LogicalPortToUserPort(port_sec_addr_entry.port_sec_addr_port, &unit, &port, &trunk_id)==SWCTRL_LPORT_TRUNK_PORT)
                  SYSFUN_Sprintf((char*)tmp_buff, "   Trunk %lu", trunk_id);
               else
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(port_sec_addr_entry.port_sec_addr_port,name);
                  if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                  {                                        /*pttch 2002.07.10*/
                     name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                  }
                  SYSFUN_Sprintf((char*)tmp_buff, "  %8s", name);
               }
#else
                  SYSFUN_Sprintf((char*)tmp_buff, "  Eth %lu/%2lu", unit, port);
#endif

               strcat((char*)buff, (char*)tmp_buff);

               SYSFUN_Sprintf((char*)tmp_buff, " Security\r\n");
               strcat((char*)buff,(char*)tmp_buff);

               /*print out*/
               PROCESS_MORE(buff);
            }
         }
         break;
      }
   }
#endif
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_MacAddressTable_AgingTime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T aging = 0;
    UI32_T status;

    if( AMTR_PMGR_GetAgingStatus(&status)!= TRUE )
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get address table aging status\r\n");
#endif
        return CLI_ERR_INTERNAL;
    }
    else
    {
        if (status == VAL_amtrMacAddrAgingStatus_enabled)
        {
            CLI_LIB_PrintStr(" Aging Status : Enabled\r\n");
        }
        else if (status == VAL_amtrMacAddrAgingStatus_disabled)
        {
            CLI_LIB_PrintStr(" Aging Status : Disabled\r\n");
        }
    }
   if (!AMTR_PMGR_GetDot1dTpAgingTime(&aging))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get address table aging time\r\n");
#endif
      return CLI_ERR_INTERNAL;
   }

   CLI_LIB_PrintStr_1(" Aging Time: %lu sec.\r\n",(unsigned long)aging);
   return CLI_NO_ERROR;
}

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
UI32_T CLI_API_Show_MacAddressTable_HashLookupDepth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T lookup_depth_config;
    UI32_T lookup_depth_chip;

    if (!AMTR_PMGR_GetHashLookupDepthFromConfig(&lookup_depth_config))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get hash lookup depth from config\r\n");
#endif
        return CLI_ERR_INTERNAL;
    }

    if (!AMTR_PMGR_GetHashLookupDepthFromChip(&lookup_depth_chip))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get hash lookup depth from chip\r\n");
#endif
        return CLI_ERR_INTERNAL;
    }

   CLI_LIB_PrintStr_1(" Configured Hash Lookup Depth: %lu\r\n",(unsigned long)lookup_depth_config);
   CLI_LIB_PrintStr_1(" Activated Hash Lookup Depth: %lu\r\n",(unsigned long)lookup_depth_chip);

   return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_Spanningtree_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T type_ret = XSTP_TYPE_RETURN_ERROR;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
        verify_port = i;
            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
               display_ethernet_msg(verify_ret, verify_unit, verify_port);
               continue;
            }
            else
            {
                switch(cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_SPANNINGTREE:
                        type_ret = XSTP_PMGR_SetPortSpanningTreeStatus(lport, VAL_staPortSystemStatus_enabled);
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_SPANNINGTREE:
                        type_ret = XSTP_PMGR_SetPortSpanningTreeStatus(lport, VAL_staPortSystemStatus_disabled);
                        break;
                    default:
                        break;
                }
            }
            if(type_ret!=XSTP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
               {
                  UI8_T name[MAXSIZE_ifName+1] = {0};
                  CLI_LIB_Ifindex_To_Name(lport,name);
                  CLI_LIB_PrintStr_1("Failed to set spanning-tree on ethernet %s.\r\n", name);
               }
#else
               CLI_LIB_PrintStr_2("Failed to set spanning-tree on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Spanningtree_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_UI == TRUE)
   UI32_T lport;
   UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
   CLI_API_TrunkStatus_T verify_ret;
   UI32_T type_ret = XSTP_TYPE_RETURN_ERROR;

   if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
   {
      display_trunk_msg(verify_ret, verify_trunk_id);
      return CLI_NO_ERROR;
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_SPANNINGTREE:
        type_ret = XSTP_PMGR_SetPortSpanningTreeStatus(lport, VAL_staPortSystemStatus_enabled);
        break;
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_SPANNINGTREE:
        type_ret = XSTP_PMGR_SetPortSpanningTreeStatus(lport, VAL_staPortSystemStatus_disabled);
        break;
    }

    if(type_ret!=XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_1("Failed to set spanning-tree on trunk %lu\r\n",  (unsigned long)verify_trunk_id);
    }
#endif /* End of #if (SYS_CPNT_XSTP_UI == TRUE) */
    return CLI_NO_ERROR;
}


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SPANNINGTREE_PER_VLAN
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree"
 *            in vlan database mode just noly for PVST
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SPANNINGTREE_PER_VLAN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==TRUE)
    UI32_T vid;
    BOOL_T spanning_tree_status_vlan;
    UI32_T spanning_tree_status_global;
    UI32_T spanning_tree_return_value;

    XSTP_PMGR_GetSystemSpanningTreeStatus(&spanning_tree_status_global);

    switch(spanning_tree_status_global)
    {
        case VAL_staSystemStatus_disabled:
            CLI_LIB_PrintStr("Failed to set spanning-tree in vlan database mode. Because spanning-tree is disabled in global mode.\r\n");
        break;

        case VAL_staSystemStatus_enabled:
            CLI_API_GET_VLAN_DATABASE_VLANID(&vid);

            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W1_SPANNINGTREE:
                    spanning_tree_status_vlan = TRUE;
                break;

                case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W2_NO_SPANNINGTREE:
                    spanning_tree_status_vlan = FALSE;
                break;

                default:
                break;
            }

            spanning_tree_return_value = XSTP_PMGR_SetVlanSpanningTreeStatus(vid,spanning_tree_status_vlan);
            if(spanning_tree_return_value!=XSTP_TYPE_RETURN_OK)
            {
                switch(spanning_tree_return_value)
                {
                    case XSTP_TYPE_RETURN_ERROR:
                        CLI_LIB_PrintStr("Failed to set spanning-tree in vlan database mode.\r\n");
                    break;

                    case XSTP_TYPE_RETURN_MASTER_MODE_ERROR:
                        CLI_LIB_PrintStr("The unit is not master mode.\r\n");
                    break;

                    case XSTP_TYPE_RETURN_INDEX_NEX:
                        CLI_LIB_PrintStr("Vlan does not exist.\r\n");
                    break;

                    default:
                    break;
                }
            }
        break;

        default:
        break;
    }

#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_RootGuard_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree root-guard"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_RootGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T status = 0;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_ROOTGUARD:
            status = VAL_staPortRootGuardAdminStatus_enabled;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_ROOTGUARD:
            status = VAL_staPortRootGuardAdminStatus_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 <<( 7-((i-1)%8))))
        {
            verify_port = i;

            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (XSTP_PMGR_SetPortRootGuardStatus(lport, status) != XSTP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_1("Failed to set root-guard status on ethernet %s\r\n", name);
    #endif
                }
#else
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_2("Failed to set root-guard status on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
#endif
            }
        }
    }
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_RootGuard_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree root-guard"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_RootGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_ROOT_GUARD == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T status = 0;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_ROOTGUARD:
            status = VAL_staPortRootGuardAdminStatus_enabled;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_ROOTGUARD:
            status = VAL_staPortRootGuardAdminStatus_disabled;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (XSTP_PMGR_SetPortRootGuardStatus(lport, status) != XSTP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set root-guard status on trunk %lu\r\n", (unsigned long)verify_trunk_id);
#endif
    }
#endif /* #if (SYS_CPNT_STP_ROOT_GUARD == TRUE) */
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduGuard_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduGuard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T ret;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1<<(7-((i-1)%8))))
        {
            verify_port = i;
            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch (cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_BPDUGUARD:
                        ret = XSTP_PMGR_SetPortBpduGuardStatus(lport, XSTP_TYPE_PORT_BPDU_GUARD_ENABLED);
                        break;

                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_BPDUGUARD:
                        ret = XSTP_PMGR_SetPortBpduGuardStatus(lport, XSTP_TYPE_PORT_BPDU_GUARD_DISABLED);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }

            if (ret != XSTP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    CLI_LIB_PrintStr_1("Failed to set BPDU guard on ethernet %s.\r\n", name);
                }
#else
                CLI_LIB_PrintStr_2("Failed to set BPDU guard on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduGuard_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduGuard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_BPDUGUARD:
            ret = XSTP_PMGR_SetPortBpduGuardStatus(lport, XSTP_TYPE_PORT_BPDU_GUARD_ENABLED);
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_BPDUGUARD:
            ret = XSTP_PMGR_SetPortBpduGuardStatus(lport, XSTP_TYPE_PORT_BPDU_GUARD_DISABLED);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (ret != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_1("Failed to set BPDU guard on port-channel %lu\r\n",  (unsigned long)verify_trunk_id);
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SpanningTree_BpduGuard_AutoRecovery_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard auto-recovery"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SpanningTree_BpduGuard_AutoRecovery_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T status = 0;
    UI32_T interval;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1<<(7-((i-1)%8))))
        {
            verify_port = i;
            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch (cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SPANNINGTREE_BPDUGUARD_AUTORECOVERY:
                        if (arg[0] == NULL)
                        {
                            status = XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED;
                        }
                        else
                        {
                            interval = atoi(arg[1]);
                        }
                        break;

                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SPANNINGTREE_BPDUGUARD_AUTORECOVERY:
                        if (arg[0] == NULL)
                        {
                            status = XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_DISABLED;
                        }
                        else
                        {
                            interval = XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL;
                        }
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }

            if (arg[0] == NULL)
            {
                if (XSTP_PMGR_SetPortBPDUGuardAutoRecovery(lport, status) != XSTP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        CLI_LIB_PrintStr_1("Failed to set BPDU guard auto recovery on ethernet %s.\r\n", name);
                    }
#else
                    CLI_LIB_PrintStr_2("Failed to set BPDU guard auto recovery on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }
            }
            else
            {
                if (XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval(lport, interval) != XSTP_TYPE_RETURN_OK)
                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        CLI_LIB_PrintStr_1("Failed to set BPDU guard auto recovery interval on ethernet %s.\r\n", name);
                    }
#else
                    CLI_LIB_PrintStr_2("Failed to set BPDU guard auto recovery interval on ethernet %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_SpanningTree_BpduGuard_AutoRecovery_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdu-guard auto-recovery"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_SpanningTree_BpduGuard_AutoRecovery_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_GUARD == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T status = 0;
    UI32_T interval;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_SPANNINGTREE_BPDUGUARD_AUTORECOVERY:
            if (arg[0] == NULL)
            {
                status = XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_ENABLED;
            }
            else
            {
                interval = atoi(arg[1]);
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_SPANNINGTREE_BPDUGUARD_AUTORECOVERY:
            if (arg[0] == NULL)
            {
                status = XSTP_TYPE_PORT_BPDU_GUARD_AUTO_RECOVERY_DISABLED;
            }
            else
            {
                interval = XSTP_TYPE_DEFAULT_PORT_BPDU_GUARD_AUTO_RECOVERY_INTERVAL;
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (arg[0] == NULL)
    {
        if (XSTP_PMGR_SetPortBPDUGuardAutoRecovery(lport, status) != XSTP_TYPE_RETURN_OK)
        {
            CLI_LIB_PrintStr_1("Failed to set BPDU guard auto recovery on port-channel %lu.\r\n",  (unsigned long)verify_trunk_id);
        }
    }
    else
    {
        if (XSTP_PMGR_SetPortBPDUGuardAutoRecoveryInterval(lport, interval) != XSTP_TYPE_RETURN_OK)
        {
            CLI_LIB_PrintStr_1("Failed to set BPDU guard auto recovery interval on port-channel %lu.\r\n",  (unsigned long)verify_trunk_id);
        }
    }
#endif /* #if (SYS_CPNT_STP_BPDU_GUARD == TRUE) */
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduFilter_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdug-filtering"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduFilter_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    UI32_T ret;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        UI32_T lport = 0;
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 <<(7-((i-1)%8))))
        {
            verify_port = i;
            if ((verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch (cmd_idx)
                {
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_BPDUFILTER:
                        ret = XSTP_PMGR_SetPortBpduFilterStatus(lport, XSTP_TYPE_PORT_BPDU_FILTER_ENABLED);
                        break;

                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_BPDUFILTER:
                        ret = XSTP_PMGR_SetPortBpduFilterStatus(lport, XSTP_TYPE_PORT_BPDU_FILTER_DISABLED);
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
            }
            if (ret != XSTP_TYPE_RETURN_OK)
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    CLI_LIB_PrintStr_1("Failed to set BPDU guard on ethernet %s.\r\n", name);
                }
#else
                CLI_LIB_PrintStr_2("Failed to set BPDU filter on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_BpduFilter_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanningtree bpdug-filtering"
 *            in interface portchannel configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_BpduFilter_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_BPDU_FILTER == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T ret;

    if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_BPDUFILTER:
            ret = XSTP_PMGR_SetPortBpduFilterStatus(lport, XSTP_TYPE_PORT_BPDU_FILTER_ENABLED);
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_BPDUFILTER:
            ret = XSTP_PMGR_SetPortBpduFilterStatus(lport, XSTP_TYPE_PORT_BPDU_FILTER_DISABLED);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (ret != XSTP_TYPE_RETURN_OK)
    {
        CLI_LIB_PrintStr_1("Failed to set BPDU filter on port-channel %lu\r\n",  (unsigned long)verify_trunk_id);
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_CiscoPrestandard
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "spanning-tree cisco-prestandard-compatibility"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_CiscoPrestandard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE)
    UI32_T type_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_SPANNINGTREE_CISCOPRESTANDARD:
            type_ret = XSTP_PMGR_SetCiscoPrestandardCompatibility(XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_ENABLED);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SPANNINGTREE_CISCOPRESTANDARD:
            type_ret = XSTP_PMGR_SetCiscoPrestandardCompatibility(XSTP_TYPE_CISCO_PRESTANDARD_COMPATIBILITY_DISABLED);
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if (type_ret != XSTP_TYPE_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set cisco prestandard compatibility \r\n");
#endif
    }
#endif /* End of #if (SYS_CPNT_STP_COMPATIBLE_WITH_CISCO_PRESTANDARD == TRUE) */
    return CLI_NO_ERROR;
}/* End of CLI_API_Spanningtree_CiscoPrestandard() */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_tc_prop_stop_Eth
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop-stop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_tc_prop_stop_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    UI32_T i;
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    BOOL_T status = FALSE;

    switch(cmd_idx)
    {
      case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SPANNINGTREE_TCPROPSTOP:
        status = TRUE;
        break;

      case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SPANNINGTREE_TCPROPSTOP:
        status = FALSE;
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

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if(XSTP_PMGR_SetPortTcPropStop(lport, status) != XSTP_TYPE_RETURN_OK)
            {
              CLI_LIB_PrintStr_2("Failed to set TC propagate on Eth%lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_tc_prop_stop_Pch
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop-stop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_tc_prop_stop_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_TC_PROP_STOP == TRUE)
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
    BOOL_T status = FALSE;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
      case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SPANNINGTREE_TCPROPSTOP:
        status = TRUE;
        break;

      case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SPANNINGTREE_TCPROPSTOP:
        status = FALSE;
        break;

      default:
          return CLI_ERR_INTERNAL;
    }
    if(XSTP_PMGR_SetPortTcPropStop(lport, status) != XSTP_TYPE_RETURN_OK)
    {
      CLI_LIB_PrintStr("Failed to set TC propagate\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Spanningtree_SetTcPropGroup
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop group"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Spanningtree_SetTcPropGroup(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T lport=0, trunk_id=0;
    UI32_T group_id = atoi((char*)arg[0]);
    BOOL_T ret;
    BOOL_T is_add = FALSE, del_group=FALSE;
    UI8_T  pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ]={0};

    /* transform interfaces into one-bit port-list
     */
    if(arg[1]!=NULL)
    {
        if (arg[1][0] == 'e' || arg[1][0] == 'E')
        {
            ret = cli_api_spanningtree_eth_port_to_bmap(arg[2], pbmp);
            if (!ret)
            {
                return CLI_NO_ERROR;
            }
        }
        else if (arg[1][0] == 'p' || arg[1][0] == 'P')
        {
            trunk_id = atoi(arg[2]);
            if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, trunk_id);
                return CLI_NO_ERROR;
            }
            L_BITMAP_PORT_SET(pbmp, lport);
        }
    }
    else
    {
        del_group = TRUE;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_SPANNINGTREE_TCPROP_GROUP:
            is_add = TRUE;
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_SPANNINGTREE_TCPROP_GROUP:
            is_add = FALSE;
            break;

      default:
          return CLI_ERR_INTERNAL;
    }

    if (del_group == TRUE)
    {
        if(XSTP_PMGR_DelTcPropGroup(group_id) != XSTP_TYPE_RETURN_OK)
        {
            CLI_LIB_PrintStr_1("Failed to delete group %lu\r\n", (unsigned long)group_id);
        }
    }
    else
    {
        if(XSTP_PMGR_SetTcPropGroupPortList(is_add, group_id, pbmp) != XSTP_TYPE_RETURN_OK)
        {
            if (is_add == TRUE)
            {
                CLI_LIB_PrintStr_1("Failed to add port to group %lu\r\n", (unsigned long)group_id);
            }
            else
            {
                CLI_LIB_PrintStr_1("Failed to remove port from group %lu\r\n", (unsigned long)group_id);
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Spanningtree_TcProp
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "Spanning-tree tc-prop"
 *            in interface ethernet configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   : CLI_ERROR_CODE_E of CLI(ref to cli_msg.h)
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Spanningtree_TcProp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
    BOOL_T have_port = FALSE;
    UI8_T  pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST ]={0};
    UI32_T group_id = XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID;
    UI32_T line_num = 0;

    if(arg[0] == NULL)
    {
        if (   (XSTP_POM_GetTcPropGroupPortbitmap(group_id, pbmp, &have_port))
            && (have_port)
           )
        {
            line_num = cli_api_show_spanningtree_one_group(group_id, pbmp, line_num);
            CLI_API_STA_CHK_LINE_NUM(line_num);
        }
        while (XSTP_POM_GetTcPropNextGroupPortbitmap(&group_id, pbmp, &have_port))
        {
            if (have_port)
            {
                CLI_LIB_PrintStr("\r\n");
                line_num = cli_api_show_spanningtree_one_group(group_id, pbmp, line_num);
                CLI_API_STA_CHK_LINE_NUM(line_num);
            }
        }
    }
    else
    {
        group_id = atoi((char*)arg[1]);
        if (XSTP_POM_GetTcPropGroupPortbitmap(group_id, pbmp, &have_port))
        {
            if (have_port)
            {
                line_num = cli_api_show_spanningtree_one_group(group_id, pbmp, line_num);
                CLI_API_STA_CHK_LINE_NUM(line_num);
            }
            else
            {
                CLI_LIB_PrintStr_1("No port in the group %lu\r\n", (unsigned long)group_id);
            }
        }
        else
        {
            CLI_LIB_PrintStr_1("No such group %lu\r\n", (unsigned long)group_id);
        }
    }
#endif
    return CLI_NO_ERROR;
}

#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)
static BOOL_T cli_api_spanningtree_eth_port_to_bmap(char *unit_port_str, UI8_T *lport_pbmp)
{
    UI32_T unit = atoi(unit_port_str);
    UI32_T port;
    UI32_T lower_val;
    UI32_T upper_val;
    UI32_T err_idx;
    UI32_T lport;
    BOOL_T is_fail = FALSE;
    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE];
    char   delemiters[2] = {0};


    op_ptr = strchr(unit_port_str, '/') + 1;
    delemiters[0] = ',';

    do
    {
        memset(Token, 0, sizeof(Token));
        op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

        if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
        {
            return FALSE;
        }
        else
        {
            for(port = lower_val; port<=upper_val; port++)
            {
                CLI_API_EthStatus_T verify_ret;
                verify_ret = verify_ethernet(unit, port, &lport);

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        L_BITMAP_PORT_SET(lport_pbmp, lport);
                        break;

                    default:
                        display_ethernet_msg(verify_ret, unit, port);
                        is_fail = TRUE;
                        break;
                }
            }
        }
    } while(op_ptr != 0 && !isspace(*op_ptr));

    if(is_fail)
        return FALSE;
    else
        return TRUE;
}


static UI32_T cli_api_show_spanningtree_one_group(UI32_T group_id, UI8_T pbmp[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T line_num)
{
    SWCTRL_Lport_Type_T port_type;
    UI32_T ifindex = 0, unit = 0, port = 0, trunk_id = 0;
    UI32_T port_counter = 0;
    UI8_T  buffTempPort[20]  = {0};
    char   buffTemp[CLI_DEF_MAX_BUFSIZE]  = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE]  = {0};

    if (group_id == XSTP_TYPE_TC_PROP_DEFAULT_GROUP_ID)
    {
        PROCESS_MORE_FUNC(" Default Group");
    }
    else
    {
        snprintf(buff, sizeof(buff), " Group %lu", (unsigned long)group_id);
        PROCESS_MORE_FUNC(buff);
    }

    for (ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT ; ifindex++)
    {
        if(   (L_BITMAP_PORT_ISSET(pbmp, ifindex))
           && (SWCTRL_POM_LogicalPortExisting(ifindex))
          )
        {
            port_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

            if (   (port_type == SWCTRL_LPORT_TRUNK_PORT)
                && (TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
               )
               continue;

            port_counter++;

            if (port_counter%5 == 1)
            {
                strncat(buffTemp, "\r\n", sizeof(buffTemp));
                PROCESS_MORE_FUNC(buffTemp);
                memset(buffTemp, 0, sizeof(buffTemp));
                strncat(buffTemp, "  ", sizeof(buffTemp));
            }

            if (port_type == SWCTRL_LPORT_NORMAL_PORT )
            {
                snprintf((char *)buffTempPort, sizeof(buffTempPort), "Eth %lu/%2lu, ",(unsigned long)unit,(unsigned long)port);
            }
            else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                snprintf((char *)buffTempPort, sizeof(buffTempPort), "Trunk %2lu, ",(unsigned long)trunk_id);
            }

            strncat(buffTemp, (char *)buffTempPort, sizeof(buffTemp));
            memset(buffTempPort, 0, sizeof(buffTempPort));
        }
    }

    if(buffTemp[strlen(buffTemp)-2]==',')
    {
        buffTemp[strlen(buffTemp)-2]=' ';
    }

    strncat(buffTemp,"\r\n", sizeof(buffTemp));
    PROCESS_MORE_FUNC(buffTemp);
    return line_num;
}
#endif /*#if(SYS_CPNT_XSTP_TC_PROP_GROUP == TRUE)*/

