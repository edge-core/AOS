#include "cli_api.h"
#include "cli_api_priority.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "pri_mgr.h"
#include "pri_pmgr.h"
#include "cos_vm.h"
#include "l4_pmgr.h"
#include <stdio.h>
/*****************************************<<PRIORITY>>***************************************/
/*change mode*/

/*execution*/

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
static UI32_T
CLI_API_Local_ShowPortQueueWeight(
    UI32_T ifindex,
    UI32_T line_num
);
#else
static UI32_T
CLI_API_Local_ShowGlobalQueueWeight(
    UI32_T line_num
);
#endif /* SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL */


#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
static UI32_T show_one_queue_cosmap(UI32_T ifindex,UI32_T line_num);
static BOOL_T restore_cos_value_to_default(UI32_T lport, UI32_T queue_id);
#endif

UI32_T CLI_API_Show_Queue_Cosmap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
    UI32_T  lport           = 0;
    UI32_T  line_num        = 0;
    UI32_T  verify_unit = ctrl_P->sys_info.my_unit_id;
    UI32_T  verify_port;
    UI32_T  i               = 0;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;

    CLI_API_EthStatus_T verify_ret;

#if (SYS_CPNT_STACKING == TRUE)
   //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[0] == NULL)
    {
      /*all eth*/
      /* for (j = 1; j <= current_max_unit; j++) */
        for (j=0; STKTPLG_POM_GetNextUnit(&j); )
        {       /*pttch stacking*/
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            for (i = 1; i <= max_port_num; i++)
            {
                verify_unit = j;
                verify_port = i;
                if( (verify_ret = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                {
                    continue;
                }
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    CLI_LIB_PrintStr_1("%s Information\r\n", name);
                }
#else
                CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                line_num += 1;
                if ((line_num = show_one_queue_cosmap(lport,line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
        }/*end of unit loop*/
      /*all trunk*/
        {
            UI32_T  verify_trunk_id;
            CLI_API_TrunkStatus_T verify_ret;

            for(verify_trunk_id = 1; verify_trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; verify_trunk_id++)
            {
                if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                    continue;
                else
                {
                    CLI_LIB_PrintStr_1("Information of Trunk %lu\r\n", (unsigned long)verify_trunk_id);
                    line_num += 1;
                    if((line_num = show_one_queue_cosmap(lport, line_num)) == JUMP_OUT_MORE)
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
    else
    {
        switch(arg[0][0])
        {
        case 'e':
        case 'E':
            verify_port = atoi(strchr(arg[1],'/')+1);
#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[1][0]))
            {
                verify_port = atoi(strchr(arg[1],'/')+1);
            }
            else/*port name*/
            {
                UI32_T  trunk_id = 0;
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
            else
            {
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T   name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    CLI_LIB_PrintStr_1("Information of %s\r\n", name);
                }
#else
                CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit,(unsigned long)verify_port);
#endif
                line_num += 1;
                if ((line_num = show_one_queue_cosmap(lport,line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
            break;

        case 'p':
        case 'P':
            {
                UI32_T  trunk_id = atoi(arg[1]);
                CLI_API_TrunkStatus_T verify_ret;

                if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret, trunk_id);
                    return CLI_NO_ERROR;
                }
                else
                {
                    CLI_LIB_PrintStr_1("Information of Trunk %lu\r\n", (unsigned long)trunk_id);
                    line_num += 1;
                    if((line_num = show_one_queue_cosmap(lport, line_num)) == JUMP_OUT_MORE)
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
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
static UI32_T show_one_queue_cosmap(UI32_T ifindex,UI32_T line_num)
{
    UI32_T  lport_ifindex = ifindex - 1;
    UI32_T  priority      = MAX_dot1dTrafficClassPriority;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char    priority_buf[40] = {0};
    char    queue_buf[40] = {0};
    char    buf[4] = {0};
//   UI8_T  UserPort[10] = {0};
    PRI_MGR_Dot1dTrafficClassEntry_T traffic_class_entry;

    memset(&traffic_class_entry,0,sizeof(PRI_MGR_Dot1dTrafficClassEntry_T));

    sprintf((char *)priority_buf, " CoS Value      : ");
    sprintf((char *)queue_buf,    " Priority Queue : ");

    while(1)
    {
        if (!PRI_PMGR_GetNextDot1dTrafficClassEntry(&lport_ifindex, &priority, &traffic_class_entry) || lport_ifindex > ifindex)
            break;
        else if (lport_ifindex < ifindex)
            continue;
        else
        {
            sprintf((char *)buf,"%lu ",(unsigned long)traffic_class_entry.dot1d_traffic_class_priority);
            strcat((char *)priority_buf, (char *)buf);
            memset(buf,0,sizeof(buf));
            sprintf((char *)buf,"%lu ",(unsigned long)traffic_class_entry.dot1d_traffic_class);
            strcat((char *)queue_buf, (char *)buf);
            memset(buf,0,sizeof(buf));
        }
    }
    strcat((char *)priority_buf, "\r\n");
    strcat((char *)queue_buf, "\r\n");
    strcpy((char *)buff, (char *)priority_buf);
    PROCESS_MORE_FUNC(buff);
    memset(buff, 0, sizeof(buff));
    strcpy((char *)buff, (char *)queue_buf);
    PROCESS_MORE_FUNC(queue_buf);

    return line_num;
}
#endif /* #if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE) */

/*configuration*/
/*not support extend priority*/
UI32_T CLI_API_Switchport_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  i;
    UI32_T  lport;

    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_SWITCHPORT_PRIORITY:
        if (*arg[0] == 'd' || *arg[0] == 'D')
        {
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
                    else if (!PRI_PMGR_SetDot1dPortDefaultUserPriority(lport , atoi(arg[1])))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr_1("Failed to set default priority on ethernet %s\r\n", name);
    #endif
                        }
#else
    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                        CLI_LIB_PrintStr_2("Failed to set default priority on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
    #endif
#endif
                    }
                }
            }
        }
        break;

   case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_SWITCHPORT_PRIORITY:
        if (*arg[0] == 'd' || *arg[0] == 'D')
        {
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
                    else if (!PRI_PMGR_SetDot1dPortDefaultUserPriority(lport , SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY))
                    {
#if (CLI_SUPPORT_PORT_NAME == 1)
                        {
                            UI8_T   name[MAXSIZE_ifName+1] = {0};
                            CLI_LIB_Ifindex_To_Name(lport,name);
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr_1("Failed to set default priority on ethernet %s\r\n", name);
    #endif
                        }
#else
    #if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
    #else
                        CLI_LIB_PrintStr_2("Failed to set default priority on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
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

UI32_T CLI_API_Switchport_Priority_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_TRUNK_UI==TRUE)
    UI32_T  ifindex         = 0;
    CLI_API_TrunkStatus_T verify_ret;


    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id,&ifindex);
    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &ifindex)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }


    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_SWITCHPORT_PRIORITY:
        if (*arg[0] == 'd' || *arg[0] == 'D')
        {
            if (!PRI_PMGR_SetDot1dPortDefaultUserPriority(ifindex , atoi(arg[1])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set default priority\r\n");
#endif
            }
        }
        break;

   case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_SWITCHPORT_PRIORITY:
        if (*arg[0] == 'd' || *arg[0] == 'D')
        {
            if (!PRI_PMGR_SetDot1dPortDefaultUserPriority(ifindex , SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set default priority\r\n");
#endif
            }
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif  /* #if (SYS_CPNT_TRUNK_UI==TRUE) */
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Queue_Cosmap(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS == TRUE)
    USED_TO_PRT_ARGS;
    CLI_LIB_PrintStr("<TBD>\r\n");
#endif
    return CLI_NO_ERROR;
}

/************************************<<PRIORITY>>*****************************/
/*change mode*/
UI32_T CLI_API_Show_Map_Ip_Dscp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE)
    char    buff[CLI_DEF_MAX_BUFSIZE];
    UI32_T  line_num = 0;
    UI32_T  lport = 0;
    UI8_T   dscp  = 0;
    UI32_T  cos   = 0;

    SWCTRL_Lport_Type_T ret;
    UI32_T              unit;
    UI32_T              port;
    UI32_T              trunk_id;
    UI8_T               UserPort[10];

    COS_VM_MAPPING_FLAG_T get_flag;

#if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_GetIpPrecedenceDscpMappingStatus(&get_flag))
#else
    if(!L4_COS_PMGR_GetIpPrecedenceDscpMappingStatus(&get_flag))
#endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get DSCP mapping status.\r\n\r\n");
#endif
    }
    else
    {
        CLI_LIB_PrintStr_1("DSCP Mapping Status: %s\r\n\r\n", get_flag == COS_VM_DSCP_MAPPING_ENABLED ?
                                                            "Enabled" : "Disabled");
    }


    CLI_LIB_PrintStr(" Port      DSCP CoS\r\n");
    CLI_LIB_PrintStr(" --------- ---- ---\r\n");
    line_num += 4;

    if(arg[0] == NULL)
    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
    while(L4_PMGR_COS_GetNextDSCPToCos(&lport, &dscp, &cos) )
    #else
    while(L4_COS_PMGR_GetNextDSCPToCos(&lport, &dscp, &cos) )
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    if (strlen(name) > 10)/*prevent name too long for sprintf*/
                    {                                        /*pttch 2002.07.10*/
                        name[10-1] = 0;
                    }
                    sprintf((char *)UserPort, "%8s", name);
                }
#else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }
            sprintf((char *)buff, " %9s %4d %3lu\r\n", UserPort, dscp, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        UI32_T  specified_lport;

        if(arg[0][0] == 'e' || arg[0][0] == 'E') /*ethernet*/
        {
            CLI_API_EthStatus_T verify_ret;

            unit = atoi(arg[1]);
            port = atoi(strchr(arg[1], '/')+1);
#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[1][0]))
            {
                unit = atoi(arg[1]);
                port = atoi(strchr(arg[1],'/')+1);
            }
            else/*port name*/
            {
                UI32_T  trunk_id = 0;
                if (!IF_PMGR_IfnameToIfindex(arg[1], &specified_lport))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
    #endif
                    return CLI_NO_ERROR;
                }
                SWCTRL_POM_LogicalPortToUserPort(specified_lport, &unit, &port, &trunk_id);
            }
#endif /* #if (CLI_SUPPORT_PORT_NAME == 1) */

            if( (verify_ret = verify_ethernet(unit, port, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, unit, port);
                return CLI_NO_ERROR;
            }
        }
        else                                     /*trunk*/
        {
            CLI_API_TrunkStatus_T   verify_ret;

            trunk_id = atoi(arg[1]);

            if( (verify_ret = verify_trunk(trunk_id, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_trunk_msg(verify_ret, trunk_id);
                return CLI_NO_ERROR;
            }
        }

        lport = specified_lport - 1;      /*primary key*/
        dscp  = COS_VM_DSCP_MAX;         /*secondary key*/

    #if(SYS_CPNT_QOS_V2 == TRUE)
        while(L4_PMGR_COS_GetNextDSCPToCos(&lport, &dscp, &cos) && lport == specified_lport)
    #else
        while(L4_COS_PMGR_GetNextDSCPToCos(&lport, &dscp, &cos) && lport == specified_lport)
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
                {
                    UI8_T name[MAXSIZE_ifName+1] = {0};
                    CLI_LIB_Ifindex_To_Name(lport,name);
                    if (strlen(name) > 10)/*prevent name too long for sprintf*/
                    {                                        /*pttch 2002.07.10*/
                        name[10-1] = 0;
                    }
                    sprintf((char *)UserPort, "%8s", name);
                }
#else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }
            sprintf((char *)buff, " %9s %4d %3lu\r\n", (char *)UserPort, dscp, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
    }
#endif /* #if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Map_Ip_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE)
    char    buff[CLI_DEF_MAX_BUFSIZE];
    UI32_T  line_num = 0;
    UI32_T  lport = 0;
    I32_T   tcpport  = -1;
    UI32_T  cos   = 0;

    SWCTRL_Lport_Type_T ret;
    UI32_T              unit;
    UI32_T              port;
    UI32_T              trunk_id;
    UI8_T               UserPort[10];
    COS_VM_TCPPORT_MAPPING_FLAG_T flag;

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_GetTcpPortMappingStatus ((UI32_T *)&flag))
    #else
    if(!L4_COS_PMGR_GetTcpPortMappingStatus (&flag))
    #endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get TCP port mapping status.\r\n\r\n");
#endif
    }
    else
    {
        CLI_LIB_PrintStr_1("TCP Port Mapping Status: %s\r\n\r\n", flag == COS_VM_MAPPING_ENABLED ?
                                                                "Enabled" : "Disabled");
    }

    CLI_LIB_PrintStr(" Port      IP Port  CoS\r\n");
    CLI_LIB_PrintStr(" --------- -------- ---\r\n");
    line_num += 4;

    if(arg[0] == NULL)
    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
        while(L4_PMGR_COS_GetNextPortToCos(&lport, &tcpport, &cos))
    #else
        while(L4_COS_PMGR_GetNextPortToCos(&lport, &tcpport, &cos))
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T   name[MAXSIZE_ifName+1] = {0};

                CLI_LIB_Ifindex_To_Name(lport,name);
                if (strlen(name) > 10)/*prevent name too long for sprintf*/
                {                                        /*pttch 2002.07.10*/
                    name[10-1] = 0;
                }
                    sprintf((char *)UserPort, "%8s", name);
            }
#else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }

            sprintf((char *)buff, " %-9s %8ld %3lu\r\n", UserPort, tcpport, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        UI32_T  specified_lport;

        if(arg[0][0] == 'e' || arg[0][0] == 'E') /*ethernet*/
        {
            CLI_API_EthStatus_T     verify_ret;

            unit = atoi(arg[1]);
            port = atoi(strchr(arg[1], '/')+1);

#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[1][0]))
            {
                unit = atoi(arg[1]);
                port = atoi(strchr(arg[1],'/')+1);
            }
            else/*port name*/
            {
                UI32_T  trunk_id = 0;

                if (!IF_PMGR_IfnameToIfindex(arg[1], &specified_lport))
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
#endif
                    return CLI_NO_ERROR;
                }
                SWCTRL_POM_LogicalPortToUserPort(specified_lport, &unit, &port, &trunk_id);
            }
#endif

            if( (verify_ret = verify_ethernet(unit, port, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, unit, port);
                return CLI_NO_ERROR;
            }
        }
        else                                     /*trunk*/
        {
            CLI_API_TrunkStatus_T verify_ret;

            trunk_id = atoi(arg[1]);

            if( (verify_ret = verify_trunk(trunk_id, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_trunk_msg(verify_ret, trunk_id);
                return CLI_NO_ERROR;
            }
        }

        lport = specified_lport - 1;  /*primary key*/
        tcpport  = COS_VM_TCPPORT_MAX;  /*secondary key*/

    #if(SYS_CPNT_QOS_V2 == TRUE)
        while(L4_PMGR_COS_GetNextPortToCos(&lport, &tcpport, &cos) && lport == specified_lport)
    #else
        while(L4_COS_PMGR_GetNextPortToCos(&lport, &tcpport, &cos) && lport == specified_lport)
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T   name[MAXSIZE_ifName+1] = {0};

                CLI_LIB_Ifindex_To_Name(lport,name);
                if (strlen(name) > 10)/*prevent name too long for sprintf*/
                {                                        /*pttch 2002.07.10*/
                    name[10-1] = 0;
                }
                sprintf((char *)UserPort, "%8s", name);
            }
#else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }
            sprintf((char *)buff, " %9s %8ld %3lu\r\n", UserPort, tcpport, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Map_Ip_Precedence(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE)
    char                buff[CLI_DEF_MAX_BUFSIZE];
    UI32_T              line_num = 0;
    UI32_T              lport = 0;
    UI8_T               tos   = 0;
    UI32_T              cos   = 0;
    SWCTRL_Lport_Type_T ret;
    UI32_T              unit;
    UI32_T              port;
    UI32_T              trunk_id;
    UI8_T               UserPort[10];
    COS_VM_MAPPING_FLAG_T get_flag;

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #else
    if(!L4_COS_PMGR_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #endif
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to get precedence mapping status.\r\n\r\n");
#endif
    }
    else
    {
        CLI_LIB_PrintStr_1("Precedence Mapping Status: %s\r\n\r\n", get_flag == COS_VM_IPPRECEDENCE_MAPPING_ENABLED ?
                                                                  "Enabled" : "Disabled");
    }


    CLI_LIB_PrintStr(" Port      Precedence CoS\r\n");
    CLI_LIB_PrintStr(" --------- ---------- ---\r\n");
    line_num += 4;

    if(arg[0] == NULL)
    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
        while(L4_PMGR_COS_GetNextIpPrecedenceToCos(&lport, &tos, &cos))
    #else
        while(L4_COS_PMGR_GetNextIpPrecedenceToCos(&lport, &tos, &cos))
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T   name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(lport,name);
                if (strlen(name) > 10)/*prevent name too long for sprintf*/
                {                                        /*pttch 2002.07.10*/
                    name[10-1] = 0;
                }
                sprintf((char *)UserPort, "%8s", name);
            }
#else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }
            sprintf((char *)buff, " %-9s %10d %3lu\r\n", UserPort, tos, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
    }
    else
    {
        UI32_T  specified_lport;

        if(arg[0][0] == 'e' || arg[0][0] == 'E') /*ethernet*/
        {
            CLI_API_EthStatus_T verify_ret;

            unit = atoi(arg[1]);
            port = atoi(strchr(arg[1], '/')+1);

#if (CLI_SUPPORT_PORT_NAME == 1)
            if (isdigit(arg[1][0]))
            {
                unit = atoi(arg[1]);
                port = atoi(strchr(arg[1],'/')+1);
            }
            else/*port name*/
            {
                UI32_T trunk_id = 0;

                if (!IF_PMGR_IfnameToIfindex(arg[1], &specified_lport))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[1]);
    #endif
                    return CLI_NO_ERROR;
                }
                SWCTRL_POM_LogicalPortToUserPort(specified_lport, &unit, &port, &trunk_id);
            }
#endif

            if( (verify_ret = verify_ethernet(unit, port, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret, unit, port);
                return CLI_NO_ERROR;
            }
        }
        else                                     /*trunk*/
        {
            CLI_API_TrunkStatus_T verify_ret;

            trunk_id = atoi(arg[1]);

            if( (verify_ret = verify_trunk(trunk_id, &specified_lport)) != CLI_API_ETH_OK)
            {
                display_trunk_msg(verify_ret, trunk_id);
                return CLI_NO_ERROR;
            }
        }

        lport = specified_lport - 1;      /*primary key*/
        tos   = COS_VM_IPPRECEDENCE_MAX; /*secondary key*/

    #if(SYS_CPNT_QOS_V2 == TRUE)
        while(L4_PMGR_COS_GetNextIpPrecedenceToCos(&lport, &tos, &cos) && lport == specified_lport)
    #else
        while(L4_COS_PMGR_GetNextIpPrecedenceToCos(&lport, &tos, &cos) && lport == specified_lport)
    #endif
        {
            ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

            switch(ret)
            {
            case SWCTRL_LPORT_NORMAL_PORT:

    #if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T   name[MAXSIZE_ifName+1] = {0};

                CLI_LIB_Ifindex_To_Name(lport,name);
                if (strlen(name) > 10)/*prevent name too long for sprintf*/
                {                                        /*pttch 2002.07.10*/
                    name[10-1] = 0;
                }
                sprintf((char *)UserPort, "%8s", name);
            }
    #else
                sprintf((char *)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
    #endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf((char *)UserPort, "Trunk %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
            }
            sprintf((char *)buff, " %9s %10d %3lu\r\n", UserPort, tos, (unsigned long)cos);
            PROCESS_MORE(buff);
        }
   }
#endif /* #if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Queue_Weight(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == FALSE)
    UI32_T  line_num = 0;

    CLI_API_Local_ShowGlobalQueueWeight(line_num);
#endif /* (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == FALSE) */

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
    UI32_T  lport           = 0;
    UI32_T  line_num        = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  i;
    UI32_T  max_port_num;
    UI32_T  j;
    CLI_API_EthStatus_T verify_ret;

    if (arg[0] == NULL)
    {
        for (j=0; STKTPLG_POM_GetNextUnit(&j); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            /*all eth*/
            for (i = 1; i <= max_port_num; i++)
            {
                verify_unit = j;
                verify_port = i;

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                if (verify_ret != CLI_API_ETH_OK &&
                    verify_ret != CLI_API_ETH_TRUNK_MEMBER)
                {
                    continue;
                }

                CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit,(unsigned long)verify_port);
                line_num += 1;
                if ((line_num = CLI_API_Local_ShowPortQueueWeight(lport,line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
#if 0 /* not support trunk */
            /*all trunk*/
            {
                UI32_T  verify_trunk_id;
                CLI_API_TrunkStatus_T verify_ret;

                for(verify_trunk_id = 1; verify_trunk_id <= SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM; verify_trunk_id++)
                {
                    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                        continue;
                    else
                    {
                        CLI_LIB_PrintStr_1("Information of Trunk %lu\r\n", verify_trunk_id);
                        line_num += 1;
                        if((line_num = CLI_API_Local_ShowPortQueueWeight(lport, line_num)) == JUMP_OUT_MORE)
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
#endif /* #if 0 not support trunk */
        }
    }
    else
    {
        switch(arg[0][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1],'/')+1);

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret != CLI_API_ETH_OK &&
                verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            else
            {
                CLI_LIB_PrintStr_2("Information of Eth %lu/%lu\r\n", (unsigned long)verify_unit,(unsigned long)verify_port);
                line_num += 1;
                if ((line_num = CLI_API_Local_ShowPortQueueWeight(lport,line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
                else if (line_num == EXIT_SESSION_MORE)
                {
                    return CLI_EXIT_SESSION;
                }
            }
            break;

        case 'p':
        case 'P':
            {
                UI32_T  trunk_id = atoi(arg[1]);

                CLI_API_TrunkStatus_T verify_ret;

                if( (verify_ret = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
                {
                    display_trunk_msg(verify_ret, trunk_id);
                    return CLI_NO_ERROR;
                }
                else
                {
                    CLI_LIB_PrintStr_1("Information of Trunk %lu\r\n", (unsigned long)trunk_id);
                    line_num += 1;
                    if((line_num = CLI_API_Local_ShowPortQueueWeight(lport, line_num)) == JUMP_OUT_MORE)
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
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE) */

   return CLI_NO_ERROR;
}

#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
static UI32_T
CLI_API_Local_ShowPortQueueWeight(
    UI32_T ifindex,
    UI32_T line_num)
{
    UI32_T  weight, queue;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};


    CLI_LIB_PrintStr(" Queue ID  Weight\r\n");
    CLI_LIB_PrintStr(" --------  ------\r\n");
    line_num += 2;

    for (queue = 0; queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; queue++)
    {
        COS_VM_QUEUE_STRICT_STATUS_T strict_status;

        if (TRUE == L4_PMGR_QOS_GetPortPriorityEgressQueueStrictStatus(
                        ifindex, COS_TYPE_PRIORITY_USER, queue, &strict_status))
        {
            if (strict_status == COS_VM_QUEUE_STRICT_STATUS_ENABLED)
            {
                SYSFUN_Snprintf(buff, sizeof(buff), "%9lu  strict-queue\r\n", (unsigned long)queue);
                line_num += 1;
            }
            else
            {

                if (TRUE == L4_PMGR_QOS_GetPortPriorityEgressWrrQueueWeight(
                                ifindex, COS_TYPE_PRIORITY_USER, queue, &weight))
                {
                    SYSFUN_Snprintf(buff, sizeof(buff), "%9lu%8lu\r\n", (unsigned long)queue, (unsigned long)weight);
                    line_num += 1;
                }
            }

            PROCESS_MORE_FUNC(buff);
        }
    }

    return line_num;
}
#else
static UI32_T
CLI_API_Local_ShowGlobalQueueWeight(
    UI32_T line_num)
{
    UI32_T  weight, queue;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};


    CLI_LIB_PrintStr(" Queue ID  Weight\r\n");
    CLI_LIB_PrintStr(" --------  ------\r\n");
    line_num += 2;

    for (queue = 0; queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; queue++)
    {
        COS_VM_QUEUE_STRICT_STATUS_T strict_status;

        if (TRUE == L4_PMGR_QOS_GetPriorityEgressQueueStrictStatus(
                        COS_TYPE_PRIORITY_USER, queue, &strict_status))
        {
            if (strict_status == COS_VM_QUEUE_STRICT_STATUS_ENABLED)
            {

                SYSFUN_Snprintf(buff, sizeof(buff), "%9lu  strict-queue\r\n", (unsigned long)queue);
                line_num += 1;
            }
            else
            {

                if (TRUE == L4_PMGR_QOS_GetPriorityEgressWrrQueueWeight(
                                COS_TYPE_PRIORITY_USER, queue, &weight))
                {
                    SYSFUN_Snprintf(buff, sizeof(buff), "%9lu%8lu\r\n", (unsigned long)queue, (unsigned long)weight);
                    line_num += 1;
                }
            }

            PROCESS_MORE_FUNC(buff);
        }
    }

    return line_num;
}
#endif /* SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL */

/*configuration*/
UI32_T CLI_API_Map_Ip_Dscp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE)
    UI32_T  i;
    UI32_T  lport = 0;

    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_MAP_IP_DSCP:
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
    #if(SYS_CPNT_QOS_V2 == TRUE)
                else if (!L4_PMGR_COS_SetDSCPToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #else
                else if (!L4_COS_PMGR_SetDSCPToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set DSCP mapping on port %lu.\r\n", (unsigned long)i);
#endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_MAP_IP_DSCP:
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
                else
                {
                    if(arg[0]==NULL)
                    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                        if (!L4_PMGR_COS_DelDscpLportAllEntry(lport))
    #else
                        if (!L4_COS_PMGR_DelDscpLportAllEntry(lport))
    #endif
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to delete DSCP mapping on port %lu.\r\n", (unsigned long)i);
#endif
                        } //end del all dscp lport
                    }
                    else
                    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                        if (L4_PMGR_COS_DelDSCPToCos(lport,atoi(arg[0]))!=TRUE)
    #else
                        if (L4_COS_PMGR_DelDSCPToCos(lport,atoi(arg[0]))!=TRUE)
    #endif
                        {
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to delete DSCP mapping on port %lu.\r\n", (unsigned long)i);
#endif
                        } //end del select dscp lport
                    }
                }
            } // end if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        } //end for loop
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Dscp_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE)
    UI32_T  lport;
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_MAP_IP_DSCP:
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
    #if(SYS_CPNT_QOS_V2 == TRUE)
        else if(!L4_PMGR_COS_SetDSCPToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #else
        else if(!L4_COS_PMGR_SetDSCPToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #endif
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr_1("Failed to set DSCP mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_MAP_IP_DSCP:
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
        else
        {
            if(arg[0]==NULL)
            {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                if(!L4_PMGR_COS_DelDscpLportAllEntry(lport))
    #else
                if(!L4_COS_PMGR_DelDscpLportAllEntry(lport))
    #endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to delete DSCP mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                }
            }
            else
            {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                if(L4_PMGR_COS_DelDSCPToCos(lport,atoi(arg[0]))!=TRUE)
    #else
                if(L4_COS_PMGR_DelDSCPToCos(lport,atoi(arg[0]))!=TRUE)
    #endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to delete DSCP mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
#endif
                }
            }
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Port(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE)
#if defined(STRATA_SWITCH) || defined(XGS_SWITCH)
    UI32_T  lport = 0;

#if (SYS_CPNT_COS_PER_PORT == TRUE)
    UI32_T  i;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;
#endif

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_MAP_IP_PORT:
#if (SYS_CPNT_COS_PER_PORT == FALSE)
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
    #if(SYS_CPNT_QOS_V2 == TRUE)
            if (!L4_PMGR_COS_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #else
            if (!L4_COS_PMGR_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #endif
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr("Failed to set port mapping on port\r\n");
    #endif
            }
        }
#else
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
        #if(SYS_CPNT_QOS_V2 == TRUE)
                else if (!L4_PMGR_COS_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
        #else
                else if (!L4_COS_PMGR_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
        #endif
                {
#if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
#else
                    CLI_LIB_PrintStr_1("Failed to set port mapping on port %lu.\r\n", (unsigned long)i);
#endif
                }
            }
        }
#endif
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_MAP_IP_PORT:
#if (SYS_CPNT_COS_PER_PORT == FALSE)
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
    #if(SYS_CPNT_QOS_V2 == TRUE)
            if (!L4_PMGR_COS_DelPortToCos(lport , atoi(arg[0])))
    #else
            if (!L4_COS_PMGR_DelPortToCos(lport , atoi(arg[0])))
    #endif
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr("Failed to set port mapping on port\r\n");
    #endif
            }
        }
#else
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
    #if(SYS_CPNT_QOS_V2 == TRUE)
                else if (!L4_PMGR_COS_DelPortToCos(lport , atoi(arg[0])))
    #else
                else if (!L4_COS_PMGR_DelPortToCos(lport , atoi(arg[0])))
    #endif
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_1("Failed to delete port mapping on port %lu.\r\n", (unsigned long)i);
    #endif
                }
            }
        }
#endif /* #if (SYS_CPNT_COS_PER_PORT == FALSE) */
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#else

    CLI_LIB_PrintStr("Not supported.\r\n");

#endif /* #if defined(STRATA_SWITCH) || defined(XGS_SWITCH) */
#endif /* #if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Port_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE)
    UI32_T  lport;
#if (SYS_CPNT_COS_PER_PORT == TRUE)
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;
#endif

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_MAP_IP_PORT:
#if (SYS_CPNT_COS_PER_PORT == FALSE)
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
    #if(SYS_CPNT_QOS_V2 == TRUE)
            if (!L4_PMGR_COS_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #else
            if (!L4_COS_PMGR_SetPortToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #endif
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr("Failed to set port mapping on port\r\n");
    #endif
            }
        }
#else
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
    #if(SYS_CPNT_QOS_V2 == TRUE)
        else if(!L4_PMGR_COS_SetPortToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #else
        else if(!L4_COS_PMGR_SetPortToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #endif
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_1("Failed to set port mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
    #endif
        }
#endif /* #if (SYS_CPNT_COS_PER_PORT == FALSE) */
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_MAP_IP_PORT:
#if (SYS_CPNT_COS_PER_PORT == FALSE)
        if (SWCTRL_POM_GetNextLogicalPort(&lport) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
    #if(SYS_CPNT_QOS_V2 == TRUE)
            if (!L4_PMGR_COS_DelPortToCos(lport , atoi(arg[0])))
    #else
            if (!L4_COS_PMGR_DelPortToCos(lport , atoi(arg[0])))
    #endif
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr("Failed to set port mapping on port\r\n");
    #endif
            }
        }
#else
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
    #if(SYS_CPNT_QOS_V2 == TRUE)
        else if(!L4_PMGR_COS_DelPortToCos(lport, atoi(arg[0])))
    #else
        else if(!L4_COS_PMGR_DelPortToCos(lport, atoi(arg[0])))
    #endif
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_1("Failed to delete port mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
    #endif
        }
#endif /* #if (SYS_CPNT_COS_PER_PORT == FALSE) */
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Precedence(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE)
    UI32_T  i;
    UI32_T  lport = 0;

    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_MAP_IP_PRECEDENCE:
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
    #if(SYS_CPNT_QOS_V2 == TRUE)
                else if (!L4_PMGR_COS_SetIpPrecedenceToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #else
                else if (!L4_COS_PMGR_SetIpPrecedenceToCos(lport , atoi(arg[0]), atoi(arg[2])))
    #endif
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_1("Failed to set precedence mapping on port %lu.\r\n", (unsigned long)i);
    #endif
                }
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_MAP_IP_PRECEDENCE:
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
                else
                {
                    if(arg[0]==NULL)
                    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                        if (!L4_PMGR_COS_DelIpPrecedenceLportAllEntry(lport))
    #else
                        if (!L4_COS_PMGR_DelIpPrecedenceLportAllEntry(lport))
    #endif
                        {
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr_1("Failed to delete precedence mapping on port %lu.\r\n", (unsigned long)i);
    #endif
                        }
                    }  //end if(arg[0]==NULL)
                    else
                    {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                        if(L4_PMGR_COS_DelIpPrecedenceToCos(lport,atoi(arg[0]))!=TRUE)
    #else
                        if(L4_COS_PMGR_DelIpPrecedenceToCos(lport,atoi(arg[0]))!=TRUE)
    #endif
                        {
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr_2("Failed to delete precedence tos mapping %d on port%lu.\r\n",atoi(arg[0]),(unsigned long)i);
    #endif
                        }
                    }
                }
            } //end if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        }  //end for loop
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Precedence_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE)
    UI32_T  lport;
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_MAP_IP_PRECEDENCE:
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
    #if(SYS_CPNT_QOS_V2 == TRUE)
        else if(!L4_PMGR_COS_SetIpPrecedenceToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #else
        else if(!L4_COS_PMGR_SetIpPrecedenceToCos(lport, atoi(arg[0]), atoi(arg[2])))
    #endif
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_1("Failed to set precedence mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
    #endif
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_MAP_IP_PRECEDENCE:
        if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret, verify_trunk_id);
            return CLI_NO_ERROR;
        }
        else
        {
            if(arg[0]==NULL)
            {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                if(!L4_PMGR_COS_DelIpPrecedenceLportAllEntry(lport))
    #else
                if(!L4_COS_PMGR_DelIpPrecedenceLportAllEntry(lport))
    #endif
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_1("Failed to delete precedence mapping on trunk %lu.\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
    #endif
                }  //end if(!L4_COS_PMGR_DelIpPrecedenceLportAllEntry(lport))
            }   //end if(arg[0]==NULL)
            else
            {
    #if(SYS_CPNT_QOS_V2 == TRUE)
                if(L4_PMGR_COS_DelIpPrecedenceToCos(lport,atoi(arg[0]))!=TRUE)
    #else
                if(L4_COS_PMGR_DelIpPrecedenceToCos(lport,atoi(arg[0]))!=TRUE)
    #endif
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr_2("Failed to delete precedence tos mapping %d on trunk %lu.\r\n",atoi(arg[0]),(unsigned long)ctrl_P->CMenu.pchannel_id);
    #endif
                }
            }
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Port_Global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE)
    COS_VM_TCPPORT_MAPPING_FLAG_T flag;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_MAP_IP_PORT:
        flag = COS_VM_MAPPING_ENABLED;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_MAP_IP_PORT:
        flag = COS_VM_MAPPING_DISABLED;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_SetTcpPortMappingStatus(flag))
    #else
    if(!L4_COS_PMGR_SetTcpPortMappingStatus(flag))
    #endif
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_1("Failed to %s port mapping\r\n", flag == COS_VM_MAPPING_ENABLED ? "enable" : "disable");
    #endif
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Dscp_Global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE)
    COS_VM_MAPPING_FLAG_T get_flag;
    COS_VM_MAPPING_FLAG_T set_flag;

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #else
    if(!L4_COS_PMGR_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #endif
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get precedence and DSCP mapping status\r\n");
    #endif
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_MAP_IP_DSCP:
        set_flag = COS_VM_DSCP_MAPPING_ENABLED;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_MAP_IP_DSCP:
        if(get_flag == COS_VM_IPPRECEDENCE_MAPPING_ENABLED)
            return CLI_NO_ERROR; /*do nothing*/
        else
            set_flag = COS_VM_DISABLE_BOTH;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_SetIpPrecedenceDscpMappingStatus(set_flag))
    #else
    if(!L4_COS_PMGR_SetIpPrecedenceDscpMappingStatus(set_flag))
    #endif
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_1("Failed to %s DSCP mapping\r\n", cmd_idx == PRIVILEGE_CFG_GLOBAL_CMD_W3_MAP_IP_DSCP ? "enable" : "disable");
    #endif
    }
#endif /* #if (SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_Ip_Precedence_Global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE)
    COS_VM_MAPPING_FLAG_T get_flag;
    COS_VM_MAPPING_FLAG_T set_flag;

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #else
    if(!L4_COS_PMGR_GetIpPrecedenceDscpMappingStatus(&get_flag))
    #endif
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr("Failed to get precedence and DSCP mapping status\r\n");
    #endif
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_MAP_IP_PRECEDENCE:
        set_flag = COS_VM_IPPRECEDENCE_MAPPING_ENABLED;
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_MAP_IP_PRECEDENCE:
        if(get_flag == COS_VM_DSCP_MAPPING_ENABLED)
            return CLI_NO_ERROR; /*do nothing*/
        else
            set_flag = COS_VM_DISABLE_BOTH;
        break;

    default:
        return CLI_ERR_INTERNAL;
    }

    #if(SYS_CPNT_QOS_V2 == TRUE)
    if(!L4_PMGR_COS_SetIpPrecedenceDscpMappingStatus(set_flag))
    #else
    if(!L4_COS_PMGR_SetIpPrecedenceDscpMappingStatus(set_flag))
    #endif
    {
    #if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
    #else
        CLI_LIB_PrintStr_1("Failed to %s precedence mapping\r\n", cmd_idx ==  PRIVILEGE_CFG_GLOBAL_CMD_W3_MAP_IP_PRECEDENCE ? "enable" : "disable");
    #endif
    }
#endif /* #if (SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Queue_Bandwidth_Global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == FALSE)
    UI32_T queue;
    UI16_T config_priority = COS_TYPE_PRIORITY_USER;
    UI32_T default_queue_weight[8] =
    {
        SYS_DFLT_WRR_Q0_WEIGHT,
        SYS_DFLT_WRR_Q1_WEIGHT,
        SYS_DFLT_WRR_Q2_WEIGHT,
        SYS_DFLT_WRR_Q3_WEIGHT,
        SYS_DFLT_WRR_Q4_WEIGHT,
        SYS_DFLT_WRR_Q5_WEIGHT,
        SYS_DFLT_WRR_Q6_WEIGHT,
        SYS_DFLT_WRR_Q7_WEIGHT
    };

    if (arg[0] != NULL)
    {
#if 0 /* change design. weight can be set in any order */
        if (atoi(arg[0])>atoi(arg[1]) || atoi(arg[1])>atoi(arg[2]) || atoi(arg[2])>atoi(arg[3]) )
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_SetandShow_Exception_Handeler_Msg(EH_TYPE_MSG_INVALID_PARAMETER, CLI_API_QUEUE_BANDWIDTH_GLOBAL, "The value of weight must satisfy W1 < W2 < W3 < W4");
    #else
            CLI_LIB_PrintStr("Failed to set queue bandwidth because of order error\r\n\r\n");
    #endif
            return CLI_NO_ERROR;
        }
#endif
   }

    /* set all value=1 */
    for (queue = 0; queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; queue++)
    {
        if (TRUE != L4_PMGR_QOS_SetPriorityEgressWrrQueueWeight(config_priority, queue, 1))
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_1("Failed to set queue bandwidth weight%lu \r\n", (unsigned long)queue);
    #endif
            return CLI_NO_ERROR;
        }
    }

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_QUEUE_WEIGHT:
        for (queue = 0;
             (NULL != arg[queue]) && (queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE);
             queue++)
        {
            if (TRUE != L4_PMGR_QOS_SetPriorityEgressWrrQueueWeight(
                            config_priority, queue, atoi(arg[queue])))
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr("Failed to set queue bandwidth\r\n");
    #endif
                return CLI_NO_ERROR;
            }
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_QUEUE_WEIGHT:
        for (queue = 0; queue < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; queue++)
        {
            if (TRUE != L4_PMGR_QOS_SetPriorityEgressWrrQueueWeight(
                            config_priority, queue, default_queue_weight[queue]))
            {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue bandwidth\r\n");
    #endif
                    return CLI_NO_ERROR;
            }
        }
        break;

    default:
        return CLI_NO_ERROR;
    }
#endif /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == FALSE) */

   return CLI_NO_ERROR;
}

/*2008-06-03, Jinfeng.Chen:
    Add this for weights per port support
 */
UI32_T CLI_API_Queue_Weight_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
    UI32_T  i,j       = 0;
    UI32_T  lport;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    UI8_T   dflt_weights [SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]
                = {SYS_DFLT_WRR_Q0_WEIGHT,
                   SYS_DFLT_WRR_Q1_WEIGHT,
                   SYS_DFLT_WRR_Q2_WEIGHT,
                   SYS_DFLT_WRR_Q3_WEIGHT,

    #if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE == 8)
                   SYS_DFLT_WRR_Q4_WEIGHT,
                   SYS_DFLT_WRR_Q5_WEIGHT,
                   SYS_DFLT_WRR_Q6_WEIGHT,
                   SYS_DFLT_WRR_Q7_WEIGHT
    #endif
                      };

    UI32_T weights[SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE];

    if (arg[0] != NULL)
    {
        for(i = 0; i < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
            weights[i] = atoi((char *)arg[i]);

#if 0 /* change design. weight can be set in any order */
        for(i = SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE - 1; i > 0; i--)
        {
            if(weights[i] < weights[i - 1])
            {
                CLI_LIB_PrintStr("Failed to set queue weight because of order error\r\n\r\n");

                return CLI_NO_ERROR;
            }
        }
#endif
    }

    for (j = 1; j <= ctrl_P->sys_info.max_port_number; j++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((j-1)/8)]  & (1 << ( 7 - ((j-1)%8))) )
        {
            verify_port = j;

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret != CLI_API_ETH_OK &&
                verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_QUEUE_WEIGHT:
                    for(i=SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;i>0;i--)
                    {
                        if (arg[i-1] == NULL)
                            break;
                        if(!L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport, i-1, weights[i-1]))
                        {
    #if (CLI_SUPPORT_PORT_NAME == 1)
                            UI8_T name[MAXSIZE_ifName+1] = {0};

                            CLI_LIB_Ifindex_To_Name(lport,name);
        #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
        #else
                            CLI_LIB_PrintStr_1("Failed to set queue weight on ethernet %s\r\n", name);
                          #endif
                        #else
                          #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
                          #else
                            CLI_LIB_PrintStr_2("Failed to set queue weight on ethernet %lu/%lu\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                          #endif
                        #endif /* #if (CLI_SUPPORT_PORT_NAME == 1) */
                            return CLI_NO_ERROR;
                        }
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_QUEUE_WEIGHT:

                    for (i=SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i>0; i--)
                    {
                        if (FALSE == L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport, i-1, dflt_weights[i-1]))
                        {
                        #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
                        #else
                            CLI_LIB_PrintStr_2("Failed to set default queue weights on ethernet %lu/%lu\r\n",(unsigned long)verify_unit, (unsigned long)verify_port);
                        #endif
                            return CLI_NO_ERROR;
                        }
                    }
                    break;

                default:
                    return CLI_ERR_INTERNAL;
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Queue_Bandwidth_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
// not support trunk
#if 0 && (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE)
    UI32_T  i       = 0;
    UI32_T  lport;
    UI8_T   dflt_qbw [SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE]
                   = {SYS_DFLT_WRR_Q0_WEIGHT,
                      SYS_DFLT_WRR_Q1_WEIGHT,
                      SYS_DFLT_WRR_Q2_WEIGHT,
                      SYS_DFLT_WRR_Q3_WEIGHT,

    #if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE == 8)
                      SYS_DFLT_WRR_Q4_WEIGHT,
                      SYS_DFLT_WRR_Q5_WEIGHT,
                      SYS_DFLT_WRR_Q6_WEIGHT,
                      SYS_DFLT_WRR_Q7_WEIGHT
    #endif
                      };
    CLI_API_TrunkStatus_T verify_ret;


    if (arg[0] != NULL)
    {
#if 0 /* change design. weight can be set in any order */
    #if (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE == 8)
        if (atoi(arg[0])>atoi(arg[1]) || atoi(arg[1])>atoi(arg[2]) || atoi(arg[2])>atoi(arg[3]) || atoi(arg[3])>atoi(arg[4])
         || atoi(arg[4])>atoi(arg[5]) || atoi(arg[5])>atoi(arg[6]) || atoi(arg[6])>atoi(arg[7]))
    #else
        if (atoi(arg[0])>atoi(arg[1]) || atoi(arg[1])>atoi(arg[2]) || atoi(arg[2])>atoi(arg[3]) )
    #endif
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_SetandShow_Exception_Handeler_Msg(EH_TYPE_MSG_INVALID_PARAMETER, CLI_API_QUEUE_BANDWIDTH_GLOBAL, "The value of weight must satisfy W1 < W2 < W3 < W4");
    #else
            CLI_LIB_PrintStr("Failed to set queue bandwidth because of order error\r\n\r\n");
    #endif
            return CLI_NO_ERROR;
        }
#endif
    }

    SWCTRL_POM_TrunkIDToLogicalPort(ctrl_P->CMenu.pchannel_id, &lport);
    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }

    /* set the bandwidth of each queue to 1,
     *  then the set & reset can be done successfully.
     */
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
    {
        if(FALSE == L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport, i, 1))
        {
    #if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
    #else
            CLI_LIB_PrintStr_1("Failed to set queue bandwidth of queue %lu \r\n", i);
    #endif
            return CLI_NO_ERROR;
        }
    }

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_QUEUE_BANDWIDTH:

        for(i=SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;i>0;i--)
        {
            if (arg[i-1] == NULL)
                break;
            if(!L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport,i-1,atoi(arg[i-1])))
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_1("Failed to set queue bandwidth on port-channel %lu\r\n",ctrl_P->CMenu.pchannel_id);
    #endif
                return CLI_NO_ERROR;
            }
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_QUEUE_BANDWIDTH:

        for (i=SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i>0; i--)
        {
            if(FALSE == L4_PMGR_QOS_SetPortEgressWrrQueueWeight(lport, i-1, dflt_qbw [i-1]))
            {
    #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
    #else
                CLI_LIB_PrintStr_1("Failed to set queue bandwidth on port-channel %lu\r\n",ctrl_P->CMenu.pchannel_id);
    #endif
                return CLI_NO_ERROR;
            }
        }
        break;

    default:
        return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL == TRUE) */

   return CLI_NO_ERROR;
}

/*  2008-08-18, Jinfeng Chen:
    According to Dan Xie's view, remove the map command, never used again.
 */
UI32_T CLI_API_Queue_Cosmap_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
    UI32_T  i     = 0;
    UI32_T  j     = 0;
    UI32_T  k     = 0;
    UI32_T  lport = 0;
    UI32_T  queue_id = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
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
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_QUEUE_COSMAP:
                    queue_id = atoi(arg[0]);
                    if (arg[1] != NULL)
                    {
                        if (restore_cos_value_to_default(lport,queue_id))
                        {
                            for(j = 1; arg[j]; j++)
                            {
                                if(!PRI_PMGR_SetDot1dTrafficClass(lport,atoi(arg[j]), atoi(arg[0])))
                                {
    #if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                                    CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
    #endif
                                    break;
                                }
                            }
                        }
                        else
                        {
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
    #endif
                        }
                    }
                    else
                    {
                        if (!restore_cos_value_to_default(lport,queue_id))
                        {
                            CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
                        }
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_QUEUE_COSMAP:
                    for (k=0;k<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;k++)
                    {
                        if (!restore_cos_value_to_default(lport,k))
                        {
                            CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
                        }
                    }
                    break;

                default:
                    return CLI_NO_ERROR;
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Queue_Cosmap_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
    UI32_T  lport,i,j;
    UI32_T  queue_id;
    UI32_T  verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }
    else
    {
        switch(cmd_idx)
        {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_QUEUE_COSMAP:
            queue_id = atoi(arg[0]);
            if (arg[1] != NULL)
            {
               if (restore_cos_value_to_default(lport,queue_id))
               {
                    for(j = 1; arg[j]; j++)
                    {
                        if(!PRI_PMGR_SetDot1dTrafficClass(lport,atoi(arg[j]), atoi(arg[0])))
                        {
    #if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
    #else
                            CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
    #endif
                            break;
                        }
                    }
                }
                else
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
    #endif
                }
            }
            else
            {
                if (!restore_cos_value_to_default(lport,queue_id))
                {
                    CLI_LIB_PrintStr("Failed to set queue cos-map\r\n");
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_QUEUE_COSMAP:
            for (i=0;i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE;i++)
            {
                if (!restore_cos_value_to_default(lport,i))
                {
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
                }
            }
            break;

        default:
            return CLI_NO_ERROR;
        }
    }
#endif /* #if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_COS_ING_COS_TO_QUEUE == TRUE)
static BOOL_T restore_cos_value_to_default(UI32_T lport, UI32_T queue_id)
{
    UI32_T  i        = 0;
//   UI32_T priority = 0;
    PRI_MGR_Dot1dTrafficClassEntry_T traffic_class_entry;

    memset(&traffic_class_entry, 0,sizeof(PRI_MGR_Dot1dTrafficClassEntry_T));

    for (i=0;i<MAX_dot1dTrafficClassPriority+1;i++)
    {
        if (PRI_PMGR_GetDot1dTrafficClassEntry(lport, i, &traffic_class_entry) && traffic_class_entry.dot1d_traffic_class == queue_id)
        {
            switch(i)
            {
            case 0:/*cos0*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 0, SYS_DFLT_1P_0_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 1:/*cos1*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 1, SYS_DFLT_1P_1_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }

                break;

            case 2:/*cos2*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 2, SYS_DFLT_1P_2_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 3:/*cos3*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 3, SYS_DFLT_1P_3_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 4:/*cos4*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 4, SYS_DFLT_1P_4_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 5:/*cos5*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 5, SYS_DFLT_1P_5_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 6:/*cos6*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 6, SYS_DFLT_1P_6_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            case 7:/*cos7*/
                if(!PRI_PMGR_SetDot1dTrafficClass(lport, 7, SYS_DFLT_1P_7_MAPPED_TRAFFIC_CLASS))
                {
    #if (SYS_CPNT_EH == TRUE)
                    CLI_API_Show_Exception_Handeler_Msg();
    #else
                    CLI_LIB_PrintStr("Failed to set queue cos-map as default\r\n");
    #endif
                    return FALSE;
                }
                break;

            default:
                break;
            }
        }
    }
    return TRUE;
}
#endif /* SYS_CPNT_COS_ING_COS_TO_QUEUE */

UI32_T CLI_API_Show_Queue_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) /* per system */
    UI32_T  mode;

    if (TRUE == L4_PMGR_QOS_GetPriorityEgressQueueMode(
                    COS_TYPE_PRIORITY_USER, &mode))
    {
        if (mode == COS_VM_SCHEDULING_STRICT_PRIORITY)
        {
            CLI_LIB_PrintStr("\r\nQueue Mode : Strict Priority Mode\r\n");
        }
        else  if(mode == COS_VM_SCHEDULING_WEIGHT_ROUND_ROBIN)
        {
            CLI_LIB_PrintStr("\r\nQueue Mode : Weighted Round Robin Mode\r\n");
        }
        else  if(mode == COS_VM_SCHEDULING_METHOD_SP_WRR)
        {
            CLI_LIB_PrintStr("\r\nQueue Mode : Strict Priority +  Weighted Round Robin Mode\r\n");
        }
    }
#else  /* per port */
    UI32_T  lport           = 0;
    UI32_T  line_num        = 0;
    UI32_T  mode;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  i;
    UI32_T  max_port_num;
    UI32_T  current_max_unit = 0;
    UI32_T  j;
    CLI_API_EthStatus_T verify_ret;
    char    ch;
    char    notice_more[] = "---More---";
    int     notice_len = strlen(notice_more);
#if (SYS_CPNT_STACKING == TRUE)
   //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[0] == NULL)
    {
     /* for (j = 1; j <= current_max_unit; j++) */
        for (j=0; STKTPLG_POM_GetNextUnit(&j); )
        {
            max_port_num = SWCTRL_POM_UIGetUnitPortNumber(j);
            /*all eth*/
            for (i = 1; i <= max_port_num; i++)
            {
                verify_unit = j;
                verify_port = i;

                verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                if (verify_ret != CLI_API_ETH_OK &&
                    verify_ret != CLI_API_ETH_TRUNK_MEMBER)
                {
                    continue;
                }

                CLI_LIB_PrintStr("Unit   Port   queue mode\r\n");
                CLI_LIB_PrintStr("----   ----   ---------------\r\n");
                CLI_LIB_PrintStr_2("%4lu   %4lu   ", (unsigned long)verify_unit, (unsigned long)verify_port);
                if (TRUE == L4_PMGR_QOS_GetPortEgressQueueMode(lport, &mode))
                {
                    switch(mode)
                    {
                        case SWCTRL_STRICT_PRIORITY_METHOD:
                            CLI_LIB_PrintStr("Strict priority\r\n");
                            break;
                        case SWCTRL_WEIGHT_ROUND_ROBIN_METHOD:
                            CLI_LIB_PrintStr("Weighted Round Robin\r\n");
                            break;
#if (SYS_CPNT_SWCTRL_Q_MODE_DRR == TRUE)
                        case SWCTRL_DEFICIT_ROUND_ROBIN_METHOD:
                            CLI_LIB_PrintStr("Deficit Round Robin mode\r\n");
                            break;
                        case SWCTRL_SP_DRR_METHOD:
                            CLI_LIB_PrintStr("Strict priority with DRR\r\n");
                            break;
#endif  /*endif SYS_CPNT_SWCTRL_Q_MODE_DRR */
                        case SWCTRL_SP_WRR_METHOD:
                            CLI_LIB_PrintStr("Strict priority with WRR\r\n");
                            break;
                        default:
                            CLI_LIB_PrintStr("Unknown mode\r\n");
                    }
                    line_num++;
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                if(0 == line_num % 8)
                {
                    int k;

                    CLI_LIB_PrintStr(notice_more);
                    CLI_IO_GetKey(&ch);
                    for(k = 0; k < notice_len; k++)
                        CLI_LIB_PrintStr(BACK);

                    if('q' == ch || 'Q' == ch)
                        return CLI_NO_ERROR;
                }
            }
        }
    }
    else
    {
        switch(arg[0][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1],'/')+1);

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret != CLI_API_ETH_OK &&
                verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            else
            {
                CLI_LIB_PrintStr("Unit   Port   queue mode\r\n");
                CLI_LIB_PrintStr("----   ----   ---------------\r\n");
                CLI_LIB_PrintStr_2("%4lu   %4lu   ", (unsigned long)verify_unit, (unsigned long)verify_port);
                if (TRUE == L4_PMGR_QOS_GetPortEgressQueueMode(lport, &mode))
                {
                    switch(mode)
                    {
                        case SWCTRL_STRICT_PRIORITY_METHOD:
                            CLI_LIB_PrintStr("Strict priority\r\n");
                            break;
                        case SWCTRL_WEIGHT_ROUND_ROBIN_METHOD:
                            CLI_LIB_PrintStr("Weighted Round Robin\r\n");
                            break;
#if (SYS_CPNT_SWCTRL_Q_MODE_DRR == TRUE)
                        case SWCTRL_DEFICIT_ROUND_ROBIN_METHOD:
                            CLI_LIB_PrintStr("Deficit Round Robin mode\r\n");
                            break;
                        case SWCTRL_SP_DRR_METHOD:
                            CLI_LIB_PrintStr("Strict priority with DRR\r\n");
                            break;
#endif  /*endif SYS_CPNT_SWCTRL_Q_MODE_DRR */
                        case SWCTRL_SP_WRR_METHOD:
                            CLI_LIB_PrintStr("Strict priority with WRR\r\n");
                            break;
                        default:
                            CLI_LIB_PrintStr("Unknown mode\r\n");
                    }
                }
                else
                {
                    return CLI_ERR_INTERNAL;
                }
            }
            break;

        case 'p':
        case 'P': /*2008-06-05, Jinfeng.Chen:  for trunk*/
            break;

        default:
            return CLI_ERR_INTERNAL;
        }
    }
#endif /* #if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) */

   return CLI_NO_ERROR;
}

/* 2008-06-04, Jinfeng.Chen: reserved for compatibility, not used in firebolt2 */
UI32_T CLI_API_Queue_Mode_Global(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE)
    UI32_T j;
    UI16_T config_priority = COS_TYPE_PRIORITY_USER;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_QUEUE_MODE_STRICT:
            if(TRUE != L4_PMGR_QOS_SetPriorityEgressQueueMode(
                           config_priority, COS_VM_SCHEDULING_STRICT_PRIORITY))
            {
                CLI_LIB_PrintStr("Failed to set strict queue mode.\r\n");
            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_QUEUE_MODE_STRICTWRR:
            if(TRUE != L4_PMGR_QOS_SetPriorityEgressQueueMode(
                           config_priority, COS_VM_SCHEDULING_METHOD_SP_WRR))
            {
                CLI_LIB_PrintStr("Failed to set strict-wrr queue mode.\r\n");
                break;
            }

            for(j= 0; (arg[j] != NULL) && (j < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE); j++)
            {
                int is_strict = atoi(arg[j]);
                if(1 == is_strict)
                {
                    if (FALSE == L4_PMGR_QOS_SetPriorityEgressQueueStrictStatus(config_priority, j, COS_VM_QUEUE_STRICT_STATUS_ENABLED))
                    {
                        CLI_LIB_PrintStr_1("Failed to set queue %lu as strict queue\r\n", (unsigned long)j);
                    }
                }
                else if(0 == is_strict)
                {
                    if (FALSE == L4_PMGR_QOS_SetPriorityEgressQueueStrictStatus(config_priority, j, COS_VM_QUEUE_STRICT_STATUS_DISABLED))
                    {
                        CLI_LIB_PrintStr_1("Failed to set queue %lu as strict queue\r\n", (unsigned long)j);
                    }
                }

            }
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_QUEUE_MODE_WRR:
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_QUEUE_MODE:

            if(TRUE != L4_PMGR_QOS_SetPriorityEgressQueueMode(
                           config_priority, COS_VM_SCHEDULING_WEIGHT_ROUND_ROBIN))
            {
                CLI_LIB_PrintStr("Failed to enable queue mode wrr.\r\n");
            }
            break;
    }
#endif /* #if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == FALSE) */

    return CLI_NO_ERROR;
}

/*2008-06-03, Jinfeng.Chen:
    This function support mode set per port instead of global set.
 */
UI32_T CLI_API_Queue_Mode_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL == TRUE)
    UI32_T i, j;
    UI32_T lport;
    UI32_T verify_unit= ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI16_T config_priority = COS_TYPE_PRIORITY_USER;
    UI32_T strict_queue_bitmap = 0;
    UI32_T result = COS_TYPE_E_NONE;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;

            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if (verify_ret != CLI_API_ETH_OK &&
                verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }

            switch(cmd_idx)
            {
#if (SYS_CPNT_SWCTRL_Q_MODE_DRR == TRUE)
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_QUEUE_MODE_DRR :
                    if(L4_PMGR_QOS_SetPortEgressQueueMode(lport, COS_VM_SCHEDULING_METHOD_DRR)!=TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to set drr queue mode\r\n\r\n");
                    }
                    break;
#endif  /*endif SYS_CPNT_SWCTRL_Q_MODE_DRR */
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_QUEUE_MODE_STRICT :
                    if(L4_PMGR_QOS_SetPortEgressQueueMode(lport, COS_VM_SCHEDULING_STRICT_PRIORITY)!=TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to set strict queue mode\r\n\r\n");
                    }
                    break;
#if (SYS_CPNT_SWCTRL_Q_MODE_DRR == TRUE)
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_QUEUE_MODE_STRICTDRR :
#if 1
                    /* New Design: call one PMGR API to enable strict DRR scheduler & configure all queues.
                     */
                    for (j = 0; (arg[j] != NULL) && (j < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE); j ++)
                    {
                        if (atoi(arg[j]) == 1)
                        {
                            strict_queue_bitmap = strict_queue_bitmap | (1 << j);
                        }
                    }    
                    result = L4_PMGR_QOS_SetPortEgressHybridQueueMode(lport, COS_VM_SCHEDULING_METHOD_SP_DRR, strict_queue_bitmap);
                    if (COS_TYPE_E_NONE != result)
                    {
                        switch (result)
                        {
                            case COS_TYPE_E_PARAMETER_STRICT_QUEUE_PRIORITY:
                                CLI_LIB_PrintStr("Not acceptable, all strict queues priority must be higher than WRR queues.\r\n");
                                break;
                            case COS_TYPE_E_SCHEDULING_METHOD:
                                CLI_LIB_PrintStr("Failed to change scheduling method to Strict-WRR.\r\n");
                                break;
                            case COS_TYPE_E_STRICT_QUEUE_CONFIG:
                                CLI_LIB_PrintStr("Failed to set queue as strict priority.\r\n");
                                break;
                            default:
                                CLI_LIB_PrintStr("Failed to enable Strict-WRR queue mode.\r\n");
                                break;
                        }
                    }
#else
                    /* OLD Design: call one PMGR API to enable strict DRR scheduler, and then call another PMGR API
                     *             x times to configure queue mode (x: number of egress queues).
                     */
                    if(L4_PMGR_QOS_SetPortEgressQueueMode(lport, COS_VM_SCHEDULING_METHOD_SP_DRR)!=TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to set strict-drr queue mode\r\n\r\n");
                        break;
                    }
                    for(j= 0; (arg[j] != NULL) && (j < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE); j++)
                    {
                        int is_strict = atoi(arg[j]);
                        if(1 == is_strict)
                        {
                            if (FALSE == L4_PMGR_QOS_SetPortPriorityEgressQueueStrictStatus(
								lport, 
								config_priority, 
								j, 
								COS_VM_QUEUE_STRICT_STATUS_ENABLED))
                            {
                                CLI_LIB_PrintStr_3("Failed to set port %lu/%lu.%lu as strict queue\r\n", verify_unit, verify_port, j);
                            }
                        }
                        else if (0 == is_strict)
                        {
                            if (FALSE == L4_PMGR_QOS_SetPortPriorityEgressQueueStrictStatus(
								lport, 
								config_priority, 
								j, 
								COS_VM_QUEUE_STRICT_STATUS_DISABLED))
                            {
                                CLI_LIB_PrintStr_3("Failed to set port %lu/%lu.%lu as wrr queue\r\n", verify_unit, verify_port, j);
                            }
                        }
                    }
#endif
                    break;
#endif  /*endif SYS_CPNT_SWCTRL_Q_MODE_DRR */
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_QUEUE_MODE_STRICTWRR :
#if 1
                    /* New Design: call one PMGR API to enable strict WRR scheduler & configure all queues.
                     * New API L4_PMGR_QOS_SetPortEgressHybridQueueMode will check the correctness of strict 
                     * queue priority (all strict queues priority must be higher than WRR queues).
                     */
                    for (j = 0; (arg[j] != NULL) && (j < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE); j ++)
                    {
                        if (atoi(arg[j]) == 1)
                        {
                            strict_queue_bitmap = strict_queue_bitmap | (1 << j);
                        }
                    }    
                    result = L4_PMGR_QOS_SetPortEgressHybridQueueMode(lport, COS_VM_SCHEDULING_METHOD_SP_WRR, strict_queue_bitmap);
                    if (COS_TYPE_E_NONE != result)
                    {
                        switch (result)
                        {
                            case COS_TYPE_E_PARAMETER_STRICT_QUEUE_PRIORITY:
                                CLI_LIB_PrintStr("Not acceptable, all strict queues priority must be higher than WRR queues.\r\n");
                                break;
                            case COS_TYPE_E_SCHEDULING_METHOD:
                                CLI_LIB_PrintStr("Failed to change scheduling method to Strict-WRR.\r\n");
                                break;
                            case COS_TYPE_E_STRICT_QUEUE_CONFIG:
                                CLI_LIB_PrintStr("Failed to set queue as strict priority.\r\n");
                                break;
                            default:
                                CLI_LIB_PrintStr("Failed to enable Strict-WRR queue mode.\r\n");
                                break;
                        }
                    }
#else
                    /* OLD Design: call one PMGR API to enable strict DRR scheduler, and then call another PMGR API
                     *             x times to configure queue mode (x: number of egress queues).
                     */
                    if(L4_PMGR_QOS_SetPortEgressQueueMode(lport, COS_VM_SCHEDULING_METHOD_SP_WRR)!=TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to set strict-wrr queue mode\r\n\r\n");
                        break;
                    }
                    for(j= 0; (arg[j] != NULL) && (j < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE); j++)
                    {
                        int is_strict = atoi(arg[j]);
                        if(1 == is_strict)
                        {
                            if (FALSE == L4_PMGR_QOS_SetPortPriorityEgressQueueStrictStatus(
								lport, 
								config_priority, 
								j, 
								COS_VM_QUEUE_STRICT_STATUS_ENABLED))
                            {
                                CLI_LIB_PrintStr_3("Failed to set port %lu/%lu.%lu as strict queue\r\n", verify_unit, verify_port, j);
                            }
                        }
                        else if (0 == is_strict)
                        {
                            if (FALSE == L4_PMGR_QOS_SetPortPriorityEgressQueueStrictStatus(
								lport, 
								config_priority, 
								j, 
								COS_VM_QUEUE_STRICT_STATUS_DISABLED))
                            {
                                CLI_LIB_PrintStr_3("Failed to set port %lu/%lu.%lu as wrr queue\r\n", verify_unit, verify_port, j);
                            }
                        }
                    }
#endif
                    break;
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_QUEUE_MODE_WRR :
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_QUEUE_MODE :
                    if(L4_PMGR_QOS_SetPortEgressQueueMode(lport, COS_VM_SCHEDULING_WEIGHT_ROUND_ROBIN)!=TRUE)
                    {
                        CLI_LIB_PrintStr("Failed to set wrr queue mode\r\n\r\n");
                    }
                    break;
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Map_IP_AccessList(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE)
#if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE)
    static UI32_T show_one_port_map_accesslist(UI32_T type, UI32_T lport, UI32_T line_num);
    UI32_T  lport = 0;
    UI32_T  line_num = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    if (arg[0] == NULL)
    {
        {
            UI32_T  i,current_max_unit;
            UI32_T  max_port_num;

#if (SYS_CPNT_STACKING == TRUE)
            //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
            current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
            current_max_unit = 1;
#endif
            /* for(i = 1; i <= current_max_unit; i++) */
            for (i=0; STKTPLG_POM_GetNextUnit(&i); )
            {
                verify_unit = i;
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

                for(verify_port = 1; verify_port <= max_port_num; verify_port++)
                {
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                    switch(verify_ret)
                    {
                    case CLI_API_ETH_OK:
                    case CLI_API_ETH_TRUNK_MEMBER:
                        if((line_num = show_one_port_map_accesslist(ACL_IP_ACL, lport, line_num)) ==  JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;

                    case CLI_API_ETH_NOT_PRESENT:
                        break;

                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                    }
                }
            }//end of unit loop
        }
   }
   else
   {
        verify_unit = atoi(arg[1]);
        verify_port = atoi(strchr(arg[1],'/')+1);
        verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
        switch(verify_ret)
        {
        case CLI_API_ETH_OK:
        case CLI_API_ETH_TRUNK_MEMBER:

            if((line_num = show_one_port_map_accesslist(ACL_IP_ACL, lport, line_num)) ==  JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            break;

        case CLI_API_ETH_NOT_PRESENT:
            break;

        case CLI_API_ETH_UNKNOWN_PORT:
            break;
        }
    }
#endif /* #if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE) */
#endif /* #if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE) */

    return CLI_NO_ERROR;
}
UI32_T CLI_API_Show_Map_MAC_AccessList(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE)
#if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE)
    static UI32_T show_one_port_map_accesslist(UI32_T type, UI32_T lport, UI32_T line_num);
    UI32_T  lport = 0;
    UI32_T  line_num = 0;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;

    if (arg[0] == NULL)
    {
        {
            UI32_T  i,current_max_unit;
            UI32_T  max_port_num;

#if (SYS_CPNT_STACKING == TRUE)
            //STKTPLG_MGR_GetNumberOfUnit(&current_max_unit);
            current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
            current_max_unit = 1;
#endif
            /* for(i = 1; i <= current_max_unit; i++) */
            for (i=0; STKTPLG_POM_GetNextUnit(&i); )
            {
                verify_unit = i;
                max_port_num = SWCTRL_POM_UIGetUnitPortNumber(verify_unit);

                for(verify_port = 1; verify_port <= max_port_num; verify_port++)
                {
                    verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
                    switch(verify_ret)
                    {
                    case CLI_API_ETH_OK:
                    case CLI_API_ETH_TRUNK_MEMBER:
                        if((line_num = show_one_port_map_accesslist(ACL_MAC_ACL, lport, line_num)) ==  JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                        else if (line_num == EXIT_SESSION_MORE)
                        {
                            return CLI_EXIT_SESSION;
                        }
                        break;

                    case CLI_API_ETH_NOT_PRESENT:
                        break;

                    case CLI_API_ETH_UNKNOWN_PORT:
                        break;
                    }
                }
            }//end of unit loop
        }
   }
   else
   {
        verify_unit = atoi(arg[1]);
        verify_port = atoi(strchr(arg[1],'/')+1);
        verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
        switch(verify_ret)
        {
        case CLI_API_ETH_OK:
        case CLI_API_ETH_TRUNK_MEMBER:
            if((line_num = show_one_port_map_accesslist(ACL_MAC_ACL, lport, line_num)) ==  JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            break;

        case CLI_API_ETH_NOT_PRESENT:
            break;

        case CLI_API_ETH_UNKNOWN_PORT:
            break;
         }
    }
#endif /* #if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE) */
#endif /* #if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE) */

   return CLI_NO_ERROR;
}


UI32_T CLI_API_Map_IP_AccessList_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE)
#if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE)
    UI32_T  i     = 0;
    UI32_T  lport = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;
    COS_TYPE_AclCosEntry_T cos_entry;

    memset(&cos_entry, 0, sizeof(cos_entry));

    strcpy(cos_entry.acl_name, arg[0]);
    cos_entry.cos = atoi(arg[2]);
    cos_entry.acl_type = ACL_IP_ACL;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if( verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_MAP_ACCESSLIST_IP:
                #if(SYS_CPNT_QOS_V2 == TRUE)
                    if (L4_PMGR_COS_AddCosEntry(lport, &cos_entry) == FALSE)
                #else
                    if (L4_COS_PMGR_AddCosEntry(lport, &cos_entry) == FALSE)
                #endif
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to bind access-list to cos\r\n");
#endif
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_MAP_ACCESSLIST_IP:
                #if(SYS_CPNT_QOS_V2 == TRUE)
                    if (L4_PMGR_COS_DelCosEntry(lport, &cos_entry) == FALSE)
                #else
                    if (L4_COS_PMGR_DelCosEntry(lport, &cos_entry) == FALSE)
                #endif
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove access-list to cos\r\n");
#endif
                    }
                    break;

                default:
                    return CLI_NO_ERROR;
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE) */
#endif /* #if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE) */

   return CLI_NO_ERROR;
}

UI32_T CLI_API_Map_MAC_AccessList_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE)
#if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE)
    UI32_T  i     = 0;
    UI32_T  lport = 0;
    UI32_T  verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T  verify_port;
    CLI_API_EthStatus_T verify_ret;
    COS_TYPE_AclCosEntry_T cos_entry;

    memset(&cos_entry, 0, sizeof(cos_entry));

    strcpy(cos_entry.acl_name, arg[0]);
    cos_entry.cos = atoi(arg[2]);
    cos_entry.acl_type = ACL_MAC_ACL;

    for (i = 1; i <= ctrl_P->sys_info.max_port_number; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
        {
            verify_port = i;
            verify_ret = verify_ethernet(verify_unit, verify_port, &lport);
            if( verify_ret != CLI_API_ETH_OK && verify_ret != CLI_API_ETH_TRUNK_MEMBER)
            {
                display_ethernet_msg(verify_ret, verify_unit, verify_port);
                continue;
            }
            else
            {
                switch(cmd_idx)
                {
                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_MAP_ACCESSLIST_MAC:
                #if(SYS_CPNT_QOS_V2 == TRUE)
                    if (L4_PMGR_COS_AddCosEntry(lport, &cos_entry) == FALSE)
                #else
                    if (L4_COS_PMGR_AddCosEntry(lport, &cos_entry) == FALSE)
                #endif
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to bind access-list to cos\r\n");
#endif
                    }
                    break;

                case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_MAP_ACCESSLIST_MAC:
                #if(SYS_CPNT_QOS_V2 == TRUE)
                    if (L4_PMGR_COS_DelCosEntry(lport, &cos_entry) == FALSE)
                #else
                    if (L4_COS_PMGR_DelCosEntry(lport, &cos_entry) == FALSE)
                #endif
                    {
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr("Failed to remove access-list to cos\r\n");
#endif
                    }
                    break;

                default:
                    return CLI_NO_ERROR;
                }
            }
        }
    }
#endif /* #if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE) */
#endif /* #if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE) */

   return CLI_NO_ERROR;
}

#if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE)
#if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE)
static UI32_T show_one_port_map_accesslist(UI32_T type, UI32_T lport, UI32_T line_num)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T  unit,port,trunk;
    UI8_T   acl_type[4] = {0};
    BOOL_T  is_show = FALSE;
    COS_TYPE_AclCosEntry_T cos_entry;

    memset(&cos_entry, 0, sizeof(cos_entry));
    SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk);

#if(SYS_CPNT_QOS_V2 == TRUE)
    while(L4_PMGR_COS_GetNextCosEntry(lport, &cos_entry) == TRUE)
#else
    while(L4_COS_PMGR_GetNextCosEntry(lport, &cos_entry) == TRUE)
#endif
    {
        if (cos_entry.acl_type == ACL_IP_ACL && type == ACL_IP_ACL)
        {
            if (is_show == FALSE)
            {
                sprintf((char *)buff, "Eth %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
                PROCESS_MORE_FUNC(buff);
                is_show = TRUE;
            }
            strcpy(acl_type, "ip");
            sprintf((char *)buff," access-list %s %s cos %lu\r\n", acl_type, cos_entry.acl_name ,(unsigned long)cos_entry.cos);
            PROCESS_MORE_FUNC(buff);
        }
        else if(cos_entry.acl_type == ACL_MAC_ACL && type == ACL_MAC_ACL)
        {
            if (is_show == FALSE)
            {
                sprintf((char *)buff, "Eth %lu/%lu\r\n", (unsigned long)unit, (unsigned long)port);
                PROCESS_MORE_FUNC(buff);
                is_show = TRUE;
            }
            strcpy(acl_type, "mac");
            sprintf((char *)buff," access-list %s %s cos %lu\r\n", acl_type, cos_entry.acl_name ,(unsigned long)cos_entry.cos);
            PROCESS_MORE_FUNC(buff);
        }
        memset(&acl_type, 0, sizeof(acl_type));
    }

    return line_num;
}
#endif /* #if (SYS_CPNT_COS == TRUE && SYS_CPNT_ACL_UI == TRUE) */

#endif /* #if(SYS_CPNT_COS_CONTAIN_CLI_MAP_ACCESS_LIST == TRUE) */

