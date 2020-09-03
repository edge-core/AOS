/* Module Name: STKTPLG_BOARD.H
 *
 * Purpose: 
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 
 *    2004-05-18    wuli
 *                  add  type define STKTPLG_MODULE_BoardType_E
 *      change function BOOL_T STKTPLG_BOARD_GetModuleInformation(STKTPLG_BOARD_ModuleType_T module_type, 
                                                                  STKTPLG_BOARD_ModuleInfo_T **module_info);
 *  
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */
 
#ifndef  STKTPLG_BOARD_H
#define  STKTPLG_BOARD_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "dev_swdrv.h"
#include "sys_hwcfg.h"
#include "sys_adpt.h"
#include "stktplg_type.h"
#include "i2c.h"

/* DATA TYPE DECLARATIONS
 */

/* wuli, 2004-05-18 
 * define module types 
 */
typedef enum STKTPLG_MODULE_BoardType_E
{
    MODULE_TYPE_8SFX = 0,
    MODULE_TYPE_ONE_PORT_10G,
    MODULE_TYPE_MAX		
} STKTPLG_BOARD_ModuleType_T;	

typedef struct STKTPLG_BOARD_Port2ModuleIndex_S
{
    UI16_T port;
    UI8_T  module_index;
} STKTPLG_BOARD_Port2ModuleIndex_T;

typedef struct STKTPLG_BOARD_Port2ModuleSlotIndex_S
{
    UI16_T port;
    UI8_T  module_slot_index;
} STKTPLG_BOARD_Port2ModuleSlotIndex_T;

/* define data structure for board information 
 * this is the first draft, later if you find you have more information
 * that should be added to this table, you can add them.
 * later you can get these different information via different board id.
 */
 
typedef struct STKTPLG_BOARD_BoardInfo_S
{
    /* how many ports are installed on mainboard
     */	
    UI8_T    max_port_number_on_board;
    /* how many ports are hot swap
     */
    UI8_T    max_hot_swap_port_number;
    /* how many slots are possible for expansion module
     */
    UI8_T    max_slots_for_expansion;
    /* start expansion port number: from this port hooking expansion slots
     */
    UI8_T    start_expansion_port_number;
    /* how many sfp port in the board (Including sfp port of combo port)
     */
    UI8_T    sfp_port_number;
    /* starting sfp port number
     */
    UI8_T    start_sfp_port_number;

    /* how many qsfp port in the board
     */
    UI8_T    qsfp_port_number;

    /* which port the qsfp start from
     */
    UI8_T    qsfp_port_start;

    /* which port the break-out port start from
     */
    UI8_T    break_out_port_start;

    /* how many ports are possible for this unit
     */
    UI8_T    max_port_number_of_this_unit;
    /* how many switch chips are installed for this unit
     */
    UI8_T    chip_number_of_this_unit;   
    /* the mapping between first expansion port to module
     */
    UI8_T    port2ModId_ptr[4];
	/* the number of possible mau for a port
	 */
    UI8_T    portMediaNum_ptr[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
	/* the media type of a port
     */ 
    UI8_T    portMediaType_ptr[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    /* the jack type of a port
     */
    UI8_T    portJackType_ptr[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]; 
    /* the figure type of a port
     */
    UI8_T    portFigureType_ptr[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    /* the media capability of a port
     */
    UI8_T    portMediaCap_ptr[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]; 
    /* device id of internal stacking port if we have two switch chips
     */ 
    UI8_T   *stackingPort2phyDevId_ptr;
    /* port id of internal stacking port if we have two switch chips
     */
    UI8_T   *stackingPort2phyPortId_ptr;

    /* this array keeps port type of fixed port, for slot-in modules, we
     * will detect them on the fly
     */
    DEV_SWDRV_Device_Port_Mapping_T    userPortMappingTable[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]; 
    
    /* indicate we adopt simplex mode for stacking or not
     * 1.if we use sipmlex mode for stacking, we only need one expansion
     *   module for stacking
     * 2.if we use duplex mode for stacking, we need two expansion module for
     *   stacking.
     */
    BOOL_T   simplex_stacking;
    /* uplink port for stacking, if simplex, the same as downlink port
     */
    UI8_T    stacking_uplink_port;
    /* downlink port for stacking, if simplex, the same as uplink port
     */
    UI8_T    stacking_downlink_port;
    
    /* whether support Redunant Power Unit or not*/
    BOOL_T   is_support_rpu;

    UI8_T    fan_number;

    UI8_T    thermal_number;
} STKTPLG_BOARD_BoardInfo_T;

/* wuli, 2004-05-18 */
typedef struct STKTPLG_BOARD_ModuleInfo_S
{
    DEV_SWDRV_Device_Port_Mapping_T    userPortMappingTable[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]; 
} STKTPLG_BOARD_ModuleInfo_T;

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
typedef struct
{
    UI32_T mapping_uport;
    UI32_T drv_device_id;
    UI32_T drv_logical_port;
    UI32_T drv_physical_port;
    UI32_T dflt_hw_port_mode;
}
STKTPLG_BOARD_HwPortModeInfo_T;
#endif


/* EXPORTED SUBPROGRAM DECLARATIONS
 */
 
BOOL_T STKTPLG_BOARD_InitiateProcessResources(void);
BOOL_T STKTPLG_BOARD_AttachProcessResources(void);

/* FUNCTION NAME : STKTPLG_BOARD_SetDebugMode
 * PURPOSE : This function will set debug mode
 * INPUT   : debug_mode
 * OUTPUT  : None.
 * RETUEN  : None.
 * NOTES   : None.
 */
void STKTPLG_BOARD_SetDebugMode(BOOL_T debug_mode);

/* FUNCTION NAME : STKTPLG_BOARD_GetDebugMode
 * PURPOSE : This function will get debug mode
 * INPUT   : debug_mode
 * OUTPUT  : None.
 * RETUEN  : debug_mode
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetDebugMode(void);

/* FUNCTION NAME : STKTPLG_BOARD_GetEPLDVer
 * PURPOSE : This function will get the epld version string
 * INPUT   : board_id      - board id of the local unit
 * OUTPUT  : epld_ver_p    - the epld version string
*            epld_ver_val_p- the raw value of the epld version register
 * RETUEN  : TRUE  -  Success
 *           FALSE -  Failed
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetEPLDVer(UI32_T board_id, char epld_ver_p[SYS_ADPT_EPLD_VER_STR_LEN+1], UI8_T* epld_ver_val_p);

/* FUNCTION NAME : STKTPLG_BOARD_GetBoardInformation
 * PURPOSE: This function will return corresponding board information based
 *          on board ID.
 * INPUT:   board_id -- board ID for this unit, it is project independent.
 * OUTPUT:  board_info -- correponding board information.
 * RETUEN:  TRUE -- successful
 *          FALSE -- fail          
 * NOTES:
 *          Due to board information table is a fixed table, caller only passes
 *          a pointer for board_info, this function will return related pointer
 *          to corresponding board information entry.
 */
BOOL_T STKTPLG_BOARD_GetBoardInformation(UI8_T board_id, STKTPLG_BOARD_BoardInfo_T *board_info);

/* wuli, 2004-05-18 
 * module_id  shall be use STKTPLG_BOARD_ModuleType_T 
 */
BOOL_T STKTPLG_BOARD_GetModuleInformation(STKTPLG_BOARD_ModuleType_T module_id, STKTPLG_BOARD_ModuleInfo_T **module_info);

/*05/03/2004 10:26¤W¤È vivid added to support read port number of option module type
 */
BOOL_T  STKTPLG_BOARD_GetPortNumberInformation(UI8_T module_id, UI32_T *port_nbr);

UI8_T STKTPLG_BOARD_GetMaxPortNumberOnBoard(void);

UI8_T STKTPLG_BOARD_GetMaxHotSwapPortNumber(void);

UI8_T STKTPLG_BOARD_GetMaxSlotsForExpansion(void);

UI8_T STKTPLG_BOARD_GetStartExpansionPortNumber(void);

UI8_T STKTPLG_BOARD_GetSfpPortNumber(void);

UI8_T STKTPLG_BOARD_GetStartSfpPortNumber(void);

UI8_T STKTPLG_BOARD_GetMaxPortNumberOfThisUnit(void);

UI8_T STKTPLG_BOARD_GetChipNumberOfThisUnit(void);

/* FUNCTION NAME : STKTPLG_BOARD_GetPortFigureType
 * PURPOSE : This function will output the port figure type by the specified board
 *           id and port id. 
 * INPUT   : board_id -- board ID
 *           port     -- port ID
 * OUTPUT  : port_figure_type_p -- port figure type of the specified port.
 * RETUEN  : TRUE  -- Output port figure type successfully.
 *           FALSE -- Failed to output port figure type.
 * NOTES   : None.
 */
BOOL_T STKTPLG_BOARD_GetPortFigureType(UI8_T board_id, UI16_T port, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p);

UI8_T STKTPLG_BOARD_GetFanNumber(void);

BOOL_T STKTPLG_BOARD_SetBoardInformation(UI8_T board_id, STKTPLG_BOARD_BoardInfo_T board_info);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetFanControllerInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get fan controller related info for the given fan index.
 * INPUT:   fan_idx         -- the index of the fan (1 based)
 * OUTPUT:  fan_ctl_info_p  -- fan controller related info, see the comment for
 *                             SYS_HWCFG_FanControllerInfo_T in sys_hwcfg_common.h
 *                             for details.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetFanControllerInfo(UI32_T fan_idx, SYS_HWCFG_FanControllerInfo_T * fan_ctl_info_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetThermalControllerInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get thermal controller related info for the given thermal index.
 * INPUT:   thermal_idx     -- the index of the thermal sensor(1 based)
 * OUTPUT:  thermal_ctl_info_p
 *                          -- thermal controller related info, see the comment for
 *                             SYS_HWCFG_ThermalControlInfo_T in sys_hwcfg_common.h
 *                             for details.
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetThermalControllerInfo(UI32_T thermal_idx, SYS_HWCFG_ThermalControlInfo_T *thermal_ctl_info_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetPowerStatusInfoByBoardId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get power status related info for the given power index of the
 *          given board id.
 * INPUT:   board id      -- board id of the power status info to be got
 *          power_idx     -- the index of the power (1 based)
 * OUTPUT:  power_status_reg_info_p  -- power status register related info
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetPowerStatusInfoByBoardId(UI32_T board_id, UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetPowerStatusInfo
 *---------------------------------------------------------------------------------
 * PURPOSE: Get power status related info for the given power index of the local
 *          unit.
 * INPUT:   power_idx     -- the index of the power (1 based)
 * OUTPUT:  power_status_reg_info_p  -- power status register related info
 * RETUEN:  TRUE if success. FALSE if failed.
 * NOTES:   This function is called when SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD is TRUE.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetPowerStatusInfo(UI32_T power_idx, SYS_HWCFG_PowerRegInfo_T *power_status_reg_info_p);

UI8_T STKTPLG_BOARD_GetThermalNumber(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_BOARD_GetONLPSFPPortMapping
 *---------------------------------------------------------------------------------
 * PURPOSE: This function output the array which maping the AOS user port id
 *          to ONLP port id.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  Number of thermal on this local unit.
 * NOTES:   All unused ports or non-sfp ports should be filled with ONLP_SFP_INVALID_PORT_ID.
 *          The mapping examples are shown below:
 *          If aos port id 1 is a sfp port and its corresponding onlp sfp port
 *          id is 0, then aos_uport_to_onlp_port[0] should be set as 0.
 *          If aos port id 2 is not a sfp port, then aos_uport_to_onlp_port[1]
 *          should be set as ONLP_SFP_INVALID_PORT_ID.
 *          If the maximum aos port id is 48, then all of the array elements
 *          after(including) aos_uport_to_onlp_port[48] should be set as
 *          ONLP_SFP_INVALID_PORT_ID when there is any unused elements in the
 *          tail of the array.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetONLPSFPPortMapping(I16_T aos_uport_to_onlp_port[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_GetHwPortModeInfo
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode info
 * INPUT   : board_id
 *           port
 *           hw_port_mode
 * OUTPUT  : hw_port_mode_info_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetHwPortModeInfo(
    UI32_T board_id,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode,
    STKTPLG_BOARD_HwPortModeInfo_T *hw_port_mode_info_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_SetHwPortModeInfo
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode info
 * INPUT   : board_id
 *           port
 *           hw_port_mode
 *           hw_port_mode_info_p
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_SetHwPortModeInfo(
    UI32_T board_id,
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode,
    STKTPLG_BOARD_HwPortModeInfo_T *hw_port_mode_info_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_SetHwPortModeStatus
 * -------------------------------------------------------------------------
 * PURPOSE : To set oper HW port mode
 * INPUT   : port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : For DEV_SWDRV_ChipInit only
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_SetHwPortModeStatus(
    UI32_T port,
    STKTPLG_TYPE_HwPortMode_T hw_port_mode);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_GetAllHwPortModeStatus
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode oper status
 * INPUT   : None
 * OUTPUT  : oper_hw_port_mode_ar
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_GetAllHwPortModeStatus(UI8_T oper_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_BOARD_CheckHwPortModeConflict
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode oper status
 * INPUT   : cfg_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE (TRUE for no conflict)
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_BOARD_CheckHwPortModeConflict(
    UI32_T board_id,
    UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

#endif /* STKTPLG_BOARD_H */

