
#if 0 /* rich */
#include "rs.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "l_cvrt.h"
#include "l_stdlib.h"
#include "l_math.h"
#include "cli_api.h"
#include "cli_api_vlan.h"
#include "netcfg_pmgr_ip.h"

#include "vlan_om.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_type.h"
#include "netcfg_type.h"

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#include "rspan_om.h"

#define CLI_PRINT_RSPAN_VLAN_HEADER \
        { \
            CLI_LIB_PrintStr("Remote SPAN VLANs\r\n"); \
            CLI_LIB_PrintStr("------------------------------------------------\r\n"); \
        }

#endif

#define CLI_API_VLAN_CHK_SWDOG(ctl, tout)                               \
            {       UI32_T  tcur;                                       \
                    tcur = SYSFUN_GetSysTick();                         \
                    if (L_MATH_TimeOut32(tcur, tout))                   \
                    {                                                   \
                        CLI_LIB_SwWatchDogRoutine(ctl);                 \
                        tout = tcur + (((SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT) - 2)/*min*/*60 * SYS_BLD_TICKS_PER_SECOND);  \
                    }   \
            }


static I32_T CLI_API_VlanStaticMemberCount(UI32_T vid);
static void CLI_API_GetPortPosition(UI32_T lport_ifindex, UI32_T *byte, UI32_T *shift);

static UI32_T vlan_database_vlan_id;
static UI32_T vlan_database_vlan_type;
static UI8_T vlan_database_vlan_name[16];

/*****************************************<<VLAN>>*******************************************/
/*change mode*/
UI32_T CLI_API_Interface_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI8_T  previous_port_id_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = {0};
    UI32_T previous_vlan  = 0;
    UI32_T previous_trunk = 0;
    UI32_T previous_loopback = 0;
    UI32_T ret;

    /*store previous interface information, and init working space*/
    memcpy(previous_port_id_list, ctrl_P->CMenu.port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
    previous_vlan = ctrl_P->CMenu.vlan_ifindex;
    previous_trunk = ctrl_P->CMenu.pchannel_id;
    previous_loopback = ctrl_P->CMenu.loopback_id;
    memset(ctrl_P->CMenu.port_id_list, 0, sizeof(ctrl_P->CMenu.port_id_list));
    ctrl_P->CMenu.vlan_ifindex = 0;
    ctrl_P->CMenu.pchannel_id = 0;
    ctrl_P->CMenu.loopback_id = 0;

    if(arg[0]==NULL)
        return CLI_ERR_INTERNAL;

#if(SYS_CPNT_CLUSTER == TRUE)
    {
        UI32_T vid=0;
        vid = atoi(arg[0]);

        if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
        {
            CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
            CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
        }
    }
#endif

    if(!VLAN_POM_IsVlanExisted((UI32_T)atoi((char*)arg[0])))
    {
        /*restore previous setting*/
        memcpy(ctrl_P->CMenu.port_id_list, previous_port_id_list, sizeof(ctrl_P->CMenu.port_id_list));
        ctrl_P->CMenu.vlan_ifindex = previous_vlan;
        ctrl_P->CMenu.pchannel_id = previous_trunk;
        ctrl_P->CMenu.loopback_id = previous_loopback;
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
    }

    if ((ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W2_INTERFACE_VLAN)
#if (CLI_SUPPORT_INTERFACE_TO_INTERFACE == 1)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_VLAN_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_INTERFACE_VLAN)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_INTERFACE_VLAN)
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_LOOPBACK_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_LOOPBACK_CMD_W2_INTERFACE_VLAN)
#endif
#if (SYS_CPNT_CRAFT_PORT == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_CRAFT_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_CRAFT_CMD_W2_INTERFACE_VLAN)
#endif
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
     || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_ETH0_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W2_INTERFACE_VLAN)
#endif
#if (SYS_CPNT_TRUNK_UI == TRUE)
	 || (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_INTERFACE_PCHANNEL_MODE && cmd_idx == PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_INTERFACE_VLAN)
#endif
#endif
        )
    {
        if ((ret = NETCFG_PMGR_IP_CreateL3If((UI32_T)atoi((char*)arg[0]))) != NETCFG_TYPE_OK)
        {
            if (ret == NETCFG_TYPE_MAC_COLLISION)
                CLI_LIB_PrintStr("Failed, MAC collision. Please clear MAC address table and try again.\r\n");
            else if(ret == NETCFG_TYPE_TABLE_FULL)
                CLI_LIB_PrintStr("Failed, interface table is full.\r\n");
            else
                CLI_LIB_PrintStr("Fail to change interface type.\r\n");
            return CLI_NO_ERROR;
        }

        VLAN_VID_CONVERTTO_IFINDEX((UI32_T)atoi((char*)arg[0]), ctrl_P->CMenu.vlan_ifindex);
        ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_INTERFACE_VLAN_MODE;
    }
#if (SYS_CPNT_ROUTING == TRUE || SYS_CPNT_MULTIPLE_MGMT_IP == TRUE)
    else if (ctrl_P->CMenu.AccMode == PRIVILEGE_CFG_GLOBAL_MODE && cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_INTERFACE_VLAN)
    {
        if (NETCFG_PMGR_IP_DeleteL3If((UI32_T)atoi((char*)arg[0])) != NETCFG_TYPE_OK)
        {
             CLI_LIB_PrintStr("Fail to change interface type.\r\n");
             return CLI_NO_ERROR;
        }
    }
#endif
    else
    {
            return CLI_ERR_CMD_INVALID;
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Vlan_Database(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==TRUE)
    UI32_T vlan_id;
    UI8_T vlan_name[16]={0};
    UI32_T vid_ifindex;
    UI32_T vlan_row_status;
    BOOL_T  ret;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

    memset(&vlan_info,0,sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_VLAN:

            if(arg[0]!=NULL)
            {
                vlan_id = atoi(arg[0]);
                CLI_API_SET_VLAN_DATABASE_VLANID(vlan_id);
                VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);
                vlan_info.dot1q_vlan_index = vid_ifindex;

                if(arg[1]==NULL||arg[1][0]=='b'||arg[1][0]=='B')
                {
                    if(arg[1][0]=='b'||arg[1][0]=='B')
                    {
                        if((arg[2]!=NULL)&&(arg[2][0]=='p'||arg[2][0]=='P'))
                        {
                            ctrl_P->CMenu.vlan_database_type = CLI_TYPE_BY_VLAN_PORT;
                        }
                        else
                        {
                            return CLI_NO_ERROR;
                        }
                    }
                    else
                    {
                        ctrl_P->CMenu.vlan_database_type = CLI_TYPE_BY_NULL;
                    }
                    CLI_API_SET_VLAN_DATABASE_VLAN_TYPE(ctrl_P->CMenu.vlan_database_type,vlan_name);
                    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_VLAN_DATABASE_MODE;

                    if (!VLAN_POM_IsVlanExisted(vlan_id)      ||
                            !VLAN_POM_GetVlanEntry(&vlan_info) ||
                            vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent)                /* not exist or exist but dynamic => create vlan first */
                    {
                        if (!VLAN_PMGR_CreateVlan(vlan_id, VAL_dot1qVlanStatus_permanent))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to create vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                            return CLI_NO_ERROR;
                        }
                    }
                    vlan_row_status = VAL_dot1qVlanStaticRowStatus_active;
                    ret = VLAN_PMGR_SetDot1qVlanStaticRowStatus(vlan_id, vlan_row_status);
                    if (!ret)
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set rowstatus of vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                    }
                }
                else
                {
                    if(arg[1][0]=='n'||arg[1][0]=='N')
                    {
                        if(arg[2]!=NULL)
                        {
                            ctrl_P->CMenu.vlan_database_type = CLI_TYPE_BY_VLAN_NAME;
                            strcpy(vlan_name,arg[2]);
                            CLI_API_SET_VLAN_DATABASE_VLAN_TYPE(ctrl_P->CMenu.vlan_database_type,vlan_name);
                            ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_VLAN_DATABASE_MODE;

                            VLAN_VID_CONVERTTO_IFINDEX(vlan_id, vid_ifindex);
                            vlan_info.dot1q_vlan_index = vid_ifindex;

                            if (!VLAN_POM_IsVlanExisted(vlan_id)      ||
                                    !VLAN_POM_GetVlanEntry(&vlan_info) ||
                                    vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent)                /* not exist or exist but dynamic => create vlan first */
                            {
                                if (!VLAN_PMGR_CreateVlan(vlan_id, VAL_dot1qVlanStatus_permanent))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to create vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                                    return CLI_NO_ERROR;
                                }
                            }

                            if (!VLAN_PMGR_SetDot1qVlanStaticName(vlan_id,vlan_name))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to set name vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                                return CLI_NO_ERROR;
                            }
                            vlan_row_status = VAL_dot1qVlanStaticRowStatus_active;
                            ret = VLAN_PMGR_SetDot1qVlanStaticRowStatus(vlan_id, vlan_row_status);
                            if (!ret)
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to set rowstatus of vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                            }
                        }
                        else
                        {
                            return CLI_NO_ERROR;
                        }
                    }
                    else
                    {
                        return CLI_NO_ERROR;
                    }
                }
            }

            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_VLAN:

            if(arg[0]!=NULL)
                vlan_id = atoi(arg[0]);

            if (!VLAN_POM_IsVlanExisted(vlan_id))
            {
                if(arg[1]!=NULL) /*attempt to remove the vlan name*/
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("No such vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                    return CLI_NO_ERROR;
                }
            }
            else
            {
                if(!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vlan_id, VAL_dot1qVlanStaticRowStatus_notInService))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to suspend VLAN %lu\r\n", (unsigned long)vlan_id);
#endif
                }
            }

            /*remove vlan*/
            if (!VLAN_PMGR_DeleteNormalVlan(vlan_id, VAL_dot1qVlanStatus_permanent))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("Failed to remove vlan %lu\r\n", (unsigned long)vlan_id);
#endif
                return CLI_NO_ERROR;
            }

            break;

        default:
            break;
    } //end switch

#else
    ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_VLAN_DATABASE_MODE;
#endif
    return CLI_NO_ERROR;
}

static UI32_T show_one_vlan(UI32_T vid, UI32_T line_num);

/*execution*/
UI32_T CLI_API_Show_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T vid      = 0;
    UI32_T line_num = 0;

/* Athena: I think it does need the show the default VLAN info. One reason is default VLAN can be changed in 08-series.
   if (VLAN_MGR_GetGlobalDefaultVlan(&vid) == TRUE)
    {
        CLI_LIB_PrintStr_1("Default VLAN ID : %ld\r\n", vid);
        CLI_LIB_PrintStr("\r\n");
        line_num += 2;
    }
*/

    vid = 0;
    if (arg[0] == NULL) /* show all */
    {
        UI32_T time_mark = 0;

        while(VLAN_POM_GetNextVlanId(time_mark, &vid))
        {
#if (SYS_CPNT_RSPAN == TRUE)
            /* show normal vlan first
             */
            if (RSPAN_OM_IsRspanVlan(vid) == TRUE)
                continue;
#endif

            if ((line_num = show_one_vlan(vid, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }

#if (SYS_CPNT_RSPAN == TRUE)
        /* show RSPAN VLAN
         */
        CLI_PRINT_RSPAN_VLAN_HEADER;

        vid = 0;
        line_num += 2;
        while (VLAN_POM_GetNextVlanId(time_mark, &vid))
        {
            if (RSPAN_OM_IsRspanVlan(vid) == FALSE)
                continue;
            if((line_num = show_one_vlan(vid, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
        }
        CLI_LIB_PrintStr("\r\n");
#endif
    }
    else
    {
        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        switch(arg[0][0])
        {
            case 'i':
            case 'I':
                {
                    vid = (UI32_T)atoi((char*)arg[1]);

                    if (VLAN_POM_IsVlanExisted(vid))
                    {
#if (SYS_CPNT_RSPAN == TRUE)
                        if (RSPAN_OM_IsRspanVlan(vid) == TRUE)
                        {
                            /* show RSPAN VLAN
                             */
                            CLI_PRINT_RSPAN_VLAN_HEADER;
                        }
#endif
                        show_one_vlan(vid, line_num);
                    }
                    else
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
                    }
                }
                break;

            case 'n':
            case 'N':
            {
                BOOL_T found = FALSE;
                VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
                UI32_T time_mark = 0;

                vid = 0;

                while (VLAN_POM_GetNextVlanId(time_mark, &vid))
                {
                    UI32_T time_mark2 = 0;

                    memset(&vlan_info, 0, sizeof(vlan_info));
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
                    VLAN_POM_GetDot1qVlanCurrentEntryAgent(time_mark2, vid, &vlan_info);
#else
                    VLAN_POM_GetDot1qVlanCurrentEntry(time_mark2, vid, &vlan_info);
#endif
                    if (strcmp((char*)arg[1], (char*)vlan_info.dot1q_vlan_static_name) == 0)
                    {
                        found = TRUE;
#if (SYS_CPNT_RSPAN == TRUE)
                        if (RSPAN_OM_IsRspanVlan(vid) == TRUE)
                        {
                            /* show RSPAN VLAN
                             */
                            CLI_PRINT_RSPAN_VLAN_HEADER;
                        }
#endif
                        if ((line_num = show_one_vlan(vid, line_num)) == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                    }
                }

                if (found)
                {
                    //show_one_vlan(vid, line_num);
                }
                else
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
                }
            }
            break;


        default:
            return CLI_ERR_INTERNAL;
        }
    }

    return CLI_NO_ERROR;
}


static UI32_T show_one_vlan(UI32_T vid, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  arBitList[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};

    UI32_T time_mark = 0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

    //UI32_T unit_id;
    UI32_T max_port_num;
    /*pttch stacking*/
    UI32_T current_max_unit = 0;
    UI32_T j;

    BOOL_T is_first_line = TRUE;
    UI32_T port_counter = 0;
    UI32_T trunk_counter = 0;

#if (SYS_CPNT_STACKING == TRUE)
    //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif
    //STKTPLG_MGR_GetMyUnitID(&unit_id);
    //max_port_num = SWCTRL_UIGetUnitPortNumber(unit_id);
    PROCESS_MORE_FUNC("\r\n");

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
    if(VLAN_POM_GetDot1qVlanCurrentEntryAgent(time_mark, vid, &vlan_info))
#else
    if(VLAN_POM_GetDot1qVlanCurrentEntry(time_mark, vid, &vlan_info))
#endif
    {
        #define VLAN_NAME_MAX_CHAR_ONE_LINE 50  /*one line display 50 character*/
        CLI_API_EthStatus_T verify_ret;
        UI32_T i;

        UI32_T ifindex;
        UI32_T vid_ifindex;

        UI32_T trunk_id;
        UI32_T trunk_ifindex;

        UI32_T verify_unit;
        UI32_T verify_port;
        UI16_T str_len=0;
        char tmp[VLAN_NAME_MAX_CHAR_ONE_LINE+1]={0};

        /*type, name, status*/

        CLI_LIB_PrintStr_1("VLAN ID             : %lu\r\n",(unsigned long)vid);
        CLI_LIB_PrintStr_1("Type                : %s\r\n",vlan_info.dot1q_vlan_status == VAL_dot1qVlanStatus_permanent ? "Static" : "Dynamic");

        str_len = strlen(vlan_info.dot1q_vlan_static_name);
        CLI_LIB_PrintStr("Name                : ");
        if (0 == str_len)
        {
            CLI_LIB_PrintStr("\r\n");
        }
        else
        {
            for(i=0; i<str_len; i+=VLAN_NAME_MAX_CHAR_ONE_LINE)
            {
                strncpy(tmp, &vlan_info.dot1q_vlan_static_name[i], VLAN_NAME_MAX_CHAR_ONE_LINE);
                tmp[VLAN_NAME_MAX_CHAR_ONE_LINE] = '\0';
                if(i==0)
                {
                    CLI_LIB_PrintStr_1("%s\r\n", tmp);
                    /*here needn't increase line_num, because later will add 4 line*/
                }
                else
                {
                    CLI_LIB_PrintStr_2("%21s %s\r\n", " ", tmp);
                    line_num ++;
                }
            }
        }
        CLI_LIB_PrintStr_1("Status              : %s\r\n",vlan_info.dot1q_vlan_static_row_status == VAL_dot1qVlanStaticRowStatus_notReady ? "Suspended" : "Active");
        line_num += 4;

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
        {
            BOOL_T learning=TRUE;
            if(AMTR_PMGR_GetVlanLearningStatus(vid, &learning))
            {
                CLI_LIB_PrintStr_1("MAC Learning        : %s\r\n", (learning == TRUE)? "Enabled" : "Disabled");
                line_num++;
            }
        }
#endif

#if (SYS_CPNT_RSPAN == TRUE)
        if (RSPAN_OM_IsRspanVlan(vid) == TRUE)
        {
            PROCESS_MORE_FUNC("\r\n");
            return line_num;
        }
#endif

        CLI_LIB_PrintStr("Ports/Port Channels :");
        L_CVRT_convert_portList_2_portMap(vlan_info.dot1q_vlan_static_egress_ports, arBitList , SYS_ADPT_TOTAL_NBR_OF_LPORT, 0x31);

        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

        /* for (j = 1; j <= current_max_unit; j++) */
        for (j=0; STKTPLG_POM_GetNextUnit(&j); )
        {   /*pttch stacking*/
            /*eth*/
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            for (i = 1 ; i <= max_port_num ; i++)
            {
                verify_unit = j;
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                    continue; /*1. not present; 2. trunk member; 3. unknown port*/

                if (VLAN_POM_IsPortVlanMember(vid_ifindex, ifindex))
                {
                    port_counter ++;

                    if (port_counter%5 == 1)
                    {
                        if(is_first_line)
                        {
                            is_first_line = FALSE;
                            CLI_LIB_PrintStr("");
                        }
                        else
                        {
                            CLI_LIB_PrintStr("                     ");
                        }
                    }
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(ifindex,name);

                        if (strlen(name) > 8)
                        {
                            name[8] = 0;
                        }
                        if(arBitList[ifindex-1] == '1')
                        {
                            CLI_LIB_PrintStr_1(" %7s(S)", name);
                        }
                        else
                        {
                            CLI_LIB_PrintStr_1(" %7s(D)", name);
                        }
                    }
#else
                    if(arBitList[ifindex-1] == '1')
                    {
                        CLI_LIB_PrintStr_2(" Eth%lu/%2lu(S)", (unsigned long)j, (unsigned long)i);
                    }
                    else
                    {
                        CLI_LIB_PrintStr_2(" Eth%lu/%2lu(D)", (unsigned long)j, (unsigned long)i);
                    }
#endif
                    if (port_counter % 5 == 0)
                    {
                        PROCESS_MORE_FUNC("\r\n");
                    }
                }
            }
        }/*for stacking unit(j)*/

        if (port_counter % 5 != 0 || port_counter == 0)
        {
            PROCESS_MORE_FUNC("\r\n");
        }

        /*trunk*/
        trunk_id = 0;
        while(TRK_PMGR_GetNextTrunkId(&trunk_id))
        {
            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &trunk_ifindex);

            if (VLAN_POM_IsPortVlanMember(vid_ifindex, trunk_ifindex))
            {
                trunk_counter ++;

                if (trunk_counter%5 == 1)
                {
                    CLI_LIB_PrintStr("                     ");
                }

                if(arBitList[trunk_ifindex-1] == '1')
                {
                    CLI_LIB_PrintStr_1(" Trunk%2lu(S)", (unsigned long)trunk_id);
                }
                else
                {
                    CLI_LIB_PrintStr_1(" Trunk%2lu(D)", (unsigned long)trunk_id);
                }

                if (trunk_counter % 5 == 0)
                {
                    PROCESS_MORE_FUNC("\r\n");
                }
            }
        }

        if (trunk_counter % 5 != 0 /*|| trunk_counter == 0*/)
        {
            PROCESS_MORE_FUNC("\r\n");
        }
    }

    PROCESS_MORE_FUNC("\r\n");
    return line_num;
}



/*
   sprintf(buff, "Eth %lu/%2lu Status\r\n", unit_id, port_id);

   sprintf(buff, "Trunk %lu Status\r\n", trunk_id);
 */
static UI32_T show_one_interface_switchport(UI32_T lport, UI32_T line_num, UI32_T port_type);

UI32_T CLI_API_Show_Interfaces_Switchport(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)/*PRIORITY*//*BROADCAST STORM CONTROL*/
{


    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;

    if(arg[0] == NULL)
    {
        /* all ethernet*/
        {
            UI32_T lport;
            UI32_T verify_unit;
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
     /*     for(i = 1; i <= current_max_unit; i++) */
            for (i=0; STKTPLG_POM_GetNextUnit(&i); )
            {
                verify_unit = i;
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);
                for(verify_port = 1; verify_port <= ctrl_P->sys_info.max_port_number; verify_port++)
                {
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);

                    if(verify_ret == CLI_API_ETH_NOT_PRESENT)  /* talor 2004-08-31 */
                        continue;

#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    sprintf((char *)buff, "Information of %s:\r\n", name);
                    PROCESS_MORE(buff);
                }
#else
                sprintf((char *)buff, "Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                PROCESS_MORE(buff);
#endif

                    switch(verify_ret)
                    {
                        case CLI_API_ETH_OK:
                            if((line_num = show_one_interface_switchport(lport, line_num, PORT_TYPE_ETH)) ==  JUMP_OUT_MORE)
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
                            if((line_num = show_one_interface_switchport(lport, line_num, PORT_TYPE_ETH)) ==  JUMP_OUT_MORE)
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
                            sprintf((char *)buff, " Not present.\r\n");
                            PROCESS_MORE(buff);
                            break;

                        case CLI_API_ETH_UNKNOWN_PORT:
                            sprintf((char*)buff, " Unknown port.\r\n");
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
                    sprintf((char *)buff, "Information of Trunk %lu\r\n", (unsigned long)verify_trunk_id);
                    PROCESS_MORE(buff);

                    if((line_num = show_one_interface_switchport(lport, line_num, PORT_TYPE_TRUNK)) == JUMP_OUT_MORE)
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
    }
    else /*user specify*/
    {
        if(arg[1]==NULL)
            return CLI_ERR_INTERNAL;

        if (arg[0][0] == 'e' || arg[0][0] == 'E')
        {
            UI32_T lport = 0;

            UI32_T verify_unit = (UI32_T)atoi((char*)arg[1]);
            UI32_T verify_port = atoi(strchr((char*)arg[1], '/')+1 );
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
                    show_one_interface_switchport(lport, line_num, PORT_TYPE_ETH);
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
                    show_one_interface_switchport(lport, line_num, PORT_TYPE_ETH);
                    break;

                case CLI_API_ETH_NOT_PRESENT:
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    break;

                case CLI_API_ETH_UNKNOWN_PORT:
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    break;
            }

        }
        else if (arg[0][0] == 'p' || arg[0][0] == 'P')
        {
            UI32_T lport = 0;
            UI32_T verify_trunk_id = (UI32_T)atoi((char*)arg[1]);
            CLI_API_TrunkStatus_T verify_ret;

            if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret, verify_trunk_id);
                return CLI_NO_ERROR;
            }
            else
            {
                CLI_LIB_PrintStr_1("Information of Trunk %lu\r\n", (unsigned long)verify_trunk_id);
                line_num += 1;

                show_one_interface_switchport(lport, line_num, PORT_TYPE_TRUNK);
            }
        }
    }
    return CLI_NO_ERROR;
}

static UI32_T show_one_interface_switchport(UI32_T lport, UI32_T line_num, UI32_T port_type)
{

   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

   BOOL_T is_trunk_member = FALSE;

   UI32_T  unit_id;
   UI32_T  port_id;
   UI32_T  trunk_id;

   UI32_T  gvrp_status  = 0;
   UI32_T  lacp_state   = 0;
   UI32_T  vid          = 0;
   UI32_T  vid_ifindex  = 0;
   UI16_T   count        = 0;
   UI32_T  time_mark    = 0;
   UI32_T  comma_flag  = 0;
   UI8_T   rate_limit_mode[20]   = {0};
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
   UI8_T   m_rate_limit_mode[20]   = {0}; /*for multicast*/
#endif
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
   UI8_T   u_rate_limit_mode[20]   = {0}; /*for unknown unicast*/
#endif

   PRI_MGR_Dot1dPortPriorityEntry_T priority_entry;
   VLAN_OM_Vlan_Port_Info_T         port_info;
   Port_Info_T                      swctr_port_info;
   VLAN_OM_Dot1qVlanCurrentEntry_T  vlan_info;

  if(port_type == PORT_TYPE_ETH)
  {
     if(SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
     {
        UI32_T trk_id;

        if( SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UIUserPortToTrunkPort(unit_id, port_id, &trk_id))
        {
            return JUMP_OUT_MORE;
        }

        if(TRK_PMGR_IsDynamicTrunkId(trk_id))
           sprintf((char *)buff, " Member port of trunk %lu created by LACP.\r\n", (unsigned long)trk_id);
        else
           sprintf((char *)buff, " Member port of trunk %lu created by user.\r\n", (unsigned long)trk_id);

         PROCESS_MORE_FUNC(buff);

         is_trunk_member = TRUE;
      }
   }
   else       /*PORT_TYPE_TRUNK*/
   {
     if(SWCTRL_POM_LogicalPortToUserPort(lport, &unit_id, &port_id, &trunk_id) == SWCTRL_LPORT_TRUNK_PORT)
     {
        if(TRK_PMGR_IsDynamicTrunkId(trunk_id))
           sprintf((char *)buff, " Trunk created by LACP.\r\n");
        else
           sprintf((char *)buff, " Trunk created by the user.\r\n");

         PROCESS_MORE_FUNC(buff);
      }
   }

   /*SWCTRL*/
   memset(&swctr_port_info, 0, sizeof(Port_Info_T));
   if(!SWCTRL_POM_GetPortInfo(lport, &swctr_port_info))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr("Failed to get switchport SWCTRL info.\r\n");
#endif
      return JUMP_OUT_MORE;
   }
   else
   {
      /*Broadcast*/
      switch(swctr_port_info.bcast_rate_mode)
      {
      case VAL_bcastStormSampleType_pkt_rate:
         strcpy((char *)rate_limit_mode, "packets/second");
         break;

      case VAL_bcastStormSampleType_octet_rate:
         strcpy((char *)rate_limit_mode, "kbits/second");
         break;

      case VAL_bcastStormSampleType_percent:
         strcpy((char *)rate_limit_mode, "percent");
         break;
      }

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
      /*Multicast*/
      switch(swctr_port_info.mcast_rate_mode)
      {
      case VAL_mcastStormSampleType_pkt_rate:
         strcpy((char *)m_rate_limit_mode, "packets/second");
         break;

      case VAL_mcastStormSampleType_octet_rate:
         strcpy((char *)m_rate_limit_mode, "kbits/second");
         break;

      case VAL_mcastStormSampleType_percent:
         strcpy((char *)m_rate_limit_mode, "percent");
         break;
      }
#endif

#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
      /*Unknown unicast */
      switch(swctr_port_info.unknown_ucast_rate_mode)
      {
      case VAL_unkucastStormSampleType_pkt_rate:
         strcpy((char *)u_rate_limit_mode, "packets/second");
         break;

      case VAL_unkucastStormSampleType_octet_rate:
         strcpy((char *)u_rate_limit_mode, "kbits/second");
         break;

      case VAL_unkucastStormSampleType_percent:
         strcpy((char *)u_rate_limit_mode, "percent");
         break;
      }
#endif
   }

#if (SYS_CPNT_LACP == TRUE)
   /*LACP*/
   if (port_type == PORT_TYPE_ETH)
   {
      LACP_MGR_Dot3adLacpPortEntry_T lacp_port_entry;

      memset(&lacp_port_entry, 0, sizeof(LACP_MGR_Dot3adLacpPortEntry_T));
      lacp_port_entry.dot3ad_lacp_port_index = lport;

      if(!LACP_PMGR_GetDot3adLacpPortEntry(&lacp_port_entry))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get switchport LACP info.\r\n");
#endif
         return JUMP_OUT_MORE;
      }
      else
      {
         lacp_state = lacp_port_entry.dot3ad_lacp_port_status;
      }
   }
#endif /*#if (SYS_CPNT_LACP == TRUE)*/

/*switchport protected*/
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
    UI32_T port_private_mode;
#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */

   if(!is_trunk_member)
   {
      /*priority*/
      memset(&priority_entry , 0 , sizeof(PRI_MGR_Dot1dPortPriorityEntry_T));
      if (!PRI_PMGR_GetDot1dPortPriorityEntry(lport, &priority_entry))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get switchport priroity info.\r\n");
#endif
         return JUMP_OUT_MORE;
      }

      /*VLAN*/
      memset(&port_info, 0, sizeof(VLAN_OM_Vlan_Port_Info_T));
      if(!VLAN_PMGR_GetPortEntry(lport, &port_info))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to get switchport VLAN info.\r\n");
#endif
         return JUMP_OUT_MORE;
      }

/*switchport protected*/
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
        /*Protected Port*/
        if (!SWCTRL_POM_GetPortPrivateMode(lport, &port_private_mode))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("Failed to get switchport Protected Port Status.\r\n");
#endif
            return JUMP_OUT_MORE;
        }
#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */
   }



   /*----------------------------------------------------------------------------------*/
   /*Broadcast limitation*/
   if(swctr_port_info.bsctrl_state == VAL_bcastStormStatus_enabled)
   {
#if defined(STRAWMAN)
      sprintf((char *)buff," Broadcast Threshold           : Enabled, %lu %s\r\n", (unsigned long)swctr_port_info.bcast_rate_limit, "packets in queue");
      PROCESS_MORE_FUNC(buff);
#else
      sprintf((char *)buff," Broadcast Threshold           : Enabled, %lu %s\r\n", (unsigned long)swctr_port_info.bcast_rate_limit, rate_limit_mode);
      PROCESS_MORE_FUNC(buff);
#endif
   }
   else
   {
      sprintf((char *)buff," Broadcast Threshold           : Disabled\r\n");
      PROCESS_MORE_FUNC(buff);
   }

#if (SYS_CPNT_MSTORM_SUPPORT_LPORT == TRUE)
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_MSTORM)
   /*----------------------------------------------------------------------------------*/
   /*Multicast limitation*/
   if(swctr_port_info.msctrl_state == VAL_mcastStormStatus_enabled)
   {
      sprintf((char *)buff," Multicast Threshold           : Enabled, %lu %s\r\n", (unsigned long)swctr_port_info.mcast_rate_limit, m_rate_limit_mode);
      PROCESS_MORE_FUNC(buff);
   }
   else
   {
      sprintf((char *)buff," Multicast Threshold           : Disabled\r\n");
      PROCESS_MORE_FUNC(buff);
   }
#endif
#endif /*#if (SYS_CPNT_MSTORM_SUPPORT_LPORT == TRUE)*/

#if (SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT == TRUE)
#if (SYS_CPNT_STORM_MODE & SYS_CPNT_STORM_UNKNOWN_USTORM)
   /*Unknown-unicast limitation*/
   if(swctr_port_info.unknown_usctrl_state == VAL_unknownUcastStormStatus_enabled)
   {
      sprintf((char *)buff," Unknown Unicast Threshold     : Enabled, %lu %s\r\n", (unsigned long)swctr_port_info.unknown_ucast_rate_limit,u_rate_limit_mode);
      PROCESS_MORE_FUNC(buff);
   }
   else
   {
      sprintf((char *)buff," Unknown Unicast Threshold     : Disabled\r\n");
      PROCESS_MORE_FUNC(buff);
   }
#endif
#endif /*#if (SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT == TRUE)*/

#if (SYS_CPNT_LACP == TRUE)
   /*LACP*/
   if (port_type == PORT_TYPE_ETH)
   {
      sprintf((char *)buff," LACP Status                   : %s\r\n", ((lacp_state == VAL_lacpPortStatus_enabled)?("Enabled"):("Disabled")));
      PROCESS_MORE_FUNC(buff);
   }
#endif

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE==TRUE)
   /*Dual-mode*/
   {
       UI32_T vlan_id;
       BOOL_T dual_mode_status;
       if(!is_trunk_member)
       {
           if(VLAN_PMGR_GetRunningPortDualMode(lport,&dual_mode_status,&vlan_id)!=SYS_TYPE_GET_RUNNING_CFG_FAIL)
           {
               if(dual_mode_status==TRUE)
               {
                   sprintf((char *)buff," Dual-mode                     : %lu\r\n",(unsigned long)vlan_id);
                   PROCESS_MORE_FUNC(buff);
               }
               else
               {
                   sprintf((char *)buff," Dual-mode                     : Disabled\r\n");
                   PROCESS_MORE_FUNC(buff);
               }
           }
       }
       else
       {
           if(VLAN_PMGR_GetRunningPortDualMode(trunk_id,&dual_mode_status,&vlan_id)!=SYS_TYPE_GET_RUNNING_CFG_FAIL)
           {
               if(dual_mode_status==TRUE)
               {
                   sprintf((char *)buff," Dual-mode                     : %lu\r\n",(unsigned long)vlan_id);
                   PROCESS_MORE_FUNC(buff);
               }
               else
               {
                   sprintf((char *)buff," Dual-mode                     : Disabled\r\n");
                   PROCESS_MORE_FUNC(buff);
               }
           }
       }
   }
#endif

   if(!is_trunk_member) /*normal port, trunk*/
   {

#if (SYS_CPNT_INGRESS_RATE_LIMIT == TRUE)
      /*rate-limit*/
      #if defined(SYS_ADPT_UI_RATE_LIMIT_FACTOR) && (SYS_ADPT_UI_RATE_LIMIT_FACTOR < 1000)
      sprintf((char *)buff," Ingress Rate Limit            : %s, %lu kbits/second\r\n", (swctr_port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled)?("Enabled"):("Disabled"), (unsigned long)swctr_port_info.ingress_rate_limit);
      #else
      sprintf((char *)buff," Ingress Rate Limit            : %s, %lu mbits/second\r\n", (swctr_port_info.ingress_rate_limit_status == VAL_rlPortInputStatus_enabled)?("Enabled"):("Disabled"), (unsigned long)(swctr_port_info.ingress_rate_limit/1000));
      #endif
      PROCESS_MORE_FUNC(buff);
#endif
#if (SYS_CPNT_EGRESS_RATE_LIMIT == TRUE)
      /*rate-limit*/
      #if defined(SYS_ADPT_UI_RATE_LIMIT_FACTOR) && (SYS_ADPT_UI_RATE_LIMIT_FACTOR < 1000)
      sprintf((char *)buff," Egress Rate Limit             : %s, %lu kbits/second\r\n", (swctr_port_info.egress_rate_limit_status == VAL_rlPortOutputStatus_enabled)?("Enabled"):("Disabled"), (unsigned long)(swctr_port_info.egress_rate_limit));
      #else
      sprintf((char *)buff," Egress Rate Limit             : %s, %lu mbits/second\r\n", (swctr_port_info.egress_rate_limit_status == VAL_rlPortOutputStatus_enabled)?("Enabled"):("Disabled"), (unsigned long)(swctr_port_info.egress_rate_limit/1000));
      #endif
      PROCESS_MORE_FUNC(buff);
#endif


      /*VLAN: port mode*/
      sprintf((char *)buff," VLAN Membership Mode          : %s\r\n",
        ((port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_hybrid) ? ("Hybrid")
        : ((port_info.vlan_port_entry.vlan_port_mode == VAL_vlanPortMode_access) ? ("Access") : ("Trunk"))));
      PROCESS_MORE_FUNC(buff);

      /*VLAN: ingress rule*/
      sprintf((char *)buff," Ingress Rule                  : %s\r\n", ((port_info.port_item.dot1q_port_ingress_filtering == VAL_dot1qPortIngressFiltering_true)?
                                              ("Enabled"):("Disabled")));
      PROCESS_MORE_FUNC(buff);

      /*VLAN: Accetable frames*/
      sprintf((char *)buff," Acceptable Frame Type         : %s\r\n", ((port_info.port_item.dot1q_port_acceptable_frame_types == VAL_dot1qPortAcceptableFrameTypes_admitAll)?
                                                   ("All frames"):((port_info.port_item.dot1q_port_acceptable_frame_types == VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged)?
                                                   ("Untagged frames only"):("Tagged frames only"))));
      PROCESS_MORE_FUNC(buff);

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE==FALSE)
      /*VLAN: pvid*/
      sprintf((char *)buff," Native VLAN                   : %lu\r\n", (unsigned long)port_info.port_item.dot1q_pvid_index);
      PROCESS_MORE_FUNC(buff);
#endif

      /*VLAN: priority*/
      sprintf((char *)buff," Priority for Untagged Traffic : %lu\r\n", (unsigned long)priority_entry.dot1d_port_default_user_priority );
      PROCESS_MORE_FUNC(buff);

      /*VLAN: egress and untag list*/
      sprintf((char *)buff," Allowed VLAN                  : ");
      CLI_LIB_PrintStr(buff);

      vid       = 0;
      time_mark = 0;
      count     = 0;
      comma_flag = 0;

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
      while(VLAN_PMGR_GetNextDot1qVlanCurrentEntryAgent(time_mark, &vid, &vlan_info))
#else
      while(VLAN_POM_GetNextDot1qVlanCurrentEntry(time_mark, &vid, &vlan_info))
#endif
      {
         VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
         if (VLAN_POM_IsPortVlanMember(vid_ifindex, lport))
         {
            count++;

            if (vlan_info.dot1q_vlan_current_untagged_ports[(UI32_T)( (lport - 1)/8 )] & (1 << ( 7 - ( (lport - 1) % 8) ) ) )
            {
               /*
                 2004/7/9 05:12 pttch 6 means a line show 6 vlans, if the last(6) vlan
                                need to show. it need to delete "," in the last
               */
               if (comma_flag == 0)
               {
                  sprintf((char *)buff, "%5lu(u)", (unsigned long)vid);
                  comma_flag = 1;
               }
               else     /* comma_flag=1 */
               {
                    if(count%5 != 1)
                    {
                        sprintf((char *)buff, ",%5lu(u)", (unsigned long)vid);
                        comma_flag = 1;
               }
               else
               {
                        sprintf((char *)buff, "                                 %5lu(u)", (unsigned long)vid);
                        comma_flag = 1;
                    }
               }
               CLI_LIB_PrintStr(buff);
            }
            else
            {
               if (comma_flag == 0)
               {
                  sprintf((char *)buff, "%5lu(t)", (unsigned long)vid);
                  comma_flag = 1;
               }
               else     /* comma_flag=1 */
               {
                    if(count%5 != 1)
               {
                        sprintf((char *)buff, ",%5lu(t)", (unsigned long)vid);
                        comma_flag = 1;
               }
               else
               {
                        sprintf((char *)buff, "                                 %5lu(t)", (unsigned long)vid);
                        comma_flag = 1;
                    }
               }
               CLI_LIB_PrintStr(buff);
            }

            /*
               2004/7/9 05:13 pttch if a line have 6 vlans need jump to next line
            */

            if (count%5 == 0)
            {
               sprintf((char *)buff,"\r\n");
               PROCESS_MORE_FUNC(buff);
            }
         }
      }

      /*
         2004/7/9 05:13 pttch if a line do not have 6 vlans or have 0 vlans need jump to next line
      */

      if (count%5 !=0 || count == 0)
      {
         PROCESS_MORE_FUNC("\r\n");
      }

      /*VLAN: forbidden list*/
      sprintf((char *)buff," Forbidden VLAN                : ");
      CLI_LIB_PrintStr(buff);
      vid       = 0;
      time_mark = 0;
      count     = 0;
      comma_flag = 0;

#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE == TRUE)
      while(VLAN_POM_GetNextDot1qVlanCurrentEntryAgent(time_mark, &vid, &vlan_info))
#else
      while(VLAN_POM_GetNextDot1qVlanCurrentEntry(time_mark, &vid, &vlan_info))
#endif
      {
         if (vlan_info.dot1q_vlan_forbidden_egress_ports[(UI32_T)((lport -1) / 8)] & (1 << (7-((lport - 1) % 8))))
         {
            count++;
            /*
              2004/7/9 05:12 pttch 9 means a line show 9 vlans, if the last(9) vlan
                                need to show. it need to delete "," in the last
            */
               if (comma_flag == 0)
            {
                  sprintf((char *)buff, "%5lu", (unsigned long)vid);
                  comma_flag = 1;
               }
               else     /* comma_flag=1 */
               {
                    if(count%8 != 1)
                    {
                        sprintf((char *)buff, ",%5lu", (unsigned long)vid);
                        comma_flag = 1;
            }
            else
            {
                        sprintf((char *)buff, "                                 %5lu", (unsigned long)vid);
                        comma_flag = 1;
                    }
            }
            CLI_LIB_PrintStr(buff);

            if (count%8 == 0)
            {
               sprintf((char *)buff,"\r\n");
               PROCESS_MORE_FUNC(buff);
            }
         }
      }

      if (count%8 !=0 || count == 0)
      {
         PROCESS_MORE_FUNC("\r\n");
      }

/*switchport protected*/
#if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE)
        /*Protected Port*/
        sprintf((char *)buff, " Protected Port Status         : %s\r\n",
            ((port_private_mode == VAL_pvePortStatus_enabled) ? ("Enabled") : ("Disabled")));

        PROCESS_MORE_FUNC(buff);
#endif /* #if (SYS_DFLT_TRAFFIC_SEG_METHOD == SYS_DFLT_TRAFFIC_SEG_METHOD_PORT_PRIVATE_MODE) */

        /*VLAN: port mode*/

   }

    if (!is_trunk_member && trunk_id == 0) /*normal port*/
    {
#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)
        if (swctr_port_info.switch_mode == VAL_swctrlSwitchModeSF)
        {
            PROCESS_MORE_FUNC(" Switching Mode                : Store and Forward\r\n");
        }
        else if (swctr_port_info.switch_mode == VAL_swctrlSwitchModeCT)
        {
            PROCESS_MORE_FUNC(" Switching Mode                : Cut Through\r\n");
        }
#endif /*#if (SYS_CPNT_SWCTRL_SWITCH_MODE_CONFIGURABLE == TRUE)*/
    }

   return line_num;
}

/*configuration*/
UI32_T CLI_API_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==FALSE)
    char    *op_ptr;
    char    Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char    delemiters[2] = {0};

    UI32_T  lower_val = 0;
    UI32_T  upper_val = 0;
    UI32_T  err_idx;

    UI32_T  vid = 0;
    UI8_T   status_position = 3;
    UI32_T  vlan_row_status;

    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T                          vid_ifindex;
    BOOL_T                          result;

#if (SYS_CPNT_RSPAN == TRUE)
    BOOL_T  isRspnVlan = FALSE;
#endif
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    UI32_T  timeout_tick = SYSFUN_GetSysTick() + (60 * SYS_BLD_TICKS_PER_SECOND);
#endif

    if (arg[0] == NULL)
        return CLI_ERR_INTERNAL;

    delemiters[0] = ',';

#if(SYS_CPNT_CLUSTER == TRUE)
    vid = atoi(arg[0]);

    if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
    {
        CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
        CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
    }
#endif

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W1_VLAN:
            if ((strchr(arg[0],',') != NULL) || (strchr(arg[0],'-') != NULL))
            {   /* set multiple VLANs */
                if ((arg[1] != NULL) &&
                    ((arg[1][0] == 'n') || (arg[1][0] == 'N'))
                   )
                {
                    /* not support to set VLAN name when setting multiple VLANs */
                    CLI_LIB_PrintStr("Not allow to set the same name for multiple VLANs.\r\n");
                    return CLI_NO_ERROR;
                }

#if (SYS_CPNT_RSPAN == TRUE)
                if (arg[status_position] != NULL)
                {
                    if (    (arg[status_position][0] == 'r')
                    	 || (arg[status_position][0] == 'R')
                         || (arg[status_position+2] != NULL)
                       )
                    {
                        isRspnVlan = TRUE;
                    }
                }
#endif

                op_ptr = arg[0];
                do
                {
                    memset(Token, 0, CLI_DEF_MAX_BUFSIZE);
                    op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                    if (!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                        break;

                    for (vid = lower_val; vid <= upper_val; vid++)
                    {
                        VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                        memset(&vlan_info, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
                        vlan_info.dot1q_vlan_index = vid_ifindex;
                        result = FALSE;

#if (SYS_CPNT_RSPAN == TRUE)
        				if ((RSPAN_OM_IsRspanVlan(vid) == FALSE) && (isRspnVlan == TRUE))
        				{
        					if (RSPAN_PMGR_CreateRspanVlan(vid) == FALSE)
        					{
#if (SYS_CPNT_EH == TRUE)
                            	CLI_API_Show_Exception_Handeler_Msg();
#else
        						CLI_LIB_PrintStr_1("Failed to create RSPAN VLAN %lu\r\n", (unsigned long)vid);
#endif
        						return CLI_NO_ERROR;
        					}
        				}
        				else if ((RSPAN_OM_IsRspanVlan(vid) == TRUE) && (isRspnVlan == FALSE))
        				{
#if (SYS_CPNT_EH == TRUE)
                            	CLI_API_Show_Exception_Handeler_Msg();
#else
        						CLI_LIB_PrintStr_1("Failed to create VLAN %lu, it is already RSPAN VLAN\r\n", (unsigned long)vid);
#endif
        						return CLI_NO_ERROR;
        				}
        				else
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

                        if (!VLAN_POM_IsVlanExisted(vid)                  ||
                            !(result = VLAN_POM_GetVlanEntry(&vlan_info)) ||
                            vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent) /* not exist or exist but dynamic => create vlan first */
                        {
                            if (!VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Operation aborted: failed to create VLAN %lu\r\n", (unsigned long)vid);
#endif
                                return CLI_NO_ERROR;
                            }
                        }

                        if (arg[status_position] != NULL)
                        {   /*specify the status*/
                            if (    (arg[status_position + 1] != NULL)
                                 && (    (arg[status_position + 1][0] == 's')
                                      || (arg[status_position + 1][0] == 'S')
                                    )
                               ) /*suspend*/
                                vlan_row_status = VAL_dot1qVlanStaticRowStatus_notInService;
                            else                                                                         /*active*/
                                vlan_row_status = VAL_dot1qVlanStaticRowStatus_active;
                        }
                        else
                        {
                            vlan_row_status = VAL_dot1qVlanStaticRowStatus_active; /*default: temp*/
                        }

                        if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, vlan_row_status))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Operation aborted: failed to set row status of VLAN %lu\r\n", (unsigned long)vid);
#endif
                            return CLI_NO_ERROR;
                        }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                    CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                    }
                } while (op_ptr != 0 && !isspace(*op_ptr));
            }
            else
            {   /* set one VLAN */
                if (    (arg[1] != NULL)
                     && ((arg[1][0] == 'n') || (arg[1][0] == 'N'))
                   )
                {
                    status_position = 5;
                }

#if (SYS_CPNT_RSPAN == TRUE)
                if (arg[status_position] != NULL)
                {
                    if (    (arg[status_position][0] == 'r')
                    	 || (arg[status_position][0] == 'R')
                         || (arg[status_position+2] != NULL)
                       )
                    {
                        isRspnVlan = TRUE;
                    }
                }
#endif
                vid = atoi(arg[0]);
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                vlan_info.dot1q_vlan_index = vid_ifindex;
                result = FALSE;

#if (SYS_CPNT_RSPAN == TRUE)
				if ((RSPAN_OM_IsRspanVlan(vid) == FALSE) && (isRspnVlan == TRUE))
				{
					if (RSPAN_PMGR_CreateRspanVlan(vid) == FALSE)
					{
	#if (SYS_CPNT_EH == TRUE)
                    	CLI_API_Show_Exception_Handeler_Msg();
	#else
						CLI_LIB_PrintStr_1("Failed to create RSPAN VLAN %lu\r\n", (unsigned long)vid);
	#endif
						return CLI_NO_ERROR;
					}
				}
				else if ((RSPAN_OM_IsRspanVlan(vid) == TRUE) && (isRspnVlan == FALSE))
				{
#if (SYS_CPNT_EH == TRUE)
                    	CLI_API_Show_Exception_Handeler_Msg();
#else
						CLI_LIB_PrintStr_1("Failed to create VLAN %lu, it is already RSPAN VLAN\r\n", (unsigned long)vid);
#endif
						return CLI_NO_ERROR;
				}
				else
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

                if (!VLAN_POM_IsVlanExisted(vid)                  ||
                    !(result = VLAN_POM_GetVlanEntry(&vlan_info)) ||
                    vlan_info.dot1q_vlan_status != VAL_dot1qVlanStatus_permanent) /* not exist or exist but dynamic => create vlan first */
                {
                    if (!VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to create VLAN %lu\r\n", (unsigned long)vid);
#endif
                        return CLI_NO_ERROR;
                    }
                }

                if (    (arg[1] != NULL)
                     && ((arg[1][0] == 'n') || (arg[1][0] == 'N'))
                   )
                {   /* set vlan name */
                    if (!VLAN_PMGR_SetDot1qVlanStaticName(vid, arg[2]))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set name VLAN %lu\r\n", (unsigned long)vid);
#endif
                        return CLI_NO_ERROR;
                    }
                }

                if (arg[status_position] != NULL)
                {   /*specify the status*/
                    if (    (arg[status_position + 1] != NULL)
                         && (    (arg[status_position + 1][0] == 's')
                              || (arg[status_position + 1][0] == 'S')
                            )
                       ) /*suspend*/
                        vlan_row_status = VAL_dot1qVlanStaticRowStatus_notInService;
                    else                                                                         /*active*/
                        vlan_row_status = VAL_dot1qVlanStaticRowStatus_active;
                }
                else
                {
                    vlan_row_status = VAL_dot1qVlanStaticRowStatus_active; /*default: temp*/
                }

                if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, vlan_row_status))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set rowstatus of VLAN %lu\r\n", (unsigned long)vid);
#endif
                    return CLI_NO_ERROR;
                }
            }

            break;

        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W2_NO_VLAN:
            if ((strchr(arg[0],',') != NULL) || (strchr(arg[0],'-') != NULL))
            {
                /* set multiple vlan */
                if (arg[1] != NULL)
                {
                	if ((arg[1][0] == 'n') || (arg[1][0] == 'N'))
                	{
                    	/* not support to set VLAN name when setting multiple VLANs */
                    	return CLI_ERR_INTERNAL;
                	}

#if (SYS_CPNT_RSPAN == TRUE)
                	if ((arg[1][0] == 'r') || (arg[1][0] == 'R'))
                	{
                    	/* not support to remove muliple RSPAN VLANs */
                    	return CLI_ERR_INTERNAL;
                	}
#endif
				}

                op_ptr = arg[0];
                do
                {
                    memset(Token, 0, CLI_DEF_MAX_BUFSIZE);
                    op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                    if (!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                        break;

                    for (vid = lower_val; vid <= upper_val; vid++)
                    {
                        if (!VLAN_POM_IsVlanExisted(vid))
                        {
                            if (arg[1])
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Operation aborted: VLAN %lu does not exist\r\n", (unsigned long)vid);
#endif
                                return CLI_NO_ERROR;
                            }
                        }
                        else
                        {
                            if (arg[1] != NULL)
                            {
                                if (arg[1][0] == 'n' || arg[1][0] == 'N') /*name => remove vlan name only*/
                                {
                                    UI8_T null_name = 0;

                                    if (!VLAN_PMGR_SetDot1qVlanStaticName(vid, (char *)&null_name)) /*null string*/
                                    {
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr_1("Operation aborted: failed to remove name of VLAN %lu\r\n", (unsigned long)vid);
#endif
                                        return CLI_NO_ERROR;
                                    }
                                }
                                else /*state => default*/
                                {
                                    if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active))
                                    {
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr_1("Operation aborted: failed to set rowstatus of VLAN %lu\r\n", (unsigned long)vid);
#endif
                                        return CLI_NO_ERROR;
                                    }
                                }
                            }
                            else /*remove vlan*/
                            {
                                if (!VLAN_PMGR_DeleteNormalVlan(vid, VAL_dot1qVlanStatus_permanent))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Operation aborted: failed to remove VLAN %lu\r\n", (unsigned long)vid);
#endif
                                    return CLI_NO_ERROR;
                                }
                            }
                        }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                        CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                    } /* for (vid = lower_val; vid <= upper_val; vid++) */
                } while (op_ptr != 0 && !isspace(*op_ptr));
            }
            else
            {   /* set one vlan */
                vid = atoi(arg[0]);
                if (!VLAN_POM_IsVlanExisted(vid))
                {
                    if (arg[1])
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("No such VLAN %lu\r\n", (unsigned long)vid);
#endif
                        return CLI_NO_ERROR;
                    }
                }
                else
                {
                    if (arg[1])
                    {
                        if (arg[1][0] == 'n' || arg[1][0] == 'N') /*name => remove vlan name only*/
                        {
                            UI8_T null_name = 0;

                            if (!VLAN_PMGR_SetDot1qVlanStaticName(vid, (char *)&null_name)) /*null string*/
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to remove name of VLAN %lu\r\n", (unsigned long)vid);
#endif
                                return CLI_NO_ERROR;
                            }
                        }
                        else                                     /*state => default*/
                        {
                            if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to set rowstatus of VLAN %lu\r\n", (unsigned long)vid);
#endif
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                    else   /*remove vlan*/
                    {
#if (SYS_CPNT_RSPAN == TRUE)
						if (RSPAN_OM_IsRspanVlan(vid) == TRUE)
						{
							if (RSPAN_PMGR_DeleteRspanVlan(vid) == FALSE)
							{
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
								CLI_LIB_PrintStr_1("Failed to remove RSPAN vlan %lu\r\n", (unsigned long)vid);
#endif
							}
							return CLI_NO_ERROR;
						}
#endif /* end #if (SYS_CPNT_RSPAN == TRUE) */

                        if (!VLAN_PMGR_DeleteNormalVlan(vid, VAL_dot1qVlanStatus_permanent))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove VLAN %lu\r\n", (unsigned long)vid);
#endif
                            return CLI_NO_ERROR;
                        }
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

/*EPR:ES4827G-FLF-ZZ-00232
 *Problem: CLI:size of vlan name different in console and mib
 *Solution: add CLI command "alias" for interface set,the
 *          alias is different from name and port descrition,so
 *          need add new command.
 *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
 *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
 *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
 *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
 *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
 *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
 *Approved by:Hardsun
 *Fixed by:Dan Xie
 */
/*configuration*/
UI32_T CLI_API_Alias_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    #if 0
    UI32_T ret;   /* modified by Jinhua Wei ,to remove warning ,becaued the variable never used */
    #endif
    UI32_T vid;
    UI8_T  null_str = 0;
    VLAN_IFINDEX_CONVERTTO_VID(ctrl_P->CMenu.vlan_ifindex, vid);

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_ALIAS:
            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;
            if(strlen((char*)arg[0]) > MAXSIZE_ifAlias)
                return CLI_ERR_CMD_INVALID_RANGE;
            if(!VLAN_PMGR_SetDot1qVlanAlias(vid,(char*)arg[0]))
                CLI_LIB_PrintStr_1("Failed to set alias on VLAN %lu\r\n",(unsigned long)vid);
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_ALIAS:

            if(!VLAN_PMGR_SetDot1qVlanAlias(vid,(char*)&null_str))
                CLI_LIB_PrintStr_1("Failed to remove alias on VLAN %lu\r\n",(unsigned long)vid);

             break;

        default:
            break;
    }
    return CLI_NO_ERROR;
}
UI32_T CLI_API_Switchport_Ingressfiltering(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T i               = 0;
   UI32_T ifindex         = 0;
   UI32_T ingress_filtering;

   UI32_T verify_unit = ctrl_P->CMenu.unit_id;
   UI32_T verify_port;
   CLI_API_EthStatus_T verify_ret;

   switch( cmd_idx )
   {
   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_INGRESSFILTERING:
      ingress_filtering = VAL_dot1qPortIngressFiltering_true;
      break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_INGRESSFILTERING:
      ingress_filtering = VAL_dot1qPortIngressFiltering_false;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
   {
      if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
      {
         verify_port = i;

         if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
         {
            display_ethernet_msg(verify_ret, verify_unit, verify_port);
            continue;
         }
         else if (!VLAN_PMGR_SetDot1qIngressFilter(ifindex, ingress_filtering))
        {
#if (CLI_SUPPORT_PORT_NAME == 1)
         {
            UI8_T name[MAXSIZE_ifName+1] = {0};
            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_2("Failed to %s ingress-filtering on ethernet %s\r\n", ingress_filtering == VAL_dot1qPortIngressFiltering_true ? "enable" : "disable", name);
#endif
         }
#else
#if (SYS_CPNT_EH == TRUE)
           CLI_API_Show_Exception_Handeler_Msg();
#else
           CLI_LIB_PrintStr_3("Failed to %s ingress-filtering on ethernet %lu/%lu\r\n", ingress_filtering == VAL_dot1qPortIngressFiltering_true ? "enable" : "disable",
                                                                                     (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
        }
      }
   }

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Ingressfiltering_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
   UI32_T trunk_ifindex;
   UI32_T ingress_filtering;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_INGRESSFILTERING:
      ingress_filtering = VAL_dot1qPortIngressFiltering_true;
      break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_INGRESSFILTERING:
      ingress_filtering = VAL_dot1qPortIngressFiltering_false;
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

   SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

   if (!VLAN_PMGR_SetDot1qIngressFilter(trunk_ifindex, ingress_filtering))
   {
#if (SYS_CPNT_EH == TRUE)
      CLI_API_Show_Exception_Handeler_Msg();
#else
      CLI_LIB_PrintStr_2("Failed to %s ingress-filtering on port channel %lu\r\n", ingress_filtering == VAL_dot1qPortIngressFiltering_true ? "enable" : "disable", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
   }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
   return CLI_NO_ERROR;
}




UI32_T CLI_API_Switchport_Acceptableframetypes(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_VLAN_PROVIDING_DUAL_MODE != TRUE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T frame_type;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_ACCEPTABLEFRAMETYPES:

            if(arg[0]!=NULL)
            {
                if (*arg[0] == 'a' || *arg[0] == 'A') /*all*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitAll;
                else if(*arg[0] == 'u' || *arg[0] == 'U') /*untagged*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged;
                else                                  /*tagged*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged;
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_ACCEPTABLEFRAMETYPES:
            frame_type = VAL_dot1qPortAcceptableFrameTypes_admitAll; /*temp: default*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!VLAN_PMGR_SetDot1qPortAcceptableFrameTypes(ifindex, frame_type))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set acceptable frame type on ethernet %s\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set acceptable frame type on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Switchport_Acceptableframetypes_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if(SYS_CPNT_VLAN_PROVIDING_DUAL_MODE != TRUE)
    UI32_T frame_type;
    UI32_T trunk_ifindex;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_ACCEPTABLEFRAMETYPES:

            if(arg[0]!=NULL)
            {
                if (*arg[0] == 'a' || *arg[0] == 'A') /*all*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitAll;
                else if (*arg[0] == 'u' || *arg[0] == 'U') /*untagged*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanUnTagged;
                else                                  /*tagged*/
                    frame_type = VAL_dot1qPortAcceptableFrameTypes_admitOnlyVlanTagged;
            }
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_ACCEPTABLEFRAMETYPES:
            frame_type = VAL_dot1qPortAcceptableFrameTypes_admitAll; /*temp: default*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

    if (!VLAN_PMGR_SetDot1qPortAcceptableFrameTypes(trunk_ifindex, frame_type))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set acceptable frame type on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
    }
#endif
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;

}

UI32_T CLI_API_Switchport_Allowed_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==FALSE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T vid             = 0;
    UI32_T time_mark = 0;
    UI32_T vid_ifindex;
    BOOL_T is_specify_untag;
    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};

    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;

    VLAN_OM_VlanPortEntry_T vlan_port_entry;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    UI32_T  timeout_tick = SYSFUN_GetSysTick() + (60 * SYS_BLD_TICKS_PER_SECOND);
#endif
    CLI_API_EthStatus_T verify_ret;
#if (SYS_CPNT_CLUSTER == TRUE)
    BOOL_T warning_msg = FALSE;
#endif
    UI8_T vlan_list_ar[VLAN_TYPE_VLAN_LIST_SIZE] = {0};

    delemiters[0] = ',';
    memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SWITCHPORT_ALLOWED_VLAN:
        if (arg[1] == NULL)
        {
            /* prepare VLAN list for below operation on ports */
            op_ptr = arg[0];
            do
            {
                memset(Token, 0, CLI_DEF_MAX_BUFSIZE);
                op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);
                
                if (!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                {
                    break;
                }
                for (vid = lower_val; vid <= upper_val; vid++)
                {
                    if (VLAN_POM_IsVlanExisted(vid) == FALSE)
                    {
#if (SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN == TRUE)
                        if (VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent) == FALSE)
                        {
                            CLI_LIB_PrintStr_1("Failed to create vlan %lu\r\n\r\n", (unsigned long)vid);
                            continue;
                        }
                        if (VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active) == FALSE)
                        {
                            CLI_LIB_PrintStr_1("Failed to set row status of vlan %lu\r\n\r\n", (unsigned long)vid);
                            continue;
                        }
#else
                        CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n", (unsigned long)vid);
                        continue;
#endif
                    }
                    
                    if (vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                    {
                        CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                        CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                    }

                    L_CVRT_ADD_MEMBER_TO_PORTLIST(vlan_list_ar, vid);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                    CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                }
            } while (op_ptr != 0 && !isspace(*op_ptr));
        } /* end if arg[1] == NULL */

        for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    continue;
                }
                else
                {
                    if (arg[1] == NULL)
                    {
                        if (VLAN_PMGR_SetPortVlanList(ifindex, vlan_list_ar) == FALSE)
                        {
#if (CLI_SUPPORT_PORT_NAME == 1)
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to set VLAN list on ethernet %s\r\n", name);
#endif
#else
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_2("Failed to set VLAN list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                        }
                        continue;
                    }

                    op_ptr = arg[1];
                    do
                    {
                        memset(Token, 0, CLI_DEF_MAX_BUFSIZE);

                        op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                        if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                            break;

                        for(vid=lower_val; vid<=upper_val; vid++)
                        {
                            if(arg[0]==NULL)
                                return CLI_ERR_INTERNAL;

                            if(!VLAN_POM_IsVlanExisted(vid))
                            {
#if (SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN == TRUE)
                                if ((arg[0][0] == 'a') || (arg[0][0] == 'A'))
                                {
                                    if (!VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent))
                                    {
                                        CLI_LIB_PrintStr_1("Failed to create vlan %lu\r\n\r\n", (unsigned long)vid);
                                        continue;
                                    }
                                    if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active))
                                    {
                                        CLI_LIB_PrintStr_1("Failed to set row status of vlan %lu\r\n\r\n", (unsigned long)vid);
                                        continue;
                                    }
                                }
                                else
                                {
#endif
    #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
    #else
                                CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
    #endif
                                continue;
#if (SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN == TRUE)
                                }
#endif
                            }

#if(SYS_CPNT_CLUSTER == TRUE)
                            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                            {
                                if(!warning_msg)
                                {
                                    CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                                    CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                                    warning_msg = TRUE;
                                }
                            }
#endif

                            VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                            if( arg[0][0] == 'a' || arg[0][0] == 'A' ) /*add*/
                            {
                                if ((arg[2] == 0) || (arg[2][0] == 'u') || (arg[2][0] == 'U'))
                                    is_specify_untag = TRUE;
                                else
                                    is_specify_untag = FALSE;


                                if (is_specify_untag == TRUE) /* join as untagged member */
                                {
                                    if (!VLAN_PMGR_AddUntagPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent))
                                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                        UI8_T name[MAXSIZE_ifName+1] = {0};
                                        CLI_LIB_Ifindex_To_Name(ifindex,name);
    #if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                                        CLI_LIB_PrintStr_1("Failed to join untagged list on ethernet %s\r\n", name);
    #endif
#else
    #if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                                        CLI_LIB_PrintStr_2("Failed to join untagged list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
#endif
                                    }
                                }
                                else
                                {
                                    if (VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, ifindex)) /* untagged member to tagged member */
                                    {
                                        if (!VLAN_PMGR_DeleteUntagPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent))
                                        {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                            UI8_T name[MAXSIZE_ifName+1] = {0};
                                            CLI_LIB_Ifindex_To_Name(ifindex,name);
    #if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                                            CLI_LIB_PrintStr_1("Failed to remove untagged list on ethernet %s\r\n", name);
    #endif
#else
    #if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                                            CLI_LIB_PrintStr_2("Failed to remove untagged list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
#endif
                                        }
                                    }
                                    else /* join as tagged member */
                                    {
                                        if  (!VLAN_PMGR_AddEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent))
                                        {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                            UI8_T name[MAXSIZE_ifName+1] = {0};
                                            CLI_LIB_Ifindex_To_Name(ifindex,name);
    #if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                                            CLI_LIB_PrintStr_1("Failed to join egress list on ethernet %s\r\n", name);
    #endif
#else
    #if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                                            CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
#endif
                                        }
                                    }
                                }
                            }
                            else     /*remove*/
                            {

 /*for provision issue, if manage_vlan != 1(running-config file) and remove last member need to set manage vlan to current
                    port native vlan*/

                                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                                {
                                    VLAN_OM_Dot1qPortVlanEntry_T vlan_port_entry;
                                    UI32_T m_vid = 0;/*manage vlan id*/
                                    UI32_T port_vid = 0;/*user port vlan id*/

                                    memset(&vlan_port_entry, 0, sizeof(vlan_port_entry));
                                    VLAN_PMGR_GetDot1qPortVlanEntry(ifindex, &vlan_port_entry);
                                    VLAN_IFINDEX_CONVERTTO_VID (vlan_port_entry.dot1q_pvid_index,port_vid);
                                    VLAN_POM_GetManagementVlan(&m_vid);

                                    if (m_vid == 1 && CLI_API_VlanStaticMemberCount(m_vid)== 1)
                                    {
                                        VLAN_PMGR_SetManagementVlan(port_vid);
                                        VLAN_PMGR_LeaveManagementVlan(m_vid);
                                    }
                                }

                                if ( VLAN_POM_IsPortVlanMember(vid_ifindex, ifindex) &&
                                    !VLAN_PMGR_DeleteEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent) )
                                {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                    {
                                        UI8_T name[MAXSIZE_ifName+1] = {0};
                                        CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr_1("Failed to remove from egress list on ethernet %s\r\n", name);
#endif
                                    }
#else
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_2("Failed to remove from egress list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                                }

                            }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                            CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                        } /* for(vid=lower_val; vid<=upper_val; vid++) */
                    }while(op_ptr != 0 && !isspace(*op_ptr));
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SWITCHPORT_ALLOWED_VLAN: /* allow vlan 1 only */
        for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
                verify_port = i;

                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                {
                    display_ethernet_msg(verify_ret, verify_unit, verify_port);
                    continue;
                }
                else
                {
                    {
                        /*add vlan 1 as default*/
                        VLAN_PMGR_DeleteForbiddenEgressPortMember(1, ifindex, VAL_dot1qVlanStatus_permanent);
                        VLAN_PMGR_AddUntagPortMember(1, ifindex, VAL_dot1qVlanStatus_permanent);

                        /*remove others*/
                        vid = 1;
                        while(VLAN_POM_GetNextVlanId(time_mark, &vid))
                        {
#if(SYS_CPNT_CLUSTER == TRUE)
                            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                            {
                                if(!warning_msg)
                                {
                                    CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                                    CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                                    warning_msg = TRUE;
                                }
                            }
#endif
                            VLAN_PMGR_DeleteEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent);
                        }
                    }
                }
            }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Allowed_Vlan_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_MSTP_SUPPORT_PVST==FALSE)
    UI32_T trunk_ifindex   = 0;
    UI32_T vid             = 0;
    UI32_T time_mark = 0;
    UI32_T vid_ifindex;
    BOOL_T is_specify_untag;
    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};

    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    UI32_T  timeout_tick = SYSFUN_GetSysTick() + (60 * SYS_BLD_TICKS_PER_SECOND);
#endif

    VLAN_OM_VlanPortEntry_T vlan_port_entry;
    UI8_T vlan_list_ar[VLAN_TYPE_VLAN_LIST_SIZE] = {0};

    delemiters[0] = ',';
    memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));

    err_idx = verify_trunk(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);
    switch(err_idx)
    {
        case CLI_API_TRUNK_NOT_EXIST:
        case CLI_API_TRUNK_INVALID_RANGE:           
            CLI_LIB_PrintStr("Port channel doesn't exist\r\n");
            return CLI_NO_ERROR;
            break;
        case CLI_API_TRUNK_NO_MEMBER:
            CLI_LIB_PrintStr("Port channel doesn't have any member\r\n");
            return CLI_NO_ERROR;            
            break;
    }

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_SWITCHPORT_ALLOWED_VLAN:
            if (arg[1] == NULL)
            {
                /* prepare VLAN list for below operation on ports */
                op_ptr = arg[0];
                do
                {
                    memset(Token, 0, CLI_DEF_MAX_BUFSIZE);
                    op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);
                    
                    if (!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                    {
                        break;
                    }
                    for (vid = lower_val; vid <= upper_val; vid++)
                    {
                        if (vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                        {
                            CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                            CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                        }

                        L_CVRT_ADD_MEMBER_TO_PORTLIST(vlan_list_ar, vid);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                        CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                    }
                } while (op_ptr != 0 && !isspace(*op_ptr));
            }

            if (arg[1] == NULL)
            {
                if (VLAN_PMGR_SetPortVlanList(trunk_ifindex, vlan_list_ar) == FALSE)
                {
                    CLI_LIB_PrintStr_1("Failed to set VLAN list on port channel %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
                }
                return CLI_NO_ERROR;
            }

            op_ptr = arg[1];
            do
            {
                memset(Token, 0, CLI_DEF_MAX_BUFSIZE);

                op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                    break;

                for(vid=lower_val; vid<=upper_val; vid++)
                {
                    if(arg[0]==NULL)
                        return CLI_ERR_INTERNAL;

                    if(!VLAN_POM_IsVlanExisted(vid))
                    {
#if (SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN == TRUE)
                        if ((arg[0][0] == 'a') || (arg[0][0] == 'A'))
                        {
                            if (!VLAN_PMGR_CreateVlan(vid, VAL_dot1qVlanStatus_permanent))
                            {
                                CLI_LIB_PrintStr_1("Failed to create vlan %lu\r\n\r\n", (unsigned long)vid);
                                continue;
                            }
                            if (!VLAN_PMGR_SetDot1qVlanStaticRowStatus(vid, VAL_dot1qVlanStaticRowStatus_active))
                            {
                                CLI_LIB_PrintStr_1("Failed to set row status of vlan %lu\r\n\r\n", (unsigned long)vid);
                                continue;
                            }
                        }
#else
    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                        CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
    #endif
                        continue;
#endif
                    }

#if(SYS_CPNT_CLUSTER == TRUE)
                    if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                    {
                        CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                        CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                    }
#endif

                    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
                    if( arg[0][0] == 'a' || arg[0][0] == 'A' ) /*add*/
                    {
                        if ((arg[2] == 0) || (arg[2][0] == 'u') || (arg[2][0] == 'U'))
                            is_specify_untag = TRUE;
                        else
                            is_specify_untag = FALSE;

                        if (is_specify_untag) /* join as untagged member */
                        {
                            if (!VLAN_PMGR_AddUntagPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent))
                            {
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_1("Failed to join untagged list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                            }
                        }
                        else
                        {
                            if (VLAN_POM_IsVlanUntagPortListMember(vid_ifindex, trunk_ifindex)) /* untagged member to tagged member */
                            {
                                if (!VLAN_PMGR_DeleteUntagPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to remove untagged list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                                }
                            }
                            else /* join as tagged member */
                            {
                                if (!VLAN_PMGR_AddEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to join egress list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                                }
                            }
                        }
                    }
                    else     /*remove*/
                    {  /*for provision issue, if manage_vlan != 1(running-config file) and remove last member need to set manage vlan to current
                        port native vlan*/

                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
                            VLAN_OM_Dot1qPortVlanEntry_T vlan_port_entry;
                            UI32_T m_vid = 0;/*manage vlan id*/
                            UI32_T port_vid = 0;/*user port vlan id*/

                            memset(&vlan_port_entry, 0, sizeof(vlan_port_entry));
                            VLAN_PMGR_GetDot1qPortVlanEntry(trunk_ifindex, &vlan_port_entry);
                            VLAN_IFINDEX_CONVERTTO_VID (vlan_port_entry.dot1q_pvid_index,port_vid);
                            VLAN_POM_GetManagementVlan(&m_vid);

                            if (m_vid == 1 && CLI_API_VlanStaticMemberCount(m_vid)== 1)
                            {
                                VLAN_PMGR_SetManagementVlan(port_vid);
                                VLAN_PMGR_LeaveManagementVlan(m_vid);
                            }
                        }

                        if ( VLAN_POM_IsPortVlanMember(vid_ifindex, trunk_ifindex) &&
                             !VLAN_PMGR_DeleteEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent) )
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove from egress list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                        }
                    }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                    CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
                }
            }while(op_ptr != 0 && !isspace(*op_ptr));
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_SWITCHPORT_ALLOWED_VLAN: /* allow vlan 1 only */
            /*add vlan 1 as default*/
            VLAN_PMGR_DeleteForbiddenEgressPortMember(1, trunk_ifindex, VAL_dot1qVlanStatus_permanent);
            VLAN_PMGR_AddEgressPortMember(1, trunk_ifindex, VAL_dot1qVlanStatus_permanent);
            VLAN_PMGR_AddUntagPortMember(1, trunk_ifindex, VAL_dot1qVlanStatus_permanent);

            /*remove others*/
            vid = 1;
            while(VLAN_POM_GetNextVlanId(time_mark, &vid))
            {
#if(SYS_CPNT_CLUSTER == TRUE)
                if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                {
                    CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                    CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                }
#endif
                VLAN_PMGR_DeleteUntagPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent);
                VLAN_PMGR_DeleteEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent);
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                CLI_API_VLAN_CHK_SWDOG(ctrl_P, timeout_tick);
#endif /* SYS_CPNT_SW_WATCHDOG_TIMER */
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

static I32_T CLI_API_VlanStaticMemberCount(UI32_T vid)
{
    UI32_T      vid_ifindex, port_num, byte, shift;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;
    I32_T      member_count =0;

    /* BODY */

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    vlan_info.dot1q_vlan_index = (UI16_T)vid_ifindex;

    if (!VLAN_POM_GetVlanEntry(&vlan_info))
    {
        return FALSE;
    } /* end of if */

    for (port_num = 1; port_num <= (SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST * 8); port_num++)
    {
        CLI_API_GetPortPosition(port_num, &byte, &shift);

        if (vlan_info.dot1q_vlan_static_egress_ports[byte] & ((0x01) << ( 7 - shift)))
        {
            member_count++;
        } /* end of if */

    } /* end of for */

    return member_count;

}

static void CLI_API_GetPortPosition(UI32_T lport_ifindex, UI32_T *byte, UI32_T *shift)
{
    *byte = (UI32_T)(lport_ifindex - 1) / 8;
    *shift = (UI32_T)(lport_ifindex - 1) % 8;
}


UI32_T CLI_API_Switchport_Native_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==FALSE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T pvid;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SWITCHPORT_NATIVE_VLAN:

            if(arg[0]!=NULL)
                pvid = (UI32_T)atoi((char*)arg[0]);
            else
                return CLI_ERR_INTERNAL;

            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SWITCHPORT_NATIVE_VLAN:
            pvid = 1; /*temp: default*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == FALSE)
    if(!VLAN_POM_IsVlanExisted(pvid))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
        return CLI_NO_ERROR;
    }
#endif

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)
            else if (!VLAN_PMGR_SetNativeVlanAgent(ifindex, pvid))
#else
            else if (!VLAN_PMGR_SetDot1qPvid(ifindex, pvid))
#endif
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set PVID on ethernet %s\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set PVID on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }
#endif

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Native_Vlan_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
#if (SYS_CPNT_MSTP_SUPPORT_PVST==FALSE)
    UI32_T trunk_ifindex   = 0;
    UI32_T pvid;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_SWITCHPORT_NATIVE_VLAN:

        if(arg[0]!=NULL)
            pvid = (UI32_T)atoi((char*)arg[0]);
        else
            return CLI_ERR_INTERNAL;

        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_SWITCHPORT_NATIVE_VLAN:
        pvid = 1; /*temp: default*/
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == FALSE)
    if(!VLAN_POM_IsVlanExisted(pvid))
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("No such VLAN.\r\n");
    #endif
         return CLI_NO_ERROR;
    }
#endif

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

#if (SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID == TRUE)
    if (!VLAN_PMGR_SetNativeVlanAgent(trunk_ifindex, pvid))
#else
    if (!VLAN_PMGR_SetDot1qPvid(trunk_ifindex, pvid))
#endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set PVID on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
    }

#endif
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Forbidden_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T vid             = 0;
    UI32_T time_mark       = 0;
    UI32_T vid_ifindex;

    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};

    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T  err_idx;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
#if (SYS_CPNT_CLUSTER == TRUE)
    BOOL_T warning_msg = FALSE;
#endif

    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_SWITCHPORT_FORBIDDEN_VLAN:
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

                    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else
                    {
                        if(arg[1]!=NULL)
                            op_ptr = arg[1];
                        else
                            return CLI_ERR_INTERNAL;

                        do
                        {
                            memset(Token, 0, CLI_DEF_MAX_BUFSIZE);
                            op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);
                            if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                            break;
                            for(vid=lower_val; vid<=upper_val; vid++)
                            {
                                if(!VLAN_POM_IsVlanExisted(vid))
                                {
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
                                    return CLI_NO_ERROR;
                                }
                                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

                                if(arg[0]==NULL)
                                    return CLI_ERR_INTERNAL;

#if(SYS_CPNT_CLUSTER == TRUE)
                                if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                                {
                                    if(!warning_msg)
                                    {
                                        CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                                        CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                                        warning_msg = TRUE;
                                    }
                                }
#endif
                                if ( !VLAN_POM_IsVlanForbiddenPortListMember(vid_ifindex, ifindex) &&
                                     (arg[0][0] == 'a' || arg[0][0] == 'A') )                         /*add*/
                                {
                                    if (!VLAN_PMGR_AddForbiddenEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent))
                                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                        {
                                            UI8_T name[MAXSIZE_ifName+1] = {0};
                                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
#else
                                            CLI_LIB_PrintStr_1("Failed to join forbidden list on ethernet %s\r\n", name);
#endif
                                        }
#else
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr_2("Failed to join forbidden list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                                    }
                                }/*add*/
                                else if( VLAN_POM_IsVlanForbiddenPortListMember(vid_ifindex, ifindex) &&
                                                    (arg[0][0] == 'r' || arg[0][0] == 'R') )                     /*remove*/
                                {
                                    if (!VLAN_PMGR_DeleteForbiddenEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent))
                                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                        {
                                            UI8_T name[MAXSIZE_ifName+1] = {0};
                                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
#else
                                            CLI_LIB_PrintStr_1("Failed to remove from forbidden list on ethernet %s\r\n", name);
#endif
                                        }
#else
#if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
#else
                                        CLI_LIB_PrintStr_2("Failed to remove from forbidden list on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
                                    }
                                }
                            }/*end of for loop*/
                        }while(op_ptr != 0 && !isspace(*op_ptr));
                    }
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_SWITCHPORT_FORBIDDEN_VLAN: /*no existed vlan forbidden this port*/
            for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

                    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else
                    {
                        vid = 0; /*initialize*/
                        while(VLAN_POM_GetNextVlanId(time_mark, &vid))
                        {
#if(SYS_CPNT_CLUSTER == TRUE)
                            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                            {
                                if(!warning_msg)
                                {
                                    CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                                    CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                                    warning_msg = TRUE;
                                }
                            }
#endif
                            VLAN_PMGR_DeleteForbiddenEgressPortMember(vid, ifindex, VAL_dot1qVlanStatus_permanent);
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

UI32_T CLI_API_Switchport_Forbidden_Vlan_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T trunk_ifindex   = 0;
    UI32_T vid             = 0;
    UI32_T time_mark       = 0;
    UI32_T vid_ifindex;

    char   *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};

    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T  err_idx;
    delemiters[0] = ',';

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_SWITCHPORT_FORBIDDEN_VLAN:

            if(arg[1]!=NULL)
                vid = (UI32_T)atoi((char*)arg[1]); /* get vlan id */
            else
                return CLI_ERR_INTERNAL;

            if(!VLAN_POM_IsVlanExisted(vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
                return CLI_NO_ERROR;
            }

            op_ptr = arg[1];
            do
            {
                memset(Token, 0, CLI_DEF_MAX_BUFSIZE);

                op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

                if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
                break;

                for(vid=lower_val; vid<=upper_val; vid++)
                {
                    if(!VLAN_POM_IsVlanExisted(vid))
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
                        return CLI_NO_ERROR;
                    }
                    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
#if(SYS_CPNT_CLUSTER == TRUE)
                    if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                    {
                        CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                        CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                    }
#endif
                    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

                    if(arg[0]==NULL)
                        return CLI_ERR_INTERNAL;

                    if ( !VLAN_POM_IsVlanForbiddenPortListMember(vid_ifindex, trunk_ifindex) &&
                                    (arg[0][0] == 'a' || arg[0][0] == 'A') )                         /*add*/
                    {
                        if (!VLAN_PMGR_AddForbiddenEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to join forbidden list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                        }
                    }
                    else if(VLAN_POM_IsVlanForbiddenPortListMember(vid_ifindex, trunk_ifindex) &&
                                        (arg[0][0] == 'r' || arg[0][0] == 'R') )                     /*remove*/
                    {
                        if (!VLAN_PMGR_DeleteForbiddenEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent))
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove from forbidden list on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                        }
                    }
                }
            }while(op_ptr != 0 && !isspace(*op_ptr));
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_SWITCHPORT_FORBIDDEN_VLAN: /*no existed vlan forbidden this port*/
            SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &trunk_ifindex);

            vid = 0; /*initialize*/
            while(VLAN_POM_GetNextVlanId(time_mark, &vid))
            {
#if(SYS_CPNT_CLUSTER == TRUE)
                if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
                {
                    CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                    CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
                }
#endif
                VLAN_PMGR_DeleteForbiddenEgressPortMember(vid, trunk_ifindex, VAL_dot1qVlanStatus_permanent);
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Mode_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T lport;
    UI32_T port_mode;
    UI32_T i;

    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_MODE:

            if(arg[0]!=NULL)
            {
                if(arg[0][0] == 'h' || arg[0][0] == 'H')
                    port_mode = VAL_vlanPortMode_hybrid;
                else if(arg[0][0] == 't' || arg[0][0] == 'T')
                    port_mode = VAL_vlanPortMode_dot1qTrunk;
                else
                    port_mode = VAL_vlanPortMode_access;
            }
            else
                return CLI_ERR_INTERNAL;
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_MODE:
            port_mode = SYS_DFLT_VLAN_PORT_MODE; /*default*/
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else if (!VLAN_PMGR_SetVlanPortMode(lport, port_mode))
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set port mode on ethernet %s\r\n", name);
#endif
                }
#else
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_2("Failed to set port mode on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
#endif
#endif
            }
        }
    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Switchport_Mode_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T lport;
    UI32_T port_mode;

    switch(cmd_idx)
    {

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_MODE:

            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            if(arg[0][0] == 'h' || arg[0][0] == 'H')
                port_mode = VAL_vlanPortMode_hybrid;
            else if(arg[0][0] == 't' || arg[0][0] == 'T')
                port_mode = VAL_vlanPortMode_dot1qTrunk;
            else
                port_mode = VAL_vlanPortMode_access;
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_MODE:
            port_mode = SYS_DFLT_VLAN_PORT_MODE; /*default*/
            break;
        default:
            return CLI_ERR_INTERNAL;
    }

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &lport);

    if (! VLAN_PMGR_SetVlanPortMode(lport, port_mode))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr_1("Failed to set port mode on trunk %lu\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
    }

#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Management(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_MANAGEMENT:
      if (!IML_PMGR_SetManagementVid(ctrl_P->CMenu.vlan_ifindex))
      {
#if (SYS_CPNT_EH == TRUE)
         CLI_API_Show_Exception_Handeler_Msg();
#else
         CLI_LIB_PrintStr("Failed to add this VLAN to management\r\n");
#endif
      }
      break;

   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_MANAGEMENT:
      /*This command will be removed */
      CLI_LIB_PrintStr("This command will be removed\r\n");
      break;

   default:
      return CLI_ERR_INTERNAL;
   }

#endif

   return CLI_NO_ERROR;
}


UI32_T CLI_API_Dual_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE==TRUE)
    UI32_T lport;
    UI32_T vid;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    int i;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W1_DUALMODE:
            if(arg[0]!=NULL)
            {
                vid = atoi(arg[0]);
            }
            else
            {
                vid = 1;
            }

            for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                    {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                        UI32_T ifindex;
                        UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                        /*if the port is module port, save command in buffer*/
                        if ( TRUE == STKTPLG_MGR_IsModulePort( verify_unit, verify_port ) )
                        {
                            sprintf(cmd_buff,"!\ninterface ethernet %lu/%lu\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)verify_port);
                            CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                            memset(cmd_buff, 0, sizeof(cmd_buff));
                            sprintf(cmd_buff," dual-mode %lu\n!\n",(unsigned long)vid);
                            CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                            return CLI_NO_ERROR;
                        }
#else
                       UI32_T ifindex = 0;
                       BOOL_T is_inherit        =TRUE;
                       UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                       SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &ifindex, &is_inherit);

                       /*if the port is module port, save command in buffer*/
                       if (TRUE == CLI_MGR_IsModulePort(ifindex))
                       {
                           sprintf(cmd_buff,"!\ninterface ethernet %lu/%lu\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)verify_port);
                           CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                           memset(cmd_buff, 0, sizeof(cmd_buff));
                           sprintf(cmd_buff," dual-mode %lu\n!\n",(unsigned long)vid);
                           CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                           return CLI_NO_ERROR;
                       }
#endif
                    }
#endif

                    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else
                    {
                        if(VLAN_PMGR_EnablePortDualMode(lport,vid)!=TRUE)
                        {
                            CLI_LIB_PrintStr_2("Failed to set ethernet %lu/%lu to dual mode port.\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                        }
                    }
                }
            }
        break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_NO_DUALMODE:
            for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
            {
                if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
                {
                    verify_port = i;

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                    {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                        UI32_T ifindex;
                        UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                        /*   if the port is module port, save command in buffer
                         */
                        if ( TRUE == STKTPLG_MGR_IsModulePort( verify_unit, verify_port ) )
                        {
                            sprintf(cmd_buff,"!\ninterface ethernet %lu/%lu\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)verify_port);
                            CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                            memset(cmd_buff, 0, sizeof(cmd_buff));
                            sprintf(cmd_buff," no dual-mode\n!\n");
                            CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                            return CLI_NO_ERROR;
                        }
#else
                       UI32_T ifindex = 0;
                       BOOL_T is_inherit        =TRUE;
                       UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                       SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &ifindex, &is_inherit);

                       /*if the port is module port, save command in buffer*/
                       if (TRUE == CLI_MGR_IsModulePort(ifindex))
                       {
                           sprintf(cmd_buff,"!\ninterface ethernet %lu/%lu\n", (unsigned long)ctrl_P->CMenu.unit_id, (unsigned long)verify_port);
                           CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                           memset(cmd_buff, 0, sizeof(cmd_buff));
                           sprintf(cmd_buff," no dual-mode\n!\n");
                           CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                           return CLI_NO_ERROR;
                       }
#endif
                    }
#endif

                    if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                    {
                        display_ethernet_msg(verify_ret, verify_unit, verify_port);
                        continue;
                    }
                    else
                    {
                        if(VLAN_PMGR_DisablePortDualMode(lport)!=TRUE)
                        {
                            CLI_LIB_PrintStr_2("Failed to revert ethernet %lu/%lu to default.\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                        }
                    }
                }
            }
        break;

        default:
        break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_tagged_Ethernet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==TRUE)
    UI32_T vid             = 0;
    UI32_T vid_ifindex;
    UI32_T trunk_id=0;
    UI32_T port_status;
    UI32_T remove_trunk_ifindex[6]={0};
    UI32_T total_number_of_trunk = 0;

    UI32_T begin_verify_unit,begin_verify_port;
    UI32_T end_verify_unit,end_verify_port;
    BOOL_T is_add_only_one = TRUE;
    BOOL_T is_remove_only_one = TRUE;

    UI32_T ifindex = 0;
    BOOL_T is_inherit        =TRUE;
    UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

    VLAN_OM_VlanPortEntry_T vlan_port_entry;

    memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W2_TAGGED_ETHERNET:

            CLI_API_GET_VLAN_DATABASE_VLANID(&vid);

            if(!VLAN_POM_IsVlanExisted(vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
            }
            else
            {
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            }

#if(SYS_CPNT_CLUSTER == TRUE)
            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
            {
                CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
            }
#endif

            /*add only one tagged ethernet port number to vlan*/
            if (isdigit(arg[0][0]))
            {
                begin_verify_unit = atoi(arg[0]);
                begin_verify_port = atoi(strchr(arg[0],'/')+1);

                /*add many tagged ethernet port numbers to vlan */
                if(arg[1][0]=='t'||arg[1][0]=='T')
                {
                    if (isdigit(arg[2][0]))
                    {
                        is_add_only_one = FALSE;
                        end_verify_unit = atoi(arg[2]);
                        end_verify_port = atoi(strchr(arg[2],'/')+1);
                    }
                }

            }

            if(is_add_only_one==TRUE)
            {
                port_status = SWCTRL_POM_UIUserPortToIfindex(begin_verify_unit, begin_verify_port, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                    UI32_T vlan_type = 0;
                    UI8_T vlan_name[16]={0};

                    CLI_API_GET_VLAN_DATABASE_VLAN_TYPE( &vlan_type, vlan_name );

                    /*if the port is module port, save command in buffer*/
                    if ( TRUE == STKTPLG_MGR_IsModulePort( begin_verify_unit, begin_verify_port ) )
                    {
                        switch ( vlan_type )
                        {
                            case CLI_TYPE_BY_VLAN_PORT:
                                sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                CLI_MGR_AddDeviceCfg( begin_verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg( begin_verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                                return CLI_NO_ERROR;
                                break;

                            case CLI_TYPE_BY_VLAN_NAME:
                                sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                CLI_MGR_AddDeviceCfg( begin_verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg( begin_verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff );
                                return CLI_NO_ERROR;
                                break;

                            default:
                                return CLI_NO_ERROR;
                            break;
                        }
                    }
#else
                   UI32_T vlan_type = 0;
                   UI8_T vlan_name[16]={0};

                   /*if the port is module port, save command in buffer*/
                   if (TRUE == CLI_MGR_IsModulePort(ifindex))
                   {
                       CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                       switch(vlan_type)
                       {
                           case CLI_TYPE_BY_VLAN_PORT:
                               sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           case CLI_TYPE_BY_VLAN_NAME:
                               sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           default:
                               return CLI_NO_ERROR;
                           break;
                       }
                   }
#endif
                } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    {
                        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                    }

                    if(!VLAN_PMGR_AddEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,TRUE))  //TRUE[tagged member]
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to join egress list on ethernet %s\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
#endif
#endif
                    }
                }
                else
                {
                    CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
                }
            }

            else
            {
                UI32_T i = 0;
                UI32_T j = 0;
                UI32_T temp_end_verify_port = 0;
                UI32_T temp_begin_verify_port = 0;

                for(i=begin_verify_unit;i<=end_verify_unit;i++)
                {
                    if(i == begin_verify_unit)
                    {
                        temp_begin_verify_port = begin_verify_port;
                    }
                    else
                    {
                        temp_begin_verify_port = 1;
                    }

                    if(i == end_verify_unit) /*get the user settings the max port number of last unit*/
                    {
                        temp_end_verify_port = end_verify_port;
                    }
                    else /*get the max port number of this unit */
                    {
                        temp_end_verify_port = SWCTRL_POM_GetUnitPortNumber(i);
                    }

                    for(j=temp_begin_verify_port;j<=temp_end_verify_port;j++)
                    {
                        port_status = SWCTRL_POM_UIUserPortToIfindex(i, j, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                            UI32_T vlan_type = 0;
                            UI8_T vlan_name[16]={0};

                            CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                            /*if the port is module port, save command in buffer*/
                            if ( TRUE == STKTPLG_MGR_IsModulePort( i, j ) )
                            {
                                switch(vlan_type)
                                {
                                    case CLI_TYPE_BY_VLAN_PORT:
                                        sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                        CLI_MGR_AddDeviceCfg(i + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    case CLI_TYPE_BY_VLAN_NAME:
                                        sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                        CLI_MGR_AddDeviceCfg(i + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    default:
                                        return CLI_NO_ERROR;
                                        break;
                                }
                            }
#else
                           UI32_T vlan_type = 0;
                           UI8_T vlan_name[16]={0};

                           /*if the port is module port, save command in buffer*/
                           if (TRUE == CLI_MGR_IsModulePort(ifindex))
                           {
                               CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                               switch(vlan_type)
                               {
                                   case CLI_TYPE_BY_VLAN_PORT:
                                       sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   case CLI_TYPE_BY_VLAN_NAME:
                                       sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff," tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   default:
                                       return CLI_NO_ERROR;
                                   break;
                               }
                           }
#endif
                        } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                        if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                        {
                            if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                            {
                                SWCTRL_POM_UIUserPortToTrunkPort(i,j,&trunk_id);
                                SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                            }

                            if(!VLAN_PMGR_AddEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,TRUE)) //TRUE[tagged member]
                            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                {
                                    UI8_T name[MAXSIZE_ifName+1] = {0};
                                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to join egress list on ethernet %s\r\n", name);
#endif
                                }
#else
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)i, (unsigned long)j);
#endif
#endif
                            }
                        }
                    }
                }
            }
        break;

        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W3_NO_TAGGED_ETHERNET:

            CLI_API_GET_VLAN_DATABASE_VLANID(&vid);

            if(!VLAN_POM_IsVlanExisted(vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
            }
            else
            {
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            }

#if(SYS_CPNT_CLUSTER == TRUE)
            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
            {
                CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
            }
#endif

            /*add only one tagged ethernet port number to vlan*/
            if (isdigit(arg[0][0]))
            {
                begin_verify_unit = atoi(arg[0]);
                begin_verify_port = atoi(strchr(arg[0],'/')+1);

                /*add many tagged ethernet port numbers to vlan */
                if(arg[1][0]=='t'||arg[1][0]=='T')
                {
                    if (isdigit(arg[2][0]))
                    {
                        is_remove_only_one = FALSE;
                        end_verify_unit = atoi(arg[2]);
                        end_verify_port = atoi(strchr(arg[2],'/')+1);
                    }
                }

            }

            if(is_remove_only_one==TRUE)
            {
                port_status = SWCTRL_POM_UIUserPortToIfindex(begin_verify_unit, begin_verify_port, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                    UI32_T vlan_type = 0;
                    UI8_T vlan_name[16]={0};

                    CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                    /*if the port is module port, save command in buffer*/
                    if ( TRUE == STKTPLG_MGR_IsModulePort( begin_verify_unit, begin_verify_port ) )
                    {
                        switch(vlan_type)
                        {
                            case CLI_TYPE_BY_VLAN_PORT:
                                sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+ SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            case CLI_TYPE_BY_VLAN_NAME:
                                sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            default:
                                return CLI_NO_ERROR;
                                break;
                        }
                    }
#else
                   UI32_T vlan_type = 0;
                   UI8_T vlan_name[16]={0};

                   /*if the port is module port, save command in buffer*/
                   if (TRUE == CLI_MGR_IsModulePort(ifindex))
                   {
                       CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                       switch(vlan_type)
                       {
                           case CLI_TYPE_BY_VLAN_PORT:
                               sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           case CLI_TYPE_BY_VLAN_NAME:
                               sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           default:
                               return CLI_NO_ERROR;
                           break;
                       }
                   }
#endif
                } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    {
                        SWCTRL_POM_UIUserPortToTrunkPort(begin_verify_unit,begin_verify_port,&trunk_id);
                        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                    }

                    if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,TRUE))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove egress list on ethernet %s\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
#endif
#endif
                    }
                }
                else
                {
                    CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
                }
            }
            else
            {
                UI32_T i = 0;
                UI32_T j = 0;
                UI32_T temp_end_verify_port = 0;
                UI32_T temp_begin_verify_port = 0;
                for(i=begin_verify_unit;i<=end_verify_unit;i++)
                {
                    if(i == begin_verify_unit)
                    {
                        temp_begin_verify_port = begin_verify_port;
                    }
                    else
                    {
                        temp_begin_verify_port = 1;
                    }

                    if(i == end_verify_unit) /*get the user settings the max port number of last unit*/
                    {
                        temp_end_verify_port = end_verify_port;
                    }
                    else /*get the max port number of this unit */
                    {
                        temp_end_verify_port = SWCTRL_POM_GetUnitPortNumber(i);
                    }

                    for(j=temp_begin_verify_port;j<=temp_end_verify_port;j++)
                    {
                        port_status = SWCTRL_POM_UIUserPortToIfindex(i, j, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                            UI32_T vlan_type = 0;
                            UI8_T vlan_name[16]={0};

                            UI32_T  master_id = 0;

                            STKTPLG_MGR_GetMasterUnitId( & master_id );
                            CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                            /*if the port is module port, save command in buffer*/
                            if ( TRUE == STKTPLG_MGR_IsModulePort( i, j ) )
                            {
                                switch(vlan_type)
                                {
                                    case CLI_TYPE_BY_VLAN_PORT:
                                        sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    case CLI_TYPE_BY_VLAN_NAME:
                                        sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    default:
                                        return CLI_NO_ERROR;
                                        break;
                                }
                            }
#else
                           UI32_T vlan_type = 0;
                           UI8_T vlan_name[16]={0};

                           /*if the port is module port, save command in buffer*/
                           if (TRUE == CLI_MGR_IsModulePort(ifindex))
                           {
                               CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                               switch(vlan_type)
                               {
                                   case CLI_TYPE_BY_VLAN_PORT:
                                       sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   case CLI_TYPE_BY_VLAN_NAME:
                                       sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   default:
                                       return CLI_NO_ERROR;
                                   break;
                               }
                           }
#endif
                        } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                        if(port_status == SWCTRL_LPORT_NORMAL_PORT)
                        {
                            if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,TRUE))
                            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                {
                                    UI8_T name[MAXSIZE_ifName+1] = {0};
                                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to remove egress list on ethernet %s\r\n", name);
#endif
                                }
#else
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)i, (unsigned long)j);
#endif
#endif
                             }
                        }
                        else if (port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                        {
                            int i =0;
                            SWCTRL_POM_UIUserPortToTrunkPort(i,j,&trunk_id);
                            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);

                            for(i=0;i<6;i++)
                            {
                                if(remove_trunk_ifindex[i]==ifindex)
                                {
                                    break;
                                }
                                else
                                {
                                    if(remove_trunk_ifindex[i]==0)
                                    {
                                        remove_trunk_ifindex[i]=ifindex;
                                        total_number_of_trunk++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                /*remove trunk from vlan*/
                if(total_number_of_trunk>0)
                {
                    int i=0;
                    for(i=0;i<total_number_of_trunk;i++)
                    {
                        if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, remove_trunk_ifindex[i], VAL_dot1qVlanStatus_permanent,TRUE))
                        {
                            trunk_id = remove_trunk_ifindex[i]  - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + 1;
                            CLI_LIB_PrintStr_1("Failed to remove trunk %lu from vlan\r\n",(unsigned long)trunk_id);
                        }
                    }
                    total_number_of_trunk=0;
                }
            }

        break;

        default:
        break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Untagged_Ethernet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MSTP_SUPPORT_PVST==TRUE)
    UI32_T vid             = 0;
    UI32_T vid_ifindex;
    UI32_T trunk_id;
    UI32_T port_status;
    UI32_T remove_trunk_ifindex[6]={0};
    UI32_T total_number_of_trunk = 0;

    UI32_T begin_verify_unit,begin_verify_port;
    UI32_T end_verify_unit,end_verify_port;
    BOOL_T is_add_only_one = TRUE;
    BOOL_T is_remove_only_one = TRUE;

    UI32_T ifindex = 0;
    BOOL_T is_inherit        =TRUE;
    UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

    VLAN_OM_VlanPortEntry_T vlan_port_entry;

    memset(&vlan_port_entry, 0, sizeof(VLAN_OM_VlanPortEntry_T));

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W2_UNTAGGED_ETHERNET:

            CLI_API_GET_VLAN_DATABASE_VLANID(&vid);

            if(!VLAN_POM_IsVlanExisted(vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
            }
            else
            {
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            }

#if(SYS_CPNT_CLUSTER == TRUE)
            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
            {
                CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
            }
#endif

            /*add only one tagged ethernet port number to vlan*/
            if (isdigit(arg[0][0]))
            {
                begin_verify_unit = atoi(arg[0]);
                begin_verify_port = atoi(strchr(arg[0],'/')+1);

                /*add many tagged ethernet port numbers to vlan (keyword:to) */
                if(arg[1][0]=='t'||arg[1][0]=='T')
                {
                    if (isdigit(arg[2][0]))
                    {
                        is_add_only_one = FALSE;
                        end_verify_unit = atoi(arg[2]);
                        end_verify_port = atoi(strchr(arg[2],'/')+1);
                    }
                }

            }

            if(is_add_only_one==TRUE)
            {
                port_status = SWCTRL_POM_UIUserPortToIfindex(begin_verify_unit, begin_verify_port, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                    UI32_T vlan_type = 0;
                    UI8_T vlan_name[16]={0};
                    UI32_T  master_id = 0;

                    STKTPLG_MGR_GetMasterUnitId( & master_id );
                    CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                    /*if the port is module port, save command in buffer*/
                    if ( TRUE == STKTPLG_MGR_IsModulePort( begin_verify_unit, begin_verify_port ) )
                    {
                        switch ( vlan_type )
                        {
                            case CLI_TYPE_BY_VLAN_PORT:
                                sprintf(cmd_buff,"!\nvlan %lu by port",(unsigned long)vid);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            case CLI_TYPE_BY_VLAN_NAME:
                                sprintf(cmd_buff,"!\nvlan %lu name %s",(unsigned long)vid,vlan_name);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            default:
                                return CLI_NO_ERROR;
                                break;
                        }
                    }
#else
                   UI32_T vlan_type = 0;
                   UI8_T vlan_name[16]={0};

                   /*if the port is module port, save command in buffer*/
                   if (TRUE== CLI_MGR_IsModulePort(ifindex))
                   {
                       CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                       switch(vlan_type)
                       {
                           case CLI_TYPE_BY_VLAN_PORT:
                               sprintf(cmd_buff,"!\nvlan %lu by port\n",vid);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           case CLI_TYPE_BY_VLAN_NAME:
                               sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           default:
                               return CLI_NO_ERROR;
                           break;
                       }
                   }
#endif
                } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    {
                        SWCTRL_POM_UIUserPortToTrunkPort(begin_verify_unit,begin_verify_port,&trunk_id);
                        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                    }

                    if(!VLAN_PMGR_AddEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,FALSE)) //FALSE[untagged member]
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to join egress list on ethernet %s\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
#endif
#endif
                    }
                }
                else
                {
                    CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
                }
            }
            else
            {
                UI32_T i = 0;
                UI32_T j = 0;
                UI32_T temp_end_verify_port = 0;
                UI32_T temp_begin_verify_port = 0;

                for(i=begin_verify_unit;i<=end_verify_unit;i++)
                {
                    if(i == begin_verify_unit)
                    {
                        temp_begin_verify_port = begin_verify_port;
                    }
                    else
                    {
                        temp_begin_verify_port = 1;
                    }

                    if(i == end_verify_unit) /*get the user settings the max port number of last unit*/
                    {
                        temp_end_verify_port = end_verify_port;
                    }
                    else /*get the max port number of this unit */
                    {
                        temp_end_verify_port = SWCTRL_POM_GetUnitPortNumber(i);
                    }

                    for(j=temp_begin_verify_port;j<=temp_end_verify_port;j++)
                    {
                        port_status = SWCTRL_POM_UIUserPortToIfindex(i, j, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                            UI32_T vlan_type = 0;
                            UI8_T vlan_name[16]={0};
                            UI32_T  master_id = 0;

                            STKTPLG_MGR_GetMasterUnitId( & master_id );
                            CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                            /*if the port is module port, save command in buffer*/
                            if ( TRUE == STKTPLG_MGR_IsModulePort( i, j ) )
                            {
                                switch(vlan_type)
                                {
                                    case CLI_TYPE_BY_VLAN_PORT:
                                        sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    case CLI_TYPE_BY_VLAN_NAME:
                                        sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff," untagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    default:
                                        return CLI_NO_ERROR;
                                        break;
                                }
                            }
#else
                           UI32_T vlan_type = 0;
                           UI8_T vlan_name[16]={0};

                           /*if the port is module port, save command in buffer*/
                           if (TRUE == CLI_MGR_IsModulePort(ifindex))
                           {
                               CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                               switch(vlan_type)
                               {
                                   case CLI_TYPE_BY_VLAN_PORT:
                                       sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"untagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   case CLI_TYPE_BY_VLAN_NAME:
                                       sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"untagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   default:
                                       return CLI_NO_ERROR;
                                   break;
                               }
                           }
#endif
                        } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                        if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                        {
                            if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                            {
                                SWCTRL_POM_UIUserPortToTrunkPort(i,j,&trunk_id);
                                SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                            }

                            if(!VLAN_PMGR_AddEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,FALSE))  //FALSE[untagged member]
                            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                {
                                    UI8_T name[MAXSIZE_ifName+1] = {0};
                                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to join egress list on ethernet %s\r\n", name);
#endif
                                }
#else
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_2("Failed to join egress list on ethernet %lu/%lu\r\n", (unsigned long)i, (unsigned long)j);
#endif
#endif
                            }
                        }
                    }
                }
            }

        break;

        case PRIVILEGE_CFG_VLAN_DATABASE_CMD_W3_NO_UNTAGGED_ETHERNET:

            CLI_API_GET_VLAN_DATABASE_VLANID(&vid);

            if(!VLAN_POM_IsVlanExisted(vid))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr_1("VLAN%lu does not exist\r\n",(unsigned long)vid);
#endif
            }
            else
            {
                VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
            }

#if(SYS_CPNT_CLUSTER == TRUE)
            if(vid == SYS_DFLT_CLUSTER_DEFAULT_VLAN)
            {
                CLI_LIB_PrintStr_1("Warning: VLAN %lu is dedicated to Clustering.\r\n", (unsigned long)vid);
                CLI_LIB_PrintStr("Operating on this VLAN may cause problems in Clustering operation.\r\n");
            }
#endif

            /*add only one tagged ethernet port number to vlan*/
            if (isdigit(arg[0][0]))
            {
                begin_verify_unit = atoi(arg[0]);
                begin_verify_port = atoi(strchr(arg[0],'/')+1);

                /*add many tagged ethernet port numbers to vlan */
                if(arg[1][0]=='t'||arg[1][0]=='T')
                {
                    if (isdigit(arg[2][0]))
                    {
                        is_remove_only_one = FALSE;
                        end_verify_unit = atoi(arg[2]);
                        end_verify_port = atoi(strchr(arg[2],'/')+1);
                    }
                }

            }

            if(is_remove_only_one==TRUE)
            {
                port_status = SWCTRL_POM_UIUserPortToIfindex(begin_verify_unit, begin_verify_port, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                    UI32_T vlan_type = 0;
                    UI8_T vlan_name[16]={0};

                    UI32_T  master_id = 0;

                    STKTPLG_MGR_GetMasterUnitId( & master_id );
                    CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                    /*if the port is module port, save command in buffer*/
                    if ( TRUE == STKTPLG_MGR_IsModulePort( begin_verify_unit, begin_verify_port ) )
                    {
                        switch(vlan_type)
                        {
                            case CLI_TYPE_BY_VLAN_PORT:
                                sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff,"no untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            case CLI_TYPE_BY_VLAN_NAME:
                                sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                memset(cmd_buff, 0, sizeof(cmd_buff));
                                sprintf(cmd_buff,"no untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                                CLI_MGR_AddDeviceCfg(begin_verify_unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                return CLI_NO_ERROR;
                                break;

                            default:
                                return CLI_NO_ERROR;
                                break;
                        }
                    }
#else
                   UI32_T vlan_type = 0;
                   UI8_T vlan_name[16]={0};

                   /*if the port is module port, save command in buffer*/
                   if (TRUE == CLI_MGR_IsModulePort(ifindex))
                   {
                       CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                       switch(vlan_type)
                       {
                           case CLI_TYPE_BY_VLAN_PORT:
                               sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff,"no untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           case CLI_TYPE_BY_VLAN_NAME:
                               sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               memset(cmd_buff, 0, sizeof(cmd_buff));
                               sprintf(cmd_buff,"no untagged ethernet %lu/%lu \n!\n",(unsigned long)begin_verify_unit,(unsigned long)begin_verify_port);
                               CLI_MGR_AddModuleCfg(begin_verify_unit, cmd_buff);
                               return CLI_NO_ERROR;
                           break;

                           default:
                               return CLI_NO_ERROR;
                           break;
                       }
                   }
#endif
                } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                if(port_status!=SWCTRL_LPORT_UNKNOWN_PORT)
                {
                    if(port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                    {
                        SWCTRL_POM_UIUserPortToTrunkPort(begin_verify_unit,begin_verify_port,&trunk_id);
                        SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);
                    }

                    if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,FALSE))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove egress list on ethernet %s\r\n", name);
#endif
                        }
#else
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
#endif
#endif
                    }
                }
                else
                {
                    CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)begin_verify_unit, (unsigned long)begin_verify_port);
                }
            }
            else
            {
                UI32_T i = 0;
                UI32_T j = 0;
                UI32_T temp_end_verify_port = 0;
                UI32_T temp_begin_verify_port = 0;
                for(i=begin_verify_unit;i<=end_verify_unit;i++)
                {
                    if(i == begin_verify_unit)
                    {
                        temp_begin_verify_port = begin_verify_port;
                    }
                    else
                    {
                        temp_begin_verify_port = 1;
                    }

                    if(i == end_verify_unit) /*get the user settings the max port number of last unit*/
                    {
                        temp_end_verify_port = end_verify_port;
                    }
                    else /*get the max port number of this unit */
                    {
                        temp_end_verify_port = SWCTRL_POM_GetUnitPortNumber(i);
                    }

                    for(j=temp_begin_verify_port;j<=temp_end_verify_port;j++)
                    {
                        port_status = SWCTRL_POM_UIUserPortToIfindex(i, j, &ifindex, &is_inherit);

/*2004/9/30 Eric Huang add for module provision*/
#if (SYS_CPNT_3COM_CLI == FALSE)
                        if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                        {
#if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                            UI32_T vlan_type = 0;
                            UI8_T vlan_name[16]={0};
                            UI32_T  master_id = 0;

                            STKTPLG_MGR_GetMasterUnitId( & master_id );
                            CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                            /*if the port is module port, save command in buffer*/
                            if ( TRUE == STKTPLG_MGR_IsModulePort( i, j ) )
                            {
                                switch(vlan_type)
                                {
                                    case CLI_TYPE_BY_VLAN_PORT:
                                        sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    case CLI_TYPE_BY_VLAN_NAME:
                                        sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        memset(cmd_buff, 0, sizeof(cmd_buff));
                                        sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                        CLI_MGR_AddDeviceCfg(i+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, cmd_buff);
                                        return CLI_NO_ERROR;
                                        break;

                                    default:
                                        return CLI_NO_ERROR;
                                        break;
                                }
                            }
#else
                           UI32_T vlan_type = 0;
                           UI8_T vlan_name[16]={0};

                           /*if the port is module port, save command in buffer*/
                           if (TRUE == CLI_MGR_IsModulePort(ifindex))
                           {
                               CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(&vlan_type,vlan_name);

                               switch(vlan_type)
                               {
                                   case CLI_TYPE_BY_VLAN_PORT:
                                       sprintf(cmd_buff,"!\nvlan %lu by port\n",(unsigned long)vid);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   case CLI_TYPE_BY_VLAN_NAME:
                                       sprintf(cmd_buff,"!\nvlan %lu name %s\n",(unsigned long)vid,vlan_name);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       memset(cmd_buff, 0, sizeof(cmd_buff));
                                       sprintf(cmd_buff,"no tagged ethernet %lu/%lu \n!\n",(unsigned long)i,(unsigned long)j);
                                       CLI_MGR_AddModuleCfg(i, cmd_buff);
                                       return CLI_NO_ERROR;
                                   break;

                                   default:
                                       return CLI_NO_ERROR;
                                   break;
                               }
                           }
#endif
                        } //end if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
#endif

                        if(port_status == SWCTRL_LPORT_NORMAL_PORT)
                        {
                            if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, ifindex, VAL_dot1qVlanStatus_permanent,FALSE))
                            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                                {
                                    UI8_T name[MAXSIZE_ifName+1] = {0};
                                    CLI_LIB_Ifindex_To_Name(ifindex,name);
#if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
#else
                                    CLI_LIB_PrintStr_1("Failed to remove egress list on ethernet %s\r\n", name);
#endif
                                }
#else
#if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
#else
                                CLI_LIB_PrintStr_2("Failed to remove egress list on ethernet %lu/%lu\r\n", (unsigned long)i, (unsigned long)j);
#endif
#endif
                            }
                        }
                        else if (port_status == SWCTRL_LPORT_TRUNK_PORT_MEMBER)
                        {
                            int i =0;
                            SWCTRL_POM_UIUserPortToTrunkPort(i,j,&trunk_id);
                            SWCTRL_POM_TrunkIDToLogicalPort(trunk_id,&ifindex);

                            for(i=0;i<6;i++)
                            {
                                if(remove_trunk_ifindex[i]==ifindex)
                                {
                                    break;
                                }
                                else
                                {
                                    if(remove_trunk_ifindex[i]==0)
                                    {
                                        remove_trunk_ifindex[i]=ifindex;
                                        total_number_of_trunk++;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                /*remove trunk from vlan*/
                if(total_number_of_trunk>0)
                {
                    int i=0;
                    for(i=0;i<total_number_of_trunk;i++)
                    {
                        if(!VLAN_PMGR_DeleteEgressPortMemberAgent(vid, remove_trunk_ifindex[i], VAL_dot1qVlanStatus_permanent,FALSE))
                        {
                            trunk_id = remove_trunk_ifindex[i]  - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + 1;
                            CLI_LIB_PrintStr_1("Failed to remove trunk %lu from vlan\r\n",(unsigned long)trunk_id);
                        }
                    }
                    total_number_of_trunk=0;
                }
            }

        break;

        default:
        break;
    }
#endif
    return CLI_NO_ERROR;
}

BOOL_T CLI_API_SET_VLAN_DATABASE_VLANID(UI32_T vlan_id)
{
    vlan_database_vlan_id = vlan_id;
    return TRUE;
}

BOOL_T CLI_API_GET_VLAN_DATABASE_VLANID(UI32_T *vlan_id)
{
    *vlan_id = vlan_database_vlan_id;
    return TRUE;
}

BOOL_T CLI_API_SET_VLAN_DATABASE_VLAN_TYPE(UI32_T vlan_type, UI8_T vlan_name[16])
{
    vlan_database_vlan_type = vlan_type;
    strcpy((char *)vlan_database_vlan_name,(char *)vlan_name);
    return TRUE;
}

BOOL_T CLI_API_GET_VLAN_DATABASE_VLAN_TYPE(UI32_T *vlan_type,UI8_T vlan_name[16])
{
    *vlan_type = vlan_database_vlan_type;
    strcpy((char *)vlan_name,(char *)vlan_database_vlan_name);
    return TRUE;
}

UI32_T CLI_API_Dual_Mode_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VLAN_PROVIDING_DUAL_MODE==TRUE)
    UI32_T trunk_ifindex=0;
    UI32_T vid;
    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&trunk_ifindex);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W1_DUALMODE:
            if(arg[0]!=NULL)
            {
                vid = atoi(arg[0]);
            }
            else
            {
                vid = 1;
            }

            if(VLAN_PMGR_EnablePortDualMode(trunk_ifindex,vid)!=TRUE)
            {
                CLI_LIB_PrintStr_1("Failed to set port channel %lu to dual mode.\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
            }
        break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_NO_DUALMODE:
            if(VLAN_PMGR_DisablePortDualMode(trunk_ifindex)!=TRUE)
            {
                CLI_LIB_PrintStr_1("Failed to revert port channel %lu to default.\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
            }
        break;

        default:
        break;
    }
#endif
    return CLI_NO_ERROR;
}

/*command: default-vlan-id <vlan-id>*/
UI32_T CLI_API_Default_Vlan_Id(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T vid = 0;

    if(arg[0]!=NULL)
        vid = (UI32_T)atoi((char*)arg[0]);
    else
    {
         CLI_LIB_PrintStr("Failed to set default VLAN.\r\n");
        return CLI_ERR_CMD_INVALID;
    }

    if(VLAN_PMGR_SetGlobalDefaultVlan(vid) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to set default VLAN.\r\n");
    }
    return CLI_NO_ERROR;
}

/*command: no default-vlan-id*/
UI32_T CLI_API_No_Default_Vlan_Id(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    if(VLAN_PMGR_SetGlobalDefaultVlan(1) != TRUE)
    {
        CLI_LIB_PrintStr("Failed to revert to default VLAN.\r\n");
    }
    return CLI_NO_ERROR;
}

