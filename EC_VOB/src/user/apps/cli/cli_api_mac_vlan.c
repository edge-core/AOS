#include "cli_api.h"
#include "cli_api_mac_vlan.h"
#include "vlan_mgr.h"

#if (SYS_CPNT_MAC_VLAN == TRUE)
static UI32_T show_one_mac_vlan_entry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry, UI32_T line_num );
#endif

UI32_T CLI_API_MAC_VLAN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MAC_VLAN == TRUE)
    UI16_T  vid = 0;
    UI8_T   mac_address[SYS_ADPT_MAC_ADDR_LEN] = {0}, mask[SYS_ADPT_MAC_ADDR_LEN]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff};;
    UI8_T   priority = SYS_DFLT_1P_PORT_DEFAULT_USER_PRIORITY;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_MACVLAN_MACADDRESS:
        CLI_LIB_ValsInMac((char *)arg[0], mac_address);

        if(arg[1][0] == 'v' || arg[1][0] == 'V')
        {
            vid = atoi(arg[2]);

            #if (SYS_CPNT_MAC_VLAN_WITH_PRIORITY == TRUE)
            if(arg[4] != NULL)
                priority = atoi(arg[4]);
            #endif
        }
        #if(SYS_CPNT_MAC_VLAN_WITH_MASK == TRUE)
        else if(arg[1][0] == 'm' || arg[1][0] == 'M')
        {
            CLI_LIB_ValsInMac(arg[2], mask);

            vid = atoi(arg[4]);

            #if (SYS_CPNT_MAC_VLAN_WITH_PRIORITY == TRUE)
            if(arg[6] != NULL)
                priority = atoi(arg[6]);
            #endif
        }
        #endif
        else
        {
          return CLI_ERR_INTERNAL;
        }

        if(FALSE == VLAN_PMGR_SetMacVlanEntry(mac_address, mask, vid, priority))
        {
            CLI_LIB_PrintStr("Failed to set MAC-based VLAN.\r\n");
        }

        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MACVLAN_MACADDRESS:
        if(arg[0][0] == 'a' && arg[0][1] == 'l' && arg[0][2] == 'l')
        {/*no mac-vlan mac-address  all*/
            //remove all entries
            if(FALSE == VLAN_PMGR_DeleteAllMacVlanEntry())
            {
                CLI_LIB_PrintStr("Failed to remove all entries.\r\n");
            }
        }
        else
        {/*no mac-vlan mac-address all */
            //remove one entry
            CLI_LIB_ValsInMac(arg[0], mac_address);
            if(arg[1]!=NULL)
                CLI_LIB_ValsInMac(arg[2], mask);
            if(FALSE == VLAN_PMGR_DeleteMacVlanEntry(mac_address, mask))
            {
                CLI_LIB_PrintStr("Failed to remove an entry.\r\n");
            }
        }
        break;

    default:
      return CLI_NO_ERROR;
    }
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Show_MAC_VLAN(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_MAC_VLAN == TRUE)
    UI32_T  line_num = 0;
    VLAN_TYPE_MacVlanEntry_T    mac_vlan_entry;
    memset(&mac_vlan_entry, 0, sizeof(VLAN_TYPE_MacVlanEntry_T));
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

#if (SYS_CPNT_MAC_VLAN_WITH_PRIORITY == TRUE)
    PROCESS_MORE("MAC Address        Mask               VLAN ID  Priority\r\n");
    PROCESS_MORE("------------------ ------------------ -------- --------\r\n");
#else
    PROCESS_MORE("MAC Address        Mask               VLAN ID\r\n");
    PROCESS_MORE("------------------ ------------------ --------\r\n");
#endif

    while(VLAN_OM_GetNextMacVlanEntry(&mac_vlan_entry))
    {
        if((line_num = show_one_mac_vlan_entry(&mac_vlan_entry, line_num)) == JUMP_OUT_MORE)
        {
            return CLI_NO_ERROR;
        }
        else if (line_num == EXIT_SESSION_MORE)
        {
            return CLI_EXIT_SESSION;
        }
    }

#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

    return CLI_NO_ERROR;
}

#if (SYS_CPNT_MAC_VLAN == TRUE)
static UI32_T show_one_mac_vlan_entry(VLAN_TYPE_MacVlanEntry_T *mac_vlan_entry, UI32_T line_num )
{
    char   mac_address_str[18] = {0};
    char   mask_str[18] = {0};
    char   buff[CLI_DEF_MAX_BUFSIZE] = {0};

    sprintf(mac_address_str, "%02X-%02X-%02X-%02X-%02X-%02X",
            mac_vlan_entry->mac_address[0],
            mac_vlan_entry->mac_address[1],
            mac_vlan_entry->mac_address[2],
            mac_vlan_entry->mac_address[3],
            mac_vlan_entry->mac_address[4],
            mac_vlan_entry->mac_address[5]
            );

    sprintf(mask_str, "%02X-%02X-%02X-%02X-%02X-%02X",
            mac_vlan_entry->mask[0],
            mac_vlan_entry->mask[1],
            mac_vlan_entry->mask[2],
            mac_vlan_entry->mask[3],
            mac_vlan_entry->mask[4],
            mac_vlan_entry->mask[5]
            );
    #if (SYS_CPNT_MAC_VLAN_WITH_PRIORITY == TRUE)
        sprintf(buff, "%-18s %-18s %8u %8u\r\n", mac_address_str, mask_str, mac_vlan_entry->vid, mac_vlan_entry->priority);
        PROCESS_MORE_FUNC(buff);
    #else
        sprintf(buff, "%-18s %-18s %8u\r\n", mac_address_str, mask_str, mac_vlan_entry->vid);
        PROCESS_MORE_FUNC(buff);
    #endif

    return line_num;
}
#endif /*end of #if (SYS_CPNT_MAC_VLAN == TRUE)*/

