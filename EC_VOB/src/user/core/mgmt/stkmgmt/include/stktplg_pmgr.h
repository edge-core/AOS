/* MODULE NAME:  stktplg_pmgr.h
 * PURPOSE:
 *    STKTPLG PMGR
 *
 * NOTES:
 *
 * HISTORY
 *    7/31/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKTPLG_PMGR_H
#define STKTPLG_PMGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "stktplg_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : STKTPLG_PMGR_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG PMGR for the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_InitiateProcessResources(void);

/* FUNCTION NAME: STKTPLG_PMGR_GetSwitchInfo
 * PURPOSE: This function is used to get this switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info);

/* FUNCTION NAME: STKTPLG_PMGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next switch information for SNMP.
 * INPUT:   switch_info->unit_id    -- unit id.
 * OUTPUT:  switch_info             -- next switch info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP switchInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetNextSwitchInfo(STKTPLG_MGR_Switch_Info_T *switch_info);

/* FUNCTION NAME: STKTPLG_PMGR_GetSwitchModuleInfoEntry
 * PURPOSE: This function is used to get this module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry);

/* FUNCTION NAME: STKTPLG_PMGR_GetNextSwitchInfo
 * PURPOSE: This function is used to get_next module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_PMGR_GetNextSwitchModuleInfoEntry(STKTPLG_MGR_switchModuleInfoEntry_T *entry);

/* FUNCTION NAME: STKTPLG_PMGR_SlaveReady
 * PURPOSE: This utiity function is to keep information about is unit is entering
 *          slave mode completely.
 * INPUT:   ready -- ready or not.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_PMGR_SlaveReady(BOOL_T ready);

BOOL_T STKTPLG_PMGR_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias);

BOOL_T STKTPLG_PMGR_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id);

BOOL_T STKTPLG_PMGR_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum);
#if 0
/* FUNCTION NAME: STKTPLG_PMGR_SetStackRoleLed
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
BOOL_T STKTPLG_PMGR_SetStackRoleLed(UI32_T stk_role);
#endif
/* FUNCTION NAME: STKTPLG_PMGR_SetTransitionMode
 * PURPOSE: This function is used to let stack tplg going to transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  Success
 *          FALSE : Fail
 * NOTES:
 *			Master only
 */
void STKTPLG_PMGR_SetTransitionMode(void);

/* FUNCTION NAME: STKTPLG_PMGR_UnitIDReNumbering
 * PURPOSE: This function is to result STKTPLG TCN and reassigned unit ID to all units
 *          based on the stacking spec.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE   -- Renumbering work (not standalone or unit ID not equal to 1)
 *          FALSE  -- otherwise
 * NOTES:   
 *
 */
BOOL_T STKTPLG_PMGR_UnitIDReNumbering(void);

void STKTPLG_PMGR_ProvisionCompleted(BOOL_T ready);

/* FUNCTION NAME: STKTPLG_PMGR_SyncModuleProcess
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
BOOL_T STKTPLG_PMGR_SyncModuleProcess(UI32_T unit_id, 
                                      BOOL_T *is_insertion, 
                                      UI32_T *starting_port, 
                                      UI32_T *number_of_port, 
                                      BOOL_T *use_default);


/* FUNCTION NAME: STKTPLG_PMGR_GetStackingPortLinkStatusByUnitId
 * PURPOSE: This function is used to get link status of the stacking ports on a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_link_status  -- phy status of stacking up port
 *          down_link_status-- phy status of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- fail to get link status.
 * NOTES:   
 */
BOOL_T STKTPLG_PMGR_GetStackingPortLinkStatusByUnitId(
            UI32_T unit_id, BOOL_T *up_link_status, BOOL_T *down_link_status);

/* FUNCTION NAME: STKTPLG_PMGR_GetStackingUserPortByUnitId
 * PURPOSE: This function is used to get the stacking ports in user port id for a unit.
 * INPUT:   unit_id     -- which unit id.
 * OUTPUT:  up_uport_p  -- user port index of stacking up port
 *          down_uport_p-- user port index of stacking down port
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure. Input parameter error ,or no stacking port on that unit.
 * NOTES:   Get board_id and find the stacking port index. 
 *          Check the port_type to tell if the unit gets stacking port on it.
 */
BOOL_T STKTPLG_PMGR_GetStackingUserPortByUnitId(
            UI32_T unit_id, UI32_T *up_uport_p, UI32_T *down_uport_p);

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
/* FUNCTION NAME : STKTPLG_PMGR_SetMasterButtonStatus
 * PURPOSE: This function is to set the master button status setting into flash for a specified unit 
 * INPUT:   unit_id
 *          status : master button status
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is set successfully
 *          FALSE        -- otherwise
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_SetMasterButtonStatus(UI32_T unit_id, BOOL_T status);
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME : STKTPLG_PMGR_SetMasterButtonStatus
 * PURPOSE: process insert/remove a unit.
 * INPUT:   None.
 * OUTPUT:  *is_insertion   -- insertion or removal.
 *          *starting_port  -- starting port ID .
 *          *number_of_port -- port number .
 *          *use_default    -- use default setting if insertion. I
 *                             Ignore this argument if removal.
 * RETUEN:  TRUE  -- Need to process.
 *          FALSE -- Not necessary to process.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_PMGR_ProcessUnitInsertRemove(BOOL_T *is_insertion, 
                                          UI32_T *starting_port, 
                                          UI32_T *number_of_port,
                                          UI8_T *unit_id,
                                          BOOL_T *use_default);


void STKTPLG_PMGR_SetSlavePastMasterMac(void);

#endif
#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_PMGR_SetSfpThresholdAlarmWarningTrapEnable
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold alarm trap policy
 * INPUT   : unit -- which unit
 *           gbic_idx -- which gbic index
 *           trap_enable -- TRUE/FALSE
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_PMGR_SetSfpThresholdAlarmWarningTrapEnable(UI32_T unit, UI32_T gbic_idx, BOOL_T trap_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_PMGR_SetSfpThresholdAlarmWarningAutoMode
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold auto mode policy
 * INPUT   : unit -- which unit
 *           gbic_idx -- which gbic index
 *           auto_mode -- TRUE/FALSE
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_PMGR_SetSfpThresholdAlarmWarningAutoMode(UI32_T unit, UI32_T gbic_idx, BOOL_T auto_mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_PMGR_SetSfpDefaultThresholdAlarmWarning
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold to default
 * INPUT   : unit   -- which ifindex
 *           gbic_idx -- which gbic_idx
 *           threshold_type -- which threshold_type
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_PMGR_SetSfpDefaultThresholdAlarmWarning(UI32_T unit, UI32_T gbic_idx, UI32_T threshold_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_PMGR_SetSfpThresholdAlarmWarning
 * -------------------------------------------------------------------------
 * FUNCTION: Set SFP ddm threshold
 * INPUT   : unit   -- which ifindex
 *           gbic_idx -- which gbic_idx
 *           threshold_type -- which threshold_type
 *           val
 *
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_PMGR_SetSfpThresholdAlarmWarning(UI32_T unit, UI32_T gbic_idx, UI32_T threshold_type, double val);
#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/* FUNCTION NAME : STKTPLG_PMGR_SetStackingButtonStatus
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
BOOL_T STKTPLG_PMGR_SetStackingButtonStatus(BOOL_T status);
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_PMGR_SetCfgHwPortMode
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
BOOL_T STKTPLG_PMGR_SetCfgHwPortMode(
    UI32_T unit,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode);
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

#endif    /* End of STKTPLG_PMGR_H */
