/* Module Name: STKTPLG_MGR.C
 * Purpose: This file contains the API of stack topology:
 *
 * Notes:
 *
 * History:
 *    11/21/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999-2001
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#if 0
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>
#endif
#include <stdlib.h> /* for aoti() */
#include <assert.h>
#include "stdio.h"
#include "string.h"
#include "leaf_sys.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"

#include "stktplg_type.h"
#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "stktplg_om_private.h"
#include "stktplg_engine.h"
#include "stktplg_board.h"
#include "sysfun.h"
#include "dev_swdrv.h"
#include "uc_mgr.h"
#include "leaf_es3626a.h"
#include "stktplg_shom.h"

#include "leddrv.h"
#include "phyaddr_access.h"
#include "sysdrv.h"

#include "dev_swdrv_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* static BOOL_T   STKTPLG_MGR_SetLogicalPortMapping(void);*/
#ifdef ECN430_FB2
static BOOL_T STKTPLG_MGR_SetBoardInformation();
#endif
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
static void STKTPLG_MGR_HwPortMode_UpdatePortMapping(void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static STKTPLG_BOARD_BoardInfo_T  board_info;
static STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

static BOOL_T                    set_transition_mode;

/* MACRO FUNCTIONS DECLARACTION
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_MGR_InitiateProcessResources
 * PURPOSE: This function initializes all releated variables and restarts
 *          the state machine.
 * INPUT:   None.
 * OUTPUT:  num_of_unit  -- the number of units existing in the stack.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
    STKTPLG_MGR_InitModule();
#endif

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    STKTPLG_MGR_HwPortMode_UpdatePortMapping();
#endif

    return ret;
} /* End of STKTPLG_MGR_Initiate_System_Resources() */


#if 0
/* FUNCTION NAME: STKTPLG_MGR_SetLogicalPortMapping
 * PURPOSE: This function will set the mapping of user port and physical port.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- Successfully
 *          FALSE   -- If priority is not available
 * NOTES:   None.
 *
 */
static BOOL_T STKTPLG_MGR_SetLogicalPortMapping(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    /* get system inforamtion first, we need board id to know board
     * information. From board information, we will know how to set
     * BCM driver
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    /* get board information
     */
    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, &board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }
#ifdef Use_BcmDrv

    DEV_SWDRV_SetDevicePortMapping(board_info_p->userPortMappingTable);

#if 0
    if (!BCMDRV_SetLogicalPortMapping(board_info_p->port2phyDevId_array_ptr,
                                      board_info_p->port2phyPortId_array_ptr))
    {
        return (FALSE);
    }

    if (!BCMDRV_SetPHYIdMapping(board_info_p->port2PHYId_array_ptr))
    {
        return (FALSE);
    }
    /* check if we need to set internal stack information
     */
    if ((board_info_p->stackingPort2phyDevId_ptr != NULL) &&
        (board_info_p->stackingPort2phyPortId_ptr != NULL))
    {
        /* note, we need to redefine SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT
         * it should be large enough. due to current project having no internal stacking.
         * I will not take care of this problem right now.
         */
        if (!BCMDRV_SetStackingPortMapping(SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT,
                                           board_info_p->stackingPort2phyDevId_ptr,
                                           board_info_p->stackingPort2phyPortId_ptr))
        {
            return (FALSE);
        }
    }
#endif
#else
    DEV_SWDRV_SetDevicePortMapping(board_info_p->userPortMappingTable);
#endif

    return (TRUE);

} /* End of STKTPLG_MGR_SetLogicalPortMapping() */
#endif

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
/* FUNCTION NAME: STKTPLG_MGR_SetBaseLed
 * PURPOSE: This function is used to set base LED.
 * INPUT:   base_led  -- base LED.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_BASE_MASTER_UNIT    0x10
 *          -- SYS_HWCFG_SYSTEM_BASE_SECOND_UNIT    0x20
 *          -- SYS_HWCFG_SYSTEM_BASE_OTHER_UNIT     0x30
 *
 */
BOOL_T STKTPLG_MGR_SetBaseLed(UI8_T base_led)
{
    BOOL_T  ret;

    if (base_led < SYS_HWCFG_SYSTEM_BASE_MASTER_UNIT || base_led > SYS_HWCFG_SYSTEM_BASE_OTHER_UNIT)
        return FALSE;

    ret = STKTPLG_OM_SetBaseLed(base_led);

    return ret;
} /* End of STKTPLG_MGR_SetBaseLed */

/* FUNCTION NAME: STKTPLG_MGR_SetStackLinkLed
 * PURPOSE: This function is used to set stack link LED.
 * INPUT:   stack_link_led  -- stack link LED.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_STACK_LINK_OK               0x00
 *          -- SYS_HWCFG_SYSTEM_STACK_LINK_DOWN_OR_NOSTACK  0x08
 *
 */
BOOL_T STKTPLG_MGR_SetStackLinkLed(UI8_T stack_link_led)
{
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
    BOOL_T  ret;

    if (stack_link_led != SYS_HWCFG_SYSTEM_STACK_LINK_OK && stack_link_led != SYS_HWCFG_SYSTEM_STACK_LINK_DOWN_OR_NOSTACK)
        return FALSE;

    ret = STKTPLG_OM_SetStackLinkLed(stack_link_led);

    return ret;
#else
    return TRUE;
#endif
} /* End of STKTPLG_MGR_SetStackLinkLed */
#endif /* #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1) */

/* FUNCTION NAME: STKTPLG_MGR_EnableModuleBus
 * PURPOSE: This function is used to enable module bus.
 * INPUT:   device_inde     -- device index.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   module index from 0 .. 3
 *
 */
BOOL_T STKTPLG_MGR_EnableModuleBus(UI32_T device_index, UI32_T module_index)
{
    BOOL_T  ret;

    if (!STKTPLG_OM_UnitExist(device_index))
        return FALSE;

    if (module_index >= SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT)
        return FALSE;

    ret = STKTPLG_OM_EnableModuleBus(module_index);

    return ret;
} /* End of STKTPLG_MGR_EnableModuleBus */


/* FUNCTION NAME: STKTPLG_MGR_DisableModuleBus
 * PURPOSE: This function is used to disable module bus.
 * INPUT:   device_inde     -- device index.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   module index from 0 .. 3
 *
 */
BOOL_T STKTPLG_MGR_DisableModuleBus(UI32_T device_index, UI32_T module_index)
{
    BOOL_T  ret;

    if (!STKTPLG_OM_UnitExist(device_index))
        return FALSE;

    if (module_index >= SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT)
        return FALSE;

    ret = STKTPLG_OM_DisableModuleBus(module_index);

    return ret;
} /* End of STKTPLG_MGR_DisableModuleBus */


/* FUNCTION NAME: STKTPLG_MGR_SetModuleABus
 * PURPOSE: This function is used to set module 1 bus enable/disable.
 * INPUT:   module_bus  -- module bus.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_MODULE_1_BUS_ENABLE   0x00
 *          -- SYS_HWCFG_SYSTEM_MODULE_1_BUS_DISABLE  0x04
 *
 */
BOOL_T STKTPLG_MGR_SetModuleABus(UI8_T module_bus)
{
    BOOL_T  ret;

    if (module_bus != SYS_HWCFG_SYSTEM_MODULE_1_BUS_ENABLE && module_bus != SYS_HWCFG_SYSTEM_MODULE_1_BUS_DISABLE)
        return FALSE;

    ret = STKTPLG_OM_SetModuleABus(module_bus);

    return ret;
} /* End of STKTPLG_MGR_SetModuleABus */


/* FUNCTION NAME: STKTPLG_MGR_SetModuleBBus
 * PURPOSE: This function is used to set module 2 bus enable/disable.
 * INPUT:   module_bus  -- module bus.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   value as following
 *          -- SYS_HWCFG_SYSTEM_MODULE_2_BUS_ENABLE   0x00
 *          -- SYS_HWCFG_SYSTEM_MODULE_2_BUS_DISABLE  0x02
 *
 */
BOOL_T STKTPLG_MGR_SetModuleBBus(UI8_T module_bus)
{
    BOOL_T  ret;

    if (module_bus != SYS_HWCFG_SYSTEM_MODULE_2_BUS_ENABLE && module_bus != SYS_HWCFG_SYSTEM_MODULE_2_BUS_DISABLE)
        return FALSE;

    ret = STKTPLG_OM_SetModuleBBus(module_bus);

    return ret;
} /* End of STKTPLG_MGR_SetModuleBBus */


/* FUNCTION NAME: STKTPLG_MGR_GetStackingPortLinkStatusByUnitId
 * PURPOSE: This function is used to get link status of the stacking ports on a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_link_status  -- phy status of stacking up port
 *          down_link_status-- phy status of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- fail to get link status.
 * NOTES:   
 */
BOOL_T STKTPLG_MGR_GetStackingPortLinkStatusByUnitId(
            UI32_T unit_id, BOOL_T *up_link_status, BOOL_T *down_link_status)
{ 
    UI32_T unit;
    STKTPLG_TYPE_Stacking_Port_Option_T stk_port_option;   
    /*do not refer to snapshot, which is not updated when RING<-> LINE altering.*/
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    
    if (up_link_status == NULL || down_link_status ==NULL)
    {
        printf("%s(%d)up_port_p or down_port_p is NULL\r\n", __FUNCTION__,
            __LINE__);    
        return FALSE;
    }
    if (!STKTPLG_OM_UnitExist(unit_id))
        return FALSE;

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    stk_port_option = ctrl_info_p->stacking_port_option;
    if(stk_port_option == STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
        return FALSE;
#endif

    *up_link_status = *down_link_status = FALSE;

    /*If standalone or master, get the link status from SHOM.*/
    if(ctrl_info_p->total_units == 1 || unit_id == ctrl_info_p->master_unit_id)
    {
        UI32_T up_state,  down_state;

        if(FALSE==STKTPLG_SHOM_GetHgPortLinkState(&up_state, &down_state))
        {
            printf("%s:STKTPLG_SHOM_GetHgPortLinkState fail\r\n", __FUNCTION__);
            return FALSE;
        }

        if(up_state == DEV_SWDRV_PORT_LINK_UP)
            *up_link_status   = TRUE;
       
        if(down_state == DEV_SWDRV_PORT_LINK_UP)
            *down_link_status = TRUE;
            
        return TRUE;            
    }
    else
    {        
        for (unit = 0; unit < ctrl_info_p->total_units_up; unit++)
        {    
            if(ctrl_info_p->stable_hbt_up.payload[unit].unit_id == unit_id)
            {
                UI8_T stacking_ports_link_status = 
                    ctrl_info_p->stable_hbt_up.payload[unit].stacking_ports_link_status;

                *up_link_status =  (stacking_ports_link_status & LAN_TYPE_TX_UP_LINK)? TRUE:FALSE;
                *down_link_status = (stacking_ports_link_status & LAN_TYPE_TX_DOWN_LINK)? TRUE:FALSE;
                return TRUE;
            }
        }
        
        for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
        {
            if(ctrl_info_p->stable_hbt_down.payload[unit].unit_id == unit_id)
            {
                UI8_T stacking_ports_link_status = 
                    ctrl_info_p->stable_hbt_down.payload[unit].stacking_ports_link_status;
                *up_link_status =  (stacking_ports_link_status & LAN_TYPE_TX_UP_LINK)? TRUE:FALSE;
                *down_link_status = (stacking_ports_link_status & LAN_TYPE_TX_DOWN_LINK)? TRUE:FALSE;
                return TRUE;
            }
        }
    }
    return FALSE;
}


/* FUNCTION NAME: STKTPLG_MGR_GetStackingUserPortByUnitId
 * PURPOSE: This function is used to get the stacking ports in user port id for a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_uport_p  -- user port index of stacking up port
 *          down_uport_p-- user port index of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure. Input parameter error ,or no stacking port on that unit.
 * NOTES:   Get board_id and find the stacking port index. 
 *          Check the port_type to tell if the unit gets stacking port on it.
 */
BOOL_T STKTPLG_MGR_GetStackingUserPortByUnitId(UI32_T unit_id, UI32_T *up_uport_p, UI32_T *down_uport_p)
{ 
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI8_T tmp_board_id;
    UI32_T port;
    STK_UNIT_CFG_T device_info;    
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    STKTPLG_TYPE_Stacking_Port_Option_T stk_port_option;

    if (up_uport_p == NULL || down_uport_p ==NULL)
    {
        printf("%s(%d)up_port_p or down_port_p is NULL\r\n", __FUNCTION__,
            __LINE__);    
        return FALSE;
    }

    *up_uport_p = *down_uport_p =0;  

    if (!STKTPLG_OM_UnitExist(unit_id))
        return FALSE; 

    STKTPLG_OM_GetDeviceInfo(unit_id, &device_info);
    tmp_board_id = device_info.board_info.board_id;

    if (!STKTPLG_BOARD_GetBoardInformation(tmp_board_id, board_info_p))
    {
        printf("Can not get related board information. bid:%u\r\n", tmp_board_id);
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

   *up_uport_p = board_info_p->stacking_uplink_port;
   *down_uport_p = board_info_p->stacking_downlink_port;
    
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    stk_port_option = ctrl_info_p->stacking_port_option;

    if(stk_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
    {
        UI8_T up_port_type ,down_port_type;
        
        UI8_T  *stackingPort2phyDevId_ar = 
            &(board_info_p->stackingPortUserConfiguration[stk_port_option-1].stackingPort2phyDevId_ar[0]);

        UI8_T  *stackingPort2phyPortId_ar = 
            &(board_info_p->stackingPortUserConfiguration[stk_port_option-1].stackingPort2phyPortId_ar[0]);

        /*Get the dev_id and phy_port of stacking port from board_info*/
        UI8_T   up_port_device_id = stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1];
        UI8_T   dn_port_device_id = stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1];

        UI8_T   up_port_phy_port = stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1];  
        UI8_T   dn_port_phy_port = stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1];  

        DEV_SWDRV_Device_Port_Mapping_T local_port_mapping[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
        STKTPLG_OM_GetPortMapping(local_port_mapping, unit_id);

        /*translate phy_port to user port by scanning mapping table.*/    
        for (port = 0; port < board_info_p->max_port_number_of_this_unit; port++)
        {
            if (ctrl_info_p->stacking_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
            {
                if ((local_port_mapping[port].device_id == up_port_device_id) &&
                    (local_port_mapping[port].device_port_id == up_port_phy_port))
                {
                    *up_uport_p = port + 1;
                }
                else if ((local_port_mapping[port].device_id == dn_port_device_id) &&
                        (local_port_mapping[port].device_port_id == dn_port_phy_port))
                {
                    *down_uport_p = port + 1;
                }
            }
        }

        /*check port_type*/
        up_port_type = local_port_mapping[*up_uport_p-1].port_type;
        down_port_type = local_port_mapping[*down_uport_p-1].port_type;        
        if(up_port_type != STKTPLG_PORT_TYPE_STACKING)
            *up_uport_p = 0;

        if(down_port_type != STKTPLG_PORT_TYPE_STACKING)
            *down_uport_p = 0;       
            
    }
#endif

    if( *up_uport_p == 0 && *down_uport_p == 0)
        return FALSE;
    else
        return TRUE;    
}    

/* FUNCTION NAME: STKTPLG_MGR_GetSwitchInfo
 * PURPOSE: This function is used to get this switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info)
{
    STKTPLG_OM_Info_T  board_info;
    UI32_T          module_type;
    UI32_T          port_number;
    UI8_T           master_unit_id;
    BOOL_T          is_unit_status_normal;
#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_EVENT_TrapData_T trap_data;
    STKTPLG_OM_Info_T board_info_master;
#endif

    if (switch_info == 0)
        return FALSE;

    if (!STKTPLG_OM_UnitExist(switch_info->sw_unit_index))
        return FALSE;

    if (!STKTPLG_OM_GetSysInfo(switch_info->sw_unit_index, &board_info))
        return FALSE;

    strncpy((char *)switch_info->sw_hardware_ver, (char *)board_info.mainboard_hw_ver, sizeof(switch_info->sw_hardware_ver));
    memset((char *)switch_info->sw_microcode_ver, 0, SYS_ADPT_HW_VER_STR_LEN + 1);
    strncpy((char *)switch_info->sw_loader_ver, (char *)board_info.loader_ver, sizeof(switch_info->sw_loader_ver));
    strncpy((char *)switch_info->sw_kernel_ver, (char *)board_info.kernel_ver, sizeof(switch_info->sw_kernel_ver));
    strncpy((char *)switch_info->sw_boot_rom_ver, (char *)board_info.post_ver, sizeof(switch_info->sw_boot_rom_ver));
    strncpy((char *)switch_info->sw_opcode_ver, (char *)board_info.runtime_sw_ver, sizeof(switch_info->sw_opcode_ver));
    strncpy((char *)switch_info->sw_epld_ver, (char *)board_info.epld_ver, sizeof(switch_info->sw_epld_ver));
    /*1231 */
   /* memcpy((UI8_T *)switch_info->module_runtime_fw_ver,(UI8_T *)device_info.module_runtime_fw_ver ,SYS_ADPT_FW_VER_STR_LEN + 1); */
    strncpy((char *)switch_info->sw_model_number, (char *)board_info.model_number, sizeof(switch_info->sw_model_number));
    STKTPLG_OM_GetMaxPortCapability(switch_info->sw_unit_index, &port_number);
    switch_info->sw_port_number = port_number;
    STKTPLG_OM_GetMasterUnitId(&master_unit_id);

    if (TRUE == STKTPLG_OM_GetUnitStatus(switch_info->sw_unit_index, &is_unit_status_normal) &&
        TRUE == is_unit_status_normal)
    {
        switch_info->sw_validation = STKTPLG_MGR_SWITCH_VALID;
    }
    else
    {
#if (SYS_CPNT_TRAPMGMT == TRUE)
        trap_data.trap_type = TRAP_EVENT_MAIN_BOARD_VER_MISMATCH;
        trap_data.u.main_board_ver_mismatch.instance_swOpCodeVerMaster[0] = master_unit_id;
        if (!STKTPLG_OM_GetSysInfo(master_unit_id, &board_info_master))
            return FALSE;
        strncpy((char *)trap_data.u.main_board_ver_mismatch.swOpCodeVerMaster,
                (char *)board_info_master.runtime_sw_ver, sizeof(trap_data.u.main_board_ver_mismatch.swOpCodeVerMaster));
        trap_data.u.main_board_ver_mismatch.instance_swOpCodeVerSlave[0] = switch_info->sw_unit_index;
        strncpy((char *)trap_data.u.main_board_ver_mismatch.swOpCodeVerSlave,
               (char *)board_info_master.runtime_sw_ver, sizeof(trap_data.u.main_board_ver_mismatch.swOpCodeVerSlave));
        TRAP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
        switch_info->sw_validation = STKTPLG_MGR_SWITCH_INVALID;
    }

    if (switch_info->sw_unit_index == master_unit_id)
        switch_info->sw_role_in_system = VAL_swRoleInSystem_master;
    else
        switch_info->sw_role_in_system = VAL_swRoleInSystem_slave;

    strncpy((char *)switch_info->sw_serial_number, (char *)board_info.serial_no, sizeof(switch_info->sw_serial_number));
    /* modified by Vincent on 5,Nov,04
     * For Products with module supported such as Hagrid
     */
#if (SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT > 0)
    {
        if (STKTPLG_OM_GetModuleType(switch_info->sw_unit_index, 1, &module_type))
        {
            switch(module_type)
            {
                case VAL_swModuleType_eightPortSfpModule:
                    switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_eightPortSfpModule;
                    break;
                case VAL_swModuleType_tenGigaPortModule:
                    switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_tenGigaPortModule;
                    break;
                default:
                    switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_other;
                    break;
           }
        }
        else
        {
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_notPresent;
        }
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_notPresent;
    }
#else
    /* Vincent: For Products without module supported such as Solberg/DrawinR2
     */
    {
        switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_notPresent ;
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_notPresent ;
    }
#endif
    /* modifiled by Vincent on 5,Aug,04
     * The following code is only for 3526's slot in module
     */
#if 0
    device_info.module_type[0] &= SYS_HWCFG_MODULE_ID_MASK;  /* 0xf8 */
    device_info.module_type[1] &= SYS_HWCFG_MODULE_ID_MASK;

    switch (device_info.module_type[0])
    {
        case SYS_HWCFG_MODULE_ID_100BASE_FX_SC_MMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_hundredBaseFxScMmf;
            break;
        case SYS_HWCFG_MODULE_ID_100BASE_FX_SC_SMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_hundredBaseFxScSmf;
            break;
        case SYS_HWCFG_MODULE_ID_100BASE_FX_MTRJ_MMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_hundredBaseFxMtrjMmf;
            break;
        case SYS_HWCFG_MODULE_ID_1000BASE_SX_SC_MMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseSxScMmf;
            break;
        case SYS_HWCFG_MODULE_ID_1000BASE_SX_MTRJ_MMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseSxMtrjMmf;
            break;
        case SYS_HWCFG_MODULE_ID_1000BASE_X_GBIC:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseXGbic;
            break;
        case SYS_HWCFG_MODULE_ID_1000BASE_LX_SC_SMF:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseLxScSmf;
            break;
        case SYS_HWCFG_MODULE_ID_1000BASE_T:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseT;
            break;
        case SYS_HWCFG_MODULE_ID_STACKING_MODULE:
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK == 1)
            /* stacking module is invalid when max allowable unit number is 1
             * Charles: 2003/4/20
             */
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_other;
#else

            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_stackingModule;
#endif
            break;
        case SYS_HWCFG_MODULE_ID_MINIGBIC:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_thousandBaseSfp;
            break;

        case SYS_HWCFG_MODULE_ID_1_PORT_10_100BASE:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_tenHundredBaseT;
            break;

        case SYS_HWCFG_MODULE_ID_MASK:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_notPresent;
            break;
        default:
            switch_info->sw_expansion_slot1 = VAL_swExpansionSlot1_other;
            break;
    }

    switch (device_info.module_type[1])
    {
    case SYS_HWCFG_MODULE_ID_100BASE_FX_SC_MMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_hundredBaseFxScMmf;
        break;
    case SYS_HWCFG_MODULE_ID_100BASE_FX_SC_SMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_hundredBaseFxScSmf;
        break;
    case SYS_HWCFG_MODULE_ID_100BASE_FX_MTRJ_MMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_hundredBaseFxMtrjMmf;
        break;
    case SYS_HWCFG_MODULE_ID_1000BASE_SX_SC_MMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseSxScMmf;
        break;
    case SYS_HWCFG_MODULE_ID_1000BASE_SX_MTRJ_MMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseSxMtrjMmf;
        break;
    case SYS_HWCFG_MODULE_ID_1000BASE_X_GBIC:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseXGbic;
        break;
    case SYS_HWCFG_MODULE_ID_1000BASE_LX_SC_SMF:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseLxScSmf;
        break;
    case SYS_HWCFG_MODULE_ID_1000BASE_T:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseT;
        break;
    case SYS_HWCFG_MODULE_ID_STACKING_MODULE:
        /* No matter in whic case, module B is not allowed to be inserted stacking module
         * Charles: 2003/4/20
         */
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_other;
        break;
    case SYS_HWCFG_MODULE_ID_MINIGBIC:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_thousandBaseSfp;
        break;
    case SYS_HWCFG_MODULE_ID_4_PORT_10_100BASE:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_tenHundredBaseT4port;
        break;
    case SYS_HWCFG_MODULE_ID_4_PORT_100BASE_FX_MTRJ:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_tenHundredBaseFxMtrj4port;
        break;
    case SYS_HWCFG_MODULE_ID_1_PORT_10_100BASE:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_tenHundredBaseT;
        break;
    case SYS_HWCFG_MODULE_ID_MASK:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_notPresent;
        break;
    default:
        switch_info->sw_expansion_slot2 = VAL_swExpansionSlot2_other;
        break;
    }
#endif

    if (!STKTPLG_OM_GetSwServiceTag(switch_info->sw_unit_index, switch_info->sw_service_tag))
        return FALSE;
    if (!STKTPLG_OM_GetSwChassisServiceTag(switch_info->sw_unit_index, switch_info->sw_chassis_service_tag))
        return FALSE;
    if (!STKTPLG_OM_GetSwIdentifier(switch_info->sw_unit_index, &(switch_info->sw_identifier)))
        return FALSE;
    if (!STKTPLG_OM_GetModuleExpRuntimeFwVer(switch_info->sw_unit_index, switch_info->sw_module_expected_opcode_ver))
        return FALSE;

    /* copy software build time from OM
     */
    switch_info->sw_software_build_time = board_info.software_build_time;

    return TRUE;
} /* End of STKTPLG_MGR_GetSwitchInfo */


/* FUNCTION NAME: STKTPLG_MGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- next switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetNextSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info)
{
    UI32_T  unit_id;

    if (switch_info == NULL)
        return FALSE;

    unit_id = (UI32_T) switch_info->sw_unit_index;

    if (!STKTPLG_OM_GetNextUnit(&unit_id))
    {
        return FALSE;
    }

    switch_info->sw_unit_index = (UI8_T) unit_id;

    return (STKTPLG_MGR_GetSwitchInfo(switch_info));

} /* End of STKTPLG_MGR_GetNextSwitchInfo */ /* End of STKTPLG_MGR_GetNextSwitchInfo */



/* FUNCTION NAME: STKTPLG_MGR_GetSwitchModuleInfoEntry
 * PURPOSE: This function is used to get this module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry)
{
    BOOL_T          ret;
    STKTPLG_OM_switchModuleInfoEntry_T  module_info;
    UI32_T          unit_id,module_index;
    BOOL_T          is_module_status_normal;
    UI32_T          module_type;
#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_EVENT_TrapData_T trap_data;
    STKTPLG_MGR_Switch_Info_T switch_info;
#endif

    if (entry == NULL)
        return FALSE;

    if (!STKTPLG_OM_UnitExist(entry->unit_index))
        return FALSE;

    unit_id = entry->unit_index;
    module_index = entry->module_index; /* Added by Vincent Yang on 12,Oct,2004
                                         * module_index is an argument from CLI
                                         * and should not be cleared
                                         */
    memset((STKTPLG_MGR_switchModuleInfoEntry_T *) entry, 0, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
    entry->unit_index = unit_id;
    entry->module_index = module_index;

    ret = STKTPLG_OM_GetModuleInfo(entry->unit_index, entry->module_index, &module_info);

    if( FALSE == ret )
    {
        return FALSE;
    }

    memcpy(entry, &module_info, sizeof(STKTPLG_MGR_switchModuleInfoEntry_T));
    memset((UI8_T *)entry->microcode_ver, 0, SYS_ADPT_HW_VER_STR_LEN + 1);
    entry->port_number = 1;

    STKTPLG_OM_GetModuleStatus(unit_id, 1, &is_module_status_normal);

    if (TRUE == STKTPLG_OM_GetModuleStatus(unit_id, 1, &is_module_status_normal) &&
        TRUE == is_module_status_normal)
    {
        entry->module_validation = STKTPLG_MGR_MODULE_VALID;
    }
    else
    {
#if (SYS_CPNT_TRAPMGMT == TRUE)
        trap_data.trap_type = TRAP_EVENT_MODULE_VER_MISMATCH;
        trap_data.u.module_ver_mismatch.instance_swModuleExpectedOpCodeVer[0] = unit_id;

        STKTPLG_MGR_GetSwitchInfo(&switch_info);
        strncpy((char *)trap_data.u.module_ver_mismatch.swModuleExpectedOpCodeVer,
                (char *)switch_info.sw_module_expected_opcode_ver, sizeof(trap_data.u.module_ver_mismatch.swModuleExpectedOpCodeVer));
        trap_data.u.module_ver_mismatch.instance_swModuleOpCodeVer[0] = unit_id;
        trap_data.u.module_ver_mismatch.instance_swModuleOpCodeVer[1] = module_index;
        strncpy((char *)trap_data.u.module_ver_mismatch.swModuleOpCodeVer,
                (char *)module_info.op_code_ver, sizeof(trap_data.u.module_ver_mismatch.swModuleOpCodeVer));
        TRAP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
        entry->module_validation = STKTPLG_MGR_MODULE_INVALID;
    }

    if (TRUE == STKTPLG_OM_GetModuleType(unit_id, 1, &module_type))
    {
        entry->module_type = module_type;
    }
    else
    {
        /* 0 for error
         */
        entry->module_type = 0;
    }

    return ret;
} /* End of STKTPLG_MGR_GetSwitchModuleInfoEntry */



/* FUNCTION NAME: STKTPLG_MGR_SetModuleInfo
 * PURPOSE: This function is used to set ModuleInfo.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: This API is currently used by Sysdrv to set xenpak_status field
 *
 */
BOOL_T STKTPLG_MGR_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info)
{

    return STKTPLG_OM_SetModuleInfo(unit, module_index, module_info);

} /* End of STKTPLG_MGR_SetModuleInfo */


/* FUNCTION NAME: STKTPLG_MGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetNextSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry)
{
    BOOL_T  over,ret;

    if (entry == NULL)
        return FALSE;

    over = FALSE;
    if (entry->unit_index ==0)
    {
        over = !STKTPLG_OM_GetNextUnit(&entry->unit_index);
        entry->module_index = 0;
    }
    ret=FALSE;
    while (over==FALSE && ret==FALSE)
    {
        if (++entry->module_index > STKTPLG_OM_MAX_MODULE_NBR)
        {
            over = !STKTPLG_OM_GetNextUnit(&entry->unit_index);
            if (over==TRUE)
                return FALSE;
        
            entry->module_index = 1;
        }
        ret = STKTPLG_MGR_GetSwitchModuleInfoEntry(entry);
    }
    return (ret);

} /* End of STKTPLG_MGR_GetNextSwitchModuleInfoEntry */

/* FUNCTION NAME: STKTPLG_MGR_SetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 1 to set the first unit information.
 *
 */
BOOL_T STKTPLG_MGR_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{

#if 0 /* we are not in master mode yet, so we'll be returned FLASE */
    if (!STKTPLG_MGR_UnitExist(unit))
        return FALSE;
#endif

    if (device_info == NULL)
        return FALSE;

    return STKTPLG_OM_SetDeviceInfo(unit, device_info);

} /* End of STKTPLG_MGR_SetDeviceInfo */

/* FUNCTION NAME: STKTPLG_MGR_ResetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 1 to set the first unit information.
 *
 */
BOOL_T STKTPLG_MGR_ResetDeviceInfo(void)
{
    return STKTPLG_OM_ResetDeviceInfo();
} /* End of STKTPLG_MGR_ResetDeviceInfo */

/* FUNCTION NAME: STKTPLG_MGR_RemoveModule
 * PURPOSE: This function is used to remove slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_RemoveModule(UI32_T unit, UI32_T module_index)
{

    if (!STKTPLG_OM_UnitExist(unit))
        return FALSE;

    return STKTPLG_OM_RemoveModule(unit, module_index);
} /* End of STKTPLG_MGR_RemoveModule */

/* FUNCTION NAME: STKTPLG_MGR_InsertModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 *          module_type     -- which module type be inserted.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_InsertModule(UI32_T unit, UI32_T module_index, UI32_T module_type)
{
    BOOL_T  ret;

    if (!STKTPLG_OM_UnitExist(unit))
        return FALSE;

    ret = STKTPLG_OM_InsertModule(unit, module_index, module_type);

    return ret;

} /* End of STKTPLG_MGR_InsertModule */


/* FUNCTION NAME: STKTPLG_MGR_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_MGR_SlaveReady(BOOL_T ready)
{

    STKTPLG_OM_SlaveReady(ready);

    return;
}

/* FUNCTION NAME: STKTPLG_MGR_SetEntPhysicalAlias
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_alias_p -- entity physical index.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_MGR_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias_p)
{
    return STKTPLG_OM_SetEntPhysicalAlias(ent_physical_index, ent_physical_alias_p);
}

/* FUNCTION NAME: STKTPLG_MGR_SetEntPhysicalSeriaNum
 * PURPOSE: Set the EntPhysicalSeriaNum field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_seriaNum_p -- entity physical serial num.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_MGR_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum_p)
{
    return STKTPLG_OM_SetEntPhysicalSeriaNum(ent_physical_index, ent_physical_seriaNum_p);
}

/* FUNCTION NAME: STKTPLG_MGR_SetEntPhysicalAssetId
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_asset_id_p -- entity physical asset id.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_MGR_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id_p)
{
    return STKTPLG_OM_SetEntPhysicalAssetId(ent_physical_index, ent_physical_asset_id_p);
}

/* The three function is for mutli-chips in one device */
/* FUNCTION NAME: STKTPLG_MGR_UPortToChipDevId
 * PURPOSE: Trasfer the logical port to physical chip device id.
 * INPUT:   u_port   -- user port(only include normal port).
 * OUTPUT:  *chip_dev_id  -- chip device id.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_MGR_UPortToChipDevId(UI32_T u_port, UI32_T *chip_dev_id)
{
    UC_MGR_Sys_Info_T         uc_sys_info;


    /* get system inforamtion first, we need board id to know board
     * information. From board information, we will know how to set
     * BCM driver
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    /* get board information
     */
    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    if (u_port == 0 || u_port > board_info_p->max_port_number_of_this_unit)
        return FALSE;

    *chip_dev_id = board_info_p->userPortMappingTable[0][u_port-1].phy_id;

    return TRUE;

}/* end of STKTPLG_MGR_UPortToChipDevId */

/* FUNCTION NAME: STKTPLG_MGR_SetStackRoleLed
 * PURPOSE: This function is used to set the role which the unit is playing
 * INPUT:   stk_role
 *          LEDDRV_STACK_ARBITRATION
 *          LEDDRV_STACK_PRE_MASTER
 *          LEDDRV_STACK_MASTER
 *          LEDDRV_STACK_SLAVE
 *          defined in leddrv.h
 * OUTPUT:  none
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetStackRoleLed(UI32_T stk_role,UI32_T up_phy_status,UI32_T down_phy_status )
{
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK == 1) /* Yongxin.Zhao added, 2009-06-15, 15:13:58 */
    return TRUE;
#else
    STKTPLG_OM_Ctrl_Info_T *ctrl_info = STKTPLG_OM_ENG_GetCtrlInfo();
    LEDDRV_System_Status_T ledStatus;
    UI32_T stacking_link;
    UI32_T up_port=0,down_port=0, dev_id;
/*EPR: ES3628BT-FLF-ZZ-00079
   Problem:stacking link led check fail
   RootCause:when stacking button is non-pushed, still shine led
                         not check the uplink/downlink ports states
   Solution:check the stacking button,and uplink/down link receive pkt state
   File:stktplg_mgr.c
*/
#define STKTPLG_MGR_IS_UPLINK_PORT_OK(LINK_STATUS)   (((LINK_STATUS)&LAN_TYPE_TX_UP_LINK) ?   TRUE : FALSE)
#define STKTPLG_MGR_IS_DOWNLINK_PORT_OK(LINK_STATUS) (((LINK_STATUS)&LAN_TYPE_TX_DOWN_LINK) ? TRUE : FALSE)

    ledStatus.master = stk_role;

    stacking_link = LEDDRV_STACKING_LINK_BOTH_UP;

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    /*BT FS requires that when the stacking is not enable, the stacking link shoule be off*/
    if(!ctrl_info->stacking_button_state)
        stacking_link = LEDDRV_STACKING_LINK_NO_STACK;
    else
#endif
    {

        /*    down/up                   OK                                Disconnect                           Fail
              OK                            Green                            Amber                                   Green blinking
              Disconnect              Amber                           off                                           Green blinking
              Fail                           Amber Blinking            Amber Blinking                     Amber and Green Alternate

         */
        /*up is down ,down port is link up*/
        if ( (FALSE == ctrl_info->up_phy_status) && (TRUE == ctrl_info->down_phy_status) )
        {
            /*up is down ,and down port is linkup but down port fail*/
            if(!STKTPLG_MGR_IS_DOWNLINK_PORT_OK(ctrl_info->stacking_ports_logical_link_status))
                stacking_link = LEDDRV_STACKING_LINK_ONLY_RX_UP;/*Amber flash*/
            else/*up is down ,and down port is linkup but down port  is ok*/
                stacking_link = LEDDRV_STACKING_LINK_ONESIDE_UNCONNECT;
        }

        if ( (TRUE == ctrl_info->up_phy_status) && (FALSE == ctrl_info->down_phy_status) )
        {
            /*downport is link down ,and up port is linkup but up port fail*/
            if(!STKTPLG_MGR_IS_UPLINK_PORT_OK(ctrl_info->stacking_ports_logical_link_status))
                stacking_link = LEDDRV_STACKING_LINK_ONLY_TX_UP; /*green flash*/
            else/*downport is link down ,and up port is linkup but up port is ok*/
                stacking_link = LEDDRV_STACKING_LINK_ONESIDE_UNCONNECT;
        }

        if ( (TRUE == ctrl_info->up_phy_status) && (TRUE == ctrl_info->down_phy_status) )
        {
            if(!STKTPLG_MGR_IS_UPLINK_PORT_OK(ctrl_info->stacking_ports_logical_link_status))
            {
                /*downport and upport are both  link up ,but  upport and downport are both fail*/
                if(!STKTPLG_MGR_IS_DOWNLINK_PORT_OK(ctrl_info->stacking_ports_logical_link_status))
                    stacking_link = LEDDRV_STACKING_LINK_BOTH_DOWN; /*green and amber alternate*/
                else /*downport and upport are both  link up ,but  upport  is fail ,and downport is ok*/
                    stacking_link = LEDDRV_STACKING_LINK_ONLY_TX_UP;
            }
            else
            {
                /*downport and upport are both  link up ,but  downport  is fail ,and upport is ok*/
                if(!STKTPLG_MGR_IS_DOWNLINK_PORT_OK(ctrl_info->stacking_ports_logical_link_status))
                    stacking_link = LEDDRV_STACKING_LINK_ONLY_RX_UP;
                else/*downport and upport are both  link up ,and upport and downport are both ok*/
                    stacking_link = LEDDRV_STACKING_LINK_BOTH_UP;/*Green*/
            }
        }
        /*downport and upport are both  link down*/
        if ( (FALSE == ctrl_info->up_phy_status) && (FALSE == ctrl_info->down_phy_status) )
            stacking_link = LEDDRV_STACKING_LINK_NO_STACK;

     /*light the stacking LED*/
     if((ctrl_info->provision_completed_state && ctrl_info->state == STKTPLG_STATE_SLAVE)||
        (ctrl_info->state == STKTPLG_STATE_MASTER &&STKTPLG_OM_ENG_ProvisionCompletedOnce()))
     {
            if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_UP_LINK, &dev_id, &up_port) == FALSE)
            {
                goto exit;
            }
            if (STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_DOWN_LINK, &dev_id, &down_port) == FALSE)
            {
                goto exit;
            }
      }
    }

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    /* Show Stacking Link LED */
    LEDDRV_SetStackingLinkStatus(ctrl_info->my_unit_id,stacking_link,ctrl_info->stacking_button_state,up_port,up_phy_status,down_port,down_phy_status);
#else
    LEDDRV_SetStackingLinkStatus(ctrl_info->my_unit_id,stacking_link,TRUE,up_port,up_phy_status,down_port,down_phy_status);
#endif

exit:
    return(LEDDRV_SetSystemStatus(0xffffffff, ledStatus));
#endif /* ES3526MA_POE_7LF_LN */
}

/* FUNCTION NAME: STKTPLG_MGR_SetTransitionMode
 * PURPOSE: This function is used to let stack tplg result TCN
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *          Master only
 */
void STKTPLG_MGR_SetTransitionMode(void)
{
    STKTPLG_ENGINE_TCN(FALSE);
}


/* FUNCTION NAME: STKTPLG_MGR_IsSetTransitionMode
 * PURPOSE: This function is used to query whether go to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Be asked to enter transition mode
 *          FALSE : Nothing
 * NOTES:
 *          For Stack Topology only
 */
BOOL_T STKTPLG_MGR_IsSetTransitionMode(void)
{
    if (set_transition_mode)
    {
        set_transition_mode = FALSE;
        return (TRUE);
    }

    return FALSE;
}

/* FUNCTION NAME: STKTPLG_MGR_SetPortMapping
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *          Master only
 */
BOOL_T STKTPLG_MGR_SetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit)
{
    STKTPLG_OM_SetPortMapping(mapping, unit);

    return (TRUE);
}
void STKTPLG_MGR_Transition(BOOL_T ready)
{

    STKTPLG_OM_Transition(ready);

    return;
}

void STKTPLG_MGR_ProvisionCompleted(BOOL_T ready)
{
    STKTPLG_OM_ProvisionCompleted(ready);

    return;
}

#if 0
/* FUNCTION NAME: STKTPLG_MGR_GetAllUnitsPortMapping
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *            Master only
 */
BOOL_T STKTPLG_MGR_GetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping)
{
    STKTPLG_MGR_LOCK();
    STKTPLG_OM_GetAllUnitsPortMapping(mapping);
    STKTPLG_MGR_UNLOCK();
    return (TRUE);
}

BOOL_T STKTPLG_MGR_SetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping)
{
    BOOL_T  ret;

    STKTPLG_MGR_LOCK();
    ret=STKTPLG_OM_SetAllUnitsPortMapping(mapping);
    STKTPLG_MGR_UNLOCK();
    return (ret);
}
#endif

/* FUNCTION NAME : STKTPLG_MGR_SetStackingDB
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetStackingDB(void)
{
    return STKTPLG_OM_SetStackingDB();
} /* End of STKTPLG_OM_GetStackingDB() */

/* FUNCTION NAME : STKTPLG_MGR_SetStackingDBEntry
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry)
{
    return STKTPLG_OM_SetStackingDBEntry(db, entry);
} /* End of STKTPLG_MGR_SetStackingDBEntry() */

/* FUNCTION NAME: STKTPLG_MGR_UnitIDReNumbering
 * PURPOSE: This function is to result STKTPLG TCN and reassigned unit ID to all units
 *          based on the stacking spec.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE   -- Renumbering work (not standalone or unit ID not equal to 1)
 *          FALSE  -- otherwise
 * NOTES:
 *
 */

BOOL_T STKTPLG_MGR_UnitIDReNumbering(void)
{
    return STKTPLG_ENGINE_TCN(TRUE);
} /* End of STKTPLG_MGR_UnitIDReNumbering */

/* FUNCTION NAME: STKTPLG_MGR_InsertExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_InsertExpModule(UI8_T unit_id)
{
     return TRUE;

} /* End of STKTPLG_MGR_InsertModule */

/* FUNCTION NAME: STKTPLG_MGR_RemoveExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_RemoveExpModule(UI8_T unit_id)
{

    return STKTPLG_OM_RemoveExpModule(unit_id);

} /* End of STKTPLG_MGR_RemoveExpModule */

/* FUNCTION NAME: STKTPLG_MGR_UpdateModuleStateChanged
 * PURPOSE: This API is used to get and put dirty bit as FALSE.
 *          i.e. clear after read.
 * INPUT:   dirty  -- a array of dirty table.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None.
 */
BOOL_T STKTPLG_MGR_UpdateModuleStateChanged(BOOL_T status_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    return STKTPLG_OM_UpdateModuleStateChanged(status_dirty);
}

/* FUNCTION NAME: STKTPLG_MGR_SyncModuleProcess
 * PURPOSE: To sync the module status.
 * INPUT:   unit_id --- Module of which unit.
 * OUTPUT:  *is_insertion   -- insertion or removal.
 *          *starting_port  -- starting port ID of changed module.
 *          *number_of_port -- port number of changed module.
 *          *use_default    -- use default setting if insertion. I
 *                             Ignore this argument if removal.
 * RETUEN:  TRUE  -- Need to process.
 *          FALSE -- Not necessary to process.
 * NOTES:   None.
 */
BOOL_T STKTPLG_MGR_SyncModuleProcess(UI32_T unit_id,
                                     BOOL_T *is_insertion,
                                     UI32_T *starting_port,
                                     UI32_T *number_of_port,
                                     BOOL_T *use_default)
{
    return STKTPLG_OM_SyncModuleProcess(unit_id,
                                        is_insertion,
                                        starting_port,
                                        number_of_port,
                                        use_default);
}
#ifdef ECN430_FB2
/* FUNCTION NAME: STKTPLG_MGR_SetPortMapping
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *          Master only
 */
static BOOL_T STKTPLG_MGR_SetBoardInformation()
{
    UC_MGR_Sys_Info_T         uc_sys_info;
    STKTPLG_BOARD_BoardInfo_T board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    STKTPLG_BOARD_BoardInfo_T board_info_p_tenG;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }
    board_info_p_tenG = *board_info_p;

    board_info_p_tenG.max_port_number_on_board = 27;
    board_info_p_tenG.max_port_number_of_this_unit= 27;
    board_info_p_tenG.start_expansion_port_number =28;

    return (STKTPLG_BOARD_SetBoardInformation(uc_sys_info.board_id, board_info_p_tenG));

}
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_MGR_ProcessUnitInsertRemove
 * PURPOSE: Process one unit insertion/removal
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  - No more units to be processed
 *          FASLE - not done;
 * NOTES:
 */
 BOOL_T STKTPLG_MGR_ProcessUnitInsertRemove(BOOL_T *is_insertion,
                                     UI32_T *starting_port,
                                     UI32_T *number_of_port,
                                     UI8_T * unit_id,
                                     BOOL_T *use_default)
{
   return STKTPLG_OM_ProcessUnitInsertRemove( is_insertion,
                                        starting_port,
                                        number_of_port,
                                        unit_id,
                                        use_default);
}

BOOL_T STKTPLG_MGR_SlaveSetPastMasterMac(void)
{
   STKTPLG_OM_SlaveSetPastMasterMac();
   return TRUE;
}

#endif
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)

/* FUNCTION NAME : STKTPLG_MGR_SetMasterButtonStatus
 * PURPOSE: This function is to set the master button status setting into flash for a specified unit
 * INPUT:   unit_id
 *          status : master button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status)
{
    BOOL_T ret;
    ret = STKTPLG_OM_SetMasterButtonStatus(unit_id, status);

    return ret;
}

#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/* FUNCTION NAME : STKTPLG_MGR_SetStackingButtonStatus
 * PURPOSE: This function is to set the stacking button status setting into flash
 * INPUT:   status : stacking button status
 *          TRUE  --  Software stacking button is pressed. The ports will be configured as
 *                  stacking ports.
 *          FALSE --  Software stacking button is not pressed. The ports will be configured as
 *                  normal ports.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- stacking button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetStackingButtonStatus(BOOL_T status)
{
    return SYSDRV_SetStackingButtonStatus(status);
}
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

/* FUNCTION NAME : STKTPLG_MGR_HandleIPCReqMsg
 * PURPOSE : This function is used to handle ipc request for stktplg mgr.
 * INPUT   : sysfun_msg_p  --  the ipc request for stktplg_mgr
 * OUTPUT  : sysfun_msg_p  --  the ipc response to send when return value is TRUE
 * RETUEN  : TRUE  --  A response is required to send
 *           FALSE --  Need not to send response.
 * NOTES   :
 */
BOOL_T STKTPLG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p)
{
    STKTPLG_MGR_IPCMsg_T *ipcmsg_p;
    UI16_T               resp_msg_size;

    if(sysfun_msg_p==NULL)
        return FALSE;

    ipcmsg_p = (STKTPLG_MGR_IPCMsg_T*)(sysfun_msg_p->msg_buf);

    switch(ipcmsg_p->type.cmd)
    {
        case STKTPLG_MGR_IPC_CMD_GET_SWITCH_INFO:
            ipcmsg_p->type.result=STKTPLG_MGR_GetSwitchInfo(&(ipcmsg_p->data.switch_info));
            resp_msg_size=STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T);
            break;
        case STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_INFO:
            ipcmsg_p->type.result=STKTPLG_MGR_GetNextSwitchInfo(&(ipcmsg_p->data.switch_info));
            resp_msg_size=STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_Switch_Info_T);
            break;
        case STKTPLG_MGR_IPC_CMD_GET_SWITCH_MODULE_INFO_ENTRY:
            ipcmsg_p->type.result=STKTPLG_MGR_GetSwitchModuleInfoEntry(&(ipcmsg_p->data.switch_module_info_entry));
            resp_msg_size=STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T);
            break;
        case STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_MODULE_INFO_ENTRY:
            ipcmsg_p->type.result=STKTPLG_MGR_GetNextSwitchModuleInfoEntry(&(ipcmsg_p->data.switch_module_info_entry));
            resp_msg_size=STKTPLG_MGR_GET_MSGBUFSIZE(STKTPLG_MGR_switchModuleInfoEntry_T);
            break;
        case STKTPLG_MGR_IPC_CMD_SLAVE_READY:
            STKTPLG_MGR_SlaveReady((BOOL_T)(ipcmsg_p->data.one_ui32));
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ALIAS:
            ipcmsg_p->type.result=STKTPLG_MGR_SetEntPhysicalAlias(ipcmsg_p->data.ent_physical_entry.ent_physical_index,
                                  ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_alias);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
        break;
        case STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ASSETID:
            ipcmsg_p->type.result=STKTPLG_MGR_SetEntPhysicalAssetId(ipcmsg_p->data.ent_physical_entry.ent_physical_index,
                                  ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_asset_id);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
        break;
    case STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_SERIANUM:
            ipcmsg_p->type.result=STKTPLG_MGR_SetEntPhysicalSeriaNum(ipcmsg_p->data.ent_physical_entry.ent_physical_index,
                                  ipcmsg_p->data.ent_physical_entry.ent_physical_entry_rw.ent_physical_serial_num);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
        break;
#if 0
        case STKTPLG_MGR_IPC_CMD_SET_STACK_ROLE_LED:
            ipcmsg_p->type.result=STKTPLG_MGR_SetStackRoleLed(ipcmsg_p->data.one_ui32);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif
        case STKTPLG_MGR_IPC_CMD_SET_TRANSITION_MODE:
            STKTPLG_MGR_SetTransitionMode();
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_MGR_IPC_CMD_UNIT_ID_RENUMBERING:
            ipcmsg_p->type.result=STKTPLG_MGR_UnitIDReNumbering();
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_MGR_IPC_CMD_PROVISION_COMPLETED:
            STKTPLG_MGR_ProvisionCompleted((BOOL_T)(ipcmsg_p->data.one_ui32));
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
        case STKTPLG_MGR_IPC_CMD_SYNC_MODULE_PROCESS:
        {
            BOOL_T is_insertion, use_default;

            ipcmsg_p->type.result=STKTPLG_MGR_SyncModuleProcess(ipcmsg_p->data.one_ui32,
                &is_insertion, &(ipcmsg_p->data.three_ui32[0]), &(ipcmsg_p->data.three_ui32[1]),
                &use_default);
            ipcmsg_p->data.three_ui32[2] = (is_insertion<<24) | (use_default<<16);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*3;
        }
            break;

        case STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_LINK_STATUS:
        {
            BOOL_T up_link_status, down_link_status;
            
            ipcmsg_p->type.result=STKTPLG_MGR_GetStackingPortLinkStatusByUnitId(
                ipcmsg_p->data.one_ui32, &up_link_status, &down_link_status);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*2;

            ipcmsg_p->data.two_ui32[0] = (UI32_T)up_link_status;
            ipcmsg_p->data.two_ui32[1] = (UI32_T)down_link_status;
            break;
        }    
        case STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_USER_PORT_INDEX:
            ipcmsg_p->type.result=STKTPLG_MGR_GetStackingUserPortByUnitId(
                ipcmsg_p->data.one_ui32, &(ipcmsg_p->data.two_ui32[0]), &(ipcmsg_p->data.two_ui32[1]));
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*2;    
            break;
                        
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
        case STKTPLG_MGR_IPC_CMD_SET_MASTER_BUTTON_STATUS:
            ipcmsg_p->type.result=STKTPLG_MGR_SetMasterButtonStatus(ipcmsg_p->data.two_ui32[0],
                (BOOL_T)ipcmsg_p->data.two_ui32[1]);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        case STKTPLG_MGR_IPC_CMD_PROCESS_UNIT_INSERT_REMOVE:
        {
            BOOL_T is_insertion, use_default;
            UI8_T unit_id;

            ipcmsg_p->type.result=STKTPLG_MGR_ProcessUnitInsertRemove(&is_insertion,
            &(ipcmsg_p->data.three_ui32[0]), &(ipcmsg_p->data.three_ui32[1]),&unit_id,&use_default);
            ipcmsg_p->data.three_ui32[2] = (is_insertion<<24) | (use_default<<16 |unit_id<<8);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(UI32_T)*3;
        }
            break;
       case STKTPLG_MGR_IPC_CMD_SET_SLAVE_PAST_MAC:
        {
            STKTPLG_MGR_SlaveSetPastMasterMac();
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
        }
            break;
#endif
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
        case STKTPLG_MGR_IPC_CMD_SET_STACKING_BUTTON_STATUS:
            ipcmsg_p->type.result=STKTPLG_MGR_SetStackingButtonStatus((BOOL_T)ipcmsg_p->data.one_ui32);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
        case STKTPLG_MGR_IPC_CMD_SET_CFG_HW_PORT_MODE:
            ipcmsg_p->type.result=STKTPLG_MGR_SetCfgHwPortMode(
                ipcmsg_p->data.three_ui32[0],
                ipcmsg_p->data.three_ui32[1],
                ipcmsg_p->data.three_ui32[2]);
            resp_msg_size=STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif
        default:
            ipcmsg_p->type.result=FALSE;
            resp_msg_size = STKTPLG_MGR_MSGBUF_TYPE_SIZE;
            SYSFUN_Debug_Printf("%s(): invalid cmd(%d)\r\n", __FUNCTION__, (int)(ipcmsg_p->type.cmd));
            break;
    }
    sysfun_msg_p->msg_size = resp_msg_size;
    return TRUE;
}
/*it is useless ,for STKTPLG_OM_IsStackingButtonChanged is used to detect stacking button info*/
#if 0
static BOOL_T STKTPLG_MGR_SetTenGModule()
{
#ifdef ECN430_FB2
    UI8_T    data=0;


    PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_EPLD_MODULE_STATUS, 1, 1, &data);


    if((data & 0x80) == 0)
    {
        STKTPLG_MGR_SetBoardInformation();
    }

#else
#endif
    return TRUE;

}
#endif

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
#define LOCAL_UNIT                      0

BOOL_T STKTPLG_MGR_SetStackingPortInBoard(BOOL_T is_stacking_port)
{
    UC_MGR_Sys_Info_T         uc_sys_info;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    STKTPLG_BOARD_BoardInfo_T new_board_info_p;
    UI8_T  up_port_type, down_port_type;
    UI32_T my_unit_id,up_port,down_port;
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    UI8_T  stackingPort2phyDevId_ar[STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_TYPE];
    UI8_T  stackingPort2phyPortId_ar[STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_TYPE];
    UI32_T portNum;
#endif

  /*  EPR: ES3628BT-FLF-ZZ-01068
Problem:Stacking:DUT displayed Error Messages continuously after

pressed the Unit4's stack_button off and on.
Rootcause:(1)when 2 dut stacking stable,slave pressed off stacking

button and pressed stacking button again
          (2)sometimes master will print Error Messages

continuously,for at this time slave give the incorrect ports info to

master
          (3)when stacking button state changed ,it will re-run

stktplg state machine,and init the current database,but here init

the stable database which supply to other csc
Solution: clear the current database instead of the stable database
Files:stktplg_mgr.c*/
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
         perror("\r\nGet UC System Information Fail.");
         assert(0);

         /* severe problem, while loop here             */
         while (TRUE);
     }

    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    new_board_info_p = *board_info_p;
    /*Must read:the default board.c need has stacking port,for when the stacking port changed to
      normal port ,here will add 2 normal ports*/

   /*EPR: N/A
        Problem: when stacking is disable,cannot get 27/28 port
        Rootcause:because when stacking is disable,not provide stacking port to others.and stk_mgr set board.c error
        Solution:when stacking is disable,set 27/28 port to normal port
        Files:stktplg_mgr.c*/
   /* The up_port and down_port is front port number, not physical port number.
    */
   up_port = new_board_info_p.stacking_uplink_port;
   down_port = new_board_info_p.stacking_downlink_port;
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if(is_stacking_port)
    {
        if(ctrl_info_p->stacking_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
        {
            memcpy(stackingPort2phyDevId_ar, new_board_info_p.stackingPortUserConfiguration[ctrl_info_p->stacking_port_option-1].stackingPort2phyDevId_ar, sizeof(stackingPort2phyDevId_ar));
            memcpy(stackingPort2phyPortId_ar, new_board_info_p.stackingPortUserConfiguration[ctrl_info_p->stacking_port_option-1].stackingPort2phyPortId_ar, sizeof(stackingPort2phyPortId_ar));
        }

        for (portNum = 0; portNum < new_board_info_p.max_port_number_of_this_unit; portNum++)
        {
            if (ctrl_info_p->stacking_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
            {
                if ((new_board_info_p.userPortMappingTable[LOCAL_UNIT][portNum].device_id == stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1]) &&
                    (new_board_info_p.userPortMappingTable[LOCAL_UNIT][portNum].device_port_id == stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1]))
                {
                    up_port = portNum + 1;
                }
                else if ((new_board_info_p.userPortMappingTable[LOCAL_UNIT][portNum].device_id == stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1]) &&
                    (new_board_info_p.userPortMappingTable[LOCAL_UNIT][portNum].device_port_id == stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1]))
                {
                    down_port = portNum + 1;
                }
            }
        }
    }
#endif

    up_port_type = new_board_info_p.userPortMappingTable[LOCAL_UNIT][up_port-1].port_type;
    down_port_type = new_board_info_p.userPortMappingTable[LOCAL_UNIT][down_port-1].port_type;

    if(is_stacking_port)
    {
      new_board_info_p.userPortMappingTable[LOCAL_UNIT][up_port-1].port_type=STKTPLG_PORT_TYPE_STACKING;
      new_board_info_p.userPortMappingTable[LOCAL_UNIT][down_port-1].port_type=STKTPLG_PORT_TYPE_STACKING;

/*EPR: ES3628BT-FLF-ZZ-00785
Problem:MSTPmib:the xstInstancePortOperPathCost index include stacking port 6/27-8/28.
Rootcause:when do hot-insert ,it will notify csc all ports including stacking ports.And xstp will set the flag for stacking

ports.
Solution:when stacking button changed,change the max port number of board
Files:stktplg_mgr.c,stktplg_board.c*/
#ifdef ECN430_FB2
      new_board_info_p.max_port_number_of_this_unit = new_board_info_p.max_port_number_of_this_unit+new_board_info_p.max_hot_swap_port_number;
      new_board_info_p.max_port_number_on_board = new_board_info_p.max_port_number_on_board +new_board_info_p.max_hot_swap_port_number;
#else
      new_board_info_p.max_port_number_of_this_unit = new_board_info_p.max_port_number_of_this_unit-new_board_info_p.max_hot_swap_port_number;
      new_board_info_p.max_port_number_on_board = new_board_info_p.max_port_number_on_board -new_board_info_p.max_hot_swap_port_number;
#endif
    }
    else
    {
    /*it depends on the port attribute*/
#ifdef ECN430_FB2
     new_board_info_p.userPortMappingTable[LOCAL_UNIT][up_port-1].port_type=VAL_portType_tenG;
     new_board_info_p.userPortMappingTable[LOCAL_UNIT][down_port-1].port_type=VAL_portType_tenG;
     new_board_info_p.max_port_number_of_this_unit = new_board_info_p.max_port_number_of_this_unit-new_board_info_p.max_hot_swap_port_number;
     new_board_info_p.max_port_number_on_board = new_board_info_p.max_port_number_on_board -new_board_info_p.max_hot_swap_port_number;
#else
     new_board_info_p.userPortMappingTable[LOCAL_UNIT][up_port-1].port_type=up_port_type;
     new_board_info_p.userPortMappingTable[LOCAL_UNIT][down_port-1].port_type=down_port_type;
     new_board_info_p.max_port_number_of_this_unit = new_board_info_p.max_port_number_of_this_unit+new_board_info_p.max_hot_swap_port_number;
     new_board_info_p.max_port_number_on_board = new_board_info_p.max_port_number_on_board +new_board_info_p.max_hot_swap_port_number;
#endif

    };

    if(!STKTPLG_BOARD_SetBoardInformation(uc_sys_info.board_id, new_board_info_p))
     return FALSE;

    //STKTPLG_OM_GetMyUnitID(&my_unit_id);
 /*EPR: ES3628BT-FLF-ZZ-00785
Problem:MSTPmib:the xstInstancePortOperPathCost index include stacking port 6/27-8/28.
Rootcause:when do hot-insert ,it will notify csc all ports including stacking ports.And xstp will set the flag for stacking

ports.
Solution:when stacking button changed,change the max port number of board
Files:stktplg_mgr.c,stktplg_board.c*/
    STKTPLG_OM_ENG_SetMaxPortCapability(ctrl_info_p->my_unit_id, new_board_info_p.max_port_number_on_board);
    STKTPLG_MGR_SetPortMapping(&(new_board_info_p.userPortMappingTable), ctrl_info_p->my_unit_id);
/*keep the info in share mem,in order to get the info quickly*/
    STKTPLG_SHOM_SetStackingPortInfo((UI32_T)is_stacking_port,up_port,down_port);
    return TRUE;



}

#endif


#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_IsModulePrzStatusChanged
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to check if module status changed.
 * INPUT   : module_slot_idx - index of the module slot
 * OUTPUT  : module_is_present_p
 *           module_id_p
 * RETURN  : TRUE  - module slot status changed.
 *           FALSE - module slot status not changed.
 * NOTE    : None.
 *-------------------------------------------------------------------------
 */
BOOL_T STKTPLG_MGR_IsModulePrzStatusChanged(UI8_T module_slot_idx, BOOL_T *module_is_present_p, UI8_T *module_id_p)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("%s():Get UC System Information Fail.\r\n", __FUNCTION__);
        return FALSE;
    }

    return STKTPLG_BOARD_IsModulePrzStatusChanged(uc_sys_info.board_id, module_slot_idx, module_is_present_p, module_id_p);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_ConfigModule
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to do module configuration.
 * INPUT   : module_slot_idx - index of the module slot status register
 *           module_slot_status_reg - current module slot status register value.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : 1. absent -> present
 *           1.1. check module id
 *           1.2. enable howswap controller
 *           1.3. transceiver tx on
 *           1.4. wait 0.5 sec
 *           1.5. enable miim bus switch
 *           1.6. reset phy
 *           2. present -> absent
 *           2.1. disable miim bus switch
 *           2.2. disable hotswap controller
 *           2.3. transceiver tx off
 *           3. Light module LED
 *           4. we use workaround to avoid fiber and copper module coexistence problem. When insert one module,
 *              we will disable the other. The will not affect mac to determine the xe port link up/down because
 *              autonegociation is done through data path. It will cause dump register value when we use bcm door.
 *-------------------------------------------------------------------------
 */
void STKTPLG_MGR_ConfigModule(UI8_T module_slot_idx, BOOL_T module_is_present, UI8_T module_id)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        printf("%s():Get UC System Information Fail.\r\n", __FUNCTION__);
        return;
    }

    STKTPLG_BOARD_ConfigModule(uc_sys_info.board_id, module_slot_idx, module_is_present, module_id);
}

/* v3-hotswap */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_InitModule
 *-------------------------------------------------------------------------
 * PURPOSE : Init module status.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *-------------------------------------------------------------------------
 */
void STKTPLG_MGR_InitModule(void)
{
    UI32_T my_unit_id, board_id;

    if(FALSE==STKTPLG_OM_GetMyUnitID(&my_unit_id))
    {
        printf("%s(%d): Failed to get my unit id.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if(FALSE==STKTPLG_OM_GetUnitBoardID(my_unit_id, &board_id))
    {
        printf("%s(%d): Failed to get my board id.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    STKTPLG_BOARD_InitModule(board_id);
}
#endif /* #if (SYS_CPNT_MODULE_WITH_CPU == FALSE) */
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_HwPortMode_UpdatePortMapping
 * -------------------------------------------------------------------------
 * PURPOSE : To sync port mapping from board info
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void STKTPLG_MGR_HwPortMode_UpdatePortMapping(void)
{
    STKTPLG_OM_HwPortMode_UpdatePortMapping();
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_InitHwPortModeDb
 * -------------------------------------------------------------------------
 * PURPOSE : To initialize HW port mode db
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void STKTPLG_MGR_InitHwPortModeDb(void)
{
    UI8_T hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];

    if (STKTPLG_BOARD_GetAllHwPortModeStatus(hw_port_mode_ar))
    {
        STKTPLG_OM_SetAllOperHwPortMode(SYS_VAL_LOCAL_UNIT_ID, hw_port_mode_ar);
    }

    if (SYSDRV_GetCfgHwPortMode(SYS_VAL_LOCAL_UNIT_ID, hw_port_mode_ar))
    {
        STKTPLG_OM_SetAllCfgHwPortMode(SYS_VAL_LOCAL_UNIT_ID, hw_port_mode_ar);
    }
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_MGR_SetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode config
 * INPUT   : unit
 *           port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_MGR_SetCfgHwPortMode(
    UI32_T unit,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode)
{
    STKTPLG_BOARD_HwPortModeInfo_T hw_port_mode_info;
    STKTPLG_TYPE_HwPortMode_T old_hw_port_mode;
    UI8_T sync_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI32_T board_id;
    UI32_T port_type;
    int sync_port_count;
    int i;

    if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (port < 1 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    if (hw_port_mode >= STKTPLG_TYPE_HW_PORT_MODE_MAX)
    {
        return FALSE;
    }

    if (!STKTPLG_OM_GetUnitBoardID(unit, &board_id))
    {
        return FALSE;
    }

    if (!STKTPLG_OM_GetCfgHwPortMode(unit, port, &old_hw_port_mode))
    {
        return FALSE;
    }

    if (old_hw_port_mode == hw_port_mode)
    {
        return TRUE;
    }

    /* check hw_port_mode validity
     */
    if (!STKTPLG_BOARD_GetHwPortModeInfo(board_id, port, hw_port_mode, &hw_port_mode_info))
    {
        return FALSE;
    }

    /* reset to default
     */
    if (hw_port_mode == hw_port_mode_info.dflt_hw_port_mode)
    {
        hw_port_mode = STKTPLG_TYPE_HW_PORT_MODE_UNSPECIFIED;
    }

    /* find sync port
     */
    sync_port_count = 0;
    {
        STKTPLG_TYPE_HwPortMode_T tmp_hw_port_mode;
        UI32_T mapping_uport[STKTPLG_TYPE_HW_PORT_MODE_MAX];
        UI32_T tmp_port;

        memset(mapping_uport, 0, sizeof(mapping_uport));

        for (tmp_hw_port_mode = 1; tmp_hw_port_mode < STKTPLG_TYPE_HW_PORT_MODE_MAX; tmp_hw_port_mode++)
        {
            if (!STKTPLG_BOARD_GetHwPortModeInfo(board_id, port, tmp_hw_port_mode, &hw_port_mode_info))
            {
                continue;
            }

            mapping_uport[tmp_hw_port_mode] = hw_port_mode_info.mapping_uport;
        }

        for (tmp_port = 1; tmp_port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; tmp_port++)
        {
            if (tmp_port == port)
            {
                continue;
            }

            for (tmp_hw_port_mode = 1; tmp_hw_port_mode < STKTPLG_TYPE_HW_PORT_MODE_MAX; tmp_hw_port_mode++)
            {
                if (!STKTPLG_BOARD_GetHwPortModeInfo(board_id, tmp_port, tmp_hw_port_mode, &hw_port_mode_info))
                {
                    continue;
                }

                if (mapping_uport[tmp_hw_port_mode] == hw_port_mode_info.mapping_uport)
                {
                    break;
                }
            }

            if (tmp_hw_port_mode < STKTPLG_TYPE_HW_PORT_MODE_MAX)
            {
                sync_port[sync_port_count++] = tmp_port;
            }
        } /* end of for (tmp_port) */
    }

    /* save config to file
     */
    {
        UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];

        if (!STKTPLG_OM_GetAllCfgHwPortMode(unit, cfg_hw_port_mode_ar))
        {
            return FALSE;
        }

        cfg_hw_port_mode_ar[port-1] = hw_port_mode;

        for (i = 0; i < sync_port_count; i++)
        {
            cfg_hw_port_mode_ar[sync_port[i]-1] = hw_port_mode;
        }

        if (!STKTPLG_BOARD_CheckHwPortModeConflict(board_id, cfg_hw_port_mode_ar))
        {
            return FALSE;
        }

        if (!SYSDRV_SetCfgHwPortMode(unit, cfg_hw_port_mode_ar))
        {
            return FALSE;
        }
    }

    /* update database
     */
    if (!STKTPLG_OM_SetCfgHwPortMode(unit, port, hw_port_mode))
    {
        return FALSE;
    }

    for (i = 0; i < sync_port_count; i++)
    {
        if (!STKTPLG_OM_SetCfgHwPortMode(unit, sync_port[i], hw_port_mode))
        {
            return FALSE;
        }
    }

    return TRUE;
}
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

