/*
 * MODULE NAME: SWCTRL_OM.h
 *
 * PURPOSE: To store and manage switch information

 *
 * NOTES:	Due to swctrl do not have OM before, only new or necessary function will join OM now.
 *
 *  History :   
 *      Date            Modifier    Reason
 *  ---------------------------------------------------------------------
 *      06-11-2007       Simon      init: Lan.c need to get QinQ information
 * ----------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2007
 * ======================================================================
*/


/**********************************/
/* INCLUDE FILE DECLARATIONS      */
/**********************************/
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "swctrl.h"
#include "swctrl_om.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "l_sort_lst.h"
#include "sysrsc_mgr.h"

/**********************************/
/* NAME	CONSTANT DECLARATIONS     */
/**********************************/

/**********************************/
/* MACRO FUNCTIONS DECLARACTION   */
/**********************************/

/**********************************/
/* LOCAL DATA TYPE DECLARATIONS   */
/**********************************/
typedef struct
{
    UI32_T debug_flag;

    SWCTRL_OM_PortInfo_T port_info_ex[SYS_ADPT_TOTAL_NBR_OF_LPORT];

    UI32_T                       common_mirror_dest_port;
    UI32_T                       common_mirror_tx_dest_port;
    UI32_T                       config_mirror_mode;

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
    SWCTRL_MacAddrMirrorEntry_T  macaddr_mirror_info[SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY];
    UI32_T mac_addr_mirror_entry_cnt; /* how many mac-address entry used */
#endif

#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
    BOOL_T pse_check_status;
#endif
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    SWCTRL_OM_PortSfpInfo_T port_sfp[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT];
#endif

#if (SYS_CPNT_HASH_SELECTION == TRUE)
    SWCTRL_OM_HashSelBlockInfo_T   hash_block[SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE]; 
#endif
} SWCTRL_OM_Shmem_Data_T;

/**********************************/
/* LOCAL SUBPROGRAM DECLARATIONS  */ 
/**********************************/

/**********************************/
/* STATIC VARIABLE DECLARATIONS   */
/**********************************/
static SWCTRL_OM_Shmem_Data_T *swctrl_shmem_data_p;

/**********************************/
/* EXPORTED SUBPROGRAM BODIES     */
/**********************************/ 
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_InitiateSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will init system resource
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_InitiateSystemResources(void)
{
    swctrl_shmem_data_p = (SWCTRL_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWCTRL_SHMEM_SEGID);
    SWCTRL_OM_Init();
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_AttachSystemResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_AttachSystemResources(void)
{
    swctrl_shmem_data_p = (SWCTRL_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWCTRL_SHMEM_SEGID);

}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: SWCTRL_OM_GetShMemInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: Provide shared memory information for SYSRSC.
 * INPUT:   None
 * OUTPUT:  segid_p  --  shared memory segment id
 *          seglen_p --  length of the shared memroy segment
 * RETUEN:  None
 * NOTES:
 *    This function is called in SYSRSC_MGR_CreateSharedMemory().
 *-----------------------------------------------------------------------------
 */
void SWCTRL_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    UI32_T shmem_size = sizeof(SWCTRL_OM_Shmem_Data_T);

    *segid_p = SYSRSC_MGR_SWCTRL_SHMEM_SEGID;
    *seglen_p = shmem_size;
}

void SWCTRL_OM_Init()
{
    UI8_T *working_buffer = (UI8_T *)(swctrl_shmem_data_p + 1);

    memset(swctrl_shmem_data_p, 0, sizeof(*swctrl_shmem_data_p));

    SWCTRL_OM_ResetAll();
}

void SWCTRL_OM_ResetAll()
{
    UI32_T ifindex, unit;
#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    UI32_T sfp_index;
#endif

    swctrl_shmem_data_p->common_mirror_dest_port = 0;
    swctrl_shmem_data_p->config_mirror_mode = SWCTRL_OM_NONE_MIRROR_MODE;

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
    memset(swctrl_shmem_data_p->macaddr_mirror_info, 0, sizeof(swctrl_shmem_data_p->macaddr_mirror_info));
    swctrl_shmem_data_p->mac_addr_mirror_entry_cnt = 0;
#endif

    for (ifindex = 1; ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ifindex++)
    {
        SWCTRL_OM_ResetPortInfo(ifindex);
    }

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
    for (unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        for (sfp_index = 1; sfp_index <= SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT; sfp_index++)
        {
            SWCTRL_OM_ResetPortSfpInfo(unit, sfp_index);
        }
    }
#endif

#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
    swctrl_shmem_data_p->pse_check_status=SYS_DFLT_PSE_CHECK_STATUS;
#endif

}

void SWCTRL_OM_ResetPortInfo(UI32_T ifindex)
{

};

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
void SWCTRL_OM_ResetPortSfpInfo(UI32_T unit, UI32_T sfp_index)
{
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].present = FALSE;
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].trap_enable = SYS_DFLT_SFP_DDM_ALARMWARN_TRAP_STATUS;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].auto_mode = SYS_DFLT_SFP_DDM_ALARMWARN_AUTO_MODE;

    /* temperature */
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_WARNING;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_WARNING;

    /* voltage */
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_WARNING;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_WARNING;

    /* bias current */
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_WARNING;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_WARNING;

    /* tx-power */
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_WARNING;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_WARNING;

    /* rx-power */
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_ALARM;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_WARNING;
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_WARNING;
    memset(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold_status, 0x0, sizeof(SWCTRL_OM_SfpDdmThresholdStatus_T));
#endif
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info.measured.rx_los_asserted = TRUE;
}
#endif /* end of #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_TrunkFollowUserPortAttributes
 * -------------------------------------------------------------------------
 * FUNCTION: This function will copy config of first member to trunk
 * INPUT   : trunk_ifindex -- the port index of trunk
             tm_ifindex    -- the port index of trunk member
 * OUTPUT  : None
 * RETURN  : None
 * Note    :
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_TrunkFollowUserPortAttributes(UI32_T trunk_ifindex, UI32_T tm_ifindex)
{
     swctrl_shmem_data_p->port_info_ex[trunk_ifindex-1] = swctrl_shmem_data_p->port_info_ex[tm_ifindex-1];
}

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set mirror MAC address entry
 * INPUT   : *mac_addr     -- mac addess
             is_add        -- TRUE  : add address entry  
                           -- FALSE : remove address entry   
 * OUTPUT  : None
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_SetMacAddrMirrorEntry(UI8_T *mac_addr, BOOL_T is_add)
{
    UI16_T mac_addr_id;
    UI16_T mac_addr_list_id = 0; 
    BOOL_T is_found  = FALSE;
    BOOL_T is_succed = FALSE;

    /* find this mac_address entry */
    for (mac_addr_id=0; mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        if ((memcmp(&swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr, 
                    mac_addr, SYS_ADPT_MAC_ADDR_LEN) == 0) && 
            (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == TRUE)) 
        {
            is_found = TRUE;
            mac_addr_list_id = mac_addr_id; /* keep entry list id */
            break;
        }
    }
    
    /* rules check */
    if ( ((is_add == TRUE) && (is_found == FALSE)) &&
         (swctrl_shmem_data_p->mac_addr_mirror_entry_cnt == SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY))
    {
        /* rule_1: all mac-address entry are full */
        return FALSE; 
    }
    else if ((is_add == TRUE) && (is_found == TRUE))
    {
        /* rule_2: this mac address is already exist entry */
        return TRUE;
    }
    else if ((is_add == FALSE) && (is_found == FALSE))
    {
        /* rule_3: we cannot delete this mac address entry */
        return FALSE;
    }
    else if ((is_add == FALSE) && (is_found == TRUE))
    {
        /* rule_4: removed this mac-address entry */
        swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_list_id].is_valid = FALSE;
        memset(&swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_list_id].mac_addr, 0, SYS_ADPT_MAC_ADDR_LEN);
         
        swctrl_shmem_data_p->mac_addr_mirror_entry_cnt--;
        return TRUE;
    }
        
    /* all rules pass, so save new mac-address mirror to entry list */    
    for (mac_addr_id=0; mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        /* check available mac-address entry list */
        if (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == FALSE)
        {
            /* copy mac-address to entry list*/
            memcpy(&swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr,
                   mac_addr, SYS_ADPT_MAC_ADDR_LEN);
            swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid = TRUE;  
            is_succed = TRUE; 
            break;   
        }
    }
    
    /* rule_5: submitted new mac-address mirror entry shall be succed */
    if ((is_succed == TRUE) && (is_add == TRUE))
    {
        swctrl_shmem_data_p->mac_addr_mirror_entry_cnt++;
        return TRUE;
    }

    return FALSE;
}/* End of SWCTRL_OM_SetMacAddrMirrorEntry() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetExactMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get exactly mirror entry by MAC-address 
 * INPUT   : addr_entry->mac_addr   - mac address
 * OUTPUT  : addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : We shall returns exactly mac_address entry and index
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetExactMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry)
{
    UI16_T mac_addr_id;
    BOOL_T is_found  = FALSE;
    
    if (swctrl_shmem_data_p->mac_addr_mirror_entry_cnt == 0 || addr_entry == NULL)
    {
        /* rule_1: there's no any available entry list */
        return FALSE;
    }
    
    /* find this mac_address entry and entry_index can be ignore*/
    for (mac_addr_id=0; mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        if ((memcmp(&swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr, 
                    addr_entry->mac_addr, SYS_ADPT_MAC_ADDR_LEN) == 0) && 
            (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == TRUE)) 
        {
            is_found = TRUE;
            addr_entry->addr_entry_index = mac_addr_id+1;
            addr_entry->is_valid = TRUE;
            
            break;
        }
    } 

    /* rule_2: found this mac_address entry */
    if (is_found == TRUE)
        return TRUE;

    return FALSE;
}/* End of SWCTRL_OM_GetExactMacAddrMirrorEntry() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetNextMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get next mac address of mirror entry
 * INPUT   : *addr_entry -- mac address entry 
 * OUTPUT  : *addr_entry -- mac entry
 *           addr_entry->addr_entry_index
 *           addr_entry->mac_addr    
 *           addr_entry->is_valid
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : if the addr_entry_index = 0
 *           , means returns first mac-address entry
 *           otherwise, we shall verify current index id and mac-address
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetNextMacAddrMirrorEntry(SWCTRL_MacAddrMirrorEntry_T *addr_entry)
{
    UI16_T mac_addr_id, mac_addr_id_st; 
    BOOL_T is_found  = FALSE;
    
    if (swctrl_shmem_data_p->mac_addr_mirror_entry_cnt == 0 || addr_entry == NULL)
    {
        /* rule_1: there's no any available entry list */
        return FALSE;
    }
    
    /* rule 1_1: check entry index shall not exceed maximal array size 
     * The addr_entry shall be 1 ~ SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY
     * The addr_entry->addr_entry_index = 0 means nothing.
     */
    if (addr_entry->addr_entry_index > SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY)
    {
        return FALSE;
    }
    
    /* rule_2: init first seeking index */
    if (addr_entry->addr_entry_index != 0)
    {
        /* go to next entry id */   
        mac_addr_id_st = addr_entry->addr_entry_index; 
        
        /* even current index and mac-address are invalid, we don't check these parameters 
         */
    }
    else
    {
        mac_addr_id_st = 0;
    }

    /* find this mac_address entry */
    for (mac_addr_id=mac_addr_id_st; 
         mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        if (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == TRUE) 
        {
            is_found = TRUE;

            /* copy entry info and returns */
            addr_entry->addr_entry_index = mac_addr_id+1;
            addr_entry->is_valid = TRUE;
            memcpy(addr_entry->mac_addr, &swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr,
                   SYS_ADPT_MAC_ADDR_LEN);
            break;       

        }
    } 

    /* rule_2: found next mac_address entry */
    if (is_found == TRUE)
    {
        return TRUE;
    }

    return FALSE;
}/* End of SWCTRL_OM_GetNextMacAddrMirrorEntry() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetNextMacAddrMirrorEntryForSnmp
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get next mac address of mirror entry (for SNMP)
 * INPUT   : *mac           -- mac address 
 * OUTPUT  : *mac           -- mac address
 *           *ifindex_dest  -- dest port
 *
 * RETURN  : TRUE: success , FALSE: fail
 * NOTE    : if the mac_addr = 00-00-00-00-00-00
 *           , means returns first mac-address entry 
 *           otherwise, we shall use current mac-address to find next one
 *           and no sort request for this API
 *
 *           SNMP must use the field(s) of mib entry to act as key(s), so 
 *           SWCTRL_POM_GetNextMacAddrMirrorEntry is not suitable for the 
 *           usage of SNMP. And this api is created for the reason.
 *
 *           SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetNextMacAddrMirrorEntryForSnmp(UI8_T *mac, UI32_T *ifindex_dst)
{
    UI16_T mac_addr_id; 
    BOOL_T is_match      = FALSE;  /* to record if succeed to find match entry by input mac */ 
    BOOL_T is_get_first  = FALSE;  /* to record if get first entry */
    UI8_T  zero_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};
    
    if (swctrl_shmem_data_p->mac_addr_mirror_entry_cnt == 0 || mac == NULL || ifindex_dst == NULL)
    {
        /* there's no any available entry list */
        return FALSE;
    }   
    
    /* init first seeking index */
    if (0 == memcmp(mac, zero_mac, SYS_ADPT_MAC_ADDR_LEN) )
    {
        is_get_first = TRUE;
    }

    /* find this mac_address entry */
    for (mac_addr_id=0; 
         mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        if (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == TRUE) 
        {
            if ((TRUE == is_get_first) || (TRUE == is_match))
            {                   
                /* copy entry info and returns */
                *ifindex_dst = swctrl_shmem_data_p->common_mirror_dest_port;
                memcpy(mac, &swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr, SYS_ADPT_MAC_ADDR_LEN);

                return TRUE;
            }
            else if (0 == memcmp(mac, swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr, SYS_ADPT_MAC_ADDR_LEN))                
            {
                is_match = TRUE;
            }   
        }
    } 

    return FALSE;
}/* End of SWCTRL_OM_GetNextMacAddrMirrorEntry() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_IsExistedMacAddrMirrorEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will check if exist the mirror entry (mac+dst-port)
 * INPUT   : *mac_addr      -- mac address 
 *           ifindex_dest   -- dest port
 * OUTPUT  : None
 *
 * RETURN  : TRUE: exist , FALSE: un-exist
 * NOTE    : SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY --  this constant 
 *           allow maximal mac based mirror entries 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_IsExistedMacAddrMirrorEntry(UI8_T *mac, UI32_T ifindex_dst)
{
    UI16_T mac_addr_id; 
    
    if (swctrl_shmem_data_p->mac_addr_mirror_entry_cnt == 0 || mac == NULL)
    {
        /* there's no any available entry list */
        return FALSE;
    }   
    
    /* check dest port */
    if (ifindex_dst != swctrl_shmem_data_p->common_mirror_dest_port)
    {
        return FALSE;    
    }
    
    /* find this mac_address entry */
    for (mac_addr_id=0; 
         mac_addr_id<SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY; mac_addr_id++)
    {
        if (swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].is_valid == TRUE) 
        {
            if (0 == memcmp(mac, swctrl_shmem_data_p->macaddr_mirror_info[mac_addr_id].mac_addr, SYS_ADPT_MAC_ADDR_LEN))                
            {
                return TRUE;
            }  
        }
    } 

    return FALSE;
}/* End of SWCTRL_OM_GetNextMacAddrMirrorEntry() */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetMacAddrMirrorCnt
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns current mirror counter
 * INPUT   : nbr
 * OUTPUT  : nbr  -- return how many mac-address mirroring counter
 *
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetMacAddrMirrorCnt(UI32_T *nbr)
{
    *nbr = swctrl_shmem_data_p->mac_addr_mirror_entry_cnt;
}

#endif /* End of #if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetMirrorConfigMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set or remove mirror mode 
 * INPUT   : mode       -- SWCTRL_OM_PORT_MIRROR_MODE
 *                         SWCTRL_OM_VLAN_MIRROR_MODE
 *                         SWCTRL_OM_MAC_MIRROR_MODE 
 *           is_active  -- TRUE : add 
 *                         FALSE: remove
 * OUTPUT  : None
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetMirrorConfigMode(UI32_T mode, BOOL_T is_active)
{
    /* updated mirror mode status */
    if (is_active == TRUE)
    {
        swctrl_shmem_data_p->config_mirror_mode |= mode;
    }
    else
    {
        swctrl_shmem_data_p->config_mirror_mode &= ~(mode);
    }    
}/* End of SWCTRL_OM_SetMirrorConfigMode()*/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetMirrorConfigMode
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns current mirror configuration
 * INPUT   : mode
 * OUTPUT  : mode       -- SWCTRL_OM_PORT_MIRROR_MODE |
 *                         SWCTRL_OM_VLAN_MIRROR_MODE |
 *                         SWCTRL_OM_MAC_MIRROR_MODE 
 *                      -- SWCTRL_OM_NONE_MIRROR_MODE, none mirror operation.
 * RETURN  : None
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetMirrorConfigMode(UI32_T *mode)
{
    /* 
     * When these mirrir mode are disabled, then returns SWCTRL_OM_NONE_MIRROR_MODE 
     * *mode & SWCTRL_OM_PORT_MIRROR_MODE == 0
     * *mode & SWCTRL_OM_VLAN_MIRROR_MODE == 0
     * *mode & SWCTRL_OM_MAC_MIRROR_MODE  == 0
     */    
    *mode = swctrl_shmem_data_p->config_mirror_mode;
}

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE) || (SYS_CPNT_VLAN_MIRROR == TRUE) || (SYS_CPNT_ACL_MIRROR == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetCommonMirrorDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set common mirror rx destination port.
 * INPUT   : ifindex       -- destination port
 * OUTPUT  : None
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetCommonMirrorDestPort(UI32_T ifindex)
{
    swctrl_shmem_data_p->common_mirror_dest_port = ifindex;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetCommonMirrorDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns common mirror rx destination port.
 * INPUT   : ifindex
 * OUTPUT  : ifindex      -- returns common mirror destination port
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetCommonMirrorDestPort(UI32_T *ifindex)
{
    *ifindex = swctrl_shmem_data_p->common_mirror_dest_port;
}
#endif /*#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)||(SYS_CPNT_VLAN_MIRROR == TRUE)||(SYS_CPNT_ACL_MIRROR == TRUE)*/

#if (SYS_CPNT_ACL_MIRROR_EGRESS == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetCommonMirrorTxDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set common mirror tx destination port.
 * INPUT   : ifindex       -- destination port
 * OUTPUT  : None
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetCommonMirrorTxDestPort(UI32_T ifindex)
{
    swctrl_shmem_data_p->common_mirror_tx_dest_port = ifindex;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetCommonMirrorTxDestPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function will returns common mirror tx destination port.
 * INPUT   : ifindex
 * OUTPUT  : ifindex      -- returns common mirror destination port
 * RETURN  : None
 * Note    : ifindex 0 indicates NULL.
 *           used for VLAN/MAC/ACL based mirror
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_GetCommonMirrorTxDestPort(UI32_T *ifindex)
{
    *ifindex = swctrl_shmem_data_p->common_mirror_tx_dest_port;
}
#endif /*#if (SYS_CPNT_ACL_MIRROR_EGRESS == TRUE)*/

/**********************************/
/* LOCAL SUBPROGRAM DEFINITIONS   */ 
/**********************************/

#if ((SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE))
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE: PSE check enabled, FALSE: PSE check disabled
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetPSECheckStatus(void)
{
    return swctrl_shmem_data_p->pse_check_status;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 * INPUT   : pse_check_status - TRUE:enabled, FALSE:disabled
 * OUTPUT  : None
 * RETURN  : TRUE: PSE check enabled, FALSE: PSE check disabled
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_SetPSECheckStatus(BOOL_T pse_check_status)
{
    swctrl_shmem_data_p->pse_check_status=pse_check_status;
}
#endif

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp present status
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWCTRL_OM_GetPortSfpPresent(UI32_T unit, UI32_T sfp_index)
{
    return swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].present;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpPresent
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp present status
 *
 * INPUT   : unit
 *           sfp_index
 *           is_present
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpPresent(UI32_T unit, UI32_T sfp_index, BOOL_T is_present)
{
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].present = is_present;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_info_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p)
{
    memcpy(sfp_info_p, &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_info, sizeof(SWCTRL_OM_SfpInfo_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpInfo_T *sfp_info_p)
{
    memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_info, sfp_info_p, sizeof(SWCTRL_OM_SfpInfo_T));
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p)
{
    memcpy(sfp_ddm_info_p, &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info, sizeof(SWCTRL_OM_SfpDdmInfo_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmInfo(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfo_T *sfp_ddm_info_p)
{
    memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info, sfp_ddm_info_p, sizeof(SWCTRL_OM_SfpDdmInfo_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm measured eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_info_measured_p
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p)

{
    memcpy(sfp_ddm_info_measured_p, &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info.measured, sizeof(SWCTRL_OM_SfpDdmInfoMeasured_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmInfoMeasured
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm measured eeprom info
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_info_measured_p
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmInfoMeasured(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p)
{
    memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info.measured, sfp_ddm_info_measured_p, sizeof(SWCTRL_OM_SfpDdmInfoMeasured_T));
}
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold alarm trap
 * INPUT   : unit
 *           sfp_index
 *           trap_enable -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmTrapEnable(UI32_T unit, UI32_T sfp_index, BOOL_T trap_enable)
{
    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].trap_enable = trap_enable;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetPortSfpDdmTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Get statu of SFP ddm threshold alarm trap
 * INPUT   : unit
 *           sfp_index
 *           trap_enable_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmTrapEnable(UI32_T unit, UI32_T sfp_index, BOOL_T *trap_enable_p)
{
    *trap_enable_p = swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].trap_enable;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Enable/Disable SFP ddm threshold auto mode
 * INPUT   : unit
 *           sfp_index
 *           auto_mode -- TRUE/FALSE
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdAutoMode(UI32_T unit, UI32_T sfp_index, BOOL_T auto_mode)
{
    if(swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].auto_mode == auto_mode)
        return;

    swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].auto_mode = auto_mode;

    if(auto_mode == TRUE)
    {
        if(swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].present == TRUE)
        {
            memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold,
                   &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info.threshold,
                   sizeof(SWCTRL_OM_SfpDdmThreshold_T));
        }
    }
    else
    {
        if(swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].present == TRUE &&
           swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_info.support_ddm == TRUE)
        {
            memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold,
                   &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_info.threshold,
                   sizeof(SWCTRL_OM_SfpDdmThreshold_T));
        }
        else
        {
            /* set all ddm threshold to default */
            SWCTRL_OM_SetPortSfpDdmThresholdDefault(unit, sfp_index, 0);
        }
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : auto_mode_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThresholdAutoMode(UI32_T unit, UI32_T sfp_index, BOOL_T *auto_mode_p)
{
    *auto_mode_p = swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].auto_mode;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdDefault
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold to default
 * INPUT   : unit
 *           sfp_index
 *           threshold_type -- which threshold_type
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdDefault(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type)
{
    switch (threshold_type)
    {
        /* all threshold */
        case 0:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_WARNING;

            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_WARNING;

            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_WARNING;

            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_WARNING;

            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_WARNING;
            break;

         /* temperature */
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_alarm = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_warning = SYS_DFLT_SFP_THRESHOLD_TEMP_LOW_WARNING;
            break;

         /* voltage */
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_alarm = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_warning = SYS_DFLT_SFP_THRESHOLD_VOLTAGE_LOW_WARNING;
            break;

         /* current */
        case VAL_trapSfpThresholdAlarmWarnType_currentHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_currentLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_currentHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_currentLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_alarm = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_warning = SYS_DFLT_SFP_THRESHOLD_BIAS_LOW_WARNING;
            break;

          /* tx power */
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_TX_POWER_LOW_WARNING;
            break;

        /* rx power */
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_alarm = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_ALARM;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_HIGH_WARNING;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_warning = SYS_DFLT_SFP_THRESHOLD_RX_POWER_LOW_WARNING;
            break;

        default:
            break;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold
 *
 * INPUT   : unit
 *           sfp_index
 *           threshold_type
 *           val(float)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThreshold(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type, float val)
{
    switch(threshold_type)
    {
        /* temperature */
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_warning = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_warning = val;
            break;

        /* voltage */
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_warning = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_warning = val;
            break;

        /* current */
        case VAL_trapSfpThresholdAlarmWarnType_currentHighAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_currentLowAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_currentHighWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_warning = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_currentLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_warning = val;
            break;

        /* tx power */
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_warning = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_warning = val;
            break;

        /* rx power */
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowAlarm:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_alarm = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_warning = val;
            break;
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_warning = val;
            break;

        default:
            break;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThresholdForWeb
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold for web which sets four val at once
 *
 * INPUT   : unit
 *           sfp_index
 *           threshold_type
 *           high_alarm(float)
 *           high_warning(float)
 *           low_warning(float)
 *           low_alarm(float)
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdForWeb(UI32_T unit, UI32_T sfp_index, UI32_T threshold_type, float high_alarm,
     float high_warning, float low_warning, float low_alarm)
{
    switch(threshold_type)
    {
        /* temperature */
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_temperatureLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_alarm = high_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_alarm = low_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_high_warning = high_warning;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.temp_low_warning = low_warning;
            break;

        /* voltage */
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_voltageHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_voltageLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_alarm = high_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_alarm = low_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_high_warning = high_warning;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.voltage_low_warning = low_warning;
            break;

        /* current */
        case VAL_trapSfpThresholdAlarmWarnType_currentHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_currentLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_currentHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_currentLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_alarm = high_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_alarm = low_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_high_warning = high_warning;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.bias_low_warning = low_warning;
            break;

        /* tx power */
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_txPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_alarm = high_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_alarm = low_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_high_warning = high_warning;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.tx_power_low_warning = low_warning;
            break;

        /* rx power */
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowAlarm:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerHighWarning:
        case VAL_trapSfpThresholdAlarmWarnType_rxPowerLowWarning:
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_alarm = high_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_alarm = low_alarm;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_high_warning = high_warning;
            swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold.rx_power_low_warning = low_warning;
            break;

        default:
            break;
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetPortSfpDdmThresholdAll
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold
 *
 * INPUT   : unit
 *           sfp_index
 *           sfp_ddm_threshold
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdAll(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p)
{
    memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold, sfp_ddm_threshold_p, sizeof(SWCTRL_OM_SfpDdmThreshold_T));
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThreshold(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThreshold_T *sfp_ddm_threshold_p)
{
    memcpy(sfp_ddm_threshold_p, &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold, sizeof(SWCTRL_OM_SfpDdmThreshold_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_SetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp ddm threshold status
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_SetPortSfpDdmThresholdStatus(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p)
{
    memcpy(&swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold_status, sfp_ddm_threshold_status_p, sizeof(SWCTRL_OM_SfpDdmThresholdStatus_T));
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_GetPortSfpDdmThresholdStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm threshold status
 * INPUT   : unit
 *           sfp_index
 * OUTPUT  : sfp_ddm_threshold_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_OM_GetPortSfpDdmThresholdStatus(UI32_T unit, UI32_T sfp_index, SWCTRL_OM_SfpDdmThresholdStatus_T *sfp_ddm_threshold_status_p)
{
    memcpy(sfp_ddm_threshold_status_p, &swctrl_shmem_data_p->port_sfp[unit-1][sfp_index-1].sfp_ddm_threshold_status, sizeof(SWCTRL_OM_SfpDdmThresholdStatus_T));
}
#endif /* #if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE) */

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_IsHashSelBindForTheService
 * -------------------------------------------------------------------------
 * FUNCTION: Check if there is hash-selection list bound for the service
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 * OUTPUT  : list_index - the list index bound for the service
 * RETURN  : TRUE  - bind
 *           FALSE - not bind
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_IsHashSelBindForTheService(SWCTRL_OM_HashSelService_T service, UI8_T *list_index)
{
    UI8_T i;

    for (i=0; i<SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE; i++)
    {
        if(swctrl_shmem_data_p->hash_block[i].ref_service_bmp & service)
        {
            *list_index = i+1;
            return TRUE;
        }
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_BindHashSelForService(SWCTRL_OM_HashSelService_T service, UI8_T list_index)
{
    if (list_index < 1 || list_index > SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE)
    {
        return FALSE;
    }

    swctrl_shmem_data_p->hash_block[list_index-1].ref_service_bmp |= service;
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection list
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_UnBindHashSelForService(SWCTRL_OM_HashSelService_T service, UI8_T list_index)
{
    if (list_index < 1 || list_index > SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE)
    {
        return FALSE;
    }

    swctrl_shmem_data_p->hash_block[list_index-1].ref_service_bmp &= ~service;
    return TRUE; 
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_AddHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: add hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_AddHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p)
{
    UI8_T index;

    if (list_index < 1 || list_index > SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE)
    {
        return FALSE;
    }
    index = list_index - 1;

    if (swctrl_shmem_data_p->hash_block[index].ref_service_bmp > 0)
    {
        return FALSE;
    }

    switch (selection_p->type)
    {
        case SWCTRL_OM_HASH_PACKET_TYPE_L2:
            swctrl_shmem_data_p->hash_block[index].pkt_l2.arg.value |= selection_p->sel.l2.arg.value;
            break;

        case SWCTRL_OM_HASH_PACKET_TYPE_IPV4:
            swctrl_shmem_data_p->hash_block[index].pkt_ipv4.arg.value |= selection_p->sel.ipv4.arg.value;
            break;

        case SWCTRL_OM_HASH_PACKET_TYPE_IPV6:
            swctrl_shmem_data_p->hash_block[index].pkt_ipv6.arg.value |= selection_p->sel.ipv6.arg.value;
            break;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_RemoveHashSelection
 * -------------------------------------------------------------------------
 * FUNCTION: remove hash-selection field
 * INPUT   : list_index - the index of hash-selection list
 *           selection_p - hash-selection field
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : If the hash-selection list has been bound, then it can't be modified
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_RemoveHashSelection(
    UI8_T list_index , 
    SWCTRL_OM_HashSelection_T *selection_p)
{
    UI8_T index;

    if (list_index < 1 || list_index > SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE)
    {
        return FALSE;
    }
    index = list_index - 1;

    if (swctrl_shmem_data_p->hash_block[index].ref_service_bmp > 0)
    {
        return FALSE;
    }

    switch (selection_p->type)
    {
        case SWCTRL_OM_HASH_PACKET_TYPE_L2:
            swctrl_shmem_data_p->hash_block[index].pkt_l2.arg.value &= ~(selection_p->sel.l2.arg.value);
            break;

        case SWCTRL_OM_HASH_PACKET_TYPE_IPV4:
            swctrl_shmem_data_p->hash_block[index].pkt_ipv4.arg.value &= ~(selection_p->sel.ipv4.arg.value);
            break;

        case SWCTRL_OM_HASH_PACKET_TYPE_IPV6:
            swctrl_shmem_data_p->hash_block[index].pkt_ipv6.arg.value &= ~(selection_p->sel.ipv6.arg.value);
            break;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetHashBlockInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection block info
 * INPUT   : list_index - the index of hash-selection list
 * OUTPUT  : block_info_p - the hash-selection block info
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetHashBlockInfo(UI8_T list_index , SWCTRL_OM_HashSelBlockInfo_T *block_info_p)
{
    if (list_index < 1 || list_index > SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE)
    {
        return FALSE;
    }

    memcpy(block_info_p, &swctrl_shmem_data_p->hash_block[list_index-1], sizeof(SWCTRL_OM_HashSelBlockInfo_T));
    return TRUE;
}
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#if(SYS_CPNT_WRED == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_SetRandomDetect
 * -------------------------------------------------------------------------
 * FUNCTION: Set ecn marking percentage
 * INPUT   : ifindex      - which port to set
 *           queue_id     - which queue to set
 *           min          - min threshold
 *           max          - max threshold
 *           drop         - drop threshold
 *           valid        - entry is valid
 * OUTPUT  : None
 * RETURN  : TRUE - success
 *           FALSE- fail
 * NOTE    : 
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_SetRandomDetect(UI32_T ifindex, 
                                 I8_T queue_id,
                                 UI32_T min,
                                 UI32_T max,
                                 UI32_T drop,
                                 UI32_T ecn,
                                 BOOL_T valid)
{
    I8_T i;
    
    if(queue_id >= 0)
    {
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.min[queue_id] = min;
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.max[queue_id] = max;
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.drop[queue_id]= drop;
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.ecn[queue_id] = ecn;	
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.valid[queue_id]= valid; 			
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.all_queue     = TRUE; 			
      
      /*check if different from other queue*/
      for(i=0; i<SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
      {
        if(i == queue_id)
			continue;
        
        if(swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.valid == FALSE)
            continue;
        
        if(memcmp(&swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.ecn[i],
                  &swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.ecn[queue_id],
                  sizeof(swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.ecn[i]))
          )
        {
          swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.all_queue = FALSE; 
          break;
        }
      }
	  
    }
    else if( queue_id == -1)
    {
      for(queue_id=0; queue_id < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; queue_id ++)
      {
        swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.min[queue_id] = min;
        swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.max[queue_id] = max;
        swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.drop[queue_id]= drop;
        swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.ecn[queue_id] = ecn;	
        swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.valid[queue_id]= valid; 			
        
      }
      swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect.all_queue = TRUE;
    }
    return TRUE;	
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_GetRandomDetect
 * -------------------------------------------------------------------------
 * FUNCTION: get ecn marking percentage
 * INPUT   : lport          - which port to set
 * RETURN  : random_detect_p - how percentage 
 * OUTPUT  : None
 * NOTE    : defualt is disable and percentage is 0
 * -------------------------------------------------------------------------*/
BOOL_T SWCTRL_OM_GetRandomDetect(UI32_T ifindex, SWCTRL_OM_RandomDetect_T *random_detect_p)
{
    *random_detect_p = swctrl_shmem_data_p->port_info_ex[ifindex-1].random_detect;
    return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_OM_CpyRandomDetectValue
 * -------------------------------------------------------------------------
 * FUNCTION: get ecn marking percentage
 * INPUT   : src_lport - which port to copy from
 *           target_lport - which port to copy to
 * RETURN  : 
 * OUTPUT  : None
 * NOTE    : 
 * -------------------------------------------------------------------------*/
void SWCTRL_OM_CpyRandomDetectValue(UI32_T src_lport, UI32_T target_lport)
{
    swctrl_shmem_data_p->port_info_ex[target_lport-1].random_detect = swctrl_shmem_data_p->port_info_ex[src_lport-1].random_detect;
    return;
}

#endif /*#if(SYS_CPNT_WRED == TRUE)*/

/*end of swctrl_om.c*/
