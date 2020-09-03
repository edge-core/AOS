
#include "cli_api.h"
#include "sys_adpt.h"
#if (SYS_CPNT_VRRP == TRUE)
#include "vrrp_type.h"
#include "vrrp_sys_adpt.h"
#include "vrrp_pmgr.h"
#include "vrrp_pom.h"
#include "vrrp_mgr.h"
#include "cli_api_vrrp.h"
#endif


#if (SYS_CPNT_VRRP == TRUE)
static UI32_T CLI_API_Vrrp_Authentication(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
static UI32_T CLI_API_Vrrp_Timer_Advertise(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
static UI32_T CLI_API_Vrrp_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
static UI32_T CLI_API_Vrrp_Preempt(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
static UI32_T CLI_API_Vrrp_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P);
static UI32_T Show_Vrrp_One_Interface(VRRP_OPER_ENTRY_T vrrp_oper_entry, UI32_T line_num);
static UI32_T Show_Vrrp_One_Interface_Brief(VRRP_OPER_ENTRY_T vrrp_oper_entry, UI32_T line_num);
static UI32_T Show_Vrrp_Global(UI32_T line_num);
#endif

UI32_T CLI_API_Vrrp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T retval = CLI_NO_ERROR;
    
#if (SYS_CPNT_VRRP == TRUE)
    
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    
    memset(&vrrp_oper_entry, 0, sizeof(VRRP_OPER_ENTRY_T));
    
    if (arg[0] == NULL)
        return CLI_ERR_INTERNAL;
    if (arg[1] != NULL)
    {
        switch (arg[1][0])
        {
            case 'a':
            case 'A':
                retval = CLI_API_Vrrp_Authentication(cmd_idx, arg, ctrl_P);
                break;
            case 'i':
            case 'I':
                retval = CLI_API_Vrrp_Ip_Address(cmd_idx, arg, ctrl_P);
                break;
                
            case 'p':
            case 'P':
                if ((arg[1][2] == 'e') || (arg[1][2] == 'E'))
                {
                    retval = CLI_API_Vrrp_Preempt(cmd_idx, arg, ctrl_P);
                }
                else if ((arg[1][2] == 'i') || (arg[1][2] == 'I'))
                {
                    retval = CLI_API_Vrrp_Priority(cmd_idx, arg, ctrl_P);
                }
                else
                {
                    retval = CLI_ERR_INTERNAL;
                }
                break;
            case 't':
            case 'T':
                retval = CLI_API_Vrrp_Timer_Advertise(cmd_idx, arg, ctrl_P);
                break;
            default: /*[no] vrrp group-id*/
                break;
        }
    }
    else
    {
        vrrp_oper_entry.ifindex = ctrl_P->CMenu.vlan_ifindex;
        vrrp_oper_entry.vrid = atoi(arg[0]);
        VRRP_PMGR_GetDefaultVrrpOperEntry(&vrrp_oper_entry);
        
        if (cmd_idx == PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP)
        {
            vrrp_oper_entry.row_status = VAL_vrrpOperRowStatus_createAndWait;
            if (VRRP_TYPE_OK != VRRP_PMGR_SetVrrpOperEntry(&vrrp_oper_entry))
            {
                CLI_LIB_PrintStr("Failed to create this VRRP entry.\r\n");
            }
        }
        else if (cmd_idx == PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP)
        {
            if (VRRP_TYPE_OK != VRRP_PMGR_DeleteVrrpOperEntry(&vrrp_oper_entry))
            {
                CLI_LIB_PrintStr("Failed to delete this VRRP entry.\r\n");
            }
        }
        else
        {
            retval = CLI_ERR_INTERNAL;
        }
    }
    
#endif /*#if (SYS_CPNT_VRRP == TRUE)*/
    
    return retval;
}

UI32_T CLI_API_Vrrp_PingEnable(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VRRP_PING == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_VRRP_PINGENABLE:
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPingStatus(VRRP_TYPE_PING_STATUS_ENABLE))
                CLI_LIB_PrintStr("Failed to set ping status.\r\n");
            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_VRRP_PINGENABLE:
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPingStatus(VRRP_TYPE_PING_STATUS_DISABLE))
                CLI_LIB_PrintStr("Failed to set ping status.\r\n");
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_VRRP == TRUE)
static UI32_T CLI_API_Vrrp_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex;
    UI32_T group_id;
    UI32_T set_status, ori_status, result;
    UI32_T change_flag = 0;
    UI32_T ipAddress = {0};
    UI32_T row_status;
    
    ifindex = ctrl_P->CMenu.vlan_ifindex;
    group_id = atoi(arg[0]);
    CLI_LIB_AtoIp(arg[2], (UI8_T*)&ipAddress);
    
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP:
            row_status = VAL_vrrpAssoIpAddrRowStatus_createAndGo;
            if (VRRP_TYPE_OK != VRRP_PMGR_SetIpAddress(ifindex, (UI8_T)group_id, (UI8_T*)&ipAddress, row_status))
                CLI_LIB_PrintStr("Failed to set VRRP.\r\n");
            else
            {
                if (VRRP_TYPE_OK != VRRP_PMGR_GetOperAdminStatus(ifindex, (UI8_T)group_id, &ori_status))
                    CLI_LIB_PrintStr("Failed to set VRRP.\r\n");
                    
                if (ori_status != VAL_vrrpOperAdminState_up)
                {
                    set_status = VAL_vrrpOperAdminState_up;
                    change_flag = 1;
                }
            }
            break;
            
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP:
            row_status = VAL_vrrpAssoIpAddrRowStatus_notInService;
            if ((group_id < SYS_ADPT_MIN_VRRP_ID) || (group_id > SYS_ADPT_MAX_VRRP_ID))
            {
                CLI_LIB_PrintStr("The parameters do not match. The deletion failed.\r\n");
                return CLI_NO_ERROR;
            }
            
            result = VRRP_PMGR_SetIpAddress(ifindex, (UI8_T)group_id, (UI8_T*) & ipAddress, row_status);
            if (result != VRRP_TYPE_OK)
                CLI_LIB_PrintStr("Failed to set VRRP.\r\n");
            else if (result == VRRP_TYPE_PARAMETER_ERROR)
                CLI_LIB_PrintStr("The parameters do not match. The deletion failed.\r\n");
                
#if 0
            if (!VRRP_PMGR_GetOperAdminStatus(ifindex, group_id, &ori_status))
                CLI_LIB_PrintStr("The no VRRP IP operation is failed\r\n");
                
            if (ori_status != VAL_vrrpOperAdminState_down)
            {
                set_status = VAL_vrrpOperAdminState_down;
                change_flag = 1;
            }
#endif
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }
    
    if (change_flag)
    {
        if (VRRP_TYPE_OK != VRRP_PMGR_SetOperAdminStatus(ifindex, (UI8_T)group_id, set_status))
            CLI_LIB_PrintStr("Failed to set VRRP.\r\n");
    }
    
    return CLI_NO_ERROR;
}

static UI32_T CLI_API_Vrrp_Preempt(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex;
    UI8_T  group_id;
    UI32_T delay_time = 0;
    UI32_T preempt_mode;
    
    ifindex = ctrl_P->CMenu.vlan_ifindex;
    group_id = atoi(arg[0]);
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP:
        
            if (arg[3])
            {
                delay_time = atoi(arg[3]);
            }
            else
            {
                delay_time = 0;
            }
            preempt_mode = VAL_vrrpOperPreemptMode_true;
            if ((delay_time < SYS_ADPT_MIN_VRRP_PREEMPT_DELAY) || (delay_time > SYS_ADPT_MAX_VRRP_PREEMPT_DELAY))
            {
                CLI_LIB_PrintStr("The delay time is out of range. The setting failed.\r\n");
                return CLI_NO_ERROR;
            }
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPreemptionMode(ifindex, group_id, preempt_mode, delay_time))
                CLI_LIB_PrintStr("Failed to configure the VRRP Preemption mode.\r\n");
                
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP:
            preempt_mode = VAL_vrrpOperPreemptMode_false;
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPreemptionMode(ifindex, group_id, preempt_mode, SYS_ADPT_VRRP_DEFL_DELAY))
                CLI_LIB_PrintStr("Failed to disable the VRRP Preemption mode.\r\n");
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
    
    return CLI_NO_ERROR;
}

static UI32_T CLI_API_Vrrp_Priority(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex;
    UI8_T  group_id;
    UI32_T priority_level;
    
    ifindex = ctrl_P->CMenu.vlan_ifindex;
    group_id = atoi(arg[0]);
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP:
            priority_level = atoi(arg[2]);
            if ((priority_level < SYS_ADPT_MIN_VRRP_PRIORITY) ||
                    (priority_level > SYS_ADPT_MAX_VRRP_PRIORITY))
            {
                CLI_LIB_PrintStr("The priority is out of range. The setting failed.\r\n");
                return CLI_NO_ERROR;
            }
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPriority(ifindex, group_id, priority_level))
                CLI_LIB_PrintStr("Failed to configure the VRRP priority.\r\n");
            break;
            
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP:
            if (VRRP_TYPE_OK != VRRP_PMGR_SetPriority(ifindex, group_id, SYS_ADPT_VRRP_DEFL_PRIORITY))
                CLI_LIB_PrintStr("Failed to restore the VRRP priority.\r\n");
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }
    
    return CLI_NO_ERROR;
}

static UI32_T CLI_API_Vrrp_Timer_Advertise(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex;
    UI8_T  group_id;
    UI32_T timer_interval;
    
    ifindex = ctrl_P->CMenu.vlan_ifindex;
    group_id = atoi(arg[0]);
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP:
            timer_interval = atoi(arg[3]);
            if ((timer_interval < SYS_ADPT_MIN_VRRP_ADVER_INTERVAL) || (timer_interval > SYS_ADPT_MAX_VRRP_ADVER_INTERVAL))
            {
                CLI_LIB_PrintStr("The advertisement interval is out of range. The setting failed.\r\n");
                return CLI_NO_ERROR;
            }
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetAdvertisementInterval(ifindex, group_id, timer_interval))
                CLI_LIB_PrintStr("Failed to configure the VRRP advertisement interval.\r\n");
                
            break;
            
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP:
        
            if (VRRP_TYPE_OK != VRRP_PMGR_SetAdvertisementInterval(ifindex, group_id, SYS_ADPT_VRRP_DEFL_ADVER_INTERVAL))
                CLI_LIB_PrintStr("Failed to restore the VRRP advertisement interval.\r\n");
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }
    
    return CLI_NO_ERROR;
}

static UI32_T CLI_API_Vrrp_Authentication(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ifindex;
    UI8_T  group_id;
    UI32_T authen_type;
    UI8_T  authen_string[VRRP_MGR_AUTH_KEY_LENGTH + 1] = {0};
    
    ifindex = ctrl_P->CMenu.vlan_ifindex;
    group_id = atoi(arg[0]);
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W1_VRRP:
            authen_type = VAL_vrrpOperAuthType_simpleTextPassword;
            
            if ((strlen(arg[2]) > VRRP_MGR_AUTH_KEY_LENGTH) || (strlen(arg[2]) <= 0))
            {
                CLI_LIB_PrintStr("Failed to configure the VRRP authentication.\r\n");
                return CLI_NO_ERROR;
            }
            
            strcpy((char *)authen_string, arg[2]);
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetAuthentication(ifindex, group_id, authen_type, authen_string))
                CLI_LIB_PrintStr("Failed to configure the VRRP authentication.\r\n");
            break;
            
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_NO_VRRP:
            
            authen_type = VAL_vrrpOperAuthType_noAuthentication;
            memset(authen_string, 0, sizeof(authen_string));
            
            if (VRRP_TYPE_OK != VRRP_PMGR_SetAuthentication(ifindex, group_id, authen_type, authen_string))
                CLI_LIB_PrintStr("Failed to disable the VRRP authentication.\r\n");
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }
    
    return CLI_NO_ERROR;
}
#endif /*#if (SYS_CPNT_VRRP == TRUE)*/

UI32_T CLI_API_Show_Vrrp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_VRRP == TRUE)
    VRRP_OPER_ENTRY_T vrrp_oper_entry;
    UI32_T line_num = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T vid, result;
    UI32_T get_flag = 0;
    
    switch (cmd_idx)
    {
        case PRIVILEGE_EXEC_CMD_W4_SHOW_VRRP_INTERFACE_VLAN:
            /*show vrrp interface*/
            vrrp_oper_entry.ifindex = 0;
            vrrp_oper_entry.vrid = 0;
            
            if (arg[1] != NULL)
            {
                if ((arg[1][0] == 'b') || (arg[1][0] == 'B'))
                {
                    /*show vrrp interface brief*/
                    snprintf(buff, sizeof(buff), "Interface   Grp    State    Virtual Addr     Interval   Preempt  Priority\r\n");
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), "---------- ----- -------- ---------------- ----------- --------- --------\r\n");
                    PROCESS_MORE(buff);
                    /*line_num += 2;*/
                    
                    while (VRRP_TYPE_OK == VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry))
                    {
                        VLAN_OM_ConvertFromIfindex(vrrp_oper_entry.ifindex, &vid);
                        if (vid == atoi(arg[0]))
                        {
                            if ((line_num = Show_Vrrp_One_Interface_Brief(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                            {
                                return CLI_NO_ERROR;
                            }
                        }
                    }
                }
            }
            else
            {
                /*show vrrp interface*/
                while (VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_OK)
                {
                    VLAN_OM_ConvertFromIfindex(vrrp_oper_entry.ifindex, &vid);
                    if (vid == atoi(arg[0]))
                    {
                        if ((line_num = Show_Vrrp_One_Interface(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                        {
                            return CLI_NO_ERROR;
                        }
                    }
                }
            }
            return CLI_NO_ERROR;
            break;
        case PRIVILEGE_EXEC_CMD_W4_SHOW_VRRP_ROUTER_COUNTERS:
            {
                VRRP_OM_Router_Statistics_Info_T vrrp_router_statistics;
                
                /*show vrrp router statistics*/
                if (VRRP_TYPE_OK != VRRP_PMGR_GetVrrpSysStatistics(&vrrp_router_statistics))
                {
                    CLI_LIB_PrintStr("Failed to get the VRRP router statistics.\r\n");
                    return CLI_NO_ERROR;
                }
                
                snprintf(buff, sizeof(buff), " Total Number of VRRP Packets with Invalid Checksum : %lu\r\n", (unsigned long)vrrp_router_statistics.vrrpRouterChecksumErrors);
                PROCESS_MORE(buff);
                snprintf(buff, sizeof(buff), " Total Number of VRRP Packets with Unknown Error    : %lu\r\n", (unsigned long)vrrp_router_statistics.vrrpRouterVersionErrors);
                PROCESS_MORE(buff);
                snprintf(buff, sizeof(buff), " Total Number of VRRP Packets with Invalid VRID     : %lu\r\n", (unsigned long)vrrp_router_statistics.vrrpRouterVrIdErrors);
                PROCESS_MORE(buff);
            }
            return CLI_NO_ERROR;
            break;
        default:
            break;
    }
    
    if (arg[0] == NULL)
    {
        /*show vrrp*/
        if ((line_num = Show_Vrrp_Global(line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        
        vrrp_oper_entry.ifindex = 0;
        vrrp_oper_entry.vrid = 0;
        
        while ((result = VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry)) == VRRP_TYPE_OK)
        {
            if ((line_num = Show_Vrrp_One_Interface(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }            
        }
        return CLI_NO_ERROR;
    }
    else if (arg[0][0] == 'a' || arg[0][0] == 'A')
    {
        /*show vrrp all [interface vlan vlan-id]*/
        vrrp_oper_entry.ifindex = 0;
        vrrp_oper_entry.vrid = 0;
        
        if (arg[1] == NULL) /*show vrrp all*/
        {
            while ((result = VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry)) == VRRP_TYPE_OK)
            {
                if ((line_num = Show_Vrrp_One_Interface(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }
            }
        }
        else /*show vrrp all interface vlan vlan-id*/
        {
            while (VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry) == VRRP_TYPE_OK)
            {
                VLAN_OM_ConvertFromIfindex(vrrp_oper_entry.ifindex, &vid);
                if (vid == atoi(arg[3]))
                {
                    if ((line_num = Show_Vrrp_One_Interface(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                    {
                        return CLI_NO_ERROR;
                    }
                }
            }
        }
        
        return CLI_NO_ERROR;
    }
    
    
    switch (arg[0][0])
    {
        case 'b':
        case 'B':
            /*show vrrp brief*/
            vrrp_oper_entry.ifindex = 0;
            vrrp_oper_entry.vrid = 0;
            
            snprintf(buff, sizeof(buff), "Interface   Grp    State    Virtual Addr     Interval   Preempt  Priority\r\n");
            PROCESS_MORE(buff);
            snprintf(buff, sizeof(buff), "---------- ----- -------- ---------------- ----------- --------- --------\r\n");
            PROCESS_MORE(buff);
            
            while ((result = VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry)) == VRRP_TYPE_OK)
            {
                if ((line_num = Show_Vrrp_One_Interface_Brief(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                {
                    return CLI_NO_ERROR;
                }             
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            /*show vrrp group-id*/
            if ((atoi(arg[0]) < SYS_ADPT_MIN_VRRP_ID) || (atoi(arg[0]) > SYS_ADPT_MAX_VRRP_ID))
                return CLI_ERR_CMD_INVALID_RANGE;
            if (arg[1] != NULL)
            {
                if ((arg[1][0] == 'i') || (arg[1][0] == 'I'))
                {
                    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
                    UI32_T if_index = 0;
                    VRRP_OM_Vrrp_Statistics_Info_T vrrp_group_statistics;
                    UI32_T result;
                    
                    /*show vrrp group-id statistics*/
                    VLAN_OM_ConvertToIfindex(atoi(arg[3]), &if_index);
                    result = VRRP_PMGR_GetVrrpGroupStatistics(if_index , atoi(arg[0]), &vrrp_group_statistics);
                    if (result == VRRP_TYPE_PARAMETER_ERROR)
                    {
                        CLI_LIB_PrintStr("The VRRP group-id statistics get failed.\r\n");
                        return CLI_NO_ERROR;
                    }
                    else if (result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
                    {
                        CLI_LIB_PrintStr("The specified vrrp group does not exist.\r\n");
                        return CLI_NO_ERROR;
                    }
                    
                    
                    snprintf(buff, sizeof(buff), " Total Number of Times Transitioned to MASTER                       : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsBecomeMaster);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Advertisements Packets                    : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsAdvertiseRcvd);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Error Advertisement Interval Packets      : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsAdvertiseIntervalErrors);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Authentication Failures Packets           : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsAuthFailures);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Error IP TTL VRRP Packets                 : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsIpTtlErrors);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Priority 0 VRRP Packets                   : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsPriorityZeroPktsRcvd);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Sent Priority 0 VRRP Packets                       : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsPriorityZeroPktsSent);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Invalid Type VRRP Packets                 : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsInvalidTypePktsRcvd);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Error Address List VRRP Packets           : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsAddressListErrors);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Invalid Authentication Type VRRP Packets  : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsInvalidAuthType);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Mismatch Authentication Type VRRP Packets : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsAuthTypeMismatch);
                    PROCESS_MORE(buff);
                    snprintf(buff, sizeof(buff), " Total Number of Received Error Packet Length VRRP Packets          : %lu\r\n", (unsigned long)vrrp_group_statistics.vrrpStatsPacketLengthErrors);
                    PROCESS_MORE(buff);
                }
            }
            else
            {
                /*show vrrp group-id*/
                vrrp_oper_entry.ifindex = 0;
                vrrp_oper_entry.vrid = 0;
                
                while ((result = VRRP_PMGR_GetNextVrrpOperEntry(&vrrp_oper_entry)) == VRRP_TYPE_OK)
                {
                    if ((vrrp_oper_entry.ifindex == 0) ||
                            ((result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST) && (vrrp_oper_entry.vrid != atoi(arg[0])) &&
                             (!get_flag)))
                    {
                        snprintf(buff, sizeof(buff), " The specified vrrp group does not exist.\r\n");
                        PROCESS_MORE(buff);
                        return CLI_NO_ERROR;
                    }
                    else if ((result == VRRP_TYPE_OPER_ENTRY_NOT_EXIST) && (get_flag == 1))
                    {
                        return CLI_NO_ERROR;
                    }
                    if (vrrp_oper_entry.vrid == atoi(arg[0]))
                    {
                        if (result != VRRP_TYPE_OPER_ENTRY_NOT_EXIST)
                        {
                            if ((line_num = Show_Vrrp_One_Interface(vrrp_oper_entry, line_num)) == JUMP_OUT_MORE)
                            {
                                return CLI_NO_ERROR;
                            }

                            if (!get_flag)
                                get_flag = 1;
                         
                        }
                        else
                            return CLI_NO_ERROR;
                    }
                }
            }
            break;
        default:
            return CLI_ERR_INTERNAL;
    }
#endif /* (SYS_CPNT_VRRP == TRUE) */
    
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_VRRP == TRUE)
static UI32_T Show_Vrrp_One_Interface(VRRP_OPER_ENTRY_T vrrp_oper_entry, UI32_T line_num)
{
    char buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  ip_addr[4] = {0, 0, 0, 0};
    UI32_T vid;
    char *str[] = {"",/*0*/
                   "Initial",
                   "Backup",
                   "Master"
                  };
    char *str1[] = {"",/*0*/
                    "No authentication",
                    "SimpleText",
                    "IpAuthenticationHeader"
                   };
    
    VLAN_OM_ConvertFromIfindex(vrrp_oper_entry.ifindex, &vid);
    snprintf(buff, sizeof(buff), " VLAN %lu - Group %d\r\n", (unsigned long)vid, vrrp_oper_entry.vrid);
    PROCESS_MORE_FUNC(buff);
    
    snprintf(buff, sizeof(buff), " State                            %s\r\n", str[vrrp_oper_entry.oper_state]);
    PROCESS_MORE_FUNC(buff);

    if(VRRP_TYPE_OK == VRRP_PMGR_GetNextIpAddress(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid, ip_addr))
    {
        snprintf(buff, sizeof(buff), " Virtual IP Address               %u.%u.%u.%u\r\n", 
            ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
        PROCESS_MORE_FUNC(buff);

        while(VRRP_TYPE_OK == VRRP_PMGR_GetNextIpAddress(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid, ip_addr))
        {
            snprintf(buff, sizeof(buff), "                                  %u.%u.%u.%u\r\n",
                ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
            PROCESS_MORE_FUNC(buff);
        }
    }
    else
    {
        snprintf(buff, sizeof(buff), " Virtual IP Address\r\n");
        PROCESS_MORE_FUNC(buff);
    }

    snprintf(buff, sizeof(buff), " Virtual MAC Address              %02X-%02X-%02X-%02X-%02X-%02X\r\n", vrrp_oper_entry.virtual_mac[0],
            vrrp_oper_entry.virtual_mac[1], vrrp_oper_entry.virtual_mac[2], vrrp_oper_entry.virtual_mac[3],
            vrrp_oper_entry.virtual_mac[4], vrrp_oper_entry.virtual_mac[5]);
    PROCESS_MORE_FUNC(buff);

    snprintf(buff, sizeof(buff), " Advertisement Interval           %lu seconds\r\n", (unsigned long)vrrp_oper_entry.advertise_interval);
    PROCESS_MORE_FUNC(buff);
    
    if (vrrp_oper_entry.preempt_mode == VAL_vrrpOperPreemptMode_true)
    {
        snprintf(buff, sizeof(buff), " Preemption                       Enabled\r\n");
        PROCESS_MORE_FUNC(buff);
    }
    else if (vrrp_oper_entry.preempt_mode == VAL_vrrpOperPreemptMode_false)
    {
        snprintf(buff, sizeof(buff), " Preemption                       Disabled\r\n");
        PROCESS_MORE_FUNC(buff);
    }
    
    snprintf(buff, sizeof(buff), " Min Delay                        %lu seconds\r\n", (unsigned long)vrrp_oper_entry.preempt_delay);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Priority                         %lu\r\n", (unsigned long)vrrp_oper_entry.priority);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Authentication                   %s\r\n", str1[vrrp_oper_entry.auth_type]);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Authentication Key               %s\r\n", vrrp_oper_entry.auth_key);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Master Router                    %d.%d.%d.%d\r\n", vrrp_oper_entry.master_ip_addr[0],
            vrrp_oper_entry.master_ip_addr[1], vrrp_oper_entry.master_ip_addr[2], vrrp_oper_entry.master_ip_addr[3]);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Master Priority                  %lu\r\n", (unsigned long)vrrp_oper_entry.master_priority);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Master Advertisement Interval    %lu seconds\r\n", (unsigned long)vrrp_oper_entry.master_advertise_int);
    PROCESS_MORE_FUNC(buff);
    snprintf(buff, sizeof(buff), " Master Down Interval             %lu\r\n", (unsigned long)vrrp_oper_entry.master_down_int);
    PROCESS_MORE_FUNC(buff);
    
    return line_num;
}

static UI32_T Show_Vrrp_One_Interface_Brief(VRRP_OPER_ENTRY_T vrrp_oper_entry, UI32_T line_num)
{
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  tempbuff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI8_T  ip_addr[4] = {0, 0, 0, 0};
    I32_T  first = 1;
    UI32_T vid;
    char *str[] = {"",/*0*/
                   "Initial",
                   "Backup ",
                   "Master "
                  };
                  
    if (VRRP_TYPE_OK == VRRP_PMGR_GetNextIpAddress(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid, ip_addr))
    {
        snprintf((char *)tempbuff, sizeof(tempbuff), "%d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
        first = 0;
    }
    else
    {
        snprintf((char *)tempbuff, sizeof(tempbuff), "               ");
        first = -1;
    }
    
    VLAN_OM_ConvertFromIfindex(vrrp_oper_entry.ifindex, &vid);
    if (vrrp_oper_entry.preempt_mode == VAL_vrrpOperPreemptMode_true)
    {
        snprintf(buff, sizeof(buff), "VLAN %-4lu  %5d  %-7s  %15s  %10lu  E        %7lu\r\n", (unsigned long)vid, vrrp_oper_entry.vrid,
                str[vrrp_oper_entry.oper_state], tempbuff, (unsigned long)vrrp_oper_entry.advertise_interval, (unsigned long)vrrp_oper_entry.priority);
        PROCESS_MORE_FUNC(buff);
    }
    else if (vrrp_oper_entry.preempt_mode == VAL_vrrpOperPreemptMode_false)
    {
        snprintf(buff, sizeof(buff), "VLAN %-4lu  %5d  %-7s  %15s  %10lu  D        %7lu\r\n", (unsigned long)vid, vrrp_oper_entry.vrid,
                str[vrrp_oper_entry.oper_state], tempbuff, (unsigned long)vrrp_oper_entry.advertise_interval, (unsigned long)vrrp_oper_entry.priority);
        PROCESS_MORE_FUNC(buff);
    }
    
    if (first == 0)
    {
        while (VRRP_TYPE_OK == VRRP_PMGR_GetNextIpAddress(vrrp_oper_entry.ifindex, vrrp_oper_entry.vrid, ip_addr))
        {
            snprintf((char *)tempbuff, sizeof(tempbuff), "                            %d.%d.%d.%d\r\n", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);
            PROCESS_MORE_FUNC((char *)tempbuff);
        }
    }
    
    return line_num;
}

static UI32_T Show_Vrrp_Global(UI32_T line_num)
{
#if (SYS_CPNT_VRRP_PING == TRUE)
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    
    UI32_T ping_status;
    if (VRRP_TYPE_OK != VRRP_POM_GetPingStatus(&ping_status))
        return line_num;
        
    switch (ping_status)
    {
        case VRRP_TYPE_PING_STATUS_ENABLE:
            snprintf(buff, sizeof(buff), "Ping Status: enabled\r\n");
            break;
        case VRRP_TYPE_PING_STATUS_DISABLE:
            snprintf(buff, sizeof(buff), "Ping Status: disabled\r\n");
            break;
        default:
            return line_num;
    }
    
    PROCESS_MORE_FUNC(buff);
#endif
    return line_num;
}
#endif /* (SYS_CPNT_VRRP == TRUE) */


