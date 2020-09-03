#include "clis.h"
#include "cli_def.h"
#include "cli_mgr.h"
#include "cli_io.h"
#include "cli_api.h"
#include "cli_task.h"
#include "cli_func.h"
#include "cli_main.h"
#include "cli_lib.h"
#include "cli_api.h"
#include "cli_pars.h"

#include "xstp_mgr.h"

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_mgr.h"
#include "debug_type.h"
#endif

#if (SYS_CPNT_DEBUG == TRUE)
static BOOL_T cmpcscflag(UI32_T module_id, UI32_T session_id ,UI32_T flag);
#endif /* #if (SYS_CPNT_DEBUG == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Aaa
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of aaa
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Aaa(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE) && 0
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();
    BOOL_T show_in_arg0=FALSE;

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_AAA)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_AAA, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_AAA_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                flag|=DEBUG_TYPE_AAA_ALL;
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_AAA_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_AAA_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_AAA_EVENT;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][1])
                {
                    case 'l':
                    case 'L':
                        flag|=DEBUG_TYPE_AAA_SHOW_ALL;
                        break;

                    case 'c':
                    case 'C':
                        flag|=DEBUG_TYPE_AAA_SHOW_ACCT;
                        break;

                    case 'u':
                    case 'U':
                       flag|=DEBUG_TYPE_AAA_SHOW_AUTHOR;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][1])
            {
                case 'l':
                case 'L':
                    flag|=DEBUG_TYPE_AAA_SHOW_ALL;
                    break;

                case 'c':
                case 'C':
                    flag|=DEBUG_TYPE_AAA_SHOW_ACCT;
                    break;

                case 'u':
                case 'U':
                    flag|=DEBUG_TYPE_AAA_SHOW_AUTHOR;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_AAA_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_AAA:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_AAA, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_AAA:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_AAA, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Aaa*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Arp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of arp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG_ARP == TRUE)
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_ARP)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_ARP, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    flag|=DEBUG_TYPE_ARP_ALL;

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_ARP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_ARP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_ARP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_ARP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Arp*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Cluster
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of cluster
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Cluster(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG == TRUE)&& 0
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_CLUSTER)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_CLUSTER, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_CLUSTER_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_CLUSTER_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_CLUSTER_EVENT;
                break;

            case 'p':
            case 'P':
                flag|=DEBUG_TYPE_CLUSTER_PACKET;
                break;

            default:
                    return CLI_ERR_INTERNAL;
        }
    }/*if(arg[0][0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_CLUSTER:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_CLUSTER, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_CLUSTER:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_CLUSTER, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Dhcp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of dhcp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG_DHCP == TRUE)
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_DHCP)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_DHCP, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    switch(arg[0][0])
    {
        case 'a':
        case 'A':
                flag|=DEBUG_TYPE_DHCP_ALL;
            break;
        case 'C':
        case 'c':
            if((arg[0][1]== 'o')||(arg[0][1]=='O'))
                flag|=DEBUG_TYPE_DHCP_CONFIG;
            else    
                flag|=DEBUG_TYPE_DHCP_CLIENT;
            break;
        case 'E':
        case 'e':
                flag|=DEBUG_TYPE_DHCP_EVENT;
            break;
        case 'D':
        case 'd':
                flag|=DEBUG_TYPE_DHCP_DATABASE;
            break;
        case 'p':
        case 'P':
                flag|=DEBUG_TYPE_DHCP_PACKET;
            break;
        case 'R':
        case 'r':
                flag|=DEBUG_TYPE_DHCP_RELAY;
            break;
        case 'S':
        case 's':
                flag|=DEBUG_TYPE_DHCP_SERVER;
            break;
        default:
                return CLI_ERR_INTERNAL;
        }

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_DHCP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_DHCP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_DHCP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_DHCP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Cluster*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Dot1x
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of dot1x
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Dot1x(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE)&& 0
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();
    BOOL_T show_in_arg0=FALSE;

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_DOT1X)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_DOT1X, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_DOT1X_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                flag|=DEBUG_TYPE_DOT1X_CONFIG;
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_DOT1X_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_DOT1X_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_DOT1X_EVENT;
                break;

            case 'p':
            case 'P':
                flag|=DEBUG_TYPE_DOT1X_PACKET;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][0])
                {
                    case 'a':
                    case 'A':
                        flag|=DEBUG_TYPE_DOT1X_SHOW_ALL;
                        break;

                    case 's':
                    case 'S':
                        switch(arg[2][0])
                        {
                            case 'b':
                            case 'B':
                                flag|=DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND;
                                break;

                            case 'p':
                            case 'P':
                                flag|=DEBUG_TYPE_DOT1X_SHOW_SM_PAE;
                                break;

                            default:
                                flag|=DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND|DEBUG_TYPE_DOT1X_SHOW_SM_PAE;
                                break;
                        }
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][0])
            {
                case 'a':
                case 'A':
                    flag|=DEBUG_TYPE_DOT1X_SHOW_ALL;
                    break;

                case 's':
                case 'S':
                    switch(arg[3][0])
                    {
                        case 'b':
                        case 'B':
                            flag|=DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND;
                            break;

                        case 'p':
                        case 'P':
                            flag|=DEBUG_TYPE_DOT1X_SHOW_SM_PAE;
                            break;

                        default:
                            flag|=DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND|DEBUG_TYPE_DOT1X_SHOW_SM_PAE;
                            break;
                  }
                  break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_DOT1X_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_DOT1X:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_DOT1X, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_DOT1X:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_DOT1X, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Dot1x*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Ipdhcpsnp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of ip dhcp snooping
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Ipdhcpsnp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG_DHCPSNP == TRUE)
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W4_DEBUG_IP_DHCP_SNOOPING)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_DHCPSNP, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    switch(arg[0][0])
    {
        case 'A':
        case 'a':
            flag|=DEBUG_TYPE_DHCPSNP_ALL;
            break;
        case 'c':
        case 'C':
            flag|=DEBUG_TYPE_DHCPSNP_CONFIG;
            break;
        case 'd':
        case 'D':
            flag|=DEBUG_TYPE_DHCPSNP_DATABASE;
            break;
        case 'e':
        case 'E':
            flag|=DEBUG_TYPE_DHCPSNP_EVENT;
            break;
        case 'p':
        case 'P':
            flag|=DEBUG_TYPE_DHCPSNP_PACKET;
            break;
        default:
                return CLI_ERR_INTERNAL;
    }

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_DEBUG_IP_DHCP_SNOOPING:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_DHCPSNP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W5_NO_DEBUG_IP_DHCP_SNOOPING:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_DHCPSNP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Ipdhcpsnp*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Lacp
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of lacp
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Lacp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG_LACP == TRUE)
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_LACP)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_LACP, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_LACP_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_LACP_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_LACP_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_LACP_EVENT;
                break;

            case 'p':
            case 'P':
                if(arg[1])
                {
                    flag|=DEBUG_TYPE_LACP_PACKET_DETAIL;
                }
                else
                {
                    flag|=DEBUG_TYPE_LACP_PACKET;
                }
                break;

            case 's':
            case 'S':
                flag|=DEBUG_TYPE_LACP_STATE_MACHING;
                break;

            default:
                    return CLI_ERR_INTERNAL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_LACP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_LACP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_LACP:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_LACP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_DEBUG_LACP == TRUE) */

    return CLI_NO_ERROR;
} /*ned-of-CLI_API_Debug_Lacp*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Portsecurity
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of port security
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Portsecurity(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE)&& 0
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();
    BOOL_T show_in_arg0=FALSE;

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W3_DEBUG_PORT_SECURITY)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_PSEC, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_PSEC_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                flag|=DEBUG_TYPE_PSEC_ALL;
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_PSEC_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_PSEC_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_PSEC_EVENT;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][0])
                {
                    case 'a':
                    case 'A':
                        flag|=DEBUG_TYPE_PSEC_SHOW_ALL;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][0])
            {
                case 'a':
                case 'A':
                    flag|=DEBUG_TYPE_PSEC_SHOW_ALL;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_PSEC_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W3_DEBUG_PORT_SECURITY:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_PSEC, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W4_NO_DEBUG_PORT_SECURITY:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_PSEC, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Portsecurity*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Radius
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of radius
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Radius(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE)&& 0
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();
    BOOL_T show_in_arg0=FALSE;

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_RADIUS)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_RADIUS, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_RADIUS_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                if(arg[0][1]=='l' || arg[0][1]=='L')
                {
                    flag|=DEBUG_TYPE_RADIUS_ALL;
                }
                else if(arg[0][1]=='v' || arg[0][1]=='V')
                {
                    flag|=DEBUG_TYPE_RADIUS_AVPAIR;
                }
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_RADIUS_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_RADIUS_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_RADIUS_EVENT;
                break;

            case 'p':
            case 'P':
                flag|=DEBUG_TYPE_RADIUS_PACKET;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][1])
                {
                    case 'l':
                    case 'L':
                        flag|=DEBUG_TYPE_RADIUS_SHOW_ALL;
                        break;

                    case 'c':
                    case 'C':
                        flag|=DEBUG_TYPE_RADIUS_SHOW_ACCT;
                        break;

                    case 'u':
                    case 'U':
                        if(arg[1][4]=='e' || arg[1][4]=='E')/*authentication*/
                        {
                            flag|=DEBUG_TYPE_RADIUS_SHOW_AUTHEN;
                        }
                        else if(arg[1][4]=='o' || arg[1][4]=='O')/*authorization*/
                        {
                            flag|=DEBUG_TYPE_RADIUS_SHOW_AUTHOR;
                        }
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][1])
            {
                case 'l':
                case 'L':
                    flag|=DEBUG_TYPE_RADIUS_SHOW_ALL;
                    break;

                case 'c':
                case 'C':
                    flag|=DEBUG_TYPE_RADIUS_SHOW_ACCT;
                    break;

                case 'u':
                case 'U':
                    if(arg[2][4]=='e' || arg[2][4]=='E')/*authentication*/
                    {
                        flag|=DEBUG_TYPE_RADIUS_SHOW_AUTHEN;
                    }
                    else if(arg[2][4]=='o' || arg[2][4]=='O')/*authorization*/
                    {
                        flag|=DEBUG_TYPE_RADIUS_SHOW_AUTHOR;
                    }
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_RADIUS_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_RADIUS:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_RADIUS, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_RADIUS:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_RADIUS, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Radius*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Tacacs
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of tacacs
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Tacacs(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE)&& 0
    UI32_T flag=0;
    BOOL_T show_in_arg0=FALSE;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_TACACS)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_TACACS, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_TACACS_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                if(arg[0][1]=='l' || arg[0][1]=='L')
                {
                    flag|=DEBUG_TYPE_TACACS_ALL;
                }
                else if(arg[0][1]=='v' || arg[0][1]=='V')
                {
                    flag|=DEBUG_TYPE_TACACS_AVPAIR;
                }
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_TACACS_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_TACACS_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_TACACS_EVENT;
                break;

            case 'p':
            case 'P':
                flag|=DEBUG_TYPE_TACACS_PACKET;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][1])
                {
                    case 'l':
                    case 'L':
                        flag|=DEBUG_TYPE_TACACS_SHOW_ALL;
                        break;

                    case 'c':
                    case 'C':
                        flag|=DEBUG_TYPE_TACACS_SHOW_ACCT;
                        break;

                    case 'u':
                    case 'U':
                        if(arg[1][4]=='e' || arg[1][4]=='E')/*authentication*/
                        {
                            flag|=DEBUG_TYPE_TACACS_SHOW_AUTHEN;
                        }
                        else if(arg[1][4]=='o' || arg[1][4]=='O')/*authorization*/
                        {
                            flag|=DEBUG_TYPE_TACACS_SHOW_AUTHOR;
                        }
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][1])
            {
                case 'l':
                case 'L':
                    flag|=DEBUG_TYPE_TACACS_SHOW_ALL;
                    break;

                case 'c':
                case 'C':
                    flag|=DEBUG_TYPE_TACACS_SHOW_ACCT;
                    break;

                case 'u':
                case 'U':
                    if(arg[2][4]=='e' || arg[2][4]=='E')/*authentication*/
                    {
                        flag|=DEBUG_TYPE_TACACS_SHOW_AUTHEN;
                    }
                    else if(arg[2][4]=='o' || arg[2][4]=='O')/*authorization*/
                    {
                        flag|=DEBUG_TYPE_TACACS_SHOW_AUTHOR;
                    }
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_TACACS_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_TACACS:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_TACACS, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_TACACS:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_TACACS, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Tacacs*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Telnet
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of telnet
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Telnet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG_TELNET == TRUE)
    UI32_T flag=0;
    BOOL_T show_in_arg0=FALSE;
    UI8_T cli_tid=CLI_TASK_GetMySessId();


    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_TELNET)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_TELNET, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    if(arg[0]==NULL)
    {
        flag|=DEBUG_TYPE_TELNET_ALL;
    }
    else
    {
        switch(arg[0][0])
        {
            case 'a':
            case 'A':
                flag|=DEBUG_TYPE_TELNET_ALL;
                break;

            case 'c':
            case 'C':
                flag|=DEBUG_TYPE_TELNET_CONFIG;
                break;

            case 'd':
            case 'D':
                flag|=DEBUG_TYPE_TELNET_DATABASE;
                break;

            case 'e':
            case 'E':
                flag|=DEBUG_TYPE_TELNET_EVENT;
                break;

            case 'p':
            case 'P':
                flag|=DEBUG_TYPE_TELNET_PACKET;
                break;

            case 's':
            case 'S':
                show_in_arg0=TRUE;
                switch(arg[1][0])
                {
                    case 'a':
                    case 'A':
                        flag|=DEBUG_TYPE_TELNET_SHOW_ALL;
                        break;

                    default:
                        return CLI_ERR_INTERNAL;
                }
                break;

            default:
                return CLI_ERR_INTERNAL;
        }

        if((arg[1]!=NULL)&&(!show_in_arg0))
        {
            switch(arg[2][0])
            {
                case 'a':
                case 'A':
                    flag|=DEBUG_TYPE_TELNET_SHOW_ALL;
                    break;

                default:
                    return CLI_ERR_INTERNAL;
            }
        }
        else if (!show_in_arg0)
        {
            flag|=DEBUG_TYPE_TELNET_SHOW_ALL;
        }
    }/*if(arg[0]==NULL)*/

    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_TELNET:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_TELNET, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_TELNET:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_TELNET, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }

#endif
    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Telnet*/


/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Spanningtree
 *------------------------------------------------------------------------------
 * PURPOSE  : set debug flag of spanning tree
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Spanningtree(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG_XSTP == TRUE)
    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx==PRIVILEGE_EXEC_CMD_W2_DEBUG_SPANNINGTREE)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_XSTP, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }

    switch(arg[0][0])
    {
        case 'a':
        case 'A':
            flag|=DEBUG_TYPE_XSTP_ALL;
            break;

        case 'b':
        case 'B':
            flag|=DEBUG_TYPE_XSTP_BPDU;
            break;

        case 'e':
        case 'E':
            flag|=DEBUG_TYPE_XSTP_EVENTS;
            break;

        case 'r':
        case 'R':
            flag|=DEBUG_TYPE_XSTP_ROOT;
            break;

        default:
                return CLI_ERR_INTERNAL;
    }


    switch( cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W2_DEBUG_SPANNINGTREE:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_XSTP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_SPANNINGTREE:
            if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_XSTP, cli_tid, flag))
            {
                CLI_LIB_PrintStr("Failed to set debug option.\r\n");
            }
            break;

        default:
                return CLI_ERR_INTERNAL;
    }
#endif /* #if (SYS_CPNT_DEBUG_XSTP == TRUE) */

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Debug_Spanningtree*/

UI32_T CLI_API_Debug_Mldsnp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG == TRUE && SYS_CPNT_MLDSNP == TRUE)
  UI32_T flag=DEBUG_TYPE_MLDSNP_NONE;
  UI8_T cli_tid=CLI_TASK_GetMySessId();

  if(cmd_idx == PRIVILEGE_EXEC_CMD_W2_DEBUG_MLDSNP)
  {
      if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_MLDSNP, cli_tid, &flag))
      {
          CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
      }
  }
  switch(arg[0][0])
  {
      case 'r':
      case 'R':
          flag|=DEBUG_TYPE_MLDSNP_DECODE;
          break;

      case 't':
      case 'T':
          flag|=DEBUG_TYPE_MLDSNP_ENCODE;
          break;

      case 'e':
      case 'E':
          flag|=DEBUG_TYPE_MLDSNP_EVENT;
          break;

      case 'a':
      case 'A':
          flag|=DEBUG_TYPE_MLDSNP_ALL;
          break;
  }

  switch(cmd_idx)
  {
    case PRIVILEGE_EXEC_CMD_W2_DEBUG_MLDSNP:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_MLDSNP, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;

    case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_MLDSNP:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_MLDSNP, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;
  }
#endif
  return CLI_NO_ERROR;
}

UI32_T CLI_API_Debug_SyncE(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_SYNCE == TRUE)
  UI32_T flag=DEBUG_TYPE_SYNC_E_NONE;
  UI8_T cli_tid=CLI_TASK_GetMySessId();

  if(cmd_idx == PRIVILEGE_EXEC_CMD_W2_DEBUG_SYNCE)
  {
      if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_SYNC_E, cli_tid, &flag))
      {
          CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
      }
  }
  switch(arg[0][0])
  {
      case 'e':
      case 'E':
          flag|=DEBUG_TYPE_SYNC_E_EVENT;
          break;

      case 'r':
      case 'R':
          flag|=DEBUG_TYPE_SYNC_E_PACKET_RX;
          break;

      case 't':
      case 'T':
          flag|=DEBUG_TYPE_SYNC_E_PACKET_TX;
          break;

      case 'a':
      case 'A':
          flag|=DEBUG_TYPE_SYNC_E_ALL;
          break;
  }

  switch(cmd_idx)
  {
    case PRIVILEGE_EXEC_CMD_W2_DEBUG_SYNCE:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_SYNC_E, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;

    case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_SYNCE:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_SYNC_E, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;
  }
#endif
  return CLI_NO_ERROR;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Debug_Vxlan
 *------------------------------------------------------------------------------
 * PURPOSE  :debug VXLAN {all|database|event|vni|vtep} at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Debug_Vxlan(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if(SYS_CPNT_DEBUG == TRUE && SYS_CPNT_VXLAN == TRUE)
    UI32_T flag=DEBUG_TYPE_SYNC_E_NONE;
    UI8_T cli_tid=CLI_TASK_GetMySessId();

    if(cmd_idx == PRIVILEGE_EXEC_CMD_W2_DEBUG_VXLAN)
    {
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(DEBUG_TYPE_VXLAN, cli_tid, &flag))
        {
            CLI_LIB_PrintStr("Failed to get original debug option.\r\n");
        }
    }
    switch(arg[0][0])
    {
        case 'd':
        case 'D':
            flag|=DEBUG_TYPE_VXLAN_DATABASE;
            break;
        case 'e':
        case 'E':
            flag|=DEBUG_TYPE_VXLAN_EVENT;
            break;

        case 'v':
        case 'V':
            if((arg[0][1]== 'n')||(arg[0][1]=='N'))
                flag|=DEBUG_TYPE_VXLAN_VNI;
            else    
                flag|=DEBUG_TYPE_VXLAN_VTEP;
            break;

        case 'a':
        case 'A':
            flag|=DEBUG_TYPE_VXLAN_ALL;
            break;
  }

  switch(cmd_idx)
  {
    case PRIVILEGE_EXEC_CMD_W2_DEBUG_VXLAN:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_EnableModuleFlag(DEBUG_TYPE_VXLAN, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;

    case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_VXLAN:
        if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_DisableModuleFlag(DEBUG_TYPE_VXLAN, cli_tid, flag))
        {
            CLI_LIB_PrintStr("Failed to set debug option.\r\n");
        }
        break;
  }
#endif
  return CLI_NO_ERROR;
}


#if (SYS_CPNT_DEBUG == TRUE)
#if 0
   static UI32_T show_debug_aaa(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_cluster(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_dot1x(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_portsecurity(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_radius(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_tacacs(UI32_T line_num, UI32_T cli_tid);
#endif
   static UI32_T show_debug_lacp(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_spanningtree(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_arp(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_dhcp(UI32_T line_num, UI32_T cli_tid);
   static UI32_T show_debug_ipdhcpsnp(UI32_T line_num, UI32_T cli_tid);
#if (SYS_CPNT_DEBUG_TELNET == TRUE)
   static UI32_T show_debug_telnet(UI32_T line_num, UI32_T cli_tid);
#endif
#if (SYS_CPNT_SYNCE == TRUE)
static UI32_T show_debug_syncE(UI32_T line_num , UI32_T cli_tid);
#endif
#if(SYS_CPNT_MLDSNP == TRUE)
static UI32_T show_debug_mldsnp(UI32_T line_num , UI32_T cli_tid);
#endif
#if(SYS_CPNT_VXLAN == TRUE)
static UI32_T show_debug_vxlan(UI32_T line_num , UI32_T cli_tid);
#endif
#endif /* #if (SYS_CPNT_DEBUG == TRUE) */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Debug
 *------------------------------------------------------------------------------
 * PURPOSE  :show debug at priviledge
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DEBUG == TRUE)
//    UI32_T flag=0;
    UI8_T cli_tid=CLI_TASK_GetMySessId();
    UI32_T line_num = 0;

#if (SYS_CPNT_CLI_FILTERING == TRUE)
    if(arg[0][0]!=NULL)
    {
        if(arg[0][0]=='|')
        {
            switch(arg[1][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[2]);
        }
    }

    if(arg[1]!=NULL)
    {
        if(arg[1][0]=='|')
        {
            switch(arg[2][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[3]);
        }
    }

    if(arg[2]!=NULL)
    {
        if(arg[2][0]=='|')
        {
            switch(arg[3][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[4]);
        }
    }

    if(arg[3]!=NULL)
    {
        if(arg[3][0]=='|')
        {
            switch(arg[4][0])
            {
                case 'b':
                case 'B':
                    ctrl_P->option_flag=CLI_API_OPTION_BEGIN;
                    break;
                case 'e':
                case 'E':
                    ctrl_P->option_flag=CLI_API_OPTION_EXCLUDE;
                    break;

                case 'i':
                case 'I':
                    ctrl_P->option_flag=CLI_API_OPTION_INCLUDE;
                    break;
                default:
                    return CLI_ERR_INTERNAL;
            }
            strcpy(ctrl_P->option_buf,arg[5]);
        }
    }
#endif /*#if (SYS_CPNT_CLI_FILTERING == TRUE)*/

    if(arg[0]==NULL || arg[0][0]=='|')
    {
        line_num =show_debug_arp(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#if 0
        line_num =show_debug_aaa(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;

        line_num =show_debug_cluster(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#endif
        line_num =show_debug_dhcp(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#if 0
        line_num =show_debug_dot1x(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#endif

        line_num =show_debug_ipdhcpsnp(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#if 0
        line_num =show_debug_portsecurity(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;

        line_num =show_debug_radius(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;

        line_num =show_debug_tacacs(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#endif /* #if 0 */

        line_num =show_debug_lacp(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;

        line_num =show_debug_spanningtree(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;

#if (SYS_CPNT_DEBUG_TELNET == TRUE)
        line_num =show_debug_telnet(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
#endif
		#if(SYS_CPNT_MLDSNP == TRUE)
        line_num =show_debug_mldsnp(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
        #endif

	#if(SYS_CPNT_SYNCE == TRUE)
        line_num =show_debug_syncE(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
        #endif
        #if(SYS_CPNT_VXLAN == TRUE)
        line_num = show_debug_vxlan(line_num,cli_tid);
        if (line_num == JUMP_OUT_MORE)
            return CLI_NO_ERROR;
        else if (line_num == EXIT_SESSION_MORE)
            return CLI_EXIT_SESSION;
        #endif
    } /*if(arg[0]==NULL || arg[0][0]=='|')*/
    else
    {
        switch (arg[0][0])
        {
            case 'a':
            case 'A':
                if(arg[0][1]=='r' || arg[0][1]=='R')
                {
                    line_num =show_debug_arp(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
#if 0                
                else
                {
                    line_num =show_debug_aaa(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
#endif                   
                break;

#if 0

            case 'c':
            case 'C':
                line_num =show_debug_cluster(line_num,cli_tid);
                if (line_num == JUMP_OUT_MORE)
                    return CLI_NO_ERROR;
                else if (line_num == EXIT_SESSION_MORE)
                    return CLI_EXIT_SESSION;
                break;
#endif
            case 'd':
            case 'D':
#if 0                
                if(arg[0][1]=='o' || arg[0][1]=='O')
                {
                    line_num =show_debug_dot1x(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
                else
#endif                    
                {
                    line_num =show_debug_dhcp(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
                break;
            case 'E':
            case 'e':
              {
                break;
             }
            case 'm':
            case 'M':
            #if (SYS_CPNT_MLDSNP == TRUE)
              /*debug mldsnp*/
              if(arg[0][1] == 'l' || arg[0][1] == 'L')
              {
                  line_num =show_debug_mldsnp(line_num,cli_tid);
                  if (line_num == JUMP_OUT_MORE)
                      return CLI_NO_ERROR;
                  else if (line_num == EXIT_SESSION_MORE)
                      return CLI_EXIT_SESSION;
              }
            #endif
              break;
            case 'i':
            case 'I':
              #if 0
            	if(arg[0][1] == 'p' || arg[0][1] == 'P')
            	{
                    line_num =show_debug_ipdhcpsnp(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
            	}
              #endif
                break;

            case 'l':
            case 'L':
                line_num =show_debug_lacp(line_num,cli_tid);
                if (line_num == JUMP_OUT_MORE)
                    return CLI_NO_ERROR;
                else if (line_num == EXIT_SESSION_MORE)
                    return CLI_EXIT_SESSION;
                break;

            case 'p':
            case 'P':
             #if 0
                line_num =show_debug_portsecurity(line_num,cli_tid);
                if (line_num == JUMP_OUT_MORE)
                    return CLI_NO_ERROR;
                else if (line_num == EXIT_SESSION_MORE)
                    return CLI_EXIT_SESSION;
             #endif
                break;
#if 0
            case 'r':
            case 'R':
                line_num =show_debug_radius(line_num,cli_tid);
                if (line_num == JUMP_OUT_MORE)
                    return CLI_NO_ERROR;
                else if (line_num == EXIT_SESSION_MORE)
                    return CLI_EXIT_SESSION;
                break;
#endif
            case 's':
            case 'S':
            #if (SYS_CPNT_SYNCE == TRUE)
                /*synce*/
                if(arg[0][1] == 'y' || arg[0][1] == 'Y')
                {
                    line_num = show_debug_syncE(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
                else
            #endif /*end #if (SYS_CPNT_SYNCE == TRUE)*/
                {
                    line_num = show_debug_spanningtree(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
                break;

            case 't':
            case 'T':
#if 0
                if(arg[0][1]=='e' || arg[0][1]=='E')
#endif
                {
                #if (SYS_CPNT_DEBUG_TELNET == TRUE)
                    line_num =show_debug_telnet(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
		        #endif
                }
#if 0
                else
                {
                    line_num =show_debug_tacacs(line_num,cli_tid);
                    if (line_num == JUMP_OUT_MORE)
                        return CLI_NO_ERROR;
                    else if (line_num == EXIT_SESSION_MORE)
                        return CLI_EXIT_SESSION;
                }
#endif
                break;
            case 'v':
            case 'V':
#if(SYS_CPNT_VXLAN == TRUE)
                line_num =show_debug_vxlan(line_num,cli_tid);
                if (line_num == JUMP_OUT_MORE)
                    return CLI_NO_ERROR;
                else if (line_num == EXIT_SESSION_MORE)
                    return CLI_EXIT_SESSION;
                break;
#endif
            default:
                return CLI_ERR_INTERNAL;

        }
    }
#endif

    return CLI_NO_ERROR;
} /*end-of-CLI_API_Show_Debug*/

/* LOCAL SUBPROGRAM BODIES
*/
#if 0
/* FUNCTION NAME: show_debug_aaa
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_aaa(UI32_T line_num, UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("AAA:\r\n");

    if( cmpcscflag(DEBUG_TYPE_AAA, cli_tid, DEBUG_TYPE_AAA_CONFIG) )
    {
        PROCESS_MORE_FUNC(" AAA config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" AAA config debugging is disable\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_AAA, cli_tid, DEBUG_TYPE_AAA_EVENT) )
    {
        PROCESS_MORE_FUNC(" AAA event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" AAA event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_AAA, cli_tid, DEBUG_TYPE_AAA_DATABASE) )
    {
        PROCESS_MORE_FUNC(" AAA database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" AAA database debugging is disabled\r\n");
    }

    /*show*/
    if( cmpcscflag(DEBUG_TYPE_AAA, cli_tid, DEBUG_TYPE_AAA_SHOW_ACCT) )
    {
        PROCESS_MORE_FUNC(" Show AAA accounting debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show AAA accounting debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_AAA, cli_tid, DEBUG_TYPE_AAA_SHOW_AUTHOR) )
    {
        PROCESS_MORE_FUNC(" Show AAA authorization debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show AAA authorization debugging is disabled\r\n");
    }

    return line_num;
} /*end-of-show_debug_aaa*/

/* FUNCTION NAME: show_debug_cluster
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_cluster(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("CLUSTER:\r\n");

    if( cmpcscflag(DEBUG_TYPE_CLUSTER, cli_tid, DEBUG_TYPE_CLUSTER_PACKET) )
    {
        PROCESS_MORE_FUNC(" CLUSTER packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" CLUSTER packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_CLUSTER, cli_tid, DEBUG_TYPE_CLUSTER_EVENT) )
    {
        PROCESS_MORE_FUNC(" CLUSTER event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" CLUSTER event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_CLUSTER, cli_tid, DEBUG_TYPE_CLUSTER_DATABASE) )
    {
        PROCESS_MORE_FUNC(" CLUSTER database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" CLUSTER database debugging is disabled\r\n");
    }

    return line_num;
} /*end-of-show_debug_cluster*/


/* FUNCTION NAME: show_debug_dot1x
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_dot1x(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("Dot1x:\r\n");

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_CONFIG) )
    {
        PROCESS_MORE_FUNC(" Dot1x config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Dot1x config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_EVENT) )
    {
        PROCESS_MORE_FUNC(" Dot1x event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Dot1x event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_DATABASE) )
    {
        PROCESS_MORE_FUNC(" Dot1x database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Dot1x database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_PACKET) )
    {
        PROCESS_MORE_FUNC(" Dot1x packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Dot1x packet debugging is disabled\r\n");
    }

    /*show*/
    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND|DEBUG_TYPE_DOT1X_SHOW_SM_PAE) )
    {
        PROCESS_MORE_FUNC(" Show dot1x state machine debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show dot1x state machine debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_SHOW_SM_PAE) )
    {
        PROCESS_MORE_FUNC(" Show dot1x PAE state machine debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show dot1x PAE state machine debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DOT1X, cli_tid, DEBUG_TYPE_DOT1X_SHOW_SM_BACKEND) )
    {
        PROCESS_MORE_FUNC(" Show dot1x backend state machine debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show dot1x backend state machine debugging is disabled\r\n");
    }

    return line_num;
} /*end-of-show_debug_dot1x*/
#endif

/* FUNCTION NAME: show_debug_lacp
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_lacp(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_DEBUG_LACP == TRUE)
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("LACP:\r\n");

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_CONFIG) )
    {
        PROCESS_MORE_FUNC(" LACP config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_DATABASE) )
    {
        PROCESS_MORE_FUNC(" LACP database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_EVENT) )
    {
        PROCESS_MORE_FUNC(" LACP event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_PACKET) )
    {
        PROCESS_MORE_FUNC(" LACP packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_PACKET_DETAIL) )
    {
        PROCESS_MORE_FUNC(" LACP packet detail debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP packet detail debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_LACP, cli_tid, DEBUG_TYPE_LACP_STATE_MACHING) )
    {
        PROCESS_MORE_FUNC(" LACP state machine debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" LACP state machine debugging is disabled\r\n");
    }
#endif /* #if (SYS_CPNT_DEBUG_LACP == TRUE) */

    return line_num;
} /*end-of-show_debug_lacp*/

#if 0
/* FUNCTION NAME: show_debug_portsecurity
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_portsecurity(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("Port security:\r\n");

    if( cmpcscflag(DEBUG_TYPE_PSEC, cli_tid, DEBUG_TYPE_PSEC_CONFIG) )
    {
        PROCESS_MORE_FUNC(" Port security config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Port security config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_PSEC, cli_tid, DEBUG_TYPE_PSEC_EVENT) )
    {
        PROCESS_MORE_FUNC(" Port security event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Port security event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_PSEC, cli_tid, DEBUG_TYPE_PSEC_DATABASE) )
    {
        PROCESS_MORE_FUNC(" Port security database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Port security database debugging is disabled\r\n");
}

    return line_num;
} /*end-of-show_debug_portsecurity*/


/* FUNCTION NAME: show_debug_radius
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_radius(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("RADIUS:\r\n");

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_CONFIG) )
    {
        PROCESS_MORE_FUNC(" RADIUS config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" RADIUS config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_EVENT) )
    {
        PROCESS_MORE_FUNC(" RADIUS event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" RADIUS event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_DATABASE) )
    {
        PROCESS_MORE_FUNC(" RADIUS database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" RADIUS database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_PACKET) )
    {
        PROCESS_MORE_FUNC(" RADIUS packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" RADIUS packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_AVPAIR) )
    {
        PROCESS_MORE_FUNC(" RADIUS av-pair debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" RADIUS av-pair debugging is disabled\r\n");
    }

    /*show*/
    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_SHOW_ACCT) )
    {
        PROCESS_MORE_FUNC(" Show RADIUS accounting debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show RADIUS accounting debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_SHOW_AUTHEN) )
    {
        PROCESS_MORE_FUNC(" Show RADIUS authentication debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show RADIUS authentication debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_RADIUS, cli_tid, DEBUG_TYPE_RADIUS_SHOW_AUTHOR) )
    {
        PROCESS_MORE_FUNC(" Show RADIUS authorization debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show RADIUS authorization debugging is disabled\r\n");
    }

    return line_num;
} /*end-of-show_debug_radius*/
#endif /* #if 0 */

/* FUNCTION NAME: show_debug_spanningtree
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_spanningtree(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_DEBUG_XSTP == TRUE)
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("Spanning Tree:\r\n");

    if( cmpcscflag(DEBUG_TYPE_XSTP, cli_tid, DEBUG_TYPE_XSTP_BPDU) )
    {
        PROCESS_MORE_FUNC(" Spanning Tree BPDU Transmitted/Received debug is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Spanning Tree BPDU Transmitted/Received debug is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_XSTP, cli_tid, DEBUG_TYPE_XSTP_EVENTS) )
    {
        PROCESS_MORE_FUNC(" Spanning Tree event debug is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Spanning Tree event debug is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_XSTP, cli_tid, DEBUG_TYPE_XSTP_ROOT) )
    {
        PROCESS_MORE_FUNC(" Spanning Tree root changes debug is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Spanning Tree root changes debug is disabled\r\n");
    }
#endif /* #if (SYS_CPNT_DEBUG_XSTP == TRUE) */

    return line_num;
} /*end-of-show_debug_spanningtree*/

#if 0
/* FUNCTION NAME: show_debug_tacacs
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_tacacs(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_CLI_TERMINAL == TRUE)
    CLI_TASK_WorkingArea_T *ctrl_P = (CLI_TASK_WorkingArea_T *)CLI_TASK_GetMyWorkingArea();
#endif
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("TACACS+:\r\n");

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_CONFIG) )
    {
        PROCESS_MORE_FUNC(" TACACS+ config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" TACACS+ config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_EVENT) )
    {
        PROCESS_MORE_FUNC(" TACACS+ event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" TACACS+ event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_DATABASE) )
    {
        PROCESS_MORE_FUNC(" TACACS+ database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" TACACS+ database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_PACKET) )
    {
        PROCESS_MORE_FUNC(" TACACS+ packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" TACACS+ packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_AVPAIR) )
    {
        PROCESS_MORE_FUNC(" TACACS+ av-pair debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" TACACS+ av-pair debugging is disabled\r\n");
    }

    /*show*/
    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_SHOW_ACCT) )
    {
        PROCESS_MORE_FUNC(" Show TACACS+ accounting debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show TACACS+ accounting debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_SHOW_AUTHEN) )
    {
        PROCESS_MORE_FUNC(" Show TACACS+ authentication debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show TACACS+ authentication debugging is disabled\r\n");
    }
    if( cmpcscflag(DEBUG_TYPE_TACACS, cli_tid, DEBUG_TYPE_TACACS_SHOW_AUTHOR) )
    {
        PROCESS_MORE_FUNC(" Show TACACS+ authorization debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Show TACACS+ authorization debugging is disabled\r\n");
    }

    return line_num;
} /*end-of-show_debug_tacacs*/
#endif

/* FUNCTION NAME: show_debug_telnet
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
#if (SYS_CPNT_DEBUG_TELNET == TRUE)
static UI32_T show_debug_telnet(UI32_T line_num , UI32_T cli_tid)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("Telnet:\r\n");

    if( cmpcscflag(DEBUG_TYPE_TELNET, cli_tid, DEBUG_TYPE_TELNET_CONFIG) )
    {
        PROCESS_MORE_FUNC(" Telnet config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Telnet config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TELNET, cli_tid, DEBUG_TYPE_TELNET_EVENT) )
    {
        PROCESS_MORE_FUNC(" Telnet event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Telnet event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TELNET, cli_tid, DEBUG_TYPE_TELNET_DATABASE) )
    {
        PROCESS_MORE_FUNC(" Telnet database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Telnet database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_TELNET, cli_tid, DEBUG_TYPE_TELNET_PACKET) )
    {
        PROCESS_MORE_FUNC(" Telnet packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" Telnet packet debugging is disabled\r\n");
    }

    return line_num;
}
#endif

/* FUNCTION NAME: show_debug_mldsnp
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
#if(SYS_CPNT_DEBUG == TRUE &&(SYS_CPNT_MLDSNP == TRUE))
static UI32_T show_debug_mldsnp(UI32_T line_num , UI32_T cli_tid)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("MLD snooping:\r\n");

    if( cmpcscflag(DEBUG_TYPE_MLDSNP, cli_tid, DEBUG_TYPE_MLDSNP_DECODE) )
    {
        PROCESS_MORE_FUNC(" MLD snooping receive debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" MLD snooping receive debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_MLDSNP, cli_tid, DEBUG_TYPE_MLDSNP_ENCODE) )
    {
        PROCESS_MORE_FUNC(" MLD snooping transmit debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" MLD snooping transmit debugging is disabled\r\n");
    }
#if 0
    if( cmpcscflag(DEBUG_TYPE_MLDSNP, cli_tid, DEBUG_TYPE_MLDSNP_EVENT) )
    {
        PROCESS_MORE_FUNC(" MLD snooping event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" MLD snooping event debugging is disabled\r\n");
    }
    #endif
    return line_num;
}
#endif

/* FUNCTION NAME: show_debug_syncE
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
#if (SYS_CPNT_DEBUG == TRUE &&(SYS_CPNT_SYNCE == TRUE))
static UI32_T show_debug_syncE(UI32_T line_num , UI32_T cli_tid)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("SyncE:\r\n");

    if( cmpcscflag(DEBUG_TYPE_SYNC_E, cli_tid, DEBUG_TYPE_SYNC_E_EVENT) )
    {
        PROCESS_MORE_FUNC(" SyncE event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" SyncE event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_SYNC_E, cli_tid, DEBUG_TYPE_SYNC_E_PACKET_RX) )
    {
        PROCESS_MORE_FUNC(" SyncE packet receive debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" SyncE packet receive debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_SYNC_E, cli_tid, DEBUG_TYPE_SYNC_E_PACKET_TX) )
    {
        PROCESS_MORE_FUNC(" SyncE packet transmit debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" SyncE packet transmit is disabled\r\n");
    }

    return line_num;

}
#endif

/* FUNCTION NAME: show_debug_arp
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
#if (SYS_CPNT_DEBUG == TRUE)
static UI32_T show_debug_arp(UI32_T line_num , UI32_T cli_tid)
{
#if (SYS_CPNT_DEBUG_ARP == TRUE)
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("ARP:\r\n");

    if( cmpcscflag(DEBUG_TYPE_ARP, cli_tid, DEBUG_TYPE_ARP_PACKETS) )
    {
        PROCESS_MORE_FUNC(" ARP packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" ARP packet debugging is disabled\r\n");
    }
#endif
    return line_num;
} /*end-of-show_debug_arp*/
#endif



/* FUNCTION NAME: show_debug_dhcp
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_dhcp(UI32_T line_num , UI32_T cli_tid)
{
#if(SYS_CPNT_DEBUG_DHCP == TRUE)
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("DHCP:\r\n");

    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_CONFIG) )
    {
        PROCESS_MORE_FUNC(" DHCP config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_EVENT) )
    {
        PROCESS_MORE_FUNC(" DHCP event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_DATABASE) )
    {
        PROCESS_MORE_FUNC(" DHCP database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_PACKET) )
    {
        PROCESS_MORE_FUNC(" DHCP packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_CLIENT) )
    {
        PROCESS_MORE_FUNC(" DHCP client debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP client debugging is disabled\r\n");
    }

#if (SYS_CPNT_DHCP_RELAY == TRUE)
    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_RELAY) )
    {
        PROCESS_MORE_FUNC(" DHCP relay debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP relay debugging is disabled\r\n");
    }
#endif

#if (SYS_CPNT_DHCP_SERVER == TRUE)
    if( cmpcscflag(DEBUG_TYPE_DHCP, cli_tid, DEBUG_TYPE_DHCP_SERVER) )
    {
        PROCESS_MORE_FUNC(" DHCP server debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP server debugging is disabled\r\n");
    }
#endif
#endif
    return line_num;
} /*end-of-show_debug_dhcp*/



/* FUNCTION NAME: show_debug_ipdhcpsnp
 * PURPOSE:
 * INPUT  : flag -- determined flag value.
 *        : cli_tid -- session-id of this task.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
static UI32_T show_debug_ipdhcpsnp(UI32_T line_num , UI32_T cli_tid)
{
#if(SYS_CPNT_DEBUG_DHCPSNP == TRUE)
	char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("DHCP snooping:\r\n");

    if( cmpcscflag(DEBUG_TYPE_DHCPSNP, cli_tid, DEBUG_TYPE_DHCPSNP_CONFIG) )
    {
        PROCESS_MORE_FUNC(" DHCP snooping config debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP snooping config debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCPSNP, cli_tid, DEBUG_TYPE_DHCPSNP_EVENT) )
    {
        PROCESS_MORE_FUNC(" DHCP snooping event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP snooping event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCPSNP, cli_tid, DEBUG_TYPE_DHCPSNP_PACKET) )
    {
        PROCESS_MORE_FUNC(" DHCP snooping packet debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP snooping packet debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_DHCPSNP, cli_tid, DEBUG_TYPE_DHCPSNP_DATABASE) )
    {
        PROCESS_MORE_FUNC(" DHCP snooping database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" DHCP snooping database debugging is disabled\r\n");
    }
#endif    
    return line_num;
} /*end-of-show_debug_ipdhcpsnp*/

#if(SYS_CPNT_VXLAN == TRUE)
static UI32_T show_debug_vxlan(UI32_T line_num , UI32_T cli_tid)
{
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};

    PROCESS_MORE_FUNC("VXLAN:\r\n");

    if( cmpcscflag(DEBUG_TYPE_VXLAN, cli_tid, DEBUG_TYPE_VXLAN_EVENT) )
    {
        PROCESS_MORE_FUNC(" VXLAN event debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" VXLAN event debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_VXLAN, cli_tid, DEBUG_TYPE_VXLAN_DATABASE) )
    {
        PROCESS_MORE_FUNC(" VXLAN database debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" VXLAN database debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_VXLAN, cli_tid, DEBUG_TYPE_VXLAN_VNI) )
    {
        PROCESS_MORE_FUNC(" VXLAN VNI debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" VXLAN VNI debugging is disabled\r\n");
    }

    if( cmpcscflag(DEBUG_TYPE_VXLAN, cli_tid, DEBUG_TYPE_VXLAN_VTEP) )
    {
        PROCESS_MORE_FUNC(" VXLAN VTEP debugging is enabled\r\n");
    }
    else
    {
        PROCESS_MORE_FUNC(" VXLAN VTEP debugging is disabled\r\n");
    }
    return line_num;
}
#endif

/* FUNCTION NAME: cmpcscflag
 * PURPOSE: compare flag by csc.
 * INPUT  : module_id  -- setting CSC name.
 *          session_id -- which session would be printed out.
 *          flag -- determined flag value.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 */
#if (SYS_CPNT_DEBUG == TRUE)
static BOOL_T cmpcscflag(UI32_T module_id, UI32_T session_id ,UI32_T flag)
{
    UI32_T tmp_flag=0;

    if(DEBUG_TYPE_RETURN_OK!=DEBUG_MGR_GetModuleFlag(module_id, session_id, &tmp_flag))
    {
        CLI_LIB_PrintStr("Failed to get debug option.\r\n");
        return FALSE;
    }

    if ( tmp_flag == 0x00000000 ) /*match none*/
    {
        return FALSE;
    }

    if(flag == (tmp_flag&flag)) /*match all*/
    {
        return TRUE;
    }

    return FALSE;
} /*end-of-cmpcscflag*/
#endif

