#include <stdio.h>
#include <stdlib.h>
#include "cli_api.h"
#include "cli_cmd.h"
#if (SYS_CPNT_MLDSNP == TRUE)
#include "cli_api_mldsnp.h"
#include "mldsnp_pmgr.h"
#include "mldsnp_pom.h"
#include "mldsnp_type.h"
#include "netcfg_type.h"
#include "vlan_pom.h"
#include "l_inet.h"
#include "stktplg_pom.h"
#include "swctrl_pom.h"
#include "if_pmgr.h"
#include "netcfg_pmgr_route.h"

#ifndef isspace
#define isspace(c)  ((c) == ' ')
#endif

UI32_T CLI_API_Ipv6_Mld_Snooping(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IPV6_MLD_SNOOPING:
            if(MLDSNP_TYPE_RETURN_FAIL==MLDSNP_PMGR_SetMldStatus(MLDSNP_TYPE_MLDSNP_ENABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable MLD snooping\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IPV6_MLD_SNOOPING:
            if(MLDSNP_TYPE_RETURN_FAIL==MLDSNP_PMGR_SetMldStatus(MLDSNP_TYPE_MLDSNP_DISABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable MLD snooping\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;

    }
    return CLI_NO_ERROR;

}


UI32_T CLI_API_Ipv6_Mld_Snooping_Querier(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERIER == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_QUERIER:
            if(MLDSNP_TYPE_RETURN_FAIL==MLDSNP_PMGR_SetQuerierStatus(MLDSNP_TYPE_QUERIER_ENABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable MLD snooping querier\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_QUERIER:
            if(MLDSNP_TYPE_RETURN_FAIL==MLDSNP_PMGR_SetQuerierStatus(MLDSNP_TYPE_QUERIER_DISABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable MLD snooping querier\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Ipv6_Mld_Snooping_Robustness(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T robust_value=0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_ROBUSTNESS:
            robust_value=atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_ROBUSTNESS:
            robust_value=MLDSNP_TYEP_DFLT_ROBUST_VALUE;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(MLDSNP_TYPE_RETURN_FAIL==MLDSNP_PMGR_SetRobustnessValue(robust_value))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set the MLD query count\r\n");
#endif
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ipv6_Mld_Snooping_Queryinterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERIER == TRUE)
    UI32_T query_interval=0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_QUERYINTERVAL:
            query_interval=atoi(arg[0]);
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_QUERYINTERVAL:
            query_interval=MLDSNP_TYPE_DFLT_QUERY_INTERVAL;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetQueryInterval(query_interval))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set MLD query interval\r\n");
#endif
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Ipv6_Mld_Snooping_Querymaxresponsetime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERIER == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_QUERYMAXRESPONSETIME:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetQueryResponseInterval(atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the maximum response time for a general query\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_QUERYMAXRESPONSETIME:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetQueryResponseInterval(MLDSNP_TYPE_DFLT_MAX_RESP_INTERVAL))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to restore the the maximum response time for a general query to default value\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Ipv6_Mld_Snooping_Routerportexpiretime(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_ROUTERPORTEXPIRETIME:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetRouterExpireTime(atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the router port expire time\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_ROUTERPORTEXPIRETIME:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetRouterExpireTime(MLDSNP_TYPE_DFLT_ROUTER_EXP_TIME))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to restore the router port expire time to default value\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;

}

UI32_T CLI_API_Ipv6_Mld_Snooping_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_VERSION:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetMldSnpVer(atoi(arg[0])))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to configure the version of MLD snooping\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_VERSION:
            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetMldSnpVer(MLDSNP_TYPE_DFLT_VER))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to restore the version of MLD snooping to default value\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Ipv6_Mld_Snooping_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T vid;
    UI32_T lport;
    UI32_T verify_unit;
    UI32_T verify_port;
    CLI_API_EthStatus_T   verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    UI32_T verify_trunk_id;
    UI8_T group_addr[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI8_T source_addr[SYS_ADPT_IPV6_ADDR_LEN] ={0};

    char  *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};

    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T  err_idx;
    delemiters[0] = ',';

    if(arg[0]!=NULL)
        op_ptr = arg[0];
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
            if (!VLAN_POM_IsVlanExisted(vid))
            {
        #if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
        #else
                CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)vid);
        #endif
                continue;
            }

            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_VLAN:

                    switch(arg[1][0])
                    {
                        case 'm': /*mrouter*/
                        case 'M':
                            if(arg[2][0] == 'e' || arg[2][0] == 'E') /*ethernet*/
                            {
                                verify_unit = atoi(arg[3]);
                                verify_port = atoi(strchr(arg[3],'/')+1);

        #if (SYS_CPNT_3COM_CLI == FALSE)
                                if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                                {
        #if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )
                                    char  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                                    UI8_T  master_id = 0;

                                    STKTPLG_POM_GetMasterUnitId( & master_id );

                                    /*   if the port is module port, save command in buffer
                                    */
                                    if ( TRUE == STKTPLG_POM_IsModulePort( verify_unit, verify_port ) )
                                    {
                                        sprintf( cmd_buff, "ipv6 mld snooping vlan %s mrouter ethernet %s\n!\n", arg[0], arg[3] );
                                        CLI_MGR_AddDeviceCfg( verify_unit + SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, (UI8_T *)cmd_buff );
                                        return CLI_NO_ERROR;
                                    }
        #else
                                    UI32_T ifindex;
                                    BOOL_T is_inherit        =TRUE;
                                    char  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                                    SWCTRL_POM_UIUserPortToIfindex(verify_unit, verify_port, &ifindex, &is_inherit);

                                    /*if the port is module port, save command in buffer*/
                                    if (TRUE == CLI_MGR_IsModulePort(ifindex))
                                    {
                                        sprintf(cmd_buff,"ipv6 mld snooping vlan %s mrouter ethernet %s\n!\n", arg[0], arg[3]);
                                        CLI_MGR_AddModuleCfg(verify_unit, (UI8_T *) cmd_buff);
                                        return CLI_NO_ERROR;
                                    }
        #endif
                                }
        #endif

        #if (CLI_SUPPORT_PORT_NAME == 1)
                                if (isdigit(arg[3][0]))
                                {
                                    verify_unit = atoi(arg[3]);
                                    verify_port = atoi(strchr(arg[3],'/')+1);
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
                                if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                                {
                                    display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                                    return CLI_NO_ERROR;
                                }
                            }
                            else
                            {
                                verify_trunk_id = atoi(arg[3]);

                                if( (verify_ret_t = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                                {
                                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                                    return CLI_NO_ERROR;
                                }
                            }

                            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_AddStaticRouterPort(vid, lport))
                            {
        #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
        #else
                                CLI_LIB_PrintStr_1("Failed to add VALN %lu static multicast router port\r\n", (unsigned long)vid);
        #endif
                            }

                            break;

                        case 's': /*static*/
                        case 'S':
                            {
                                MLDSNP_TYPE_CurrentMode_T list_mode=MLDSNP_TYPE_IS_EXCLUDE_MODE;

                                if(L_INET_Pton(L_INET_AF_INET6, arg[2], group_addr)!=TRUE)
                                {
                                    CLI_LIB_PrintStr_1("Failed to set VALN %lu IPv6 address.\r\n", (unsigned long)vid);
                                    return CLI_NO_ERROR;
                                }

                                if(arg[3][0] == 'e' || arg[3][0] == 'E') /*ethernet*/
                                {
                                    verify_unit = atoi(arg[4]);
                                    verify_port = atoi(strchr(arg[4],'/')+1);

                                    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
                                    {
        #if ( SYS_CPNT_UNIT_HOT_SWAP == TRUE )

                                        UI8_T  cmd_buff[CLI_DEF_MAX_BUFSIZE+1] = {0};

                                        UI8_T  master_id = 0;

                                        STKTPLG_POM_GetMasterUnitId( & master_id );

                                        /*   if the port is module port, save command in buffer
                                        */
                                        if ( TRUE == STKTPLG_POM_IsModulePort( verify_unit, verify_port ) )
                                        {
                                            sprintf((char *)cmd_buff,"ipv6 mld snooping vlan %s static %s ethernet %s\n!\n", arg[0], arg[2], arg[4]);
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
                                            sprintf(cmd_buff,"ipv6 mld snooping vlan %s static %s ethernet %s\n!\n", arg[0], arg[2], arg[4]);
                                            CLI_MGR_AddModuleCfg(verify_unit, cmd_buff);
                                            return CLI_NO_ERROR;
                                        }
        #endif
                                    }

        #if (CLI_SUPPORT_PORT_NAME == 1)
                                    if (isdigit(arg[4][0]))
                                    {
                                        verify_unit = atoi(arg[4]);
                                        verify_port = atoi(strchr(arg[4],'/')+1);
                                    }
                                    else/*port name*/
                                    {
                                        UI32_T trunk_id = 0;
                                        if (!IF_PMGR_IfnameToIfindex(arg[4], &lport))
                                        {
        #if (SYS_CPNT_EH == TRUE)
                                            CLI_API_Show_Exception_Handeler_Msg();
        #else
                                            CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[4]);
        #endif
                                            return CLI_NO_ERROR;
                                        }
                                        SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                                    }
        #endif

                                    if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                                    {
                                        display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                                        return CLI_NO_ERROR;
                                    }
                                }
                                else
                                {
                                    verify_trunk_id = atoi(arg[4]);

                                    if( (verify_ret_t = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                                    {
                                        display_trunk_msg(verify_ret_t, verify_trunk_id);
                                        return CLI_NO_ERROR;
                                    }
                                }

                                /*check list mode*/
                                if(arg[5])
                                {
                                    if(arg[5][0]=='i' || arg[5][0]=='I') /*include list*/
                                    {
                                        list_mode=MLDSNP_TYPE_IS_INCLUDE_MODE;
                                    }
                                    else if(arg[5][0]=='e' || arg[5][0]=='E') /*include list*/
                                    {
                                        list_mode=MLDSNP_TYPE_IS_EXCLUDE_MODE;
                                    }
                                    else
                                    {
                                        CLI_LIB_PrintStr_1("Failed to add VALN %lu static multicast group list mode\r\n", (unsigned long)vid);
                                        return CLI_NO_ERROR;
                                    }

                                    if(L_INET_Pton(L_INET_AF_INET6, arg[6], source_addr)!=TRUE)
                                    {
                                        CLI_LIB_PrintStr_1("Failed to set VALN %lu IPv6 address.\r\n", (unsigned long)vid);
                                        return CLI_NO_ERROR;
                                    }
                                }

                                if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_AddPortStaticJoinGroup(vid, group_addr, source_addr, lport, list_mode))
                                {
                                    UI32_T total=0;

                                    MLDSNP_POM_GetTotalEntry(&total);
                                    if(total == SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY)
                                    {
                                        CLI_LIB_PrintStr_2("Reach VALN %lu allowed group limit %d\r\n", (unsigned long)vid, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY);
                                    }
                                    else
                                    {
                                        CLI_LIB_PrintStr_1("Failed to add VALN %lu static multicast group member port.\r\n", (unsigned long)vid);
                                    }
                                }
                            }
                            break;

                        case 'i':
                        case 'I':
                            if(arg[2]== NULL)  /*immediate leave*/
                            {
                                if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetImmediateLeaveStatus(vid, MLDSNP_TYPE_IMMEDIATE_ENABLED))
                                {
        #if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
        #else
                                    CLI_LIB_PrintStr_1("Failed to enable VALN %lu MLD Snooping Immediate leave\r\n", (unsigned long)vid);
        #endif
                                }
                                break;
                            }
                            else if((arg[2][0]=='b')||(arg[2][0]=='B'))  /*immediate leave by-host-ip*/
                            {
                                if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetImmediateLeaveByHostStatus(vid, MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED))
                                {
        #if (SYS_CPNT_EH == TRUE)
                                    CLI_API_Show_Exception_Handeler_Msg();
        #else
                                    CLI_LIB_PrintStr_1("Failed to enable VALN %lu MLD Snooping Immediate leave by host ip\r\n", (unsigned long)vid);
        #endif
                                }
                                break;
                            }
                        default:
                            return CLI_ERR_INTERNAL;
                    }
                    break;

                case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_VLAN:

                    switch(arg[1][0])
                    {
                        case 'm': /*mrouter*/
                        case 'M':
                            if(arg[2][0] == 'e' || arg[2][0] == 'E') /*ethernet*/
                            {
                                verify_unit = atoi(arg[3]);
                                verify_port = atoi(strchr(arg[3],'/')+1);

        #if (CLI_SUPPORT_PORT_NAME == 1)
                                if (isdigit(arg[3][0]))
                                {
                                    verify_unit = atoi(arg[3]);
                                    verify_port = atoi(strchr(arg[3],'/')+1);
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
                                if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                                {
                                    if (  (ctrl_P->sess_type != CLI_TYPE_PROVISION)
                                        || (verify_ret_e != CLI_API_ETH_TRUNK_MEMBER)
                                       )
                                    {
                                        display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                                        return CLI_NO_ERROR;
                                    }
                                }
                            }
                            else/*port-channel*/
                            {
                                verify_trunk_id = atoi(arg[3]);

                                if( (verify_ret_t = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                                {
                                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                                    return CLI_NO_ERROR;
                                }
                            }

                            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_DeleteStaticRouterPort(vid, lport))
                            {
        #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
        #else
                                CLI_LIB_PrintStr_1("Failed to remove VALN %lu static multicast router port\r\n", (unsigned long)vid);
        #endif
                            }
                            break;

                        case 's': /*static*/
                        case 'S':
                            if(L_INET_Pton(L_INET_AF_INET6, arg[2], group_addr)!=TRUE)
                            {
                                CLI_LIB_PrintStr_1("Failed to set VALN %lu IPv6 address.\r\n", (unsigned long)vid);
                                return CLI_NO_ERROR;
                            }

                            if(arg[3][0] == 'e' || arg[3][0] == 'E') /*ethernet*/
                            {
                                verify_unit = atoi(arg[4]);
                                verify_port = atoi(strchr(arg[4],'/')+1);

        #if (CLI_SUPPORT_PORT_NAME == 1)
                                if (isdigit(arg[4][0]))
                                {
                                    verify_unit = atoi(arg[4]);
                                    verify_port = atoi(strchr(arg[4],'/')+1);
                                }
                                else/*port name*/
                                {
                                    UI32_T trunk_id = 0;
                                    if (!IF_PMGR_IfnameToIfindex(arg[4], &lport))
                                    {
        #if (SYS_CPNT_EH == TRUE)
                                        CLI_API_Show_Exception_Handeler_Msg();
        #else
                                        CLI_LIB_PrintStr_1("%s does not exist\r\n",arg[4]);
        #endif
                                        return CLI_NO_ERROR;
                                    }
                                    SWCTRL_POM_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);
                                }
        #endif
                                if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
                                {
                                    if (  (ctrl_P->sess_type != CLI_TYPE_PROVISION)
                                        || (verify_ret_e != CLI_API_ETH_TRUNK_MEMBER)
                                       )
                                    {
                                        display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                                        return CLI_NO_ERROR;
                                    }
                                }
                            }
                            else/*port-channel*/
                            {
                                verify_trunk_id = atoi(arg[4]);

                                if( (verify_ret_t = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
                                {
                                    display_trunk_msg(verify_ret_t, verify_trunk_id);
                                    return CLI_NO_ERROR;
                                }
                            }

                            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_DeletePortStaticJoinGroup(vid, group_addr, source_addr, lport))
                            {
        #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
        #else
                                CLI_LIB_PrintStr_1("Failed to remove VALN %lu multicast group static port\r\n", (unsigned long)vid);
        #endif
                            }
                            break;

                        case 'i':
                        case 'I':
                            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetImmediateLeaveStatus(vid, MLDSNP_TYPE_IMMEDIATE_DISABLED))
                            {
        #if (SYS_CPNT_EH == TRUE)
                                CLI_API_Show_Exception_Handeler_Msg();
        #else
                                CLI_LIB_PrintStr_1("Failed to enable VALN %lu MLD Snooping Immediate leave\r\n", (unsigned long)vid);
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
    }while(op_ptr != 0 && !isspace(*op_ptr));
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Ipv6_Mld_Snooping_Unknownmulticast_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    UI32_T flood_mode = MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W5_IPV6_MLD_SNOOPING_UNKNOWNMULTICAST_MODE:

            if(arg[0]!=NULL)
            {
                if(arg[0][0]=='f'||arg[0][0]=='F')
                    flood_mode=MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD;
                else if(arg[0][0] == 'd' || arg[0][0] == 'D')
                    flood_mode = MLDSNP_TYPE_UNKNOWN_BEHAVIOR_DROP;
                else
                    flood_mode=MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_IPV6_MLD_SNOOPING_UNKNOWNMULTICAST_MODE:

            flood_mode = SYS_DFLT_MLDSNP_UNKNOWN_MULTICAST_MOD;
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetUnknownFloodBehavior(0, flood_mode))
    {
        CLI_LIB_PrintStr("Failed to configure the mode of processing umknown multicast data packets\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ipv6_Mld_Snooping_Unknownmulticast_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == TRUE)
    char  *op_ptr;
    char   Token[CLI_DEF_MAX_BUFSIZE] = {0};
    char   delemiters[2] = {0};
    UI32_T lower_val = 0;
    UI32_T upper_val = 0;
    UI32_T err_idx;
    UI32_T flood_mode = MLDSNP_TYPE_DFLT_FLOOD_BEHAVIOR;
    UI32_T vlan_id = 0;

    delemiters[0] = ',';

    if(arg[0]!=NULL)
        op_ptr = arg[0];
    else
        return CLI_ERR_INTERNAL;

    do
    {
        memset(Token, 0, CLI_DEF_MAX_BUFSIZE);

        op_ptr = CLI_LIB_Get_Token(op_ptr, Token, delemiters);

        if(!CLI_LIB_Get_Lower_Upper_Value(Token, &lower_val, &upper_val, &err_idx))
            break;

        for(vlan_id=lower_val; vlan_id<=upper_val; vlan_id++)
        {
            switch(cmd_idx)
            {
                case PRIVILEGE_CFG_GLOBAL_CMD_W5_IPV6_MLD_SNOOPING_UNKNOWNMULTICAST_VLAN:

                    if(arg[2]!=NULL)
                    {
                        if(arg[2][0]=='f'||arg[0][0]=='F')
                            flood_mode = MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD;
                        else if(arg[2][0] == 'd' || arg[2][0] == 'D')
                            flood_mode = MLDSNP_TYPE_UNKNOWN_BEHAVIOR_DROP;
                        else
                            flood_mode = MLDSNP_TYPE_UNKNOWN_BEHAVIOR_TO_ROUTER_PORT;
                    }
                    break;

                case PRIVILEGE_CFG_GLOBAL_CMD_W6_NO_IPV6_MLD_SNOOPING_UNKNOWNMULTICAST_VLAN:

                    flood_mode = SYS_DFLT_MLDSNP_UNKNOWN_MULTICAST_MOD;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }

            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetUnknownFloodBehavior(vlan_id, flood_mode))
            {
                CLI_LIB_PrintStr("Failed to configure the mode of processing umknown multicast data packets\r\n");
            }
        }
    }while(op_ptr != 0 && !isspace(*op_ptr));
#endif
    return CLI_NO_ERROR;
}

#if 0 /*move to ipv6 mld snooping vlan 1 immediate-leve*/
UI32_T CLI_API_Ipv6_Mld_Snooping_Immediateleave(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T vid;

    VLAN_OM_ConvertFromIfindex(ctrl_P->CMenu.vlan_ifindex, &vid);

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IPV6_MLD_SNOOPING_IMMEDIATELEAVE:

            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetImmediateLeaveStatus(vid,MLDSNP_TYPE_IMMEDIATE_ENABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to enable MLD Snooping Immediate leave\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IPV6_MLD_SNOOPING_IMMEDIATELEAVE:

            if(MLDSNP_TYPE_RETURN_FAIL== MLDSNP_PMGR_SetImmediateLeaveStatus(vid,MLDSNP_TYPE_IMMEDIATE_DISABLED))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable MLD Snooping Immediate leave\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;
}
#endif

UI32_T CLI_API_Show_Ipv6_Mld_Snooping(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    MLDSNP_TYPE_MLDSNP_STATUS_T mld_status;
    MLDSNP_TYPE_QuerierStatus_T querier_status;
#if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    MLDSNP_TYPE_UnknownBehavior_T flood_mode;
#endif
    UI32_T line_num = 0;
    UI16_T query_interval;
    UI16_T res_time;
    UI16_T robustness;
    UI16_T expiry_time;
    UI16_T mld_version;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI32_T unsolicit_interval;
#endif

    /*configuration*/
    if(MLDSNP_POM_GetMldStatus(&mld_status))
    {
        if (mld_status == MLDSNP_TYPE_MLDSNP_ENABLED)
            CLI_LIB_PrintStr(" Service Status              : Enabled\r\n");
        else
            CLI_LIB_PrintStr(" Service Status              : Disabled\r\n");

        line_num++;
    }
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if(MLDSNP_POM_GetProxyReporting(&proxy_status))
    {
        if (proxy_status == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
            CLI_LIB_PrintStr(" Proxy Reporting             : Enabled\r\n");
        else
            CLI_LIB_PrintStr(" Proxy Reporting             : Disabled\r\n");

        line_num++;
    }
#endif
    if (MLDSNP_POM_GetQuerierStatus(&querier_status))
    {
        if (querier_status == MLDSNP_TYPE_QUERIER_ENABLED)
            CLI_LIB_PrintStr(" Querier Status              : Enabled\r\n");
        else
            CLI_LIB_PrintStr(" Querier Status              : Disabled\r\n");

        line_num++;
    }

    if (MLDSNP_POM_GetRobustnessValue(&robustness))
    {
        CLI_LIB_PrintStr_1(" Robustness                  : %u\r\n", robustness);
        line_num++;
    }

    if (MLDSNP_POM_GetQueryInterval(&query_interval))
    {
        CLI_LIB_PrintStr_1(" Query Interval              : %u sec\r\n", query_interval);
        line_num++;
    }

    if (MLDSNP_POM_GetQueryResponseInterval(&res_time))
    {
        CLI_LIB_PrintStr_1(" Query Max Response Time     : %u sec\r\n", res_time);
        line_num++;
    }

    if (MLDSNP_POM_GetRouterExpireTime(&expiry_time))
    {
        CLI_LIB_PrintStr_1(" Router Port Expiry Time     : %u sec\r\n", expiry_time);
        line_num++;
    }
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if(MLDSNP_POM_GetUnsolicitedReportInterval(&unsolicit_interval))
    {
        CLI_LIB_PrintStr_1(" Unsolicit Report Interval   : %lu sec\r\n", (unsigned long)unsolicit_interval);
        line_num++;
    }
#endif
    /*immediate leave*/
    #if 0
    #else
    {
        MLDSNP_TYPE_ImmediateStatus_T imm_leave_stat;
        MLDSNP_TYPE_ImmediateByHostStatus_T imm_leave_byhost_stat;
        UI16_T vid = 0;
        BOOL_T disable_flag = 1;
        BOOL_T disable_flag_byhost = 1;
        UI32_T vlan_count = 3;

        while(MLDSNP_POM_GetNextImmediateLeaveStatus(&vid, &imm_leave_stat))
        {
            if (imm_leave_stat == MLDSNP_TYPE_IMMEDIATE_ENABLED)
            {
                disable_flag = 0;
                break;
            }
        }

        if (disable_flag == 1)
        {
            CLI_LIB_PrintStr(" Immediate Leave             : Disabled on all VLAN");
        }
        /*use on " show ipv6 mlds snooping vlan"*/
        else
        {
            vid=0;

            CLI_LIB_PrintStr(" Immediate Leave             : Enabled on VLAN ");

            while(MLDSNP_POM_GetNextImmediateLeaveStatus(&vid, &imm_leave_stat))
            {
                if(imm_leave_stat == MLDSNP_TYPE_IMMEDIATE_ENABLED)
                {
                    if(vlan_count==3) /*first term*/
                    {
                        CLI_LIB_PrintStr_1("%4u", vid);
                    }
                    else
                    {
                        CLI_LIB_PrintStr_1(", %4u", vid);
                    }

                    vlan_count++;
                    if (vlan_count % 8 == 0)
                    {
                        PROCESS_MORE("\r\n");
                        CLI_LIB_PrintStr("                             ");
                    }
                }
            }
        }
        PROCESS_MORE("\r\n");

        /*immediate leave by host ip*/
        vlan_count=3;
        vid=0;

        while(MLDSNP_POM_GetNextImmediateLeaveByHostStatus(&vid, &imm_leave_byhost_stat))
        {
            if (imm_leave_byhost_stat == MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED)
            {
                disable_flag_byhost = 0;
                break;
            }
        }

        if (disable_flag_byhost == 1)
        {
            CLI_LIB_PrintStr(" Immediate Leave By Host     : Disabled on all VLAN");
        }
        else
        {
            vid=0;
            CLI_LIB_PrintStr(" Immediate Leave By Host     : Enabled on VLAN ");

            while(MLDSNP_POM_GetNextImmediateLeaveByHostStatus(&vid, &imm_leave_byhost_stat))
            {
                if(imm_leave_byhost_stat == MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED)
                {
                    if(vlan_count==3) /*first term*/
                    {
                        CLI_LIB_PrintStr_1("%4u", vid);
                    }
                    else
                    {
                        CLI_LIB_PrintStr_1(", %4u", vid);
                    }
                    vlan_count++;
                    if (vlan_count % 8 == 0)
                    {
                        PROCESS_MORE("\r\n");
                        CLI_LIB_PrintStr("                             ");
                    }
                }
            }
        }
        PROCESS_MORE("\r\n");
    }
    #endif
    #if(SYS_CPNT_MLDSNP_UNKNOWN_BY_VLAN == FALSE)
    if(MLDSNP_POM_GetUnknownFloodBehavior(0, &flood_mode))
    {
        if(flood_mode==MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD)
        {
            PROCESS_MORE(" Unknown Flood Behavior      : Flood\r\n");
        }
        else
        {
            PROCESS_MORE(" Unknown Flood Behavior      : To Router Port\r\n");
        }
    }
    #endif
    if (MLDSNP_POM_GetMldSnpVer(&mld_version))
    {
        CLI_LIB_PrintStr_1(" MLD Snooping Version        : Version %u", mld_version );
        PROCESS_MORE("\r\n");
    }
    PROCESS_MORE("\r\n");
    { /*static configured group*/
        MLDSNP_TYPE_RecordType_T rec_type;
        UI16_T vid = 0, nxt_id=0, join_port=0;
        char   ipv6_group_addr_str[46]/*, ipv6_src_addr_str[46]*/;
        UI8_T group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
        UI8_T source_addr[MLDSNP_TYPE_IPV6_SRC_IP_LEN]={0};
        //UI8_T null_source_addr[MLDSNP_TYPE_IPV6_SRC_IP_LEN]={0};
        char  UserPort[20] = {0};
        UI32_T unit;
        UI32_T port;
        UI32_T trunk_id;
        SWCTRL_Lport_Type_T ret;

        #if 1
        PROCESS_MORE("VLAN Group IPv6 Address                      Port     \r\n");
        PROCESS_MORE("---- --------------------------------------- ---------\r\n");
        #else
        PROCESS_MORE("VLAN Group/Source IPv6 Address               Port      Type\r\n");
        PROCESS_MORE("---- --------------------------------------- --------- -------\r\n");
        #endif

        while(MLDSNP_POM_GetNextPortStaticGroup(&nxt_id, &vid, group_addr, source_addr, &join_port, &rec_type))
        {
            L_INET_Ntop(L_INET_AF_INET6, (void *)group_addr, ipv6_group_addr_str, sizeof(ipv6_group_addr_str));

            ret = SWCTRL_POM_LogicalPortToUserPort(join_port, &unit, &port, &trunk_id);

            switch(ret)
            {
                case SWCTRL_LPORT_NORMAL_PORT:
                    SYSFUN_Sprintf(UserPort, "Eth %lu/%lu", (unsigned long)unit, (unsigned long)port);
                    break;

                case SWCTRL_LPORT_TRUNK_PORT:
                    SYSFUN_Sprintf(UserPort, "Pch %lu", (unsigned long)trunk_id);
                    break;

                default:
                    continue;
            }

            #if 1
            sprintf(buff, "%4u %39s %-9s\r\n", vid, ipv6_group_addr_str, UserPort);
            PROCESS_MORE(buff);
            #else
            sprintf(buff, "%4u %39s %-9s %-7s\r\n", vid, ipv6_group_addr_str, UserPort, " ");
            PROCESS_MORE(buff);
            if(memcmp(source_addr, null_source_addr, MLDSNP_TYPE_IPV6_SRC_IP_LEN))
            {
              L_INET_Ntop(L_INET_AF_INET6, (void *)source_addr, ipv6_src_addr_str, sizeof(ipv6_src_addr_str));
              sprintf(buff, "%4s %39s %-9s %-7s\r\n", " ", ipv6_src_addr_str, " ", rec_type == MLDSNP_TYPE_IS_INCLUDE_MODE?"Include":"Exclude");
              PROCESS_MORE(buff);
            }
            #endif
        }
    }

    return CLI_NO_ERROR;
}

static BOOL_T cli_api_mldsnp_get_port(char result[], char append[], UI32_T lport)
{
    SWCTRL_Lport_Type_T ret;
    UI32_T unit, port, trunk_id;

    ret = SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id);

    switch(ret)
    {
        case SWCTRL_LPORT_NORMAL_PORT:
        #if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(i,name);
                if (strlen(name) > 8)
                {
                    name[8] = 0;
                }
                sprintf((char *)result, "%8s", name);
            }
        #else
            sprintf((char *)result, "Eth %1lu/%2lu%s", (unsigned long)unit, (unsigned long)port, append);
        #endif
        break;

        case SWCTRL_LPORT_TRUNK_PORT:
            sprintf((char *)result, "Pch %lu%s", (unsigned long)trunk_id, append);
            break;

      default:
            sprintf((char *)result, "can't find");
            return FALSE;
    }

    return TRUE;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Group(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    MLDSNP_OM_RouterPortList_T router_port_list;
    UI32_T line_num = 0, i=0, total=0;
    UI16_T vid=0, old_vid=0;
    UI8_T  group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
    UI8_T  s_group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
    UI8_T  source_addr[MLDSNP_TYPE_IPV6_SRC_IP_LEN]={0};
    UI8_T  ipv6_addr_str[46];
    BOOL_T dis_group = TRUE, specify_group = FALSE;
    char   UserPort[15] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    MLDSNP_OM_GroupInfo_T entry;
    memset(&entry, 0, sizeof(MLDSNP_OM_GroupInfo_T));

    MLDSNP_POM_GetTotalEntry(&total);

    CLI_LIB_PrintStr_2("\r\nTotal Entries %lu, limit %d\r\n", (unsigned long)total, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY);
    CLI_LIB_PrintStr("\r\nVLAN Multicast IPv6 Address                  Member Port     Type\r\n");
    CLI_LIB_PrintStr("---- --------------------------------------- --------------- ---------------\r\n");
    line_num+=5;

    if(arg[0]!=NULL)
    {
        if(L_INET_Pton(L_INET_AF_INET6, arg[0], s_group_addr)!=TRUE)
        {
            CLI_LIB_PrintStr("Failed to get IPv6 address.\r\n");
            return CLI_NO_ERROR;
        }
        specify_group = TRUE;
    }

    while(MLDSNP_POM_GetNextGroupPortlist(&vid, group_addr, source_addr, &entry))
    {
        if(specify_group
           && memcmp(group_addr, s_group_addr, SYS_ADPT_IPV6_ADDR_LEN))
            continue;

        if(old_vid!=vid)
        {
            old_vid = vid;

            if(FALSE == MLDSNP_POM_GetVlanRouterPortlist(vid, &router_port_list))
            {
                memset(&router_port_list, 0, sizeof(MLDSNP_OM_RouterPortList_T));
            }
        }

        dis_group = TRUE;
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry.gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));

        for (i=1; i<=SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.dynamic_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
                        sprintf(buff, "%4u %39s %-15s MLD Snooping%s",entry.vid, ipv6_addr_str, UserPort, "|User\r\n");
                    else
                        sprintf(buff, "%4u %39s %-15s MLD Snooping%s",entry.vid, ipv6_addr_str, UserPort, "\r\n");
                }
                else
                {
                    if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
                        sprintf(buff, "%4s %39s %-15s MLD Snooping%s"," ", " ", UserPort, "|User\r\n");
                    else
                        sprintf(buff, "%4s %39s %-15s MLD Snooping%s"," ", " ", UserPort, "\r\n");

                }
                PROCESS_MORE(buff);
            }
            else if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    sprintf(buff, "%4u %39s %-15s User\r\n",entry.vid, ipv6_addr_str, UserPort);
                }
                else
                    sprintf(buff, "%4s %39s %-15s User\r\n", " ", " ", UserPort);
                PROCESS_MORE(buff);
            }
            else if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.unknown_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    sprintf(buff, "%4u %39s %-15s Multicast Data\r\n",entry.vid, ipv6_addr_str, UserPort);
                }
                else
                    sprintf(buff, "%4s %39s %-15s Multicast Data\r\n", " ", " ", UserPort);
                PROCESS_MORE(buff);
            }
        }
    }

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Group_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    MLDSNP_OM_RouterPortList_T router_port_list;
    UI32_T line_num = 0, i=0;
    UI16_T vid=0, old_vid=0;
    UI8_T group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
    UI8_T source_addr[MLDSNP_TYPE_IPV6_SRC_IP_LEN]={0};
    UI8_T   ipv6_addr_str[46];
    BOOL_T dis_group = TRUE;
    char  UserPort[15] = {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

    MLDSNP_OM_GroupInfo_T entry;
    memset(&entry, 0, sizeof(MLDSNP_OM_GroupInfo_T));

    CLI_LIB_PrintStr("\r\nVLAN Multicast IPv6 Address                  Member Port     Type\r\n");
    CLI_LIB_PrintStr("---- --------------------------------------- --------------- ---------------\r\n");

    if(arg[0]==NULL)
        return CLI_NO_ERROR;

    vid = old_vid= atoi(arg[0]);

    while(MLDSNP_POM_GetNextGroupPortlist(&vid, group_addr, source_addr, &entry))
    {
        if(old_vid!=vid)
          break;

        if(FALSE == MLDSNP_POM_GetVlanRouterPortlist(vid, &router_port_list))
        {
          memset(&router_port_list, 0, sizeof(MLDSNP_OM_RouterPortList_T));
        }
        dis_group = TRUE;
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry.gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));

        for (i=1; i<=SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.dynamic_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
                        sprintf(buff, "%4u %39s %-15s MLD Snooping%s",entry.vid, ipv6_addr_str, UserPort, "|User\r\n");
                    else
                        sprintf(buff, "%4u %39s %-15s MLD Snooping%s",entry.vid, ipv6_addr_str, UserPort, "\r\n");
                }
                else
                {
                    if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
                        sprintf(buff, "%4s %39s %-15s MLD Snooping%s"," ", " ", UserPort, "|User\r\n");
                    else
                        sprintf(buff, "%4s %39s %-15s MLD Snooping%s"," ", " ", UserPort, "\r\n");

                }
                PROCESS_MORE(buff);
            }
            else if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.static_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    sprintf(buff, "%4u %39s %-15s User\r\n",entry.vid, ipv6_addr_str, UserPort);
                }
                else
                    sprintf(buff, "%4s %39s %-15s User\r\n", " ", " ", UserPort);
                PROCESS_MORE(buff);
            }
            else if(MLDSNP_TYPE_IsPortInPortBitMap(i, entry.unknown_port_bitmap))
            {
                if(MLDSNP_TYPE_IsPortInPortBitMap(i, router_port_list.router_port_bitmap))
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(R)", i))
                        continue;
                }
                else
                {
                    if(FALSE == cli_api_mldsnp_get_port(UserPort, "(M)", i))
                        continue;
                }
                if(dis_group)
                {
                    dis_group = FALSE;
                    sprintf(buff, "%4u %39s %-15s Multicast Data\r\n",entry.vid, ipv6_addr_str, UserPort);
                }
                sprintf(buff, "%4s %39s %-15s Multicast Data\r\n", " ", " ", UserPort);
                PROCESS_MORE(buff);
            }
        }
    }

    return CLI_NO_ERROR;
}

#define CLI_API_MLDSNP_HOST_DISPLAY_LINE
static UI32_T cli_api_show_group_host(MLDSNP_OM_PortHostInfo_T *entry_p, UI32_T line_num, BOOL_T new_group)
{
    char   user_port[15] = {0};
    UI8_T  ipv6_addr_str[46]={0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

#ifdef CLI_API_MLDSNP_HOST_DISPLAY_LINE
    if(new_group)
    {
        cli_api_mldsnp_get_port(user_port, " ", entry_p->port);
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-9s %4u %-39s\r\n", user_port, entry_p->vid, ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
        #if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->sip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-9s %4s %-39s\r\n", " ", " ", ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
        #endif
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->host_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-9s %4s %-39s\r\n", " ", " ", ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->host_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-9s %4s %-39s\r\n", " ", " ", ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
    }
#else
    if(new_group)
    {
        cli_api_mldsnp_get_port(user_port, " ", entry_p->port);
        sprintf(buff, "%-15s : %s\r\n", "Member Port", user_port);
        PROCESS_MORE_FUNC(buff);

        sprintf(buff, "%-15s : %d\r\n", "VLAN ID", entry_p->vid);
        PROCESS_MORE_FUNC(buff);

        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-15s : %s\r\n", "Group Address",  ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
        #if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->sip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-15s : %s\r\n", "Source Address",  ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
        #endif
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->host_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-15s : %s\r\n", "Host Address",  ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
    }
    else
    {
        L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->host_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
        sprintf(buff, "%-15s : %s\r\n", " ",  ipv6_addr_str);
        PROCESS_MORE_FUNC(buff);
    }
#endif
    return line_num;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Group_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    SWCTRL_Lport_Type_T ret;
    CLI_API_EthStatus_T   verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    MLDSNP_OM_PortHostInfo_T entry;
    UI32_T line_num = 0;
    UI32_T next_lport = 0;
    UI32_T verify_unit;
    UI32_T verify_port;
    UI32_T verify_trunk_id;
    UI16_T old_vid;
    UI8_T  gip[SYS_ADPT_IPV6_ADDR_LEN];
    UI8_T  sip[SYS_ADPT_IPV6_ADDR_LEN];
    BOOL_T new_group = FALSE;

#ifdef CLI_API_MLDSNP_HOST_DISPLAY_LINE
    CLI_LIB_PrintStr("Port      VLAN Group\r\n");
    #if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
    CLI_LIB_PrintStr("               Source\r\n");
    #endif
    CLI_LIB_PrintStr("               Host\r\n");
    CLI_LIB_PrintStr("--------- ---- ---------------------------------------\r\n");
#endif

    if(arg[0]!=NULL)
    {
        if(arg[0][0] == 'e' || arg[2][0] == 'E') /*ethernet*/
        {
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1],'/')+1);

            if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &next_lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
        else
        {
            verify_trunk_id = atoi(arg[1]);

            if( (verify_ret_t = verify_trunk(verify_trunk_id, &next_lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
        }

        memset(&entry, 0, sizeof(MLDSNP_OM_PortHostInfo_T));
        IPV6_ADDR_SET(gip);
        IPV6_ADDR_SET(sip);
        old_vid =0;
        entry.port = next_lport;

        while(MLDSNP_POM_GetNextPortGroupSourceHost(&entry))
        {
            if(IPV6_ADDR_CMP(gip, entry.gip_a)
                ||IPV6_ADDR_CMP(sip, entry.sip_a)
                || old_vid != entry.vid)
            {
                new_group = TRUE;

                IPV6_ADDR_COPY(gip, entry.gip_a);
                IPV6_ADDR_COPY(sip, entry.sip_a);
                old_vid  = entry.vid;
                #ifndef CLI_API_MLDSNP_HOST_DISPLAY_LINE
                PROCESS_MORE("\r\n");
                #endif
            }
            else
                new_group = FALSE;

            line_num = cli_api_show_group_host(&entry, line_num, new_group);
            if (line_num == EXIT_SESSION_MORE
                ||line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }

         return CLI_NO_ERROR;
    }

    /*loop all port
    */
    while(SWCTRL_LPORT_UNKNOWN_PORT!=(ret = SWCTRL_POM_GetNextLogicalPort(&next_lport)))
    {
        memset(&entry, 0, sizeof(MLDSNP_OM_PortHostInfo_T));
        IPV6_ADDR_SET(gip);
        IPV6_ADDR_SET(sip);
        old_vid =0;
        entry.port = next_lport;

        while(MLDSNP_POM_GetNextPortGroupSourceHost(&entry))
        {
            if(IPV6_ADDR_CMP(gip, entry.gip_a)
                ||IPV6_ADDR_CMP(sip, entry.sip_a)
                || old_vid != entry.vid)
            {
                new_group = TRUE;

                IPV6_ADDR_COPY(gip, entry.gip_a);
                IPV6_ADDR_COPY(sip, entry.sip_a);
                old_vid  = entry.vid;
                #ifndef CLI_API_MLDSNP_HOST_DISPLAY_LINE
                PROCESS_MORE("\r\n");
                #endif
            }
            else
                new_group = FALSE;

            line_num = cli_api_show_group_host(&entry, line_num, new_group);
            if (line_num == EXIT_SESSION_MORE
                ||line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }

    return CLI_NO_ERROR;
}

#define CLI_API_MLDSNP_PORT_DISPLAY_LINE
static UI32_T cli_api_show_port_group(MLDSNP_OM_PortSourceInfo_T *entry_p, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  ipv6_addr_str[46]={0};
    char   user_port[15] = {0};
#ifdef CLI_API_MLDSNP_PORT_DISPLAY_LINE
    char  expire[10]={0}, uptime[10]={0};
#endif

    cli_api_mldsnp_get_port(user_port, " ", entry_p->port);
#ifdef CLI_API_MLDSNP_PORT_DISPLAY_LINE
    L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
    sprintf(expire, "%lu:%lu", (unsigned long)(entry_p->expire/60),(unsigned long)(entry_p->expire%60));
    sprintf(uptime, "%lu:%lu:%lu", (unsigned long)(entry_p->up_time/3600),(unsigned long)(entry_p->up_time/60)%60,(unsigned long)(entry_p->up_time%60));
    sprintf(buff, "%-9s %4u %-39s %-6s %-10s %-1s %2lu\r\n", user_port, entry_p->vid, ipv6_addr_str, expire, uptime, (MLDSNP_TYPE_JOIN_DYNAMIC == entry_p->join_type?"D":"S"), (unsigned long)entry_p->unreply_q_count);
    PROCESS_MORE_FUNC(buff);

    #if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
    L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->sip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
    sprintf(buff, "%-9s %-4s %-39s\r\n", " ", " ", ipv6_addr_str);
    PROCESS_MORE_FUNC(buff);
    #endif
#else
    sprintf(buff, "%-15s : %s\r\n", "Member Port", user_port);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-15s : %d\r\n", "VLAN ID", entry_p->vid);
    PROCESS_MORE_FUNC(buff);

    L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
    sprintf(buff, "%-15s : %s\r\n", "Group Address",  ipv6_addr_str);
    PROCESS_MORE_FUNC(buff);
#if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
    L_INET_Ntop(L_INET_AF_INET6, (void *) entry_p->sip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
    sprintf(buff, "%-15s : %s\r\n", "Source Address",  ipv6_addr_str);
    PROCESS_MORE_FUNC(buff);
#endif
    sprintf(buff, "%-15s : %s\r\n", "MLD Snooping", (MLDSNP_TYPE_JOIN_DYNAMIC == entry_p->join_type?"Dynamic":"Static"));
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, "%-15s : %lu(m):%lu(s) \r\n","Expire time", (unsigned long)(entry_p->expire/60),
                                                           (unsigned long)(entry_p->expire%60));
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, "%-15s : %lu(h):%lu(m):%lu(s) \r\n","Up time", (unsigned long)(entry_p->up_time/3600),
                                                       (unsigned long)(entry_p->up_time/60)%60,
                                                       (unsigned long)(entry_p->up_time%60));
    PROCESS_MORE_FUNC(buff);

    PROCESS_MORE_FUNC("\r\n");
#endif
    return line_num;
}
UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Port_Group(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    SWCTRL_Lport_Type_T ret;
    CLI_API_EthStatus_T   verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;
    MLDSNP_OM_PortSourceInfo_T entry;
    UI32_T line_num = 0;
    UI32_T next_lport = 0 ;
    UI32_T verify_unit;
    UI32_T verify_port;
    UI32_T verify_trunk_id;

#ifdef CLI_API_MLDSNP_PORT_DISPLAY_LINE
    CLI_LIB_PrintStr("\r\nExpire: H:M:S; Uptime: H:M:S; T: Dynamic/Stattic; Q: Unreply query\r\n");
    CLI_LIB_PrintStr("Port      VLAN Group                                   Expire Uptime     T Q\r\n");
    #if(SYS_CPNT_MLDSNP_V2_ASM == FALSE)
    CLI_LIB_PrintStr("               Source\r\n");
    line_num ++;
    #endif
    CLI_LIB_PrintStr("--------- ---- --------------------------------------- ------ ---------- - --\r\n");
    line_num +=4;
#endif

    if(arg[0]!=NULL)
    {
        if(arg[0][0] == 'e' || arg[2][0] == 'E') /*ethernet*/
        {
            verify_unit = atoi(arg[1]);
            verify_port = atoi(strchr(arg[1],'/')+1);

            if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &next_lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
        }
        else
        {
            verify_trunk_id = atoi(arg[1]);

            if( (verify_ret_t = verify_trunk(verify_trunk_id, &next_lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
        }

        memset(&entry, 0, sizeof(MLDSNP_OM_PortSourceInfo_T));
        entry.port = next_lport;

        while(MLDSNP_POM_GetNextPortGroupSource(&entry))
        {
            if(MLDSNP_TYPE_JOIN_UNKNOWN == entry.join_type)
                continue;

            line_num = cli_api_show_port_group(&entry, line_num);
            if (line_num == EXIT_SESSION_MORE
                ||line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
        return CLI_NO_ERROR;
    }

    while(SWCTRL_LPORT_UNKNOWN_PORT!=(ret = SWCTRL_POM_GetNextLogicalPort(&next_lport)))
    {
        memset(&entry, 0, sizeof(MLDSNP_OM_PortSourceInfo_T));
        entry.port=next_lport;

        while(MLDSNP_POM_GetNextPortGroupSource(&entry))
        {
            if(MLDSNP_TYPE_JOIN_UNKNOWN == entry.join_type)
                continue;

            line_num = cli_api_show_port_group(&entry, line_num);
            if (line_num == EXIT_SESSION_MORE
                ||line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Group_SourceList(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T next_port=0;
    UI32_T unit, port, trunk_id;
    UI8_T   ipv6_addr_str[46]={0};
    UI16_T i=0;
    BOOL_T specify_group = FALSE;
    UI8_T s_group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
    SWCTRL_Lport_Type_T ret;

    if(arg[0]!=NULL)
    {
        if(L_INET_Pton(L_INET_AF_INET6, arg[0], s_group_addr)!=TRUE)
        {
            CLI_LIB_PrintStr("Failed to get IPv6 address.\r\n");
            return CLI_NO_ERROR;
        }
        specify_group = TRUE;
    }

    MLDSNP_OM_PortSourceListInfo_T entry;
    while(SWCTRL_LPORT_UNKNOWN_PORT!=(ret = SWCTRL_POM_GetNextLogicalPort(&next_port)))
    {
        memset(&entry, 0, sizeof(MLDSNP_OM_PortSourceListInfo_T));
        entry.port=next_port;
        while(MLDSNP_POM_GetNextPortGroupSourceList(&entry))
        {
            if(specify_group
               && memcmp(entry.gip_a, s_group_addr, SYS_ADPT_IPV6_ADDR_LEN))
              continue;

            PROCESS_MORE("\r\n");
            sprintf(buff, "%-23s : %d\r\n", "VLAN ID", entry.vid);
            PROCESS_MORE(buff);

            L_INET_Ntop(L_INET_AF_INET6, (void *) entry.gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
            sprintf(buff, "%-23s : %s\r\n", "Mutlicast IPv6 Address",  ipv6_addr_str);
            PROCESS_MORE(buff);

            sprintf(buff, "%-23s :", "Member Port");
            CLI_LIB_PrintStr(buff);

            SWCTRL_POM_LogicalPortToUserPort(next_port, &unit, &port, &trunk_id);

            switch(ret)
            {
                case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(i,name);
                        if (strlen(name) > 8)
                        {
                            name[8] = 0;
                        }
                        sprintf(buff, " %8s", name);
                    }
#else
                    sprintf(buff, " Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                    break;

                case SWCTRL_LPORT_TRUNK_PORT:
                    sprintf(buff, " Pch %lu", (unsigned long)trunk_id);
                    break;

                default:
                    sprintf(buff, "Can't find this port");
            }
            CLI_LIB_PrintStr(buff);
            PROCESS_MORE("\r\n");

            sprintf(buff, "%-23s : %s\r\n", "MLD Snooping", (MLDSNP_TYPE_JOIN_DYNAMIC == entry.join_type?"Dynamic":(MLDSNP_TYPE_JOIN_STATIC== entry.join_type?"Static":"Multicast Data")));
            PROCESS_MORE(buff);

            sprintf(buff, "%-23s : %s\r\n","Filter Mode",  (entry.cur_mode == MLDSNP_TYPE_IS_INCLUDE_MODE? "Include":"Exclude"));
            PROCESS_MORE(buff);

            if(entry.cur_mode == MLDSNP_TYPE_IS_EXCLUDE_MODE)
            {
                sprintf(buff, "%-23s : %lu sec.\r\n","Filter Timer Elapse", (unsigned long)entry.filter_time_elapse);
                PROCESS_MORE(buff);

                sprintf(buff, "%-23s : ", "Request List");
                CLI_LIB_PrintStr(buff);

                if(0 == entry.num_of_req)
                {
                    PROCESS_MORE("\r\n");
                }
                else
                {
                    for(i=0;i<entry.num_of_req; i++)
                    {
                        if(i>0)
                        {
                            sprintf(buff, "%26s", " ");
                            CLI_LIB_PrintStr(buff);
                        }

                        L_INET_Ntop(L_INET_AF_INET6, (void *) entry.request_list[i],  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                        sprintf(buff, "%s\r\n", ipv6_addr_str);
                        PROCESS_MORE(buff);
                    }
                }
                sprintf(buff, "%-23s : ", "Exclude List");
                CLI_LIB_PrintStr(buff);

                for(i=0;i<entry.num_of_ex; i++)
                {
                    if(i>0)
                    {
                        sprintf(buff, "%26s", " ");
                        CLI_LIB_PrintStr(buff);
                    }

                    L_INET_Ntop(L_INET_AF_INET6, (void *) entry.exclude_list[i],  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                    sprintf(buff, "%s\r\n", ipv6_addr_str);
                    PROCESS_MORE(buff);
                }
            }
            else
            {
                sprintf(buff, "%-23s : ", "Include List");
                CLI_LIB_PrintStr(buff);

                for(i=0;i<entry.num_of_req; i++)
                {
                    if(i>0)
                    {
                        sprintf(buff, "%26s", " ");
                        CLI_LIB_PrintStr(buff);
                    }

                    L_INET_Ntop(L_INET_AF_INET6, (void *) entry.request_list[i], (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                    sprintf(buff, "%s\r\n", ipv6_addr_str);
                    PROCESS_MORE(buff);
                }
            }
            PROCESS_MORE("\r\n");
        }
    }
    PROCESS_MORE("\r\n");
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Group_SourceList_Vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T next_port=0, specify_vlan=0;
    UI32_T unit, port, trunk_id;
    UI8_T   ipv6_addr_str[46]={0};
    UI16_T i=0;
    SWCTRL_Lport_Type_T ret;

    if(arg[0]== NULL)
        return CLI_NO_ERROR;

    specify_vlan = atoi(arg[0]);

    MLDSNP_OM_PortSourceListInfo_T entry;
    while(SWCTRL_LPORT_UNKNOWN_PORT!=(ret = SWCTRL_POM_GetNextLogicalPort(&next_port)))
    {
        memset(&entry, 0, sizeof(MLDSNP_OM_PortSourceListInfo_T));
        entry.port=next_port;
        while(MLDSNP_POM_GetNextPortGroupSourceList(&entry))
        {
            if(specify_vlan!=0
               && specify_vlan != entry.vid)
              continue;

            PROCESS_MORE("\r\n");
            sprintf(buff, "%-23s : %d\r\n", "VLAN ID", entry.vid);
            PROCESS_MORE(buff);

            L_INET_Ntop(L_INET_AF_INET6, (void *) entry.gip_a,  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
            sprintf(buff, "%-23s : %s\r\n", "Mutlicast IPv6 Address",  ipv6_addr_str);
            PROCESS_MORE(buff);

            sprintf(buff, "%-23s :", "Member Port");
            CLI_LIB_PrintStr(buff);

            SWCTRL_POM_LogicalPortToUserPort(next_port, &unit, &port, &trunk_id);

            switch(ret)
            {
                case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(i,name);
                        if (strlen(name) > 8)
                        {
                            name[8] = 0;
                        }
                        sprintf(buff, " %8s", name);
                    }
#else
                    sprintf(buff, " Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                    break;

                case SWCTRL_LPORT_TRUNK_PORT:
                    sprintf(buff, " Pch %lu", (unsigned long)trunk_id);
                    break;

                default:
                    sprintf(buff, "Can't find this port");
            }
            CLI_LIB_PrintStr(buff);
            PROCESS_MORE("\r\n");

            sprintf(buff, "%-23s : %s\r\n", "MLD Snooping", (MLDSNP_TYPE_JOIN_DYNAMIC == entry.join_type?"Dynamic":(MLDSNP_TYPE_JOIN_STATIC== entry.join_type?"Static":"Multicast Data")));
            PROCESS_MORE(buff);

            sprintf(buff, "%-23s : %s\r\n","Filter Mode",  (entry.cur_mode == MLDSNP_TYPE_IS_INCLUDE_MODE? "Include":"Exclude"));
            PROCESS_MORE(buff);

            if(entry.cur_mode == MLDSNP_TYPE_IS_EXCLUDE_MODE)
            {
                sprintf(buff, "%-23s : %lu sec.\r\n","Filter Timer Elapse", (unsigned long)entry.filter_time_elapse);
                PROCESS_MORE(buff);

                sprintf(buff, "%-23s : ", "Request List");
                CLI_LIB_PrintStr(buff);

                if(0 == entry.num_of_req)
                {
                    PROCESS_MORE("\r\n");
                }
                else
                {
                    for(i=0;i<entry.num_of_req; i++)
                    {
                        if(i>0)
                        {
                            sprintf(buff, "%26s", " ");
                            CLI_LIB_PrintStr(buff);
                        }

                        L_INET_Ntop(L_INET_AF_INET6, (void *) entry.request_list[i],  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                        sprintf(buff, "%s\r\n", ipv6_addr_str);
                        PROCESS_MORE(buff);
                    }
                }
                sprintf(buff, "%-23s : ", "Exclude List");
                CLI_LIB_PrintStr(buff);

                for(i=0;i<entry.num_of_ex; i++)
                {
                    if(i>0)
                    {
                        sprintf(buff, "%26s", " ");
                        CLI_LIB_PrintStr(buff);
                    }

                    L_INET_Ntop(L_INET_AF_INET6, (void *) entry.exclude_list[i],  (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                    sprintf(buff, "%s\r\n", ipv6_addr_str);
                    PROCESS_MORE(buff);
                }
            }
            else
            {
                sprintf(buff, "%-23s : ", "Include List");
                CLI_LIB_PrintStr(buff);

                for(i=0;i<entry.num_of_req; i++)
                {
                    if(i>0)
                    {
                        sprintf(buff, "%26s", " ");
                        CLI_LIB_PrintStr(buff);
                    }

                    L_INET_Ntop(L_INET_AF_INET6, (void *) entry.request_list[i], (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
                    sprintf(buff, "%s\r\n", ipv6_addr_str);
                    PROCESS_MORE(buff);
                }
            }
            PROCESS_MORE("\r\n");
        }
    }
    PROCESS_MORE("\r\n");
    return CLI_NO_ERROR;
}

static UI32_T show_mld_mcast_router_port(UI16_T vlan_id, UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE]= {0};
    UI8_T  mld_status [8] = {0};
    char  UserPort[10] = {0};
    UI32_T  next_port=0;
    UI32_T unit, port, trunk_id, expire;
    MLDSNP_OM_RouterPortList_T router_entry;
    SWCTRL_Lport_Type_T ret;

    memset(&router_entry, 0, sizeof(MLDSNP_OM_RouterPortList_T));

    MLDSNP_POM_GetVlanRouterPortlist(vlan_id, &router_entry);

    while(SWCTRL_LPORT_UNKNOWN_PORT!=(ret = SWCTRL_POM_GetNextLogicalPort(&next_port)))
     {
        SWCTRL_POM_LogicalPortToUserPort(next_port, &unit, &port, &trunk_id);

        switch(ret)
        {
            case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
            {
                UI8_T name[MAXSIZE_ifName+1] = {0};
                CLI_LIB_Ifindex_To_Name(j,name);
                if (strlen(name) > 8)
                {
                    name[8] = 0;
                }
                sprintf(UserPort, "%8s", name);
            }
#else
                sprintf(UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                break;

            case SWCTRL_LPORT_TRUNK_PORT:
                sprintf(UserPort, "Pch %lu", (unsigned long)trunk_id);
                break;

            default:
                continue;
        }

        if( router_entry.router_port_bitmap[(UI32_T)((next_port-1)/8)]  & (1 << (7 - ((next_port-1) % 8))) )
        {
            if (router_entry.static_router_port_bitmap[(UI32_T)((next_port-1)/8)]  &  (1 << (7 - ((next_port-1) % 8))) )
            {
                strcpy ((char *)mld_status,"Static");
                sprintf((char *)buff, " %4u %-21s %-9s\r\n", vlan_id, UserPort, mld_status);
            }
            else
            {
                strcpy ((char *)mld_status,"Dynamic");
                if(TRUE == MLDSNP_POM_GetRouterPortExpireInterval(vlan_id, next_port, &expire))
                {
                  sprintf((char *)buff, " %4u %-21s %-9s %lu:%lu\r\n", vlan_id, UserPort, mld_status, (unsigned long)expire/60, (unsigned long)expire%60);
                }
                else
                {
                  sprintf((char *)buff, " %4u %-21s %-9s\r\n", vlan_id, UserPort, mld_status);
                }
            }

            PROCESS_MORE_FUNC(buff);
        }
    }

    return line_num;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Mrouter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num = 0;
    UI32_T time_mark = 0;
    UI32_T vid;

    if(arg[0]) /*specify the vlan id*/
    {
        vid = atoi(arg[1]);

        if (VLAN_POM_IsVlanExisted(vid))
        {
            CLI_LIB_PrintStr(" VLAN Multicast Router Port Type      Expire\r\n");
            CLI_LIB_PrintStr(" ---- --------------------- --------- ------\r\n");

            line_num = 2;

            show_mld_mcast_router_port(atoi(arg[1]), line_num);
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
    else
    {
        CLI_LIB_PrintStr(" VLAN Multicast Router Port Type      Expire\r\n");
        CLI_LIB_PrintStr(" ---- --------------------- --------- ------\r\n");

        line_num = 2;

        vid = 0;
        while(VLAN_POM_GetNextVlanId(time_mark, &vid))
        {
            line_num = show_mld_mcast_router_port(vid, line_num);

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

    return CLI_NO_ERROR;
}

static UI32_T show_mld_mcast_group_member_port(UI32_T vlan_id, UI32_T port_type, UI32_T line_num)
{
#if(SYS_CPNT_MLDSNP==TRUE)
   #define SHOW_PORT_TYPE_ALL       0
   #define SHOW_PORT_TYPE_MLDSNP    1
   #define SHOW_PORT_TYPE_USER      2

   UI32_T j = 0;
   UI16_T vid;
   UI8_T  UserPort[10] = {0};
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T group_addr[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0};
   UI8_T source_addr[MLDSNP_TYPE_IPV6_SRC_IP_LEN]={0};
   UI8_T  mac_addr[18]  = {0};
   MLDSNP_OM_GroupInfo_T group_entry;

   memset(&group_entry, 0, sizeof(MLDSNP_OM_GroupInfo_T));
   vid = vlan_id;

   while(MLDSNP_POM_GetNextGroupPortlist(&vid, group_addr,  source_addr, &group_entry))
   {
        if(vid==vlan_id)
        {
            SYSFUN_Sprintf((char*)mac_addr, "33-33-%02X-%02X-%02X-%02X", group_addr[12], group_addr[13],group_addr[14],group_addr[15]);

            for(j=1; j<=SYS_ADPT_TOTAL_NBR_OF_LPORT; j++)
            {
                UI32_T unit;
                UI32_T port;
                UI32_T trunk_id;
                SWCTRL_Lport_Type_T ret;

                ret = SWCTRL_POM_LogicalPortToUserPort(j, &unit, &port, &trunk_id);
                switch(ret)
                {
                    case SWCTRL_LPORT_NORMAL_PORT:
#if (CLI_SUPPORT_PORT_NAME == 1)
                    {
                        UI8_T name[MAXSIZE_ifName+1] = {0};
                        CLI_LIB_Ifindex_To_Name(j,name);
                        if (strlen((char*)name) > CLI_DEF_MAX_BUFSIZE/2)/*prevent name too long for strcat*/
                        {                                        /*pttch 2002.07.10*/
                            name[(CLI_DEF_MAX_BUFSIZE/2)-1] = 0;
                        }
                            SYSFUN_Sprintf((char*)UserPort, "  %8s", name);
                    }
#else
                        SYSFUN_Sprintf((char*)UserPort, "Eth %1lu/%2lu", (unsigned long)unit, (unsigned long)port);
#endif
                        break;

                    case SWCTRL_LPORT_TRUNK_PORT:
                        SYSFUN_Sprintf((char*)UserPort, "Pch %lu", (unsigned long)trunk_id);
                        break;

                    default:
                        continue;
                }

                if( group_entry.dynamic_port_bitmap[(UI32_T)((j-1)/8)]  & (1 << (7 - ((j-1) % 8))) )
                {
                    /*Mld Snoop or all*/
                    if(port_type==SHOW_PORT_TYPE_ALL || port_type==SHOW_PORT_TYPE_MLDSNP)
                    {
                        SYSFUN_Sprintf((char*)buff, "%4u %s %-8s Mld Snooping\n\r", vid, mac_addr, UserPort);
                        PROCESS_MORE_FUNC(buff);
                    }
                }

                /*User or all*/
                if( group_entry.static_port_bitmap[(UI32_T)((j-1)/8)]  & (1 << (7 - ((j-1) % 8))) )
                {
                    if(port_type==SHOW_PORT_TYPE_ALL || port_type==SHOW_PORT_TYPE_USER)
                    {
                        SYSFUN_Sprintf((char*)buff, "%4u %s %-8s User\n\r", vid, mac_addr, UserPort);
                        PROCESS_MORE_FUNC(buff);
                    }
                }
                /*data*/
                if( group_entry.unknown_port_bitmap[(UI32_T)((j-1)/8)]  & (1 << (7 - ((j-1) % 8))) )
                {
                    if(port_type==SHOW_PORT_TYPE_ALL || port_type==SHOW_PORT_TYPE_USER)
                    {
                        SYSFUN_Sprintf((char*)buff, "%4u %s %-8s Multicast Data\n\r", vid, mac_addr, UserPort);
                        PROCESS_MORE_FUNC(buff);
                    }
                }
            }
        }
        else
            break;
    }

  #endif
    return line_num;
}

UI32_T CLI_API_Show_MacAddressTable_Ipv6_Multicast(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_MLDSNP==TRUE)
    #define SHOW_ALL_VLAN            0xffffffff
    #define SHOW_PORT_TYPE_ALL       0
    #define SHOW_PORT_TYPE_MLDSNP   1
    #define SHOW_PORT_TYPE_USER      2

    UI32_T line_num = 0;
    UI32_T specified_vid = SHOW_ALL_VLAN;
    UI32_T specified_type = SHOW_PORT_TYPE_USER+1;
    UI32_T vid;
    UI32_T time_mark = 0;

    if(arg[0])
    {
        switch(arg[0][0])
        {
        case 'v':
        case 'V':
            specified_vid = atoi((char*)arg[1]);
            if(arg[2])
            {
                if(arg[2][0] == 'm' || arg[2][0] == 'M')
                {
                    specified_type = SHOW_PORT_TYPE_MLDSNP;
                }
                else
                {
                    specified_type = SHOW_PORT_TYPE_USER;
                }
            }
            else
            {
                specified_type = SHOW_PORT_TYPE_ALL;
            }
            break;

        case 'm':
        case 'M':
            specified_vid  = SHOW_ALL_VLAN;
            specified_type = SHOW_PORT_TYPE_MLDSNP;
            break;

        case 'u':
        case 'U':
            specified_vid  = SHOW_ALL_VLAN;
            specified_type = SHOW_PORT_TYPE_USER;
            break;
        }
    }
    else
    {
        specified_vid  = SHOW_ALL_VLAN;
        specified_type = SHOW_PORT_TYPE_ALL;
    }

    /*  m'cast group member port */
    CLI_LIB_PrintStr(" VLAN M'cast addr     Member ports Type\r\n");
    CLI_LIB_PrintStr(" ---- --------------- ------------ -------\r\n");
    line_num += 2;
    vid       = 0;
    time_mark = 0;

    /*print some VLAN(s)*/
    if(specified_vid == SHOW_ALL_VLAN)
    {
        while(VLAN_POM_GetNextVlanId(time_mark, &vid))
        {
            if( (line_num = show_mld_mcast_group_member_port(vid, specified_type, line_num)) == JUMP_OUT_MORE)
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
        if(!VLAN_POM_IsVlanExisted(specified_vid))
        {
#if (SYS_CPNT_EH == TRUE)
            CLI_API_Show_Exception_Handeler_Msg();
#else
            CLI_LIB_PrintStr("No such VLAN.\r\n");
#endif
        }
        else
        {
            show_mld_mcast_group_member_port(specified_vid, specified_type, line_num);
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Ipv6_Mld_Snooping_Filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IPV6_MLD_FILTER:
            /*printf("ipv6 mld snooping router-alert-option-check\r\n");*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldFilter(VAL_mldSnoopFilterStatus_enabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set MLD filter\r\n");
#endif
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IPV6_MLD_FILTER:
            /*printf("no ipv6 mld snooping router-alert-option-check\r\n");*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldFilter(VAL_mldSnoopFilterStatus_disabled))
            {
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to set MLD filter\r\n");
#endif
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Ipv6_Mld_Snooping_Profile(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T profile_id;
    /*copy pid to ctrl_p->menu.profile_id*/
    profile_id = CLI_LIB_AtoUl(arg[0],10);
    ctrl_P->CMenu.profile_id = profile_id;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_IPV6_MLD_PROFILE:/*creat a profile*/
             if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_CreateMLDProfileEntry(profile_id))
             {
                CLI_LIB_PrintStr("Failed to create the profile or the profile exists\r\n");
             }
             else
             {
                /*enter profile mode*/
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_MLD_PROFILE_MODE;
             }
             break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IPV6_MLD_PROFILE:/*delete a profile*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_DestroyMLDProfileEntry(profile_id))
            {
                CLI_LIB_PrintStr("Failed to destory the profile\r\n");
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Ipv6_Mld_Snooping_ProfileAction(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T pid;

    pid = ctrl_P->CMenu.profile_id;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_MLD_PROFILE_CMD_W1_DENY:/*deny*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDProfileAccessMode(pid, VAL_mldSnoopProfileAction_deny))
            {
                CLI_LIB_PrintStr("Failed to set MLD profile AccessMode\r\n");
            }
            break;
        case PRIVILEGE_CFG_MLD_PROFILE_CMD_W1_PERMIT:/*permit*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDProfileAccessMode(pid, VAL_mldSnoopProfileAction_permit))
            {
                CLI_LIB_PrintStr("Failed to set MLD profile AccessMode\r\n");
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Ipv6_Mld_Snooping_ProfileMipRange(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T pid;
    UI8_T mip_begin[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI8_T mip_end[SYS_ADPT_IPV6_ADDR_LEN] ={0};

    pid = ctrl_P->CMenu.profile_id;

    if(L_INET_Pton(L_INET_AF_INET6, arg[0], mip_begin)!=TRUE)
    {
        CLI_LIB_PrintStr("Failed to set start IPv6 address.\r\n");
        return CLI_NO_ERROR;
    }

    if(NULL==arg[1])
    {
        if(L_INET_Pton(L_INET_AF_INET6, arg[0], mip_end)!=TRUE)
        {
            CLI_LIB_PrintStr("Failed to set end IPv6 address.\r\n");
            return CLI_NO_ERROR;
        }
    }
    else
    {
        if(L_INET_Pton(L_INET_AF_INET6, arg[1], mip_end)!=TRUE)
        {
            CLI_LIB_PrintStr("Failed to set start IPv6 address.\r\n");
            return CLI_NO_ERROR;
        }
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_MLD_PROFILE_CMD_W1_RANGE:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_AddMLDProfileGroup(pid, mip_begin, mip_end))
            {
                CLI_LIB_PrintStr("Failed to add the range to this profile\r\n");
            }
            break;
        case PRIVILEGE_CFG_MLD_PROFILE_CMD_W2_NO_RANGE:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_DeleteMLDProfileGroup(pid, mip_begin, mip_end))
            {
                CLI_LIB_PrintStr("Failed to delete the range to this profile\r\n");
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Ipv6_Mld_Snooping_FilterIntf_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T i;
    UI32_T verify_port;
    UI32_T verify_unit;
    CLI_API_EthStatus_T verify_ret;
    UI32_T ifindex;
    UI32_T pid;

    verify_unit=ctrl_P->CMenu.unit_id;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_IPV6_MLD_FILTER:/*apply a profile to a eth*/

            pid=CLI_LIB_AtoUl(arg[0],10);

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


                    if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_AddMLDProfileToPort(ifindex, pid))
                    {
                        CLI_LIB_PrintStr_3("Failed to apply profile %lu to Ethernet %lu/%lu\r\n ",pid,(unsigned long)verify_unit,(unsigned long)verify_port);
                    }
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_IPV6_MLD_FILTER:/*disapply a profile to a eth*/
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
                    if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_RemoveMLDProfileFromPort(ifindex))
                    {
                        CLI_LIB_PrintStr_2("Failed to remove profile from Ethernet %lu/%lu\r\n ",(unsigned long)verify_unit,(unsigned long)verify_port);
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

UI32_T
CLI_API_Ipv6_Mld_Snooping_FilterIntf_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T ifindex;
    UI32_T pid;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &ifindex))!= CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_IPV6_MLD_FILTER:

            pid=CLI_LIB_AtoUl(arg[0],10);

            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_AddMLDProfileToPort(ifindex, pid))
            {
                CLI_LIB_PrintStr_2("Failed to apply profile %lu to Port channel %lu\r\n ",(unsigned long)pid, (unsigned long)ctrl_P->CMenu.pchannel_id);
            }

            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_IPV6_MLD_FILTER:

            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_RemoveMLDProfileFromPort(ifindex))
            {
                CLI_LIB_PrintStr_1("Failed to remove profile from Port channel %lu\r\n ", (unsigned long)ctrl_P->CMenu.pchannel_id);
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T
CLI_API_Ipv6_Mld_Snooping_MaxGroupsAction_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T throttling_action;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_IPV6_MLD_MAXGROUPS_ACTION:/*configure max groups of ethernet port*/
            if(arg[0][0]=='d'||arg[0][0]=='D')
              throttling_action=VAL_mldSnoopThrottlePortAction_deny;
            else
              throttling_action=VAL_mldSnoopThrottlePortAction_replace;
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
                        /*set the throttling of this port*/
                        if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingActionToPort(ifindex, throttling_action))
                        {
                            CLI_LIB_PrintStr_2("Failed to set Ethernet %lu/%lu throttling action\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                            continue;
                        }
                    }
                }
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W5_NO_IPV6_MLD_MAXGROUPS_ACTION: /* no max groups of ethernet port */

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
                        if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingActionToPort(ifindex, SYS_DFLT_MLD_THROTTLE_ACTION))
                        {
                            CLI_LIB_PrintStr_2("Failed to set Ethernet %lu/%lu throttling action\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                            continue;
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

UI32_T
CLI_API_Ipv6_Mld_Snooping_MaxGroupsAction_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)

    UI32_T ifindex         = 0;
    UI32_T throttling_action;
    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &ifindex))!= CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_IPV6_MLD_MAXGROUPS_ACTION: /*  max groups of ethernet port */
        if(arg[0][0]=='d'||arg[0][0]=='D')
            throttling_action=VAL_mldSnoopThrottlePortAction_deny;
        else
            throttling_action=VAL_mldSnoopThrottlePortAction_replace;

            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingActionToPort(ifindex, throttling_action))
            {
                CLI_LIB_PrintStr_1("Failed to set Port channel %lu throttling action\r\n",(unsigned long)ctrl_P->CMenu.pchannel_id);
            }
        break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W5_NO_IPV6_MLD_MAXGROUPS_ACTION: /* no max groups of ethernet port */

            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingActionToPort(ifindex, SYS_DFLT_MLD_THROTTLE_ACTION))
            {
                CLI_LIB_PrintStr_1("Failed to set Port channel %lu throttling action\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
            }
            break;
    default:
        return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T
CLI_API_Ipv6_Mld_Snooping_MaxGroups_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T i               = 0;
    UI32_T ifindex         = 0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port;
    CLI_API_EthStatus_T verify_ret;
    UI32_T throttling_num;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_IPV6_MLD_MAXGROUPS:/*configure max groups of ethernet port*/

        for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
        {
            if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)]  & (1 << ( 7 - ((i-1)%8))) )
            {
               verify_port = i;

               throttling_num=atoi(arg[0]);
               if( (verify_ret = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
               {
                   display_ethernet_msg(verify_ret, verify_unit, verify_port);
                   continue;
               }
               else
               {
                   /*set the throttling of this port*/
                   if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingNumberToPort(ifindex, throttling_num))
                   {
                       CLI_LIB_PrintStr_2("Failed to set Ethernet %lu/%lu max-groups value\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                       continue;
                   }
               }
            }
        }
    break;

    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_IPV6_MLD_MAXGROUPS: /* no max groups of ethernet port */
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
                    if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingNumberToPort(ifindex, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY))
                    {
                        CLI_LIB_PrintStr_2("Failed to set Ethernet %lu/%lu max-groups value\r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
                        continue;
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

UI32_T
CLI_API_Ipv6_Mld_Snooping_MaxGroups_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T ifindex         = 0;
    UI32_T throttling_num;

    CLI_API_TrunkStatus_T verify_ret;

    if( (verify_ret = verify_trunk(ctrl_P->CMenu.pchannel_id, &ifindex))!= CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, ctrl_P->CMenu.pchannel_id);
        return CLI_NO_ERROR;
    }

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_IPV6_MLD_MAXGROUPS:

         throttling_num=atoi(arg[0]);

         if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingNumberToPort(ifindex, throttling_num))
         {
            CLI_LIB_PrintStr_1("Failed to set Port channel %lu max-groups value\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
         }
     break;

    case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_IPV6_MLD_MAXGROUPS:
        if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMLDThrottlingNumberToPort(ifindex, SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY))
        {
            CLI_LIB_PrintStr_1("Failed to set Port channel %lu max-groups value\r\n", (unsigned long)ctrl_P->CMenu.pchannel_id);
        }
    break;

    default:
        return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;

}

UI32_T
CLI_API_Show_Ipv6_Mld_Snooping_filter(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T filter_status;
    UI32_T lport;
    UI32_T pid;
    UI32_T trunk_id;
    UI32_T unit,port;
    UI32_T access_mode;
    UI8_T  start_ip[SYS_ADPT_IPV6_ADDR_LEN]={0};
    UI8_T  end_ip[SYS_ADPT_IPV6_ADDR_LEN]={0};
    char  sip[48]={0};
    char  eip[48]={0};
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;
    CLI_API_EthStatus_T   verify_ret_e;
    CLI_API_TrunkStatus_T verify_ret_t;

    if(arg[0]==NULL)/*display filter status*/
    {
        MLDSNP_POM_GetMLDFilterStatus(&filter_status);
        if(filter_status==VAL_mldSnoopFilterStatus_enabled)
        {
            PROCESS_MORE(" MLD Filter Enabled\r\n");
        }
        else
        {
            PROCESS_MORE(" MLD Filter Disabled\r\n");
        }
        return CLI_NO_ERROR;
    }
    if(((arg[0][0]=='i')||arg[0][0]=='I')&&(arg[2]==NULL))/*display all interface's profile applied*/
    {
        lport=0;
        pid=0;
        while(MLDSNP_TYPE_RETURN_SUCCESS == MLDSNP_PMGR_GetNextPortMLDProfileID(&lport, &pid))
        {
            if(pid==0)
                continue;

            memset(start_ip, 0, SYS_ADPT_IPV6_ADDR_LEN);
            memset(end_ip, 0, SYS_ADPT_IPV6_ADDR_LEN);

            if(SWCTRL_LPORT_TRUNK_PORT==SWCTRL_POM_LogicalPortToUserPort(lport,&unit,&port,&trunk_id))/*trunk*/
            {
                sprintf(buff,"Port Channel %lu information\r\n",(unsigned long)trunk_id);
                PROCESS_MORE(buff);
            }
            if(SWCTRL_LPORT_NORMAL_PORT==SWCTRL_POM_LogicalPortToUserPort(lport,&unit,&port,&trunk_id))
            {
                sprintf(buff,"Ethernet %lu/%lu information\r\n",(unsigned long)unit,(unsigned long)port);
                PROCESS_MORE(buff);
            }
            PROCESS_MORE("---------------------------------\r\n");
            sprintf(buff," MLD  Profile %lu\r\n",(unsigned long)pid);
            PROCESS_MORE(buff);
            if(!MLDSNP_POM_GetMLDProfileAccessMode(pid, &access_mode))
            {
                PROCESS_MORE(" Failed get profile access mode\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_deny)
            {
                PROCESS_MORE(" Deny\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_permit)
            {
                PROCESS_MORE(" Permit\r\n");
            }
            while(MLDSNP_POM_GetNextMLDProfileGroupbyPid(pid, start_ip, end_ip))
            {
                L_INET_Ntop(L_INET_AF_INET6, (void *)start_ip, sip, sizeof(sip));
                L_INET_Ntop(L_INET_AF_INET6, (void *)end_ip, eip, sizeof(eip));
                sprintf(buff," Range  %-15s   %-15s\r\n",sip, eip);
                PROCESS_MORE(buff);
            }
            PROCESS_MORE("\r\n");
            pid=0;

        }
        return CLI_NO_ERROR;
    }

    if(arg[1][0]=='e'||arg[1][0]=='E')/*display a ethernet interface's filter*/
    {
        unit = atoi(arg[2]);
        port = atoi(strchr(arg[2],'/')+1);
        pid=0;

        if( (verify_ret_e = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
        {
            display_ethernet_msg(verify_ret_e, unit, port);
            return CLI_NO_ERROR;
        }

        if(MLDSNP_POM_GetPortMLDProfileID(lport,&pid))
        {
            if(0==pid)
                return CLI_NO_ERROR;

            sprintf(buff,"Ethernet %lu/%lu information\r\n",(unsigned long)unit,(unsigned long)port);
            PROCESS_MORE(buff);
            PROCESS_MORE("---------------------------------\r\n");
            sprintf(buff," MLD  Profile %lu\r\n",(unsigned long)pid);
            PROCESS_MORE(buff);
            if(!MLDSNP_POM_GetMLDProfileAccessMode(pid, &access_mode))
            {
                PROCESS_MORE(" Failed get profile access mode\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_deny)
            {
                PROCESS_MORE(" Deny\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_permit)
            {
                PROCESS_MORE(" Permit\r\n");
            }
            while(MLDSNP_POM_GetNextMLDProfileGroupbyPid(pid, start_ip, end_ip))
            {
                L_INET_Ntop(L_INET_AF_INET6, (void *)start_ip, sip, sizeof(sip));
                L_INET_Ntop(L_INET_AF_INET6, (void *)end_ip, eip, sizeof(eip));
                sprintf(buff," Range  %-15s   %-15s\r\n",sip,eip);
                PROCESS_MORE(buff);
            }
        }
        return CLI_NO_ERROR;
    }

    if(arg[1][0]=='p'||arg[1][0]=='P')
    {
        pid=0;
        trunk_id=atoi(arg[2]);
        if( (verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret_t, trunk_id);
            return CLI_NO_ERROR;
        }
        if(MLDSNP_POM_GetPortMLDProfileID(lport, &pid))
        {
            if(0==pid)
                return CLI_NO_ERROR;
            sprintf(buff,"Port Channel %lu  information\r\n",(unsigned long)trunk_id);
            PROCESS_MORE(buff);
            PROCESS_MORE("---------------------------------\r\n");
            sprintf(buff, " MLD  Profile %lu\r\n",(unsigned long)pid);
            PROCESS_MORE(buff);
            if(!MLDSNP_POM_GetMLDProfileAccessMode(pid, &access_mode))
            {
                PROCESS_MORE(" Failed get profile access mode\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_deny)
            {
                PROCESS_MORE(" Deny\r\n");
            }
            if(access_mode==VAL_mldSnoopProfileAction_permit)
            {
                PROCESS_MORE(" Permit\r\n");
            }
            while(MLDSNP_POM_GetNextMLDProfileGroupbyPid(pid, start_ip, end_ip))
            {
                L_INET_Ntop(L_INET_AF_INET6, (void *)start_ip, sip, sizeof(sip));
                L_INET_Ntop(L_INET_AF_INET6, (void *)end_ip, eip, sizeof(eip));
                sprintf(buff," Range  %-15s   %-15s\r\n",sip,eip);
                PROCESS_MORE(buff);
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Show_Ipv6_Mld_Snooping_Profile(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    UI32_T pid;
    UI32_T access_mode;
    UI8_T  start_ip[SYS_ADPT_IPV6_ADDR_LEN]={0};
    UI8_T  end_ip[SYS_ADPT_IPV6_ADDR_LEN]={0};
    char  sip[48]={0};
    char  eip[48]={0};
    char  buff[CLI_DEF_MAX_BUFSIZE]       = {0};
    UI32_T line_num = 0;

    pid=0;
    if(arg[0]==NULL)/*display the profile list*/
    {
        while(MLDSNP_POM_GetNextMLDProfileID(&pid))
        {
            sprintf(buff," MLD  Profile %lu\r\n",(unsigned long)pid);
            PROCESS_MORE(buff);
        }
        return CLI_NO_ERROR;
    }
    else/*display a specific profile information*/
    {
        pid=CLI_LIB_AtoUl(arg[0],10);
        if(!MLDSNP_POM_IsMLDProfileExist(pid))
        {
            PROCESS_MORE(" This profile does not exist\r\n");
            return CLI_NO_ERROR;
        }
        sprintf(buff," MLD  Profile %lu\r\n",(unsigned long)pid);
        PROCESS_MORE(buff);
        if(!MLDSNP_POM_GetMLDProfileAccessMode(pid, &access_mode))
        {
            PROCESS_MORE(" Failed get profile access mode\r\n");
        }
        if(access_mode==VAL_mldSnoopProfileAction_deny)
        {
            PROCESS_MORE(" Deny\r\n");
        }
        if(access_mode==VAL_mldSnoopProfileAction_permit)
        {
            PROCESS_MORE(" Permit\r\n");
        }
        while(MLDSNP_POM_GetNextMLDProfileGroupbyPid(pid, start_ip, end_ip))
        {
            L_INET_Ntop(L_INET_AF_INET6, (void *)start_ip, sip, sizeof(sip));
            L_INET_Ntop(L_INET_AF_INET6, (void *)end_ip, eip, sizeof(eip));
            sprintf(buff," Range  %-15s   %-15s\r\n",sip,eip);
            PROCESS_MORE(buff);
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T
CLI_API_Show_Ipv6_Mld_Snooping_throttle(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    MLDSNP_OM_Throttle_T throttling_info;
    UI32_T lport,verify_port,verify_unit, trunk_id;
    CLI_API_EthStatus_T   verify_ret_e;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0, port_type;

    if(arg[1]==NULL)/*display filter status*/
    {
        for(lport=1;lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT;lport++)
        {
            if(!MLDSNP_POM_GetMLDThrottlingInfo(lport, &throttling_info))
                continue;

            port_type = SWCTRL_PMGR_LogicalPortToUserPort(lport, &verify_unit, &verify_port, &trunk_id);

            if(SWCTRL_LPORT_NORMAL_PORT == port_type)
            {
                sprintf(buff, "Eth  %lu/%lu Information \r\n",(unsigned long)verify_unit, (unsigned long)verify_port);
            }
            else if(SWCTRL_LPORT_TRUNK_PORT == port_type)
            {
                sprintf(buff, "Pch %lu Information \r\n", (unsigned long)trunk_id);
            }
            else
                continue;

            PROCESS_MORE(buff);
            if(throttling_info.throttle_status==VAL_mldSnoopThrottlePortRunningStatus_true)
            {
                sprintf(buff, " %-24s : TRUE\r\n", "Status");
                PROCESS_MORE(buff);
            }
            else
            {
                sprintf(buff, " %-24s : FALSE\r\n", "Status");
                PROCESS_MORE(buff);
            }
            if(throttling_info.action==VAL_mldSnoopThrottlePortAction_deny)
            {
                sprintf(buff, " %-24s : Deny \r\n", "Action:");
                PROCESS_MORE(buff);
            }
            else
            {
                sprintf(buff, " %-24s : Replace\r\n", "Action");
                PROCESS_MORE(buff);
            }
            sprintf(buff, " %-24s : %lu\r\n", "Max Multicast Groups", (unsigned long)throttling_info.max_group_number);
            PROCESS_MORE(buff);
            sprintf(buff, " %-24s : %lu\r\n", "Current Multicast Groups", (unsigned long)throttling_info.current_group_count);
            PROCESS_MORE(buff);
            sprintf(buff, "\r\n");
            PROCESS_MORE(buff);
        }
    }
    else
    {
        UI32_T trunk_id =0;

        if(arg[1][0] == 'e' || arg[1][0] == 'E')
        {
            verify_unit = atoi(arg[2]);
            verify_port = atoi(strchr(arg[2],'/')+1);
            if( (verify_ret_e = verify_ethernet(verify_unit, verify_port, &lport)) != CLI_API_ETH_OK)
            {
                CLI_LIB_PrintStr("This interface does not exist\r\n");
                return CLI_NO_ERROR;
            }
        }
        else
        {
            trunk_id = atoi(arg[2]);
            if(FALSE == SWCTRL_POM_TrunkIDToLogicalPort(trunk_id, &lport))
                return CLI_NO_ERROR;
        }

        if(!MLDSNP_POM_GetMLDThrottlingInfo(lport, &throttling_info))
        {
            CLI_LIB_PrintStr("Failed to get the throttling information\r\n");
            return CLI_NO_ERROR;
        }

        if(arg[1][0] == 'e' || arg[1][0] == 'E')
        {
            CLI_LIB_PrintStr_2("Eth  %lu/%lu Information \r\n",(unsigned long)verify_unit,(unsigned long)verify_port);
        }
        else
        {
            CLI_LIB_PrintStr_1("Pch  %lu Information \r\n", (unsigned long)trunk_id);
        }

        if(throttling_info.throttle_status==VAL_mldSnoopThrottlePortRunningStatus_true)
        {
            CLI_LIB_PrintStr_2(" %-24s %s\r\n", "Status", ": TRUE");
        }
        else
        {
            CLI_LIB_PrintStr_2(" %-24s %s\r\n", "Status", ": FALSE");
        }
        if(throttling_info.action==VAL_mldSnoopThrottlePortAction_deny)
        {
            CLI_LIB_PrintStr_2(" %-24s %s\r\n", "Action", ": Deny");
        }
        else
        {
            CLI_LIB_PrintStr_2(" %-24s %s\r\n", "Action", ": Replace");
        }
        CLI_LIB_PrintStr_3(" %-24s %s %lu\r\n", "Max Multicast Groups", ":",  (unsigned long)throttling_info.max_group_number);
        CLI_LIB_PrintStr_3(" %-24s %s %lu\r\n", "Current Multicast Groups", ":", (unsigned long)throttling_info.current_group_count);
        CLI_LIB_PrintStr("\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_IPv6_Mld_Snooping_ProxyReporting(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_PROXYREPORTING:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldSnoopProxyReporting(VAL_mldSnoopProxyReporting_enabled))
            {
                CLI_LIB_PrintStr("Failed to enable MLD snooping proxy reporting\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_PROXYREPORTING:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldSnoopProxyReporting(VAL_mldSnoopProxyReporting_disabled))
            {
                CLI_LIB_PrintStr("Failed to disable MLD snooping proxy reporting\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_IPv6_Mld_Snooping_UnsolicitedReportInterval(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_SNOOPING_UNSOLICITEDREPORTINTERVAL:
            /*printf("ip mld snooping Unsolicited Report Interval %s\r\n",arg[0]);*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetUnsolicitedReportInterval(atoi(arg[0])))
            {
                CLI_LIB_PrintStr("Failed to set unlolicited report interval\r\n");
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_SNOOPING_UNSOLICITEDREPORTINTERVAL:
            /*printf("no ip mld snooping Unsolicited Report Interval %s\r\n",arg[0]);*/
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetUnsolicitedReportInterval(SYS_DFLT_MLDSNP_UNSOLICIT_REPORT_INTERVAL))
            {
                CLI_LIB_PrintStr("Failed to set unlolicited report interval\r\n");
            }
            break;

        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_IPv6_Mld_Query_Guard_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T i=0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T verify_port=0;
    UI32_T status=0, lport=0;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))) )
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
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_IPV6_MLD_QUERYDROP:
                        status = VAL_mldSnoopQueryDrop_enable;
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W4_NO_IPV6_MLD_QUERYDROP:
                        status = VAL_mldSnoopQueryDrop_disable;
                        break;
                }

                if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetQueryDropStatus(lport, status))
                {
                    CLI_LIB_PrintStr("Failed to set MLD Query Guard.\r\n");
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_IPv6_Mld_Query_Guard_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    CLI_API_TrunkStatus_T verify_ret;
    UI32_T lport;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_IPV6_MLD_QUERYDROP:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetQueryDropStatus(lport, VAL_mldSnoopQueryDrop_enable))
            {
                CLI_LIB_PrintStr("Failed to set MLD Query guard.\r\n");
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W4_NO_IPV6_MLD_QUERYDROP:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetQueryDropStatus(lport, VAL_mldSnoopQueryDrop_disable))
            {
                CLI_LIB_PrintStr("Failed to set MLD Query guard.\r\n");
            }
            break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Ipv6_Mld_Query_Guard(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_QUERY_DROP== TRUE)
    UI32_T next_lport=0, lport=0,  line_num=0;
    UI32_T unit=0, port=0, trunk_id=0;
    UI32_T status;
    char buff[40]={0};

    if(arg[1]!=NULL)
    {
        if(arg[1][0]=='e'||arg[1][0]=='E')/*display a ethernet interface's filter*/
        {
            CLI_API_EthStatus_T  verify_ret_e;

            unit = atoi(arg[2]);
            port = atoi(strchr(arg[2],'/')+1);

            if( (verify_ret_e = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, unit, port);
                return CLI_NO_ERROR;
            }

            if(MLDSNP_POM_GetQueryDropStatus(lport, &status))
            {
                sprintf(buff, "Ethernet %lu/%lu: %s\r\n", (unsigned long)unit, (unsigned long)port, (status == VAL_mldSnoopQueryDrop_enable?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
            }
        }
        else
        if(arg[1][0]=='p'||arg[1][0]=='P')
        {
            UI32_T verify_ret_t;

            trunk_id=atoi(arg[2]);

            if( (verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, trunk_id);
                return CLI_NO_ERROR;
            }

            if(MLDSNP_POM_GetQueryDropStatus(lport, &status))
            {
                sprintf(buff, "Port Channel %lu: %s\r\n", (unsigned long)trunk_id, (status == VAL_mldSnoopQueryDrop_enable?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
            }
        }

        return CLI_NO_ERROR;
    }
    else
    while(MLDSNP_TYPE_RETURN_SUCCESS == MLDSNP_PMGR_GetNextQueryDropStatus(&next_lport, &status))
    {
        if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(next_lport, &unit, &port, &trunk_id))
            break;

        if(trunk_id == 0)
        {
            sprintf(buff, "Ethernet %lu/%lu: %s\r\n", (unsigned long)unit, (unsigned long)port, (status == VAL_mldSnoopQueryDrop_enable?"Enabled":"Disabled"));
        }
        else
        {
            if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                continue;

            sprintf(buff, "Port Channel %lu: %s\r\n", (unsigned long)trunk_id, (status == VAL_mldSnoopQueryDrop_enable?"Enabled":"Disabled"));
        }
        PROCESS_MORE(buff);
    }
#endif
    return CLI_NO_ERROR;
}



UI32_T CLI_API_IPv6_Multicast_Data_Drop_Eth(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T i;
    UI32_T verify_port=0;
    UI32_T verify_unit = ctrl_P->CMenu.unit_id;
    UI32_T status=0, lport=0;

    for (i = 1 ; i <= ctrl_P->sys_info.max_port_number ; i++)
    {
        if (ctrl_P->CMenu.port_id_list[(UI32_T)((i-1)/8)] & (1 << ( 7 - ((i-1)%8))) )
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
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W2_IPV6_MULTICASTDATADROP:
                        status = VAL_mldSnoopMulticastDataDrop_enable;
                        break;
                    case PRIVILEGE_CFG_INTERFACE_ETH_CMD_W3_NO_IPV6_MULTICASTDATADROP:
                        status = VAL_mldSnoopMulticastDataDrop_disable;
                        break;
                }

                if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMulticastDataDropStatus(lport, status))
                {
                    CLI_LIB_PrintStr("Failed to set multicast data drop\r\n");
                }
            }
        }
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_IPv6_Multicast_Data_Drop_Pch(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
    CLI_API_EthStatus_T verify_ret;
    UI32_T verify_trunk_id = ctrl_P->CMenu.pchannel_id;
    UI32_T lport;

    if( (verify_ret = verify_trunk(verify_trunk_id, &lport)) != CLI_API_TRUNK_OK)
    {
        display_trunk_msg(verify_ret, verify_trunk_id);
        return CLI_NO_ERROR;
    }

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W2_IPV6_MULTICASTDATADROP:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMulticastDataDropStatus(lport, VAL_mldSnoopMulticastDataDrop_enable))
            {
                CLI_LIB_PrintStr("Failed to set multicast data drop\r\n");
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_PCHANNEL_CMD_W3_NO_IPV6_MULTICASTDATADROP:
            if(MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMulticastDataDropStatus(lport, VAL_mldSnoopMulticastDataDrop_disable))
            {
                CLI_LIB_PrintStr("Failed to set multicast data drop\r\n");
            }
            break;
    }
#endif
    return CLI_NO_ERROR;
}


UI32_T CLI_API_Show_Ipv6_Multicast_Data_Drop(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
    UI32_T next_lport=0, lport=0, line_num=0;
    UI32_T unit, port, trunk_id;
    UI32_T status;
    char buff[40]={0};

    if(arg[1]!=NULL)
    {
        if(arg[1][0]=='e'||arg[1][0]=='E')/*display a ethernet interface's filter*/
        {
            CLI_API_EthStatus_T  verify_ret_e;

            unit = atoi(arg[2]);
            port = atoi(strchr(arg[2],'/')+1);

            if( (verify_ret_e = verify_ethernet(unit, port, &lport)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, unit, port);
                return CLI_NO_ERROR;
            }

            if(MLDSNP_POM_GetMulticastDataDropStatus(lport, &status))
            {
                sprintf(buff, "Ethernet %lu/%lu: %s\r\n", (unsigned long)unit, (unsigned long)port, (status == VAL_mldSnoopMulticastDataDrop_enable?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
            }
        }
        else if(arg[1][0]=='p'||arg[1][0]=='P')
        {
            UI32_T verify_ret_t;

            trunk_id=atoi(arg[2]);
            if( (verify_ret_t = verify_trunk(trunk_id, &lport)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, trunk_id);
                return CLI_NO_ERROR;
            }

            if(MLDSNP_POM_GetMulticastDataDropStatus(lport, &status))
            {
                sprintf(buff, "Port Channel %lu: %s\r\n", (unsigned long)trunk_id, (status == VAL_mldSnoopMulticastDataDrop_enable?"Enabled":"Disabled"));
                PROCESS_MORE(buff);
            }
        }

        return CLI_NO_ERROR;
    }
    else
    {
        while(MLDSNP_TYPE_RETURN_SUCCESS == MLDSNP_PMGR_GetNextMulticastDataDropStatus(&next_lport, &status))
        {
            if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(next_lport, &unit, &port, &trunk_id))
                break;

            if(trunk_id == 0)
            {
                sprintf(buff, "Ethernet %lu/%lu: %s\r\n", (unsigned long)unit, (unsigned long)port, (status == VAL_mldSnoopMulticastDataDrop_enable?"Enabled":"Disabled"));
            }
            else
            {
                if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                    continue;

                sprintf(buff, "Port Channel %lu: %s\r\n", (unsigned long)trunk_id, (status == VAL_mldSnoopMulticastDataDrop_enable?"Enabled":"Disabled"));
            }
            PROCESS_MORE(buff);
        }
    }
#endif
    return CLI_NO_ERROR;
}

/* command: ipv6 mld limit rate pps interface [eth|pch|vlan]
                               arg[0]
no ipv6 mld limit interface [eth|pch|vlan]
                  arg[0]
 */
UI32_T CLI_API_Ipv6_Mld_RateLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN== TRUE)
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    UI32_T                  vid, verify_unit =0, verify_port =0, verify_trunk_id=0;
    UI32_T                  ifindex=0, arg_chk_idx =2;
    UI16_T                  report_limit = SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT;
    BOOL_T                  is_trunk = FALSE, is_vlan = FALSE, loop_all = FALSE;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W4_IPV6_MLD_REPORTLIMIT_RATE:
        report_limit = atoi(arg[0]);
        arg_chk_idx=2;
        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W5_NO_IPV6_MLD_REPORTLIMIT_RATE:

        if (NULL == arg[0])
        {
            loop_all = TRUE;
        }
        else
        {
            arg_chk_idx = 1;
        }
        break;
    }

    if ((FALSE == loop_all) && (NULL != arg[arg_chk_idx]))
    {
        switch (arg[arg_chk_idx][0])
        {
        #if( SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
        case 'e':
        case 'E':
            verify_unit = atoi(arg[arg_chk_idx+1]);
            verify_port = atoi(strchr(arg[arg_chk_idx+1], '/') + 1);
            if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            break;

        case 'p':
        case 'P':
            verify_trunk_id = atoi(arg[arg_chk_idx+1]);
            is_trunk = TRUE;
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
            break;
        #endif
        #if( SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
        case 'v':
        case 'V':
            vid = atoi(arg[arg_chk_idx+1]);
            is_vlan = TRUE;

            if (FALSE == VLAN_POM_IsVlanExisted(vid))
            {
                CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)vid);
                return CLI_NO_ERROR;
            }
            VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);
            break;
        #endif
        default:
            return CLI_ERR_INTERNAL;
        }
    }

    if (FALSE == loop_all)
    {
        if (MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldReportLimitPerSec(ifindex, report_limit))
        {
            if (TRUE == is_vlan)
            {
                CLI_LIB_PrintStr_1(
                    "Failed to set MLD rate limit on VLAN %lu.\r\n", (unsigned long)vid);
            }
            else
            {
                if (TRUE == is_trunk)
                {
                    CLI_LIB_PrintStr_1(
                        "Failed to set MLD rate limit on trunk %lu.\r\n", (unsigned long)verify_trunk_id);
                }
                else
                {
                    CLI_LIB_PrintStr_2(
                        "Failed to set MLD rate limit on port %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
                }
            }
        }
    }
    else
    {
        ifindex=0;

        while(TRUE == SWCTRL_POM_GetNextLogicalPort(&ifindex))
        {
            if (FALSE == MLDSNP_PMGR_SetMldReportLimitPerSec(ifindex, SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT))
            {
                CLI_LIB_PrintStr_2(
                    "Failed to set MLD rate limit on port %lu/%lu.\r\n", (unsigned long)verify_unit, (unsigned long)verify_port);
            }
        }

        vid = 0;
        while(TRUE == VLAN_OM_GetNextVlanId(0, &vid))
        {
            if(VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(vid))
                continue;

            VLAN_VID_CONVERTTO_IFINDEX(vid, ifindex);

            if (MLDSNP_TYPE_RETURN_FAIL == MLDSNP_PMGR_SetMldReportLimitPerSec(ifindex, SYS_DFLT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN))
            {
                CLI_LIB_PrintStr_1(
                    "Failed to set MLD rate limit on VLAN %lu.\r\n", (unsigned long)vid);
            }
        }
    }

#endif /* #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND == TRUE) */

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE)
/* Sample:
 *         9                  18
 * Interface  Report Limit (pps)
 * ---------  ------------------
 * Eth 1/ 1                    0
 *
 */
static UI32_T Show_ReportLimit_Title(
    UI32_T  line_num,
    char    *buff)
{
    sprintf(buff, " %-9s  %-18s\r\n",
                    "Interface", "Rate Limit (pps)");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, " %-9s  %18s\r\n",
                    "---------", "------------------");
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T Show_ReportLimit_One(
    UI32_T  line_num,
    UI32_T  if_id,
    BOOL_T  is_vlan,
    char    *buff)
{
    UI32_T                      unit, port, trunk_id, in_vid =0;
    SWCTRL_Lport_Type_T         port_type = SWCTRL_LPORT_NORMAL_PORT;
    UI16_T                      report_limit;
    char                        inf_str[12] = {0};

    if (FALSE == is_vlan)
    {
        port_type = SWCTRL_POM_LogicalPortToUserPort(if_id, &unit, &port, &trunk_id);

        if (!(  (port_type == SWCTRL_LPORT_TRUNK_PORT)
              ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
             )
           )
        {
            return line_num;
        }
    }
    else
    {
        in_vid = if_id;
        VLAN_VID_CONVERTTO_IFINDEX(if_id, if_id);
    }


    if (FALSE == MLDSNP_POM_GetMldReportLimitPerSec(
                    if_id, &report_limit))
    {
        if (FALSE == is_vlan)
        {
            if (port_type == SWCTRL_LPORT_TRUNK_PORT)
            {
                sprintf(buff, "Failed to display MLD rate limit on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                sprintf(buff, "Failed to display MLD rate limit on trunk %lu.\r\n",
                    (unsigned long)trunk_id);
            }
        }
        else
        {
            sprintf(buff, "Failed to display MLD rate limit on VLAN %lu.\r\n",
                (unsigned long)if_id);
        }

        PROCESS_MORE_FUNC(buff);
        return line_num;
    }

    if (FALSE == is_vlan)
    {
        if (port_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(inf_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
        }
        else
        {
            sprintf(inf_str, "Pch %2lu", (unsigned long)trunk_id);
        }
    }
    else
    {
        sprintf(inf_str, "VLAN %4lu", (unsigned long)in_vid);
    }

    sprintf(buff, " %-9s  %18u\r\n",
                  inf_str, report_limit);

    PROCESS_MORE_FUNC(buff);
    return line_num;
}
#endif /* #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND == TRUE) */

/* command: show ip mld limit [interface eth|pch|vlan]
 */
UI32_T CLI_API_Show_Ipv6_Mld_RateLimit(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE)
    UI32_T  ifindex=0;
    UI32_T  line_num = 1;
    UI32_T  unit;
    UI32_T  port;
    UI32_T  trunk_id = 0;
    UI32_T  current_max_unit = 0;
    UI32_T  arg_chk_idx = 0;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    BOOL_T                  is_vlan = FALSE;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};

#if (SYS_CPNT_STACKING == TRUE)
    current_max_unit = SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;
#else
    current_max_unit = 1;
#endif

    if (arg[arg_chk_idx] == NULL)
    {
        line_num = Show_ReportLimit_Title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE)
        while(TRUE == SWCTRL_POM_GetNextLogicalPort(&ifindex))
        {
            if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id))
                break;

            if(trunk_id != 0)
            {
                if(TRK_PMGR_GetTrunkMemberCounts(trunk_id) == 0)
                    continue;
            }

            if ((line_num = Show_ReportLimit_One(line_num, ifindex, FALSE, buff)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
        #endif
        #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE)
        ifindex = 0;
        while(TRUE == VLAN_POM_GetNextVlanId(0, &ifindex))
        {
            if(VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(ifindex))
                continue;

            if ((line_num = Show_ReportLimit_One(line_num, ifindex, TRUE, buff)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
        #endif
    }
    else
    {
        switch(arg[arg_chk_idx+1][0])
        {
        #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE)
        case 'e':
        case 'E':
            unit = atoi(arg[arg_chk_idx+2]);
            port = atoi(strchr(arg[arg_chk_idx+2], '/') + 1);

            if ((verify_ret_e = verify_ethernet(unit, port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, unit, port);
                return CLI_NO_ERROR;
            }
            break;

        case 'p':
        case 'P':
            trunk_id = atoi(arg[arg_chk_idx+2]);
            if ((verify_ret_t = verify_trunk(trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, trunk_id);
                return CLI_NO_ERROR;
            }
            break;
        #endif
        #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
        case 'v':
        case 'V':
            ifindex = atoi(arg[arg_chk_idx+2]);
            is_vlan = TRUE;
            if (FALSE == VLAN_POM_IsVlanExisted(ifindex))
            {
                CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)ifindex);
                return CLI_NO_ERROR;
            }
            break;
        #endif
        default:
            return CLI_ERR_INTERNAL;
        }

        line_num = Show_ReportLimit_Title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        line_num = Show_ReportLimit_One(line_num, ifindex, is_vlan, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

    }
#endif /* #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND == TRUE) */

    return CLI_NO_ERROR;
}

UI32_T CLI_API_ClearIpv6MldSnoopingGroupDynamic(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    if(FALSE == MLDSNP_PMGR_ClearMldSnoopingDynamicgGroup())
    {
        CLI_LIB_PrintStr("Failed to clear MLD snooping dynamically learned group\r\n");
    }

    return CLI_NO_ERROR;
}

/* Sample:
 * Input Statistics:
 *         9        8        8        8         13           8         9      6
 * Interface Report   Leave    G Query  G(-S)-S Query Drop     Join Succ Group
 * --------- -------- -------- -------- ------------- -------- --------- ------
 * Eth 1/1          0        0        0             0        0        0       0
 * VLAN 1           0        0        0             0        0        0       0
 *
 */
static UI32_T show_mldsnp_input_statistics_title(
    UI32_T  line_num,
    char    *buff)
{
    PROCESS_MORE_FUNC(" Input Statistics:\r\n");
    sprintf(buff, " %-9s %-8s %-8s %-8s %-13s %-8s %-9s %-6s\r\n",
                    "Interface", "Report", "Leave", "G Query", "G(-S)-S Query",
                    "Drop", "Join Succ", "Group");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, " %-9s %-8s %-8s %-8s %-13s %-8s %-9s %-6s\r\n",
                    "---------", "--------", "--------","--------",
                    "-------------", "--------","---------", "------");
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T show_statistics_one_interface(
    UI32_T  line_num,
    UI32_T  if_id,
    BOOL_T  is_vlan,
    BOOL_T  is_output,
    char    *buff)
{
    UI32_T                      unit, port, trunk_id;
    SWCTRL_Lport_Type_T         port_type = SWCTRL_LPORT_NORMAL_PORT;
    MLDSNP_MGR_InfStat_T        inf_sts;
    char                        inf_str[12] = {0};

    if (FALSE == is_vlan)
    {
        port_type = SWCTRL_POM_LogicalPortToUserPort(if_id, &unit, &port, &trunk_id);

        if (!(  (port_type == SWCTRL_LPORT_TRUNK_PORT)
              ||(port_type == SWCTRL_LPORT_NORMAL_PORT)
             )
           )
        {
            return line_num;
        }
    }

    if (FALSE == MLDSNP_PMGR_GetInfStatistics(
                    if_id, is_vlan , &inf_sts))
    {
        if (FALSE == is_vlan)
        {
            if (port_type != SWCTRL_LPORT_TRUNK_PORT)
            {
                sprintf(buff, "Failed to display MLD snooping statistics on port %lu/%lu.\r\n",
                    (unsigned long)unit, (unsigned long)port);
            }
            else
            {
                sprintf(buff, "Failed to display MLD snooping statistics on port-channel %lu.\r\n",
                    (unsigned long)trunk_id);
            }
        }
        else
        {
            sprintf(buff, "Failed to display MLD snooping statistics on VLAN %lu.\r\n",
                (unsigned long)if_id);
        }

        PROCESS_MORE_FUNC(buff);
        return line_num;
    }

    if (FALSE == is_vlan)
    {
        if (port_type == SWCTRL_LPORT_NORMAL_PORT)
        {
            sprintf(inf_str, "Eth %lu/%2lu", (unsigned long)unit, (unsigned long)port);
        }
        else
        {
            sprintf(inf_str, "Pch %2lu", (unsigned long)trunk_id);
        }
    }
    else
    {
        sprintf(inf_str, "VLAN %4lu", (unsigned long)if_id);
    }

    if(!is_output)
    {
        sprintf(buff, " %-9s %8lu %8lu %8lu %13lu %8lu %9lu %6lu\r\n",
                      inf_str,
                      (unsigned long)inf_sts.counter.num_joins,
                      (unsigned long)inf_sts.counter.num_leaves,
                      (unsigned long)inf_sts.counter.num_gq_recv,
                      (unsigned long)inf_sts.counter.num_sq_recv,
                      (unsigned long)(inf_sts.counter.num_invalid_mld_recv +
                          inf_sts.counter.num_drop_by_filter +
                          #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
                          inf_sts.counter.num_drop_by_rate_limit +
                          #endif
                          inf_sts.counter.num_drop_by_mroute_port),
                      (unsigned long)inf_sts.counter.num_joins_succ,
                      (unsigned long)inf_sts.counter.num_grecs);
    }
    else
    {
        sprintf(buff, " %-9s %8lu %8lu %8lu %13lu %8lu %6lu\r\n",
                      inf_str,
                      (unsigned long)inf_sts.counter.num_joins_send,
                      (unsigned long)inf_sts.counter.num_leaves_send,
                      (unsigned long)inf_sts.counter.num_gq_send,
                      (unsigned long)inf_sts.counter.num_sq_send,
                      (unsigned long)inf_sts.counter.num_drop_output,
                      (unsigned long)inf_sts.counter.num_grecs);
    }

    PROCESS_MORE_FUNC(buff);
    return line_num;
}

static UI32_T show_query_statistics(UI32_T line_num,
        char    *buff,
        MLDSNP_MGR_InfStat_T *inf_sts)
{
    #define TITLE_DISPLAY_WIDE " %-25s "
    UI8_T   ipv6_addr_str[46]={0};
    UI8_T null_ip[SYS_ADPT_IPV6_ADDR_LEN] = {0};

    if(!IPV6_ADDR_SAME(inf_sts->other_querier_ip_addr, null_ip))
    {
      L_INET_Ntop(L_INET_AF_INET6, (void *) inf_sts->other_querier_ip_addr,
      (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
      sprintf(buff, TITLE_DISPLAY_WIDE": %s\r\n", "Other Querier Address", ipv6_addr_str);
    }
    else
        sprintf(buff, TITLE_DISPLAY_WIDE": None\r\n", "Other Querier Address");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld(m):%ld(s)\r\n", "Other Querier Expire", (long)(inf_sts->other_query_expire/60),
                                                                  (long)inf_sts->other_query_expire%60);
    PROCESS_MORE_FUNC(buff);

    sprintf(buff, TITLE_DISPLAY_WIDE": %ld(h):%ld(m):%ld(s)\r\n", "Other Querier Uptime", (long)(inf_sts->other_query_uptime/3600),
                                                                  (long)(inf_sts->other_query_uptime/60)%60,
                                                                  (long)inf_sts->other_query_uptime%60);
    PROCESS_MORE_FUNC(buff);
    L_INET_Ntop(L_INET_AF_INET6, (void *) inf_sts->self_querier_ip_addr,
                       (char *)ipv6_addr_str, sizeof(ipv6_addr_str));
    sprintf(buff, TITLE_DISPLAY_WIDE": %s\r\n", "Self Querier Address", ipv6_addr_str);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld(m):%ld(s)\r\n", "Self Querier Expire Time",
                                                                  (long)(inf_sts->query_exptime/60),
                                                                  (long)inf_sts->query_exptime%60);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld(h):%ld(m):%ld(s)\r\n", "Self Querier UpTime", (long)(inf_sts->query_uptime/3600),
                                                                  (long)(inf_sts->query_uptime/60)%60,
                                                                  (long)inf_sts->query_uptime%60);

    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld\r\n", "General Query Received", (long)inf_sts->counter.num_gq_recv);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld\r\n", "General Query Sent", (long)inf_sts->counter.num_gq_send);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld\r\n", "Specific Query Received", (long)inf_sts->counter.num_sq_recv);
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, TITLE_DISPLAY_WIDE": %ld\r\n", "Specific Query Sent", (long)inf_sts->counter.num_sq_send);
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

static UI32_T show_mld_snooping_vlan_query_statitics(
    UI32_T  line_num,
    UI32_T  if_id,
    char    *buff)
{
    MLDSNP_MGR_InfStat_T       inf_sts;

    if (FALSE == MLDSNP_PMGR_GetInfStatistics(
                    if_id, TRUE , &inf_sts))
    {
        sprintf(buff, "Failed to display MLD snooping query statistics on VLAN %lu.\r\n",
            (unsigned long)if_id);

        PROCESS_MORE_FUNC(buff);
        return line_num;
    }

    return show_query_statistics(line_num, buff, &inf_sts);
}

/* Sample:
 * Output Statistics:
 *         9        8        8        8         13
 * Interface Report   Leave    G Query  G(-S)-S Query
 * --------- -------- -------- -------- -------------
 * Eth 1/1          0        0        0             0
 * VLAN 1           0        0        0             0
 *
 */
static UI32_T show_output_statistics_title(
    UI32_T  line_num,
    char    *buff)
{
    PROCESS_MORE_FUNC(" Output Statistics:\r\n");
    sprintf(buff, " %-9s %-8s %-8s %-8s %-13s %-8s %-6s\r\n",
                     "Interface", "Report", "Leave", "G Query", "G(-S)-S Query", "Drop", "Group");
    PROCESS_MORE_FUNC(buff);
    sprintf(buff, " %-9s %-8s %-8s %-8s %-13s %-8s %-6s\r\n",
                    "---------", "--------", "--------",
                    "--------", "-------------", "--------", "------");
    PROCESS_MORE_FUNC(buff);

    return line_num;
}

/* command: show ip mld snooping statistics input [interface eth|pch|vlan]
 */
UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Statistics_Input(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  ifindex =0;
    UI32_T  line_num = 1;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  arg_chk_idx = 0;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    BOOL_T                  is_vlan = FALSE;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (arg[arg_chk_idx] == NULL)
    {
        line_num = show_mldsnp_input_statistics_title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        while(SWCTRL_POM_GetNextLogicalPort(&ifindex)!=SWCTRL_LPORT_UNKNOWN_PORT)
        {
            if ((line_num = show_statistics_one_interface(line_num, ifindex, FALSE, FALSE, buff)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }

        ifindex = 0;
        while(TRUE == VLAN_OM_GetNextVlanId(0, &ifindex))
        {
            if(VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(ifindex))
                continue;

            if ((line_num = show_statistics_one_interface(line_num, ifindex, TRUE, FALSE,buff)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }
    else
    {
        switch(arg[arg_chk_idx+1][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[arg_chk_idx+2]);
            verify_port = atoi(strchr(arg[arg_chk_idx+2], '/') + 1);

            if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            break;

        case 'p':
        case 'P':
            verify_trunk_id = atoi(arg[arg_chk_idx+2]);
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
            break;

        case 'v':
        case 'V':
            ifindex = atoi(arg[arg_chk_idx+2]);
            is_vlan = TRUE;
            if (FALSE == VLAN_POM_IsVlanExisted(ifindex))
            {
                CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)ifindex);
                return CLI_NO_ERROR;
            }

            break;

        default:
            return CLI_ERR_INTERNAL;
        }

        line_num = show_mldsnp_input_statistics_title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        line_num = show_statistics_one_interface(line_num, ifindex, is_vlan, FALSE, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
    }

    return CLI_NO_ERROR;
}


/* command: show ip mld snooping statistics output [interface eth|pch|vlan]
 */
UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Statistics_Output(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T  ifindex=0;
    UI32_T  line_num = 1;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    UI32_T  arg_chk_idx = 0;
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    BOOL_T                  is_vlan = FALSE;
    char                    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    if (arg[arg_chk_idx] == NULL)
    {
        line_num = show_output_statistics_title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        while(SWCTRL_PMGR_GetNextLogicalPort(&ifindex) != SWCTRL_LPORT_UNKNOWN_PORT)
        {
          if ((line_num = show_statistics_one_interface(line_num, ifindex, FALSE, TRUE, buff)) == EXIT_SESSION_MORE)
          {
              return CLI_EXIT_SESSION;
          }
          if (line_num == JUMP_OUT_MORE)
          {
              return CLI_NO_ERROR;
          }
        }

        ifindex = 0;
        while(TRUE == VLAN_OM_GetNextVlanId(0, &ifindex))
        {
            if(VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(ifindex))
                continue;

            if ((line_num = show_statistics_one_interface(line_num, ifindex, TRUE, TRUE,buff)) == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }
            if (line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
        }
    }
    else
    {
        switch(arg[arg_chk_idx+1][0])
        {
        case 'e':
        case 'E':
            verify_unit = atoi(arg[arg_chk_idx+2]);
            verify_port = atoi(strchr(arg[arg_chk_idx+2], '/') + 1);

            if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
            {
                display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
                return CLI_NO_ERROR;
            }
            break;

        case 'p':
        case 'P':
            verify_trunk_id = atoi(arg[arg_chk_idx+2]);
            if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
            {
                display_trunk_msg(verify_ret_t, verify_trunk_id);
                return CLI_NO_ERROR;
            }
            break;

        case 'v':
        case 'V':
            ifindex = atoi(arg[arg_chk_idx+2]);
            is_vlan = TRUE;
            if (FALSE == VLAN_POM_IsVlanExisted(ifindex))
            {
                CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)ifindex);
                return CLI_NO_ERROR;
            }

            break;

        default:
            return CLI_ERR_INTERNAL;
        }

        line_num = show_output_statistics_title(line_num, buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

        line_num = show_statistics_one_interface(line_num, ifindex, is_vlan, TRUE,buff);
        if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
        if (line_num == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }

    }

    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_Ipv6_Mld_Snooping_Statistics_query(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T line_num=0, vid=0;
    char buff[CLI_DEF_MAX_BUFSIZE];

    if(arg[0]!=NULL)
    {
        vid = atoi(arg[1]);
        line_num =show_mld_snooping_vlan_query_statitics(line_num, vid, buff);
    }
    else
    {
        while(TRUE == VLAN_OM_GetNextVlanId(0, &vid))
        {
            if(VAL_dot1qVlanStaticRowStatus_active != VLAN_OM_GetDot1qVlanRowStatus(vid))
                continue;

            PROCESS_MORE_1("VLAN %lu\r\n", (unsigned long)vid);
            line_num =show_mld_snooping_vlan_query_statitics(line_num, vid, buff);

            if (line_num == EXIT_SESSION_MORE
            ||line_num == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            PROCESS_MORE("\r\n");
        }
    }

    return CLI_NO_ERROR;
}

static BOOL_T clear_statistics_get_interface(UI32_T *ifindex, char *arg[])
{
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    UI32_T unit, port, trunk_id, vid;

    switch(arg[1][0])
    {
    case 'e':
    case 'E':
        unit = atoi(arg[2]);
        port = atoi(strchr(arg[2], '/') + 1);

        if ((verify_ret_e = verify_ethernet(unit, port, ifindex)) != CLI_API_ETH_OK)
        {
            display_ethernet_msg(verify_ret_e, unit, port);
            return FALSE;
        }
        break;

    case 'p':
    case 'P':
        trunk_id = atoi(arg[2]);
        if ((verify_ret_t = verify_trunk(trunk_id, ifindex)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret_t, trunk_id);
            return FALSE;
        }
        break;

    case 'v':
    case 'V':
        vid = atoi(arg[2]);

        if (FALSE == VLAN_POM_IsVlanExisted(vid))
        {
            CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)vid);
            return FALSE;
        }

        VLAN_VID_CONVERTTO_IFINDEX(vid , *ifindex);
        break;
    }
    return TRUE;
}


UI32_T CLI_API_Clear_Ipv6_Mld_snooping_Statistics(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex=0;

    if(arg[0] == NULL)
    {
        MLDSNP_PMGR_Clear_Ipv6_Mld_snooping_Statistics(0);
        return CLI_NO_ERROR;
    }

    if(FALSE == clear_statistics_get_interface(&ifindex, &arg[0]))
        return CLI_NO_ERROR;

    MLDSNP_PMGR_Clear_Ipv6_Mld_snooping_Statistics(ifindex);

    return CLI_NO_ERROR;
}


UI32_T CLI_API_Show_Ipv6_mldsnp_vlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status;
    MLDSNP_TYPE_ImmediateByHostStatus_T immediate_leave_byhost_status;
    MLDSNP_TYPE_UnknownBehavior_T flood_behavior;
    UI32_T vid;

    if(arg[0] == NULL)
    {/*all vlan*/
        vid =0;
        while(TRUE == VLAN_OM_GetNextVlanId(0, &vid))
        {
            CLI_LIB_PrintStr_1("VLAN %lu \r\n", (unsigned long)vid);

            MLDSNP_POM_GetUnknownFloodBehavior(vid, &flood_behavior);
            MLDSNP_POM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
            MLDSNP_POM_GetImmediateLeaveByHostStatus(vid, &immediate_leave_byhost_status);

            CLI_LIB_PrintStr_1(" Immediate Leave        : %s\r\n",
                       MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == immediate_leave_byhost_status?"Enabled (by host ip)":
                       MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status?"Enabled":"Disabled"
                       );

            CLI_LIB_PrintStr_1(" Unknown Flood Behavior : %s\r\n",
                               MLDSNP_TYPE_UNKNOWN_BEHAVIOR_DROP ==flood_behavior?"Drop":
                               MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD==flood_behavior?"Flood":"To Router Port"
                               );
        }
    }
    else
    {/*specify vlan*/
        vid = atoi(arg[0]);
        CLI_LIB_PrintStr_1("VLAN %lu \r\n", (unsigned long)vid);
        MLDSNP_POM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
        MLDSNP_POM_GetImmediateLeaveByHostStatus(vid, &immediate_leave_byhost_status);
        MLDSNP_POM_GetUnknownFloodBehavior(vid, &flood_behavior);

        CLI_LIB_PrintStr_1(" Immediate Leave        : %s\r\n",
                   MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == immediate_leave_byhost_status?"Enabled (by host ip)":
                   MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status?"Enabled":"Disabled"
                   );

        CLI_LIB_PrintStr_1(" Unknown Flood Behavior : %s\r\n",
                           MLDSNP_TYPE_UNKNOWN_BEHAVIOR_DROP ==flood_behavior?"Drop":
                           MLDSNP_TYPE_UNKNOWN_BEHAVIOR_FLOOD==flood_behavior?"Flood":"To Router Port"
                           );
    }

    return CLI_NO_ERROR;
}


static UI32_T show_vlan_statistics_summary(UI32_T line_num,
                                           BOOL_T is_vlan,
                                           MLDSNP_MGR_InfStat_T *inf_sts,
                                           char *buff)
{
                                    /*titile: value    title: value */
    #define MLDSNP_STAT_TWO_COLUM " %-16s: %-15s     %-18s: %-12s\r\n"
    #define MLDSNP_STAT_ONE_COLUM_R " %-16s  %-15s     %-18s: %-12s\r\n"
    #define MLDSNP_STAT_ONE_COLUM_L " %-16s: %-15s     %-18s  %-12s\r\n"

    char cp[L_INET_MAX_IP6ADDR_STR_LEN+1]={0};
    char tmp2[26], tmp4[26];

    SYSFUN_Sprintf(buff, "%-15s %ld\r\n", "Number of Groups: ", (long)inf_sts->counter.num_grecs);
    PROCESS_MORE_FUNC(buff);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, "Querier:", "", "Report & Leave:", "");
    PROCESS_MORE_FUNC(buff);

    if(is_vlan)
    {
        L_INET_Ntop(L_INET_AF_INET6, inf_sts->other_querier_ip_addr, cp, sizeof(cp));
        if(strlen(cp) != 2)
          SYSFUN_Sprintf(tmp2, "%s", cp);
        else
          SYSFUN_Sprintf(tmp2, "%s", "None");

        L_INET_Ntop(L_INET_AF_INET6, inf_sts->host_ip_addr, cp, sizeof(cp));
        if(strlen(cp) != 2)
            SYSFUN_Sprintf(tmp4, "%s", cp);
        else
            SYSFUN_Sprintf(tmp4, "%s", "None");

        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_L, " Other Querier", tmp2, " ", " ");
        PROCESS_MORE_FUNC(buff);
        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, " ", " ", " Host Addr", tmp4);
        PROCESS_MORE_FUNC(buff);

        SYSFUN_Sprintf(tmp2, "%ld(h):%ld(m):%ld(s)", (long)(inf_sts->other_query_uptime/3600),
                                                            (long)(inf_sts->other_query_uptime/60)%60,
                                                            (long)inf_sts->other_query_uptime%60);
        SYSFUN_Sprintf(tmp4, "%lu sec", (unsigned long)inf_sts->unsolict_expire);
        SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, " Other Uptime", tmp2," Unsolicit Expire" , tmp4);
        PROCESS_MORE_FUNC(buff);
        SYSFUN_Sprintf(tmp2, "%ld(m):%ld(s)", (long)(inf_sts->other_query_expire/60),
                                                 (long)inf_sts->other_query_expire%60);
        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_L, " Other Expire", tmp2," " , " ");
        PROCESS_MORE_FUNC(buff);

        L_INET_Ntop(L_INET_AF_INET6, inf_sts->self_querier_ip_addr, cp, sizeof(cp));
        if(strlen(cp) != 2)
          SYSFUN_Sprintf(tmp2, "%s", cp);
        else
          SYSFUN_Sprintf(tmp2, "%s", "None");
        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_L, " Self Addr", tmp2, " ", " ");
        PROCESS_MORE_FUNC(buff);
        SYSFUN_Sprintf(tmp2, "%2ld(m):%2ld(s)", (long)(inf_sts->query_exptime/60),
                                                 (long)inf_sts->query_exptime%60);
        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_L, " Self Expire", tmp2, " "," ");
        PROCESS_MORE_FUNC(buff);
        SYSFUN_Sprintf(tmp2, "%ld(h):%ld(m):%ld(s)", (long)(inf_sts->query_uptime/3600),
                                                            (long)(inf_sts->query_uptime/60)%60,
                                                            (long)inf_sts->query_uptime%60);
        SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_L, " Self Uptime", tmp2, " "," ");
        PROCESS_MORE_FUNC(buff);
    }
    /*tranmsit
     */
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, " Transmit", " ", " Transmit", " ");
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp2, "%ld", (long)inf_sts->counter.num_gq_send);
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_joins_send);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, "  General", tmp2, "  Report", tmp4);
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp2, "%ld", (long)inf_sts->counter.num_sq_send);
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_leaves_send);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, "  Group Specific", tmp2, "  Leave", tmp4);
    PROCESS_MORE_FUNC(buff);
    /*receive
     */
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, " Recieved ", " ", " Recieved", " ");
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp2, "%ld", (long)inf_sts->counter.num_gq_recv);
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_joins);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, "  General ", tmp2, "  Report", tmp4);
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp2, "%ld", (long)inf_sts->counter.num_sq_recv);
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_leaves);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_TWO_COLUM, "  Group Specific", tmp2, "  Leave", tmp4);
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_joins_succ);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, "  ", " ", "  join Success", tmp4);
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_drop_by_filter);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, "  ", " ", "  Filter Drop", tmp4);
    PROCESS_MORE_FUNC(buff);
    #if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_drop_by_rate_limit);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, "  ", " ", "  Rate Limit Drop", tmp4);
    PROCESS_MORE_FUNC(buff);
    #endif
    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_drop_by_mroute_port);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, "  ", " ", "  Source Port Drop", tmp4);
    PROCESS_MORE_FUNC(buff);

    SYSFUN_Sprintf(tmp4, "%ld", (long)inf_sts->counter.num_invalid_mld_recv);
    SYSFUN_Sprintf(buff, MLDSNP_STAT_ONE_COLUM_R, "  ", " ", "  Others Drop", tmp4);
    PROCESS_MORE_FUNC(buff);


    return line_num;
}


UI32_T CLI_API_Show_IPv6_MLD_Snoop_Statitics_Summary(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    MLDSNP_MGR_InfStat_T       inf_sts;
    char    buff[CLI_DEF_MAX_BUFSIZE]={0};
    CLI_API_EthStatus_T     verify_ret_e;
    CLI_API_TrunkStatus_T   verify_ret_t;
    UI32_T  line_num=0;
    UI32_T  ifindex;
    UI32_T  verify_unit;
    UI32_T  verify_port;
    UI32_T  verify_trunk_id = 0;
    BOOL_T  is_vlan = FALSE;

    switch(arg[1][0])
    {
    case 'e':
    case 'E':
        verify_unit = atoi(arg[2]);
        verify_port = atoi(strchr(arg[2], '/') + 1);

        if ((verify_ret_e = verify_ethernet(verify_unit, verify_port, &ifindex)) != CLI_API_ETH_OK)
        {
            display_ethernet_msg(verify_ret_e, verify_unit, verify_port);
            return CLI_NO_ERROR;
        }
        break;

    case 'p':
    case 'P':
        verify_trunk_id = atoi(arg[2]);
        if ((verify_ret_t = verify_trunk(verify_trunk_id, &ifindex)) != CLI_API_TRUNK_OK)
        {
            display_trunk_msg(verify_ret_t, verify_trunk_id);
            return CLI_NO_ERROR;
        }
        break;

    case 'v':
    case 'V':
        ifindex = atoi(arg[2]);
        is_vlan = TRUE;
        if (FALSE == VLAN_POM_IsVlanExisted(ifindex))
        {
            CLI_LIB_PrintStr_1("VALN %lu does not exist.\r\n", (unsigned long)ifindex);
            return CLI_NO_ERROR;
        }
    }

    if (FALSE == MLDSNP_PMGR_GetInfStatistics(
                    ifindex, is_vlan , &inf_sts))
    {
        sprintf(buff, "Failed to display statistics\r\n");
        PROCESS_MORE(buff);
    }

    show_vlan_statistics_summary(line_num, is_vlan, &inf_sts, buff);

    return CLI_NO_ERROR;
}


#endif /* SYS_CPNT_MLDSNP == TRUE */
