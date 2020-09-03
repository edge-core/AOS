/* MODULE NAME:  syncedrv.h
 * PURPOSE:
 *  This module provides APIs for operations that is related to synchronous
 *  ethernet.
 * NOTES:
 *   This module does not support stacking.
 * HISTORY
 *    12/15/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
#ifndef SYNCEDRV_H
#define SYNCEDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "syncedrv_type.h"
#include "syncedrv_om.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* enum debug message level
 */
enum {
    SYNCEDRV_DEBUG_MSG_LEVEL_NO_MSG = 0,
    SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,
    SYNCEDRV_DEBUG_MSG_LEVEL_IMPORTANT_MSG,
    SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG
};

/* MACRO FUNCTION DECLARATIONS
 */
/* macro function to print debug message to console according to
 * msg_level.
 * debug message is shown only when shmem_data_p->show_watchdog_debug_msg_level
 * does not equal to SW_WATCHDOG_MGR_DEBUG_MSG_LEVEL_NO_MSG and msg_level
 * is less than or equal to shmem_data_p->show_watchdog_debug_msg_level.
 */
#define SYNCEDRV_SHOW_DEBUG_MSG(msg_level, fmtstr, ...) do { \
    if(SYNCEDRV_OM_GetDebugMsgLevel()!=SYNCEDRV_DEBUG_MSG_LEVEL_NO_MSG && \
        msg_level<=SYNCEDRV_OM_GetDebugMsgLevel()) \
        {BACKDOOR_MGR_Printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYNCEDRV_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYNCEDRV_AttachSystemResources(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYNCEDRV_Create_InterCSC_Relation(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set SYNCEDRV to transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_SetTransitionMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initialize data variables for transition mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterTransitionMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterMasterMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for master mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterMasterMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnterSlaverMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for slave mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_EnterSlaverMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_ProvisionComplete
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for provision complete operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_ProvisionComplete(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_EnableSyncE
 *-----------------------------------------------------------------
 * FUNCTION: This function will enable the functionality of
 *           synchronous ethernet on the specified ports.
 * INPUT   : pbmp  -  port bitmap to specify which ports to enable synce
 *                    Callers shall use the macro in l_pbmp.h to generate the
 *                    port bitmap.
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK             - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG   - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL  - an error occurs while enabling synce
 * NOTE    : 1. Only the ports that support synce are allowed to enable.
 *           2. No port will be set as enabled if the pbmp contains ports that
 *              do not support synce.
 *           3. SyncE enabled status will be kept unchanged if the port that
 *              supports synce is not included in pbmp.
 *           4. The default role for a synce-enabled port is clock-distributed port
 *              Callers need to call SYNCEDRV_SetSyncEClockSource() to change its
 *              role as clock-sourced port
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_EnableSyncE(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DisableSyncE
 *-----------------------------------------------------------------
 * FUNCTION: This function will disable the functionality of
 *           synchronous ethernet on the specified ports.
 * INPUT   : pbmp  -  port bitmap to specify which ports to disable synce
 *                    Callers shall use the macro in l_pbmp.h to generate the
 *                    port bitmap.
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK             - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG   - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL  - an error occurs while enabling synce
 * NOTE    : 1. Only the ports that support synce are allowed to disable.
 *           2. No port will be set as disabled if the pbmp contains ports that
 *              do not support synce.
 *           3. SyncE disabled status will be kept unchanged if the port that
 *              supports synce is not included in pbmp.
 *           4. If the port to disable synce is a clock source port, it will be
 *              removed from the clock source implicitly.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_DisableSyncE(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_UpdateSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synchronous ethernet clock source
 *           to the specified ports.
 * INPUT   : clock_src_lst     - an array which contains the clock source info
 *               ->port        - user port id of the clock source
 *               ->priority    - the prority value of the clock source, smaller
 *                               value gets higher priority
 *           clock_src_lst_len - number of element in clock_src_lst
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK                  - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG             - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL            - an error occurs while setting synce clock
 *                                                   source
 *           SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE - error due to out of resource
 * NOTE    : 1. Only the ports that have enabled synce are allowed to be set as
 *              clock source.
 *           2. No port will be set as clock source if the return value of this
 *              function is not SYNCEDRV_TYPE_RET_OK.
 *           3. The clock source info will be updated if the clock source had been
 *              set in the previous function call.
 *           4. The clock source set in the previous call will not be changed if
 *              they are not included in this function call.
 *           5. If the number of clock source to be set is more than
 *              SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, SYNCEDRV_TYPE_RET_ERR_OUT_OF_RESOURCE
 *              will be returned.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_UpdateSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_RemoveSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will remove the synchronous ethernet clock source
 *           according to the specified ports.
 * INPUT   : clock_src_lst     - an array which contains the clock source info
 *               ->port        - user port id of the clock source
 *           clock_src_lst_len - number of element in clock_src_lst
 * OUTPUT  : None
 * RETURN  : SYNCEDRV_TYPE_RET_OK                  - successful
 *           SYNCEDRV_TYPE_RET_ERR_ARG             - invalid argument
 *           SYNCEDRV_TYPE_RET_ERR_FAIL            - an error occurs while removing
 *                                                   synce clock source
 * NOTE    : 1. Only the ports that are set as clock source are allowed to be
 *              removed from the clock source.
 *           2. No port will be removed from the clock source if the return value
 *              of this function is not SYNCEDRV_TYPE_RET_OK.
 *           3. If the number of clock source to be set is more than
 *              SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC, SYNCEDRV_TYPE_RET_ERR_ARG
 *              will be returned.
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_RemoveSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetClockSrcSelectMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will configure the clock source selection
 *           mode on chip
 * INPUT   : is_auto - TRUE : clock source selection is performed by chip
 *                            automatically.
 *                     FALSE: clock source selection is determined by the
 *                            register setting which is configured by software
 *           is_revertive  -  This argument takes effect only when is_auto==TRUE
 *                            TRUE : The chip always revert the clock source to
 *                                   the highest priority clock source when it is
 *                                   available
 *                            FALSE: The chip will not revert the clock source.
 *                                   It will keep using the current selected
 *                                   clock source as long as it is available.
 *           force_clock_src_port - This argument takes effect only when is_auto==FALSE
 *                                  The specified clock source port
 *                                  will be selected as active clock source.
 *                                  If force_clock_src_port is 0, that means local
 *                                  clock will be used as synce clock source.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_SetClockSrcSelectMode(BOOL_T is_auto, BOOL_T is_revertive, UI32_T force_clock_src_port);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_GetSyncEClockSourceStatus
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the clock source status
 * INPUT   : None
 * OUTPUT  : clock_src_lst   - the clock source status in order of priority
 *                             (Highest priority in the first entry)
 *           
 *           clock_src_num_p - number of element in clock_src_lst
 *           is_locked_p     - TRUE: the selected clock source input is in locked state
 * RETURN  : SYNCEDRV_TYPE_RET_ERR_ARG  - error due to incorrect arguments
 *           SYNCEDRV_TYPE_RET_ERR_FAIL - error due to chip error
 *           SYNCEDRV_TYPE_RET_OK       - successfully
 * NOTE    : None
 *----------------------------------------------------------------*/
SYNCEDRV_TYPE_RET_T SYNCEDRV_GetSyncEClockSourceStatus(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T* clock_src_num_p, BOOL_T *is_locked_p);

/*-----------------------------------------------------------------------------
 * NAME    : SYNCEDRV_GetChipOperatingMode
 *------------------------------------------------------------------------------
 * PURPOSE : Get SyncE chip operating mode.
 * INPUT   : None
 * OUTPUT  : mode_p - SyncE chip operating mode. (SYNCEDRV_TYPE_CHIP_OPERATING_MODE_E)
 * RETURN  : SYNCEDRV_TYPE_RET_OK       - Success
 *           SYNCEDRV_TYPE_RET_ERR_FAIL - Failed
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
SYNCEDRV_TYPE_RET_T SYNCEDRV_GetChipOperatingMode(UI32_T* mode_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_DumpSyncEAsicInfo
 *-----------------------------------------------------------------
 * FUNCTION: This function will dump the syncE ASIC related info.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function is for debug only.
 *----------------------------------------------------------------*/
void SYNCEDRV_DumpSyncEAsicInfo(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synchronous ethernet ASIC
 *           mode according to the given argument.
 * INPUT   : mode  - The chip mode to be set to the ASIC and board.
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : 1. All synchronous ethernet ASIC must support
 *              SYNCEDRV_CHIP_MODE_TYPE_NORMAL.
 *           2. Not all synchronous ethernet ASIC support
 *              SYNCEDRV_CHIP_MODE_TYPE_DCO
 *           3. The clock source select mode will be restored to
 *              default setting (using local clock source, i.e. free-run mode)
 *              when chip mode is changed from other mode to NORMAL mode.
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_SetChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_SetOutputFreqOffsetInPPM
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a frequency offset in PPM for
 *           output clock with respect to master clock of synchronous
 *           ethernet chip.
 * INPUT   : ppm  - the frequency offset in PPM
 * OUTPUT  : None
 * RETURN  : TRUE  - Successfully
 *           FALSE - Failed
 * NOTE    : 1. This function can only be called after setting
 *              chip mode as SYNCEDRV_CHIP_MODE_TYPE_DCO. If call
 *              this function in chip mode other than SYNCEDRV_CHIP_MODE_TYPE_DCO
 *              , this function returns false.
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_SetOutputFreqOffsetInPPM(float ppm);

/* FUNCTION NAME: SYNCEDRV_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYNCEDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYNCEDRV_CreateTask(void);

#endif    /* End of SYNCEDRV_H */

