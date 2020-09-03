/*system*/
#include "sysfun.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include <time.h>

//#include "skt_vx.h"
//#include "socket.h"

#include "sys_dflt.h"

#include "sys_pmgr.h"    /* 2007-12, Joseph */

/*cli internal*/
#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
#include "cli_type.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"
#include "cli_msg.h"
#include "cli_cmd.h"
#include "cli_auth.h"
#include "cli_runcfg.h"

#if (CLI_SUPPORT_L2_DHCP_RELAY == 1)
#include "brg_mgr.h"
#endif
#if (CLI_SUPPORT_DHCP_CLIENTID == 1)
#include "dhcp_mgr.h"
#endif
#include "dhcp_type.h"
#include "dhcp_pmgr.h"
#include "dhcp_mgr.h"
#include "dhcp_pom.h"
#include "dhcp_om.h"

#if (SYS_CPNT_DHCP_INFORM==TRUE)
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#endif
#define USED_TO_PRT_ARGS                                                     \
        {  int counter;                                                      \
           for (counter=0; arg[counter]!=0; counter++)                       \
           { CLI_LIB_PrintStr_2("arg[%d] = %s\r\n", counter, arg[counter]);} \
        }


UI32_T CLI_API_IP_Dhcp_Relay(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) 
{
#if (CLI_SUPPORT_L2_DHCP_RELAY == 1)
   BOOL_T status = FALSE;
   UI32_T vid;
   
   VLAN_MGR_ConvertFromIfindex(ctrl_P->CMenu.vlan_ifindex, &vid);

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_DHCP_RELAY:
      status = BRG_MGR_DHCP_ENABLED;
      break;
      
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_DHCP_RELAY:
      status = BRG_MGR_DHCP_DISABLED;
      break;
      
   default:
      break;	
   }
   if (!BRG_MRG_SetBridgeDHCPRelayVlanId(vid,status))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to %s IP DHCP relay\r\n",(status == BRG_MGR_DHCP_ENABLED)?("enable"):("disable"));
#endif
   }
#endif   
   return CLI_NO_ERROR;	
}
/* command not support */
#if 0
UI32_T CLI_API_IP_Dhcp_ClientIdentifier(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) 
{
#if (CLI_SUPPORT_DHCP_CLIENTID == 1)
   UI32_T id_mode;
   UI32_T length;
   UI8_T  str[MAXSIZE_dhcpcIfClientId] = {0};
   
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_DHCP_CLIENTIDENTIFIER:
      switch(arg[0][0])
      {
      case 'h':
      case 'H':
         id_mode = DHCP_MGR_CID_HEX;
         {
            UI32_T i;
            UI32_T j = 0;
            UI8_T  temp_str[MAXSIZE_dhcpcIfClientId*2] = {0};
            char  buff[3] = {0}; 
            if (strlen(arg[1])%2 != 0)
            {
               length = strlen(arg[1])/2 + 1;
               temp_str[0] = '0';
               memcpy(&(temp_str[1]), arg[1], sizeof(UI8_T)*strlen(arg[1]));	
            } 
            else
            {
               length = strlen(arg[1])/2;
               memcpy(temp_str, arg[1], sizeof(UI8_T)*strlen(arg[1]));	
            }            
            for (i = 0; i < MAXSIZE_dhcpcIfClientId - 1; i++)   
            {
               buff[0] = temp_str[j];
               buff[1] = temp_str[j+1];
               buff[2] = 0;
               str[i] = (UI8_T)CLI_LIB_AtoUl(buff,16);
               j += 2;	
            }
         }            
         break;
         
      case 't':
      case 'T':
         id_mode = DHCP_MGR_CID_TEXT;
         length = strlen(arg[1]);
         memcpy(str, arg[1], sizeof(UI8_T)*strlen(arg[1]));	
         break;
         
      default:
         return CLI_ERR_INTERNAL;	
      }
      if (DHCP_PMGR_C_SetClientId(ctrl_P->CMenu.vlan_ifindex, id_mode, length, (char *)str) != DHCP_MGR_OK)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set DHCP client identifier\r\n");
#endif
         return CLI_NO_ERROR;	
      }
      break;
      
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_DHCP_CLIENTIDENTIFIER:
      if (DHCP_PMGR_C_DeleteClientId(ctrl_P->CMenu.vlan_ifindex) != DHCP_MGR_OK)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to remove DHCP client identifier\r\n");
#endif
         return CLI_NO_ERROR;	
      }
      break;
      
   default:
      return CLI_ERR_INTERNAL;	
   }

#endif
   return CLI_NO_ERROR;	
}
#endif
/* begin 2007-12, Joseph */

UI32_T CLI_API_IP_Dhcp_VendorClassIdentifier(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P) 
{
#if (SYS_CPNT_DHCP_CLIENT_CLASSID == TRUE)
   UI32_T id_mode = 0;
   UI32_T length = 0;
   UI8_T  str[DHCP_MGR_CLASSID_BUF_MAX_SIZE+1] = {0};
   UI8_T  model_name[DHCP_MGR_CLASSID_BUF_MAX_SIZE+1] = {0};
   
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_DHCP_CLIENT_CLASSID:

      if(arg[0] == NULL)
      {
        id_mode = DHCP_MGR_CLASSID_TEXT;
         if (TRUE == SYS_PMGR_GetModelName(0, model_name))
         {
            length = strlen((char *)model_name);
            memcpy(str, model_name, sizeof(UI8_T)*length);	
         }
         else
         {
            length = 0;
         }
      }
      else
      {
      switch(arg[0][0])
      {
      case 'h':
      case 'H':
         id_mode = DHCP_MGR_CLASSID_HEX;
         {
            UI32_T i;
            UI32_T j = 0;
            UI8_T  temp_str[DHCP_MGR_CLASSID_BUF_MAX_SIZE*2] = {0};
            char  buff[3] = {0}; 
            if (strlen(arg[1])%2 != 0)
            {
               length = strlen(arg[1])/2 + 1;
               temp_str[0] = '0';
               memcpy(&(temp_str[1]), arg[1], sizeof(UI8_T)*strlen(arg[1]));	
            } 
            else
            {
               length = strlen(arg[1])/2;
               memcpy(temp_str, arg[1], sizeof(UI8_T)*strlen(arg[1]));	
            }            
            for (i = 0; i < length; i++)
            {
               buff[0] = temp_str[j];
               buff[1] = temp_str[j+1];
               buff[2] = 0;
               str[i] = (UI8_T)CLI_LIB_AtoUl(buff,16);
               j += 2;	
            }
         }            
         break;
         
      case 't':
      case 'T':
         id_mode = DHCP_MGR_CLASSID_TEXT;
         length = strlen(arg[1]);
         memcpy(str, arg[1], sizeof(UI8_T)*strlen(arg[1]));	
         break;
         
      default:
         id_mode = DHCP_MGR_CLASSID_TEXT;
         if (TRUE == SYS_PMGR_GetModelName(0, model_name))
         {
            length = strlen((char *)model_name);
            memcpy(str, model_name, sizeof(UI8_T)*length);	
         }
         else
         {
            length = 0;
         }
         break;
      }
      }
      if (DHCP_PMGR_C_SetVendorClassId(ctrl_P->CMenu.vlan_ifindex, id_mode, length, (char *)str) != DHCP_MGR_OK)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to set DHCP vendor class identifier\r\n");
#endif
         return CLI_NO_ERROR;	
      }
      break;
      
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_DHCP_CLIENT_CLASSID:
      if (DHCP_PMGR_C_DeleteVendorClassId(ctrl_P->CMenu.vlan_ifindex) != DHCP_MGR_OK)
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to remove DHCP vendor class identifier\r\n");
#endif
         return CLI_NO_ERROR;	
      }
      break;
      
   default:
      return CLI_ERR_INTERNAL;	
   }
#endif
   return CLI_NO_ERROR;	
}

/* end 2007-12 */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Ip_Dhcp_Hostname
 *------------------------------------------------------------------------------
 * PURPOSE  : ip dhcp hostname host-name
 * NOTES    : 2006-07-07
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Ip_Dhcp_Hostname(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DHCP_HOSTNAME==TRUE)

    UI8_T  HostNameBuf[SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN + 1]={0};
    DHCP_MGR_HostName_T  host;

    memset(&host, 0, sizeof(host));

#if (SYS_CPNT_CFGDB == TRUE)
    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
    {
        return CLI_NO_ERROR;
    }
#endif

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_DHCP_HOSTNAME:

        if (arg[0] == NULL)
        {
            memset(HostNameBuf, 0, sizeof(HostNameBuf));
        }
        else
        {
            memcpy(HostNameBuf, arg[0], sizeof(UI8_T)*strlen(arg[0]));

            if( DHCP_PMGR_C_SetHostName( strlen(arg[0]), HostNameBuf )!=DHCP_MGR_OK )
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set dhcp hostname\r\n");
#endif
            }
        }
        break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_DHCP_HOSTNAME:
        {
           if ( DHCP_PMGR_C_GetHostName(&host) != DHCP_MGR_OK )
           {
#if (SYS_CPNT_EH == TRUE)
              CLI_API_Show_Exception_Handeler_Msg();
#else
              CLI_LIB_PrintStr("Can not get dhcp hostname\r\n");
#endif
           }
           else
           {
                if( DHCP_PMGR_C_SetHostName( 0, HostNameBuf )!=DHCP_MGR_OK )
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("Failed to set dhcp hostname\r\n");
#endif
                }
           }
        }
        break;
   }

#endif /* #if(SYS_CPNT_DHCP_HOSTNAME==TRUE)   */
   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_IP_Dhcp_Relay(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)

    UI32_T status;
    UI32_T policy;
    UI32_T rid_mode;
    BOOL_T subtype_format;
    
	if((DHCP_OM_OK == DHCP_POM_GetDhcpRelayOption82Status(&status))&&
       (DHCP_OM_OK == DHCP_POM_GetDhcpRelayOption82Policy(&policy)))
    {
        CLI_LIB_PrintStr("Status of DHCP relay information:\r\n");    
        if(status == DHCP_OPTION82_ENABLE)
        {
            CLI_LIB_PrintStr("Insertion of relay information: enabled.\r\n");
        }
		else if (status == DHCP_OPTION82_DISABLE)
        {
    	    CLI_LIB_PrintStr("Insertion of relay information: disabled.\r\n");
        }
		   
	    if(policy == DHCP_OPTION82_POLICY_DROP)
        {   
            CLI_LIB_PrintStr("DHCP option policy: drop.\r\n");
        }
		else if (policy == DHCP_OPTION82_POLICY_REPLACE)
        {
    	    CLI_LIB_PrintStr("DHCP option policy: replace.\r\n");
        } 
		else if (policy == DHCP_OPTION82_POLICY_KEEP)
        {
    	    CLI_LIB_PrintStr("DHCP option policy: keep.\r\n");
        } 
		               
 	}
    else
    {

        CLI_LIB_PrintStr("Fail to get status of DHCP relay information.\r\n");

    }                             

    {
        UI32_T relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};
        UI32_T index = 0;
        if(DHCP_OM_OK == DHCP_POM_GetRelayServerAddress(relay_server))
        {
            CLI_LIB_PrintStr("DHCP relay-server address: ");
            for( index = 0; index < SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; index++)
            {
                CLI_LIB_PrintStr_4("%u.%u.%u.%u",
                    ((UI8_T *)(&relay_server[index]))[0],((UI8_T *)(&relay_server[index]))[1],
                    ((UI8_T *)(&relay_server[index]))[2],((UI8_T *)(&relay_server[index]))[3]);
                if(index != (SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER -1))
                {
                    CLI_LIB_PrintStr("\r\n");
                    CLI_LIB_PrintStr("                           ");

                }
            }
            CLI_LIB_PrintStr("\r\n");
        
        }
        else
        {

            CLI_LIB_PrintStr("Fail to get status of DHCP relay server.\r\n");

        }
    }

    if(DHCP_POM_GetDhcpRelayOption82Format(&subtype_format) == DHCP_OM_OK)
    {   
        if(subtype_format)
        {
            CLI_LIB_PrintStr("DHCP sub-option format: extra subtype included\r\n");
        }
        else
        {
            CLI_LIB_PrintStr("DHCP sub-option format: no extra subtype included\r\n");
        }
    }


#if (SYS_CPNT_DHCP_RELAY_OPTION82_CONFIGURABLE_RID == TRUE)
    if(DHCP_OM_OK == DHCP_POM_GetDhcpRelayOption82RidMode(&rid_mode))
    {
        switch(rid_mode)
        {
            case DHCP_OPTION82_RID_MAC_HEX:
                CLI_LIB_PrintStr("DHCP remote id sub-option: mac address (hex encoded)\r\n");
            break;

            case DHCP_OPTION82_RID_MAC_ASCII:
                CLI_LIB_PrintStr("DHCP remote id sub-option: mac address (ascii encoded)\r\n");
            break;

            case DHCP_OPTION82_RID_IP_HEX:
                CLI_LIB_PrintStr("DHCP remote id sub-option: ip address (hex encoded)\r\n");
            break;

            case DHCP_OPTION82_RID_IP_ASCII:
                CLI_LIB_PrintStr("DHCP remote id sub-option: ip address (ascii encoded)\r\n");
            break;

            case DHCP_OPTION82_RID_CONFIGURED_STRING:
            {
                UI8_T   rid_value[SYS_ADPT_MAX_LENGTH_OF_RID+1]={0};
                if(DHCP_OM_OK != DHCP_POM_GetDhcpRelayOption82RidValue(rid_value))
                    CLI_LIB_PrintStr("Failed to get remote id value\r\n");
                CLI_LIB_PrintStr_1("DHCP remote id sub-option: %s\r\n",rid_value);
            }
            break;
        }
    }
    else
    {
        CLI_LIB_PrintStr("Fail to get RID mode of DHCP relay information.\r\n");

    }
#endif
#endif
    return CLI_NO_ERROR;		

}

UI32_T CLI_API_Ip_Dhcp_Relay_Information_Option(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)

     switch(cmd_idx)
     {
     case PRIVILEGE_CFG_GLOBAL_CMD_W5_IP_DHCP_RELAY_INFORMATION_OPTION:          

        if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Status(DHCP_OPTION82_ENABLE))
        {

            CLI_LIB_PrintStr("Fail to insert DHCP relay option82 information.\r\n");

            return CLI_NO_ERROR;
        }
        break;  
        
        
        
     case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_IP_DHCP_RELAY_INFORMATION_OPTION:        
        
        if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Status(DHCP_OPTION82_DISABLE))
        {

            CLI_LIB_PrintStr("Fail to disable DHCP relay option82.\r\n");

         return CLI_NO_ERROR;
        }
        break;   
        
    default:
        return CLI_ERR_INTERNAL;
     }
     
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Dhcp_Relay_Information_Policy(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)

    

      switch(cmd_idx)
      {
      case PRIVILEGE_CFG_GLOBAL_CMD_W5_IP_DHCP_RELAY_INFORMATION_POLICY:          

        
	    switch(arg[0][0])
          {

          case 'd':/*drop*/
          case 'D':
             if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Policy(DHCP_OPTION82_POLICY_DROP))
             {

                CLI_LIB_PrintStr("Fail to define reforwarding policy.\r\n");

                return CLI_NO_ERROR;
             }
          break;
		  
          case 'r':/*replace*/
          case 'R':
	      if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Policy(DHCP_OPTION82_POLICY_REPLACE))
	      {

               CLI_LIB_PrintStr("Fail to define reforwarding policy.\r\n");

               return CLI_NO_ERROR;
            }
	   break;

         case 'k':/*keep*/
         case 'K':
	     if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Policy(DHCP_OPTION82_POLICY_KEEP))
	     {

              CLI_LIB_PrintStr("Fail to define reforwarding policy.\r\n");

              return CLI_NO_ERROR;
            }
	   break;
	     
	   default:
            return CLI_ERR_INTERNAL;
         }	
         
      break;
            
      case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_IP_DHCP_RELAY_INFORMATION_POLICY:        

	  if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Policy(SYS_DFLT_DHCP_OPTION82_POLICY))
        {

             CLI_LIB_PrintStr("Fail to disable define reforwarding policy.\r\n");

             return CLI_NO_ERROR;
        }
      break;     
        
        
    default:
        return CLI_ERR_INTERNAL;
    }
      
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Dhcp_Relay_Information_Option_Rid_Mode(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY_OPTION82_CONFIGURABLE_RID == TRUE)
      UI8_T   rid_string[SYS_ADPT_MAX_LENGTH_OF_RID + 1] = {0};
      BOOL_T  encode_hex = SYS_DFLT_DHCP_OPTION82_RID_ENCODE_HEX;

      switch(cmd_idx)
      {
      case PRIVILEGE_CFG_GLOBAL_CMD_W6_IP_DHCP_RELAY_INFORMATION_OPTION_REMOTEID:          

        if(arg[2] !=NULL)
        {
            switch(arg[2][0])
            {
                case 'a':   /*encode ascii*/
                case 'A':
                {
                    encode_hex = FALSE;
                }
                break;

                case 'h':   /*encode hex */
                case 'H':
                {
                    encode_hex = TRUE;
                }
                break;
                
            }
                
        }
        else   /* if user doesn't specify encoded type, use default value */
            encode_hex = SYS_DFLT_DHCP_OPTION82_RID_ENCODE_HEX;   

	    switch(arg[0][0])
        {
          /* ip dhcp relay information option remote-id mac-address */
          case 'm':
          case 'M':
          {
             if(encode_hex)    /* encode hex */
             {
                if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_MAC_HEX))
                {

                    CLI_LIB_PrintStr("Fail to set remote id mode.\r\n");

                    return CLI_NO_ERROR;
                }
             }
             else              /* encode ascii */
             {
                if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_MAC_ASCII))
                {

                    CLI_LIB_PrintStr("Fail to set remote id mode.\r\n");

                    return CLI_NO_ERROR;
                }
             }
             
          }
        break;
		  /* ip dhcp relay information option remote-id ip-address */
          case 'i':
          case 'I':
          {
              if(encode_hex)   /* encode hex */
              {
                  if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_IP_HEX))
	              {

                      CLI_LIB_PrintStr("Fail to set remote id mode.\r\n");
		  
                      return CLI_NO_ERROR;
                  }
              }
              else             /* encode ascii */
              {
                  if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_IP_ASCII))
	              {

                      CLI_LIB_PrintStr("Fail to set remote id mode.\r\n");

                      return CLI_NO_ERROR;
                  }
              }
	          
          }
	    break;
        /* ip dhcp relay information option remote-id string */
        case 'S':
        case 's':
        {
            memcpy(rid_string, arg[1], sizeof(rid_string));
           
            if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_CONFIGURED_STRING))
	        {
                CLI_LIB_PrintStr("Fail to change DHCP remote-id sub-option.\r\n");
                return CLI_NO_ERROR;
            }   
     
            if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidValue(rid_string))
            {
                CLI_LIB_PrintStr("Fail to change DHCP remote-id sub-option.\r\n");
                return CLI_NO_ERROR;            
            }
        }
        break;
	    default:
            return CLI_ERR_INTERNAL;
        }	
      break;
            
      case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_IP_DHCP_RELAY_INFORMATION_OPTION_REMOTEID:        

if(arg[0] == NULL)  /* no ip dhcp relay information option remote-id */
      {
	  if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(SYS_DFLT_DHCP_OPTION82_RID_MODE))
        {

             CLI_LIB_PrintStr("Fail to disable remote id mode .\r\n");

             return CLI_NO_ERROR;
        }
      }
      else
      {
          UI32_T   mode; 
          if(DHCP_POM_GetDhcpRelayOption82RidMode(&mode)!= DHCP_OM_OK)
          {
                CLI_LIB_PrintStr("Failed to unset DHCP relay information option remote id mode\r\n");
                return CLI_NO_ERROR;
          }
          
          switch(arg[0][0])
          {
             case 'M':
             case 'm':
             {

                  if((mode != DHCP_OPTION82_RID_MAC_HEX) && (mode != DHCP_OPTION82_RID_MAC_ASCII))
                  {
                        CLI_LIB_PrintStr("Current mode is not mac-address mode\r\n");
                        return CLI_NO_ERROR;
                  }
                
                  if(SYS_DFLT_DHCP_OPTION82_RID_ENCODE_HEX)
                  {
                      if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_MAC_HEX))
                      {
                          CLI_LIB_PrintStr("Failed to unset DHCP relay information option remote id mode\r\n");
                          return CLI_NO_ERROR;
                      }
                  }
                  else
                  {
                      if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_MAC_ASCII))
                      {
                          CLI_LIB_PrintStr("Failed to unset DHCP relay information option remote id mode\r\n");
                          return CLI_NO_ERROR;
                      }
                  }
             }
             break;
             case 'I':
             case 'i':
             {
                  if((mode != DHCP_OPTION82_RID_IP_HEX) && (mode != DHCP_OPTION82_RID_IP_ASCII))
                  {
                        CLI_LIB_PrintStr("Current mode is not ip-address mode\r\n");
                        return CLI_NO_ERROR;
                  }
                
                  if(SYS_DFLT_DHCP_OPTION82_RID_ENCODE_HEX)
                  {
                      if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_IP_HEX))
                      {
                          CLI_LIB_PrintStr("Failed to unset DHCP relay information option remote id mode\r\n");
                          return CLI_NO_ERROR;
                      }
                  }
                  else
                  {
                      if(DHCP_MGR_OK != DHCP_PMGR_SetOption82RidMode(DHCP_OPTION82_RID_IP_ASCII))
                      {
                         CLI_LIB_PrintStr("Failed to unset DHCP relay information option remote id mode\r\n");
                         return CLI_NO_ERROR;
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

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Dhcp_Relay_Information_Option_Encode(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W6_IP_DHCP_RELAY_INFORMATION_OPTION_ENCODE:
        {
            if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Format(FALSE))
            {
                CLI_LIB_PrintStr("Failed to set DHCP relay information option sub-option format\r\n");
            }
        }
        break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W7_NO_IP_DHCP_RELAY_INFORMATION_OPTION_ENCODE:   
        {
            if(DHCP_MGR_OK != DHCP_PMGR_SetOption82Format(TRUE))
            {
                CLI_LIB_PrintStr("Failed to set DHCP relay information option sub-option format\r\n");
            }
        }
        break;
        default:
        return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_Dhcp_Relay_Server(UI16_T cmd_idx, char *arg[],CLI_TASK_WorkingArea_T  *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)

   UI32_T   ip_Address[5]={0};
   UI32_T   number_of_relay_server=0;
   UI32_T   count;
   

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W4_IP_DHCP_RELAY_SERVER:
      for (count=0;count<5;count++)
      {
          if(arg[count]!=NULL)
          {
              CLI_LIB_AtoIp((char *)arg[count], (UI8_T*)&ip_Address[count]);
              number_of_relay_server++;
          }
          else
          {
              break;
          }
      }
      
      if (DHCP_MGR_OK != DHCP_PMGR_SetRelayServerAddress(ip_Address[0],ip_Address[1],ip_Address[2],ip_Address[3],ip_Address[4]))
      {
      	 CLI_LIB_PrintStr("The setup for relay server ip operation is failed.\n\r");

      }
       break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IP_DHCP_RELAY_SERVER:

      if (DHCP_MGR_OK != DHCP_PMGR_DeleteGlobalRelayServerAddress())
      {
      	 CLI_LIB_PrintStr("The setup for relay server ip operation is failed.\n\r");

      }
      break;

    default:
        return CLI_ERR_INTERNAL;

   }

#endif

    return CLI_NO_ERROR;
}

/* ip dhcp inform*/
UI32_T CLI_API_Ip_Dhcp_Inform(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_INFORM == TRUE)
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI32_T addr_mode=0;
	switch(cmd_idx)
	{
		case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_DHCP_INFORM:
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
            if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ctrl_P->CMenu.vlan_ifindex, &addr_mode))
            {
                CLI_LIB_PrintStr("Failed to enable DHCP inform.\r\n");
                return CLI_NO_ERROR;
            }

            /* If address mode is not user configured or there's no ip address configured, 
             * notify user with prompted warning message
             */
            if(NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE == addr_mode)
            {
                if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
                {
                    CLI_LIB_PrintStr("Warning: IP address is not configured.\r\n");
                }
            }
            else
            {
                CLI_LIB_PrintStr("Warning: IP address mode is not user specified mode.\r\n");
            }
            
            
			if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetDhcpInform(ctrl_P->CMenu.vlan_ifindex,TRUE))
				CLI_LIB_PrintStr("Failed to enable DHCP inform.\r\n");
		    
		break;
		
		case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_DHCP_INFORM:
			if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetDhcpInform(ctrl_P->CMenu.vlan_ifindex,FALSE))
				CLI_LIB_PrintStr("Failed to disable DHCP inform.\r\n");
		    
		break;
		default:
			return CLI_ERR_INTERNAL;
		
	}
	
#endif
	return CLI_NO_ERROR;
}
#if 0
/*show command display vlan interface  dhcp client -identifier*/
UI32_T CLI_API_Show_Dhcp_ClientId(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_CLIENT == TRUE)
    UI32_T ifindex, i;
    char name[32];
	char buff[3]={0};
    char tmp_str[DHCP_TYPE_CID_BUF_MAX_SIZE*2+1]={0};
    DHCP_MGR_ClientId_T cid_p;
	
    CLI_LIB_PrintStr("Interface     mode      client-identifier  \r\n");
    CLI_LIB_PrintStr("---------     ----      -----------------  \r\n");
	
    while(DHCP_PMGR_C_GetNextClientId(&ifindex, &cid_p) == DHCP_MGR_OK)
    {   
        if(cid_p.id_mode != DHCP_MGR_CID_TEXT && cid_p.id_mode != DHCP_MGR_CID_HEX)
			continue;
		
        sprintf(name,"vlan%ld",ifindex-SYS_ADPT_VLAN_1_IF_INDEX_NUMBER+1);
        printf("%-14s",name); 
        
        if(DHCP_MGR_CID_TEXT == cid_p.id_mode)
        {
        	printf("%-10s", "TEXT");
            printf("%s\r\n", cid_p.id_buf);
        }
        else 
        {
            printf("%-10s", "HEX");
            tmp_str[0] = '\0';
	        for (i=0; i<cid_p.id_len;i++)
            {
                 sprintf(buff, "%02x", cid_p.id_buf[i]);
                 strcat(tmp_str,buff);
            }
            printf("%-10s\r\n",tmp_str);    
        }
    }
#endif
    return CLI_NO_ERROR;
}
#endif
