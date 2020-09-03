/* MODULE NAME:  syncedrv_om.h
 * PURPOSE:
 *   This module provides APIs for accessing syncedrv OM
 * 
 * NOTES:
 *
 * HISTORY
 *    12/16/2011 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Networks, 2011
 */
#ifndef SYNCEDRV_OM_H
#define SYNCEDRV_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_hwcfg.h"
#include "l_sort_lst.h"
#include "syncedrv_type.h"
#include "sysrsc_mgr.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_NUM SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC
#define SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_SZ  sizeof(SYNCEDRV_TYPE_ClockSource_T)
#define SYNCEDRV_OM_CLOCK_SRC_LIST_WK_BUF_SZ L_SORT_LST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_NUM,SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_SZ)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* syncedrv om shared memory layout
 * +-----------------------------------------+
 * | SYNCEDRV_OM_Shmem_Data_T                |
 * +-----------------------------------------+
 * | working buffer of sorted clock src list |
 * +-----------------------------------------+
 */
typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T provision_complete;
    /* port that enable synce will have bit value 1 on the corresponding bit of
     * the port bit map
     */
    UI8_T synce_enabled_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    /* current clock source settings will be kept in this sorted list
     * which is sorted in priority order, the highest priority clock source port
     * entry is put in the first entry of the list.
     */
    L_SORT_LST_ShMem_List_T clock_src_lst;
	/* keep current chip mode
	 */
	SYNCEDRV_CHIP_MODE_TYPE_T chip_mode;
    /* this array keeps the binding of logical reference input clock index(ref_idx)
     * and the clock source port, the ref_idx is not binding to a clock source port
     * if the value of the element is 0
     */
    UI8_T  ref_idx_to_clock_src_port[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC];
    /* debug message level
     */
    UI8_T  debug_msg_level;
} SYNCEDRV_OM_Shmem_Data_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*---------------------------------------------------------------------------------
 * FUNCTION : SYNCEDRV_OM_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void SYNCEDRV_OM_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYNCEDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYNCEDRV_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYNCEDRV_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function get the size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void SYNCEDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set SYNCEDRV to transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetTransitionMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initialize data variables for transition mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterTransitionMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterMasterMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for master mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterMasterMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterSlaverMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for slave mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterSlaveMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetProvisionComplete
 *-----------------------------------------------------------------
 * FUNCTION: This function set provision complete status to OM
 * INPUT   : provision_complete
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetProvisionComplete(BOOL_T provision_complete);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetSyncEEnabledPbmp
 *-----------------------------------------------------------------
 * FUNCTION: This function will output the synce enabled status of
 *           all of the port.
 * INPUT   : pbmp  -  port bitmap to indicate which port enable synce
 *                    If the corresponding bit of the port is 1, it means
 *                    synce enabled on the port.
 *                    Callers shall use the macro in l_pbmp.h to interpret.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_GetSyncEEnabledPbmp(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetSyncEEnabledPbmp
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the synce enabled status of
 *           all of the port.
 * INPUT   : pbmp  -  port bitmap to indicate which port enable synce
 *                    If the corresponding bit of the port is 1, it means
 *                    synce enabled on the port.
 *                    Callers shall use the macro in l_pbmp.h to generate.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function shall be called by SYNCEDRV only.
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetSyncEEnabledPbmp(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetSyncEClockSourceStatus
 *-----------------------------------------------------------------
 * FUNCTION: This function will output the status of the synce clock
 *           source.
 * INPUT   : None
 * OUTPUT  : clock_src_lst         - an array which contains the clock source
 *                                   info
 *           clock_src_num_p       - number of element in clock_src_lst
 * RETURN  : None
 * NOTE    : The output of clock_src_lst is sorted from highest priority
 *           to the lowest priority.
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_GetSyncEClockSourceStatus(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T* clock_src_num_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetSyncEClockSourceStatusByPortandPriority
 *-----------------------------------------------------------------
 * FUNCTION: This function will output the status of the synce clock
 *           source by the specified port and priority.
 * INPUT   : clock_src_p->port     - user port id of the clock source
 *           clock_src_p->priority - the prority value of the clock source, smaller
 *                                   value gets higher priority
 * OUTPUT  : clock_src_p           - clock source info
 * RETURN  : TRUE  - a clock source status is found
 *           FALSE - not found the clock source status on the speceified port and priority
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_OM_GetSyncEClockSourceStatusByPortandPriority(SYNCEDRV_TYPE_ClockSource_T* clock_src_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_UpdateSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will update the synce clock source
 * INPUT   : clock_src_lst         - an array which contains the clock source
 *                                   info
 *               ->port            - user port id of the clock source
 *               ->priority        - the prority value of the clock source, smaller
 *                                   value gets higher priority
 *               ->lock_status     - TRUE:clock source locked, FALSE: clock source not locked
 *               ->is_active       - TRUE:active, FALSE: not active
 *           clock_src_num         - number of element in clock_src_lst
 * RETURN  : None.
 * NOTE    : Caller must ensure that the resulting number of entry
 *           is not larger than the upper limit.
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_UpdateSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T clock_src_num);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_RemoveSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will remove the synce clock source
 * INPUT   : clock_src_lst         - an array which contains the clock source
 *                                   info to be removed
 *               ->port            - user port id of the clock source
 *               ->priority        - the prority value of the clock source, smaller
 *                                   value gets higher priority
 *           clock_src_lst_len     - number of element in clock_src_lst
 * RETURN  : None.
 * NOTE    : 1. clock_src_lst_len must not be greater than
 *              SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC
 *           2. Need to fill exactly the correct port and priority to remove
 *              sucessfully
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_RemoveSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_RemoveAllSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will remove all of the the synce clock source
 * INPUT   : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_RemoveAllSyncEClockSource(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the binding of the given reference
 *           index to clock source port to SYNCEDRV OM.
 * INPUT   : ref_idx        -  input clock reference index
 * RETURN  : clock_src_port -  the user port id of the clock source port
 * NOTE    : No port is binded to given ref_idx if clock_src_port is 0
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding(UI32_T ref_idx, UI32_T *clock_src_port_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetRefIdxToClockSrcPortBinding
 *-----------------------------------------------------------------
 * FUNCTION: This function will set the binding of the given reference
 *           index to clock source port to SYNCEDRV OM.
 * INPUT   : ref_idx        -  input clock reference index
 *           clock_src_port -  the user port id of the clock source port
 * RETURN  : None
 * NOTE    : No port is binded to given ref_idx if clock_src_port is 0
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetRefIdxToClockSrcPortBinding(UI32_T ref_idx, UI32_T clock_src_port);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetRefIdxByClockSrcPort
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the binded ref idx of the given
 *           clock source port if the binding exists.
 * INPUT   : clock_src_port -  the user port id of the clock source port
 * OUTPUT  : ref_idx_p      - the binded logical input reference index if the binding
 *                            exists
 * RETURN  : TRUE  - The binded logical input reference index of the given clock
 *                   source port is found
 *           FALSE - No binded logical input reference index of the given clock
 *                   source port is found
 * NOTE    : None
 *----------------------------------------------------------------*/
BOOL_T SYNCEDRV_OM_GetRefIdxByClockSrcPort(UI32_T clock_src_port, UI32_T *ref_idx_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_CompareClockSource
 *-----------------------------------------------------------------
 * FUNCTION: Compare function for clock source list.
 * INPUT   : inlist_element - element in the sorted list
 *           input_element  - input element
 * OUTPUT  : None.
 * RETURN  : -1  - inlist_element is smaller than input_element
 *            0  - inlist_element equals to input_element
 *            1  - inlist_element is larger than input_element
 * NOTE    : Priority precedence: priority > port (smaller value get higher priority)
 *----------------------------------------------------------------*/
int SYNCEDRV_OM_CompareClockSource(void *inlist_element, void *input_element);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: Set the current chip mode to OM
 * INPUT   : mode -- current chip mode
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : none
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: Get the current chip mode from OM
 * INPUT   : none
 * OUTPUT  : none
 * RETURN  : current chip mode
 * NOTE    : none
 *----------------------------------------------------------------*/
SYNCEDRV_CHIP_MODE_TYPE_T SYNCEDRV_OM_GetChipMode(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetDebugMsgLevel
 *-----------------------------------------------------------------
 * FUNCTION: Set the status of syncedrv debug message level
 * INPUT   : debug_msg_level -- SYNCEDRV_DEBUG_MSG_LEVEL_XXX
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : When debug_msg_level == SYNCEDRV_DEBUG_MSG_LEVEL_NO_MSG
 *           no debug message will be output to console.
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetDebugMsgLevel(UI8_T debug_msg_level);

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetDebugMsgLevel
 *-----------------------------------------------------------------
 * FUNCTION: Get the status of syncedrv debug message level
 * INPUT   : none
 * OUTPUT  : none
 * RETURN  : debug message level
 * NOTE    : When debug_msg_level == SYNCEDRV_OM_DEBUG_MSG_LEVEL_NO_MSG
 *           no debug message will be output to console.
 *----------------------------------------------------------------*/
UI8_T SYNCEDRV_OM_GetDebugMsgLevel(void);

#endif    /* End of SYNCEDRV_OM_H */

