/* MODULE NAME: cli_api_show.c
 *
 * PURPOSE: For CLI to show those information, which does not belong to
 * any existing CSC-based "cli_api_....c" files.
 *
 * NOTES:
 *
 * HISTORY (mm/dd/yyyy)
 *    07/27/2011 - Qiyao Zhong, Added this file-head comment
 *
 * Copyright(C)      Accton Corporation, 2011
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

/* system
 */
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"

/* common library
 */
#include "l_stdlib.h"
#include "l_mm.h"

/* other CSCs
 */
#include "http_type.h"
#include "http_pmgr.h"

#if(SYS_CPNT_SSH2 == TRUE)
#include "keygen_type.h"
#include "sshd_pmgr.h"
#endif

#include "lacp_pom.h"

#if (SYS_CPNT_TELNET == TRUE)
#include "telnet_pmgr.h"
#endif

#if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
#include "sys_pmgr.h"
#endif

#include "stktplg_pmgr.h"
#include "stktplg_board_util.h"
#include "nmtr_pmgr.h"
#include "sys_time.h"
#include "mib2_pom.h"
#include "swctrl.h"
#include "sys_module.h"
#include "buffer_mgr.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "uc_mgr.h"
#include "psec_task.h"
#include "psec_pmgr.h"
#include "swdrv.h"
#include "swdrv_lib.h"
#if (SYS_CPNT_CRAFT_PORT == TRUE) && (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
#include "swdrv_om.h"
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE) || (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
#include "sysdrv.h"
#endif

/* CLI itself
 */
#include "cli_def.h"
#include "cli_type.h"
#include "cli_runcfg.h"
#include "cli_tbl.h"

#include "cli_api.h"
#include "cli_api_ethernet.h"
#include "cli_api_file.h"
#include "cli_api_l2_ip.h"
#include "cli_api_l3.h"
#include "cli_api_netaccess.h"
#include "cli_api_show.h"
#include "cli_api_sntp.h"
#include "cli_api_sta.h"
#include "cli_api_syslog.h"
#include "cli_api_vlan.h"

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE ) || (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
#include "stktplg_board.h"
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void UI64_T_2_STR(UI64_T value, UI8_T *str, UI32_T sizeofstr);

BOOL_T is_show_interface_counters_brief = FALSE;
static UI32_T Show_interface_port_info(UI32_T lport,  UI32_T verify_unit, UI32_T verify_port, UI32_T line_num);
static UI32_T Show_interface_vlan_info(UI32_T lport, UI32_T line_num);
static UI32_T Show_interface_stp(UI32_T lport, UI32_T line_num);
static UI32_T Show_interface_mirror(UI32_T lport, UI32_T line_num);
static UI32_T Show_interface_trunk(UI32_T lport, UI32_T line_num, UI32_T verify_unit, UI32_T verify_port);
static void Show_interface_trunk_port_sequence(UI32_T *ports_of_trunk, UI8_T *port_list, UI8_T *status, UI32_T port_count, UI32_T verify_port);
static UI32_T Show_interface_traffic(UI32_T lport, UI32_T line_num);
static UI32_T Show_interface_portname(UI32_T lport, UI32_T line_num);
static UI32_T print_config_buffer(char* buffer_p, UI32_T line_num);
static UI32_T show_command_history_by_mode(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T mode,UI32_T line_num);

#if(SYS_CPNT_SYSTEMWIDE_COUNTER == FALSE)
static void GetDeltaIfCounter(CLI_API_ShowIfCounter_T *RefIfTable, CLI_API_ShowIfCounter_T *CurrentIfTable);
static void SUB_UI64_T(UI64_T *RefIfTable, UI64_T *CurrentIfTable);
#endif

/* LOCAL VARIABLES DECLARATIONS
 */
static struct {
    UI32_T val;
    char *brief_desc;
    char *desc;
} cli_api_show_shutdown_reasons[] = {
    { SWCTRL_PORT_STATUS_SET_BY_CFG,                        " Disabled ", "Port Admin" },
    { SWCTRL_PORT_STATUS_SET_BY_XSTP_LBD,                   " STP LBD  ", "Spanning Tree Loopback Detection" },
    { SWCTRL_PORT_STATUS_SET_BY_XSTP_BPDUGUARD,             " BpduGuard", "Spanning Tree BPDU Guard" },
    { SWCTRL_PORT_STATUS_SET_BY_NETACCESS_LINK_DETECTION,   " LinkDet  ", "Link Detection" },
    { SWCTRL_PORT_STATUS_SET_BY_NETACCESS_DYNAMIC_QOS,      " DynQoS   ", "Dynamic QoS" },
    { SWCTRL_PORT_STATUS_SET_BY_PORTSEC,                    " PortSec  ", "Port Security" },
    { SWCTRL_PORT_STATUS_SET_BY_LBD,                        " LBD      ", "Loopback Detection" },
    { SWCTRL_PORT_STATUS_SET_BY_ATC_BSTORM,                 " ATC Bcast", "Auto Traffic Control - Broadcast" },
    { SWCTRL_PORT_STATUS_SET_BY_ATC_MSTORM,                 " ATC Mcast", "Auto Traffic Control - Multicast" },
    { SWCTRL_PORT_STATUS_SET_BY_UDLD,                       " UDLD     ", "UniDirectional Link Detection" },
    { SWCTRL_PORT_STATUS_SET_BY_SW_LICENSE,                 " License  ", "Invalid License or Trial License" },
};

/* FUNCTION DEFINITIONS
 */
UI32_T CLI_API_Show_Interface_Runningconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char*  buffer_p = NULL;
    UI32_T result = 0;
    UI32_T line_num = 0; /* For PROCESS_MORE */
    char   buff[CLI_DEF_MAX_BUFSIZE]; /* For PROCESS_MORE */

    if ((buffer_p = (char*) CLI_MGR_AllocateBuffer()) == NULL)
    {
        CLI_LIB_PrintStr("Configuration buffer is being used by another user\r\n");
        return CLI_NO_ERROR;
    }

    result = CLI_RUNCFG_Get_Interface_RunningCfg((char*)buffer_p, SYS_ADPT_CLI_MAX_CONFIG_SIZE, arg);
    line_num = print_config_buffer(buffer_p, line_num);
    if (line_num == JUMP_OUT_MORE)
    {
        CLI_MGR_FreeBuffer(buffer_p);
        return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
        CLI_MGR_FreeBuffer(buffer_p);
        return CLI_EXIT_SESSION;
    }

    if (result == CLI_RUNCFG_RETURN_NO_ENOUGH_MEMORY)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        PROCESS_MORE("\r\n");
        PROCESS_MORE("No enough space to collect all information\r\n");
#endif
    }

#if (SYS_CPNT_CLI_RUNCFG_DYNAMIC_BUFFER == TRUE)
    free(buffer_p);
#else
    CLI_MGR_FreeBuffer(buffer_p);
#endif /* SYS_CPNT_CLI_RUNCFG_DYNAMIC_BUFFER == TRUE */

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Show_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    #define EXEC_COMMANDS    1
    #define CONFIG_COMMANDS  2

    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num=0;

    /*I. EXEC commands*/
    PROCESS_MORE("Execution Command History:\r\n");

    if( (line_num=show_command_history_by_mode(ctrl_P, EXEC_COMMANDS,line_num)) == JUMP_OUT_MORE )
    {
       return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
       return CLI_EXIT_SESSION;
    }
    PROCESS_MORE("\r\n");

#if 1
    /*II. CONFIG commands*/
    if(ctrl_P->CMenu.AccMode == PRIVILEGE_EXEC_MODE)
    {
        PROCESS_MORE("Configuration Command History:\r\n");

        if( (line_num=show_command_history_by_mode(ctrl_P, CONFIG_COMMANDS,line_num)) == JUMP_OUT_MORE )
        {
           return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
           return CLI_EXIT_SESSION;
        }
        PROCESS_MORE("\r\n");
    }
#endif

    return CLI_NO_ERROR;
}


static UI32_T show_command_history_by_mode(CLI_TASK_WorkingArea_T *ctrl_P, UI32_T mode,UI32_T line_num)
{
    UI16_T head;
    UI16_T tail;
    int    i;
    UI8_T  PreAccMode = ctrl_P->CMenu.AccMode;
    char   buff[CLI_DEF_MAX_BUFSIZE]={0};

    if(mode == EXEC_COMMANDS)
       ctrl_P->CMenu.AccMode = PRIVILEGE_EXEC_MODE;
    else
       ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_GLOBAL_MODE;

    if( CLI_LIB_IsHistBufEmpty(ctrl_P)) /* buffer empty */
    {
        ctrl_P->CMenu.AccMode = PreAccMode;
        return line_num;
    }

    head = CLI_LIB_GetHistHeadIdx(ctrl_P);
    tail = CLI_LIB_GetHistTailIdx(ctrl_P);

    if(head < tail)
    {
        for(i=head ; i<=tail-1; i++)
        {
            sprintf(buff," %u ", tail-i);
            strcat(buff,CLI_LIB_GetHistBuf(i, ctrl_P));
            PROCESS_MORE_FUNC(buff);
            CLI_LIB_PrintNullStr(1);
        }
    }
    else /* head > tail */
    {
        int j = CLI_DEF_MAX_HIST;
#if (SYS_CPNT_CLI_TERMINAL==TRUE)
        for( i=head; i < ctrl_P->CMenu.histsize; i++)
#else
        for( i=head; i < CLI_DEF_MAX_HIST; i++)
#endif
        {
            sprintf(buff," %u ", j);
            strcat(buff,CLI_LIB_GetHistBuf(j, ctrl_P));
            PROCESS_MORE_FUNC(buff);
            CLI_LIB_PrintNullStr(1);
            j --;
        }

        for( i=0; i <= tail; i++)
        {
            sprintf(buff," %u ", j);
            strcat(buff,CLI_LIB_GetHistBuf(i, ctrl_P));
            PROCESS_MORE_FUNC(buff);
            CLI_LIB_PrintNullStr(1);
            j --;
        }
    }

    ctrl_P->CMenu.AccMode = PreAccMode;
    return line_num;
}

/*********************************<<SYSTEM MANAGEMENT>>**************************************/
/*change mode*/

/*execution*/
UI32_T CLI_API_Show_Startupconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    char   *buffer;
    UI32_T buffer_length;
    UI32_T line_num = 0;

    if ((buffer = BUFFER_MGR_Allocate()) == NULL)
    {
        return CLII_ERR_MEMORY_NOT_ENOUGH;
    }

    if (XFER_PMGR_ReadSystemConfig((UI8_T *)buffer, &buffer_length) != FS_RETURN_OK)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get startup configuration file\r\n");
#endif
    }
    else
    {
        print_config_buffer(buffer, line_num);
    }

    BUFFER_MGR_Free((void *)buffer);

    return CLI_NO_ERROR;
}

/* FUNCTION : CLI_API_Show_Runningconfig
 * PURPOSE  : for session to display running config
 * INPUT    : NONE
 * OUTPUT   : NONE
 * RETURN   : success - CLI_NO_ERROR
 *            fail - CLI_RUNCFG_RETURN_NO_ENOUGH_MEMORY, CLI_EXIT_SESSION
 * NOTE     : refine for 4k vlan issue, in this config will exceed 2xxk bytes
 *            size, thus we use cli buffer (size = CLI_DEF_MAX_CONFIG_SIZE ,
 *            appro.1M, this is porting from Mercury platform made by Davy
 */
UI32_T CLI_API_Show_Runningconfig(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char* buffer_p = NULL;
    UI32_T result = 0;
    UI32_T line_num = 0; /* For PROCESS_MORE */
    char   buff[CLI_DEF_MAX_BUFSIZE]; /* For PROCESS_MORE */

    if((buffer_p =  CLI_MGR_AllocateBuffer()) == NULL)
    {
        CLI_LIB_PrintStr("Configuration buffer is being used by another user\r\n");
        return CLI_NO_ERROR;
    }

    PROCESS_MORE("Building running configuration. Please wait...\r\n");

    result = CLI_RUNCFG_Get_RunningCfg(buffer_p, SYS_ADPT_CLI_MAX_CONFIG_SIZE);
    line_num = print_config_buffer(buffer_p, line_num);
    if(line_num == JUMP_OUT_MORE)
    {
        CLI_MGR_FreeBuffer(buffer_p);
        return CLI_NO_ERROR;
    }
    else if(line_num == EXIT_SESSION_MORE)
    {
        CLI_MGR_FreeBuffer(buffer_p);
        return CLI_EXIT_SESSION;
    }

    if(result == CLI_RUNCFG_RETURN_NO_ENOUGH_MEMORY)
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        PROCESS_MORE("\r\n");
        PROCESS_MORE("No enough space to collect all information\r\n");
#endif
    }

    CLI_MGR_FreeBuffer(buffer_p);
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_System(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T Contact[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1]  = {0};
    UI8_T Name[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1]     = {0};
    UI8_T Location[SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN + 1] = {0};
    UI32_T days        = 0;
    UI32_T hours       = 0;
    UI32_T minutes     = 0;
    UI32_T seconds     = 0;
    UI32_T miliseconds = 0;
    UI32_T unit        = 1;
    STK_UNIT_CFG_T device_info;
    UI8_T sys_desc[MAXSIZE_sysDescr+1] = {0};
    UI32_T oid_array[64];
    UI32_T oid_array_len = 0;
    char oid_string[100] = {0};
    UI32_T line_num = 0; /* For PROCESS_MORE */
    char   buff[CLI_DEF_MAX_BUFSIZE]; /* For PROCESS_MORE */
    UI32_T buff_cursor;

#if ((SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS == TRUE) || \
     (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE) && (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == TRUE) && (SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS != TRUE))
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;
#endif

#if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
    SYS_MGR_SwitchThermalEntry_T entry;
    UI8_T nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;
#endif

   memset(sys_desc, 0, sizeof(sys_desc));
   MIB2_POM_GetSysDescr(sys_desc);
   PROCESS_MORE_F("System Description : %s\r\n", sys_desc);

   memset(oid_array, 0, sizeof(oid_array));
   memset(oid_string, 0, sizeof(oid_string));
   if (MIB2_PMGR_GetSysObjectID(oid_array, &oid_array_len) == TRUE)
   {
       UI32_T  i = 0;
       char   buf[15];
       /* convert oid to string */
       for (i=0; i<oid_array_len; i++)
       {
           memset(buf, 0, sizeof(buf));
           CLI_LIB_UltoA(oid_array[i], buf);
           strcat(oid_string,buf);

           if (i != oid_array_len-1)
           {
               strcat(oid_string,(char *)".");
           }
       }
   }
   PROCESS_MORE_F("System OID String  : %s\r\n", oid_string);

   memset(&device_info,0,sizeof(device_info));

   MIB2_POM_GetSysContact(Contact);
   MIB2_POM_GetSysName(Name);
   MIB2_POM_GetSysLocation(Location);
   if (!STKTPLG_POM_GetDeviceInfo(unit,&device_info))/*now only 1 unit*/
   {
#if (SYS_CPNT_EH == TRUE)
       CLI_API_Show_Exception_Handeler_Msg();
#else
       PROCESS_MORE("Failed to get device info\r\n");
#endif
   }

   SYS_TIME_GetSystemUpTime(&days,&hours,&minutes,&seconds,&miliseconds);

   PROCESS_MORE("System Information\r\n");
   /*system up time*/
   sprintf(buff, " System Up Time         : %lu days, %lu hours, %lu minutes, and %lu.%lu seconds\r\n",
       (unsigned long)days, (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds, (unsigned long)miliseconds);
   PROCESS_MORE(buff);

   /*name*/
   buff_cursor = sprintf(buff, " System Name            : ");
   if(strlen((char *)Name) != 0)
   {
       sprintf(buff+buff_cursor, "%s\r\n", Name);
   }
   else
   {
       sprintf(buff+buff_cursor, "\r\n");
   }
   PROCESS_MORE(buff);

   /*location*/
   buff_cursor = sprintf(buff, " System Location        : ");
   if(strlen((char *)Location) != 0)
   {
       sprintf(buff+buff_cursor, "%s\r\n", Location);
   }
   else
   {
       sprintf(buff+buff_cursor, "\r\n");
   }
   PROCESS_MORE(buff);

   /*contact*/
   buff_cursor = sprintf(buff, " System Contact         : ");
   if(strlen((char *)Contact) != 0)
   {
       sprintf(buff+buff_cursor, "%s\r\n", Contact);
   }
   else
   {
       sprintf(buff+buff_cursor, "\r\n");
   }
   PROCESS_MORE(buff);

   /*MAC , IP addr , gateway ,http */
   {
       UI32_T now_unit = 0;

       while (STKTPLG_POM_GetNextUnit(&now_unit))
       {
           if (STKTPLG_POM_GetDeviceInfo(now_unit,&device_info))/*now only 1 unit*/
           {
               buff_cursor = sprintf(buff, " MAC Address (Unit %lu)   : %02X-%02X-%02X-",(unsigned long)now_unit , device_info.board_info.mac_addr[0],device_info.board_info.mac_addr[1], device_info.board_info.mac_addr[2]);
               sprintf(buff+buff_cursor, "%02X-%02X-%02X\r\n", device_info.board_info.mac_addr[3],device_info.board_info.mac_addr[4], device_info.board_info.mac_addr[5]);
               PROCESS_MORE(buff);
           }
       }
    }

    {
#if (SYS_CPNT_HTTP_UI == TRUE)
        PROCESS_MORE_F(" Web Server             : %s\r\n",(HTTP_PMGR_Get_Http_Status() == HTTP_STATE_ENABLED)?("Enabled"):("Disabled"));
        PROCESS_MORE_F(" Web Server Port        : %lu\r\n",(unsigned long)HTTP_PMGR_Get_Http_Port());

#if (SYS_CPNT_HTTPS == TRUE)
        {
            UI32_T secure_port = 0;

            HTTP_PMGR_Get_Secure_Port(&secure_port);
            PROCESS_MORE_F(" Web Secure Server      : %s\r\n",(HTTP_PMGR_Get_Secure_Http_Status() == SECURE_HTTP_STATE_ENABLED)?("Enabled"):("Disabled"));
            PROCESS_MORE_F(" Web Secure Server Port : %lu\r\n",(unsigned long)secure_port);
        }
#endif  /* #if (SYS_CPNT_HTTPS == TRUE) */
#endif  /* #if (SYS_CPNT_HTTP_UI == TRUE) */

#if (CLI_SUPPORT_L2_DHCP_RELAY == 1)
        {
            UI32_T vid;
            BRG_MGR_DHCP_SETTING_FLAG_T status;
            BRG_PMGR_GetNextBridgeDHCPRelayVlanId(&vid,&status);
            {
                PROCESS_MORE_F(" DHCP Relay             : %s on VLAN %lu\r\n",(status == BRG_MGR_DHCP_ENABLED)?("Enabled"):("Disabled"),(unsigned long)vid);
            }
        }
#endif
    }

#if (SYS_CPNT_TELNET == TRUE)
    {
        TELNET_State_T  telnet_status;
        UI32_T telnet_port;

        TELNET_PMGR_GetTnpdStatus(&telnet_status);
        PROCESS_MORE_F(" Telnet Server          : %s\r\n",(telnet_status == TELNET_STATE_ENABLED)?("Enabled"):("Disabled"));
        TELNET_PMGR_GetTnpdPort(&telnet_port);
        PROCESS_MORE_F(" Telnet Server Port     : %lu\r\n",(unsigned long)telnet_port);
    }
#endif  /* #if (SYS_CPNT_TELNET == TRUE) */

#if (SYS_CPNT_TACACS == TRUE )
    /*authentication login*/
    {
        USERAUTH_Auth_Method_T auth_method[5]  = {0};
        UI8_T temp_string[25] = {0};
        UI8_T i               = 0;
        if (USERAUTH_PMGR_GetRunningAuthMethod(auth_method) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
        {
            for (i = 0; i < 3; i++)
            {
                switch(auth_method[i])
                {
                    case USERAUTH_AUTH_LOCAL:
                        strcat((char *)temp_string," Local");
                        break;

                    case USERAUTH_AUTH_RADIUS:
                        strcat((char *)temp_string," RADIUS");
                        break;

                    case USERAUTH_AUTH_TACACS:
                        strcat((char *)temp_string," TACACS");
                        break;

                    default:
                        strcat((char *)temp_string," None");
                        break;
                }
            }
            PROCESS_MORE_F(" Authentication Login   : %s\r\n", temp_string);
        }/*End of if (USERAUTH_PMGR_GetRunningAuthMethod(...)*/
    }
    /*authentication enable*/
    {
        USERAUTH_Auth_Method_T auth_method[5]  = {0};
        UI8_T temp_string[25] = {0};
        UI8_T i               = 0;
        if (USERAUTH_PMGR_GetRunningEnableAuthMethod(auth_method) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
        {
            for (i = 0; i < 3; i++)
            {
                switch(auth_method[i])
                {
                    case USERAUTH_AUTH_LOCAL:
                        strcat((char *)temp_string," Local");
                        break;

                    case USERAUTH_AUTH_RADIUS:
                        strcat((char *)temp_string," RADIUS");
                        break;

                    case USERAUTH_AUTH_TACACS:
                        strcat((char *)temp_string," TACACS");
                        break;

                    default:
                        strcat((char *)temp_string," None");
                        break;
                }
            }
            PROCESS_MORE_F(" Authentication Enabled : %s\r\n", temp_string);
        }
    }
#endif/*End of #if (SYS_CPNT_TACACS == TRUE )*/

#if (SYS_CPNT_JUMBO_FRAMES == TRUE)
    /*jumbo frame*/
    {
        UI32_T jumbo_frame_status = 0;
        SWCTRL_POM_GetJumboFrameStatus (&jumbo_frame_status);
        if(jumbo_frame_status==SWCTRL_JUMBO_FRAME_ENABLE)
        {
            PROCESS_MORE(" Jumbo Frame            : Enabled \r\n");
        }
        else
        {
            PROCESS_MORE(" Jumbo Frame            : Disabled \r\n");
        }
    }
#endif

#if (SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS == TRUE)
    UI8_T nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;

    nbr_of_fan = STKTPLG_BOARD_GetFanNumber();
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    if(nbr_of_fan != 0)
    {
        PROCESS_MORE("\r\n");
        PROCESS_MORE("System Fan:\r\n");

        BOOL_T mode = FALSE;

        if (SYS_PMGR_GetFanSpeedForceFull(&mode) == TRUE)
        {
            PROCESS_MORE_F(" Force Fan Speed Full   : %s\r\n", (mode==TRUE)?"Enabled":"Disabled");
        }
        else
        {
            PROCESS_MORE(" Force Fan Speed Full   : getting Fail!\r\n");
        }
    }
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    /*Fan status*/
    {
        char  fan_buff[5] = {0};
        SYS_MGR_SwitchFanEntry_T    entry;

        for (entry.switch_unit_index=0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )
        {
            STKTPLG_OM_GetUnitBoardID(entry.switch_unit_index, &board_id);
            STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
            nbr_of_fan = board_info.fan_number;
            if(nbr_of_fan == 0)
                continue;
            PROCESS_MORE_F("Unit %lu\r\n",(unsigned long)entry.switch_unit_index);
            buff_cursor=0;
            for (entry.switch_fan_index=1;entry.switch_fan_index<=nbr_of_fan;entry.switch_fan_index++)
            {
                memset(fan_buff,0,sizeof(fan_buff));
                if(SYS_PMGR_GetFanStatus(&entry))
                {
                    if (entry.switch_fan_status == VAL_switchFanStatus_failure)
                    {
                        strcpy(fan_buff,"Fail");
                    }

                    if (entry.switch_fan_status == VAL_switchFanStatus_ok)
                    {
                        strcpy(fan_buff,"Ok");
                    }
                }
                if(entry.switch_fan_index %3 == 1)
                {
                    buff_cursor += sprintf(buff+buff_cursor, " Fan %lu: %-4s                 ", (unsigned long)entry.switch_fan_index, fan_buff);
                }
                else if(entry.switch_fan_index %3 == 2)
                {
                    buff_cursor += sprintf(buff+buff_cursor, " Fan %lu: %-4s                 ", (unsigned long)entry.switch_fan_index, fan_buff);
                }
                else
                {
                    sprintf(buff+buff_cursor, " Fan %lu: %-4s\r\n", (unsigned long)entry.switch_fan_index, fan_buff);
                    PROCESS_MORE(buff);
                    buff_cursor=0;
                }
            }
            if (buff_cursor)
            {
                buff[buff_cursor]='\r';
                buff[buff_cursor+1]='\n';
                PROCESS_MORE(buff);
            }
            PROCESS_MORE("\r\n");
        }
    }
#endif
#endif


/* 2009-03-30 [ES4626H-FLF-17-00259] Eugene added for show temperature
 */
#if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)
#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == FALSE)
    nbr_of_thermal= SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;
#else
    nbr_of_thermal= STKTPLG_BOARD_GetThermalNumber();
#endif
    if(nbr_of_thermal != 0)
    {
        PROCESS_MORE("\r\n")
        PROCESS_MORE("System Temperature:\r\n");
    }
    for (entry.switch_unit_index=0; STKTPLG_OM_GetNextUnit(&entry.switch_unit_index); )
    {
#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == TRUE)
        STKTPLG_OM_GetUnitBoardID(entry.switch_unit_index, &board_id);
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
        nbr_of_thermal= board_info.thermal_number;
        if(nbr_of_thermal== 0)
            continue;
#endif

        PROCESS_MORE_F("Unit %lu\r\n",(unsigned long)entry.switch_unit_index);
        buff_cursor=0;
        for (entry.switch_thermal_index=1;entry.switch_thermal_index<=nbr_of_thermal;entry.switch_thermal_index++)
        {
            if (SYS_PMGR_GetThermalStatus(&entry) == TRUE)
            {
                buff_cursor+=sprintf(buff+buff_cursor, " Temperature %lu: %3ld degrees    ",(unsigned long)entry.switch_thermal_index,(long)entry.switch_thermal_temp_value);
            }
            else
            {
                buff_cursor+=sprintf(buff+buff_cursor, " Temperature %lu: getting Fail!   ",(unsigned long)entry.switch_thermal_index);
            }
            if ((entry.switch_thermal_index % 2) ==0)
            {
                sprintf(buff+buff_cursor, "\r\n");
                PROCESS_MORE(buff);
                buff_cursor=0;
            }
        }
        if (buff_cursor)
        {
            buff[buff_cursor]='\r';
            buff[buff_cursor+1]='\n';
            PROCESS_MORE(buff);
        }
        PROCESS_MORE("\r\n");
    }
#endif/*End of #if (SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE == TRUE)*/

    /* Module/Xenpak Information */
    {
        UI32_T      m_unit_id = 0;
        UI32_T      module_unit_id = 0;
        STKTPLG_MGR_switchModuleInfoEntry_T module_info;
        STKTPLG_MGR_switchModuleInfoEntry_T entry;

        while(STKTPLG_POM_GetNextUnit(&m_unit_id))
        {
            if(STKTPLG_POM_OptionModuleIsExist(m_unit_id, &module_unit_id)==TRUE)
            {
                memset(&entry, 0, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
                entry.unit_index = m_unit_id;
                entry.module_index = 1;

                STKTPLG_POM_GetModuleInfo(m_unit_id, 1, &module_info);
                switch(module_info.xenpak_status)
                {
                    case VAL_swModuleXenpakType_notPresent:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                "XENPAK Not Present");
                        break;
                    case VAL_swModuleXenpakType_unknown:
                    case VAL_swModuleXenpakType_unsupportedLR:
                    case VAL_swModuleXenpakType_unsupportedER:
                    case VAL_swModuleXenpakType_unsupportedCX4:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                "Unsupported XENPAK");
                        break;
                    case VAL_swModuleXenpakType_supportedLR:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                SYS_DFLT_XENPAK_PARTNUMBER_LR);
                        break;
                    case VAL_swModuleXenpakType_supportedER:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                SYS_DFLT_XENPAK_PARTNUMBER_ER);
                        break;
                    case VAL_swModuleXenpakType_supportedCX4:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                SYS_DFLT_XENPAK_PARTNUMBER_CX4);
                        break;
                    default:
                        PROCESS_MORE_F(" Module (Unit %lu)        : %s 10-Gigabit module (%s)\r\n",
                                (unsigned long)m_unit_id, SYS_DFLT_MODULE_PRODUCT_NUMBER,
                                "XENPAK Not Present");
                        break;
                }

                if(STKTPLG_PMGR_GetSwitchModuleInfoEntry(&entry)==TRUE)
                {
                    PROCESS_MORE_F(" Module Type (Unit %lu)   : %d\r\n",(unsigned long)m_unit_id, entry.module_type);
                }
            }
        }
    }

    {
        #define UC_POST_RESULT_SIZE (SYS_ADPT_POST_DESC_MAX_LENGTH*SYS_ADPT_POST_ITEM_DESC_MAX_LENGTH) /* uc post result size used in POST_Main() in post.c */
        char *ptr_in_uc;
        char *post_result_cursor_p;
        char *post_result_line_end_p;
        BOOL_T uc_post_result_reach_end=FALSE;

        ptr_in_uc = (char*)UC_MGR_Allocate (UC_MGR_POST_RESULT_INFO_INDEX, 2000, 1);

        if(ptr_in_uc == NULL)
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            PROCESS_MORE("Failed to get POST result.\r\n");
#endif
        }
        else if (ptr_in_uc[0] != '\0')
        {
            post_result_cursor_p = ptr_in_uc;
            while((post_result_cursor_p != NULL) && (uc_post_result_reach_end==FALSE))
            {
                /* locate '\n' for the end of the line in uc post result
                 */
                post_result_line_end_p = strchr(post_result_cursor_p, 0x0A);
                /* check buffer overlow
                 */
                if (post_result_line_end_p >= (ptr_in_uc + (UC_POST_RESULT_SIZE)))
                {
                    /* force line end pointing to the end address of the uc post result
                     */
                    post_result_line_end_p = (ptr_in_uc + (UC_POST_RESULT_SIZE) -1);
                    uc_post_result_reach_end=TRUE;
                }

                if(post_result_line_end_p!=NULL)
                {
                    UI8_T line_len;

                    /* found end of line
                     */

                    line_len = (UI8_T)(post_result_line_end_p - post_result_cursor_p + 1);
                    memcpy(buff, post_result_cursor_p, line_len);

                    if (*post_result_line_end_p == '\n')
                    {
                        buff[line_len] = '\0';
                    }
                    else
                    {
                        buff_cursor = line_len;
                        buff[buff_cursor]  ='\r';
                        buff[buff_cursor+1]='\n';
                        buff[buff_cursor+2]='\0';
                    }
                    PROCESS_MORE(buff);
                }
                else if (post_result_cursor_p != '\0')
                {
                    /* the remaining line might not terminated with "\r\n",
                     * copy the content of remaining line to buff and append
                     * new line in buff
                     */
                    UI16_T line_start_offset = post_result_cursor_p - ptr_in_uc;
                    UI16_T line_end_offset;


                    /* locate end of uc post result
                     */
                    for(line_end_offset=line_start_offset;
                        line_end_offset<UC_POST_RESULT_SIZE;
                        line_end_offset++)
                    {
                        if (ptr_in_uc[line_end_offset]=='\0')
                            break;
                    }
                    memcpy(buff, post_result_cursor_p, line_end_offset-line_start_offset+1);
                    buff_cursor = line_end_offset-line_start_offset+1;
                    buff[buff_cursor] = '\r';
                    buff[buff_cursor+1] = '\n';
                    buff[buff_cursor+2] = '\0';
                    PROCESS_MORE(buff);
                    break;
                }

                if (*(post_result_line_end_p+1)=='\0')
                {
                    break;
                }
                post_result_cursor_p = post_result_line_end_p + 1;
            }
        }
    }

#if defined(ASF4526B_FLF_P5) || (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE)
    {
        SYS_MGR_IndivPower_T indiv_power;
        UI8_T power_status[20] = {0};
        #ifdef ECS4810_12MV2
        BOOL_T main_power_working=FALSE;
        #endif
        memset(&indiv_power,0,sizeof(SYS_MGR_IndivPower_T));

        indiv_power.sw_indiv_power_unit_index = unit;
        indiv_power.sw_indiv_power_index = 1;
        if(SYS_PMGR_GetSwitchIndivPower(&indiv_power)==TRUE)
        {
            switch(indiv_power.sw_indiv_power_status)
            {
                case VAL_swIndivPowerStatus_green:
                #ifdef ECS4810_12MV2
                /* using * to represent whether
                 * power supply is working or not
                 */
                    strcpy((char *)power_status,"Up(*)");
                    main_power_working = TRUE;
                #elif defined(ASF4512MP)
                    strcpy((char *)power_status,"Active");
                #else
                    strcpy((char *)power_status,"Up");
                #endif
                    break;

                case VAL_swIndivPowerStatus_red:
                #if defined(ASF4512MP)
                    strcpy((char *)power_status,"Inactive");
                #else
                    strcpy((char *)power_status,"Down");
                #endif
                    break;

                case VAL_swIndivPowerStatus_notPresent:
                    strcpy((char *)power_status,"Not present");
                    break;

                default:
                    break;
            }
            PROCESS_MORE("\r\n");
            PROCESS_MORE_F(" Main Power Status      : %s\r\n",power_status);
        }

#if (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)

    if (STKTPLG_BOARD_UTIL_GetPowerNumber() >= 2)
    {
        memset(power_status,0,sizeof(power_status));

        indiv_power.sw_indiv_power_unit_index = unit;
        indiv_power.sw_indiv_power_index = 2;
        if(SYS_PMGR_GetSwitchIndivPower(&indiv_power)==TRUE)
        {
            switch(indiv_power.sw_indiv_power_status)
            {
                case VAL_swIndivPowerStatus_green:
                #ifdef ECS4810_12MV2
                    if(main_power_working == TRUE)
                        strcpy((char *)power_status,"Up");
                    else
                        strcpy((char *)power_status,"Up(*)");
                #elif defined(ASF4512MP)
                    strcpy((char *)power_status,"Active");
                #else
                    strcpy((char *)power_status,"Up");
                #endif
                    break;

                case VAL_swIndivPowerStatus_red:
                #if defined(ASF4512MP)
                    strcpy((char *)power_status,"Inactive");
                #else
                    strcpy((char *)power_status,"Down");
                #endif
                    break;

                case VAL_swIndivPowerStatus_notPresent:
                    strcpy((char *)power_status,"Not present");
                    break;

                default:
                    break;
            }
            PROCESS_MORE_F(" Redundant Power Status : %s\r\n",power_status);
        }
    }
#endif /* end of #if (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE) */

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE == TRUE)
    /* Only support display of power module types up to 2 power modules
     * Show warning if number of power is more than 2
     */
    #if(SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT>2)
    #warning "Only two power module types can be displayed!!!"
    #endif
        {
            UI32_T power_index;
            char*  power_type_format_str[2] = {
                " Main Power Type        : %s\r\n",
                " Redundant Power Type   : %s\r\n"};
            char   power_type_buf[68];

            for(power_index=1; power_index<=2; power_index++)
            {
                memset(power_status,0,sizeof(power_status));
                memset(&indiv_power,0,sizeof(SYS_MGR_IndivPower_T));
                indiv_power.sw_indiv_power_unit_index=unit;
                indiv_power.sw_indiv_power_index=power_index;

                if(SYS_PMGR_GetSwitchIndivPower(&indiv_power)==FALSE)

                {
                    snprintf(power_type_buf, sizeof(power_type_buf),
                        power_type_format_str[power_index-1], "[Error]");
                    power_type_buf[sizeof(power_type_buf)-1]='\0';
                    PROCESS_MORE(power_type_buf);
                    continue;
                }
                if (indiv_power.sw_indiv_power_status == VAL_swIndivPowerStatus_notPresent)
                {
                    snprintf(power_type_buf, sizeof(power_type_buf),
                        power_type_format_str[power_index-1], "[None]");
                    power_type_buf[sizeof(power_type_buf)-1]='\0';
                    PROCESS_MORE(power_type_buf);
                    continue;
                }
                else
                {
                    switch(indiv_power.sw_indiv_power_type)
                    {
                        case VAL_swIndivPowerType_DC_N48:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[DC/DC -48V to +12V Module]");
                            break;

                        case VAL_swIndivPowerType_DC_P24:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[DC/DC +27V to +12V Module]");
                            break;

                        case VAL_swIndivPowerType_AC:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[AC100~240V to +12V Module]");
                            break;

                        case VAL_swIndivPowerType_DC_N48_Wrong:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[DC/DC -48V to +12V Module][WRONG]");
                            break;

                        case VAL_swIndivPowerType_DC_P24_Wrong:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[DC/DC +27V to +12V Module][WRONG]");
                            break;

                        case VAL_swIndivPowerType_AC_Wrong:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[AC100~240V to +12V Module][WRONG]");
                            break;
                        default:
                            snprintf(power_type_buf, sizeof(power_type_buf),
                                power_type_format_str[power_index-1],
                                "[Unknown]");
                            break;
                    }
                    power_type_buf[sizeof(power_type_buf)-1]='\0';
                    PROCESS_MORE(power_type_buf);
                }
            } /* end of for(power_index=1; power_index<=2; power_index++) */

        }
#endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE == TRUE) */

    }
#endif /* end of #if defined(ASF4526B_FLF_P5) || (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Alarm_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ALARM_DETECT == TRUE)
    UI32_T                 unit_id;
    SYS_MGR_SwAlarmEntry_T alarm;

    for (unit_id=0; STKTPLG_OM_GetNextUnit(&unit_id); )
    {
        CLI_LIB_PrintStr_1("Unit %lu\r\n", (unsigned long)unit_id);
#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE && SYS_CPNT_CLI_SHOW_ALARM_INPUT == TRUE)
        {
            char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
            char   tmp_buff[CLI_DEF_MAX_BUFSIZE/2] = {0};
            BOOL_T alarm_input_asserted = FALSE;
            UI32_T alarm_input_status = 0;

            memset(buff,0,sizeof(buff));
            memset(&alarm, 0, sizeof(SYS_MGR_SwAlarmEntry_T));

            alarm.sw_alarm_unit_index=unit_id;
            if(SYS_PMGR_GetSwAlarmInputStatus(&alarm)==TRUE)
            {
                alarm_input_status = alarm.sw_alarm_status;
                if(SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK))
                {
                    alarm_input_asserted=TRUE;
                    alarm.sw_alarm_input_index = 1;
                    if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
                        sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
                    strcat(buff, tmp_buff);
                }

                if(SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK))
                {
                    alarm_input_asserted=TRUE;
                    alarm.sw_alarm_input_index = 2;
                    if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
                        sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
                    strcat(buff, tmp_buff);
                }

                if(SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK))
                {
                    alarm_input_asserted=TRUE;
                    alarm.sw_alarm_input_index = 3;
                    if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
                        sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
                    strcat(buff, tmp_buff);
                }

                if(SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK))
                {
                    alarm_input_asserted=TRUE;
                    alarm.sw_alarm_input_index = 4;
                    if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
                        sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
                    strcat(buff, tmp_buff);
                }

                if(alarm_input_asserted==FALSE)
                {
                    sprintf(tmp_buff, "[NONE]");
                    strcat(buff, tmp_buff);
                }
            }

            CLI_LIB_PrintStr_1(" Asserted Alarm Input   : %s\r\n",buff);
        }
#endif /* #if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE && SYS_CPNT_CLI_SHOW_ALARM_INPUT == TRUE) */

#if (SYS_CPNT_ALARM_DETECT == TRUE)
        {
            UI32_T current_major_status;
            UI32_T current_minor_status;

            alarm.sw_alarm_unit_index=unit_id;
            current_major_status=current_minor_status=0;
            if(SYS_PMGR_GetMajorAlarmOutputCurrentStatus(&alarm)==FALSE)
            {
                SYSFUN_Debug_Printf("%s(%d)Failed to get major alarm output status.\r\n",
                    __FUNCTION__, __LINE__);
            }
            else
            {
                current_major_status = alarm.sw_alarm_status;
            }
            if(SYS_PMGR_GetMinorAlarmOutputCurrentStatus(&alarm)==FALSE)
            {
                SYSFUN_Debug_Printf("%s(%d)Failed to get major alarm output status.\r\n",
                    __FUNCTION__, __LINE__);

            }
            else
            {
                current_minor_status = alarm.sw_alarm_status;
            }

            CLI_LIB_PrintStr(" Current Major Alarm Status:");
            if (current_major_status != 0)
            {
                CLI_LIB_PrintStr("\r\n");
                if(current_major_status & SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
                {
#if (SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER==TRUE)
                    CLI_LIB_PrintStr("    One of the power status is failed.\r\n");
#else
                    CLI_LIB_PrintStr("    The main power status is failed.\r\n");
#endif
                }

                if(current_major_status & SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK)
                {
                    CLI_LIB_PrintStr("    All fans are failed.\r\n");
                }

                if(current_major_status & SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK)
                {
                    CLI_LIB_PrintStr("    The power modules in the system are wrong.\r\n");
                }
            }
            else
            {
                CLI_LIB_PrintStr("[NONE]\r\n");
            }

            CLI_LIB_PrintStr(" Current Minor Alarm Status:");
            if (current_minor_status != 0)
            {
                CLI_LIB_PrintStr("\r\n");
                if(current_minor_status & SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
                {
                    CLI_LIB_PrintStr("    The thermal temperature in the system is overheated.\r\n");
                }
                if(current_minor_status & SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK)
                {
                    CLI_LIB_PrintStr("    More than one fan is failed.\r\n");
                }
            }
            else
            {
                CLI_LIB_PrintStr("[NONE]\r\n");
            }

            CLI_LIB_PrintStr(" Current Major Alarm Output Status:");
            if (current_major_status)
            {
                CLI_LIB_PrintStr("[ACTIVE]\r\n");
            }
            else
            {
                CLI_LIB_PrintStr("[INACTIVE]\r\n");
            }

            CLI_LIB_PrintStr(" Current Minor Alarm Output Status:");
            if (current_minor_status)
            {
                CLI_LIB_PrintStr("[ACTIVE]\r\n");
            }
            else
            {
                CLI_LIB_PrintStr("[INACTIVE]\r\n");
            }

        }
    } /* for (sw_alarm_input.sw_alarm_unit_index=0; STKTPLG_OM_GetNextUnit(&sw_alarm_input.sw_alarm_unit_index); ) */
#endif /* #if (SYS_CPNT_ALARM_DETECT == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Alarm_Input_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE && SYS_CPNT_CLI_SHOW_ALARM_INPUT == TRUE)
    UI32_T  unit, index;
    UI32_T verify_unit, verify_index;
    SYS_MGR_SwAlarmEntry_T alarm;
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    char   tmp_buff[CLI_DEF_MAX_BUFSIZE/2] = {0};

    if(arg[0] == NULL)
    {
        for (unit=0; STKTPLG_OM_GetNextUnit(&unit); )
        {
            for(index=1; index <= SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT; index++)
            {
                memset(buff, 0, sizeof(buff));
                memset(&alarm, 0, sizeof(SYS_MGR_SwAlarmEntry_T));

                alarm.sw_alarm_unit_index = unit;
                alarm.sw_alarm_input_index = index;
                if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
                {
                    sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
                    strcat(buff, tmp_buff);
                }
                CLI_LIB_PrintStr_2("Name of Alarm Input of unit %lu index %lu:\r\n", (unsigned long)unit, (unsigned long)index);
                CLI_LIB_PrintStr_1(" %s\r\n",buff);
            }
        }
    }
    else/* if(arg[0] == NULL) */
    {
        verify_unit = atoi((char *) arg[0]);
        verify_index = atoi((char *) arg[1]);

        if (!STKTPLG_POM_UnitExist(verify_unit))
        {
        #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
        #else
            CLI_LIB_PrintStr_1("Unit%lu does not exist\r\n", (unsigned long)verify_unit);
        #endif
            return CLI_NO_ERROR;
        }

        if(verify_index < 1 || verify_index > SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
        {
        #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
        #else
            CLI_LIB_PrintStr_1("Alarm input %lu does not exist\r\n", (unsigned long)verify_index);
        #endif
            return CLI_NO_ERROR;
        }

        memset(buff, 0, sizeof(buff));
        memset(&alarm, 0, sizeof(SYS_MGR_SwAlarmEntry_T));

        alarm.sw_alarm_unit_index = verify_unit;
        alarm.sw_alarm_input_index = verify_index;
        if(SYS_PMGR_GetSwAlarmInputName(&alarm)==TRUE)
        {
            sprintf(tmp_buff, "%s ", alarm.sw_alarm_input_name);
            strcat(buff, tmp_buff);
        }
        CLI_LIB_PrintStr_2("Name of Alarm Input of unit %lu index %lu:\r\n", (unsigned long)verify_unit, (unsigned long)verify_index);
        CLI_LIB_PrintStr_1(" %s\r\n",buff);
    }
#endif /* #if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE && SYS_CPNT_CLI_SHOW_ALARM_INPUT == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Show_Users(
    UI16_T cmd_idx,
    char *arg[],
    CLI_TASK_WorkingArea_T *ctrl_P)
{
    enum
    {
        TBL_ACCOUNT_USER_NAME = 0,
        TBL_ACCOUNT_PRIVILEGE,
#if(SYS_CPNT_SSH2 == TRUE)
        TBL_ACCOUNT_PUBLIC_KEY
#endif /* #if(SYS_CPNT_SSH2 == TRUE) */
    };

    enum
    {
        TBL_USER_LINE = 0,
        TBL_USER_SESSION_ID,
        TBL_USER_USER_NAME,
        TBL_USER_IDLE_TIME,
        TBL_USER_REMOTE_IP_ADDR
    };

    enum
    {
        TBL_WEB_USER_LINE = 0,
        TBL_WEB_USER_USER_NAME,
        TBL_WEB_USER_IDLE_TIME,
        TBL_WEB_USER_REMOTE_IP_ADDR
    };

    UI32_T my_seessin_id = (UI32_T) CLI_TASK_GetMySessId();
    UI32_T line_num = 0;
    UI32_T index;
#if(SYS_CPNT_SSH2 == TRUE)
    UI32_T key_type = KEY_TYPE_NONE;
#endif
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    int    rc;
    HTTP_Session_T            web_user_connection_info;
    CLI_TASK_OnlineUserInfo_T user_info;
    USERAUTH_LoginLocal_T     login_user;
    CLI_TBL_Object_T          account_tb;
    CLI_TBL_Object_T          user_tb;
    CLI_TBL_Object_T          web_user_tb;

    CLI_TBL_Temp_T account_group_tbl[] =
    {
        {TBL_ACCOUNT_USER_NAME,  20, CLI_TBL_ALIGN_LEFT},
        {TBL_ACCOUNT_PRIVILEGE,  10, CLI_TBL_ALIGN_RIGHT},
#if(SYS_CPNT_SSH2 == TRUE)
        {TBL_ACCOUNT_PUBLIC_KEY, 15, CLI_TBL_ALIGN_LEFT},
#endif /* #if(SYS_CPNT_SSH2 == TRUE) */
    };

    CLI_TBL_Temp_T group_tbl[] =
    {
        {TBL_USER_LINE,            9, CLI_TBL_ALIGN_LEFT},
        {TBL_USER_SESSION_ID,     10, CLI_TBL_ALIGN_RIGHT},
        {TBL_USER_USER_NAME,      20, CLI_TBL_ALIGN_LEFT},
        {TBL_USER_IDLE_TIME,      17, CLI_TBL_ALIGN_RIGHT},
        {TBL_USER_REMOTE_IP_ADDR, 18, CLI_TBL_ALIGN_LEFT},
    };

    CLI_TBL_Temp_T web_group_tbl[] =
    {
        {TBL_WEB_USER_LINE,           10, CLI_TBL_ALIGN_LEFT},
        {TBL_WEB_USER_USER_NAME,      20, CLI_TBL_ALIGN_LEFT},
        {TBL_WEB_USER_IDLE_TIME,      20, CLI_TBL_ALIGN_RIGHT},
        {TBL_WEB_USER_REMOTE_IP_ADDR, 20, CLI_TBL_ALIGN_LEFT},
    };

    /* User name account table
     */
    PROCESS_MORE("User Name Accounts:\r\n");

    CLI_TBL_InitWithBuf(&account_tb, buff, sizeof(buff));
    CLI_TBL_SetColIndirect(&account_tb,
                           account_group_tbl,
                           sizeof(account_group_tbl)/sizeof(account_group_tbl[0]));
    CLI_TBL_SetLineNum(&account_tb, line_num);

    CLI_TBL_SetColTitle(&account_tb, TBL_ACCOUNT_USER_NAME, "User Name");
    CLI_TBL_SetColTitle(&account_tb, TBL_ACCOUNT_PRIVILEGE, "Privilege");
#if(SYS_CPNT_SSH2 == TRUE)
    CLI_TBL_SetColTitle(&account_tb, TBL_ACCOUNT_PUBLIC_KEY, "Public-Key");
#endif /* #if(SYS_CPNT_SSH2 == TRUE) */
    CLI_TBL_Print(&account_tb);

    CLI_TBL_SetLine(&account_tb);
    CLI_TBL_Print(&account_tb);

    memset(&login_user, 0, sizeof(USERAUTH_LoginLocal_T));

    while( USERAUTH_PMGR_GetNextLoginLocalUser(&login_user))
    {
#if(SYS_CPNT_SSH2 == TRUE)
        if (FALSE == SSHD_PMGR_GetUserPublicKeyType(login_user.username,
            &key_type))
        {
            key_type = KEY_TYPE_NONE;
        }

        switch(key_type)
        {
            case  KEY_TYPE_RSA:
                CLI_TBL_SetColText(&account_tb, TBL_ACCOUNT_PUBLIC_KEY, "RSA");
                break;
            case  KEY_TYPE_DSA:
                CLI_TBL_SetColText(&account_tb, TBL_ACCOUNT_PUBLIC_KEY, "DSA");
                break;
            case  KEY_TYPE_BOTH_RSA_AND_DSA:
                CLI_TBL_SetColText(&account_tb, TBL_ACCOUNT_PUBLIC_KEY, "Both");
                break;
            default:
                CLI_TBL_SetColText(&account_tb, TBL_ACCOUNT_PUBLIC_KEY, "None");
                break;
        }
#endif
        CLI_TBL_SetColText(&account_tb, TBL_ACCOUNT_USER_NAME, (char *)login_user.username);
        CLI_TBL_SetColInt(&account_tb, TBL_ACCOUNT_PRIVILEGE, (unsigned long) login_user.privilege);

        rc = CLI_TBL_Print(&account_tb);

        if (CLI_TBL_PRINT_RC_SUCCESS != rc)
        {
            return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
        }
    }
    PROCESS_MORE_FUNC("\r\n");

    /* Onile user table
     */
    PROCESS_MORE("Online Users:\r\n");

    CLI_TBL_InitWithBuf(&user_tb, buff, sizeof(buff));
    CLI_TBL_SetColIndirect(&user_tb, group_tbl, sizeof(group_tbl)/sizeof(group_tbl[0]));
    CLI_TBL_SetLineNum(&user_tb, line_num);

    CLI_TBL_SetColTitle(&user_tb, TBL_USER_LINE, "Line");
    CLI_TBL_SetColTitle(&user_tb, TBL_USER_SESSION_ID, "Session ID");
    CLI_TBL_SetColTitle(&user_tb, TBL_USER_USER_NAME, "User Name");
    CLI_TBL_SetColTitle(&user_tb, TBL_USER_IDLE_TIME, "Idle Time (h:m:s)");
    CLI_TBL_SetColTitle(&user_tb, TBL_USER_REMOTE_IP_ADDR, "Remote IP Addr");
    CLI_TBL_Print(&user_tb);

    CLI_TBL_SetLine(&user_tb);
    CLI_TBL_Print(&user_tb);

    for(index = CLI_TASK_MIN_SESSION_ID;
        index <= CLI_TASK_MAX_SESSION_ID - 1;
        index++)
    {
        if(!CLI_TASK_GetOnLineUserInfoByIndex(index, &user_info))
        {
            continue;
        }
        else
        {
            enum
            {
                IDLE_TIME_STR_LEN = sizeof("00:00:00")-1,
            };

            char  line_string[CLI_TASK_MAX_LINE_STR_LEN + 2] = {0};
            char  idle_time_string[IDLE_TIME_STR_LEN + 1] = {0};

            sprintf(line_string, (index == my_seessin_id)?"*%s":" %s", user_info.line);

            {
                UI32_T hour;
                UI32_T min;
                UI32_T sec;

                sec  = (user_info.idel_time/100)%60;
                min  = ((user_info.idel_time/100)%3600)/60;
                hour = ((user_info.idel_time/100) - sec - min*60)/(60*60);

                sprintf(idle_time_string, "%ld:%02ld:%02ld", (long)hour, (long)min, (long)sec);
            }

            CLI_TBL_SetColText(&user_tb, TBL_USER_LINE, line_string);
            CLI_TBL_SetColInt(&user_tb, TBL_USER_SESSION_ID, index);
            CLI_TBL_SetColText(&user_tb, TBL_USER_USER_NAME, (char *)user_info.username);
            CLI_TBL_SetColText(&user_tb, TBL_USER_IDLE_TIME, idle_time_string);
            CLI_TBL_SetColText(&user_tb, TBL_USER_REMOTE_IP_ADDR, (char *)user_info.location);

            rc = CLI_TBL_Print(&user_tb);

            if (CLI_TBL_PRINT_RC_SUCCESS != rc)
            {
                return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
            }
        }
    }
    PROCESS_MORE("\r\n");

    /* Web online user table
     */
    PROCESS_MORE("Web Online Users:\r\n");

    CLI_TBL_InitWithBuf(&web_user_tb, buff, sizeof(buff));
    CLI_TBL_SetColIndirect(&web_user_tb, web_group_tbl, sizeof(web_group_tbl)/sizeof(web_group_tbl[0]));
    CLI_TBL_SetLineNum(&web_user_tb, line_num);

    CLI_TBL_SetColTitle(&web_user_tb, TBL_WEB_USER_LINE, "Line");
    CLI_TBL_SetColTitle(&web_user_tb, TBL_WEB_USER_USER_NAME, "User Name");
    CLI_TBL_SetColTitle(&web_user_tb, TBL_WEB_USER_IDLE_TIME, "Idle Time (h:m:s)");
    CLI_TBL_SetColTitle(&web_user_tb, TBL_WEB_USER_REMOTE_IP_ADDR, "Remote IP Addr");
    CLI_TBL_Print(&web_user_tb);

    CLI_TBL_SetLine(&web_user_tb);
    CLI_TBL_Print(&web_user_tb);

    memset(&web_user_connection_info, 0, sizeof(HTTP_Session_T));
    index = 0;
#if (SYS_CPNT_HTTP_UI == TRUE)
    while(HTTP_PMGR_Get_Next_User_Connection_Info(&web_user_connection_info))
    {
        enum
        {
            IDLE_TIME_STR_LEN = sizeof("00:00:00") - 1,
        };

        char  idle_time_string[IDLE_TIME_STR_LEN + 1] = {0};
        char  ipadd_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};

        index += 1;

        if( web_user_connection_info.protocol == HTTP_CONNECTION)
        {
            CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_LINE, "HTTP");
        }
#if(SYS_CPNT_HTTPS == TRUE)
        else if( web_user_connection_info.protocol == HTTPS_CONNECTION )
        {
            CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_LINE, "HTTPS");
        }
#endif /* #if(SYS_CPNT_HTTPS == TRUE) */
        else
        {
            CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_LINE, "Web");
        }

        {
            UI32_T idle_time;
            UI32_T hour;
            UI32_T min;
            UI32_T sec;

            idle_time = SYSFUN_GetSysTick();
            idle_time = idle_time - web_user_connection_info.last_access_time;
            sec  = (idle_time/100)%60;
            min  = ((idle_time/100)%3600)/60;
            hour = ((idle_time/100) - sec - min*60)/(60*60);

           sprintf(idle_time_string, "%ld:%02ld:%02ld", (long)hour, (long)min, (long)sec);
        }

        L_INET_InaddrToString((L_INET_Addr_T *)&(web_user_connection_info.remote_ip),
                              ipadd_str,
                              sizeof(ipadd_str));

        CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_USER_NAME, (char *)web_user_connection_info.username);
        CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_IDLE_TIME, idle_time_string);
        CLI_TBL_SetColText(&web_user_tb, TBL_WEB_USER_REMOTE_IP_ADDR, ipadd_str);

        rc = CLI_TBL_Print(&web_user_tb);

        if (CLI_TBL_PRINT_RC_SUCCESS != rc)
        {
            return CLI_TBL_PRINT_FAIL_RC_TO_CLI(rc);
        }
    }
#endif /* #if (SYS_CPNT_HTTP_UI == TRUE) */
    PROCESS_MORE("\r\n");

    return CLI_NO_ERROR;
}


   static UI32_T Show_one_version(UI32_T unit, UI32_T line_num);

UI32_T CLI_API_Show_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T unit = 0;
#if (SYS_CPNT_STACKING == TRUE)
    if (arg[0] == NULL)
    {
        UI32_T max_unit_num = 0;
        //STKTPLG_MGR_GetNumberOfUnit(&max_unit_num);
        max_unit_num = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
        /*  for (unit = 1; unit <= max_unit_num; unit++) */
        for (unit=0; STKTPLG_POM_GetNextUnit(&unit); )
        {
            if((line_num = Show_one_version(unit, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
    }
    else
    {
        unit = atoi(arg[0]);
        if (!STKTPLG_POM_UnitExist(unit))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Unit%lu does not exist\r\n",(unsigned long)unit);
#endif
            return CLI_NO_ERROR;
        }
        if((line_num = Show_one_version(unit, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }
#else
    STKTPLG_POM_GetMyUnitID(&unit);
    if((line_num = Show_one_version(unit, line_num)) == JUMP_OUT_MORE)
    {
        return CLI_NO_ERROR;
    }
    else if (line_num == EXIT_SESSION_MORE)
    {
        return CLI_EXIT_SESSION;
    }
#endif
    return CLI_NO_ERROR;
}

static UI32_T Show_one_version(UI32_T unit, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]                 = {0};
    STKTPLG_MGR_Switch_Info_T switch_info;
    STKTPLG_OM_Info_T sys_info;
    SYS_MGR_IndivPower_T indiv_power;
    UI8_T power_status[20] = {0};
#if (SYS_CPNT_SYSMGMT_SHOW_EXPAN_MODULE == TRUE)
    UI8_T type[20] = {0};
#endif
#ifdef ECS4810_12MV2
    BOOL_T main_power_working=FALSE;
#endif

    memset(&switch_info,0,sizeof(STKTPLG_MGR_Switch_Info_T));
    memset(&sys_info,0,sizeof(STKTPLG_OM_Info_T));
    memset(&indiv_power,0,sizeof(SYS_MGR_IndivPower_T));
    switch_info.sw_unit_index = unit;
    if (STKTPLG_PMGR_GetSwitchInfo(&switch_info) == TRUE)/*now only 1 unit*/
    {
#if (SYS_CPNT_SYSMGMT_SHOW_EXPAN_MODULE == TRUE)
        memset(type,0,sizeof(type));
#endif
        memset(power_status,0,sizeof(power_status));
        if (!STKTPLG_POM_GetSysInfo(switch_info.sw_unit_index, &sys_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            sprintf(buff,"Failed to get unit%u info\r\n",switch_info.sw_unit_index);
            PROCESS_MORE_FUNC(buff);
#endif
        }
        else
        {
            sprintf(buff,"Unit %u\r\n",switch_info.sw_unit_index);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff," Serial Number          : %s\r\n", switch_info.sw_serial_number);
            PROCESS_MORE_FUNC(buff);
#if (SYS_CPNT_SYSMGMT_SHOW_SERVICE_TAG == TRUE)
#if defined(STRAWMAN) || defined(STRAWMANHD)/*special for STRAWMAN*/
            sprintf(buff," Service Tag            : %s\r\n", switch_info.sw_chassis_service_tag);
            PROCESS_MORE_FUNC(buff);
#else
            sprintf(buff," Service Tag            : %s\r\n", switch_info.sw_service_tag);
            PROCESS_MORE_FUNC(buff);
#endif
#endif

            sprintf(buff," Hardware Version       : %s\r\n", switch_info.sw_hardware_ver);
            PROCESS_MORE_FUNC(buff);

            sprintf(buff," EPLD Version           : %s\r\n", switch_info.sw_epld_ver);
            PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_SYSMGMT_SHOW_MICROCODE_VER == TRUE)
            sprintf(buff," Micro Code F/W Version : %s\r\n", switch_info.sw_microcode_ver);
            PROCESS_MORE_FUNC(buff);
#endif



#if (SYS_CPNT_SYSMGMT_SHOW_EXPAN_MODULE == TRUE)
            switch(switch_info.sw_expansion_slot1)
            {
            case VAL_swExpansionSlot1_notPresent:
                strcpy((char*)type,"Not present");
                break;

            case VAL_swExpansionSlot1_hundredBaseFxScMmf:
                strcpy((char*)type,"100Base-FX-SC MMF");
                break;

            case VAL_swExpansionSlot1_hundredBaseFxScSmf:
                strcpy((char*)type,"100Base-FX-SC SMF");
                break;

            case VAL_swExpansionSlot1_hundredBaseFxMtrjMmf:
                strcpy((char*)type,"100Base-FX-MTRJ MMF");
                break;

            case VAL_swExpansionSlot1_thousandBaseSxScMmf:
                strcpy((char*)type,"1000Base-SX-SC MMF");
                break;

            case VAL_swExpansionSlot1_thousandBaseSxMtrjMmf:
                strcpy((char*)type,"1000Base-SX-MTRJ MMF");
                break;

            case VAL_swExpansionSlot1_thousandBaseXGbic:
                strcpy((char*)type,"1000BaseX-GBIC");
                break;

            case VAL_swExpansionSlot1_thousandBaseLxScSmf:
                strcpy((char*)type,"1000Base-LX-SC SMF");
                break;

            case VAL_swExpansionSlot1_thousandBaseT:
                strcpy((char*)type,"1000BaseT");
                break;

            case VAL_swExpansionSlot1_stackingModule:
                strcpy((char*)type,"Stacking Module");
                break;

            case VAL_swExpansionSlot1_thousandBaseSfp:
                strcpy((char*)type,"1000Base-SFP");
                break;

            case VAL_swExpansionSlot1_tenHundredBaseT4port:
                strcpy((char*)type,"4 port 10/100BaseT");
                break;

            case VAL_swExpansionSlot1_tenHundredBaseFxMtrj4port:
                strcpy((char*)type,"4 port 10/100Base-FX-MTRJ");
                break;

            case VAL_swExpansionSlot1_comboStackingSfp:
                strcpy((char*)type,"Combo Stacking SFP");
                break;

            case VAL_swExpansionSlot1_tenHundredBaseT:
                strcpy((char*)type,"10/100BaseT");
                break;

            case VAL_swExpansionSlot1_comboThousandBaseTxSfp:
                strcpy((char*)type,"Combo 1000BaseT SFP");
                break;
            default:
            case VAL_swExpansionSlot1_other:
                strcpy((char*)type,"Other");
                break;
            }
            sprintf((char*)buff," Module A Type          : %s\r\n", type);
            PROCESS_MORE_FUNC(buff);

            switch(switch_info.sw_expansion_slot2)
            {
            case VAL_swExpansionSlot2_notPresent:
                strcpy((char*)type,"Not present");
                break;

            case VAL_swExpansionSlot2_hundredBaseFxScMmf:
                strcpy((char*)type,"100Base-FX-SC MMF");
                break;

            case VAL_swExpansionSlot2_hundredBaseFxScSmf:
                strcpy((char*)type,"100Base-FX-SC SMF");
                break;

            case VAL_swExpansionSlot2_hundredBaseFxMtrjMmf:
                strcpy((char*)type,"100Base-FX-MTRJ MMF");
                break;

            case VAL_swExpansionSlot2_thousandBaseSxScMmf:
                strcpy((char*)type,"1000Base-SX-SC MMF");
                break;

            case VAL_swExpansionSlot2_thousandBaseSxMtrjMmf:
                strcpy((char*)type,"1000Base-SX-MTRJ MMF");
                break;

            case VAL_swExpansionSlot2_thousandBaseXGbic:
                strcpy((char*)type,"1000BaseX-GBIC");
                break;

            case VAL_swExpansionSlot2_thousandBaseLxScSmf:
                strcpy((char*)type,"1000Base-LX-SC SMF");
                break;

            case VAL_swExpansionSlot2_thousandBaseT:
                strcpy((char*)type,"1000BaseT");
                break;

            case VAL_swExpansionSlot2_stackingModule:
                strcpy((char*)type,"Stacking Module");
                break;

            case VAL_swExpansionSlot2_thousandBaseSfp:
                strcpy((char*)type,"1000Base-SFP");
                break;

            case VAL_swExpansionSlot2_tenHundredBaseT4port:
                strcpy((char*)type,"4 port 10/100BaseT");
                break;

            case VAL_swExpansionSlot2_tenHundredBaseFxMtrj4port:
                strcpy((char*)type,"4 port 10/100Base-FX-MTRJ");
                break;

            case VAL_swExpansionSlot2_comboStackingSfp:
                strcpy((char*)type,"Combo Stacking SFP");
                break;

            case VAL_swExpansionSlot2_tenHundredBaseT:
                strcpy((char*)type,"10/100BaseT");
                break;

            case VAL_swExpansionSlot2_comboThousandBaseTxSfp:
                strcpy((char*)type,"Combo 1000BaseT SFP");
                break;

            default:
            case VAL_swExpansionSlot2_other:
                strcpy((char*)type,"Other");
                break;
            }
            sprintf((char*)buff," Module B Type          : %s\r\n",type);
            PROCESS_MORE_FUNC(buff);
#endif

            sprintf(buff," Number of Ports        : %d\r\n",switch_info.sw_port_number);
            PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_SYSMGMT_SHOW_POWER==TRUE)
            indiv_power.sw_indiv_power_unit_index = unit;
            indiv_power.sw_indiv_power_index = 1;
            if(SYS_PMGR_GetSwitchIndivPower(&indiv_power)==TRUE)
            {
                switch(indiv_power.sw_indiv_power_status)
                {
                    case VAL_swIndivPowerStatus_green:
                    #ifdef ECS4810_12MV2
                    /* using * to represent whether
                    * power supply is working or not
                    */
                        strcpy((char *)power_status,"Up(*)");
                        main_power_working = TRUE;
                    #elif defined(ASF4512MP)
                        strcpy((char *)power_status,"Active");
                    #else
                        strcpy((char *)power_status,"Up");
                    #endif
                        break;

                    case VAL_swIndivPowerStatus_red:
                    #if defined(ASF4512MP)
                        strcpy((char *)power_status,"Inactive");
                    #else
                        strcpy((char *)power_status,"Down");
                    #endif
                        break;

                    case VAL_swIndivPowerStatus_notPresent:
                        strcpy((char *)power_status,"Not present");
                        break;

                    default:
                        break;
                }

                sprintf(buff," Main Power Status      : %s\r\n",power_status);
                PROCESS_MORE_FUNC(buff);
            }
#endif

#if (SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER == TRUE)
            memset(power_status,0,sizeof(power_status));

            indiv_power.sw_indiv_power_unit_index = unit;
            indiv_power.sw_indiv_power_index = 2;
            if(SYS_PMGR_GetSwitchIndivPower(&indiv_power)==TRUE)
            {
                switch(indiv_power.sw_indiv_power_status)
                {
                    case VAL_swIndivPowerStatus_green:
                    #ifdef ECS4810_12MV2
                        if(main_power_working == TRUE)
                            strcpy((char *)power_status,"Up");
                        else
                            strcpy((char *)power_status,"Up(*)");
                    #elif defined(ASF4512MP)
                        strcpy((char *)power_status,"Active");
                    #else
                        strcpy((char *)power_status,"Up");
                    #endif
                    break;

                    case VAL_swIndivPowerStatus_red:
                    #if defined(ASF4512MP)
                        strcpy((char *)power_status,"Inactive");
                    #else
                        strcpy((char *)power_status,"Down");
                    #endif
                        break;

                    case VAL_swIndivPowerStatus_notPresent:
                        strcpy((char *)power_status,"Not present");
                        break;

                    default:
                        break;
                }
                sprintf(buff," Redundant Power Status : %s\r\n",power_status);
                PROCESS_MORE_FUNC(buff);
            }
#endif
            {
                UI8_T role[15] = {0};
                switch(switch_info.sw_role_in_system)
                {
                case VAL_swRoleInSystem_master:
                    strcpy((char *)role, "Master");
                    break;
                case VAL_swRoleInSystem_backupMaster:
                    strcpy((char *)role, "Backup master");
                    break;
                case VAL_swRoleInSystem_slave:
                    strcpy((char *)role, "Slave");
                    break;
                default:
                    role[0] = ' ';
                }
                sprintf(buff," Role                   : %s\r\n",role);
                PROCESS_MORE_FUNC(buff);
            }

            sprintf(buff," Loader Version         : %s\r\n",switch_info.sw_loader_ver);
            PROCESS_MORE_FUNC(buff);

            sprintf(buff," Linux Kernel Version   : %s\r\n",switch_info.sw_kernel_ver);
            PROCESS_MORE_FUNC(buff);

            /* when diag_proc is not executed, switch_info.sw_boot_rom_ver will be empty
             * do not show Boot ROM Version when diag_proc is not executed.
             */
            if (switch_info.sw_boot_rom_ver[0]!=0)
            {
                sprintf(buff," Boot ROM Version       : %s\r\n",switch_info.sw_boot_rom_ver);
                PROCESS_MORE_FUNC(buff);
            }

            sprintf(buff, " Operation Code Version : %s\r\n",switch_info.sw_opcode_ver);
            PROCESS_MORE_FUNC(buff);

            {
                UI32_T      module_unit_id = 0;
                STKTPLG_MGR_switchModuleInfoEntry_T entry;

                if(STKTPLG_POM_OptionModuleIsExist(unit, &module_unit_id)==TRUE)
                {
                    memset(&entry, 0, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
                    entry.unit_index = unit;
                    entry.module_index = 1;

                    if(STKTPLG_PMGR_GetSwitchModuleInfoEntry(&entry)==TRUE)
                    {
                        sprintf(buff," Module Version         : %s\r\n",entry.op_code_ver);
                        PROCESS_MORE_FUNC(buff);
                    }
                }
            }
            PROCESS_MORE_FUNC("\r\n");
        }
    }

    return line_num;
}

static UI32_T Show_one_interface_status(UI32_T lport, UI32_T line_num, UI32_T port_type); /*eth, trunk*/
static UI32_T Show_one_vlan_interface(UI32_T vid, UI32_T line_num);
static UI32_T show_one_trunk_member(UI32_T trunk_id,UI32_T line_num);

/*execution*/
UI32_T CLI_API_Show_Interfaces_Status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    if (arg[0] == NULL)
    {
        /* all ethernet*/
        {
            UI32_T lport;
            UI32_T verify_unit = ctrl_P->sys_info.my_unit_id;
            UI32_T verify_port;
            UI32_T i,current_max_unit;
            UI32_T max_port_num;
            CLI_API_EthStatus_T verify_ret;

#if (SYS_CPNT_STACKING == TRUE)
            //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
            current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
            current_max_unit = 1;
#endif
            /*   for(i = 1; i <= current_max_unit; i++) */
            for (i=0; STKTPLG_POM_GetNextUnit(&i); )
            {
                verify_unit = i;
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

                for(verify_port = 1; verify_port <= max_port_num; verify_port++)
                {
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                    if(verify_ret == CLI_API_ETH_NOT_PRESENT) /* talor 2004-08-31 */
                        continue;

#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        sprintf((char*)buff, "Information of %s:\r\n", name);
                        PROCESS_MORE(buff);
                    }
#else
                    sprintf(buff, "Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                    PROCESS_MORE(buff);
#endif

                    switch(verify_ret)
                    {
                        case CLI_API_ETH_OK:
                            if((line_num = Show_one_interface_status(lport, line_num, PORT_TYPE_ETH)) ==  JUMP_OUT_MORE)
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
                            break;

                        case CLI_API_ETH_TRUNK_MEMBER:
                            if((line_num = Show_one_interface_status(lport, line_num, PORT_TYPE_ETH)) ==  JUMP_OUT_MORE)
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
                            break;

                        case CLI_API_ETH_NOT_PRESENT:
                            sprintf(buff, " Not present.\r\n");
                            PROCESS_MORE(buff);
                            break;

                        case CLI_API_ETH_UNKNOWN_PORT:
                            sprintf(buff, " Unknown port.\r\n");
                            PROCESS_MORE(buff);
                            break;
                    }
                }
            }/*end of unit loop*/
        }

        /*all trunk*/
        {
            UI32_T lport = 0;
            UI32_T verify_trunk_id;
            CLI_API_TrunkStatus_T verify_ret;

            for(verify_trunk_id = 1; verify_trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; verify_trunk_id++)
            {
                if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    continue;
                }
                else
                {
                    sprintf(buff, "Information of Trunk %lu\r\n", (unsigned long)verify_trunk_id);
                    PROCESS_MORE(buff);

                    if((line_num = Show_one_interface_status(lport, line_num, PORT_TYPE_TRUNK)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }

                    if((line_num = show_one_trunk_member(verify_trunk_id, line_num)) == JUMP_OUT_MORE)
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
        }

        /*all vlan*/
        {
            UI32_T time_mark = 0;
            UI32_T vid       = 0;

            while(VLAN_POM_GetNextVlanId(time_mark, &vid))
            {
                sprintf(buff, "Information of VLAN %lu\r\n", (unsigned long)vid);
                PROCESS_MORE(buff);

                if((line_num = Show_one_vlan_interface(vid, line_num)) == JUMP_OUT_MORE)
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
    }
    else
    {
        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                UI32_T lport = 0;
                UI32_T verify_unit = atoi((char *) arg[1]);
                UI32_T verify_port = atoi( strchr(arg[1], '/')+1 );
                CLI_API_EthStatus_T verify_ret;

#if (CLI_SUPPORT_PORT_NAME == 1)
                if (isdigit(arg[1][0]))
                {
                    verify_unit = atoi(arg[1]);
                    verify_port = atoi(strchr(arg[1],'/')+1);
                }
                else/*port name*/
                {
                    UI32_T trunk_id = 0;
                    if (!IF_PMGR_IfnameToIfindex(arg[1], &lport))
                    {
                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
                        return CLI_NO_ERROR;
                    }
                    SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                }
#endif

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        CLI_LIB_PrintStr_1("Information of %s\r\n", name);
                    }
#else
                        CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                        line_num++;
                        Show_one_interface_status(lport, line_num, PORT_TYPE_ETH);
                        break;

                    case CLI_API_ETH_TRUNK_MEMBER:
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        CLI_LIB_PrintStr_1("Information of %s\r\n", name);
                    }
#else
                        CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
                        line_num++;
                        Show_one_interface_status(lport, line_num, PORT_TYPE_ETH);
                        break;

                    case CLI_API_ETH_NOT_PRESENT:
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        break;

                    case CLI_API_ETH_UNKNOWN_PORT:
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        break;
                }
            }
                break;

            case 'p':
            case 'P':
            {
                UI32_T lport = 0;
                UI32_T verify_trunk_id = atoi(arg[1]);
                CLI_API_TrunkStatus_T verify_ret;

                if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret, verify_trunk_id);
                    return CLI_NO_ERROR;
                }
                else
                {
                    sprintf(buff, "Information of Trunk %lu\r\n", (unsigned long)verify_trunk_id);
                    PROCESS_MORE(buff);

                    if((line_num = Show_one_interface_status(lport, line_num, PORT_TYPE_TRUNK)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                    else if (line_num == EXIT_SESSION_MORE)
                    {
                        return CLI_EXIT_SESSION;
                    }

                    show_one_trunk_member(verify_trunk_id, line_num);
                    break;
                }
            }
                break;

            case 'v':
            case 'V':
            {
                UI32_T vid = atoi(arg[1]);

                if (!VLAN_POM_IsVlanExisted(vid))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    PROCESS_MORE("No such VLAN.\r\n");
#endif
                }
                else
                {
                    sprintf(buff, "Information of VLAN %lu\r\n", (unsigned long)vid);
                    PROCESS_MORE(buff);

                    Show_one_vlan_interface(vid, line_num);
                }
            }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
    }

    return CLI_NO_ERROR;
}

#define MAX_TRUNCATE_PORT_NAME_LENGTH    17

UI32_T CLI_API_Show_Interfaces_Brief(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T lport;
    UI32_T verify_unit = ctrl_P->sys_info.my_unit_id;
    UI32_T verify_port;
    UI32_T i;
    UI32_T max_port_num;
    UI32_T trunk_id      = 0;
    UI32_T name_length;
    char   tmp_buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    CLI_API_EthStatus_T verify_ret;
    PRI_MGR_Dot1dPortPriorityEntry_T priority_entry;
    Port_Info_T swctr_port_info;
    VLAN_OM_Vlan_Port_Info_T port_info;

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == FALSE)
    PROCESS_MORE("Interface Name              Status    PVID Pri Speed/Duplex  Type         Trunk\r\n");
    PROCESS_MORE("--------- ----------------- --------- ---- --- ------------- ------------ -----\r\n");
#else
    PROCESS_MORE("Interface Name              Status    Pri Speed/Duplex  Type         Trunk\r\n");
    PROCESS_MORE("--------- ----------------- --------- --- ------------- ------------ -----\r\n");
#endif

    for (i=0; STKTPLG_POM_GetNextUnit(&i); )
    {
        verify_unit = i;
        max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

        for (verify_port = 1; verify_port <= max_port_num; verify_port++)
        {
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

            if (verify_ret == CLI_API_ETH_NOT_PRESENT)
                continue;

            /* ethernet port
             */
            sprintf(tmp_buff, "Eth %lu/%2lu ", (unsigned long)verify_unit, (unsigned long)verify_port);
            strcat(buff, tmp_buff);
            memset(tmp_buff , 0, sizeof(tmp_buff));

            if (SWCTRL_POM_GetPortInfo(lport, &swctr_port_info))
            {
                /* port name
                 */
                name_length = strlen((char *)swctr_port_info.port_name);

                if ( name_length >= MAX_TRUNCATE_PORT_NAME_LENGTH )
                {
                    /* 1 space char + MAX_TRUNCATE_PORT_NAME_LENGTH + 1 null char
                     */
                    snprintf(tmp_buff, 19, " %s", swctr_port_info.port_name);
                }
                else
                {
                    sprintf(tmp_buff, " %s", swctr_port_info.port_name);
                    memset(tmp_buff + name_length +1, ' ', (MAX_TRUNCATE_PORT_NAME_LENGTH - name_length));
                }
                strcat(buff, tmp_buff);
                memset(tmp_buff , 0, sizeof(tmp_buff));

                /* current status / shudown reason
                 */
                if (swctr_port_info.admin_state == VAL_ifAdminStatus_up)
                {
                    if (swctr_port_info.link_status == SWCTRL_LINK_UP)
                    {
                        strcat(buff, " Up       ");
                    }
                    else
                    {
                        char *desc = " Down     ";
                        int i;

                        for (i = 0; i < sizeof(cli_api_show_shutdown_reasons)/sizeof(*cli_api_show_shutdown_reasons); i++)
                        {
                            if (swctr_port_info.shutdown_reason & cli_api_show_shutdown_reasons[i].val)
                            {
                                desc = cli_api_show_shutdown_reasons[i].brief_desc;
                                break;
                            }
                        }
                        strcat(buff, desc);
                    }
                }
                else
                {
                    strcat(buff, " Disabled ");
                }

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == FALSE)
                /* pvid
                 */
                VLAN_PMGR_GetPortEntry(lport, &port_info);
                sprintf(tmp_buff," %4lu", (unsigned long)port_info.port_item.dot1q_pvid_index);
                strcat(buff, tmp_buff);
                memset(tmp_buff , 0, sizeof(tmp_buff));
#endif

                /* priority
                 */
                PRI_PMGR_GetDot1dPortPriorityEntry(lport, &priority_entry);
                sprintf(tmp_buff, " %3lu", (unsigned long)priority_entry.dot1d_port_default_user_priority );
                strcat(buff, tmp_buff);
                memset(tmp_buff , 0, sizeof(tmp_buff));

                /* speed duplex current status
                 */
                if (swctr_port_info.link_oper_status == VAL_ifOperStatus_up)
                {
                    char speed_duplex_str[SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN];

                    if (swctr_port_info.autoneg_state == VAL_portAutonegotiation_enabled)
                    {
                        strcat(buff, " Auto-");
                    }
                    else
                    {
                        strcat(buff, " ");    /* preceding space */
                        strcpy(tmp_buff, "     ");    /* spaces# in the length of "Auto-", to be appended later */
                    }

                    if (SWDRV_LIB_MapSpeedDuplex(swctr_port_info.speed_duplex_oper, sizeof(speed_duplex_str),speed_duplex_str)==TRUE )
                    {
                        sprintf(buff+strlen(buff), "%-8s", speed_duplex_str);
                    }
                    else
                    {
                        sprintf(buff+strlen(buff), "%-8s", "");
                    }

                    strcat(buff, tmp_buff);    /* append last spaces once linkup in forced mode */
                    memset(tmp_buff , 0, sizeof(tmp_buff));
                }
                else
                {
                    /* force mode speed-duplex
                     */
                    if (swctr_port_info.autoneg_state == VAL_portAutonegotiation_enabled)
                    {
                        strcat(buff, " Auto         ");
                    }
                    else
                    {
                        /* speed duplex config
                         */
                        char speed_duplex_str[SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN];

                        if ( SWDRV_LIB_MapSpeedDuplex(swctr_port_info.speed_duplex_cfg, sizeof(speed_duplex_str), speed_duplex_str)==TRUE )
                        {
                            sprintf(buff+strlen(buff), " %-13s", speed_duplex_str);
                        }
                        else
                        {
                            sprintf(buff+strlen(buff), "%14s", "");
                        }

                    }
                } /* end of if(swctr_port_info.link_oper_status == VAL_ifOperStatus_up) */

                /* port type
                 */
                if ( SWDRV_LIB_MapPortType(swctr_port_info.port_type, CLI_DEF_MAX_BUFSIZE, tmp_buff)==TRUE)
                {
                    sprintf(buff+strlen(buff), " %-12s", tmp_buff);
                }
                else
                {
                    sprintf(buff+strlen(buff), " %-12s", "Not Present");
                }

                /* trunk id
                 */
                if (verify_ret == CLI_API_ETH_TRUNK_MEMBER)
                {
                    SWCTRL_POM_UserPortToTrunkPort(verify_unit, verify_port, &trunk_id);
                    sprintf(tmp_buff, " %-5lu\r\n", (unsigned long)trunk_id);
                    strcat(buff, tmp_buff);
                    memset(tmp_buff , 0, sizeof(tmp_buff));
                }
                else
                {
                    strcat(buff, " None \r\n");
                }

                PROCESS_MORE(buff);
                memset(buff , 0, sizeof(buff));

            } /* end of if (SWCTRL_POM_GetPortInfo(lport, &swctr_port_info)) */
        } /* end of for(verify_port = 1; verify_port <= max_port_num; verify_port++) */
    } /* end of for (i=0; STKTPLG_MGR_GetNextUnit(&i); ) */

    /* all trunk
     */
    {
        UI32_T lport = 0;
        UI32_T verify_trunk_id;
        CLI_API_TrunkStatus_T verify_ret;

        for (verify_trunk_id = 1; verify_trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; verify_trunk_id++)
        {
            if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
                continue;
            }
            else
            {
                /* port-channel
                 */
                sprintf(tmp_buff, "Trunk %2lu ", (unsigned long)verify_trunk_id);
                strcat(buff, tmp_buff);
                memset(tmp_buff, 0, sizeof(tmp_buff));

                if (SWCTRL_POM_GetPortInfo(lport, &swctr_port_info))
                {
                    /* port-channel name
                     */
                    name_length = strlen((char *)swctr_port_info.port_name);

                    if ( name_length >= MAX_TRUNCATE_PORT_NAME_LENGTH )
                    {
                        /* 1 space char + MAX_TRUNCATE_PORT_NAME_LENGTH + 1 null char
                         */
                        snprintf(tmp_buff, 20, " %s", swctr_port_info.port_name);
                    }
                    else
                    {
                        sprintf(tmp_buff, " %s", swctr_port_info.port_name);
                        memset(tmp_buff + name_length +1 , ' ', (MAX_TRUNCATE_PORT_NAME_LENGTH - name_length));
                    }
                    strcat(buff, tmp_buff);
                    memset(tmp_buff , 0, sizeof(tmp_buff));

                    /* current status
                     */
                    if (swctr_port_info.admin_state == VAL_ifAdminStatus_up)
                    {
                        if (swctr_port_info.link_status == SWCTRL_LINK_UP)
                        {
                            strcat(buff, " Up       ");
                        }
                        else
                        {
                            char *desc = " Down     ";
                            int i;

                            for (i = 0; i < sizeof(cli_api_show_shutdown_reasons)/sizeof(*cli_api_show_shutdown_reasons); i++)
                            {
                                if (swctr_port_info.shutdown_reason & cli_api_show_shutdown_reasons[i].val)
                                {
                                    desc = cli_api_show_shutdown_reasons[i].brief_desc;
                                    break;
                                }
                            }
                            strcat(buff, desc);
                        }
                    }
                    else
                    {
                        strcat(buff, " Disabled ");
                    }

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == FALSE)
                    /* pvid
                     */
                    VLAN_PMGR_GetPortEntry(lport, &port_info);
                    sprintf(tmp_buff," %4lu", (unsigned long)port_info.port_item.dot1q_pvid_index);
                    strcat(buff, tmp_buff);
                    memset(tmp_buff , 0, sizeof(tmp_buff));
#endif

                    /* priority
                     */
                    PRI_PMGR_GetDot1dPortPriorityEntry(lport, &priority_entry);
                    sprintf(tmp_buff, " %3lu", (unsigned long)priority_entry.dot1d_port_default_user_priority );
                    strcat(buff, tmp_buff);
                    memset(tmp_buff , 0, sizeof(tmp_buff));

                    /* speed duplex current status
                     */
                    if (swctr_port_info.link_oper_status == VAL_ifOperStatus_up)
                    {
                        char speed_duplex_str[SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN];

                        if (swctr_port_info.autoneg_state == VAL_portAutonegotiation_enabled)
                        {
                            strcat(buff, " Auto-");
                        }
                        else
                        {
                            strcat(buff, " ");    /* preceding space */
                            strcpy(tmp_buff, "     ");    /* spaces# in the length of "Auto-", to be appended later */
                        }

                        if ( SWDRV_LIB_MapSpeedDuplex(swctr_port_info.speed_duplex_oper, sizeof(speed_duplex_str), speed_duplex_str)==TRUE )
                        {
                            sprintf(buff+strlen(buff), "%-8s", speed_duplex_str);
                        }
                        else
                        {
                            sprintf(buff+strlen(buff), "%-8s", "");
                        }


                        strcat(buff, tmp_buff); /* Append last spaces once linkup in forced mode */
                        memset(tmp_buff , 0, sizeof(tmp_buff));
                    }
                    else
                    {
                        /* force mode speed-duplex
                         */
                        if (swctr_port_info.autoneg_state == VAL_portAutonegotiation_enabled)
                        {
                            strcat(buff, " Auto         ");
                        }
                        else
                        {
                            /* speed duplex config
                             */
                            char speed_duplex_str[SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN];

                            if ( SWDRV_LIB_MapSpeedDuplex(swctr_port_info.speed_duplex_cfg, sizeof(speed_duplex_str), speed_duplex_str)==TRUE )
                            {
                                sprintf(buff+strlen(buff), " %-13s", speed_duplex_str);
                            }
                            else
                            {
                                sprintf(buff+strlen(buff), " %-13s", "");
                            }

                        }
                    } /* end of if(swctr_port_info.link_oper_status == VAL_ifOperStatus_up) */

                    /* port type
                     */
                    if ( SWDRV_LIB_MapPortType(swctr_port_info.port_type, CLI_DEF_MAX_BUFSIZE, tmp_buff)==TRUE)
                    {
                        sprintf(buff+strlen(buff), " %-12s", tmp_buff);
                    }
                    else
                    {
                        sprintf(buff+strlen(buff), " %-12s", "Not Present");
                    }

                    /* trunk id
                     */
                    sprintf(tmp_buff, " %-5lu\r\n", (unsigned long)verify_trunk_id);
                    strcat(buff, tmp_buff);
                    memset(tmp_buff, 0, sizeof(tmp_buff));
                    PROCESS_MORE(buff);
                    memset(buff , 0, sizeof(buff));
                } /* if (SWCTRL_POM_GetPortInfo(lport, &swctr_port_info)) */
            } /* if ((verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK) */
        } /* for (verify_trunk_id = 1; verify_trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; verify_trunk_id++) */
    } /* all trunk */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_CRAFT_PORT == TRUE)
UI32_T CLI_API_Show_Interfaces_Craft(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CRAFT_PORT_MODE == SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT)
    UI32_T line_num = 0;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    SWDRV_CraftPort_Info_T swdrv_craftport_info;

    SWDRV_OM_GetCraftPortInfo(&swdrv_craftport_info);

    if(swdrv_craftport_info.link_status == TRUE)
        strcat(buff, "Craft Port Link Status:  Up\r\n");
    else
        strcat(buff, "Craft Port Link Status:  Down\r\n");
    PROCESS_MORE(buff);
#endif
    return CLI_NO_ERROR;
}
#endif
static void show_uport_list(UI8_T *port_list, int indent, char *buff)
{
    UI32_T port_counter = 0;
    BOOL_T is_first_line = TRUE;
    char buffTempIndent[indent+3]; /* prefix "\r\n" and null terminate */
    int i;

    sprintf(buffTempIndent, "\r\n%*s", indent, " ");

    buff[0] = 0;

    for (i = 1; i <= ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST)*8); i++)
    {
        if (port_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            UI32_T unit = 0;
            UI32_T port = 0;
            UI32_T trunk_id = 0;
            UI8_T  buffTempPort[10]  = {0};

            port_counter++;

            /* for new line space */
            if (port_counter%5 == 1)
            {
                if(is_first_line)
                {
                    is_first_line = FALSE;
                }
                else
                {
                    strcat(buff, buffTempIndent);
                }
            }

#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(i,name);
                sprintf((char *)buffTempPort,"%7s, \r\n", name);
                strcat(buff, (char *)buffTempPort);
            }
#else
            SWCTRL_POM_LogicalPortToUserPort(i, &unit, &port, &trunk_id);
            sprintf((char *)buffTempPort,"Eth%lu/%lu, ",(unsigned long)unit,(unsigned long)port);
            strcat(buff, (char *)buffTempPort);
#endif
        }
    }

    if (port_counter > 0)
    {
        buff[strlen(buff)-2] = 0;
    }
    else
    {
        sprintf(buff, "None");
    }
}

static UI32_T show_one_trunk_member(UI32_T trunk_id, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]      = {0};
    char   buffTemp[CLI_DEF_MAX_BUFSIZE]  = {0};
    TRK_MGR_TrunkEntry_T trunk_entry;
    Port_Info_T port_info;

    memset(&port_info,0,sizeof(Port_Info_T));
    memset(&trunk_entry, 0 , sizeof(TRK_MGR_TrunkEntry_T));

    /* Show specific trunk informations */

    trunk_entry.trunk_index = trunk_id;

    if(TRK_PMGR_GetTrunkEntry(&trunk_entry))
    {
        UI32_T trunk_ifindex = 0;
        SWCTRL_POM_TrunkIDToLogicalPort(trunk_entry.trunk_index,&trunk_ifindex);

        if (!SWCTRL_POM_GetPortInfo(trunk_ifindex, &port_info))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to get trunk port info\r\n");
#endif
            return JUMP_OUT_MORE;
        }

        {
            char *title = "  Member Ports                : ";

            show_uport_list(trunk_entry.trunk_ports, strlen(title), buffTemp);

            sprintf(buff, "%s%s\r\n", title, buffTemp);
            PROCESS_MORE_FUNC(buff);
        }

        {
            char *title = "  Active Member Ports         : ";

            show_uport_list(trunk_entry.active_trunk_ports, strlen(title), buffTemp);

            sprintf(buff, "%s%s\r\n", title, buffTemp);
            PROCESS_MORE_FUNC(buff);
        }
    }
    else
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Trunk %lu does not exist.\r\n", (unsigned long)trunk_id);
#endif
        return JUMP_OUT_MORE;
    }
    return line_num;
}

static UI32_T Show_Counters_One_Port(UI32_T ifindex,UI32_T line_num, CLI_TASK_WorkingArea_T *ctrl_P);

UI32_T CLI_API_Show_Interfaces_Counters(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    SWDRV_IfTableStats_T if_table_stats;
    UI32_T ifindex  = 0;
    UI32_T line_num = 0;


    if (arg[0] == NULL)/*show all counters*/
    {
        while(NMTR_PMGR_GetNextIfTableStats(&ifindex, &if_table_stats))
        {
            UI32_T unit;
            UI32_T port;
            UI32_T trunk_id;
            SWCTRL_Lport_Type_T ret;

            ret = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

            if(ret == SWCTRL_LPORT_UNKNOWN_PORT)
                continue;

            line_num = Show_Counters_One_Port(ifindex, line_num, ctrl_P);

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
    else /*specify one port or trunk*/
    {
        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                UI32_T              verify_unit = atoi(arg[1]);
                UI32_T              verify_port = atoi( strchr(arg[1], '/')+1 );
                CLI_API_EthStatus_T verify_ret  = 0;
#if (CLI_SUPPORT_PORT_NAME == 1)
                if (isdigit(arg[1][0]))
                {
                    verify_unit = atoi(arg[1]);
                    verify_port = atoi(strchr(arg[1],'/')+1);
                }
                else/*port name*/
                {
                    UI32_T trunk_id = 0;
                    if (!IF_PMGR_IfnameToIfindex(arg[1], &ifindex))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
#endif
                        return CLI_NO_ERROR;
                    }
                    SWCTRL_PMGR_LogicalPortToUserPort(ifindex, &verify_unit, &verify_port, &trunk_id);
                }
#endif
                verify_ethernet(verify_unit, verify_port, &ifindex);
                if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT )
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    return CLI_NO_ERROR;
                }
                else
                {
                    Show_Counters_One_Port(ifindex, line_num, ctrl_P);
                }
            }
                break;

        case 'p':
        case 'P':
        {
            UI32_T  trunk_id = atoi(arg[1]);
            CLI_API_TrunkStatus_T verify_ret_t;

            if( (verify_ret_t = verify_trunk(trunk_id, &ifindex)) == CLI_API_TRUNK_NOT_EXIST)
            {
                display_trunk_msg(verify_ret_t, trunk_id);
                return CLI_NO_ERROR;
            }

            Show_Counters_One_Port(ifindex, line_num, ctrl_P);
        }
            break;

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
        case 'v':
        case 'V':
        {
            UI32_T  vid = atoi(arg[1]);

            if (!VLAN_POM_IsVlanExisted(vid))
            {
                CLI_LIB_PrintStr_1("VLAN %lu does not exist\r\n", (unsigned long)vid);
                return CLI_NO_ERROR;
            }

            VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
            Show_Counters_One_Port(ifindex, line_num, ctrl_P);
        }
            break;
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

        default:
            break;
        }
    }
    return CLI_NO_ERROR;
}

#define PORT_NAME_LEN_IN_ONE_LINE 45
static UI32_T Show_one_interface_status(UI32_T lport, UI32_T line_num, UI32_T port_type)
{
    enum SHOW_STATUS
    {
        SHOW_STATUS_ERROR = 0,
        SHOW_STATUS_DISABLE,
        SHOW_STATUS_ENABLE,
        SHOW_STATUS_UP,
        SHOW_STATUS_DOWN,
        SHOW_STATUS_TESTING,

        SHOW_STATUS_UNKNOWM,
        SHOW_STATUS_DORMANT,
        SHOW_STATUS_NOT_PRESENT,
        SHOW_STATUS_LOWLAYERDOWN,


        SHOW_STATUS_TRUE,
        SHOW_STATUS_FALSE,
        SHOW_STATUS_SPEED_10HALF,
        SHOW_STATUS_SPEED_10FULL,
        SHOW_STATUS_SPEED_100HALF,
        SHOW_STATUS_SPEED_100FULL,
        SHOW_STATUS_SPEED_1000HALF,
        SHOW_STATUS_SPEED_1000FULL,
        SHOW_STATUS_SPEED_10G_HALF,
        SHOW_STATUS_SPEED_10G_FULL,
        SHOW_STATUS_SPEED_40G_HALF,
        SHOW_STATUS_SPEED_40G_FULL,
        SHOW_STATUS_SPEED_AUTO,
        SHOW_STATUS_BACK_PRESSURE,
        SHOW_STATUS_DOT3X,
        SHOW_STATUS_NONE,
        SHOW_STATUS_PACKET,
        SHOW_STATUS_PERCENT,
        SHOW_STATUS_RATE,
        SHOW_STATUS_TYPE_OTHER,
        SHOW_STATUS_TYPE_100TX,
        SHOW_STATUS_TYPE_100FX,
        SHOW_STATUS_TYPE_1000SX,
        SHOW_STATUS_TYPE_1000LX,
        SHOW_STATUS_TYPE_1000T,
        SHOW_STATUS_TYPE_1000GBIC,
        SHOW_STATUS_TYPE_SFP,
        SHOW_STATUS_TYPE_TenG,
        SHOW_STATUS_TYPE_TenG_COPPER,
        SHOW_STATUS_TYPE_TenG_XFP,
        SHOW_STATUS_TYPE_TenG_SFP,
        SHOW_STATUS_TYPE_FortyG_QSFP,
        SHOW_STATUS_TYPE_FxSc_SINGLE_MODE,
        SHOW_STATUS_TYPE_FxSc_MULTI_MODE,
        SHOW_STATUS_TRUNK_TYPE_USER,
        SHOW_STATUS_TRUNK_TYPE_LACP,
        SHOW_STATUS_PSEC_ACTION_NONE,
        SHOW_STATUS_PSEC_ACTION_SHUTDOWN,
        SHOW_STATUS_PSEC_ACTION_TRAP,
        SHOW_STATUS_PSEC_ACTION_TRAPANDSHUTDOWN,
        SHOW_STATUS_COMBO_FORCED_MODE_NONE,
        SHOW_STATUS_COMBO_FORCED_MODE_COPPER_FORCED,
        SHOW_STATUS_COMBO_FORCED_MODE_COPPER_PREFERRED_AUTO,
        SHOW_STATUS_COMBO_FORCED_MODE_SFP_FORCED,
        SHOW_STATUS_COMBO_FORCED_MODE_SFP_PREFERRED_AUTO,
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)
        SHOW_STATUS_COMBO_GIGA_PHY_MODE_MASTER,
        SHOW_STATUS_COMBO_GIGA_PHY_MODE_SLAVE,
        SHOW_STATUS_COMBO_GIGA_PHY_MODE_NONE,
#endif /* #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE) */


#if ( SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE )
        SHOW_STATUS_MDIX_AUTO,
        SHOW_STATUS_MDIX_CROSSOVER,
        SHOW_STATUS_MDIX_STRAIGHT,
        SHOW_STATUS_MDIX_NOTSUPPORT,
#endif


    };


    char *str[] =   /*added by Jinhua Wei ,to remove warning ,becaued the type isn't match with where it is used*/
    {
        "ERROR",
        "Disabled",
        "Enabled",
        "Up",
        "Down",
        "Testing",
        "Unknown",
        "Dormant",
        "Not present",
        "Low layer down",
        "True",
        "False",
        "10half",
        "10full",
        "100half",
        "100full",
        "1000half",
        "1000full",
        "10G half",
        "10G full",
        "40G half",
        "40G full",
        "Auto",
        "Back pressure",
        "Dot3X",
        "None",
#if defined(UNICORN)/*special for UNICORN*/
        "counts/frame",
#else
        "packets/second",
#endif
        "percent",
        "kbits/second",
        "Other",
        "100BASE-TX",
        "100BASE-FX",
        "1000BASE-SX",
        "1000BASE-LX",
        "1000BASE-T",
        "1000BASE GBIC",
        "1000BASE SFP",
        "10GBASE",
        "10GBASE-T",
        "10GBASE XFP",
        "10GBASE SFP+",
        "40GBASE QSFP",
        "FxSc single",
        "FxSc multi",
        "User",
        "LACP",
        "None",
        "Shutdown",
        "Trap",
        "Trap and shutdown",
        "None",
        "Copper forced",
        "Copper preferred auto",
        "SFP forced",
        "SFP preferred auto",
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)
        "Master",
        "Slave",
        "None",
#endif /* #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE) */

#if ( SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE )
        "Auto", /* Auto: AUTO */
        "Crossover", /* Crossover:MDIX */
        "Straight",  /* Straight:MDI */
        "None", /* not support  */
#endif

    };

    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   capability1[80]            = {0};
    char   capability2[80]            = {0};
    char speed_duplex_str[SWDRV_LIB_SPEED_DUPLEX_STR_MAX_LEN] = {0};
    char port_type_str[SWDRV_LIB_PORT_TYPE_STR_MAX_LEN] = {0};

    UI32_T user_unit     = 0;
    UI32_T user_port     = 0;
    UI32_T trunk_id      = 0;
    UI32_T lacp_state   = 0;

    UI8_T speed_c       = 0;/*set config*/
    UI8_T speed_s       = 0;/*current status*/
    UI8_T admin_status  = 0;
    UI8_T link_status   = 0;
    UI8_T oper_status   = 0;
    UI8_T flowcontrol_c = 0;/*set config*/
    UI8_T flowcontrol_s = 0;/*current status*/
    UI8_T broadcast     = 0;
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
    UI8_T multicast     = 0;
#endif
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
    UI8_T unicast       = 0;
#endif
    UI8_T storm_mode_type = 0;
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
    UI8_T mcast_storm_mode      = 0;
#endif
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
    UI8_T unicast_storm_mode    = 0;
#endif
    UI8_T autoneg_state = 0;
    UI8_T lacp_ptr     = 0;
    UI8_T porttype     = 0;
    UI8_T trunktype    = 0;
#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    UI8_T combo_mode   = 0;
#endif

#if ( SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE )
    UI32_T mdix_mode     = 0; /* mdix mode, 1:auto, 2:straight, 3:crossover */
    UI8_T mdix_status   = 0;/* MDIX mode status */
#endif

#ifdef ASF4526B_FLF_P5
    BOOL_T is_present;
#endif

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)
    UI8_T force_mode_1000full = 0;
#endif /* #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE) */
    Port_Info_T port_info;
    SWCTRL_PortAbility_T port_ability;

    BOOL_T is_1st_line = TRUE;
    UI32_T port_name_char_index = 0;
    UI32_T port_name_len = 0;
    UI8_T  port_name_buff[MAXSIZE_ifName + 1] = {0};

    if (!SWCTRL_POM_GetPortAbility(lport, &port_ability))
    {
        memset(&port_ability, 0, sizeof(port_ability));
    }

    if (SWCTRL_POM_GetPortInfo(lport, &port_info))
    {
        UI8_T mac[6] = {0};

        SWCTRL_POM_GetPortMac(lport, mac);
        SWCTRL_POM_LogicalPortToUserPort(lport, &user_unit, &user_port, &trunk_id);

        /* admin status */
        switch(port_info.admin_state)
        {
        case VAL_ifAdminStatus_up:
            admin_status = SHOW_STATUS_UP;
            break;

        case VAL_ifAdminStatus_down:
            admin_status = SHOW_STATUS_DOWN;
            break;

        case VAL_ifAdminStatus_testing:
            admin_status = SHOW_STATUS_TESTING;

        default:
            admin_status=0;
            ;
        }

#if ( SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE )
        /* MDIX mode status */
        if (SWCTRL_PMGR_GetMDIXMode(lport, &mdix_mode) == TRUE)
        {
            switch(mdix_mode)
            {
                case VAL_portMdixMode_auto:
                    mdix_status = SHOW_STATUS_MDIX_AUTO;
                break;

                case VAL_portMdixMode_crossover:
                    mdix_status = SHOW_STATUS_MDIX_CROSSOVER;
                break;

                case VAL_portMdixMode_straight:
                    mdix_status = SHOW_STATUS_MDIX_STRAIGHT;
                break;

                default:
                    mdix_status = SHOW_STATUS_MDIX_NOTSUPPORT;
                 ;
            }
        }
#endif

        /* link status*/
        if (port_info.link_status == SWCTRL_LINK_UP)
            link_status = SHOW_STATUS_UP;
        else
            link_status = SHOW_STATUS_DOWN;

        /*oper status*/
        switch(port_info.link_oper_status)
        {
        case VAL_ifOperStatus_up:
            oper_status = SHOW_STATUS_UP;
            break;

        case VAL_ifOperStatus_down:
            oper_status = SHOW_STATUS_DOWN;
            break;

        case VAL_ifOperStatus_testing:
            oper_status = SHOW_STATUS_TESTING;
            break;

        case VAL_ifOperStatus_unknown:
            oper_status = SHOW_STATUS_UNKNOWM;
            break;

        case VAL_ifOperStatus_dormant:
            oper_status = SHOW_STATUS_DORMANT;
            break;

        case VAL_ifOperStatus_notPresent:
            oper_status = SHOW_STATUS_NOT_PRESENT;
            break;

        case VAL_ifOperStatus_lowerLayerDown:
            oper_status = SHOW_STATUS_LOWLAYERDOWN;
            break;

        default:
            oper_status=0;
            ;
        }

        /* speed duplex config */
        switch(port_info.speed_duplex_cfg)
        {
        case VAL_portSpeedDpxCfg_halfDuplex10:
            speed_c = SHOW_STATUS_SPEED_10HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10:
            speed_c = SHOW_STATUS_SPEED_10FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex100:
            speed_c = SHOW_STATUS_SPEED_100HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex100:
            speed_c = SHOW_STATUS_SPEED_100FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex1000:
            speed_c = SHOW_STATUS_SPEED_1000HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex1000:
            speed_c = SHOW_STATUS_SPEED_1000FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex10g:
            speed_c = SHOW_STATUS_SPEED_10G_HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10g:
            speed_c = SHOW_STATUS_SPEED_10G_FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex40g:
            speed_c = SHOW_STATUS_SPEED_40G_HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex40g:
            speed_c = SHOW_STATUS_SPEED_40G_FULL;
            break;

        default:
            speed_c=0;
            break;
        }

        /* speed duplex current status */
        switch(port_info.speed_duplex_oper)
        {
        case VAL_portSpeedDpxCfg_halfDuplex10:
            speed_s = SHOW_STATUS_SPEED_10HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10:
            speed_s = SHOW_STATUS_SPEED_10FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex100:
            speed_s = SHOW_STATUS_SPEED_100HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex100:
            speed_s = SHOW_STATUS_SPEED_100FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex1000:
            speed_s = SHOW_STATUS_SPEED_1000HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex1000:
            speed_s = SHOW_STATUS_SPEED_1000FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex10g:
            speed_s = SHOW_STATUS_SPEED_10G_HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10g:
            speed_s = SHOW_STATUS_SPEED_10G_FULL;
            break;

        case VAL_portSpeedDpxCfg_halfDuplex40g:
            speed_s = SHOW_STATUS_SPEED_40G_HALF;
            break;

        case VAL_portSpeedDpxCfg_fullDuplex40g:
            speed_s = SHOW_STATUS_SPEED_40G_FULL;
            break;

        default:
            speed_s=0;
            break;
        }

        /*nego state*/
        switch(port_info.autoneg_state)
        {
        case VAL_portAutonegotiation_enabled:
            autoneg_state = SHOW_STATUS_ENABLE;
            break;

        case VAL_portAutonegotiation_disabled:
            autoneg_state = SHOW_STATUS_DISABLE;
            break;

        default:
            autoneg_state=0;
            break;
        }

        /* flow control config*/
        switch(port_info.flow_control_cfg)
        {
        case VAL_portFlowCtrlCfg_enabled:
            flowcontrol_c = SHOW_STATUS_ENABLE;
            break;

        case VAL_portFlowCtrlCfg_disabled:
            flowcontrol_c = SHOW_STATUS_DISABLE;
            break;

        default:
            flowcontrol_c=0;
            break;
        }

        /* flow control current status*/
        switch(port_info.flow_control_oper)
        {
        case VAL_portFlowCtrlStatus_error:
            flowcontrol_s = SHOW_STATUS_ERROR;
            break;

        case VAL_portFlowCtrlStatus_backPressure:
            flowcontrol_s = SHOW_STATUS_BACK_PRESSURE;
            break;

        case VAL_portFlowCtrlStatus_dot3xFlowControl:
            flowcontrol_s = SHOW_STATUS_DOT3X;
            break;

        case VAL_portFlowCtrlStatus_none:
            flowcontrol_s = SHOW_STATUS_NONE;
            break;

        default:
            flowcontrol_s=0;
            break;
        }

        /* broadcast storm */
        if (port_info.bsctrl_state == VAL_bcastStormStatus_enabled)
            broadcast = SHOW_STATUS_ENABLE;
        else
            broadcast = SHOW_STATUS_DISABLE;

        /* broadcast storm mode */
        switch(port_info.bcast_rate_mode)
        {
        case VAL_bcastStormSampleType_pkt_rate:
            storm_mode_type = SHOW_STATUS_PACKET;
            break;

        case VAL_bcastStormSampleType_octet_rate:
            storm_mode_type = SHOW_STATUS_RATE;
            break;

        case VAL_bcastStormSampleType_percent:
            storm_mode_type = SHOW_STATUS_PERCENT;
            break;

        default:
            storm_mode_type =0;
            break;
        }

#if ( SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
        /* multicast storm */
        if (port_info.msctrl_state == VAL_mcastStormStatus_enabled)
            multicast = SHOW_STATUS_ENABLE;
        else
            multicast = SHOW_STATUS_DISABLE;

        /* multicast storm mode */
        switch(port_info.mcast_rate_mode)
        {
        case VAL_mcastStormSampleType_pkt_rate:
            mcast_storm_mode = SHOW_STATUS_PACKET;
            break;

        case VAL_mcastStormSampleType_octet_rate:
            mcast_storm_mode = SHOW_STATUS_RATE;
            break;

        case VAL_mcastStormSampleType_percent:
            mcast_storm_mode = SHOW_STATUS_PERCENT;
            break;

        default:
            break;
        }
#endif /*#if ( SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)*/

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
        /* unicast storm */
        if (port_info.unknown_usctrl_state == VAL_unknownUcastStormStatus_enabled)
            unicast = SHOW_STATUS_ENABLE;
        else
            unicast = SHOW_STATUS_DISABLE;

        /* unicast storm mode */
        switch(port_info.unknown_ucast_rate_mode)
        {
        case VAL_unknownUcastStormSampleType_packet_rate:
            unicast_storm_mode = SHOW_STATUS_PACKET;
            break;

        case VAL_unknownUcastStormSampleType_octet_rate:
            unicast_storm_mode = SHOW_STATUS_RATE;
            break;

        case VAL_unknownUcastStormSampleType_percent:
            unicast_storm_mode = SHOW_STATUS_PERCENT;
            break;

        default:
            break;
        }
#endif /*#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)*/

        /* port type */
        switch(port_info.port_type)
        {
        case VAL_portType_other:
            porttype = SHOW_STATUS_TYPE_OTHER;
            break;

        case VAL_portType_hundredBaseTX:
            porttype = SHOW_STATUS_TYPE_100TX;
            break;

        case VAL_portType_hundredBaseFX:
            porttype = SHOW_STATUS_TYPE_100FX;
            break;

        case VAL_portType_thousandBaseSX:
            porttype = SHOW_STATUS_TYPE_1000SX;
            break;

        case VAL_portType_thousandBaseLX:
            porttype = SHOW_STATUS_TYPE_1000LX;
            break;

        case VAL_portType_thousandBaseT:
            porttype = SHOW_STATUS_TYPE_1000T;
            break;

        case VAL_portType_thousandBaseGBIC:
            porttype = SHOW_STATUS_TYPE_1000GBIC;
            break;

        case VAL_portType_thousandBaseSfp:
            porttype = SHOW_STATUS_TYPE_SFP;
            break;

        case VAL_portType_tenG:
            porttype = SHOW_STATUS_TYPE_TenG;
            break;

        case VAL_portType_tenGBaseT:
            porttype = SHOW_STATUS_TYPE_TenG_COPPER;
            break;

        case VAL_portType_tenGBaseXFP:
            porttype = SHOW_STATUS_TYPE_TenG_XFP;
            break;

        case VAL_portType_tenGBaseSFP:
            porttype = SHOW_STATUS_TYPE_TenG_SFP;
            break;

        case VAL_portType_fortyGBaseQSFP:
            porttype = SHOW_STATUS_TYPE_FortyG_QSFP;
            break;

        case VAL_portType_hundredBaseFxScSingleMode:
           porttype = SHOW_STATUS_TYPE_FxSc_SINGLE_MODE;
           break;

        case VAL_portType_hundredBaseFxScMultiMode:
           porttype = SHOW_STATUS_TYPE_FxSc_MULTI_MODE;
           break;

        default:
            porttype=0;
            break;
        }

        /*capability*/
        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap10half) == SYS_VAL_portCapabilities_portCap10half)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1, "10half");
            else
            {
             strcat((char *)capability1, ",");
             strcat((char *)capability1, " 10half");
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap10full) == SYS_VAL_portCapabilities_portCap10full)
        {
           if (strlen((char *)capability1) == 0)
            strcat((char *)capability1,"10full");
           else
           {
            strcat((char *)capability1,",");
            strcat((char *)capability1," 10full");
           }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap100half) == SYS_VAL_portCapabilities_portCap100half)
        {
           if (strlen((char *)capability1) == 0)
            strcat((char *)capability1,"100half");
           else
           {
            strcat((char *)capability1,",");
            strcat((char *)capability1," 100half");
           }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap100full) == SYS_VAL_portCapabilities_portCap100full)
        {
           if (strlen((char *)capability1) == 0)
            strcat((char *)capability1,"100full");
           else
           {
            strcat((char *)capability1,",");
            strcat((char *)capability1," 100full");
           }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap1000half) == SYS_VAL_portCapabilities_portCap1000half)
        {
           if (strlen((char *)capability1) == 0)
            strcat((char *)capability1,"1000half");
           else
           {
            strcat((char *)capability1,",");
            strcat((char *)capability1," 1000half");
           }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap1000full) == SYS_VAL_portCapabilities_portCap1000full)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1,"1000full");
            else
            {
             strcat((char *)capability1,",");
             strcat((char *)capability1," 1000full");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }
        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap10gHalf) == SYS_VAL_portCapabilities_portCap10gHalf)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1,"10Ghalf");
            else
            {
             strcat((char *)capability1,",");
             strcat((char *)capability1," 10Ghalf");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap10gFull) == SYS_VAL_portCapabilities_portCap10gFull)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1,"10Gfull");
            else
            {
             strcat((char *)capability1,",");
             strcat((char *)capability1," 10Gfull");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap40gHalf) == SYS_VAL_portCapabilities_portCap40gHalf)
        {
            if (strlen((char *)capability1) == 0)
                strcat((char *)capability1,"40Ghalf");
            else
            {
                strcat((char *)capability1,",");
                strcat((char *)capability1," 40Ghalf");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap40gFull) == SYS_VAL_portCapabilities_portCap40gFull)
        {
            if (strlen((char *)capability1) == 0)
                strcat((char *)capability1,"40Gfull");
            else
            {
                strcat((char *)capability1,",");
                strcat((char *)capability1," 40Gfull");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCap100gFull) == SYS_VAL_portCapabilities_portCap100gFull)
        {
            if (strlen((char *)capability1) == 0)
                strcat((char *)capability1,"100Gfull");
            else
            {
                strcat((char *)capability1,",");
                strcat((char *)capability1," 100Gfull");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCapSym) == SYS_VAL_portCapabilities_portCapSym)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1,"symmetric");
            else
            {
             strcat((char *)capability1,",");
             strcat((char *)capability1," symmetric");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if ((port_info.autoneg_capability & SYS_VAL_portCapabilities_portCapFlowCtrl) == SYS_VAL_portCapabilities_portCapFlowCtrl)
        {
            if (strlen((char *)capability1) == 0)
             strcat((char *)capability1,"flowcontrol");
            else
            {
             strcat((char *)capability1,",");
             strcat((char *)capability1," flowcontrol");
            }

            if (strlen((char *)capability1) > 40)
            {
                strcpy((char *)capability2,(char *)capability1);
                memset(capability1,0,sizeof(capability1));
            }
        }

        if (capability2[0] == 0)
        {
            strcpy((char *)capability2,(char *)capability1);
            memset(capability1,0,sizeof(capability1));
        }

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
        /*combo-forced-mode*/
        {
            switch(port_info.medium_forced_mode)
            {
            case VAL_portComboForcedMode_copperForced:
               combo_mode = SHOW_STATUS_COMBO_FORCED_MODE_COPPER_FORCED;
               break;

/*pttch 2004/8/5 04:20 do not support this mode*/
#if 0
            case VAL_portComboForcedMode_copperPreferredAuto:
               combo_mode = SHOW_STATUS_COMBO_FORCED_MODE_COPPER_PREFERRED_AUTO;
               break;
#endif

            case VAL_portComboForcedMode_sfpForced:
               combo_mode = SHOW_STATUS_COMBO_FORCED_MODE_SFP_FORCED;
               break;

            case VAL_portComboForcedMode_sfpPreferredAuto:
               combo_mode = SHOW_STATUS_COMBO_FORCED_MODE_SFP_PREFERRED_AUTO;
               break;

            case VAL_portComboForcedMode_none:
            default:
               combo_mode = SHOW_STATUS_COMBO_FORCED_MODE_NONE;
               break;
            }
        }
#endif

        /*giga-phy-mode*/
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)
        {
            switch(port_info.forced_1000t_mode)
            {
                case VAL_portMasterSlaveModeCfg_master:
                    force_mode_1000full = SHOW_STATUS_COMBO_GIGA_PHY_MODE_MASTER;
                break;

                case VAL_portMasterSlaveModeCfg_slave:
                    force_mode_1000full = SHOW_STATUS_COMBO_GIGA_PHY_MODE_SLAVE;
                break;

                default:
                    force_mode_1000full = SHOW_STATUS_COMBO_GIGA_PHY_MODE_NONE;
                break;
            }
        }
#endif
#if (SYS_CPNT_LACP==TRUE)
        /*lacp */
        if (LACP_POM_GetRunningDot3adLacpPortEnabled(lport, &lacp_state) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
        {
            switch(lacp_state)
            {
            case VAL_lacpPortStatus_enabled:
                lacp_ptr = SHOW_STATUS_ENABLE;
                break;

             case VAL_lacpPortStatus_disabled:
                lacp_ptr = SHOW_STATUS_DISABLE;
                break;

             default:
                lacp_ptr=0;
                 break;
             }
        }
#endif
        if(port_type == PORT_TYPE_TRUNK)
        {
            if(TRK_PMGR_IsDynamicTrunkId(trunk_id))
                trunktype = SHOW_STATUS_TRUNK_TYPE_LACP;
            else
                trunktype = SHOW_STATUS_TRUNK_TYPE_USER;
        }


        /* Basic information
         * i.e. native characteristics
         */
        sprintf(buff," Basic Information: \r\n");
        PROCESS_MORE_FUNC(buff);

        /*trunk member*/
        if(port_type == PORT_TYPE_ETH)
        {
           UI32_T unit;
           UI32_T port;
           UI32_T trunk_id;

           if(SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
           {
              UI32_T trk_id;

              if( SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToTrunkPort(unit, port, &trk_id))
              {
                return FALSE;
              }
              if(TRK_PMGR_IsDynamicTrunkId(trk_id))
                 sprintf(buff, "  Member port of trunk %lu was created by LACP.\r\n", (unsigned long)trk_id);
              else
                 sprintf(buff, "  Member port of trunk %lu was created by user.\r\n", (unsigned long)trk_id);

              PROCESS_MORE_FUNC(buff);
           }
        }

        /*port type*/
#if (SYS_CPNT_VDSL == TRUE)
        if (port_info.ethertype == VAL_portEthernetType_longReachEthernet)
        {
        sprintf(buff, "  Port Type                   : %s", str[porttype]);
           strcat(buff,"-LRE\r\n");
           PROCESS_MORE_FUNC(buff);
        }
        else
#endif
        if (SWDRV_LIB_MapPortType(port_info.port_type, sizeof(port_type_str), port_type_str))
        {
        sprintf(buff, "  Port Type                   : %s\r\n", port_type_str);
           PROCESS_MORE_FUNC(buff);
        }
        else
        {
        sprintf(buff, "  Port Type                   : %s\r\n", str[porttype]);
           PROCESS_MORE_FUNC(buff);
        }

        /*MAC address*/
        sprintf(buff, "  MAC Address                 : %02X-%02X-%02X-%02X-%02X-%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        PROCESS_MORE_FUNC(buff);


        /* configuration from the user
         * i.e. user specify issues
         */
        sprintf(buff," Configuration: \r\n");
        PROCESS_MORE_FUNC(buff);

        /*name*/
#if 0
        sprintf(buff, "  Name                        : %s\r\n", port_info.port_name);
        PROCESS_MORE_FUNC(buff);
#endif
        is_1st_line = TRUE;
        port_name_char_index = 0;
        port_name_len = strlen((char *)port_info.port_name);

        if (port_name_len == 0)
        {
            PROCESS_MORE_FUNC("  Name                        : \r\n");
        }

        while (port_name_len > 0)
        {
            if (port_name_len >= PORT_NAME_LEN_IN_ONE_LINE)
            {
                memcpy(port_name_buff, port_info.port_name + port_name_char_index, PORT_NAME_LEN_IN_ONE_LINE);
                port_name_len -= PORT_NAME_LEN_IN_ONE_LINE;
                port_name_char_index += PORT_NAME_LEN_IN_ONE_LINE;
            }
            else
            {
                memcpy(port_name_buff, port_info.port_name + port_name_char_index, port_name_len);
                port_name_buff[port_name_len] = '\0';
                port_name_len -= port_name_len;
            }
            if (is_1st_line == TRUE)
            {
                sprintf(buff, "  Name                        : %s\r\n", port_name_buff);
                PROCESS_MORE_FUNC(buff);
                is_1st_line = FALSE;
            }
            else
            {
                sprintf(buff, "                                %s\r\n", port_name_buff);
                PROCESS_MORE_FUNC(buff);
            }
        }

        /*Admin status*/
        sprintf(buff, "  Port Admin                  : %s\r\n", str[admin_status]);
        PROCESS_MORE_FUNC(buff);

        /*negoation*/
        //sprintf((char*)buff, "  Negotiation status: %s\r\n", str[autoneg_state]);
        //PROCESS_MORE_FUNC(buff);

        /*force mode speed-duplex*/
        if(port_info.autoneg_state == VAL_portAutonegotiation_enabled)
        sprintf(buff, "  Speed-duplex                : %s\r\n", "Auto");
        else if (SWDRV_LIB_MapSpeedDuplex(port_info.speed_duplex_cfg, sizeof(speed_duplex_str), speed_duplex_str))
        sprintf(buff, "  Speed-duplex                : %s\r\n", speed_duplex_str);
        else
        sprintf(buff, "  Speed-duplex                : %s\r\n", str[speed_c]);
        PROCESS_MORE_FUNC(buff);

        /*capability*/
        if (port_ability.port_autoneg_supported & BIT_VALUE(VAL_portAutonegotiation_enabled))
        {
        sprintf(buff, "  Capabilities                : %s\r\n", capability2);
        PROCESS_MORE_FUNC(buff);
        if (capability1[0] != 0)
        {
        sprintf(buff, "                          %s\r\n", capability1);
            PROCESS_MORE_FUNC(buff);
        }
        }

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
        /*fec*/
        if (port_type == PORT_TYPE_ETH)
        {
            char *fec_mode_str = "Unknown";

            switch (port_info.port_fec_mode)
            {
                case VAL_portFecMode_disabled:
                    fec_mode_str = "Disabled";
                    break;
                case VAL_portFecMode_baseR:
                    fec_mode_str = "BASE-R FEC";
                    break;
                case VAL_portFecMode_rs:
                    fec_mode_str = "RS-FEC";
                    break;
            }

            sprintf(buff,"  FEC Mode                    : %s\r\n", fec_mode_str);
            PROCESS_MORE_FUNC(buff);
        }
#endif

        /*broadcast*/
#if (SYS_CPNT_BSTORM_SUPPORT_LPORT != TRUE)
        if (port_type == PORT_TYPE_ETH)
#endif
        {
            sprintf(buff,"  Broadcast Storm             : %s\r\n", str[broadcast]);
            PROCESS_MORE_FUNC(buff);
#if defined(STRAWMAN)
            sprintf(buff,"  Broadcast Storm Limit       : %lu %s\r\n", (unsigned long)port_info.bcast_rate_limit, "packets in queue");
            PROCESS_MORE_FUNC(buff);
#else
            sprintf(buff,"  Broadcast Storm Limit       : %lu %s\r\n", (unsigned long)port_info.bcast_rate_limit, str[storm_mode_type]);
            PROCESS_MORE_FUNC(buff);
#endif
        }

#if ( SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
        /*multicast*/
#if (SYS_CPNT_MSTORM_SUPPORT_LPORT != TRUE)
        if (port_type == PORT_TYPE_ETH)
#endif
        {
            sprintf(buff,"  Multicast Storm             : %s\r\n", str[multicast]);
                PROCESS_MORE_FUNC(buff);
#if defined(STRAWMAN)
            sprintf(buff,"  Multicast Storm Limit       : %lu %s\r\n", (unsigned long)port_info.mcast_rate_limit, "packets in queue");
                PROCESS_MORE_FUNC(buff);
#else
            sprintf(buff,"  Multicast Storm Limit       : %lu %s\r\n", (unsigned long)port_info.mcast_rate_limit, str[mcast_storm_mode]);
                PROCESS_MORE_FUNC(buff);
#endif
        }
#endif /*#if ( SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)*/

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
        /*unicast*/
#if (SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT != TRUE)
        if (port_type == PORT_TYPE_ETH)
#endif
        {
            sprintf(buff,"  Unknown Unicast Storm       : %s\r\n", str[unicast]);
                PROCESS_MORE_FUNC(buff);
#if defined(STRAWMAN)
            sprintf(buff,"  Unknown Unicast Storm Limit : %lu %s\r\n", (unsigned long)port_info.unknown_ucast_rate_limit, "packets in queue");
                PROCESS_MORE_FUNC(buff);
#else
            sprintf(buff,"  Unknown Unicast Storm Limit : %lu %s\r\n", (unsigned long)port_info.unknown_ucast_rate_limit, str[unicast_storm_mode]);
                PROCESS_MORE_FUNC(buff);
#endif
        }
#endif /*#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)*/


/*EPR:ES4827G-FLF-ZZ-00372
  *Problem:storm information shown in trunk was not correctly.
  *              another proble:trunk broadcast information didn't
  *              synced to new added normal port.
  *RootCause:we didn't show trunk storm information before.
  *                   we didn't  synce broadcast database operation
  *                   in setting new port to trunk
  *Solution:get correct trunk_number,show storm information for trunk.
  *Changed file:cli_api_show.c,swctrl.c
  *Approved by:Hard Sun
  *Fixed by:Jinhua Wei
*/

#if (TRUE == SYS_CPNT_FLOW_CONTROL)
        /*flow control*/
        sprintf(buff,"  Flow Control                : %s\r\n", str[flowcontrol_c]);
        PROCESS_MORE_FUNC(buff);
#endif

#if (SYS_CPNT_LACP==TRUE)
        if (port_type == PORT_TYPE_ETH)
        {
            sprintf(buff,"  LACP                        : %s\r\n", str[lacp_ptr]);
            PROCESS_MORE_FUNC(buff);
        }
#endif

#if (SYS_CPNT_AMTR_PORT_MAC_LEARNING == TRUE)
         /*mac-learning*/
         sprintf((char*)buff,"  MAC Learning                : %s\r\n", port_info.port_macaddr_learning?"Enabled":"Disabled");
         PROCESS_MORE_FUNC(buff);
#endif

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE)
        {
            /* link-up-down trap
             */
            IF_MGR_IfXEntry_T  if_x_entry;

            memset(&if_x_entry, 0, sizeof(if_x_entry));
            if_x_entry.if_index = lport;

            if (TRUE == IF_PMGR_GetIfXEntry(&if_x_entry))
            {
                sprintf((char*)buff,"  Link-up-down Trap           : %s\r\n",
                    (if_x_entry.if_link_up_down_trap_enable == VAL_ifLinkUpDownTrapEnable_enabled)?
                    ("Enabled"):("Disabled"));
                PROCESS_MORE_FUNC(buff);
            }
        }
#endif /* #if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == TRUE) */

#if (SYS_CPNT_PORT_SECURITY == TRUE) && 0 /* these code are not needed by refined psec */
        {
           UI32_T  port_security_status;
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
           UI32_T  port_security_a = SHOW_STATUS_ERROR;  /*added by Jinhua Wei,because it is only used in this macro and here it is false*/
#endif
           UI32_T  port_security_s;
           UI32_T type;
           UI32_T unit;
           UI32_T port;
           UI32_T trunk_id;

           type = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);
           /* If is trunk member, don't display port security */
           if ((type != SWCTRL_LPORT_TRUNK_PORT_MEMBER) && (type != SWCTRL_LPORT_TRUNK_PORT))
           {
                /*port security status*/

                /*
                EPR:       ES3628BT-FLF-ZZ-00020
                Problem:   LACP member port will display port security status as 'ERROR'
                rootcasue: System can not get running status of port seurity and will display 'error' if the port is trunk member.
                sloution:  display the status as 'disable' if get running status of port security failed.
                File:      cli_api_show.c
                */
                /* if get running status fail, display status as 'disable' */
                if (!PSEC_PMGR_GetPortSecurityStatus( lport, &port_security_status))
                port_security_s = SHOW_STATUS_DISABLE;
                else
                {
                switch(port_security_status)
                    {
                    case VAL_portSecPortStatus_disabled:
                        port_security_s = SHOW_STATUS_DISABLE;
                        break;

                    case VAL_portSecPortStatus_enabled:
                        port_security_s = SHOW_STATUS_ENABLE;
                        break;

                    default:
                        port_security_s = 0;
                        break;
                    }
                }

               sprintf(buff,"  Port Security               : %s\r\n", str[port_security_s]);
               PROCESS_MORE_FUNC(buff);

#if (CLI_SUPPORT_PSEC_MAC_COUNT == 1)
               {
                    UI32_T count =0;

                    PSEC_PMGR_GetPortSecurityMacCount(lport, &count);
                    sprintf(buff,"  Max MAC Count               : %lu\r\n", (unsigned long)count);
                  PROCESS_MORE_FUNC(buff);
               }
#endif

               /*port security action shutdown status*/
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)

                  if (PSEC_PMGR_GetPortSecurityActionStatus( lport, &port_security_status))
                  {
                     switch(port_security_status)
                     {
                     case VAL_portSecAction_none:
                        port_security_a = SHOW_STATUS_PSEC_ACTION_NONE;
                        break;

                     case VAL_portSecAction_trap:
                        port_security_a = SHOW_STATUS_PSEC_ACTION_TRAP;
                        break;

                     case VAL_portSecAction_shutdown:
                        port_security_a = SHOW_STATUS_PSEC_ACTION_SHUTDOWN;
                        break;

                     case VAL_portSecAction_trapAndShutdown:
                        port_security_a = SHOW_STATUS_PSEC_ACTION_TRAPANDSHUTDOWN;
                        break;

                     default:
                        port_security_a=0;
                        break;
                     }
                  }

                    sprintf(buff,"  Port Security Action        : %s\r\n", str[port_security_a]);
                    PROCESS_MORE_FUNC(buff);

#endif
            }
        }
#endif

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
        if (port_type == PORT_TYPE_ETH)
        {
            sprintf(buff,"  Media Type                  : %s\r\n", str[combo_mode]);
            PROCESS_MORE_FUNC(buff);
        }
#endif

#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE==TRUE)
        if (port_type == PORT_TYPE_ETH ||port_type == PORT_TYPE_TRUNK)
        {
           sprintf(buff,"  Giga PHY mode               : %s\r\n", str[force_mode_1000full]);
           PROCESS_MORE_FUNC(buff);
        }
#endif

#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_PER_PORT )
           sprintf(buff,"  MTU                         : %lu\r\n", (unsigned long)port_info.mtu);
           PROCESS_MORE_FUNC(buff);
#endif

#if (SYS_CPNT_TRUNK_MAX_ACTIVE_PORTS_CONFIGURABLE == TRUE)
        if (port_type == PORT_TYPE_TRUNK)
        {
            UI32_T max_num_of_active_ports;

            if (SWCTRL_POM_GetTrunkMaxNumOfActivePorts(trunk_id, &max_num_of_active_ports))
            {
                sprintf(buff,"  Max Active Ports            : %lu\r\n", (unsigned long)max_num_of_active_ports);
                PROCESS_MORE_FUNC(buff);
            }
        }
#endif

        /* Current status not from the user
         * i.e. meybe from negoation
         */
        sprintf(buff," Current Status: \r\n");
        PROCESS_MORE_FUNC(buff);

        if (port_type == PORT_TYPE_TRUNK)
        {
        sprintf(buff,"  Created By                  : %s\r\n", str[trunktype]);
            PROCESS_MORE_FUNC(buff);
        }

        /*link status*/
        sprintf(buff,"  Link Status                 : %s\r\n", str[link_status]);
        PROCESS_MORE_FUNC(buff);

        /* shudown reason
         */
        if (port_info.link_status == SWCTRL_LINK_DOWN)
        {
            char *prefix;
            int i, len;

            if (port_info.shutdown_reason)
            {
                len = 0;
                prefix = "  Link Down Reason            : ";

                for (i = 0; i < sizeof(cli_api_show_shutdown_reasons)/sizeof(*cli_api_show_shutdown_reasons); i++)
                {
                    if (port_info.shutdown_reason & cli_api_show_shutdown_reasons[i].val)
                    {
                        if (len + strlen(prefix) + strlen(cli_api_show_shutdown_reasons[i].desc) > 79)
                        {
                            sprintf(buff + len, ",\r\n");
                            PROCESS_MORE_FUNC(buff);
                            len = 0;
                            prefix = "                           ";
                        }

                        len += sprintf(buff + len, "%s%s", prefix, cli_api_show_shutdown_reasons[i].desc);
                        prefix = ", ";
                    }
                }

                sprintf(buff + len, "\r\n");
                PROCESS_MORE_FUNC(buff);
            }
        }

        /*oper status*/
        if(port_info.link_status == SWCTRL_LINK_UP)
        {
        sprintf(buff,"  Port Operation Status       : %s\r\n", str[oper_status]);
           PROCESS_MORE_FUNC(buff);
        }

        /*oper speed-duplex*/
        if (SWDRV_LIB_MapSpeedDuplex(port_info.speed_duplex_oper, sizeof(speed_duplex_str), speed_duplex_str))
        sprintf(buff,"  Operation Speed-duplex      : %s\r\n", speed_duplex_str);
        else
        sprintf(buff,"  Operation Speed-duplex      : %s\r\n", str[speed_s]);
        PROCESS_MORE_FUNC(buff);

        /*port up time*/
        if (port_info.link_status == SWCTRL_LINK_UP)
        {
            UI32_T all_seconds;
            UI32_T weeks, days, hours, minutes, seconds, milliseconds;

            all_seconds = port_info.uptime / SYS_BLD_TICKS_PER_SECOND;

            SYS_TIME_ConvertTicksToDiffTime(port_info.uptime, &weeks, &days, &hours, &minutes, &seconds, &milliseconds);

            sprintf(buff, "  Up Time                     : %luw %lud %luh %lum %lus (%lu seconds)\r\n",
                                 (unsigned long)weeks, (unsigned long)days, (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds, (unsigned long)all_seconds);
            PROCESS_MORE_FUNC(buff);
        }

#if (SYS_CPNT_SWCTRL_FEC == TRUE)
        /*fec*/
        if (port_type == PORT_TYPE_ETH)
        {
            char *fec_mode_str = "Unknown";

            switch (port_info.port_fec_status)
            {
                case VAL_portFecMode_disabled:
                    fec_mode_str = "Disabled";
                    break;
                case VAL_portFecMode_baseR:
                    fec_mode_str = "BASE-R FEC";
                    break;
                case VAL_portFecMode_rs:
                    fec_mode_str = "RS-FEC";
                    break;
            }

            sprintf(buff,"  FEC Mode                    : %s\r\n", fec_mode_str);
            PROCESS_MORE_FUNC(buff);
        }
#endif

#if (TRUE == SYS_CPNT_FLOW_CONTROL)
        /*oper flowcontrol*/
        sprintf(buff,"  Flow Control Type           : %s\r\n", str[flowcontrol_s]);
        PROCESS_MORE_FUNC(buff);
#endif

#ifdef ASF4526B_FLF_P5
        if(STKTPLG_POM_DetectSfpInstall(user_unit, user_port, &is_present) == TRUE)
        {
            if(is_present == TRUE)
            {
                sprintf(buff,"  SFP Present 		      : yes\r\n");
            }
            else
            {
                sprintf(buff,"  SFP Present 		      : no\r\n");
            }
            PROCESS_MORE_FUNC(buff);
        }
#endif

        /* MDIX mode status */
#if ( SYS_CPNT_SWCTRL_MDIX_CONFIG == TRUE )
        sprintf(buff, "  Mdix Mode                   : %s\r\n", str[mdix_status]);
        PROCESS_MORE_FUNC(buff);
#endif

        sprintf(buff, "  Max Frame Size              : %lu bytes (%lu bytes for tagged frames)\r\n", (unsigned long)port_info.untagged_max_frame_sz, (unsigned long)port_info.tagged_max_frame_sz);
        PROCESS_MORE_FUNC(buff);

        /* learning status
         */
        {
            UI32_T learning_disabled_status;
            UI32_T intruder_handlers;
            char *learning_status_str = "Enabled";

            if (SWCTRL_POM_GetPortLearningStatusEx(lport, &learning_disabled_status, &intruder_handlers))
            {
                if (intruder_handlers)
                {
                    learning_status_str = "Secure";
                }
                else if (learning_disabled_status)
                {
                    if (learning_disabled_status & SWCTRL_LEARNING_DISABLED_BY_RSPAN)
                    {
                        learning_status_str = "Disabled by RSPAN";
                    }
                    else
                    {
                        learning_status_str = "Disabled";
                    }
                }

                sprintf(buff, "  MAC Learning Status         : %s\r\n", learning_status_str);
                PROCESS_MORE_FUNC(buff);
            }
        }
    }
    else
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to get port info, lport = %lu\r\n", (unsigned long)lport);
#endif
        line_num += 1;
    }
    return line_num;
}

static UI32_T Show_one_vlan_interface(UI32_T vid, UI32_T line_num)
{
   char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T vid_ifindex;
   UI8_T  vlan_mac[6] = {0};

   VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);

   if(VLAN_PMGR_GetVlanMac(vid_ifindex, vlan_mac))
   {
        sprintf(buff, "  MAC Address                 : %02X-%02X-%02X-%02X-%02X-%02X\r\n", vlan_mac[0], vlan_mac[1], vlan_mac[2],
                                                                       vlan_mac[3], vlan_mac[4], vlan_mac[5]);
       PROCESS_MORE_FUNC(buff);
   }
   else
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_1("Failed to get VLAN%lu info \r\n",(unsigned long)vid);
#endif
      return JUMP_OUT_MORE;
   }
   return line_num;
}


#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
static UI32_T CLI_API_SHOW_ShowOneInterfaceTransceiverThreshold(UI32_T unit, UI32_T lport, UI32_T line_num)
{
    SWCTRL_OM_SfpDdmThresholdEntry_T sfp_ddm_threshold_entry;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff_t[CLI_DEF_MAX_BUFSIZE] = {0};
    SWCTRL_OM_SfpInfo_T sfp_info;
    BOOL_T is_present = FALSE;

    memset(&sfp_info, 0, sizeof(SWCTRL_OM_SfpInfo_T));
    if (SWCTRL_POM_GetPortSfpPresent(unit, lport, &is_present) == FALSE ||
       is_present == FALSE ||
       SWCTRL_POM_GetPortSfpInfo(unit, lport, &sfp_info) == FALSE )
    {
        sprintf(buff,"  No SFP inserted\r\n");
        PROCESS_MORE_FUNC(buff);
        return line_num;
    }

    /* Get Sfp ddm info */
    SWCTRL_POM_GetPortSfpDdmThresholdEntry(lport, &sfp_ddm_threshold_entry);
    sprintf(buff," DDM Thresholds\r\n");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff," Transceiver-monitor        : %s\r\n", (sfp_ddm_threshold_entry.trap_enable==TRUE)?"Enabled":"Disabled");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff," Transceiver-threshold-auto : %s\r\n", (sfp_ddm_threshold_entry.auto_mode==TRUE)?"Enabled":"Disabled");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "   %-20s  %12s  %12s  %12s  %12s\r\n", "", "Low Alarm", "Low Warning", "High Warning", "High Alarm");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "   %-20s  %12s  %12s  %12s  %12s\r\n", "-----------", "------------", "------------", "------------", "------------");
    PROCESS_MORE_FUNC(buff);

    /* show temperature informations */
    sprintf(buff, "   %-20s   %11.2f  %11.2f  %11.2f  %11.2f\r\n","Temperature(Celsius)",
                        sfp_ddm_threshold_entry.threshold.temp_low_alarm,
                        sfp_ddm_threshold_entry.threshold.temp_low_warning,
                        sfp_ddm_threshold_entry.threshold.temp_high_warning,
                        sfp_ddm_threshold_entry.threshold.temp_high_alarm);
    PROCESS_MORE_FUNC(buff);

    /* show voltage informations */
    sprintf(buff, "   %-20s   %11.2f  %11.2f  %11.2f  %11.2f\r\n", "Voltage(Volts)",
                        sfp_ddm_threshold_entry.threshold.voltage_low_alarm,
                        sfp_ddm_threshold_entry.threshold.voltage_low_warning,
                        sfp_ddm_threshold_entry.threshold.voltage_high_warning,
                        sfp_ddm_threshold_entry.threshold.voltage_high_alarm);
    PROCESS_MORE_FUNC(buff);

    /* show Tx bias current informations */
    sprintf(buff, "   %-20s   %11.2f  %11.2f  %11.2f  %11.2f\r\n", "Current(mA)",
                        sfp_ddm_threshold_entry.threshold.bias_low_alarm,
                        sfp_ddm_threshold_entry.threshold.bias_low_warning,
                        sfp_ddm_threshold_entry.threshold.bias_high_warning,
                        sfp_ddm_threshold_entry.threshold.bias_high_alarm);
    PROCESS_MORE_FUNC(buff);

    /* show Tx power informations, convert unit from watts to dBm */
    if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP ||
       sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP )
    {
        sprintf(buff_t, "   %-20s", "TxPower(dBm)");
        strcat(buff, buff_t);

        sprintf(buff_t, "   %11.2f", sfp_ddm_threshold_entry.threshold.tx_power_low_alarm);
        strcat(buff, buff_t);

        sprintf(buff_t, "  %11.2f", sfp_ddm_threshold_entry.threshold.tx_power_low_warning);
        strcat(buff, buff_t);

        sprintf(buff_t, "  %11.2f", sfp_ddm_threshold_entry.threshold.tx_power_high_warning);
        strcat(buff, buff_t);

        sprintf(buff_t, "  %11.2f\r\n", sfp_ddm_threshold_entry.threshold.tx_power_high_alarm);
        strcat(buff, buff_t);
        PROCESS_MORE_FUNC(buff);
    }

    /* show Rx power informations, convert unit from watts to dBm */
    sprintf(buff_t, "   %-20s", "RxPower(dBm)");
    strcat(buff, buff_t);

    sprintf(buff_t, "   %11.2f", sfp_ddm_threshold_entry.threshold.rx_power_low_alarm);
    strcat(buff, buff_t);

    sprintf(buff_t, "  %11.2f", sfp_ddm_threshold_entry.threshold.rx_power_low_warning);
    strcat(buff, buff_t);

    sprintf(buff_t, "  %11.2f", sfp_ddm_threshold_entry.threshold.rx_power_high_warning);
    strcat(buff, buff_t);

    sprintf(buff_t, "  %11.2f\r\n", sfp_ddm_threshold_entry.threshold.rx_power_high_alarm);
    strcat(buff, buff_t);
    PROCESS_MORE_FUNC(buff);

    return line_num;
}
#endif/* End of #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
static UI32_T CLI_API_SHOW_ShowOneInterfaceTransceiver(UI32_T unit, UI32_T lport, UI32_T sfp_index, UI32_T line_num)
{
    SWCTRL_OM_SfpInfo_T sfp_info;
    SWCTRL_OM_SfpDdmInfoMeasured_T    sfp_ddm_info_measured;
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    SWCTRL_OM_SfpDdmThreshold_T       sfp_ddm_threshold;
    SWCTRL_OM_SfpDdmThresholdStatus_T sfp_ddm_threshold_status;
#endif
    UI8_T  date_code_year, date_code_month, date_code_day;
    BOOL_T is_present = FALSE;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char   buff_tmp[CLI_DEF_MAX_BUFSIZE] = {0};

    memset(&sfp_info, 0, sizeof(SWCTRL_OM_SfpInfo_T));
    if(SWCTRL_POM_GetPortSfpPresent(unit, sfp_index, &is_present) == FALSE ||
       is_present == FALSE ||
       SWCTRL_POM_GetPortSfpInfo(unit, sfp_index, &sfp_info) == FALSE)
    {
        sprintf(buff,"  No SFP inserted\r\n");
        PROCESS_MORE_FUNC(buff);
        return line_num;
    }

    if (sfp_info.is_invalid)
    {
        if (sfp_info.is_invalid & SWDRV_TYPE_GBIC_INVALID_INFO_READ_ERROR)
        {
            sprintf(buff,"  Read error\r\n");
            PROCESS_MORE_FUNC(buff);
        }
        if ((sfp_info.is_invalid & SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_BASE_ERROR) ||
            (sfp_info.is_invalid & SWDRV_TYPE_GBIC_INVALID_INFO_CHECKSUM_EXT_ERROR))
        {
            sprintf(buff,"  Checksum error\r\n");
            PROCESS_MORE_FUNC(buff);
        }
        return line_num;
    }

    /* Connector Type */
    memset(buff_tmp, 0, CLI_DEF_MAX_BUFSIZE);
    SWDRV_LIB_MapSFPEEPROMConnector(sfp_info.connector, buff_tmp);
    sprintf(buff, " Connector Type        : %s\r\n", buff_tmp);
    PROCESS_MORE_FUNC(buff);

    /* Transmission Media */
    memset(buff_tmp, 0, CLI_DEF_MAX_BUFSIZE);
    if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP)
    {
        SWDRV_LIB_MapSFPEEPROMTransceiverMedia(sfp_info.transceiver[6],
                                               sfp_info.link_length_support_km, buff_tmp);
    }
    else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP)
    {
        SWDRV_LIB_MapXFPEEPROMTransceiverMedia(sfp_info.link_length_support_km, buff_tmp);
    }
    else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP_PLUS)
    {
        SWDRV_LIB_MapQSFPEEPROMTransceiverMedia(sfp_info.transceiver[6],
                                                sfp_info.link_length_support_km, buff_tmp);
    }
    sprintf(buff, " Fiber Type            : %s\r\n", buff_tmp);
    PROCESS_MORE_FUNC(buff);

    /* Ethernet compliance codes */
    memset(buff_tmp, 0, CLI_DEF_MAX_BUFSIZE);
    if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP)
    {
        SWDRV_LIB_MapSFPEEPROMTransceiverCompCode(sfp_info.transceiver[3],
                                                  sfp_info.transceiver[0],
                                                  sfp_info.link_length_support_km,
                                                  sfp_info.link_length_support_100m, buff_tmp);
#if (SYS_CPNT_SFP_INFO_FOR_BROCADE == TRUE)
        SWDRV_LIB_GetCustomizedSfpTransceiverTypeStr(unit, sfp_index, sizeof(buff_tmp), buff_tmp);
#endif
        sprintf(buff, " Eth Compliance Codes  : %s\r\n", buff_tmp);
    }
    else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP)
    {
        SWDRV_LIB_MapXFPEEPROMTransceiverCompCode(sfp_info.transceiver[0],
                                                  sfp_info.link_length_support_km, buff_tmp);
        sprintf(buff, " 10G Eth Compliance    : %s\r\n", buff_tmp);
    }
    else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP ||
            sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP_PLUS)
    {
        SWDRV_LIB_MapQSFPEEPROMTransceiverCompCode(sfp_info.transceiver[0],
                                                   sfp_info.link_length_support_km, buff_tmp);
        sprintf(buff, " 40G Eth Compliance    : %s\r\n", buff_tmp);
    }
    PROCESS_MORE_FUNC(buff);

#if 0 /* Because stktplg om does not have wavelength info */
        /* Wavelength, byte 0 as high order byte, byte 1 as low order byte */
        central_wavelength = (sfp_info.wavelength[0] << 8);

        central_wavelength += sfp_info.wavelength[1];
        sprintf(buff, " Tx Central Wavelength : %d nm\r\n", central_wavelength);
        PROCESS_MORE_FUNC(buff);
#endif

    /* Bit Rate, in unit of 100 MBd (Baud) */
    sprintf(buff, " Baud Rate             : %d MBd\r\n", sfp_info.bitrate * 100);
    PROCESS_MORE_FUNC(buff);

    /* Vendor OUI */
    sprintf(buff, " Vendor OUI            : %02X-%02X-%02X\r\n", sfp_info.vendor_oui[0],
                                                                 sfp_info.vendor_oui[1],
                                                                 sfp_info.vendor_oui[2]);
    PROCESS_MORE_FUNC(buff);

    /* Vendor Name */
    sprintf(buff, " Vendor Name           : %s\r\n", sfp_info.vendor_name);
    PROCESS_MORE_FUNC(buff);

    /* Vendor Part Number */
    sprintf(buff, " Vendor PN             : %s\r\n", sfp_info.vendor_pn);
    PROCESS_MORE_FUNC(buff);

    /* Vendor Revision */
    sprintf(buff, " Vendor Rev            : %s\r\n", sfp_info.vendor_rev);
    PROCESS_MORE_FUNC(buff);

    /* Vendor Serial Number */
    sprintf(buff, " Vendor SN             : %s\r\n", sfp_info.vendor_sn);
    PROCESS_MORE_FUNC(buff);

    /* Date Code, year, month, day */
    date_code_year  = (UI32_T) ((sfp_info.date_code[0]-'0')*10 + (sfp_info.date_code[1]-'0'));
    date_code_month = (UI32_T) ((sfp_info.date_code[2]-'0')*10 + (sfp_info.date_code[3]-'0'));
    date_code_day   = (UI32_T) ((sfp_info.date_code[4]-'0')*10 + (sfp_info.date_code[5]-'0'));

    sprintf(buff, " Date Code             : %02d-%02d-%02d\r\n", date_code_year, date_code_month, date_code_day);
    PROCESS_MORE_FUNC(buff);

    /* Tx-disable */
    sprintf(buff, " Tx-disable Capability : %s\r\n", sfp_info.support_tx_disable ? "Yes":"No");
    PROCESS_MORE_FUNC(buff);

    sprintf(buff," DDM Information\r\n");
    PROCESS_MORE_FUNC(buff);

    memset(&sfp_ddm_info_measured, 0, sizeof(SWCTRL_OM_SfpDdmInfoMeasured_T));
    if(sfp_info.support_ddm == TRUE && SWCTRL_POM_GetPortSfpDdmInfoMeasured(unit, sfp_index, &sfp_ddm_info_measured) != FALSE)
    {
        /* Temperature (celsius) */
        sprintf(buff, "   Temperature         : %.2f degree C\r\n", sfp_ddm_info_measured.temperature);
        PROCESS_MORE_FUNC(buff);

        /* Voltage (Volts) */
        sprintf(buff, "   Vcc                 : %.2f V\r\n", sfp_ddm_info_measured.voltage);
        PROCESS_MORE_FUNC(buff);

        /* Current (mAmpere) */
        if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP ||
           sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP)
        {
            sprintf(buff, "   Bias Current        : %.2f mA\r\n", sfp_ddm_info_measured.tx_bias_current);
            PROCESS_MORE_FUNC(buff);
        }
        else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP ||
                sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP_PLUS)
        {
            sprintf(buff, "   Bias Current(ch 1)  : %.2f mA\r\n", sfp_ddm_info_measured.tx_bias_current);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   Bias Current(ch 2)  : %.2f mA\r\n", sfp_ddm_info_measured.tx_bias_current_2);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   Bias Current(ch 3)  : %.2f mA\r\n", sfp_ddm_info_measured.tx_bias_current_3);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   Bias Current(ch 4)  : %.2f mA\r\n", sfp_ddm_info_measured.tx_bias_current_4);
            PROCESS_MORE_FUNC(buff);
        }

        /* Tx Power (dBm) */
        if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP ||
           sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP )
        {
            sprintf(buff, "   TX Power            : %.2f dBm\r\n", sfp_ddm_info_measured.tx_power);
            PROCESS_MORE_FUNC(buff);
        }

        /* Rx Power (dBm) */
        if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP ||
           sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP)
        {
            sprintf(buff, "   RX Power            : %.2f dBm\r\n", sfp_ddm_info_measured.rx_power);
            PROCESS_MORE_FUNC(buff);
        }
        else if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP ||
                sfp_info.identifier == SWDRV_TYPE_GBIC_ID_QSFP_PLUS)
        {
            sprintf(buff, "   RX Power(ch1)       : %.2f dBm\r\n", sfp_ddm_info_measured.rx_power);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   RX Power(ch2)       : %.2f dBm\r\n", sfp_ddm_info_measured.rx_power_2);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   RX Power(ch3)       : %.2f dBm\r\n", sfp_ddm_info_measured.rx_power_3);
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   RX Power(ch4)       : %.2f dBm\r\n", sfp_ddm_info_measured.rx_power_4);
            PROCESS_MORE_FUNC(buff);
        }
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
        if(TRUE == SWCTRL_POM_GetPortSfpDdmThreshold(lport, &sfp_ddm_threshold) &&
           TRUE == SWCTRL_POM_GetPortSfpDdmThresholdStatus(lport, &sfp_ddm_threshold_status))
        {
            sprintf(buff," DDM Thresholds\r\n");
            PROCESS_MORE_FUNC(buff);

            sprintf(buff, "   %-20s  %12s  %12s  %12s  %12s\r\n", "", "Low Alarm", "Low Warning", "High Warning", "High Alarm");
            PROCESS_MORE_FUNC(buff);
            sprintf(buff, "   %-20s  %12s  %12s  %12s  %12s\r\n", "-----------", "------------", "------------", "------------", "------------");
            PROCESS_MORE_FUNC(buff);

            /* show temperature informations */
            sprintf(buff, "   %-20s   %11.2f%s  %11.2f%s  %11.2f%s  %11.2f%s\r\n","Temperature(Celsius)",
                    sfp_ddm_threshold.temp_low_alarm,
                    sfp_ddm_threshold_status.temp_low_alarm?"!":" ",
                    sfp_ddm_threshold.temp_low_warning,
                    sfp_ddm_threshold_status.temp_low_warning?"!":" ",
                    sfp_ddm_threshold.temp_high_warning,
                    sfp_ddm_threshold_status.temp_high_warning?"!":" ",
                    sfp_ddm_threshold.temp_high_alarm,
                    sfp_ddm_threshold_status.temp_high_alarm?"!":" ");
            PROCESS_MORE_FUNC(buff);

            /* show temperature informations */
            sprintf(buff, "   %-20s   %11.2f%s  %11.2f%s  %11.2f%s  %11.2f%s\r\n", "Voltage(Volts)",
                    sfp_ddm_threshold.voltage_low_alarm,
                    sfp_ddm_threshold_status.voltage_low_alarm?"!":" ",
                    sfp_ddm_threshold.voltage_low_warning,
                    sfp_ddm_threshold_status.voltage_low_warning?"!":" ",
                    sfp_ddm_threshold.voltage_high_warning,
                    sfp_ddm_threshold_status.voltage_high_warning?"!":" ",
                    sfp_ddm_threshold.voltage_high_alarm,
                    sfp_ddm_threshold_status.voltage_high_alarm?"!":" ");
            PROCESS_MORE_FUNC(buff);

            /* show Tx bias current informations */
            sprintf(buff, "   %-20s   %11.2f%s  %11.2f%s  %11.2f%s  %11.2f%s\r\n", "Current(mA)",
                    sfp_ddm_threshold.bias_low_alarm,
                    sfp_ddm_threshold_status.bias_low_alarm?"!":" ",
                    sfp_ddm_threshold.bias_low_warning,
                    sfp_ddm_threshold_status.bias_low_warning?"!":" ",
                    sfp_ddm_threshold.bias_high_warning,
                    sfp_ddm_threshold_status.bias_high_warning?"!":" ",
                    sfp_ddm_threshold.bias_high_alarm,
                    sfp_ddm_threshold_status.bias_high_alarm?"!":" ");
            PROCESS_MORE_FUNC(buff);

            /* show Tx power informations, convert unit from watts to dBm */
            if(sfp_info.identifier == SWDRV_TYPE_GBIC_ID_SFP ||
               sfp_info.identifier == SWDRV_TYPE_GBIC_ID_XFP)
            {
                sprintf(buff_tmp, "   %-20s", "TxPower(dBm)");
                strcat(buff, buff_tmp);

                sprintf(buff_tmp, "   %11.2f%s", sfp_ddm_threshold.tx_power_low_alarm,
                        sfp_ddm_threshold_status.tx_power_low_alarm?"!":" ");
                strcat(buff, buff_tmp);

                sprintf(buff_tmp, "  %11.2f%s", sfp_ddm_threshold.tx_power_low_warning,
                        sfp_ddm_threshold_status.tx_power_low_warning?"!":" ");
                strcat(buff, buff_tmp);

                sprintf(buff_tmp, "  %11.2f%s", sfp_ddm_threshold.tx_power_high_warning,
                        sfp_ddm_threshold_status.tx_power_high_warning?"!":" ");
                strcat(buff, buff_tmp);

                sprintf(buff_tmp, "  %11.2f%s\r\n", sfp_ddm_threshold.tx_power_high_alarm,
                        sfp_ddm_threshold_status.tx_power_high_alarm?"!":" ");
                strcat(buff, buff_tmp);
                PROCESS_MORE_FUNC(buff);
            }

            /* show Rx power informations, convert unit from watts to dBm */
            sprintf(buff_tmp, "   %-20s", "RxPower(dBm)");
            strcat(buff, buff_tmp);

            sprintf(buff_tmp, "   %11.2f%s", sfp_ddm_threshold.rx_power_low_alarm,
                    sfp_ddm_threshold_status.rx_power_low_alarm?"!":" ");
            strcat(buff, buff_tmp);

            sprintf(buff_tmp, "  %11.2f%s", sfp_ddm_threshold.rx_power_low_warning,
                    sfp_ddm_threshold_status.rx_power_low_warning?"!":" ");
            strcat(buff, buff_tmp);

            sprintf(buff_tmp, "  %11.2f%s", sfp_ddm_threshold.rx_power_high_warning,
                    sfp_ddm_threshold_status.rx_power_high_warning?"!":" ");
            strcat(buff, buff_tmp);

            sprintf(buff_tmp, "  %11.2f%s\r\n", sfp_ddm_threshold.rx_power_high_alarm,
                    sfp_ddm_threshold_status.rx_power_high_alarm?"!":" ");
            strcat(buff, buff_tmp);
            PROCESS_MORE_FUNC(buff);
        }
#endif
    }
    else
    {
        sprintf(buff,"   DDM not supported\r\n");
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}
#endif/* End of #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

UI32_T CLI_API_Show_Interfaces_Transceiver_Threshold(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T line_num = 0;
    UI32_T unit, max_port_num;
    UI32_T verify_unit, verify_port, lport, sfp_index;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
#if (CLI_SUPPORT_PORT_NAME == 1)
    UI32_T trunk_id = 0;
    UI8_T  name[MAXSIZE_ifName+1] = {0};
#endif

    if (arg[0] == NULL)
    {
        verify_unit = ctrl_P->sys_info.my_unit_id;

        for (unit=0; STKTPLG_POM_GetNextUnit(&unit); )
        {
            verify_unit = unit;
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

            for(verify_port = 1; verify_port <= max_port_num; verify_port++)
            {
                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
                    continue;

                if(FALSE==STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                    continue;

            #if (CLI_SUPPORT_PORT_NAME == 1)
                CLI_LIB_Ifindex_To_Name(lport, name);
                sprintf(buff, "Information of %s:\r\n", name);
                PROCESS_MORE(buff);
            #else
                sprintf(buff, "Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                PROCESS_MORE(buff);
            #endif

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        if((line_num = CLI_API_SHOW_ShowOneInterfaceTransceiverThreshold(verify_unit, lport, line_num)) ==  JUMP_OUT_MORE)
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
                        break;

                    case CLI_API_ETH_TRUNK_MEMBER:
                        if((line_num = CLI_API_SHOW_ShowOneInterfaceTransceiverThreshold(verify_unit, lport, line_num)) ==  JUMP_OUT_MORE)
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
                        break;

                    case CLI_API_ETH_NOT_PRESENT:
                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                }/* End of switch(verify_ret) */
            }/* End of for(verify_port = 1; verify_port <= max_port_num; verify_port++) */
        }/* End of for (unit=0; STKTPLG_POM_GetNextUnit(&unit); ) */
    }
    else
    {
        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                verify_unit = atoi(arg[1]);
                verify_port = atoi( strchr(arg[1], '/')+1 );
    #if (CLI_SUPPORT_PORT_NAME == 1)
                if (isdigit(arg[1][0]))
                {
                    verify_unit = atoi(arg[1]);
                    verify_port = atoi(strchr(arg[1],'/')+1);
                }
                else/*port name*/
                {
                    if (!IF_MGR_IfnameToIfindex(arg[1], &lport))
                    {
                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
                        return CLI_NO_ERROR;
                    }
                    SWCTRL_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                }
    #endif

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        if(FALSE == STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                            break;

                    #if (CLI_SUPPORT_PORT_NAME == 1)
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        PROCESS_MORE_1("Information of %s\r\n", name);
                    #else
                        CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                    #endif

                        line_num++;
                        line_num = CLI_API_SHOW_ShowOneInterfaceTransceiverThreshold(verify_unit, lport, line_num);
                        if(line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;

                    case CLI_API_ETH_TRUNK_MEMBER:
                        if(FALSE==STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                            break;

                    #if (CLI_SUPPORT_PORT_NAME == 1)
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        PROCESS_MORE_1("Information of %s\r\n", name);
                    #endif
                        line_num++;
                        line_num = CLI_API_SHOW_ShowOneInterfaceTransceiverThreshold(verify_unit, lport, line_num);
                        if(line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;
                    case CLI_API_ETH_NOT_PRESENT:
                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                }
            }
                break;
            default:
                return CLI_ERR_INTERNAL;
        }/*switch(arg[0][0])*/
    }
#endif /* End of #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Interfaces_Transceiver(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T line_num = 0;
    UI32_T verify_unit, verify_port, sfp_index, lport;
    UI32_T unit, max_port_num;
    #if (CLI_SUPPORT_PORT_NAME == 1)
    UI32_T trunk_id = 0;
    UI8_T  name[MAXSIZE_ifName+1] = {0};
    #endif
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (arg[0] == NULL)
    {
        for (unit=0; STKTPLG_POM_GetNextUnit(&unit); )
        {
            verify_unit = unit;
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

            for(verify_port = 1; verify_port <= max_port_num; verify_port++)
            {
                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
                    continue;

                if(FALSE==STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                    continue;

    #if (CLI_SUPPORT_PORT_NAME == 1)
                CLI_LIB_Ifindex_To_Name(lport,name);
                sprintf(buff, "Information of %s:\r\n", name);
                PROCESS_MORE(buff);
    #else
                sprintf(buff, "Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                PROCESS_MORE(buff);
    #endif

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        if((line_num = CLI_API_SHOW_ShowOneInterfaceTransceiver(verify_unit, lport, sfp_index, line_num)) ==  JUMP_OUT_MORE)
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
                        break;

                    case CLI_API_ETH_TRUNK_MEMBER:
                        if((line_num = CLI_API_SHOW_ShowOneInterfaceTransceiver(verify_unit, lport, sfp_index, line_num)) ==  JUMP_OUT_MORE)
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
                        break;

                    case CLI_API_ETH_NOT_PRESENT:
                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                    default:
                        break;
                }/* End of switch(verify_ret) */
            }/* End of for(verify_port = 1; ... */
        }/* End of for (unit=0; ... */
    }
    else
    {
        switch(arg[0][0])
        {
            case 'e':
            case 'E':
            {
                verify_unit = atoi(arg[1]);
                verify_port = atoi( strchr(arg[1], '/')+1 );
    #if (CLI_SUPPORT_PORT_NAME == 1)
                if (isdigit(arg[1][0]))
                {
                    verify_unit = atoi(arg[1]);
                    verify_port = atoi(strchr(arg[1],'/')+1);
                }
                else/*port name*/
                {
                    if (!IF_MGR_IfnameToIfindex(arg[1], &lport))
                    {
                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
                        return CLI_NO_ERROR;
                    }
                    SWCTRL_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                }
    #endif

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                switch(verify_ret)
                {
                    case CLI_API_ETH_OK:
                        if(FALSE==STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                            break;

    #if (CLI_SUPPORT_PORT_NAME == 1)
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        PROCESS_MORE_1("Information of %s\r\n", name);
    #else
                        CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
                        line_num++;
                        line_num = CLI_API_SHOW_ShowOneInterfaceTransceiver(verify_unit, lport, sfp_index, line_num);
                        if(line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;

                    case CLI_API_ETH_TRUNK_MEMBER:
                        if(FALSE==STKTPLG_OM_UserPortToSfpIndex(verify_unit, verify_port, &sfp_index))
                            break;

    #if (CLI_SUPPORT_PORT_NAME == 1)
                        CLI_LIB_Ifindex_To_Name(lport,name);
                        PROCESS_MORE_1("Information of %s\r\n", name);
    #endif
                        line_num++;
                        line_num = CLI_API_SHOW_ShowOneInterfaceTransceiver(verify_unit, lport, sfp_index, line_num);
                        if(line_num == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;
                    case CLI_API_ETH_NOT_PRESENT:
                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                }
            }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }/* End of switch(arg[0][0]) */
    }
#endif/* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */
    return CLI_NO_ERROR;
}

#define STRLEN_OF_UI64 21

static UI32_T Show_Counters_One_Port(UI32_T ifindex, UI32_T line_num, CLI_TASK_WorkingArea_T *ctrl_P)
{


   UI32_T unit;
   UI32_T port;
   UI32_T trunk_id;
   UI32_T idx;
   UI32_T ret_type = 0;
   char str1[STRLEN_OF_UI64] = {0};
   char str2[STRLEN_OF_UI64] = {0};
   char str3[STRLEN_OF_UI64] = {0};

   char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
   CLI_API_ShowIfCounter_T IfCounter;
   NMTR_MGR_Utilization_300_SECS_T port_util;

    assert(ctrl_P != NULL);

   memset(&IfCounter, 0, sizeof(CLI_API_ShowIfCounter_T));
   memset(str1, 0, sizeof(str1));
   memset(str2, 0, sizeof(str2));
   memset(&port_util, 0, sizeof(NMTR_MGR_Utilization_300_SECS_T));

   ret_type = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

   if (ret_type == SWCTRL_LPORT_NORMAL_PORT || ret_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
   {
      idx = (unit - 1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + port - 1;      /*idx is the port number reduce 1*/
#if (CLI_SUPPORT_PORT_NAME == 1)
      {
         UI8_T name[MAXSIZE_ifName+1] = {0};
         CLI_LIB_Ifindex_To_Name(ifindex,name);
         sprintf(buff, "%s :\r\n", name);
         PROCESS_MORE(buff);
      }
#else
      sprintf(buff, "Ethernet %lu/%2lu\r\n", (unsigned long)unit, (unsigned long)port);
      PROCESS_MORE_FUNC(buff);
#endif
   }
   else if (ret_type == SWCTRL_LPORT_TRUNK_PORT)
   {
      idx = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + trunk_id -1;
      sprintf(buff, "Trunk %lu\r\n", (unsigned long)trunk_id);
      PROCESS_MORE_FUNC(buff);
   }
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
   else if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
   {
      UI32_T vid;

      VLAN_IFINDEX_CONVERTTO_VID(ifindex, vid);
      sprintf(buff, "VLAN %lu\r\n", (unsigned long)vid);
      PROCESS_MORE_FUNC(buff);
   }
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */
   else
      return JUMP_OUT_MORE;
   /*function GetIfCounter have show error message so here need return*/
   if (GetIfCounter(ifindex, &IfCounter) == FALSE)
   {
      return JUMP_OUT_MORE;
   }


#if(SYS_CPNT_SYSTEMWIDE_COUNTER == FALSE)
   if(ctrl_P->CMenu.counters_table[idx] != NULL)
   {
      GetDeltaIfCounter((CLI_API_ShowIfCounter_T *) ctrl_P->CMenu.counters_table[idx], &IfCounter);
   }
#endif

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
   /* temporarily solution for vlan counter
    */
   if (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
   {
       UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCInOctets, (UI8_T *)str1, STRLEN_OF_UI64);
       sprintf(buff,"%24s Octets Input \r\n", str1);
       PROCESS_MORE_FUNC(buff);

       UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCInUcastPkts, (UI8_T *)str1, STRLEN_OF_UI64);
       sprintf(buff,"%24s Packets Input \r\n", str1);
       PROCESS_MORE_FUNC(buff);

       return line_num;
   }
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

   sprintf(buff," ===== IF table Stats =====\r\n");
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_table_stats.ifInOctets, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_table_stats.ifOutOctets, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff,"%24s Octets Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff,"%24s Octets Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_table_stats.ifInUcastPkts, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_table_stats.ifOutUcastPkts, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff,"%24s Unicast Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff,"%24s Unicast Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_table_stats.ifInDiscards, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_table_stats.ifOutDiscards, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff,"%24s Discard Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff,"%24s Discard Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_table_stats.ifInErrors,(UI8_T *) str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_table_stats.ifOutErrors, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff,"%24s Error Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff,"%24s Error Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_table_stats.ifInUnknownProtos, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_table_stats.ifOutQLen,(UI8_T *) str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Unknown Protocols Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s QLen Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   sprintf(buff, " ===== Extended Iftable Stats =====\r\n");
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCInMulticastPkts,(UI8_T *) str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCOutMulticastPkts,(UI8_T *) str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Multi-cast Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Multi-cast Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);


   UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCInBroadcastPkts,(UI8_T *) str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.if_xtable_stats.ifHCOutBroadcastPkts, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Broadcast Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Broadcast Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   sprintf(buff, " ===== Ether-like Stats =====\r\n");
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsAlignmentErrors, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsFCSErrors, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Alignment Errors \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s FCS Errors \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsSingleCollisionFrames, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsMultipleCollisionFrames,(UI8_T *) str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Single Collision Frames \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Multiple Collision Frames \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsSQETestErrors, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsDeferredTransmissions, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s SQE Test Errors \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Deferred Transmissions \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsLateCollisions, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsExcessiveCollisions, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Late Collisions \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Excessive Collisions \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsInternalMacTransmitErrors, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsInternalMacReceiveErrors,(UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Internal Mac Transmit Errors \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Internal Mac Receive Errors \r\n", str2);
   PROCESS_MORE_FUNC(buff);


   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsFrameTooLongs, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsCarrierSenseErrors, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Frames Too Long \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Carrier Sense Errors \r\n", str2);
   PROCESS_MORE_FUNC(buff);


   UI64_T_2_STR(IfCounter.ether_like_stats.dot3StatsSymbolErrors, (UI8_T *)str1, STRLEN_OF_UI64);
   sprintf(buff, "%24s Symbol Errors \r\n", str1);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.ether_like_pause.dot3InPauseFrames, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.ether_like_pause.dot3OutPauseFrames, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Pause Frames Input \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Pause Frames Output \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   sprintf(buff, " ===== RMON Stats =====\r\n");
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsDropEvents, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsOctets, (UI8_T *)str2, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts, (UI8_T *)str3, STRLEN_OF_UI64);
   sprintf(buff, "%24s Drop Events \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Octets \r\n", str2);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Packets \r\n", str3);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsBroadcastPkts, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsMulticastPkts, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Broadcast PKTS \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Multi-cast PKTS \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsUndersizePkts, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsOversizePkts, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Undersize PKTS \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Oversize PKTS \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsFragments, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsJabbers, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Fragments \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Jabbers \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsCRCAlignErrors, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsCollisions, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s CRC Align Errors \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Collisions \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts64Octets, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts65to127Octets, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Packet Size <= 64 Octets \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Packet Size 65 to 127 Octets \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts128to255Octets, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts256to511Octets, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Packet Size 128 to 255 Octets \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Packet Size 256 to 511 Octets \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts512to1023Octets, (UI8_T *)str1, STRLEN_OF_UI64);
   UI64_T_2_STR(IfCounter.rmon_stats.etherStatsPkts1024to1518Octets, (UI8_T *)str2, STRLEN_OF_UI64);
   sprintf(buff, "%24s Packet Size 512 to 1023 Octets \r\n", str1);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "%24s Packet Size 1024 to 1518 Octets \r\n", str2);
   PROCESS_MORE_FUNC(buff);

   if (NMTR_PMGR_GetPortUtilization300secs(ifindex, &port_util))
   {
      sprintf(buff, " ===== Port Utilization (recent 300 seconds) =====\r\n");
      PROCESS_MORE_FUNC(buff);

      UI64_T_2_STR(port_util.ifInOctets / (1000/8), (UI8_T *)str1, STRLEN_OF_UI64);
      UI64_T_2_STR(port_util.ifInPackets, (UI8_T *)str2, STRLEN_OF_UI64);
      sprintf(buff, "%24s Octets Input in kbits per second\r\n", str1);
      PROCESS_MORE_FUNC(buff);
      sprintf(buff, "%24s Packets Input per second\r\n", str2);
      PROCESS_MORE_FUNC(buff);
      sprintf(buff, "%21lu.%02lu %% Input Utilization\r\n", (unsigned long)(port_util.ifInOctets_utilization / 100), (unsigned long)(port_util.ifInOctets_utilization % 100));
      PROCESS_MORE_FUNC(buff);

      UI64_T_2_STR(port_util.ifOutOctets / (1000/8), (UI8_T *)str1, STRLEN_OF_UI64);
      UI64_T_2_STR(port_util.ifOutPackets, (UI8_T *)str2, STRLEN_OF_UI64);
      sprintf(buff, "%24s Octets Output in kbits per second\r\n", str1);
      PROCESS_MORE_FUNC(buff);
      sprintf(buff, "%24s Packets Output per second\r\n", str2);
      PROCESS_MORE_FUNC(buff);
      sprintf(buff, "%21lu.%02lu %% Output Utilization\r\n", (unsigned long)(port_util.ifOutOctets_utilization / 100), (unsigned long)(port_util.ifOutOctets_utilization % 100));
      PROCESS_MORE_FUNC(buff);
   }

   return line_num;
}



#if(SYS_CPNT_SYSTEMWIDE_COUNTER == FALSE)

static void SUB_UI64_T(UI64_T *RefIfTable, UI64_T *CurrentIfTable)
{
   L_STDLIB_UI64_Sub ( &(L_STDLIB_UI64_H32(*CurrentIfTable)),
                       &(L_STDLIB_UI64_L32(*CurrentIfTable)),
                       L_STDLIB_UI64_H32(*RefIfTable),
                       L_STDLIB_UI64_L32(*RefIfTable));

   return ;
}

static void GetDeltaIfCounter(CLI_API_ShowIfCounter_T *RefIfTable, CLI_API_ShowIfCounter_T *CurrentIfTable)
{

   SUB_UI64_T(&RefIfTable->if_table_stats.ifInOctets, &CurrentIfTable->if_table_stats.ifInOctets);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifInUcastPkts, &CurrentIfTable->if_table_stats.ifInUcastPkts);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifInNUcastPkts, &CurrentIfTable->if_table_stats.ifInNUcastPkts);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifInDiscards, &CurrentIfTable->if_table_stats.ifInDiscards);

   SUB_UI64_T(&RefIfTable->if_table_stats.ifInErrors, &CurrentIfTable->if_table_stats.ifInErrors);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifInUnknownProtos, &CurrentIfTable->if_table_stats.ifInUnknownProtos);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutOctets, &CurrentIfTable->if_table_stats.ifOutOctets);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutUcastPkts, &CurrentIfTable->if_table_stats.ifOutUcastPkts);

   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutNUcastPkts, &CurrentIfTable->if_table_stats.ifOutNUcastPkts);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutDiscards, &CurrentIfTable->if_table_stats.ifOutDiscards);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutErrors, &CurrentIfTable->if_table_stats.ifOutErrors);
   SUB_UI64_T(&RefIfTable->if_table_stats.ifOutQLen, &CurrentIfTable->if_table_stats.ifOutQLen);


   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsAlignmentErrors, &CurrentIfTable->ether_like_stats.dot3StatsAlignmentErrors);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsFCSErrors, &CurrentIfTable->ether_like_stats.dot3StatsFCSErrors);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsSingleCollisionFrames, &CurrentIfTable->ether_like_stats.dot3StatsSingleCollisionFrames);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsMultipleCollisionFrames, &CurrentIfTable->ether_like_stats.dot3StatsMultipleCollisionFrames);

   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsSQETestErrors, &CurrentIfTable->ether_like_stats.dot3StatsSQETestErrors);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsDeferredTransmissions, &CurrentIfTable->ether_like_stats.dot3StatsDeferredTransmissions);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsLateCollisions, &CurrentIfTable->ether_like_stats.dot3StatsLateCollisions);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsExcessiveCollisions, &CurrentIfTable->ether_like_stats.dot3StatsExcessiveCollisions);

   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsInternalMacTransmitErrors, &CurrentIfTable->ether_like_stats.dot3StatsInternalMacTransmitErrors);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsCarrierSenseErrors, &CurrentIfTable->ether_like_stats.dot3StatsCarrierSenseErrors);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsFrameTooLongs, &CurrentIfTable->ether_like_stats.dot3StatsFrameTooLongs);
   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsInternalMacReceiveErrors, &CurrentIfTable->ether_like_stats.dot3StatsInternalMacReceiveErrors);

   SUB_UI64_T(&RefIfTable->ether_like_stats.dot3StatsSymbolErrors, &CurrentIfTable->ether_like_stats.dot3StatsSymbolErrors);

   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifInMulticastPkts, &CurrentIfTable->if_xtable_stats.ifInMulticastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifInBroadcastPkts, &CurrentIfTable->if_xtable_stats.ifInBroadcastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifOutMulticastPkts, &CurrentIfTable->if_xtable_stats.ifOutMulticastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifOutBroadcastPkts, &CurrentIfTable->if_xtable_stats.ifOutBroadcastPkts);

   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCInOctets, &CurrentIfTable->if_xtable_stats.ifHCInOctets);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCInUcastPkts, &CurrentIfTable->if_xtable_stats.ifHCInUcastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCInMulticastPkts, &CurrentIfTable->if_xtable_stats.ifHCInMulticastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCInBroadcastPkts, &CurrentIfTable->if_xtable_stats.ifHCInBroadcastPkts);

   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCOutOctets, &CurrentIfTable->if_xtable_stats.ifHCOutOctets);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCOutUcastPkts, &CurrentIfTable->if_xtable_stats.ifHCOutUcastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCOutMulticastPkts, &CurrentIfTable->if_xtable_stats.ifHCOutMulticastPkts);
   SUB_UI64_T(&RefIfTable->if_xtable_stats.ifHCOutBroadcastPkts, &CurrentIfTable->if_xtable_stats.ifHCOutBroadcastPkts);



   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsDropEvents, &CurrentIfTable->rmon_stats.etherStatsDropEvents);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsOctets, &CurrentIfTable->rmon_stats.etherStatsOctets);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts, &CurrentIfTable->rmon_stats.etherStatsPkts);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsBroadcastPkts, &CurrentIfTable->rmon_stats.etherStatsBroadcastPkts);

   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsMulticastPkts, &CurrentIfTable->rmon_stats.etherStatsMulticastPkts);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsCRCAlignErrors, &CurrentIfTable->rmon_stats.etherStatsCRCAlignErrors);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsUndersizePkts, &CurrentIfTable->rmon_stats.etherStatsUndersizePkts);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsOversizePkts, &CurrentIfTable->rmon_stats.etherStatsOversizePkts);

   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsFragments, &CurrentIfTable->rmon_stats.etherStatsFragments);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsJabbers, &CurrentIfTable->rmon_stats.etherStatsJabbers);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsCollisions, &CurrentIfTable->rmon_stats.etherStatsCollisions);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts64Octets, &CurrentIfTable->rmon_stats.etherStatsPkts64Octets);

   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts65to127Octets, &CurrentIfTable->rmon_stats.etherStatsPkts65to127Octets);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts128to255Octets, &CurrentIfTable->rmon_stats.etherStatsPkts128to255Octets);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts256to511Octets, &CurrentIfTable->rmon_stats.etherStatsPkts256to511Octets);
   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts512to1023Octets, &CurrentIfTable->rmon_stats.etherStatsPkts512to1023Octets);

   SUB_UI64_T(&RefIfTable->rmon_stats.etherStatsPkts1024to1518Octets, &CurrentIfTable->rmon_stats.etherStatsPkts1024to1518Octets);

}
#endif /* #if(SYS_CPNT_SYSTEMWIDE_COUNTER == FALSE) */

static UI32_T show_log_ram  (UI8_T **logging_level, UI32_T line_num,char *arg[]);
static UI32_T show_log_flash  (UI8_T **logging_level, UI32_T line_num,char *arg[]);

UI32_T CLI_API_Show_Log(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    UI8_T  line_num = 0;
    UI8_T  *logging_level[] = { (UI8_T *) "emergencies",
                                (UI8_T *) "alerts",
                                (UI8_T *) "critical",
                                (UI8_T *) "errors",
                                (UI8_T *) "warnings",
                                (UI8_T *) "notifications",
                                (UI8_T *) "informational",
                                (UI8_T *) "debugging"
                              };

    line_num += 1;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

    if(arg[0][0] == 'r' || arg[0][0] == 'R')/*RAM*/
    {
        show_log_ram(logging_level, line_num,arg);
    }
    else if(arg[0][0] == 'f' || arg[0][0] == 'F')/*FLASH*/
    {
        show_log_flash(logging_level, line_num,arg);
    }

    return CLI_NO_ERROR;

}


static UI32_T show_log_ram  (UI8_T **logging_level, UI32_T line_num,char *arg[])
{
    SYSLOG_MGR_Record_T record;
    BOOL_T is_setting = FALSE;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if( arg[1]!= NULL && (arg[1][0] == 'l' || arg[1][0] == 'L'))/*Login*/
    {
        if(arg[2] != NULL && (arg[2][0] == 't' || arg[2][0] == 'T'))/*Tail*/
        {
            memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
            record.entry_index = (UI32_T) (-1);
            while(SYSLOG_PMGR_SnmpGetNextUcNormalEntry(&record))
            {
                if(line_num >= 24 || is_setting)
                {
                    switch(CLI_LIB_ProcMoreFeature())
                    {
                        case 1:
                            line_num = 0;
                            break;
                        case 2:
                            if(is_setting)
                            {
                                line_num = 0;
                                is_setting = FALSE;
                            }
                            else
                                line_num -= 3;
                            break;
                        case CLI_EXIT_SESSION:
                            return CLI_EXIT_SESSION;
                        default:
                        case 3:
                            return CLI_NO_ERROR;
                    }
                }

                if(record.owner_info.level!=SYSLOG_LEVEL_INFO || record.owner_info.module_no!=SYS_MODULE_CLI ||
                                                    record.owner_info.function_no!=1 || record.owner_info.error_no !=1)
                continue;
                CLI_LIB_PrintStr_1("   \"%s\"\r\n", record.message);
                line_num += 1;
            }
        }
        else
        {
            memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
            record.entry_index = (UI32_T) (-1);
            while(SYSLOG_PMGR_GetNextUcNormalEntries(&record))
            {
                if(line_num >= 24 || is_setting)
                {
                    switch(CLI_LIB_ProcMoreFeature())
                    {
                        case 1:
                            line_num = 0;
                            break;
                        case 2:
                            if(is_setting)
                            {
                                line_num = 0;
                                is_setting = FALSE;
                            }
                            else
                                line_num -= 3;
                            break;
                        case CLI_EXIT_SESSION:
                           return CLI_EXIT_SESSION;
                        default:
                        case 3:
                            return CLI_NO_ERROR;
                    }
                }

                if(record.owner_info.level!=SYSLOG_LEVEL_INFO || record.owner_info.module_no!=SYS_MODULE_CLI ||
                                                    record.owner_info.function_no!=1 || record.owner_info.error_no !=1)
                continue;
                CLI_LIB_PrintStr_1("   \"%s\"\r\n", record.message);
                line_num += 1;
            }
        }
    }
    else
    {
        if(arg[1] != NULL && (arg[1][0] == 't' || arg[1][0] == 'T'))/*Tail*/
        {
            memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
            record.entry_index = (UI32_T) (-1);

            while(SYSLOG_PMGR_SnmpGetNextUcNormalEntry(&record))
            {
                sprintf(buff,"[%lu] %02d:%02d:%02d %04d-%02d-%02d\r\n", (unsigned long)record.entry_index,    record.rtc_time.hour, record.rtc_time.minute, record.rtc_time.second
                                                          ,record.rtc_time.year, record.rtc_time.month, record.rtc_time.day);
                PROCESS_MORE(buff);
                sprintf(buff,"   \"%s\"\r\n", record.message);
                PROCESS_MORE(buff);
                sprintf(buff,"   level : %d, module : %lu, function : %d, and event no. : %d\r\n", record.owner_info.level,
                                                                                              (unsigned long)record.owner_info.module_no,
                                                                                              record.owner_info.function_no,
                                                                                              record.owner_info.error_no);
                PROCESS_MORE(buff);
            }
        }
        else
        {
            memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
            record.entry_index = (UI32_T) (-1);

            while(SYSLOG_PMGR_GetNextUcNormalEntries(&record))
            {
                sprintf(buff,"[%lu] %02d:%02d:%02d %04d-%02d-%02d\r\n", (unsigned long)record.entry_index,    record.rtc_time.hour, record.rtc_time.minute, record.rtc_time.second
                                                          ,record.rtc_time.year, record.rtc_time.month, record.rtc_time.day);
                PROCESS_MORE(buff);
                sprintf(buff,"   \"%s\"\r\n", record.message);
                PROCESS_MORE(buff);
                sprintf(buff,"   level : %d, module : %lu, function : %d, and event no. : %d\r\n", record.owner_info.level,
                                                                                              (unsigned long)record.owner_info.module_no,
                                                                                              record.owner_info.function_no,
                                                                                              record.owner_info.error_no);
                PROCESS_MORE(buff);
            }
        }
    }
   return CLI_NO_ERROR;
}

static UI32_T show_log_flash  (
    UI8_T **logging_level,
    UI32_T line_num,char *arg[])
{

    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    SYSLOG_MGR_Record_T record;
    BOOL_T is_setting = FALSE;

    if(arg[1] != NULL && (arg[1][0] == 'l' || arg[1][0] == 'L'))/*Login*/
    {
        memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
        record.entry_index = (UI32_T) (-1);

        while(SYSLOG_PMGR_GetNextUcFlashEntry(&record))
        {
            if(line_num >= 24 || is_setting)
            {
                switch(CLI_LIB_ProcMoreFeature())
                {
                    case 1:
                        line_num = 0;
                        break;
                    case 2:
                        if(is_setting)
                        {
                            line_num = 0;
                            is_setting = FALSE;
                        }
                        else
                            line_num -= 3;
                        break;
                    case CLI_EXIT_SESSION:
                        return CLI_EXIT_SESSION;
                    default:
                    case 3:
                        return CLI_NO_ERROR;
                }
            }

            if(record.owner_info.level!=SYSLOG_LEVEL_INFO || record.owner_info.module_no!=SYS_MODULE_CLI ||
                                                record.owner_info.function_no!=1 || record.owner_info.error_no !=1)
            continue;

            CLI_LIB_PrintStr_1("   \"%s\"\r\n", record.message);
            line_num += 1;
        }
    }
    else
    {
        memset(&record, 0, sizeof(SYSLOG_MGR_Record_T));
        record.entry_index = (UI32_T) (-1);

        while(SYSLOG_PMGR_GetNextUcFlashEntry(&record))
        {
            sprintf(buff,"[%lu] %02d:%02d:%02d %04d-%02d-%02d\r\n", (unsigned long)record.entry_index,
                                                                    record.rtc_time.hour,
                                                                    record.rtc_time.minute,
                                                                    record.rtc_time.second,
                                                                    record.rtc_time.year,
                                                                    record.rtc_time.month,
                                                                    record.rtc_time.day);
            PROCESS_MORE(buff);
            sprintf(buff,"   \"%s\"\r\n", record.message);
            PROCESS_MORE(buff);
            sprintf(buff,"   level : %d, module : %lu, function : %d, and event no. : %d\r\n", record.owner_info.level,
                                                                                              (unsigned long)record.owner_info.module_no,
                                                                                              record.owner_info.function_no,
                                                                                              record.owner_info.error_no);
            PROCESS_MORE(buff);
        }
    }
    return CLI_NO_ERROR;
}


static void UI64_T_2_STR(UI64_T value, UI8_T *str, UI32_T sizeofstr)
{
   memset(str, 0, sizeof(UI8_T)*sizeofstr);
   if (strlen((char *)str) > sizeofstr)
   {
       return;
   }
   L_STDLIB_UI64toa (L_STDLIB_UI64_H32(value), L_STDLIB_UI64_L32(value), (char *)str);
   L_STDLIB_Trim_Left((char *)str, STRLEN_OF_UI64 - 1);
   return;
}

#if(SYS_CPNT_MASTER_BUTTON==SYS_CPNT_MASTER_BUTTON_SOFTWARE)
UI32_T CLI_API_Show_SwitchMasterButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T unit = 0;
   STKTPLG_MGR_Switch_Info_T switch_info;
   BOOL_T isPressed;

   CLI_LIB_PrintStr("Switch ID  Master-button-pressed\r\n");
   CLI_LIB_PrintStr("---------  ---------------------\r\n");

   for (unit=0; STKTPLG_POM_GetNextUnit(&unit); )
   {

      memset(&switch_info,0,sizeof(STKTPLG_MGR_Switch_Info_T));
      switch_info.sw_unit_index = unit;
      isPressed=STKTPLG_POM_GetMasterButtonStatus(unit);
      if (STKTPLG_PMGR_GetSwitchInfo(&switch_info) == TRUE)
      {
          CLI_LIB_PrintStr_2("    %d                %c          \r\n", switch_info.sw_unit_index, isPressed==TRUE? 'Y':'N');
      }
   }
   return CLI_NO_ERROR;
}
#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
UI32_T CLI_API_Show_SwitchStackingButton(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    BOOL_T isPressed, isActived;

    CLI_LIB_PrintStr("Stacking-button-pressed status:\r\n");
    CLI_LIB_PrintStr("Config Status  Active Status\r\n");
    CLI_LIB_PrintStr("-------------  -------------\r\n");
    STKTPLG_OM_GetStackingButtonStatus(&isPressed, &isActived);
    CLI_LIB_PrintStr_2("      %c              %c      \r\n", isPressed==TRUE? 'Y':'N', isActived==TRUE? 'Y':'N');
    return CLI_NO_ERROR;
}
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */


/*command: show interfaces [brief]*/
UI32_T CLI_API_Show_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

    /*the usage of PROCESS_MORE()*/
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

    /* show interface <cr> */
    if(arg[0] == NULL)
    {
        UI32_T lport, verify_unit, verify_port;
        UI32_T current_max_unit = 0;
        UI32_T max_port_num;
        BOOL_T is_inherit;
        CLI_API_EthStatus_T verify_ret;

#if (SYS_CPNT_STACKING == TRUE)
        /*STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);*/
        current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
      current_max_unit = 1;
#endif
        /*for(verify_unit = 1; verify_unit <= current_max_unit; verify_unit++)*/ /* unit loop */
        for (verify_unit=0; STKTPLG_POM_GetNextUnit(&verify_unit); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

            for(verify_port = 1; verify_port <= max_port_num; verify_port++) /* port loop */
            {
                if(!SWCTRL_POM_UIUserPortExisting(verify_unit, verify_port))
                {
                    continue;
                }

                is_inherit = FALSE;
                verify_ret = SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &lport, &is_inherit);
                if (verify_ret == SWCTRL_LPORT_UNKNOWN_PORT || verify_ret == SWCTRL_LPORT_TRUNK_PORT)
                {
                    continue;
                }

                /* port info */
                if((line_num = Show_interface_port_info(lport, verify_unit, verify_port, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* vlan */
                if((line_num = Show_interface_vlan_info(lport, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* STP & 802.1d priority & flow control*/
                if((line_num = Show_interface_stp(lport, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* mirror & monitor */
                if((line_num = Show_interface_mirror(lport, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* trunk info */
                if((line_num = Show_interface_trunk(lport, line_num, verify_unit, verify_port)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* port name info */
                if((line_num = Show_interface_portname(lport, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

                /* traffic info */
                if((line_num = Show_interface_traffic(lport, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }

#if (SYS_CPNT_CLI_SHOW_TECH_SUPPORT == TRUE)
                    /* release CPU for a while*/
                    SYSFUN_Sleep(10);
#endif
            } /* end of port loop */
        } /* end of unit loop */

        return CLI_NO_ERROR;
    } /* end of "show interface <cr> "*/

    /* show interface ethernet unit/port */
    else if (arg[0][0] == 'e' || arg[0][0] == 'E')
    {
        UI32_T lport, trunk_id, verify_unit, verify_port;
        CLI_API_EthStatus_T verify_ret;

        verify_unit = 0;
        verify_port = 0;
        trunk_id = 0;
        lport = 0;

/* port name supported */
#if (CLI_SUPPORT_PORT_NAME == 1)
        if (IF_PMGR_IfnameToIfindex(arg[1], &lport) == TRUE)
        {
            SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("%s does not exist"NEWLINE, arg[1]);
#endif
            return CLI_NO_ERROR;
        }

/* port name not supported */
#else
        if (isdigit(arg[1][0]))
        {
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1], '/') + 1);
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
             CLI_API_Show_Exception_Handeler_Msg();
#else
             CLI_LIB_PrintStr_1("%s is an incorrrect input."NEWLINE, arg[1]);
#endif
            return CLI_NO_ERROR;
        }
#endif

        /* show */
        verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
        if(verify_ret == CLI_API_ETH_NOT_PRESENT || verify_ret == CLI_API_ETH_UNKNOWN_PORT)
        {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            return CLI_NO_ERROR;
        }

        /* port info */
        if((line_num = Show_interface_port_info(lport, verify_unit, verify_port, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* vlan */
        if((line_num = Show_interface_vlan_info(lport, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* STP & 802.1d priority & flow control*/
        if((line_num = Show_interface_stp(lport, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* mirror & monitor */
        if((line_num = Show_interface_mirror(lport, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* trunk info */
        if((line_num = Show_interface_trunk(lport, line_num, verify_unit, verify_port)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* port name info */
        if((line_num = Show_interface_portname(lport, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        /* traffic info */
        if((line_num = Show_interface_traffic(lport, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        return CLI_NO_ERROR;
}

    /* show interface brief */
    else if (arg[0][0] == 'b' || arg[0][0] == 'B')
    {
        /*local variable*/
        UI32_T lport, verify_unit, verify_port;
        UI32_T current_max_unit;
        UI32_T max_port_num;
        UI32_T trunk_id = 0;
        UI32_T vid;
        UI32_T port_priority = 9; /*9 only for error detect */
        UI32_T state; /*spanning-tree state*/
        UI8_T  theNameOfPort[MAXSIZE_ifName+1] = {0};
        UI8_T  link_status[5] = {0}, speed[5] = {0}, duplex[5] = {0}, tag[4] = {0};
        UI8_T  mac[6] = {0}, str_trunk_id[5] = {0}, mstp_state[9] = {0};
        BOOL_T is_getted_tag = FALSE, is_getted_state = FALSE, is_inherit;
        SWCTRL_Lport_Type_T verify_ret;
        Port_Info_T port_info;
        PRI_MGR_Dot1dPortPriorityEntry_T priority_entry;
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;

        memset(&port_info, 0, sizeof(Port_Info_T));
        memset(&vlan_entry, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
        memset(&priority_entry, 0, sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));

        /* captured from Foundry's switch by ericali
        Port  Link State     Dupl Speed Trunk Tag Priori MAC            Name
        1     Up   Forward   Full 100M  1     Yes level0 0004.80ff.d000
        2     Down None      None None  1     Yes level0 0004.80ff.d000
        3     Up   Forward   Full 100M  1     Yes level0 0004.80ff.d000
        4     Down None      None None  1     Yes level0 0004.80ff.d000
        5     Down None      None None  None  No  level0 0004.80ff.d004
        6     Down None      None None  None  No  level0 0004.80ff.d005

        */

        PROCESS_MORE("Port Link State    Dupl Speed Trunk Tag Priori MAC               Name(15)\r\n");
        PROCESS_MORE("---- ---- -------- ---- ----- ----- --- ------ ----------------- ---------------\r\n");

        /* all ethernet*/
#if (SYS_CPNT_STACKING == TRUE)
        /*STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);*/
        current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
        current_max_unit = 1;
#endif
        /*for(verify_unit = 1; verify_unit <= current_max_unit; verify_unit++)*/
        for (verify_unit=0; STKTPLG_POM_GetNextUnit(&verify_unit); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

            for(verify_port = 1; verify_port <= max_port_num; verify_port++)
            {
                if(!SWCTRL_POM_UIUserPortExisting(verify_unit, verify_port))
                {
                    continue;
                }

                is_inherit = FALSE;
                verify_ret = SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &lport, &is_inherit);

                if (verify_ret == SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    continue;
                }

                if(SWCTRL_POM_GetPortInfo(lport, &port_info) == TRUE)
                {

                    /*get the name of this port*/
                    sprintf((char *)theNameOfPort, "%s", port_info.port_name);

                    /*the max length is 10 to display and exceeding portion will be omitted and replace with "..." ,
                      becasue of the limitation of display format -- the default console is 80 chars*/
                    if(strlen((char *)theNameOfPort) > 15)
                    {
                        theNameOfPort[12] = '.';
                        theNameOfPort[13] = '.';
                        theNameOfPort[14] = '.';
                        theNameOfPort[15] = '\0';
                    }

                    /*Mac Address*/
                    SWCTRL_POM_GetPortMac(lport, mac);

                    /*get trunk id*/
                    if(verify_ret == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    {
                        SWCTRL_POM_UserPortToTrunkPort(verify_unit, verify_port, &trunk_id);
                        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &lport);  /*change from port's ifindex to trunk's ifindex */
                        sprintf((char *)str_trunk_id, "%lu", (unsigned long)trunk_id);
                    }
                    else
                    {
                        sprintf((char *)str_trunk_id, "None");
                    }

                    if (port_info.link_status == SWCTRL_LINK_UP)
                    {
                        sprintf((char *)link_status, "Up");

                        /* speed and duplex config */
                        switch(port_info.speed_duplex_oper)
                        {
                            case VAL_portSpeedDpxCfg_halfDuplex10:
                                sprintf((char *)speed, "10M");
                                sprintf((char *)duplex, "Half");
                                break;

                            case VAL_portSpeedDpxCfg_fullDuplex10:
                                sprintf((char *)speed, "10M");
                                sprintf((char *)duplex, "Full");
                                break;

                            case VAL_portSpeedDpxCfg_halfDuplex100:
                                sprintf((char *)speed, "100M");
                                sprintf((char *)duplex, "Half");
                                break;

                            case VAL_portSpeedDpxCfg_fullDuplex100:
                                sprintf((char *)speed, "100M");
                                sprintf((char *)duplex, "Full");
                                break;

                            case VAL_portSpeedDpxCfg_halfDuplex1000:
                                sprintf((char *)speed, "1G");
                                sprintf((char *)duplex, "Half");
                                break;

                            case VAL_portSpeedDpxCfg_fullDuplex1000:
                                sprintf((char *)speed, "1G");
                                sprintf((char *)duplex, "Full");
                                break;

                            default:
                                break;
                        }
                    }
                    else
                    {
                        sprintf((char *)link_status, "Down");
                        sprintf((char *)speed, "None");
                        sprintf((char *)duplex, "None");
                        sprintf((char *)mstp_state, "None");
                    }
                }

                /*decide which spanning-tree state and tagged or untagged*/
                /*"Forward" > "Learning" > "Blocked"*/
                /* the port only belong to one type (either tagged or untaaged) */
                vid = 0;
                is_getted_tag = FALSE;
                is_getted_state = FALSE;
                while (VLAN_POM_GetNextDot1qVlanCurrentEntry_With_PortJoined(0, &vid, lport, &vlan_entry) == TRUE)
                {
                    if (is_getted_tag == FALSE)
                    {
                        is_getted_tag = TRUE;
                        if (VLAN_POM_IsVlanUntagPortListMember(vlan_entry.dot1q_vlan_index, lport) == TRUE)
                        {
                            sprintf((char *)tag, "No");
                        }
                        else
                        {
                            sprintf((char *)tag, "Yes");
                        }
                    }

                    if(port_info.link_status != SWCTRL_LINK_UP) /*not display mstp-state because link down*/
                    {
                        break;
                    }

                    if(XSTP_POM_GetMstPortState(lport, vid, &state) == XSTP_TYPE_RETURN_OK)
                    {
                        if(state == XSTP_TYPE_PORT_STATE_DISCARDING)
                        {
                            if (is_getted_state == FALSE)
                            {
                                sprintf((char *)mstp_state, "Blocked");
                            }
                        }
                        else if (state == XSTP_TYPE_PORT_STATE_LEARNING)
                        {
                            sprintf((char *)mstp_state, "Learning");
                        }
                        else if (state == XSTP_TYPE_PORT_STATE_FORWARDING)
                        {
                            sprintf((char *)mstp_state, "Forward");
                            break;
                        }
                        /*set the flag of getting state to be TRUE*/
                        if (is_getted_state == FALSE)
                        {
                            is_getted_state = TRUE;
                        }
                    }
                }

                /*Port Priority*/
                if(PRI_PMGR_GetDot1dPortPriorityEntry(lport, &priority_entry) == TRUE)
                {
                    port_priority = priority_entry.dot1d_port_default_user_priority;
                }

                /*display*/
                sprintf((char *)buff, "%lu/%-2lu %-4s %-8s %-4s %-5s %-5s %-3s level%lu %02X-%02X-%02X-%02X-%02X-%02X %-10s\r\n",
                          (unsigned long)verify_unit, (unsigned long)verify_port, link_status, mstp_state, duplex, speed, str_trunk_id, tag, (unsigned long)port_priority,
                          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], theNameOfPort);
                PROCESS_MORE(buff);

            } /*end for port*/
        } /*end for unit*/
    }/*end if(arg[0][0] == 'b' || arg[0][0] == 'B')*/
    return CLI_NO_ERROR;
}

static UI32_T Show_interface_port_info(UI32_T lport, UI32_T verify_unit,UI32_T  verify_port, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    Port_Info_T port_info;
    char  mac[6] = {0};
    char  hardware_type[30] = {"Unknown"};
    char  link_status[20] = {"unknown"};
    char  configured_speed[20] = {"unknown"};
    char  configured_duplex[20] = {"unknown"};
    char  actual_speed[20] = {"unknown"};
    char  actual_duplex[20] = {"unknown"};
    char  line_protocol[20] = {"unknown"};

    memset(&port_info, 0, sizeof(port_info));

    if(SWCTRL_POM_GetPortInfo(lport, &port_info) == FALSE)
    {
        return line_num;
    }

    /* hardware type */
    switch(port_info.port_type)
    {
        case VAL_portType_other:
            sprintf((char *)hardware_type, "%s", "Other");
            break;

        case VAL_portType_hundredBaseTX:
        case VAL_portType_hundredBaseFX:
        case VAL_portType_hundredBaseFxScSingleMode:
        case VAL_portType_hundredBaseFxScMultiMode:
            sprintf((char *)hardware_type, "%s", "FastEthernet");
            break;

        case VAL_portType_thousandBaseSX:
        case VAL_portType_thousandBaseLX:
        case VAL_portType_thousandBaseT:
        case VAL_portType_thousandBaseGBIC:
        case VAL_portType_thousandBaseSfp:
        case VAL_portType_thousandBaseCX:
            sprintf((char *)hardware_type, "%s", "GigabitEthernet");
            break;

        case VAL_portType_tenG:
        case VAL_portType_tenGBaseT:
        case VAL_portType_tenGBaseXFP:
        case VAL_portType_tenGBaseSFP:
            sprintf((char *)hardware_type, "%s", "TenGigabitEthernet");
            break;

        case VAL_portType_fortyGBaseQSFP:
            sprintf((char *)hardware_type, "%s", "FortyGigabitEthernet");
            break;

        default:
            break;
    }

    /* link status */
    if (port_info.link_status == SWCTRL_LINK_UP)
    {
        sprintf((char *)link_status, "%s", "up");
    }
    else
    {
        sprintf((char *)link_status, "%s", "down");
    }

    /* line protocol == port admin */
    switch(port_info.admin_state)
    {
        case VAL_ifAdminStatus_up:
            sprintf((char *)line_protocol, "%s", "up");
            break;

        case VAL_ifAdminStatus_down:
            sprintf((char *)line_protocol, "%s", "down");
            break;

        case VAL_ifAdminStatus_testing:
            sprintf((char *)line_protocol, "%s", "testing");

        default:
            break;
    }

    sprintf((char *)buff, "%s%lu is %s, line protocol is %s"NEWLINE, hardware_type, (unsigned long)verify_port, link_status, line_protocol);
    PROCESS_MORE_FUNC(buff);

    /* mac address */
    SWCTRL_POM_GetPortMac(lport, (UI8_T *)mac);
    sprintf((char *)buff, "  Hardware is %s, address is %02x%02x.%02x%02x.%02x%02x (bia %02x%02x.%02x%02x.%02x%02x)"NEWLINE, \
                                    hardware_type, \
                                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],\
                                    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    PROCESS_MORE_FUNC(buff);

    /* configured speed and duplex*/
    switch(port_info.speed_duplex_cfg)
    {
        case VAL_portSpeedDpxCfg_reserved:
            sprintf((char *)configured_speed, "%s", "reserved");
            sprintf((char *)configured_duplex, "%s", "reserved");
            break;

        case VAL_portSpeedDpxCfg_halfDuplex10:
            sprintf((char *)configured_speed, "%s", "10Mbit");
            sprintf((char *)configured_duplex, "%s", "hdx");
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10:
            sprintf((char *)configured_speed, "%s", "10Mbit");
            sprintf((char *)configured_duplex, "%s", "fdx");
            break;

        case VAL_portSpeedDpxCfg_halfDuplex100:
            sprintf((char *)configured_speed, "%s", "100Mbit");
            sprintf((char *)configured_duplex, "%s", "hdx");
            break;

        case VAL_portSpeedDpxCfg_fullDuplex100:
            sprintf((char *)configured_speed, "%s", "100Mbit");
            sprintf((char *)configured_duplex, "%s", "fdx");
            break;

        case VAL_portSpeedDpxCfg_halfDuplex1000:
            sprintf((char *)configured_speed, "%s", "1Gbit");
            sprintf((char *)configured_duplex, "%s", "hdx");
            break;

        case VAL_portSpeedDpxCfg_fullDuplex1000:
            sprintf((char *)configured_speed, "%s", "1Gbit");
            sprintf((char *)configured_duplex, "%s", "fdx");
            break;

        case VAL_portSpeedDpxCfg_halfDuplex10g:
            sprintf((char *)configured_speed, "%s", "10Gbit");
            sprintf((char *)configured_duplex, "%s", "hdx");
            break;

        case VAL_portSpeedDpxCfg_fullDuplex10g:
            sprintf((char *)configured_speed, "%s", "10Gbit");
            sprintf((char *)configured_duplex, "%s", "fdx");
            break;

        case VAL_portSpeedDpxCfg_halfDuplex40g:
            sprintf((char *)configured_speed, "%s", "40Gbit");
            sprintf((char *)configured_duplex, "%s", "hdx");
            break;

        case VAL_portSpeedDpxCfg_fullDuplex40g:
            sprintf((char *)configured_speed, "%s", "40Gbit");
            sprintf((char *)configured_duplex, "%s", "fdx");
            break;

        default:
            break;
    }

    if(port_info.autoneg_state == VAL_portAutonegotiation_enabled)
    {
        sprintf((char *)configured_speed, "%s", "auto");
    }

    /* actual speed and duplex */
    if (port_info.link_status == SWCTRL_LINK_UP)
    {
        switch(port_info.speed_duplex_oper)
        {
             case VAL_portSpeedDpxCfg_reserved:
                sprintf((char *)actual_speed, "%s", "reserved");
                sprintf((char *)actual_duplex, "%s", "reserved");
                break;

            case VAL_portSpeedDpxCfg_halfDuplex10:
                sprintf((char *)actual_speed, "%s", "10Mbit");
                sprintf((char *)actual_duplex, "%s", "hdx");
                break;

            case VAL_portSpeedDpxCfg_fullDuplex10:
                sprintf((char *)actual_speed, "%s", "10Mbit");
                sprintf((char *)actual_duplex, "%s", "fdx");
                break;

            case VAL_portSpeedDpxCfg_halfDuplex100:
                sprintf((char *)actual_speed, "%s", "100Mbit");
                sprintf((char *)actual_duplex, "%s", "hdx");
                break;

            case VAL_portSpeedDpxCfg_fullDuplex100:
                sprintf((char *)actual_speed, "%s", "100Mbit");
                sprintf((char *)actual_duplex, "%s", "fdx");
                break;

            case VAL_portSpeedDpxCfg_halfDuplex1000:
                sprintf((char *)actual_speed, "%s", "1Gbit");
                sprintf((char *)actual_duplex, "%s", "hdx");
                break;

            case VAL_portSpeedDpxCfg_fullDuplex1000:
                sprintf((char *)actual_speed, "%s", "1Gbit");
                sprintf((char *)actual_duplex, "%s", "fdx");
                break;

            case VAL_portSpeedDpxCfg_halfDuplex10g:
                sprintf((char *)actual_speed, "%s", "10Gbit");
                sprintf((char *)actual_duplex, "%s", "hdx");
                break;

            case VAL_portSpeedDpxCfg_fullDuplex10g:
                sprintf((char *)actual_speed, "%s", "10Gbit");
                sprintf((char *)actual_duplex, "%s", "fdx");
                break;

            case VAL_portSpeedDpxCfg_halfDuplex40g:
                sprintf((char *)configured_speed, "%s", "40Gbit");
                sprintf((char *)configured_duplex, "%s", "hdx");
                break;

            case VAL_portSpeedDpxCfg_fullDuplex40g:
                sprintf((char *)configured_speed, "%s", "40Gbit");
                sprintf((char *)configured_duplex, "%s", "fdx");
                break;

            default:
                break;
        }
    }

    sprintf((char *)buff, "  Configured speed %s, actual %s, configured duplex %s, actual %s"NEWLINE, \
                        configured_speed, actual_speed, configured_duplex, actual_duplex);

    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_interface_vlan_info(UI32_T lport, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]  = {0};

    UI32_T vid = 0;
    UI32_T state = 0;
    UI32_T vlan[256] = {0};
    char  *vlan_tag[SYS_ADPT_MAX_NBR_OF_VLAN];  //remove warning ,change type from UI8_T * to char *
    char  *vlan_stp_state[SYS_ADPT_MAX_NBR_OF_VLAN];/*changed by Jinhua Wei ,to remove warning ,becaued the type isn't match with where it is used*/
    UI32_T i, idx = 0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;

    memset(&vlan_entry, 0, sizeof(vlan_entry));
    memset(&vlan, 0, sizeof(vlan));
    for(i = 0; i < SYS_ADPT_MAX_NBR_OF_VLAN; i++)
    {
        vlan_tag[i] = "unknown";
        vlan_stp_state[i] = "unknown";
    }

    while(VLAN_POM_GetNextDot1qVlanCurrentEntry_With_PortJoined(0, &vid, lport, &vlan_entry) == TRUE)
    {
        /* vid */
        vlan[idx] = vid;

        /* tag */
        if(VLAN_POM_IsVlanUntagPortListMember(vlan_entry.dot1q_vlan_index, lport) == TRUE)
        {
            vlan_tag[idx] = "untagged";
        }
        else
        {
            vlan_tag[idx ] = "tagged";
        }

        /* port state */
        if(XSTP_POM_GetPortStateByVlan(vid, lport, &state) == TRUE)
        {
            switch(state)
            {
                case XSTP_TYPE_PORT_STATE_DISABLED:
                    vlan_stp_state[idx] = "DISABLED";
                    break;

                case XSTP_TYPE_PORT_STATE_BLOCKING: /* same as XSTP_TYPE_PORT_STATE_DISCARDING */
                    vlan_stp_state[idx] = "BLOCKING";
                    break;

                case XSTP_TYPE_PORT_STATE_LISTENING:
                    vlan_stp_state[idx] = "LISTENING";
                    break;

                case XSTP_TYPE_PORT_STATE_LEARNING:
                    vlan_stp_state[idx] = "LEARNING";
                    break;

                case XSTP_TYPE_PORT_STATE_FORWARDING:
                    vlan_stp_state[idx] = "FORWARDING";
                    break;

                case XSTP_TYPE_PORT_STATE_BROKEN:
                    vlan_stp_state[idx] = "BROKEN";
                    break;

                default:
                    break;
            }
        }

        idx++;
    }

    for (i = 0; i < idx; i++)
    {
        sprintf((char *)buff, "  Member of L2 VLAN ID %lu, port is %s, port state is %s"NEWLINE, (unsigned long)vlan[i], vlan_tag[i], vlan_stp_state[i]);
        PROCESS_MORE_FUNC(buff);
    }

    return line_num;
}

static UI32_T Show_interface_stp(UI32_T lport, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]  = {0};
    UI32_T stp_status = 0;
    char  stp[20] = {"unknown"};
    UI32_T port_priority;
    char  flow_control[30] = {"unknown"};
    PRI_MGR_Dot1dPortPriorityEntry_T priority_entry;
    Port_Info_T port_info;

    memset(&priority_entry, 0, sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));
    memset(&port_info, 0, sizeof(port_info));

    /* STP */
    if(XSTP_POM_GetPortSpanningTreeStatus(lport,&stp_status) == TRUE)
    {
        switch(stp_status)
        {
            case VAL_staPortSystemStatus_enabled:
                sprintf((char *)stp, "%s", "ON");
                break;

            case VAL_staPortSystemStatus_disabled:
                sprintf((char *)stp, "%s", "OFF");
                break;

            default:
                break;
        }
    }
    sprintf((char *)buff, "  STP configured to %s,", stp);

    /* 802.1p priority*/
    port_priority = -1;
    if(PRI_PMGR_GetDot1dPortPriorityEntry(lport, &priority_entry) == TRUE)
    {
        port_priority =  priority_entry.dot1d_port_default_user_priority;
    }

    if(port_priority < 0)
    {
        sprintf((char *)buff+strlen((char *)buff), " priority is level %s,", "unknown");
    }
    else
    {
        sprintf((char *)buff+strlen((char *)buff), " priority is level%lu,", (unsigned long)port_priority);
    }

    /* flow control */
    if(SWCTRL_POM_GetPortInfo(lport, &port_info) == TRUE)
    {
        switch(port_info.flow_control_cfg)
        {
            case VAL_portFlowCtrlCfg_enabled:
                sprintf((char *)flow_control, "%s", "enabled");
                break;

            case VAL_portFlowCtrlCfg_disabled:
                sprintf((char *)flow_control, "%s", "disabled");
                break;

            default:
                break;
        }
    }
    sprintf((char *)buff+strlen((char *)buff), " flow control %s"NEWLINE, flow_control);

    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_interface_mirror(UI32_T lport, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]  = {0};

    SWCTRL_MirrorEntry_T mirror_entry;
    char  monitor[10] = {"disabled"};    /* source */
    char  mirror[10] = {"disabled"};      /* destination */

    memset(&mirror_entry , 0 , sizeof(mirror_entry));

    while (SWCTRL_POM_GetNextMirrorEntry(&mirror_entry))
    {
        if(lport == mirror_entry.mirror_destination_port)
            strcpy((char *)mirror, "enabled");

        if(lport == mirror_entry.mirror_source_port)
            strcpy((char *)monitor, "enabled");
    }

    sprintf((char *)buff, "  mirror %s, monitor %s"NEWLINE, mirror, monitor);
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_interface_trunk(UI32_T lport, UI32_T line_num, UI32_T verify_unit, UI32_T verify_port)
{
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    TRK_MGR_TrunkEntry_T trunk_entry;
    BOOL_T is_a_trunk_port;
    char  status[200];
    char  port_list[200];
    UI32_T ports_of_trunk[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK];
    UI32_T port_count;
    UI32_T i;

    /* active trunks */
    memset(&trunk_entry, 0, sizeof(trunk_entry));
    memset(status, 0, sizeof(status));
    memset(port_list, 0, sizeof(port_list));
    memset(ports_of_trunk, 0, sizeof(ports_of_trunk));

    trunk_entry.trunk_index = 0;
    is_a_trunk_port = FALSE;
    port_count = 0;
    while(TRK_PMGR_GetNextRunningTrunkEntry(&trunk_entry) == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
    {
        port_count = 0;
        memset(ports_of_trunk, 0, sizeof(ports_of_trunk));
        is_a_trunk_port = FALSE;

        for (i = 1; i <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST *8); i++)
        {
            if(trunk_entry.trunk_ports[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                ports_of_trunk[port_count] = i;
                if(i == lport)
                {
                    is_a_trunk_port = TRUE;
                }

                port_count++;
            }
        }

        if(is_a_trunk_port == TRUE)
            break;
    }

    if(port_count > 0)
    {
        Show_interface_trunk_port_sequence(ports_of_trunk, (UI8_T *)port_list, (UI8_T *)status, port_count, verify_port);
    }

    if(is_a_trunk_port == TRUE)
    {
        sprintf((char *)buff, "  Member of active trunk ports %s, %s"NEWLINE, port_list, status);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        sprintf((char *)buff, "  Not member of any active trunks"NEWLINE);
        PROCESS_MORE_FUNC(buff);
    }

    /* configured trunks */
    memset(&trunk_entry, 0, sizeof(trunk_entry));
    memset(status, 0, sizeof(status));
    memset(port_list, 0, sizeof(port_list));
    memset(ports_of_trunk, 0, sizeof(ports_of_trunk));

    trunk_entry.trunk_index = 0;
    is_a_trunk_port = FALSE;
    port_count = 0;
    while(TRK_PMGR_GetNextTrunkEntry(&trunk_entry) == TRUE)
    {
        port_count = 0;
        memset(ports_of_trunk, 0, sizeof(ports_of_trunk));
        is_a_trunk_port = FALSE;

        for (i = 1; i <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST *8); i++)
        {
            if(trunk_entry.trunk_ports[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                ports_of_trunk[port_count] = i;
                if(i == lport)
                {
                    is_a_trunk_port = TRUE;
                }

                port_count++;
            }
        }

        if(is_a_trunk_port == TRUE)
            break;
    }

    if(port_count > 0)
    {
        Show_interface_trunk_port_sequence(ports_of_trunk, (UI8_T *)port_list, (UI8_T *)status, port_count, verify_port);
    }

    if(is_a_trunk_port == TRUE)
    {
        sprintf((char *)buff, "  Member of configured trunk ports %s, %s"NEWLINE, port_list, status);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        sprintf((char *)buff, "  Not member of any configured trunks"NEWLINE);
        PROCESS_MORE_FUNC(buff);
    }
    return line_num;
}

static void Show_interface_trunk_port_sequence(UI32_T *ports_of_trunk, UI8_T *port_list, UI8_T *status, UI32_T port_count, UI32_T verify_port)
{
    /* port sequence */
    BOOL_T first = TRUE;
    UI32_T flag = 0;
    UI32_T i;

    if(port_count < 1)
    {
        return;
    }

    port_list[0] = '\0';
    if(ports_of_trunk[port_count-1] == ports_of_trunk[0]+port_count-1)
    {
        sprintf((char *)port_list, "%lu-%lu", (unsigned long)ports_of_trunk[0], (unsigned long)ports_of_trunk[port_count-1]);
    }
    else
    {
        for(i = 0; i < port_count; i++)
        {
            if(first == TRUE)
            {
                sprintf((char *)port_list+strlen((char *)port_list), "%lu-", (unsigned long)ports_of_trunk[i]);
                flag = ports_of_trunk[i];
                first = FALSE;
            }

            if(ports_of_trunk[i+1] == ports_of_trunk[i]+1)
            {
                first = FALSE;
                continue;
            }
            else
            {
                if(flag == ports_of_trunk[i])
                {
                    port_list[strlen((char *)port_list)-1] = '\0';
                    sprintf((char *)port_list+strlen((char *)port_list), ",");
                    first = TRUE;
                    continue;
                }
                sprintf((char *)port_list+strlen((char *)port_list), "%lu,", (unsigned long)ports_of_trunk[i]);
                first = TRUE;
            }
        }
            port_list[strlen((char *)port_list) - 1] = '\0';
    }

    /* primary or secondary port */
    if(verify_port == ports_of_trunk[0])
    {
        strcpy((char *)status, "primary port");
    }
    else
    {
        sprintf((char *)status, "secondary port, primary port is %lu", (unsigned long)ports_of_trunk[0]);
    }
}

static UI32_T Show_interface_portname(UI32_T lport, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    Port_Info_T port_info;
    char  port_name[MAXSIZE_ifName+1] = {"unknown"};

    memset(&port_info, 0, sizeof(port_info));
    if(SWCTRL_POM_GetPortInfo(lport, &port_info) == TRUE)
    {
        sprintf((char *)port_name, "%s", port_info.port_name);
    }

    if(strlen((char *)port_name) > 0)
    {
        sprintf((char *)buff, "  Port name is %s"NEWLINE, port_name);
    }
    else
    {
        sprintf((char *)buff, "  No port name"NEWLINE);
    }

    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_interface_traffic(UI32_T lport, UI32_T line_num)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    IF_MGR_IfEntry_T  if_entry;
    char  mtu[20] = {"unknown"};
    CLI_API_ShowIfCounter_T IfCounter;

    NMTR_MGR_Utilization_300_SECS_T cpu_utilization;
    UI32_T whole_number = 0;
    UI32_T fraction = 0;
#if 0 /* current kernel already supports 64 bits calculation */
    UI32_T         temp8;
    UI64_T         temp4,temp10;
    UI64_T         *temp5;

    temp5=&temp10;
#endif
    memset(&if_entry, 0, sizeof(if_entry));
    memset(&IfCounter, 0, sizeof(IfCounter));
    memset(&cpu_utilization, 0, sizeof(cpu_utilization));

    /* MTU */
    if_entry.if_index = lport;
    if (IF_PMGR_GetIfEntry(&if_entry) == TRUE)
    {
        sprintf((char *)mtu, "%lu", (unsigned long)if_entry.if_mtu);
    }

    sprintf((char *)buff, "  MTU %s bytes, encapsulation ethernet"NEWLINE, mtu);
    PROCESS_MORE_FUNC(buff);

    /* port utilization */
    if(NMTR_PMGR_GetPortUtilization300secs(lport, &cpu_utilization) == TRUE)
    {
        whole_number = cpu_utilization.ifInOctets_utilization / 100;
        fraction = cpu_utilization.ifInOctets_utilization % 100;
        sprintf(buff, "  300 second input rate: ");
        sprintf(buff+strlen(buff), "%llu bits/sec, %llu packets/sec, %3lu.%02lu%% utilization"NEWLINE, (unsigned long long)(cpu_utilization.ifInOctets*8), (unsigned long long)cpu_utilization.ifInPackets, (unsigned long)whole_number, (unsigned long)fraction);
        PROCESS_MORE_FUNC(buff);

        whole_number = cpu_utilization.ifOutOctets_utilization / 100;
        fraction = cpu_utilization.ifOutOctets_utilization % 100;
        sprintf(buff, "  300 second output rate: ");
        sprintf(buff+strlen(buff), "%llu bits/sec, %llu packets/sec, %3lu.%02lu%% utilization"NEWLINE, (unsigned long long)(cpu_utilization.ifOutOctets*8), (unsigned long long)cpu_utilization.ifOutPackets, (unsigned long)whole_number, (unsigned long)fraction);
        PROCESS_MORE_FUNC(buff);
#if 0 /* current kernel already supports 64 bits calculation */
        whole_number = cpu_utilization.ifInOctets_utilization / 100;
        fraction = cpu_utilization.ifInOctets_utilization % 100;
        sprintf((char *)buff, "  300 second input rate: ");
        temp4 = cpu_utilization.ifInOctets;
        temp8=8;
        *temp5=temp4;
        L_STDLIB_UI64_Multi(&(L_STDLIB_UI64_H32(temp5)), &(L_STDLIB_UI64_L32(temp5)), temp8);
        temp4=*temp5;

        if( (((UI32_T*)&cpu_utilization.ifInOctets)[0]==0) && (((UI32_T*)&cpu_utilization.ifInPackets)[0]==0) )
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu bits/sec, %lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE,((UI32_T*)&temp4)[1], ((UI32_T*)&cpu_utilization.ifInPackets)[1], whole_number, fraction);
        }
        else if((((UI32_T*)&cpu_utilization.ifInOctets)[0]==0) && (((UI32_T*)&cpu_utilization.ifInPackets)[0]!=0))
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu bits/sec, %lu%lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[1], ((UI32_T*)&cpu_utilization.ifInPackets)[0],((UI32_T*)&cpu_utilization.ifInPackets)[1], whole_number, fraction);
        }
        else if((((UI32_T*)&cpu_utilization.ifInOctets)[0]!=0) && (((UI32_T*)&cpu_utilization.ifInPackets)[0]==0))
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu%lu bits/sec, %lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[0], ((UI32_T*)&temp4)[1],((UI32_T*)&cpu_utilization.ifInPackets)[1], whole_number, fraction);
        }
        else
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu%lu bits/sec, %lu%lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[0], ((UI32_T*)&temp4)[1],((UI32_T*)&cpu_utilization.ifInPackets)[0],((UI32_T int*)&cpu_utilization.ifInPackets)[1], whole_number, fraction);
        }
        PROCESS_MORE_FUNC(buff);

        whole_number = cpu_utilization.ifOutOctets_utilization / 100;
        fraction = cpu_utilization.ifOutOctets_utilization % 100;
        sprintf((char *)buff, "  300 second output rate: ");
        PROCESS_MORE_FUNC(buff);

        temp4 = cpu_utilization.ifOutOctets;
        temp8=8;
        *temp5=temp4;
        L_STDLIB_UI64_Multi(&(L_STDLIB_UI64_H32(temp5)), &(L_STDLIB_UI64_L32(temp5)), temp8);
        temp4=*temp5;

        if ((((UI32_T*)&cpu_utilization.ifOutOctets)[0]==0) && (((UI32_T*)&cpu_utilization.ifOutPackets)[0]==0))
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu bits/sec, %lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE,((UI32_T*)&temp4)[1], ((UI32_T*)&cpu_utilization.ifOutPackets)[1], whole_number, fraction);
        }
        else if((((UI32_T*)&cpu_utilization.ifOutOctets)[0]==0) && (((UI32_T*)&cpu_utilization.ifOutPackets)[0]!=0))
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu bits/sec, %lu%lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[1], ((UI32_T*)&cpu_utilization.ifOutPackets)[0],((UI32_T*)&cpu_utilization.ifOutPackets)[1], whole_number, fraction);
        }
        else if((((UI32_T*)&cpu_utilization.ifOutOctets)[0]!=0) && (((UI32_T*)&cpu_utilization.ifOutPackets)[0]==0))
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu%lu bits/sec, %lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[0], ((UI32_T*)&temp4)[1],((UI32_T*)&cpu_utilization.ifOutPackets)[1], whole_number, fraction);
        }
        else
        {
            sprintf((char *)buff+strlen((char *)buff), "%lu%lu bits/sec, %lu%lu packets/sec, %3lu.%02lu%%%% utilization"NEWLINE, ((UI32_T*)&temp4)[0], ((UI32_T*)&temp4)[1],((UI32_T*)&cpu_utilization.ifOutPackets)[0],((UI32_T*)&cpu_utilization.ifOutPackets)[1], whole_number, fraction);
        }
        PROCESS_MORE_FUNC(buff);
#endif
    }

    /* traffic */
    if(GetIfCounter(lport, &IfCounter) == TRUE)
    {
        UI32_T rx_unicast = 0;
        UI32_T rx_nunicast = 0;
        UI32_T rx_total_pkts = 0;
        UI32_T rx_total_bytes = 0;
        UI32_T rx_multicast = 0;
        UI32_T rx_broadcast = 0;
        /*UI32_T rx_no_buffer = 0;*/        /* not implemented yet */
        UI32_T rx_total_errors = 0;
        UI32_T rx_crc_errors = 0;
        /*UI32_T rx_frame_errors = 0;*/         /* not implemented yet */
        UI32_T rx_drop_events = 0;
        /*UI32_T rx_runts = 0;*/            /* not implemented yet */
        UI32_T rx_giant_pkts = 0;
        /*UI32_T rx_DMA_received = 0;*/ /* not implemented yet */

        UI32_T tx_unicast = 0;
        UI32_T tx_nunicast = 0;
        UI32_T tx_total_pkts = 0;
        UI32_T tx_total_bytes = 0;
        /*UI32_T tx_underruns = 0; */    /* not implemented yet */
        UI32_T tx_multicast = 0;
        UI32_T tx_broadcast = 0;
        UI32_T tx_total_errors = 0;
        UI32_T tx_collisions = 0;
        /*UI32_T tx_DMA_transmitted = 0;*/ /* not implemented yet */

        /* traffic received */
        rx_unicast = IfCounter.if_table_stats.ifInUcastPkts;
        rx_nunicast = IfCounter.if_table_stats.ifInNUcastPkts;
        rx_total_pkts = rx_unicast + rx_nunicast;
        rx_total_bytes = IfCounter.if_table_stats.ifInOctets;
        rx_multicast = IfCounter.if_xtable_stats.ifInMulticastPkts;
        rx_broadcast = IfCounter.if_xtable_stats.ifInBroadcastPkts;
        rx_total_errors = IfCounter.if_table_stats.ifInErrors;
        rx_crc_errors = IfCounter.ether_like_stats.dot3StatsFCSErrors;
        rx_drop_events = IfCounter.if_table_stats.ifInDiscards;
        rx_giant_pkts = IfCounter.ether_like_stats.dot3StatsFrameTooLongs;

        sprintf((char *)buff, "  %lu packets input, %lu bytes"NEWLINE, \
                            (unsigned long)rx_total_pkts, (unsigned long)rx_total_bytes);
        PROCESS_MORE_FUNC(buff);

        sprintf((char *)buff, "  Received %lu broadcasts, %lu multicasts, %lu unicasts"NEWLINE, \
                            (unsigned long)rx_broadcast, (unsigned long)rx_multicast, (unsigned long)rx_unicast);
        PROCESS_MORE_FUNC(buff);

        sprintf((char *)buff, "  %lu input errors, %lu CRC, %lu ignore"NEWLINE, \
                            (unsigned long)rx_total_errors, (unsigned long)rx_crc_errors, (unsigned long)rx_drop_events);
        PROCESS_MORE_FUNC(buff);

        sprintf((char *)buff, "  %lu giants"NEWLINE,\
                            (unsigned long)rx_giant_pkts);
        PROCESS_MORE_FUNC(buff);

        /* traffic transmitted */
        tx_unicast = IfCounter.if_table_stats.ifOutUcastPkts;
        tx_nunicast = IfCounter.if_table_stats.ifOutNUcastPkts;
        tx_total_pkts = tx_unicast + tx_nunicast;
        tx_total_bytes = IfCounter.if_table_stats.ifOutOctets;
        tx_multicast = IfCounter.if_xtable_stats.ifOutMulticastPkts;
        tx_broadcast = IfCounter.if_xtable_stats.ifOutBroadcastPkts;
        tx_total_errors = IfCounter.if_table_stats.ifOutErrors;
        tx_collisions = IfCounter.ether_like_stats.dot3StatsSingleCollisionFrames + \
                            IfCounter.ether_like_stats.dot3StatsMultipleCollisionFrames + \
                            IfCounter.ether_like_stats.dot3StatsLateCollisions + \
                            IfCounter.ether_like_stats.dot3StatsExcessiveCollisions;

        sprintf((char *)buff, "  %lu packets output, %lu bytes"NEWLINE, \
                            (unsigned long)tx_total_pkts, (unsigned long)tx_total_bytes);
        PROCESS_MORE_FUNC(buff);

        sprintf((char *)buff, "  Transmitted %lu broadcaste, %lu multicasts, %lu unicasts"NEWLINE,\
                            (unsigned long)tx_broadcast, (unsigned long)tx_multicast, (unsigned long)tx_unicast);
        PROCESS_MORE_FUNC(buff);

        sprintf((char *)buff, "  %lu output errors, %lu collisions"NEWLINE, \
                            (unsigned long)tx_total_errors, (unsigned long)tx_collisions);
        PROCESS_MORE_FUNC(buff);
    }
    return line_num;
}


UI32_T CLI_API_Show_Memory(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    SYS_MGR_MemoryUtilBrief_T sys_mem_brief;
    UI32_T free_percentage;

    if (!SYS_PMGR_GetMemoryUtilizationBrief(&sys_mem_brief))
    {
        return CLI_NO_ERROR;
    }

    if (!SYS_PMGR_GetMemoryUtilFreePercentage(&free_percentage))
    {
        return CLI_NO_ERROR;
    }

    PROCESS_MORE(" Status Bytes      %\r\n");
    PROCESS_MORE(" ------ ---------- ---\r\n");
    sprintf(buff, " Free   %10lu %3lu\r\n",
                       (unsigned long)sys_mem_brief.free_bytes,
                       (unsigned long)free_percentage);
    PROCESS_MORE(buff);
    sprintf(buff, " Used   %10lu %3lu\r\n",
                       (unsigned long)sys_mem_brief.used_bytes,
                       (unsigned long)(100 - free_percentage));
    PROCESS_MORE(buff);
    sprintf(buff, " Total  %10lu\r\n",
                       (unsigned long)(sys_mem_brief.free_bytes + sys_mem_brief.used_bytes));
    PROCESS_MORE(buff);
    PROCESS_MORE("\r\n");

    /* alarm config */
    {
        UI32_T rising_threshold, falling_threshold;

        SYS_PMGR_GetMemoryUtilRisingThreshold(&rising_threshold);
        SYS_PMGR_GetMemoryUtilFallingThreshold(&falling_threshold);

        PROCESS_MORE(" Alarm Configuration\r\n");

        sprintf(buff, "  Rising Threshold         : %lu%%\r\n", (unsigned long)rising_threshold);
        PROCESS_MORE(buff);
        sprintf(buff, "  Falling Threshold        : %lu%%\r\n", (unsigned long)falling_threshold);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");

#else
    MEM_SIZE_T total_bytes, free_bytes;

    /* the total_bytes returned by SYSFUN_GetMemoryUsage() means system heap size
     * but the spec of total size here is DRAM size.
     */
    if (SYSFUN_GetMemoryUsage(&total_bytes, &free_bytes) != SYSFUN_OK)
    {
        return CLI_NO_ERROR;
    }

    PROCESS_MORE(" Status Bytes\r\n");
    PROCESS_MORE(" ------ ----------\r\n");
    PROCESS_MORE_1(" Free   %10lu\r\n",
                       (unsigned long)free_bytes);
    PROCESS_MORE_1(" Used   %10lu\r\n",
                       (unsigned long)(SYS_HWCFG_DRAM_SIZE - free_bytes));
    PROCESS_MORE_1(" Total  %10lu\r\n",
                       (unsigned long)SYS_HWCFG_DRAM_SIZE);
    PROCESS_MORE("\r\n");
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Process_CPU(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

    /* current */
    {
        UI32_T total_cpu_percentage = 0, float_cpu_percentage = 0;

        SYS_PMGR_GetCpuUsagePercentage(&total_cpu_percentage, &float_cpu_percentage);

        PROCESS_MORE_1(" CPU Utilization in the past 5 seconds : %lu%%\r\n", (unsigned long)total_cpu_percentage);
    }
    PROCESS_MORE("\r\n");

    /* recent 60s */
    {
        UI32_T cpu_util_max, cpu_util_avg;

        SYS_PMGR_GetCpuUsageMaximum(&cpu_util_max);
        SYS_PMGR_GetCpuUsageAverage(&cpu_util_avg);

        PROCESS_MORE(" CPU Utilization in the past 60 seconds\r\n");

        sprintf(buff, "  Average Utilization      : %lu%%\r\n", (unsigned long)cpu_util_avg);
        PROCESS_MORE(buff);
        sprintf(buff, "  Maximum Utilization      : %lu%%\r\n", (unsigned long)cpu_util_max);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");

    /* alarm status */
    {
        char cpu_util_peak_time_str[MAXSIZE_cpuPeakTime + 1] = { 0 };
        UI32_T cpu_util_peak_time, cpu_util_peak_duration;
        BOOL_T alarm_status;

        SYS_PMGR_GetCpuUtilAlarmStatus(&alarm_status);
        SYS_PMGR_GetCpuUsagePeakTime(&cpu_util_peak_time);
        SYS_PMGR_GetCpuUsagePeakDuration(&cpu_util_peak_duration);

        if (cpu_util_peak_time)
        {
            SYS_TIME_ConvertTime(cpu_util_peak_time, cpu_util_peak_time_str);
        }

        PROCESS_MORE(" Alarm Status\r\n");

        sprintf(buff, "  Current Alarm Status     : %s\r\n", alarm_status ? "On" : "Off");
        PROCESS_MORE(buff);
        sprintf(buff, "  Last Alarm Start Time    : %s\r\n", cpu_util_peak_time_str);
        PROCESS_MORE(buff);
        sprintf(buff, "  Last Alarm Duration Time : %lu seconds\r\n", (unsigned long)cpu_util_peak_duration);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");

    /* alarm config */
    {
        UI32_T rising_threshold, falling_threshold;

        SYS_PMGR_GetCpuUtilRisingThreshold(&rising_threshold);
        SYS_PMGR_GetCpuUtilFallingThreshold(&falling_threshold);

        PROCESS_MORE(" Alarm Configuration\r\n");

        sprintf(buff, "  Rising Threshold         : %lu%%\r\n", (unsigned long)rising_threshold);
        PROCESS_MORE(buff);
        sprintf(buff, "  Falling Threshold        : %lu%%\r\n", (unsigned long)falling_threshold);
        PROCESS_MORE(buff);
    }
    PROCESS_MORE("\r\n");
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Cpu_Guard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

    SYS_MGR_CpuGuardInfo_T info;

    if (SYS_PMGR_GetCpuGuardInfo(&info))
    {
        PROCESS_MORE(" CPU Guard Configuration\r\n");
        sprintf(buff,"  Status            : %s\r\n", info.status ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);
        sprintf(buff,"  High Watermark    : %lu%%\r\n", (unsigned long)info.watermark_high);
        PROCESS_MORE(buff);
        sprintf(buff,"  Low Watermark     : %lu%%\r\n", (unsigned long)info.watermark_low);
        PROCESS_MORE(buff);
        sprintf(buff,"  Maximum Threshold : %lupps\r\n", (unsigned long)info.threshold_max);
        PROCESS_MORE(buff);
        sprintf(buff,"  Minimum Threshold : %lupps\r\n", (unsigned long)info.threshold_min);
        PROCESS_MORE(buff);
        sprintf(buff,"  Trap Status       : %s\r\n", info.trap_status ? "Enabled" : "Disabled");
        PROCESS_MORE(buff);
        PROCESS_MORE(" CPU Guard Operation\r\n");
        sprintf(buff,"  Current Threshold : %lupps\r\n", (unsigned long)info.cpu_rate);
        PROCESS_MORE(buff);
        PROCESS_MORE("\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Process_CPU_Task(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK == TRUE)
    char   buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

    SYS_MGR_TaskCpuUtilInfo_T cpu_util_info;
    int tbl_name_len, tbl_util_len;
    char str_line[] = "-------------------------";

    tbl_name_len = sizeof(cpu_util_info.task_name)-1;
    tbl_util_len = 8;

    sprintf(buff, "%-*.*s %-*.*s %-*.*s %-*.*s\r\n",
        tbl_name_len, tbl_name_len, "Task",
        tbl_util_len, tbl_util_len, "Util (%)",
        tbl_util_len, tbl_util_len, "Avg (%)",
        tbl_util_len, tbl_util_len, "Max (%)");
    PROCESS_MORE(buff);
    sprintf(buff, "%-*.*s %-*.*s %-*.*s %-*.*s\r\n",
        tbl_name_len, tbl_name_len, str_line,
        tbl_util_len, tbl_util_len, str_line,
        tbl_util_len, tbl_util_len, str_line,
        tbl_util_len, tbl_util_len, str_line);
    PROCESS_MORE(buff);

    cpu_util_info.task_name[0] = 0;

    while (SYS_PMGR_GetCpuUtilByName(&cpu_util_info, TRUE))
    {
        sprintf(buff, "%-*.*s %*lu.%02lu %*lu.%02lu %*lu.%02lu\r\n",
            tbl_name_len, tbl_name_len, cpu_util_info.task_name,
            tbl_util_len-3, (unsigned long)cpu_util_info.cpu_util, (unsigned long)(cpu_util_info.cpu_util_float/10),
            tbl_util_len-3, (unsigned long)cpu_util_info.cpu_util_avg, (unsigned long)(cpu_util_info.cpu_util_avg_float/10),
            tbl_util_len-3, (unsigned long)cpu_util_info.cpu_util_max, (unsigned long)(cpu_util_info.cpu_util_max_float/10));
        PROCESS_MORE(buff);
    }
#endif
    return CLI_NO_ERROR;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_TechSupport
 * ------------------------------------------------------------------------
 * PURPOSE  :   This carries out the command "show tech-support".
 *
 * INPUT    :   cmd_idx - CLI command index
 *              arg     - pointer to CLI comand argument array
 *              ctrl_P  - pointer to CLI working area
 *
 * OUTPUT   :   prints output to CLI session
 *
 * RETURN   :   CLI_NO_ERROR    - no error
 *
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
UI32_T CLI_API_Show_TechSupport(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_CLI_SHOW_TECH_SUPPORT == TRUE)
    UI16_T cmd;
    char *argument[CLI_DEF_MAX_ARGS_NUM+1];
    UI32_T i;

    for (i = 0; i < CLI_DEF_MAX_ARGS_NUM+1; i++)
    {
        argument[i] = NULL;
    }

    /* begin: turn print interactive mode ("MORE...") off
     */
    CLI_API_Set_Print_Interactive_Mode(FALSE);

    SYSFUN_Sleep(10);

    /* dir
     */
    CLI_LIB_PrintStr((NEWLINE"dir:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W1_DIR;
    argument[0] = NULL;
    CLI_API_Dir(cmd, argument, ctrl_P);

    /* show arp
     */
    CLI_LIB_PrintStr((NEWLINE"show arp:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_ARP;
    argument[0] = NULL;
#if (CLI_SUPPORT_L3_FEATURE != 1)
    CLI_API_Show_Arp(cmd, argument, ctrl_P);
#else
    CLI_API_L3_Show_Arp(cmd, argument, ctrl_P);
#endif

    SYSFUN_Sleep(10);

    /* show interfaces brief
     */
    CLI_LIB_PrintStr((NEWLINE"show interfaces brief:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_INTERFACES_BRIEF;
    argument[0] = NULL;
    CLI_API_Show_Interfaces_Brief(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show interfaces counters
     */
    CLI_LIB_PrintStr((NEWLINE"show interfaces counters:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_INTERFACES_COUNTERS;
    argument[0] = NULL;
    CLI_API_Show_Interfaces_Counters(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show interfaces status
     */
    CLI_LIB_PrintStr((NEWLINE"show interfaces status:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_INTERFACES_STATUS;
    argument[0] = NULL;
    CLI_API_Show_Interfaces_Status(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show interfaces switchport
     */
    CLI_LIB_PrintStr((NEWLINE"show interfaces switchport:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_INTERFACES_SWITCHPORT;
    argument[0] = NULL;
    CLI_API_Show_Interfaces_Switchport(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show interfaces transceiver
     */
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    CLI_LIB_PrintStr((NEWLINE"show interfaces transceiver:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_INTERFACES_TRANSCEIVER;
    argument[0] = NULL;
    CLI_API_Show_Interfaces_Transceiver(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif

    /* show log flash
     */
    CLI_LIB_PrintStr((NEWLINE"show log flash:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_LOG;
    argument[0] = "flash";
    argument[1] = NULL;
    CLI_API_Show_Log(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show log ram
     */
    CLI_LIB_PrintStr((NEWLINE"show log ram:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_LOG;
    argument[0] = "ram";
    argument[1] = NULL;
    CLI_API_Show_Log(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show logging flash
     */
    CLI_LIB_PrintStr((NEWLINE"show logging flash:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_LOGGING;
    argument[0] = "flash";
    argument[1] = NULL;
    CLI_API_Show_Logging(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show logging ram
     */
    CLI_LIB_PrintStr((NEWLINE"show logging ram:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_LOGGING;
    argument[0] = "ram";
    argument[1] = NULL;
    CLI_API_Show_Logging(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show logging trap
     */
    CLI_LIB_PrintStr((NEWLINE"show logging trap:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_LOGGING;
    argument[0] = "trap";
    argument[1] = NULL;
    CLI_API_Show_Logging(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

#if (SYS_CPNT_AMTR_UI == TRUE)
    /* show mac-address-table
     */
    CLI_LIB_PrintStr((NEWLINE"show mac-address-table:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_MACADDRESSTABLE;
    argument[0] = NULL;
    CLI_API_Show_MacAddressTable(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif /* #if (SYS_CPNT_AMTR_UI == TRUE) */

    /* show mac-address-table multicast
     *
     * Past: This command was designed for MAC multicast.
     * Because for the user, IGMP Snooping works for IP addresses,
     * and we do not have a ¡§MAC Multicast¡¨ feature,
     * the Linux platform does not have this command any more.
     */

    /* show memory
     */
#if (SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION == TRUE)
    CLI_LIB_PrintStr((NEWLINE"show memory:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_MEMORY;
    argument[0] = NULL;
    CLI_API_Show_Memory(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif

    /* show port security
     *
     * Platform: Even though almost all platforms have Port Security,
     * not all platforms have a separate "show port security" command.
     * Old platforms print it in "show interfaces status".
     */
#if (SYS_CPNT_PORT_SECURITY == TRUE)
    CLI_LIB_PrintStr((NEWLINE"show port security:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_PORT_SECURITY;
    argument[0] = NULL;
    CLI_API_Show_Port_Security(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif

    /* show process cpu
     */
#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
    CLI_LIB_PrintStr((NEWLINE"show process cpu:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W3_SHOW_PROCESS_CPU;
    argument[0] = NULL;
    CLI_API_Show_Process_CPU(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif

    /* show running-config
     */
    CLI_LIB_PrintStr((NEWLINE"show running-config:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_RUNNINGCONFIG;
    argument[0] = NULL;
    CLI_API_Show_Runningconfig(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

#if (SYS_CPNT_SNTP == TRUE)
    /* show sntp
     */
    CLI_LIB_PrintStr((NEWLINE"show sntp:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_SNTP;
    argument[0] = NULL;
    CLI_API_Show_Sntp(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);
#endif  /* #if (SYS_CPNT_SNTP == TRUE) */

    /* show spanning-tree
     */
    CLI_LIB_PrintStr((NEWLINE"show spanning-tree:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_SPANNINGTREE;
    argument[0] = NULL;
    CLI_API_Show_Spanningtree(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show spanning-tree brief
     */
    CLI_LIB_PrintStr((NEWLINE"show spanning-tree brief:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_SPANNINGTREE;
    argument[0] = "brief";
    argument[1] = NULL;
    CLI_API_Show_Spanningtree(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show spanning-tree interface
     *
     * Future: Currently, "show spanning-tree" already includes interface
     * information.  But if, in the future, we split the commands
     * "show spanning-tree" (globals) and "show spanning-tree interface
     * [ethernet <unit>/<port> | port-channel <pc-id>]",
     * then we will need a separate call to "show spanning-tree interface".
     */

    /* show spanning-tree mst <instance-id>
     */
#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    {
        UI32_T mstid = 0;
        UI8_T arg_buf[10] = {0};
        CLI_LIB_PrintStr((NEWLINE"show spanning-tree mst:"NEWLINE));
        cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_SPANNINGTREE;
        argument[0] = "mst";

        while (XSTP_POM_GetNextExistedInstance(&mstid) == TRUE)
        {
            CLI_LIB_PrintStr_1(NEWLINE"Mst: %lu"NEWLINE, (unsigned long)mstid);
            sprintf((char*)arg_buf, "%lu", (unsigned long)mstid);
            argument[1] = (char *)arg_buf;
            argument[2] = NULL;
            CLI_API_Show_Spanningtree(cmd, argument, ctrl_P);

            SYSFUN_Sleep(10);
        }
    }
#endif

    /* show system
     */
    CLI_LIB_PrintStr((NEWLINE"show system:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_SYSTEM;
    argument[0] = NULL;
    CLI_API_Show_System(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show version
     */
    CLI_LIB_PrintStr((NEWLINE"show version:"NEWLINE));
    cmd =  PRIVILEGE_EXEC_CMD_W2_SHOW_VERSION;
    argument[0] = NULL;
    CLI_API_Show_Version(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* show vlan
     */
    CLI_LIB_PrintStr((NEWLINE"show vlan:"NEWLINE));
    cmd = PRIVILEGE_EXEC_CMD_W2_SHOW_VLAN;
    argument[0] = NULL;
    CLI_API_Show_Vlan(cmd, argument, ctrl_P);

    SYSFUN_Sleep(10);

    /* end: turn print interactive mode ("MORE...") back on
     */
    CLI_API_Set_Print_Interactive_Mode(TRUE);
#endif  /* #if (SYS_CPNT_CLI_SHOW_TECH_SUPPORT == TRUE) */

    return CLI_NO_ERROR;
}


BOOL_T CLI_API_Get_Print_Interactive_Mode()
{
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
    if(ctrl_P == NULL)
        return FALSE;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    CLI_LIB_SwWatchDogRoutine(ctrl_P);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */

    return ctrl_P->print_interactive;
}

void CLI_API_Set_Print_Interactive_Mode(BOOL_T mode)
{
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();

    ctrl_P->print_interactive = mode;
}

/*
 * FUNCTION NAME: print_config_buffer
 * PURPOSE: Print running/startup configuration buffer.
 * INPUT:   buffer_p -- Configuration buffer.
 *          line_num -- PROCESS_MORE line number.
 * RETURN:  line_num -¡V PROCESS_MORE line number after processing.
 * NOTES:   Configuration buffer size = SYS_ADPT_CLI_MAX_CONFIG_SIZE.
 */
static UI32_T print_config_buffer(char* buffer_p, UI32_T line_num)
{
    char* current_p = buffer_p;
    char* next_p = buffer_p;
    char  buff[CLI_DEF_MAX_BUFSIZE+3]; /* For PROCESS_MORE */

    memset(buff, 0, sizeof(buff));

    while(next_p < (buffer_p + SYS_ADPT_CLI_MAX_CONFIG_SIZE))
    {
        switch(*next_p)
        {
            case '\0':
                return line_num;
            case '\n':
                *next_p = '\0';
                snprintf((char*)buff, sizeof(buff), "%s\r\n", current_p);
                buff[sizeof(buff)-1] = '\0';
                PROCESS_MORE_FUNC(buff);
                current_p = ++next_p;
                break;
            default:
                next_p++;
                break;
        }
    }

    return line_num;
}

#if (SYS_CPNT_POWER_SAVE == TRUE)
UI32_T CLI_API_Show_PowerSave(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_POWER_SAVE == TRUE)
    UI32_T  line_num = 0;
    UI32_T  lport = 0;
    UI32_T  verify_unit = 0;
    UI32_T  verify_port = 0;
    UI32_T  current_max_unit = 1;
    UI32_T  max_port_num = 0;
    UI32_T  start_unit, start_port;
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};
    CLI_API_EthStatus_T verify_ret;
    BOOL_T power_save_status=0;
    BOOL_T is_inherit;
    BOOL_T show_all;

    PROCESS_MORE("Power Saving Status:\r\n");

    /* show power saving interface */
    if (arg[0][0] == 'I' ||arg[0][0] == 'i')
    {
        /* set unit and port if have specified interface */
        if(arg[1] != NULL && (arg[1][0] == 'e' || arg[1][0] == 'E'))
        {
            start_unit = atoi(arg[2]);
            start_port = atoi(strchr(arg[2], '/') + 1);
            show_all = FALSE;
        }
        else
        {
            start_unit = 0;
            STKTPLG_POM_GetNextUnit(&start_unit);
            start_port = 1;
            show_all = TRUE;
        }

#if (SYS_CPNT_STACKING == TRUE)
        current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
        current_max_unit = 1;
#endif

        verify_unit = start_unit;

        do
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);
            for(verify_port = start_port; verify_port <= max_port_num; verify_port++) /* port loop */
            {
                if(!SWCTRL_POM_UIUserPortExisting(verify_unit, verify_port))
                {
                    continue;
                }

                is_inherit=FALSE;
                verify_ret = SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &lport, &is_inherit);
                if (verify_ret == SWCTRL_LPORT_UNKNOWN_PORT || verify_ret == SWCTRL_LPORT_TRUNK_PORT)
                {
                    continue;
                }

                /* check port status */
                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                if ((verify_ret == CLI_API_ETH_NOT_PRESENT) || (verify_ret == CLI_API_ETH_UNKNOWN_PORT))
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    continue;
                }

                /* get power save status */
                if (!(SWCTRL_PMGR_GetPortPowerSaveStatus(verify_port, &power_save_status)))
                {
                    sprintf((char *)buff, " Ethernet %lu/%-2lu : %s\r\n", (unsigned long)verify_unit, (unsigned long)verify_port, "Not support");
                    PROCESS_MORE(buff);
                }
                /* show power save status */
                else
                {
                    sprintf((char *)buff, " Ethernet %lu/%-2lu : %s\r\n", (unsigned long)verify_unit, (unsigned long)verify_port, (power_save_status == VAL_portPowerSave_enabled)? "Enabled" : "Disabled");
                    PROCESS_MORE(buff);
                }

                if (!show_all)
                {
                    break;
                }
            }

            if (!show_all)
            {
                break;
            }
        }
        while (STKTPLG_POM_GetNextUnit(&verify_unit));
    }
    else
    {
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_POWER_SAVE == TRUE) */

   return CLI_NO_ERROR;
}
#endif

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
static char *history_convert_tick_2_str(UI32_T ticks, char *buf)
{
    UI32_T days, hours, minutes, seconds, miliseconds;
    UI32_T  tmp;

    miliseconds = ticks % SYS_BLD_TICKS_PER_SECOND;
    tmp = ticks / SYS_BLD_TICKS_PER_SECOND;      /* in seconds   */

    seconds = tmp % 60;
    tmp = tmp / 60;         /* in minutes   */

    minutes = tmp % 60;
    tmp = tmp / 60;         /* in hours     */

    hours = tmp % 24;
    days = tmp / 24;       /* in days      */

    sprintf(buf, "%02lud %02lu:%02lu:%02lu", (unsigned long)days, (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds);

    return buf;
}

#define HIST_ARG_TIME   BIT_0
#define HIST_ARG_IOCTET BIT_1
#define HIST_ARG_IPKT   BIT_2
#define HIST_ARG_IERR   BIT_3
#define HIST_ARG_OOCTET BIT_4
#define HIST_ARG_OPKT   BIT_5
#define HIST_ARG_OERR   BIT_6

static char *history_convert_entry_2_str(NMTR_TYPE_HistSampleEntry_T *entry_p, unsigned int arg_bmp, char *buf)
{
    char time_str[13];
    char octets_str[23];

    if (arg_bmp & HIST_ARG_TIME)
        history_convert_tick_2_str(entry_p->counter.start_time, time_str);
    else
        time_str[0] = 0;

    if (arg_bmp & HIST_ARG_IOCTET)
        sprintf(octets_str, "%3lu.%02lu %15llu",
            (unsigned long)(entry_p->counter.ifInUtilization.basis_point / 100),
            (unsigned long)(entry_p->counter.ifInUtilization.basis_point % 100),
            (unsigned long long)entry_p->counter.ifInOctets);
    else if (arg_bmp & HIST_ARG_OOCTET)
        sprintf(octets_str, "%3lu.%02lu %15llu",
            (unsigned long)(entry_p->counter.ifOutUtilization.basis_point / 100),
            (unsigned long)(entry_p->counter.ifOutUtilization.basis_point % 100),
            (unsigned long long)entry_p->counter.ifOutOctets);
    else
        octets_str[0] = 0;

    if (arg_bmp & HIST_ARG_IPKT)
    {
        sprintf(buf, " %-12s %22s %13llu %13llu %13llu\r\n", time_str, octets_str,
                (unsigned long long)entry_p->counter.ifInUcastPkts,
                (unsigned long long)entry_p->counter.ifInMulticastPkts,
                (unsigned long long)entry_p->counter.ifInBroadcastPkts);
    }
    else if (arg_bmp & HIST_ARG_IERR)
    {
        sprintf(buf, " %-12s %13llu %13llu %13llu\r\n", time_str,
                (unsigned long long)entry_p->counter.ifInDiscards,
                (unsigned long long)entry_p->counter.ifInErrors,
                (unsigned long long)entry_p->counter.ifInUnknownProtos);
    }
    else if (arg_bmp & HIST_ARG_OPKT)
    {
        sprintf(buf, " %-12s %22s %13llu %13llu %13llu\r\n", time_str, octets_str,
                (unsigned long long)entry_p->counter.ifOutUcastPkts,
                (unsigned long long)entry_p->counter.ifOutMulticastPkts,
                (unsigned long long)entry_p->counter.ifOutBroadcastPkts);
    }
    else if (arg_bmp & HIST_ARG_OERR)
    {
        sprintf(buf, " %-12s %13llu %13llu\r\n", time_str,
                (unsigned long long)entry_p->counter.ifOutDiscards,
                (unsigned long long)entry_p->counter.ifOutErrors);
    }

    return buf;
}
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

UI32_T CLI_API_Show_Interfaces_History(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
    UI32_T              line_num = 0;
    char                buff[CLI_DEF_MAX_BUFSIZE] = {0};

    UI32_T  ifindex;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  verify_vid;
    UI32_T  arg_chk_idx = 0;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    char ifindex_str[10];
    BOOL_T show_all_ctrl_entry, show_current, show_previous, show_input, show_output;
    UI32_T previous_index, previous_start, previous_count;
    NMTR_TYPE_HistCtrlInfo_T ctrl_info;
    NMTR_TYPE_HistSampleEntry_T entry;

    /* show interfaces
     *     history [<interface> [<name>
     *         [current | previous <index> <count>] [input | output]]]
     */
    if (arg[arg_chk_idx])
    {
        switch(arg[arg_chk_idx][0])
        {
            case 'e':
            case 'E':
                verify_unit = atoi(arg[arg_chk_idx+1]);
                verify_port = atoi(strchr(arg[arg_chk_idx+1], '/') + 1);

                verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex);
                if (verify_ret_e != CLI_API_ETH_OK && verify_ret_e != CLI_API_ETH_TRUNK_MEMBER)
                {
                    display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                    return CLI_NO_ERROR;
                }
                sprintf(ifindex_str, "Eth%2lu/%2lu", (unsigned long)verify_unit, (unsigned long)verify_port);
                break;

            case 'p':
            case 'P':
                verify_trunk_id = atoi(arg[arg_chk_idx+1]);
                if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                    return CLI_NO_ERROR;
                }
                sprintf(ifindex_str, "Trunk %2lu", (unsigned long)verify_trunk_id);
                break;

            case 'v':
            case 'V':
                verify_vid = atoi(arg[arg_chk_idx+1]);
                VLAN_VID_CONVERTTO_IFINDEX(verify_vid, ifindex);
                if (FALSE == VLAN_POM_IsVlanExisted(verify_vid))
                {
                    CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)verify_vid);
                    return CLI_NO_ERROR;
                }
                sprintf(ifindex_str, "VLAN %4lu", (unsigned long)verify_vid);
                break;

            default:
                return CLI_ERR_INTERNAL;
        }
        arg_chk_idx += 2;

        show_all_ctrl_entry = show_current = show_previous = show_input = show_output = TRUE;
        previous_start = 1;
        previous_count = NMTR_TYPE_HIST_CTRL_BUCKETS_MAX;

        memset(&ctrl_info, 0, sizeof(ctrl_info));
        ctrl_info.data_source = ifindex;

        if (arg[arg_chk_idx])
        {
            strncpy(ctrl_info.name, arg[arg_chk_idx++], sizeof(ctrl_info.name));
            ctrl_info.name[sizeof(ctrl_info.name)-1] = 0;

            show_all_ctrl_entry = FALSE;
        }

        while (arg[arg_chk_idx])
        {
            switch (arg[arg_chk_idx++][0])
            {
                case 'c':
                case 'C':
                    show_previous = FALSE;
                    break;

                case 'p':
                case 'P':
                    show_current = FALSE;
                    if (arg[arg_chk_idx])
                    {
                        previous_start = atoi(arg[arg_chk_idx++]);
                    }
                    if (arg[arg_chk_idx])
                    {
                        previous_count = atoi(arg[arg_chk_idx++]);
                    }
                    break;

                case 'i': /* show input only */
                case 'I':
                    show_output = FALSE;
                    break;

                case 'o': /* show output only */
                case 'O':
                    show_input = FALSE;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }

        if (show_all_ctrl_entry)
        {
            show_previous = FALSE;
        }

        while (NMTR_PMGR_GetHistoryCtrlEntryByDataSrc(&ctrl_info, show_all_ctrl_entry))
        {
            sprintf(buff, "Interface         : %s\r\n", ifindex_str);
            PROCESS_MORE(buff);
            sprintf(buff, "Name              : %s\r\n", ctrl_info.name);
            PROCESS_MORE(buff);
            sprintf(buff, "Interval          : %lu second(s)\r\n", (unsigned long)ctrl_info.interval);
            PROCESS_MORE(buff);
            sprintf(buff, "Buckets Requested : %lu\r\n", (unsigned long)ctrl_info.buckets_requested);
            PROCESS_MORE(buff);
            sprintf(buff, "Buckets Granted   : %lu\r\n", (unsigned long)ctrl_info.buckets_granted);
            PROCESS_MORE(buff);
            sprintf(buff, "Status            : %s\r\n", ctrl_info.status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE ? "Active" : "Inactive");
            PROCESS_MORE(buff);
            PROCESS_MORE("\r\n");

            if (show_current)
            {
                PROCESS_MORE("Current Entries\r\n");
                PROCESS_MORE("\r\n");

                entry.ctrl_idx = ctrl_info.ctrl_idx;
                if (NMTR_PMGR_GetHistoryCurrentEntry(&entry, FALSE))
                {
                    if (show_input)
                    {
                        PROCESS_MORE(" Start Time   %      Octets Input    Unicast       Multicast     Broadcast\r\n");
                        PROCESS_MORE(" ------------ ------ --------------- ------------- ------------- -------------\r\n");
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_IOCTET|HIST_ARG_IPKT, buff));
                        PROCESS_MORE("\r\n");

                        PROCESS_MORE("              Discards      Errors        Unknown Proto\r\n");
                        PROCESS_MORE("              ------------- ------------- -------------\r\n");
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_IERR, buff));
                        PROCESS_MORE("\r\n");
                    }

                    if (show_output)
                    {
                        if (show_input)
                        {
                            PROCESS_MORE("              %      Octets Output   Unicast       Multicast     Broadcast\r\n");
                            PROCESS_MORE("              ------ --------------- ------------- ------------- -------------\r\n");
                            PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_OOCTET|HIST_ARG_OPKT, buff));
                        }
                        else
                        {
                            PROCESS_MORE(" Start Time   %      Octets Output   Unicast       Multicast     Broadcast\r\n");
                            PROCESS_MORE(" ------------ ------ --------------- ------------- ------------- -------------\r\n");
                            PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_OOCTET|HIST_ARG_OPKT, buff));
                        }
                        PROCESS_MORE("\r\n");

                        PROCESS_MORE("              Discards      Errors\r\n");
                        PROCESS_MORE("              ------------- -------------\r\n");
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_OERR, buff));
                        PROCESS_MORE("\r\n");
                    }
                }
            } /* end of if (show_current) */

            if (show_previous)
            {
                PROCESS_MORE("Previous Entries\r\n");
                PROCESS_MORE("\r\n");

                if (show_input)
                {
                    PROCESS_MORE(" Start Time   %      Octets Input    Unicast       Multicast     Broadcast\r\n");
                    PROCESS_MORE(" ------------ ------ --------------- ------------- ------------- -------------\r\n");

                    entry.ctrl_idx = ctrl_info.ctrl_idx;
                    entry.sample_idx = 0;

                    for (previous_index = 1;
                        previous_index < previous_start + previous_count && NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(&entry);
                        previous_index++)
                    {
                        if (previous_index < previous_start)
                            continue;
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_IOCTET|HIST_ARG_IPKT, buff));
                    }
                    PROCESS_MORE("\r\n");

                    PROCESS_MORE(" Start Time   Discards      Errors        Unknown Proto\r\n");
                    PROCESS_MORE(" ------------ ------------- ------------- -------------\r\n");

                    entry.ctrl_idx = ctrl_info.ctrl_idx;
                    entry.sample_idx = 0;

                    for (previous_index = 1;
                        previous_index < previous_start + previous_count && NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(&entry);
                        previous_index++)
                    {
                        if (previous_index < previous_start)
                            continue;
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_IERR, buff));
                    }
                    PROCESS_MORE("\r\n");
                }

                if (show_output)
                {
                    PROCESS_MORE(" Start Time   %      Octets Output   Unicast       Multicast     Broadcast\r\n");
                    PROCESS_MORE(" ------------ ------ --------------- ------------- ------------- -------------\r\n");

                    entry.ctrl_idx = ctrl_info.ctrl_idx;
                    entry.sample_idx = 0;

                    for (previous_index = 1;
                        previous_index < previous_start + previous_count && NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(&entry);
                        previous_index++)
                    {
                        if (previous_index < previous_start)
                            continue;
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_OOCTET|HIST_ARG_OPKT, buff));
                    }
                    PROCESS_MORE("\r\n");

                    PROCESS_MORE(" Start Time   Discards      Errors\r\n");
                    PROCESS_MORE(" ------------ ------------- -------------\r\n");

                    entry.ctrl_idx = ctrl_info.ctrl_idx;
                    entry.sample_idx = 0;

                    for (previous_index = 1;
                        previous_index < previous_start + previous_count && NMTR_PMGR_GetNextHistoryPreviousEntryByCtrlIdx(&entry);
                        previous_index++)
                    {
                        if (previous_index < previous_start)
                            continue;
                        PROCESS_MORE(history_convert_entry_2_str(&entry, HIST_ARG_TIME|HIST_ARG_OERR, buff));
                    }
                    PROCESS_MORE("\r\n");
                }
            } /* end of if (show_previous) */

            if (!show_all_ctrl_entry)
            {
                break;
            }
        } /* end of show a ctrl entry */
    }
    else /* show brief */
    {
        PROCESS_MORE("Interface Name                            Interval (s) Buckets Status\r\n");
        PROCESS_MORE("--------- ------------------------------- ------------ ------- --------\r\n");

        for (ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
        {
            memset(&ctrl_info, 0, sizeof(ctrl_info));
            ctrl_info.data_source = ifindex;

            while (NMTR_PMGR_GetHistoryCtrlEntryByDataSrc(&ctrl_info, TRUE))
            {
                switch (SWCTRL_POM_LogicalPortToUserPort(ifindex, &verify_unit, &verify_port, &verify_trunk_id))
                {
                    case SWCTRL_LPORT_NORMAL_PORT:
                    case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
                        sprintf(ifindex_str, "Eth%2lu/%2lu ", (unsigned long)verify_unit, (unsigned long)verify_port);
                        break;
                    case SWCTRL_LPORT_TRUNK_PORT:
                        sprintf(ifindex_str, "Trunk %2lu ", (unsigned long)verify_trunk_id);
                        break;
                    default:
                        continue;
                }

                sprintf(buff, "%-9s %-31s %12lu %3lu/%3lu %-8s\r\n",
                    ifindex_str,
                    ctrl_info.name,
                    (unsigned long)ctrl_info.interval,
                    (unsigned long)ctrl_info.buckets_granted,
                    (unsigned long)ctrl_info.buckets_requested,
                    ctrl_info.status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE ? "Active" : "Inactive");
                PROCESS_MORE(buff);
            }
        } /* end of for (ifindex) */

        PROCESS_MORE("\r\n");
    }
#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

    return CLI_NO_ERROR;
}
