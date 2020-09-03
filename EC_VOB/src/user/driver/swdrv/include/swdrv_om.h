#ifndef _SWDRV_OM_H_
#define _SWDRV_OM_H_

/* INCLUDE FILE DECLARATIONS
 */
#include "swdrv_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum {
    SWDRV_OM_F_ANNOUNCED,
    SWDRV_OM_F_REALTIME,
    SWDRV_OM_F_ALL,
} SWDRV_OM_FilterFlag_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void SWDRV_OM_Reset() ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_AttachSystemResources(void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterMasterMode (void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterSlaveMode (void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterSlaveMode (void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_SetTransitionMode (void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterTransitionMode (void) ;

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T SWDRV_OM_GetOperatingMode();

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_SetTaskTransitionDone
 *---------------------------------------------------------------------------------
 * PURPOSE: Set task transition done flag.
 * INPUT:   is_transition_done
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_SetTaskTransitionDone(BOOL_T is_transition_done);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_WaitTaskTransitionDone
 *---------------------------------------------------------------------------------
 * PURPOSE: To wait until task receives ENTER TRANSITION MODE.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_WaitTaskTransitionDone(void);

void SWDRV_OM_Init() ;

void SWDRV_OM_SetDebugFlag(UI32_T debug_flag, BOOL_T enable);

BOOL_T SWDRV_OM_GetDebugFlag(UI32_T debug_flag);

BOOL_T SWDRV_OM_GetChipTaskStatus(BOOL_T *status) ;

BOOL_T SWDRV_OM_SetChipTaskStatus(BOOL_T status) ;

BOOL_T SWDRV_OM_SetTrunkMode(UI32_T mode) ;

BOOL_T SWDRV_OM_SetUnitBitMap(UI32_T bitmap) ;

BOOL_T SWDRV_OM_SetThreadId(UI32_T thread_id) ;

BOOL_T SWDRV_OM_SetSfpThreadId(UI32_T thread_id) ;

BOOL_T SWDRV_OM_SetSystemInfoBasePortId(UI16_T base_port_id) ;

BOOL_T SWDRV_OM_SetSystemInfoPortNum(UI16_T port_num) ;

BOOL_T SWDRV_OM_SetPortInfoPortType(UI32_T port, UI32_T port_type) ;

BOOL_T SWDRV_OM_SetPortInfoExisting(UI32_T port, BOOL_T exist) ;

BOOL_T SWDRV_OM_GetSystemInfoStackId(UI32_T *stack_id) ;

BOOL_T SWDRV_OM_SetSystemInfoStackUnit(UI32_T unit_idx, UI32_T stack_id) ;

BOOL_T SWDRV_OM_GetSystemInfo(SWDRV_Switch_Info_T *swdrv_system_info) ;

BOOL_T SWDRV_OM_ResetSystemInfoStackUnitTable() ;

BOOL_T SWDRV_OM_SetSystemInfoStackId(UI32_T stack_id) ;

BOOL_T SWDRV_OM_SetComboForceMode(UI32_T port, UI32_T mode) ;

BOOL_T SWDRV_OM_GetComboForceSpeed(UI32_T port, UI32_T *fiber_speed_p);

BOOL_T SWDRV_OM_SetComboForceSpeed(UI32_T port, UI32_T fiber_speed);

BOOL_T SWDRV_OM_GetPortInfo(UI32_T port, SWDRV_Port_Info_T *swdrv_port_info) ;

BOOL_T SWDRV_OM_GetCraftPortInfo(SWDRV_CraftPort_Info_T *swdrv_craftport_info) ;

BOOL_T SWDRV_OM_SetCraftPortInfoLinkStatus(BOOL_T status);

BOOL_T SWDRV_OM_SetSystemInfoStackingPort(UI32_T port) ;

BOOL_T SWDRV_OM_SetPortInfoLinkStatus(UI32_T port, BOOL_T status) ;

BOOL_T SWDRV_OM_SetPortInfoSpeedDuplexOper(UI32_T port, UI8_T status) ;

BOOL_T SWDRV_OM_SetPortInfoFlowControlOper(UI32_T port, UI8_T status) ;

BOOL_T SWDRV_OM_SetPortInfoIsAutoNeg(UI32_T port, BOOL_T status) ;

BOOL_T SWDRV_OM_GetPortSfpPresent(UI32_T sfp_index, BOOL_T *status_p);

BOOL_T SWDRV_OM_SetPortSfpPresent(UI32_T sfp_index, BOOL_T status) ;

BOOL_T SWDRV_OM_GetPortSfpInfoValid(UI32_T sfp_index, BOOL_T *valid_p);

BOOL_T SWDRV_OM_SetPortSfpInfoValid(UI32_T sfp_index, BOOL_T valid);

BOOL_T SWDRV_OM_SetPortInfoSpeedDuplexCfg(UI32_T port, UI8_T status) ;

BOOL_T SWDRV_OM_SetPortInfoForced1000tMode(UI32_T port, UI8_T status) ;

BOOL_T SWDRV_OM_SetPortInfoModuleId(UI32_T port, UI8_T module_id);

BOOL_T SWDRV_OM_GetTrunkMode(UI32_T *mode) ;

BOOL_T SWDRV_OM_SetTrunkInfoBcmTrunkId(UI32_T trk_idx) ;

BOOL_T SWDRV_OM_SetTrunkInfoUsed(UI32_T trk_idx, UI8_T status) ;

BOOL_T SWDRV_OM_SetSystemInfoNumOfUnit(UI32_T num_of_units) ;

BOOL_T SWDRV_OM_SetProvisionComplete(UI32_T status) ;

BOOL_T SWDRV_OM_GetProvisionComplete(BOOL_T *status) ;

BOOL_T SWDRV_OM_SetPortLoopbackList(UI8_T loop_back_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]) ;

BOOL_T SWDRV_OM_GetTrunkInfo(UI32_T trunk_id, SWDRV_Trunk_Info_T *swdrv_trunk_info) ;

BOOL_T SWDRV_OM_SetThreadIdle(BOOL_T status) ;

BOOL_T SWDRV_OM_GetPortLinkStatusBitmaps(UI32_T unit, SWDRV_LinkStatus_T *link_status) ;

BOOL_T SWDRV_OM_SetPortLinkStatusBitmaps(UI32_T unit, SWDRV_LinkStatus_T *link_status, SWDRV_OM_FilterFlag_T flag) ;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpPresentStatusBitmaps
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp present status bitmaps
 *
 * INPUT   : unit
 * OUTPUT  : present_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpPresentStatusBitmaps(UI32_T unit, SWDRV_TYPE_SfpPresentStatus_T *present_status_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpPresentStatusBitmaps
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp present status bitmaps
 *
 * INPUT   : unit
 *           flag -- SWDRV_OM_F_ANNOUNCED: update previous_sfp_present_st_bitmap
 *                   SWDRV_OM_F_REALTIME : update sfp_present_st_bitmap
 *                   SWDRV_OM_F_ALL      : update both
 * OUTPUT  : present_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpPresentStatusBitmaps(UI32_T unit, SWDRV_TYPE_SfpPresentStatus_T *present_status_p, SWDRV_OM_FilterFlag_T flag);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp eeprom info
 *
 * INPUT   : sfp_index
 * OUTPUT  : sfp_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpInfo(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpInfo(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info
 *
 * INPUT   : sfp_index
 * OUTPUT  : sfp_ddm_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpDdmInfo(UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpDdmInfo(UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info raw data
 *
 * INPUT   : sfp_index
 * OUTPUT  : info_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpDdmInfoRaw(UI32_T sfp_index, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH]);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info raw data
 *
 * INPUT   : sfp_index
 *           info_ar
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpDdmInfoRaw(UI32_T sfp_index, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH]);

BOOL_T SWDRV_OM_SetTrunkInfoMemberUnit(UI32_T trk_idx, UI32_T member_idx, UI32_T unit) ;

BOOL_T SWDRV_OM_SetTrunkInfoMemberPort(UI32_T trk_idx, UI32_T member_idx, UI32_T port) ;

BOOL_T SWDRV_OM_SetTrunkInfoMemberNum(UI32_T trunk_id, UI32_T port_count) ;

BOOL_T SWDRV_OM_GetThreadId(UI32_T *swdrv_thread_id) ;

BOOL_T SWDRV_OM_GetSfpThreadId(UI32_T *thread_id);

BOOL_T SWDRV_OM_GetUnitBitmap(UI32_T *swdrv_unit_bitmap) ;

BOOL_T SWDRV_OM_GetComboForceMode(UI32_T port, UI32_T *combo_forced_mode) ;

BOOL_T SWDRV_OM_GetWorkAroundStauts(UI32_T port, UI32_T *workaround_status) ;

BOOL_T SWDRV_OM_SetWorkAroundStatus(UI32_T port, UI32_T workaround_status) ;

void SWDRV_OM_HotSwapRemove(UI32_T unit_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_UpdateLocalPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Update port admin status of the local unit.
 *
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           is_admin_enable -- TRUE if port admin is enabled.
 * OUTPUT  : None
 * RETURN  : TRUE if update succesfully.
 * NOTE    : When SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT is TRUE
 *           the PHY admin status might be overriden by SWDRV. So need to keep
 *           the PHY admin status set by upper layer.
 * -------------------------------------------------------------------------*/
 BOOL_T SWDRV_OM_UpdateLocalPortAdminStatus(UI32_T unit, UI32_T port, BOOL_T is_admin_enable);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetLocalPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get port admin status of the local unit.
 *
 * INPUT   : port -- which port
 *           admin_status_p -- port admin status set by upper layer
 *           status_changed_p -- TRUE if the port admin status had ever been
 *                               changed since last read on the specified port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT is TRUE
 *           the PHY admin status might be overriden by SWDRV.
 *           In SWDRV_MonitorPDPortPowerGoodStatus(), it will call this function
 *           to determine the real PHY admin status.
 * -------------------------------------------------------------------------*/
void SWDRV_OM_GetLocalPortAdminStatus(UI32_T port, BOOL_T *admin_status_p, BOOL_T* status_changed_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  -- PSE heck is enabled.
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_GetPSECheckStatus(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 *
 * INPUT   : pse_check_status  --  TRUE: enable, FALSE: disable
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
void SWDRV_OM_SetPSECheckStatus(BOOL_T pse_check_status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentStatusAddr
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present status address
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_status_addr_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentStatusAddr(UI32_T *sfp_present_status_addr_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentMask
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present mask
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_mask_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentMask(UI8_T *sfp_present_mask_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentBitShift
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present bit shift
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_bit_shift_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentBitShift(UI8_T *sfp_present_bit_shift_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosStatusAddr
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los status address
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_status_addr_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosStatusAddr(UI32_T *sfp_rx_los_status_addr_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosMask
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los mask
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_mask_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosMask(UI8_T *sfp_rx_los_mask_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosBitShift
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los bit shift
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_bit_shift_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosBitShift(UI8_T *sfp_rx_los_bit_shift_ar);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentDelay
 * -------------------------------------------------------------------------
 * FUNCTION: To get the sfp present delay value in SWDRV_OM
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The value of sfp Present Delay in ticks.
 * NOTE    : 1. This function is for debug purpose.
 * -------------------------------------------------------------------------
 */
UI32_T SWDRV_OM_GetSfpPresentDelay(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetSfpPresentDelay
 * -------------------------------------------------------------------------
 * FUNCTION: To set the sfp present delay value to SWDRV_OM
 * INPUT   : sfp_present_delay  -  the sfp present delay value(in ticks)
 *                                  to be set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1. This function is for debug purpose.
 * -------------------------------------------------------------------------
 */
void SWDRV_OM_SetSfpPresentDelay(UI32_T sfp_present_delay);

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetHashSelDevBlockIndex
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection list index and block index for the service
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 * OUTPUT  : list_index - the index of hash-selection list
 *           hw_block_index - the index of hash-selection block
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_GetHashSelDevBlockIndex(
    SWDRV_HashSelService_T service,
    UI8_T *list_index, 
    UI8_T *hw_block_index
);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection block
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : hw_block_index - the index of hash-selection block
 *           share          - TRUE: share with other services
 *                            FALSE: no services use the same block
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_BindHashSelForService(
    SWDRV_HashSelService_T service, 
    UI8_T list_index,
    UI8_T *hw_block_index,
    BOOL_T *share
);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection block
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_UnBindHashSelForService(
    SWDRV_HashSelService_T service, 
    UI8_T list_index
);
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

#endif /*#ifndef _SWDRV_OM_H_*/
