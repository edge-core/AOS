/* MODULE NAME:  stktplg_om_private.h
 * PURPOSE:
 *  This header file defines the APIs that can only be called within STKTPLG.
 *
 * NOTES:
 *
 * HISTORY
 *    7/16/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKTPLG_OM_PRIVATE_H
#define STKTPLG_OM_PRIVATE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"

#include "stktplg_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : STKTPLG_OM_SetUnitRuntimeFirmwareVer
 * PURPOSE: Set the runtime firmware version of the specified unit
 * INPUT:   unit              - id of the unit of the runtime firmware version
 *                              to be set into
 *          runtime_fw_ver_ar - Runtime firmware version to be set
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *          
 */
void STKTPLG_OM_SetUnitRuntimeFirmwareVer(UI32_T unit, UI8_T runtime_firmware_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1]);

/* FUNCTION NAME: STKTPLG_OM_SetDeviceInfo
 * PURPOSE: This function is used to set this device information.
 * INPUT:   unit_id         -- unit id
 *          *device_info    -- device info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info);

/* FUNCTION NAME: STKTPLG_OM_ResetDeviceInfo
 * PURPOSE: This function is used to reset the device information.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ResetDeviceInfo(void);

/* FUNCTION NAME: STKTPLG_OM_SetModuleInfo
 * PURPOSE: This function is used to set this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info);

/* FUNCTION NAME: STKTPLG_OM_RemoveModule
 * PURPOSE: This function is used to remove slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_RemoveModule(UI32_T unit, UI32_T module_index);


/* FUNCTION NAME: STKTPLG_OM_InsertModule
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
BOOL_T STKTPLG_OM_InsertModule(UI32_T unit, UI32_T module_index, UI32_T module_type);

/* FUNCTION NAME: STKTPLG_OM_AddOnePortNumber
 * PURPOSE: This function is used to add one port number in this unit.
 * INPUT:   device_index    -- device index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_AddOnePortNumber(UI32_T device_index);

/* FUNCTION NAME: STKTPLG_OM_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_OM_SlaveReady(BOOL_T ready);

/* FUNCTION NAME: STKTPLG_OM_GetCtrlInfo
 * PURPOSE: This routine is used to get control information of stack topology
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  pointer to control information of stack topology.
 * NOTES:   
 *
 */
STKTPLG_OM_Ctrl_Info_T *STKTPLG_OM_GetCtrlInfo(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME -  STKTPLG_OM_GetStackingInfo
 * -------------------------------------------------------------------------
 * FUNCTION: get stktplg stacking information
 * INPUT   : NONE      
 * OUTPUT  : NONE    
 * RETURN  : Stacking_Info_T
 * NOTE    : 
 * ------------------------------------------------------------------------
 */
Stacking_Info_T   *STKTPLG_OM_GetStackingInfo(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME -  STKTPLG_OM_SetStackingInfo
 * -------------------------------------------------------------------------
 * FUNCTION: set stktplg stacking information
 * INPUT   : stack_info      
 * OUTPUT  : NONE    
 * RETURN  : NONE
 * NOTE    : 
 * -------------------------------------------------------------------------*/

void  STKTPLG_OM_SetStackingInfo(Stacking_Info_T  stack_info);

/* FUNCTION NAME : STKTPLG_OM_SetPortMapping
 * PURPOSE: To set the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_OM_SetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit);

/* FUNCTION NAME : STKTPLG_OM_Transition
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
void STKTPLG_OM_Transition(BOOL_T ready);

/* FUNCTION NAME : STKTPLG_OM_ResetStackingDB
 * PURPOSE: To reset the stackingDB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *          
 */
void STKTPLG_OM_ResetStackingDB(void);

/* FUNCTION NAME : STKTPLG_OM_GetStackingDB
 * PURPOSE: To get the stacking db from cli_om.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- Success
 *          FALSE        -- Fail
 * NOTES:
 *          
 */
 BOOL_T STKTPLG_OM_GetStackingDB(void);

/* FUNCTION NAME : STKTPLG_OM_SetStackingDB
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_OM_SetStackingDB(void);

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBEntryByMac
 * PURPOSE: This api will return the unit id according to the given mac and set
 *          the specified device_type to the entry of the given mac.
 *
 * INPUT:   mac          -- the mac which is used to find the db entry
 *          device_type  -- board id of the found db entry. this id will be set
 *                          to the entry if it is found.
 * OUTPUT:  None.
 * RETUEN:  Non-Zero: unit id of the db entry with the given mac
 *          Zero: not found
 * NOTES:
 *
 */
UI32_T STKTPLG_OM_GetStackingDBEntryByMac(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type);

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBFreeEntry
 * PURPOSE: This api will return the best-fit unit id according to the given mac
 *          and device_type(a.k.a. board_id).
 *
 * INPUT:   mac          -- the mac of the unit which needs to be assigned a unit id.
 *          device_type  -- board id of the unit which needs to be assigned a unit id.
 *
 * OUTPUT:  None.
 * RETUEN:  Non-Zero: An available unit id
 *          Zero: No availabe unit id
 * NOTES:
 *       This api will first try to find whether the given mac and device_type
 *       have ever existed in stackingdb before(id_flag==1). If the entry does
 *       not exist before, it will try to assign the unit id from an empty entry
 *       (id_flag==0). If no empty entry is available, it will try to assign the
 *       unit id that had ever been assigned before(id_flag==1).
 *
 */
UI32_T STKTPLG_OM_GetStackingDBFreeEntry(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type);

/* FUNCTION NAME : STKTPLG_OM_SetStackingDBEntry
 * PURPOSE: Set the stacking db entry according to the given entry id.
 * INPUT:   db           -- the entry to be set to the stacking db
 *          entry        -- the index of the stacking db entry to be set, this
 *                          is also the value of the unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *          
 */
BOOL_T STKTPLG_OM_SetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry);

/* FUNCTION NAME : STKTPLG_OM_UpdateTplgSyncBmp
 * PURPOSE  : To update sync bitmap in control info
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
void STKTPLG_OM_UpdateTplgSyncBmp(UI8_T sync_bmp);

/* FUNCTION NAME : STKTPLG_OM_SetTplgSyncBmp
 * PURPOSE  : To set all other units need to sync to.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_SetTplgSyncBmp(void);

/* FUNCTION NAME : STKTPLG_OM_GetTplgSyncBmp
 * PURPOSE  : To get databse sync bitmap
 * INPUT    : None
 * OUTPUT   : None.
 * RETUEN   : The bitmap.
 */
UI8_T STKTPLG_OM_GetTplgSyncBmp(void);

/* FUNCTION NAME : STKTPLG_OM_InsertBoardID
 * PURPOSE: Insert BoardID to OM
 * INPUT:   unit             -- unit id. 
 *          board_id  
 * OUTPUT:  NONE
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   
 */
BOOL_T STKTPLG_OM_InsertBoradID(UI8_T unit,UI8_T board_id);

/* FUNCTION NAME: STKTPLG_OM_ProvisionCompleted
 * PURPOSE: To set provision complete flag.
 * INPUT:   ready     -- provision complete flag to be set.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKTPLG_OM_ProvisionCompleted(BOOL_T ready);

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)

/* FUNCTION NAME : STKTPLG_OM_SetMasterButtonStatus
 * PURPOSE: This function is to set the master button status setting into flash for a specified unit 
 * INPUT:   unit_id
 *          status : master button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *          
 */
BOOL_T STKTPLG_OM_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status);

#endif /* end of if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE) */

/* FUNCTION NAME: STKTPLG_OM_InsertExpModule
 * PURPOSE: This function is used to insert slot-in module.
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InsertExpModule(UI32_T unit_id,UI8_T module_type,UI8_T *module_runtime_fw_ver);

/* FUNCTION NAME: STKTPLG_OM_RemoveExpModule
 * PURPOSE: This function is used to remove slot-in module.
 * INPUT:   unit            -- unit id.
 *          module_index    -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_RemoveExpModule(UI32_T unit_id);

/* FUNCTION NAME: STKTPLG_OM_UpdateModuleStateChanged
 * PURPOSE: This API is used to get and put dirty bit as FALSE.
 *          i.e. clear after read.
 * INPUT:   status_dirty            -- a array of dirty table.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success and any dirty.
 *          FALSE -- 1) failure due to system not stable.
 *                   2) Not dirty;
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UpdateModuleStateChanged(BOOL_T status_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);


/* FUNCTION NAME: STKTPLG_OM_SyncModuleProcess
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
BOOL_T STKTPLG_OM_SyncModuleProcess(UI32_T unit_id, 
                                     BOOL_T *is_insertion, 
                                     UI32_T *starting_port, 
                                     UI32_T *number_of_port, 
                                     BOOL_T *use_default);

/* FUNCTION NAME: STKTPLG_OM_UpdateExpectedModuleType
 * PURPOSE: This function is used to update expected module type, 
 *          that is used to make decision if module insertion, the
 *          provision shall be default or not.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE/FALSE
 * NOTES:   In This API, TPLG call CLI, it violate hierachy, but
 *          this is special case, that CLI is viewed as a storage t keep 
 *          info of TPLG.
 */
void STKTPLG_OM_UpdateExpectedModuleType(void);

/* FUNCTION NAME: STKTPLG_OM_SetModuleABus
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
BOOL_T STKTPLG_OM_SetModuleABus(UI8_T module_bus);

/* FUNCTION NAME: STKTPLG_OM_SetModuleBBus
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
BOOL_T STKTPLG_OM_SetModuleBBus(UI8_T module_bus);

/* FUNCTION NAME: STKTPLG_OM_EnableModuleBus
 * PURPOSE: This function is used to enable module bus.
 * INPUT:   module_index  -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   
 *
 */
BOOL_T STKTPLG_OM_EnableModuleBus(UI32_T module_index);

/* FUNCTION NAME: STKTPLG_OM_DisableModuleBus
 * PURPOSE: This function is used to disable module bus.
 * INPUT:   module_index  -- module index.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- successful
 *          FALSE  -- failure
 * NOTES:   
 *
 */
BOOL_T STKTPLG_OM_DisableModuleBus(UI32_T module_index);

/* FUNCTION NAME: STKTPLG_OM_SetBaseLed
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
BOOL_T STKTPLG_OM_SetBaseLed(UI8_T base_led);

/* FUNCTION NAME: STKTPLG_OM_SetStackLinkLed
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
BOOL_T STKTPLG_OM_SetStackLinkLed(UI8_T stack_link_led);

/* FUNCTION NAME : STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus
 * PURPOSE: To set the status of configuring topology info.
 * INPUT:   TRUE   -- STKTPLG had configured topology
 *                    info to lower layer.
 *          FALSE  -- STKTPLG had not configured
 *                    topology info to lower layer.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *          This function is called by stktplg_engine to
 *          set ths status of configuring topology info to
 *          lower layer.
 */
void STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus(BOOL_T isDone);

#endif    /* End of STKTPLG_OM_PRIVATE_H */

