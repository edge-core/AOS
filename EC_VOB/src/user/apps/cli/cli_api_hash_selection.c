/*system*/
#include "sysfun.h"
#include <string.h>
#include <stdlib.h>
#include <sys_type.h>
#include "sys_cpnt.h"
#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include "sys_dflt.h"
#include "l_inet.h"
/*cli internal*/
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
#include "cli_msg.h"
#include "sys_adpt.h"
#include "cli_tbl.h"

#if (SYS_CPNT_HASH_SELECTION == TRUE)
static BOOL_T CLI_API_LocalShowOneHashSelection(UI8_T hash_list);
#endif

UI32_T CLI_API_HashSelection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_HASHSELECTION_LIST:
            ctrl_P->CMenu.list_index = atoi(arg[0]);
            if (arg[1][0] == 'm')
            {
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_HASH_SELECTION_MAC_MODE;
            }
            else if (arg[1][0] == 'i' && arg[1][3] == '4')
            {
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_HASH_SELECTION_IPV4_MODE;
            }
            else if (arg[1][0] == 'i' && arg[1][3] == '6')
            {
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_HASH_SELECTION_IPV6_MODE;
            }                               
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_HashSelection_MAC(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_OM_HashSelection_T  selection;

    selection.type = SWCTRL_OM_HASH_PACKET_TYPE_L2;
    selection.sel.l2.arg.value = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_HASH_SELECTION_MAC_CMD_W1_DSTMAC:
            selection.sel.l2.arg.item.dst_mac = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_MAC_CMD_W1_SRCMAC:
            selection.sel.l2.arg.item.src_mac = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;
            
        case PRIVILEGE_CFG_HASH_SELECTION_MAC_CMD_W1_ETHERTYPE:
            selection.sel.l2.arg.item.ether_type = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_MAC_CMD_W1_VLAN:
            selection.sel.l2.arg.item.vlan = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_MAC_CMD_W1_NO:
            if (arg[0][0] == 's') /*src-mac*/
            {
                selection.sel.l2.arg.item.src_mac = 1;
            }
            else if (arg[0][0] == 'd') /*dst-mac*/
            {
                selection.sel.l2.arg.item.dst_mac = 1;
            }
            else if (arg[0][0] == 'e') /*ethertype*/
            {
                selection.sel.l2.arg.item.ether_type = 1;
            }
            else if (arg[0][0] == 'v') /*vlan*/
            {
                selection.sel.l2.arg.item.vlan = 1;
            }

            if (selection.sel.l2.arg.value == 0)
            {
                return CLI_ERR_INTERNAL; 
            }
            if (TRUE != SWCTRL_PMGR_RemoveHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }            
            break;

        default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_HashSelection_IPv4(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_OM_HashSelection_T  selection;

    selection.type = SWCTRL_OM_HASH_PACKET_TYPE_IPV4;
    selection.sel.ipv4.arg.value = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_DSTIP:
            selection.sel.ipv4.arg.item.dst_ip = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_SRCIP:
            selection.sel.ipv4.arg.item.src_ip = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_DSTL4PORT:
            selection.sel.ipv4.arg.item.dst_l4_port = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_SRCL4PORT:
            selection.sel.ipv4.arg.item.src_l4_port = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_VLAN:
            selection.sel.ipv4.arg.item.vlan = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_PROTOCOLID:
            selection.sel.ipv4.arg.item.protocol_id = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV4_CMD_W1_NO:
            if (arg[0][0] == 's') /* src-ip or src-l4-port */
            {
                if (arg[0][4] == 'i') /* src-ip */
                {
                    selection.sel.ipv4.arg.item.src_ip = 1;
                }
                else if (arg[0][4] == 'l') /*src-l4-port*/
                {
                    selection.sel.ipv4.arg.item.src_l4_port = 1;
                }
            }
            else if (arg[0][0] == 'd') /* dst-ip or dst-l4-port */
            {
                if (arg[0][4] == 'i') /* dst-ip */
                {
                    selection.sel.ipv4.arg.item.dst_ip = 1;
                }
                else if (arg[0][4] == 'l') /*dst-l4-port*/
                {
                    selection.sel.ipv4.arg.item.dst_l4_port = 1;
                }
            }
            else if (arg[0][0] == 'p') /* protocol-id */
            {
                selection.sel.ipv4.arg.item.protocol_id = 1;
            }
            else if (arg[0][0] == 'v') /* vlan-id */
            {
                selection.sel.ipv4.arg.item.vlan = 1;
            }

            if (selection.sel.ipv4.arg.value == 0)
            {
                return CLI_ERR_INTERNAL; 
            }
            if (TRUE != SWCTRL_PMGR_RemoveHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }            
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_HashSelection_IPv6(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_OM_HashSelection_T  selection;

    selection.type = SWCTRL_OM_HASH_PACKET_TYPE_IPV6;
    selection.sel.ipv6.arg.value = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_COLLAPSEDDSTIP:
            selection.sel.ipv6.arg.item.dst_ip = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_COLLAPSEDSRCIP:
            selection.sel.ipv6.arg.item.src_ip = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_DSTL4PORT:
            selection.sel.ipv6.arg.item.dst_l4_port = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_SRCL4PORT:
            selection.sel.ipv6.arg.item.src_l4_port = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_VLAN:
            selection.sel.ipv6.arg.item.vlan = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_NEXTHEADER:
            selection.sel.ipv6.arg.item.next_header = 1;
            if (TRUE != SWCTRL_PMGR_AddHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }
            break;

        case PRIVILEGE_CFG_HASH_SELECTION_IPV6_CMD_W1_NO:
            if (arg[0][0] == 'c') /* collapsed-src-ip or collapsed-dst-ip */
            {
                if (arg[0][10] == 's') /* collapsed-src-ip */
                {
                    selection.sel.ipv6.arg.item.src_ip = 1;
                }
                else if (arg[0][10] == 'd') /* collapsed-drc-ip */
                {
                    selection.sel.ipv6.arg.item.dst_ip = 1;
                }
            }
            else if (arg[0][0] == 'd') /* dst-l4-port */
            {
                selection.sel.ipv6.arg.item.dst_l4_port = 1;
            }
            else if (arg[0][0] == 's') /* src-l4-port */
            {
                selection.sel.ipv6.arg.item.src_l4_port = 1;
            }            
            else if (arg[0][0] == 'n') /* next-header */
            {
                selection.sel.ipv6.arg.item.next_header = 1;
            }
            else if (arg[0][0] == 'v') /* vlan-id */
            {
                selection.sel.ipv6.arg.item.vlan = 1;
            }

            if (selection.sel.ipv6.arg.value == 0)
            {
                return CLI_ERR_INTERNAL; 
            }
            if (TRUE != SWCTRL_PMGR_RemoveHashSelection(ctrl_P->CMenu.list_index, &selection))
            {
                CLI_LIB_PrintStr("Failed to set hash-selection \r\n");
            }            
            break;
            
        default:
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/    
    return CLI_NO_ERROR;
}

UI32_T CLI_API_ShowHashSelection(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_HASH_SELECTION == TRUE)
    UI8_T   hash_list;

    if(arg[0])
    {
        hash_list = atoi(arg[0]);
        if (TRUE != CLI_API_LocalShowOneHashSelection(hash_list))
        {
            CLI_LIB_PrintStr_1("Fail to get hash-selection list %d. \r\n", hash_list);
        }
    }
    else
    {
        for (hash_list = 1; hash_list <= SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE; hash_list ++)
        {
            if (TRUE != CLI_API_LocalShowOneHashSelection(hash_list))
            {
                CLI_LIB_PrintStr_1("Fail to get hash-selection list %d. \r\n", hash_list);
            }
        }
    }

#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_HASH_SELECTION == TRUE)
static BOOL_T CLI_API_LocalShowOneHashSelection(UI8_T hash_list)
{
    UI32_T  line_num = 0;
    SWCTRL_OM_HashSelBlockInfo_T block_info;
    char    buff[CLI_DEF_MAX_BUFSIZE] = {0};
    int     now_size;

#define MAX_FIELD_SELECTION_CHARACTERS_PER_LINE        59
#define COUNT_REST_SIZE(size, string)                           \
        size = size - (sizeof(string)+1);                       \
        if (size < 0)                                           \
        {                                                       \
            size = MAX_FIELD_SELECTION_CHARACTERS_PER_LINE;     \
            sprintf(buff,"%s \r\n   ", buff);                   \
        }

    if (TRUE != SWCTRL_POM_GetHashBlockInfo(hash_list, &block_info))
    {
        return FALSE;
    }

    PROCESS_MORE_1("Hash-selection list %d\r\n", hash_list);

    if (block_info.pkt_l2.arg.value > 0)
    {
        now_size = MAX_FIELD_SELECTION_CHARACTERS_PER_LINE;
    
        sprintf(buff,"  Packet type : MAC\r\n");
        sprintf(buff,"%s    Field selection :", buff);
        if (block_info.pkt_l2.arg.item.dst_mac)
        {
            COUNT_REST_SIZE(now_size, "dst-mac")
            sprintf(buff,"%s dst-mac ", buff);
        }
        if (block_info.pkt_l2.arg.item.src_mac)
        {
            COUNT_REST_SIZE(now_size, "src-mac")
            sprintf(buff,"%s src-mac ", buff);
        }
        if (block_info.pkt_l2.arg.item.ether_type)
        {
            COUNT_REST_SIZE(now_size, "ether-type")
           sprintf(buff,"%s ether-type ", buff);
        }
        if (block_info.pkt_l2.arg.item.vlan)
        {
            COUNT_REST_SIZE(now_size, "vlan-id")
            sprintf(buff,"%s vlan-id ", buff);
        }        

        sprintf(buff, "%s \r\n", buff);
        PROCESS_MORE(buff);
    }
    if (block_info.pkt_ipv4.arg.value > 0)
    {
        now_size = MAX_FIELD_SELECTION_CHARACTERS_PER_LINE;
    
        sprintf(buff,"  Packet type : IPv4\r\n");
        sprintf(buff,"%s    Field selection :", buff);
        if (block_info.pkt_ipv4.arg.item.dst_ip)
        {
            COUNT_REST_SIZE(now_size, "dst-ip")
            sprintf(buff,"%s dst-ip ", buff);
        }
        if (block_info.pkt_ipv4.arg.item.src_ip)
        {
            COUNT_REST_SIZE(now_size, "src-ip")
            sprintf(buff,"%s src-ip ", buff);
        }
        if (block_info.pkt_ipv4.arg.item.dst_l4_port)
        {
            COUNT_REST_SIZE(now_size, "dst-l4-port")
            sprintf(buff,"%s dst-l4-port ", buff);
        }
        if (block_info.pkt_ipv4.arg.item.src_l4_port)
        {
            COUNT_REST_SIZE(now_size, "src-l4-port")
            sprintf(buff,"%s src-l4-port ", buff);
        }
        if (block_info.pkt_ipv4.arg.item.vlan)
        {
            COUNT_REST_SIZE(now_size, "vlan-id")
            sprintf(buff,"%s vlan-id ", buff);
        }
        if (block_info.pkt_ipv4.arg.item.protocol_id)
        {
            COUNT_REST_SIZE(now_size, "protocol-id")
            sprintf(buff,"%s protocol-id ", buff);
        }

        sprintf(buff, "%s \r\n", buff);
        PROCESS_MORE(buff);
    }
    if (block_info.pkt_ipv6.arg.value > 0)
    {
        now_size = MAX_FIELD_SELECTION_CHARACTERS_PER_LINE;
    
        sprintf(buff,"  Packet type : IPv6\r\n");
        sprintf(buff,"%s    Field selection :", buff);
        if (block_info.pkt_ipv6.arg.item.dst_ip)
        {
            COUNT_REST_SIZE(now_size, "collapsed-dst-ip")
            sprintf(buff,"%s collapsed-dst-ip ", buff);
        }
        if (block_info.pkt_ipv6.arg.item.src_ip)
        {
            COUNT_REST_SIZE(now_size, "collapsed-src-ip")
            sprintf(buff,"%s collapsed-src-ip ", buff);
        }
        if (block_info.pkt_ipv6.arg.item.dst_l4_port)
        {
            COUNT_REST_SIZE(now_size, "dst-l4-port")
            sprintf(buff,"%s dst-l4-port ", buff);
        }
        if (block_info.pkt_ipv6.arg.item.src_l4_port)
        {
            COUNT_REST_SIZE(now_size, "src-l4-port")
            sprintf(buff,"%s src-l4-port ", buff);
        }
        if (block_info.pkt_ipv6.arg.item.vlan)
        {
            COUNT_REST_SIZE(now_size, "vlan-id")
            sprintf(buff,"%s vlan-id ", buff);
        }
        if (block_info.pkt_ipv6.arg.item.next_header)
        {
            COUNT_REST_SIZE(now_size, "next-header")
            sprintf(buff,"%s next-header ", buff);
        }

        sprintf(buff, "%s \r\n", buff);
        PROCESS_MORE(buff);
    }

    PROCESS_MORE("\r\n");
    return TRUE;    
}
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

