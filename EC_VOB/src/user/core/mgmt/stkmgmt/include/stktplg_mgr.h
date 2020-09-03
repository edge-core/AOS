/* Module Name: STKTPLG_MGR.H
 * Purpose: 
 *        The topology object manager provides the following APIs for other functionalities
 *        to access total number of units in the stack and ID, type, existence, 
 *        serial number, hardware version and software version of specified unit.
 * Notes: 
 *        
 * History:                                                               
 *       Date          -- Modifier,  Reason
 *
 * Copyright(C)      Accton Corporation, 1999, 2000   				
 */

#ifndef 	STKTPLG_MGR_H
#define 	STKTPLG_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "dev_swdrv.h"
#include "stktplg_om.h"

#include "sysfun.h"

/* NAME CONSTANT DECLARATIONS
 */

#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
#define STKTPLG_MGR_MODULE_1            1
#define STKTPLG_MGR_MODULE_2            2
#endif

/*
 * Define topology event and event message
 */
/* definitions of ipc commands in STKTPLG_MGR which will be used in PMGR operation
 */
enum
{
    STKTPLG_MGR_IPC_CMD_SLAVE_READY=1,
    STKTPLG_MGR_IPC_CMD_GET_SWITCH_INFO,
    STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_INFO,
    STKTPLG_MGR_IPC_CMD_GET_SWITCH_MODULE_INFO_ENTRY,
    STKTPLG_MGR_IPC_CMD_GET_NEXT_SWITCH_MODULE_INFO_ENTRY,
    STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ALIAS,
    STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_ASSETID,
    STKTPLG_MGR_IPC_CMD_SET_ENT_PHYSICAL_SERIANUM,
    STKTPLG_MGR_IPC_CMD_SET_STACK_ROLE_LED,
    STKTPLG_MGR_IPC_CMD_SET_TRANSITION_MODE,
    STKTPLG_MGR_IPC_CMD_UNIT_ID_RENUMBERING,
    STKTPLG_MGR_IPC_CMD_PROVISION_COMPLETED,
    STKTPLG_MGR_IPC_CMD_SYNC_MODULE_PROCESS,
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
    STKTPLG_MGR_IPC_CMD_SET_MASTER_BUTTON_STATUS,
#endif
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_MGR_IPC_CMD_PROCESS_UNIT_INSERT_REMOVE,
    STKTPLG_MGR_IPC_CMD_SET_SLAVE_PAST_MAC,
#endif
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    STKTPLG_MGR_IPC_CMD_SET_STACKING_BUTTON_STATUS,
#endif
    STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_LINK_STATUS,
    STKTPLG_MGR_IPC_CMD_GET_STACKING_PORT_USER_PORT_INDEX,   
    STKTPLG_MGR_IPC_CMD_SET_CFG_HW_PORT_MODE,
};

#define STKTPLG_MGR_MSGBUF_TYPE_SIZE ((uintptr_t)(&(((STKTPLG_MGR_IPCMsg_T*)0)->data)))

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in STKTPLG_MGR_IPCMsg_T.data
 */
#define STKTPLG_MGR_GET_MSGBUFSIZE(msg_data_type) \
        (STKTPLG_MGR_MSGBUF_TYPE_SIZE + sizeof(msg_data_type))

/* Define for switch/unit validation, and wait for leaf value.
 */
#define STKTPLG_MGR_SWITCH_VALID        1
#define STKTPLG_MGR_SWITCH_INVALID      2

#define STKTPLG_MGR_MODULE_VALID        1
#define STKTPLG_MGR_MODULE_INVALID      2

/* DATA TYPE DECLARATIONS
 */
typedef struct STKTPLG_MGR_Switch_Info_S
{
    UI8_T   sw_unit_index;
    UI8_T   sw_hardware_ver[SYS_ADPT_HW_VER_STR_LEN + 1];
    UI8_T   sw_microcode_ver[SYS_ADPT_HW_VER_STR_LEN + 1];
    UI8_T   sw_loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   sw_kernel_ver[SYS_ADPT_KERNEL_VER_STR_LEN + 1];
    UI8_T   sw_boot_rom_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   sw_opcode_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   sw_epld_ver[SYS_ADPT_EPLD_VER_STR_LEN+1];
    UI8_T   sw_port_number;
    UI8_T   sw_role_in_system;
    UI8_T   sw_serial_number[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI8_T   sw_expansion_slot1;
    UI8_T   sw_expansion_slot2;
    UI8_T   sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI8_T   sw_model_number[SYS_ADPT_MODEL_NUMBER_LEN + 1];
    UI8_T   sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI32_T  sw_identifier;
    UI8_T   sw_module_expected_opcode_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   sw_validation;

    /* This records software build time as Unix seconds,
     * i.e. number of seconds since 1970.01.01 00:00 UTC.
     * Use SYS_TIME_ConvertTime for display to UI.
     */
    UI32_T  sw_software_build_time;
} STKTPLG_MGR_Switch_Info_T;

#define STKTPLG_MGR_switchModuleInfoEntry_T STKTPLG_OM_switchModuleInfoEntry_T

#define STKTPLG_MGR_StackingDB_T STKTPLG_OM_StackingDB_T

/* NAMING CONSTANT DECLARARTIONS
 */
/* structure for the request/response ipc message in stktplg pmgr and mgr
 */
typedef struct
{
    union STKTPLG_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. STKTPLG_MGR_IPC_CMD_XXX */
        UI32_T result; /* for response */
    } type;

    union
    {
        UI32_T                         one_ui32;
        UI32_T                         two_ui32[2];
        UI32_T                         three_ui32[3];
        STKTPLG_OM_EntPhysicalEntry_T  ent_physical_entry;
        STKTPLG_MGR_Switch_Info_T      switch_info;
        STKTPLG_MGR_switchModuleInfoEntry_T switch_module_info_entry;
    } data; /* contains the supplemntal data for the corresponding cmd */
} STKTPLG_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
BOOL_T STKTPLG_MGR_InitiateProcessResources(void);

/* FUNCTION NAME: STKTPLG_MGR_Register_StackState_CallBack
 * PURPOSE: This procedure hooks the topology change request handler to
 *          the topology discovery subsystem.
 * INPUT:   fun -- call back function pointer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   1. This procedure must be invoked before topology change request handler
 *             can be invoked.
 *          2. The topology change request handler is declared in the stack control
 *             subsystem (STK_CTRL package).
 *          3. The main (root) task/stack control subsystem shall invoke this
 *             procedure before STK_TPLG task can be created.
 *
 */
void STKTPLG_MGR_Register_StackState_CallBack (void (*fun)(UI32_T msg));

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
BOOL_T STKTPLG_MGR_SetBaseLed(UI8_T base_led);

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
BOOL_T STKTPLG_MGR_SetStackLinkLed(UI8_T stack_link_led);

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
BOOL_T STKTPLG_MGR_EnableModuleBus(UI32_T device_index, UI32_T module_index);

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
BOOL_T STKTPLG_MGR_DisableModuleBus(UI32_T device_index, UI32_T module_index);

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
BOOL_T STKTPLG_MGR_SetModuleABus(UI8_T module_bus);

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
BOOL_T STKTPLG_MGR_SetModuleBBus(UI8_T module_bus);

/* FUNCTION NAME: STKTPLG_MGR_GetSwitchInfo
 * PURPOSE: This function is used to get this switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info);


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
            UI32_T unit_id, BOOL_T *up_link_status, BOOL_T *down_link_status);

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
BOOL_T STKTPLG_MGR_GetStackingUserPortByUnitId(UI32_T unit_id, UI32_T *up_uport_p, UI32_T *down_uport_p);

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
BOOL_T STKTPLG_MGR_GetNextSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info);

/* FUNCTION NAME: STKTPLG_MGR_GetSwitchModuleInfoEntry
 * PURPOSE: This function is used to get this module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_MGR_GetSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry);

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
BOOL_T STKTPLG_MGR_GetNextSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry);

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
BOOL_T STKTPLG_MGR_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info);

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
BOOL_T STKTPLG_MGR_ResetDeviceInfo(void);

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
BOOL_T STKTPLG_MGR_RemoveModule(UI32_T unit, UI32_T module_index);


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
BOOL_T STKTPLG_MGR_InsertModule(UI32_T unit, UI32_T module_index, UI32_T module_type);


/* FUNCTION NAME: STKTPLG_MGR_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_MGR_SlaveReady(BOOL_T ready);

/* FUNCTION NAME: STKTPLG_MGR_StktplgEngineerDebugMenu
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
void STKTPLG_MGR_StktplgEngineerDebugMenu (void);

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
BOOL_T STKTPLG_MGR_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias_p);

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
BOOL_T STKTPLG_MGR_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id_p);

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
BOOL_T STKTPLG_MGR_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum_p);

/* FUNCTION NAME: STKTPLG_MGR_UPortToChipDevId
 * PURPOSE: Trasfer the logical port to physical chip device id.
 * INPUT:   u_port   -- user port(only include normal port).
 * OUTPUT:  *chip_dev_id  -- chip device id.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_MGR_UPortToChipDevId(UI32_T u_port, UI32_T *chip_dev_id);

/* FUNCTION NAME: STKTPLG_MGR_GetUnitBootReason
 * PURPOSE: This function is used to get the boot reason of some unit
 * INPUT:   unit --- the unit to get.
 * OUTPUT:  *boot_reason -- boot reason.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART         1
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART         2
 */
BOOL_T STKTPLG_MGR_GetUnitBootReason(UI32_T unit, UI32_T *boot_reason);

/* FUNCTION NAME: STKTPLG_MGR_SetStackRoleLed
 * PURPOSE: This function is used to set the role which the unit is playing
 * INPUT:   stk_role
 *          	LEDDRV_STACK_ARBITRATION
 *				LEDDRV_STACK_PRE_MASTER
 *				LEDDRV_STACK_MASTER
 *				LEDDRV_STACK_SLAVE
 *				defined in leddrv.h
 * OUTPUT:  none
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_SetStackRoleLed(UI32_T stk_role,UI32_T up_phy_status,UI32_T down_phy_status);

/* FUNCTION NAME: STKTPLG_MGR_SetTransitionMode
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
void STKTPLG_MGR_SetTransitionMode(void);

/* FUNCTION NAME: STKTPLG_MGR_IsSetTransitionMode
 * PURPOSE: This function is used to query whether go to transition mode 
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Be asked to enter transition mode
 *          FALSE : Nothing
 * NOTES:
 *			For Stack Topology only
 */
BOOL_T STKTPLG_MGR_IsSetTransitionMode(void);

/* FUNCTION NAME: STKTPLG_MGR_SetPortMapping
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
BOOL_T STKTPLG_MGR_SetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit);

/* FUNCTION NAME : STKTPLG_MGR_SetStackingDB
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_MGR_SetStackingDB(void);

/* FUNCTION NAME : STKTPLG_MGR_SetStackingDBEntry
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_MGR_SetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry);

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
BOOL_T STKTPLG_MGR_UnitIDReNumbering(void);

#if 0
/* FUNCTION NAME: STKTPLG_MGR_GetAllUnitsPortMapping
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
BOOL_T STKTPLG_MGR_GetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping);
#endif

void STKTPLG_MGR_ProvisionCompleted(BOOL_T ready);


void STKTPLG_MGR_Transition(BOOL_T ready);
/* FUNCTION NAME: STKTPLG_MGR_InsertExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *BOOL_T STKTPLG_MGR_InsertExpModule(unit)
 */
BOOL_T STKTPLG_MGR_InsertExpModule(UI8_T unit_id);

/* FUNCTION NAME: STKTPLG_MGR_RemoveExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_MGR_RemoveExpModule(UI8_T unit_id);

#if 0
BOOL_T STKTPLG_MGR_SetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping);
#endif

/* FUNCTION NAME: STKTPLG_MGR_UpdateModuleStateChanged
 * PURPOSE: This API is used to get and put dirty bit as FALSE.
 *          i.e. clear after read.
 * INPUT:   dirty  -- a array of dirty table.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None.
 */
BOOL_T STKTPLG_MGR_UpdateModuleStateChanged(BOOL_T status_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

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
                                     BOOL_T *use_default);

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
BOOL_T STKTPLG_MGR_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status);

#endif

/* FUNCTION NAME: STKTPLG_MGR_SetModuleInfo
 * PURPOSE: This function is used to set ModuleInfo.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES: This API is currently used by Sysdrv to set xenpak_status field
 *
 */

BOOL_T STKTPLG_MGR_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info);

/* FUNCTION NAME : STKTPLG_MGR_HandleIPCReqMsg
 * PURPOSE : This function is used to handle ipc request for stktplg mgr.
 * INPUT   : sysfun_msg_p  --  the ipc request for stktplg_mgr
 * OUTPUT  : sysfun_msg_p  --  the ipc response to send when return value is TRUE
 * RETUEN  : TRUE  --  A response is required to send
 *           FALSE --  Need not to send response.
 * NOTES   :
 */
BOOL_T STKTPLG_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);
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
                                        BOOL_T *use_default);

BOOL_T STKTPLG_MGR_SlaveSetPastMasterMac();

#endif
#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
BOOL_T STKTPLG_MGR_SetStackingPortInBoard(BOOL_T is_stacking_port);
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
BOOL_T STKTPLG_MGR_IsModulePrzStatusChanged(UI8_T module_slot_idx, BOOL_T *module_is_present_p, UI8_T *module_id_p);

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
void STKTPLG_MGR_ConfigModule(UI8_T module_slot_idx, BOOL_T module_is_present, UI8_T module_id);

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
void STKTPLG_MGR_InitModule(void);

#endif /* #if (SYS_CPNT_MODULE_WITH_CPU == FALSE) */
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

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
BOOL_T STKTPLG_MGR_SetStackingButtonStatus(BOOL_T status);
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
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
void STKTPLG_MGR_InitHwPortModeDb(void);

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
    STKTPLG_TYPE_HwPortMode_T hw_port_mode);
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

#endif   /* STKTPLG_MGR_H */

