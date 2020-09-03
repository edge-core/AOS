/* MODULE NAME:  stktplg_pom.c
 * PURPOSE:
 *     Implementations of stktplg pom.
 *
 * NOTES:
 *
 * REASON:
 * HISTORY
 *    7/16/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>

#include "sys_bld.h"
#include "sys_module.h"
#include "sys_cpnt.h"
#include "sysfun.h"

#include "stktplg_om.h"
#include "stktplg_pom.h"
#include "stktplg_shom.h"
#include "swctrl.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_POM_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG POM for the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_InitiateProcessResources(void)
{    
    return STKTPLG_OM_AttachProcessResources();
}

/* FUNCTION NAME : STKTPLG_POM_GetNumberOfUnit
 * PURPOSE: To get the number of units existing in the stack.
 * INPUT:   None.
 * OUTPUT:  num_of_unit  -- the number of units existing in the stack.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetNumberOfUnit(UI32_T *num_of_unit)
{
    return STKTPLG_OM_GetNumberOfUnit(num_of_unit);
}

/* FUNCTION NAME : STKTPLG_POM_GetLocalMaxPortCapability
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetLocalMaxPortCapability(UI32_T *max_port_number)
{
    return STKTPLG_OM_GetLocalMaxPortCapability(max_port_number);
}

/* FUNCTION NAME : STKTPLG_POM_GetMaxPortCapability
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   unit             -- unit id. (0:local device)
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_POM_GetMaxPortCapability(UI8_T unit, UI32_T *max_port_number)
{
    return STKTPLG_OM_GetMaxPortCapability(unit, max_port_number);
}


/* FUNCTION NAME : STKTPLG_POM_GetPortType
 * PURPOSE: To get the type of the specified port of the specified unit.
 * INPUT:   unit_id  	 -- unit id.
 *          port       -- phyical(user) port id
 * OUTPUT:  port_type    -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Reference leaf_es3526a.mib
 *          -- #define VAL_portType_other	1L
 *          -- #define VAL_portType_hundredBaseTX	2L
 *          -- #define VAL_portType_hundredBaseFX	3L
 *          -- #define VAL_portType_thousandBaseSX	4L
 *          -- #define VAL_portType_thousandBaseLX	5L
 *          -- #define VAL_portType_thousandBaseT	6L
 *          -- #define VAL_portType_thousandBaseGBIC	7L
 *          -- #define VAL_portType_thousandBaseMiniGBIC	8L
 *          -- #define VAL_portType_hundredBaseFxScSingleMode	9L
 *          -- #define VAL_portType_hundredBaseFxScMultiMode	10L
 *
 */
BOOL_T STKTPLG_POM_GetPortType(UI32_T unit, UI32_T port, UI32_T *port_type)
{
    return STKTPLG_OM_GetPortType(unit, port, port_type);
}

/* FUNCTION NAME : STKTPLG_POM_GetSfpPortType
 * PURPOSE: To get the type of fiber medium of the specified port of the specified unit.
 * INPUT:   unit_id  	 -- unit id.
 *          sfp_index
 * OUTPUT:  port_type_p  -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_POM_GetSfpPortType(UI32_T unit, UI32_T sfp_index, UI32_T *port_type_p)
{
    return STKTPLG_OM_GetSfpPortType(unit, sfp_index, port_type_p);
}

/* FUNCTION NAME : STKTPLG_POM_GetModuleID
 * PURPOSE: To get the module id of the specified port of the specified unit.
 * INPUT:   unit_id  	 -- unit id.
 *          u_port       -- phyical(user) port id
 * OUTPUT:  module_id    -- module id of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Here are two defined return module id
 *          SYS_HWCFG_MODULE_ID_10G_COPPER
 *          SYS_HWCFG_MODULE_ID_XFP
 *          SYS_HWCFG_MODULE_ID_10G_SFP
 *
 */
BOOL_T STKTPLG_POM_GetModuleID(UI32_T unit, UI32_T u_port, UI8_T *module_id)
{
    return STKTPLG_OM_GetModuleID(unit, u_port, module_id);
}

/* FUNCTION NAME : STKTPLG_POM_GetMyRuntimeFirmwareVer
 * PURPOSE: Get the runtime firmware version of the local unit.
 * INPUT:   None
 * OUTPUT:  runtime_fw_ver_ar - Runtime firmware version will be put to this array.
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_POM_GetMyRuntimeFirmwareVer(UI8_T runtime_fw_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1])
{
    return STKTPLG_OM_GetMyRuntimeFirmwareVer(runtime_fw_ver_ar);
}


/* FUNCTION NAME : STKTPLG_POM_GetLocalUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetLocalUnitBaseMac(UI8_T *base_mac_addr)
{
    return STKTPLG_OM_GetLocalUnitBaseMac(base_mac_addr);
}

#if (SYS_CPNT_MGMT_PORT == TRUE)
BOOL_T STKTPLG_POM_GetLocalUnitBaseMac_ForMgmtPort(UI8_T *base_mac_addr)
{
    return STKTPLG_OM_GetLocalUnitBaseMac_ForMgmtPort(base_mac_addr);
}
#endif

/* FUNCTION NAME : STKTPLG_POM_GetUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetUnitBaseMac(UI8_T unit, UI8_T *base_mac_addr)
{
    return STKTPLG_OM_GetUnitBaseMac(unit, base_mac_addr);
}

/* FUNCTION NAME: STKTPLG_POM_GetUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_POM_GetUnitBoardID(UI32_T unit, UI32_T *board_id)
{
    return STKTPLG_OM_GetUnitBoardID(unit, board_id);
}

/* FUNCTION NAME : STKTPLG_POM_PortExist
 * PURPOSE: This function is used to check if the specified port is
 *          existing or not.
 * INPUT:   logical_unit_id -- Logical unit ID
 *          logical_port_id -- logical port ID
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_PortExist(UI32_T unit_id, UI32_T port_id)
{
    return STKTPLG_OM_PortExist(unit_id, port_id);
}

/* FUNCTION NAME: STKTPLG_POM_GetBoardModuleTypeReg
 * PURPOSE: This function is used to get module type from board register.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetBoardModuleTypeReg(UI8_T module_index, UI8_T *module_type)
{
    return STKTPLG_OM_GetBoardModuleTypeReg(module_index, module_type);
}

/* FUNCTION NAME: STKTPLG_POM_GetModuleAType
 * PURPOSE: This function is used to get module 1 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetModuleAType(UI8_T *module_type)
{
    return STKTPLG_OM_GetModuleAType( module_type);
}


/* FUNCTION NAME: STKTPLG_POM_GetModuleBType
 * PURPOSE: This function is used to get module 2 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetModuleBType(UI8_T *module_type)
{
    return STKTPLG_OM_GetModuleBType(module_type);
}


/* FUNCTION NAME: STKTPLG_POM_GetOldModuleType
 * PURPOSE: This function is used to get old module type.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetOldModuleType(UI8_T module_index, UI8_T *module_type)
{
    return STKTPLG_OM_GetOldModuleType(module_index,module_type);
}


/* FUNCTION NAME: STKTPLG_POM_GetModuleAOldType
 * PURPOSE: This function is used to get module 1 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetModuleAOldType(UI8_T *module_type)
{
    return STKTPLG_OM_GetModuleAOldType(module_type);
}


/* FUNCTION NAME: STKTPLG_POM_GetModuleBOldType
 * PURPOSE: This function is used to get module 2 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetModuleBOldType(UI8_T *module_type)
{
    return STKTPLG_OM_GetModuleBOldType(module_type);
}


/* FUNCTION NAME: STKTPLG_POM_GetSysInfo
 * PURPOSE: This function is used to get this system information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *sys_info       -- sys info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This funxtion is used by sysmgmt to get system information.
 *
 */
BOOL_T STKTPLG_POM_GetSysInfo(UI32_T unit, STKTPLG_OM_Info_T *sys_info)
{
    return STKTPLG_OM_GetSysInfo(unit, sys_info);
}


/* FUNCTION NAME: STKTPLG_POM_GetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *device_info    -- device info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info)
{
    return STKTPLG_OM_GetDeviceInfo(unit, device_info);
}


/* FUNCTION NAME: STKTPLG_POM_GetModuleInfo
 * PURPOSE: This function is used to get this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info)
{
    return STKTPLG_OM_GetModuleInfo(unit,module_index,module_info);
}


/* FUNCTION NAME: STKTPLG_POM_SetModuleInfo
 * PURPOSE: This function is used to set this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *    In usual, set om operation should be done through MGR. This API is a
 *    special case because sysdrv need to call this API and sysdrv is not allowed
 *    to call STKTPLG_PMGR(SYSDRV is lower than STKTPLG).
 */
BOOL_T STKTPLG_POM_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info)
{
    return STKTPLG_OM_SetModuleInfo(unit,module_index,module_info);
}


/* FUNCTION NAME: STKTPLG_POM_SlaveIsReady
 * PURPOSE: This utiity function is to report is slave mode is initialized completely
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE: Yes.
 *          FALSE: No.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_SlaveIsReady(void)
{
    return STKTPLG_OM_SlaveIsReady();
}

/* FUNCTION NAME: STKTPLG_POM_GetSwitchOperState
 * PURPOSE: This function is used to get the whole system oper state.
 * INPUT:   *switch_oper_state -- buffer of oper state.
 * OUTPUT:  *switch_oper_state -- oper state.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   VAL_switchOperState_other           1
 *          VAL_switchOperState_unknown         2
 *          VAL_switchOperState_ok              3
 *          VAL_switchOperState_noncritical     4
 *          VAL_switchOperState_critical        5
 *          VAL_switchOperState_nonrecoverable  6
 */
BOOL_T STKTPLG_POM_GetSwitchOperState(UI8_T *switch_oper_state)
{
    return STKTPLG_OM_GetSwitchOperState(switch_oper_state);
}


/* FUNCTION NAME: STKTPLG_POM_GetMasterUnitId
 * PURPOSE: This routine is used to get master unit id.
 * INPUT:   *master_unit_id  -- master unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- successful
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetMasterUnitId(UI8_T *master_unit_id)
{
    return STKTPLG_OM_GetMasterUnitId(master_unit_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetStackingPort
 * PURPOSE: This function is used to get stacking port for a specified unit.
 * INPUT:   src_unit -- unit number
 * OUTPUT:  stack_port -- stacking port
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetSimplexStackingPort(UI32_T src_unit, UI32_T *stack_port)
{
    return STKTPLG_OM_GetSimplexStackingPort(src_unit,stack_port);
}


/* FUNCTION NAME: STKTPLG_POM_GetUnitBootReason
 * PURPOSE: This function is used to get boot reason of some unit.
 * INPUT:   unit -- unit.
 * OUTPUT:  *boot_reason -- boot reason.
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART
 */
BOOL_T STKTPLG_POM_GetUnitBootReason(UI32_T unit, UI32_T *boot_reason)
{
    return STKTPLG_OM_GetUnitBootReason(unit,boot_reason);
}


#if (SYS_CPNT_MAU_MIB == TRUE)

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetJackType
 * -------------------------------------------------------------------------
 * FUNCTION: Get jack type of some user port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 *           jack_index -- Which jack.
 * OUTPUT  : jack_type  -- VAL_ifJackType_other
 *                         VAL_ifJackType_rj45
 *                         VAL_ifJackType_rj45S
 *                         VAL_ifJackType_db9
 *                         VAL_ifJackType_bnc
 *                         VAL_ifJackType_fAUI
 *                         VAL_ifJackType_mAUI
 *                         VAL_ifJackType_fiberSC
 *                         VAL_ifJackType_fiberMIC
 *                         VAL_ifJackType_fiberST
 *                         VAL_ifJackType_telco
 *                         VAL_ifJackType_mtrj
 *                         VAL_ifJackType_hssdc
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetJackType (UI32_T unit,
                                UI32_T port,
                                UI32_T mau_index,
                                UI32_T jack_index,
                                UI32_T *jack_type)
{
    return STKTPLG_OM_GetJackType(unit,port,mau_index,jack_index,jack_type);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetMauMediaType
 * -------------------------------------------------------------------------
 * FUNCTION: Get media type of some MAU.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 * OUTPUT  : media_type -- STKTPLG_MGR_MEDIA_TYPE_E.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetMauMediaType (UI32_T unit, UI32_T port, UI32_T mau_index, UI32_T *media_type)
{
    return STKTPLG_OM_GetMauMediaType(unit,port,mau_index,media_type);
}
#endif /* (SYS_CPNT_MAU_MIB == TRUE) */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetPortMediaCapability
 * -------------------------------------------------------------------------
 * FUNCTION: Get media capability of port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port. *
 * OUTPUT  : media_cap  -- STKTPLG_TYPE_PORT_MEDIA_CAP_E
 * RETURN  : TRUE/FALSE
 * NOTE    : media_cap is a bitmap of possible media capability. It's
 *           useful to identify the combo port
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetPortMediaCapability(UI32_T unit, UI32_T port, UI32_T* media_cap)
{
    return STKTPLG_OM_GetPortMediaCapability(unit,port,media_cap);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetNextUnit(UI32_T *unit_id)
{
    return STKTPLG_OM_GetNextUnit(unit_id);
}


/* FUNCTION NAME : STKTPLG_POM_GetPortMapping
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit)
{
    return STKTPLG_OM_GetPortMapping(mapping,unit);
}


/* FUNCTION NAME : STKTPLG_POM_IsTransition
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
BOOL_T STKTPLG_POM_IsTransition(void)
{
    return STKTPLG_OM_IsTransition();
}


/* FUNCTION NAME : STKTPLG_POM_IsProvisionCompleted
 * PURPOSE  : To check if the system provision complete or not.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE.
 * Notes    : None.
 */
BOOL_T STKTPLG_POM_IsProvisionCompleted(void)
{
    return STKTPLG_OM_IsProvisionCompleted();
}


/* FUNCTION NAME : STKTPLG_POM_GetStackingDBEntry
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry)
{
    return STKTPLG_OM_GetStackingDBEntry(db, entry);
}


/* FUNCTION NAME : STKTPLG_POM_GetStackingDBEntryByMac
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
UI32_T STKTPLG_POM_GetStackingDBEntryByMac(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type)
{
    return STKTPLG_OM_GetStackingDBEntryByMac(mac,device_type);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetNextUnitUp
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID Up
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetNextUnitUp(UI32_T *unit_id)
{
    return STKTPLG_OM_GetNextUnitUp(unit_id);
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetUnitsRelPosition
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_a       -- Which Unit
 * INPUT   : unit_b       -- Which Unit
 * OUTPUT  : position     -- Relative position from unit_a,
 *                           high-bit set if unit_b is in the Down direction
 *                           of unit_a.
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_POM_GetUnitsRelPosition(UI32_T unit_a, UI32_T unit_b, UI32_T *position)
{
    return STKTPLG_OM_GetUnitsRelPosition(unit_a,unit_b, position);
}


/* FUNCTION NAME: STKTPLG_POM_ExpModuleIsInsert
 * PURPOSE: This function is used to check  slot-in module. insert or not
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- insert and ready
 *          FALSE -- non-insert ,or un-ready
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_ExpModuleIsInsert(UI32_T unit_id)
{
    return STKTPLG_OM_ExpModuleIsInsert(unit_id);
}


void STKTPLG_POM_GetHiGiPortNum(UI32_T *port_num)
{
    return STKTPLG_OM_GetHiGiPortNum(port_num);
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_OptionModuleIsExist
 *---------------------------------------------------------------------------
 * PURPOSE:  to  check the unit  insered the option module  or not
 * INPUT:    unit :  unit
 * OUTPUT:   ext_unit_id :option module  unit id
 * RETURN:   TRUE   : exist
 *           FALSE  : non-exist
 * NOTE:     1. this function is for a specify unit only  or  for each unit existing in the stacking
 *              it should check the unit insered the option module  or not
 *              if it has option module, the isc_remote_call or isc_send  also need send to the option module
 *          example:
 *                   isc_remote_call(unit, );
 *                   if(STKTPLG_POM_OptionModule_Exist(unit,&ext_unit_id)
 *                   {
 *                       isc_remote_call(unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK,);
 *                   }
 *           2. now is only one option module
 *           STKTPLG_POM_OptionModuleIsExist(UI32_T unit, UI32_T  option_nums) int the feature
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_POM_OptionModuleIsExist(UI32_T unit,UI32_T *ext_unit_id)
{
    return STKTPLG_OM_OptionModuleIsExist(unit,ext_unit_id);
}


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_IsLocalUnit
 *---------------------------------------------------------------------------
 * PURPOSE:  to  the port of  the specify unit  is belong to local or remote unit
 * INPUT:    unit :  destination unit
 *           port :
 * OUTPUT:   ext_unit_id  :  unit id
 * RETURN:   TRUE   : is local
 *           FALSE   : is remote
 * NOTE:     this function is for (unit,port)
 *           x*n+y :
 *           its meaning the option module x  of unit #y
 *           n: SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK
 *           when x=0 , y  is mainborad

 *
 *----------------------------------------------------------------
 */
BOOL_T STKTPLG_POM_IsLocalUnit(UI32_T unit,UI32_T port,UI32_T *ext_unit_id)
{
    return STKTPLG_OM_IsLocalUnit(unit,port,ext_unit_id);
}


/* FUNCTION NAME :
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_POM_GetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping[][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    return STKTPLG_OM_GetAllUnitsPortMapping(mapping);
}


/* FUNCTION NAME : STKTPLG_POM_GetMaxPortNumberOfModuleOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_POM_GetMaxPortNumberOfModuleOnBoard(UI8_T unit, UI32_T *max_option_port_number)
{
    return STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(unit,max_option_port_number);
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetModuleHiGiPortNum
 *---------------------------------------------------------------------------
 * PURPOSE:  To provide dev_nic HiGi port number for different modules
 * INPUT:    IUC_unit_id        ---IUC_unit_id
 * OUTPUT:   port_num           ---HiGi port number
 * RETURN:   None
 * NOTE:     Hard coded to retrieve module type for the first module
 *           in the array. Also assuming that every mainboard only
 *           have one module only.
 *----------------------------------------------------------------
 */
UI8_T STKTPLG_POM_GetModuleHiGiPortNum(UI16_T IUC_unit_id)
{
    return STKTPLG_OM_GetModuleHiGiPortNum(IUC_unit_id);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetUnitModulePorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in module.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit or Module not present.
 *           FALSE --- Got information.
 * NOTE:
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_POM_GetUnitModulePorts(UI32_T unit_id, UI32_T *start_port_id, UI32_T *nbr_of_ports)
{
    return STKTPLG_OM_GetUnitModulePorts(unit_id,start_port_id, nbr_of_ports);
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetUnitMainboardPorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in mainboard.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit not present.
 *           FALSE --- Got information.
 * NOTE:     Base on current synced module database.
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_POM_GetUnitMainboardPorts(UI32_T unit, UI32_T *start_port_id, UI32_T *nbr_of_ports)
{
    return STKTPLG_OM_GetUnitMainboardPorts(unit,start_port_id,nbr_of_ports);
}



/* FUNCTION NAME : STKTPLG_POM_GetStackStatus
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
BOOL_T STKTPLG_POM_GetStackStatus(BOOL_T *is_stack_status_normal)
{
    return STKTPLG_OM_GetStackStatus(is_stack_status_normal);
}


/* FUNCTION NAME : STKTPLG_POM_GetUnitStatus
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
BOOL_T STKTPLG_POM_GetUnitStatus(UI32_T unit, BOOL_T *is_unit_status_normal)
{
    return STKTPLG_OM_GetUnitStatus(unit,is_unit_status_normal);
}


/* FUNCTION NAME : STKTPLG_POM_GetModuleStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_module_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_POM_GetModuleStatus(UI32_T unit, UI32_T module, BOOL_T *is_module_status_normal)
{
    return STKTPLG_OM_GetModuleStatus(unit,module,is_module_status_normal);
}


/* FUNCTION NAME : STKTPLG_POM_GetModuleType
 * PURPOSE  : STKTPLG to get module type.
 * INPUT    : unit -- Which unit.
 *            module -- which module.
 * OUTPUT     module_type -- Type of module.
 * RETUEN   : TRUE/FALSE
 * Notes    : None.
 */
BOOL_T STKTPLG_POM_GetModuleType(UI32_T unit, UI32_T module, UI32_T *module_type)
{
    return STKTPLG_OM_GetModuleType(unit,module, module_type);
}


/* FUNCTION NAME : STKTPLG_POM_IsValidDriverUnit
 * PURPOSE  : To know the driver is valid or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- Version is valid.
 *            FALSE -- For main board: Version is different from master main board.
 *                     For module: Version is dofferent from expected module version.
 */
BOOL_T STKTPLG_POM_IsValidDriverUnit(UI32_T driver_unit)
{
    return STKTPLG_OM_IsValidDriverUnit(driver_unit);
}


/* FUNCTION NAME : STKTPLG_POM_GetMyDriverUnit
 * PURPOSE  : To get my driver unit ID.
 * INPUT    : None.
 * OUTPUT   : *my_driver_unit -- my driver ID.
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_POM_GetMyDriverUnit(UI32_T *my_driver_unit)
{
    return STKTPLG_OM_GetMyDriverUnit(my_driver_unit);
}


/* FUNCTION NAME : STKTPLG_POM_DriverUnitExist
 * PURPOSE  : To know the driver exist or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_POM_DriverUnitExist(UI32_T driver_unit)
{
    return STKTPLG_OM_DriverUnitExist(driver_unit);
}


/* FUNCTION NAME : STKTPLG_POM_GetNextDriverUnit
 * PURPOSE  : To get next driver unit.
 * INPUT    : Next to which driver unit.
 * OUTPUT   : Next driver is which.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_POM_GetNextDriverUnit(UI32_T *driver_unit)
{
    return STKTPLG_OM_GetNextDriverUnit( driver_unit);
}


/* FUNCTION NAME: STKTPLG_POM_GetCurrentStackingDB
 * PURPOSE: This API is used to get the mac and unit id and device type
 * in the stacking
 * INPUT:   None.
 * OUTPUT:  stacking_db : array of structure.
 * RETUEN:  0  -- failure
 *          otherwise -- success. the returned value is the number of entries
 * NOTES:   None.
 */

UI8_T STKTPLG_POM_GetCurrentStackingDB(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
    return STKTPLG_OM_GetCurrentStackingDB(stacking_db);
}


/* FUNCTION NAME: STKTPLG_POM_SfpIndexToUserPort
 * PURPOSE: This function is translate from SFP index to user port.
 * INPUT:   unit      -- which unit.
 *          sfp_index -- which SFP.
 * OUTPUT:  port -- which user port
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_POM_SfpIndexToUserPort(UI32_T unit, UI32_T sfp_index, UI32_T *port)
{
    return STKTPLG_OM_SfpIndexToUserPort(unit,sfp_index, port);
}

/* FUNCTION NAME: STKTPLG_POM_UserPortToSfpIndex
 * PURPOSE: This function is translate from user port to SFP index.
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_POM_UserPortToSfpIndex(UI32_T unit, UI32_T port, UI32_T *sfp_index)
{
    return STKTPLG_OM_UserPortToSfpIndex( unit,port, sfp_index);
}

/* FUNCTION NAME : STKTPLG_POM_GetMainBoardPortNum
 * PURPOSE: To get the max port number on main board .(24/48)
 * INPUT:   NONE.
 * OUTPUT:  portnum -- max port number on main board
 * RETUEN:  NONE
 * NOTES:  This API is useless for mainboard.
 *
 */
void  STKTPLG_POM_GetMainBoardPortNum(UI32_T  *portnum)
{
    return STKTPLG_OM_GetMainBoardPortNum(portnum);
}

/* FUNCTION NAME : STKTPLG_POM_GetMyModuleID
 * PURPOSE: To get my module type and assigned module ID.
 * INPUT:   NONE.
 * OUTPUT:  mymodid -- my assigned module ID
 *          mymodtype -- my module type
 * RETUEN:  NONE
 * NOTES:   This API is useless for mainboard.
 *
 */
void STKTPLG_POM_GetMyModuleID(UI8_T *mymodid,UI8_T *mymodtype)
{
    return STKTPLG_OM_GetMyModuleID(mymodid,mymodtype);
}
/* FUNCTION NAME : STKTPLG_POM_GetLocalModulePortNumber
 * PURPOSE: To get the max port number of module conncted to me .
 * INPUT:   NONE.
 * OUTPUT:  portnum -- max port number of module
 * RETUEN:  FALSE : no module inserted
            TRUE  : otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetLocalModulePortNumber(UI32_T *portnum)
{
    return STKTPLG_OM_GetLocalModulePortNumber(portnum);
}


/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_UnitModuleToExtUnit
 *---------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *----------------------------------------------------------------
 */
void STKTPLG_POM_UnitModuleToExtUnit( UI32_T unit,UI32_T module_x,UI32_T *ext_unit_id)
{
    return STKTPLG_OM_UnitModuleToExtUnit(unit,module_x, ext_unit_id);
}

/* FUNCTION NAME : STKTPLG_POM_IsModulePort
 * PURPOSE: This function check if a port is a module port
 * INPUT:   unit:
 *          port:
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- is a module port
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T	STKTPLG_POM_IsModulePort(UI32_T unit, UI32_T port)
{
    return STKTPLG_OM_IsModulePort(unit,port);
}


/* FUNCTION NAME : STKTPLG_POM_PortList2DriverUnitList
 * PURPOSE: This function is to convert the port bitmap into driver unit bitmap
 * INPUT:   port_list: the port bitmap, size: SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * OUTPUT:  driver_unit_list: the driver unit bitmap
 * RETUEN:  NONE
 * NOTES:
 *
 */
void STKTPLG_POM_PortList2DriverUnitList(UI8_T *port_list, UI32_T *driver_unit_list)
{
    return STKTPLG_OM_PortList2DriverUnitList( port_list, driver_unit_list);
}


#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
/* FUNCTION NAME : STKTPLG_POM_GetMasterButtonStatus
 * PURPOSE: This function is to get the master button status setting for a specified unit
 * INPUT:   unit_id
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is enabled
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetMasterButtonStatus(UI32_T unit_id)
{
    return STKTPLG_OM_GetMasterButtonStatus(unit_id);
}


/* FUNCTION NAME : STKTPLG_POM_IsHBTTooOld
 * PURPOSE  : check if runime version of slave is too old to have different HBT2 format.
 * INPUT    : *runtime_sw_ver: runtime version
 * OUTPUT   : None
 * RETUEN   : TRUE: Old format of HBT2
 *            FALSE: Otherwise
 */
BOOL_T STKTPLG_POM_IsHBTTooOld(UI8_T *runtime_sw_vesion)
{
    return STKTPLG_OM_IsHBTTooOld(runtime_sw_vesion);
}


#endif /* end of if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE) */

/* FUNCTION NAME : STKTPLG_POM_IsAllSlavesAndModlesWithMC
 * PURPOSE  : check if any version of slave or optional module is too old to have MC mechanism.
 * INPUT    : None.
 * OUTPUT   : None
 * RETUEN   : TRUE: Normal
 *            FALSE: Some slave or module is too old
 */
BOOL_T STKTPLG_POM_IsAllSlavesAndModulesWithMC(void)
{
    return STKTPLG_OM_IsAllSlavesAndModulesWithMC();
}


/* FUNCTION NAME: STKTPLG_POM_Logical2PhyDevicePortID
 * PURPOSE: This function is used to convert logical address mode to
 *          physical address mode.
 * INPUT:   unit_id -- unit id of logical view
 *          port_id -- port id of logical view
 * OUTPUT:  phy_device_id   -- device id of physical view
 *          phy_port_id     -- port id of physical view
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_Logical2PhyDevicePortID(UI32_T unit_id,
                                           UI32_T port_id,
                                           UI32_T *phy_device_id,
                                           UI32_T *phy_port_id)
{
    return STKTPLG_OM_Logical2PhyDevicePortID(unit_id,port_id, phy_device_id, phy_port_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetMaxChipNum
 * PURPOSE: Get the switch chip number in this unit.
 * INPUT:   unit   -- unit number.
 * OUTPUT:  *max_chip_nmber  -- maximum chip number.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_POM_GetMaxChipNum(UI32_T unit, UI32_T *max_chip_nmber)
{
    return STKTPLG_OM_GetMaxChipNum(unit, max_chip_nmber);
}


/* FUNCTION NAME: STKTPLG_POM_GetNeighborStackingPort
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(broadcom view)-- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_POM_GetNeighborStackingPort(UI32_T src_unit, UI32_T src_chip_id,
                                           UI32_T specific_unit, UI32_T specific_chip_id,
                                           UI32_T *port)
{
    return STKTPLG_OM_GetNeighborStackingPort(src_unit,src_chip_id, specific_unit, specific_chip_id, port);
}

/* FUNCTION NAME: STKTPLG_POM_GetNeighborStackingPortByChipView
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(chip view)    -- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip
 *                                 (return chip physical port).
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by HR module.
 *
 */
BOOL_T STKTPLG_POM_GetNeighborStackingPortByChipView(UI32_T src_unit, UI32_T src_chip_id,
                                                     UI32_T specific_unit, UI32_T specific_chip_id,
                                                     UI32_T *port)
{
    return STKTPLG_OM_GetNeighborStackingPortByChipView(src_unit,src_chip_id, specific_unit, specific_chip_id, port);
}


/* FUNCTION NAME: STKTPLG_POM_GetNextUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_POM_GetNextUnitBoardID(UI32_T *unit, UI32_T *board_id)
{
    return STKTPLG_OM_GetNextUnitBoardID(unit, board_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_POM_GetUnitFamilyID(UI32_T unit, UI32_T *family_id)
{
    return STKTPLG_OM_GetUnitFamilyID(unit, family_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetNextUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_POM_GetNextUnitFamilyID(UI32_T *unit, UI32_T *family_id)
{
    return STKTPLG_OM_GetNextUnitFamilyID(unit, family_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_POM_GetUnitProjectID(UI32_T unit, UI32_T *project_id)
{
    return STKTPLG_OM_GetUnitProjectID(unit, project_id);
}


/* FUNCTION NAME: STKTPLG_POM_GetNextUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_POM_GetNextUnitProjectID(UI32_T *unit, UI32_T *project_id)
{
    return STKTPLG_OM_GetNextUnitProjectID(unit,project_id);
}


/* FUNCTION NAME : STKTPLG_POM_GetStackingState
 * PURPOSE: To get the stacking state
 * INPUT:   None.
 * OUTPUT:  state        -- The stacking state
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetStackingState(UI32_T *state)
{
    return STKTPLG_OM_GetStackingState(state);
}


/* FUNCTION NAME: STKTPLG_POM_IsStkTplgStateChanged
 * PURPOSE: This function is used to let other cpnt to know whether the state is changed in real time
 * INPUT:   stk_state == stktplg state
 *				STKTPLG_OM_SIMPLE_STATE_ARBITRATION = 0,
 *    			STKTPLG_OM_SIMPLE_STATE_MASTER,
 *   			STKTPLG_OM_SIMPLE_STATE_SLAVE
 * OUTPUT:  none
 * RETUEN:  TRUE :  changed
 *          FALSE : not changed
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_IsStkTplgStateChanged(STKTPLG_OM_Simple_State_T stk_state)
{
    return STKTPLG_OM_IsStkTplgStateChanged(stk_state);
}



#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_POM_IsAnyUnitNeedToProcess
 * PURPOSE: This function is used to check if there is unit need to process
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE :  have units to process
 *          FALSE : none
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_IsAnyUnitNeedToProcess(void)
{
    return STKTPLG_OM_IsAnyUnitNeedToProcess();
}

#endif

#if (SYS_CPNT_POE==TRUE)
/* FUNCTION: STKTPLG_POM_IsPoeDevice
 * PURPOSE:  For POE_MGR to check if some unit is POE device or not.
 * INPUT:    unit_id -- The unit that caller want to check.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- This unit is a POE device.
 *           FALSE -- This unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_POM_IsPoeDevice(UI32_T unit_id)
{
    return STKTPLG_OM_IsPoeDevice(unit_id);
}

/* FUNCTION: STKTPLG_POM_IsLocalPoeDevice
 * PURPOSE:  For POE_MGR to check if local unit is POE device or not.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Local unit is a POE device.
 *           FALSE -- Local unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_POM_IsLocalPoeDevice()
{
    return STKTPLG_OM_IsLocalPoeDevice();
}
#endif

/* LOCAL SUBPROGRAM BODIES
 */

#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 4:47:06 */

/* FUNCTION NAME : STKTPLG_POM_GetMyUnitID
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetMyUnitID(UI32_T *my_unit_id)
{
    return STKTPLG_OM_GetMyUnitID(my_unit_id);
}


/* FUNCTION NAME: STKTPLG_POM_UnitExist
 * PURPOSE: This function is used to check if the specified unit is
 *          existing or not.
 * INPUT:   unit_id  -- unit id
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- exist
 *          FALSE  -- not exist
 * NOTES:   Use got mac address of each unit to
 *          know if unit exist or not.
 *
 */
BOOL_T STKTPLG_POM_UnitExist(UI32_T unit_id)
{
    return STKTPLG_OM_UnitExist(unit_id);
}


/* FUNCTION NAME : STKTPLG_POM_GetMaxPortNumberOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_POM_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number)
{
    return STKTPLG_OM_GetMaxPortNumberOnBoard(unit, max_port_number);
}


/* FUNCTION NAME : STKTPLG_POM_IsOptionModule
 * PURPOSE  : STKTPLG to check if I am a module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_POM_IsOptionModule(void)
{
    return STKTPLG_OM_IsOptionModule();
}

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)

/* FUNCTION NAME : STKTPLG_POM_GetStackingState
 * PURPOSE: To get the stacking state
 * INPUT:   None.
 * OUTPUT:  state        -- The stacking state
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetStackingPortInfo(UI32_T *state,UI32_T *uplinkport,UI32_T *downlinkport)
{
    return STKTPLG_OM_GetStackingPortInfo(state, uplinkport, downlinkport);
}

#endif

#else /* SYS_CPNT_STKTPLG_SHMEM */

/* FUNCTION NAME : STKTPLG_POM_GetMyUnitID
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetMyUnitID(UI32_T *my_unit_id)
{
    if(my_unit_id==NULL)
        return FALSE;

    return STKTPLG_SHOM_GetMyUnitID(my_unit_id);

}
/* FUNCTION NAME: STKTPLG_POM_UnitExist
 * PURPOSE: This function is used to check if the specified unit is
 *          existing or not.
 * INPUT:   unit_id  -- unit id
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- exist
 *          FALSE  -- not exist
 * NOTES:   Use got mac address of each unit to
 *          know if unit exist or not.
 *
 */
BOOL_T STKTPLG_POM_UnitExist(UI32_T unit_id)
{
   return STKTPLG_SHOM_UnitExist(unit_id);
}

/* FUNCTION NAME : STKTPLG_POM_GetMaxPortNumberOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_POM_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number)
{
     if(max_port_number==NULL)
        return FALSE;

     return STKTPLG_SHOM_GetMaxPortNumberOnBoard(unit,max_port_number);

 }

/* FUNCTION NAME : STKTPLG_POM_IsOptionModule
 * PURPOSE  : STKTPLG to check if I am a module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_POM_IsOptionModule(void)
{  
    return STKTPLG_SHOM_IsOptionModule();
}


#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)

/* FUNCTION NAME : STKTPLG_POM_GetStackingState
 * PURPOSE: To get the stacking state
 * INPUT:   None.
 * OUTPUT:  state        -- The stacking state
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_POM_GetStackingPortInfo(UI32_T *state,UI32_T *uplinkport,UI32_T *downlinkport)
{
    return STKTPLG_SHOM_GetStackingPortInfo(state,uplinkport,downlinkport);


}
#endif

/* FUNCTION NAME : STKTPLG_POM_GetConfigTopologyInfoDoneStatus
 * PURPOSE: To get the status of configuring topology info.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- STKTPLG had configured topology
 *                    info to lower layer.
 *          FALSE  -- STKTPLG had not configured
 *                    topology info to lower layer.
 * NOTES:
 *          Tables in ASIC need to be initialized after
 *          the module id had been configured through
 *          the operation "Configure Topology Info" in
 *          STKTPLG. Thus in SWDRV_EnterTransitionMode(),
 *          it needs to poll this status and call
 *          DEV_PMGR_ResetOnDemand() to re-initialize ASIC
 *          after "Configure Topology Info" is finished.
 */
BOOL_T STKTPLG_POM_GetConfigTopologyInfoDoneStatus(void)
{
    return STKTPLG_SHOM_GetConfigTopologyInfoDoneStatus();
}

#endif /* end of #if (SYS_CPNT_STKTPLG_SHMEM == FALSE) */

#ifdef ASF4526B_FLF_P5
BOOL_T STKTPLG_POM_DetectSfpInstall(UI32_T unit, UI32_T port, BOOL_T *is_present)
{
	return STKTPLG_OM_DetectSfpInstall(unit, port, is_present);
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_GetModulePortListByModuleSlotIndex
 * -------------------------------------------------------------------------
 * FUNCTION: To get port list of the specified module slot
 * INPUT   : module_slot_index
 *           module_id
 * OUTPUT  : portlist - module port list
 * RETURN  : number of module ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T STKTPLG_POM_GetModulePortListByModuleSlotIndex(UI8_T module_slot_index, UI8_T module_id, UI32_T *portlist)
{
    return STKTPLG_OM_GetModulePortListByModuleSlotIndex(module_slot_index, module_id, portlist);
}

