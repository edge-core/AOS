/* MODULE NAME:  syncedrv_om.c
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
 
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_cpnt.h"
#if(SYS_CPNT_SYNCE == TRUE)
#include "sys_bld.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "syncedrv_type.h"
#include "syncedrv_om.h"
#include "syncedrv.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define SYNCEDRV_OM_DEBUG

#define SYNCEDRV_OM_SHMEM_TOTAL_SIZE (sizeof(SYNCEDRV_OM_Shmem_Data_T) + SYNCEDRV_OM_CLOCK_SRC_LIST_WK_BUF_SZ)

/* MACRO FUNCTION DECLARATIONS
 */
#define SYNCEDRV_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(syncedrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYNCEDRV_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(syncedrv_om_sem_id)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T syncedrv_om_sem_id;
static SYNCEDRV_OM_Shmem_Data_T *shmem_data_p;

/* EXPORTED SUBPROGRAM BODIES
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
void SYNCEDRV_OM_InitiateSystemResources(void)
{
    void* working_buffer;

    shmem_data_p = (SYNCEDRV_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYNCEDRV_SHMEM_SEGID);
    memset(shmem_data_p, 0, SYNCEDRV_OM_SHMEM_TOTAL_SIZE);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    working_buffer = (void*)(shmem_data_p+1);
    L_SORT_LST_ShMem_Create(
        &shmem_data_p->clock_src_lst,
        working_buffer,
        SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_NUM,
        SYNCEDRV_OM_CLOCK_SRC_LIST_ELEM_SZ,
        L_SORT_LST_SHMEM_COMPARE_FUNC_ID_SYNCEDRV_CLOCK_SOURCE);
	shmem_data_p->chip_mode = SYNCEDRV_CHIP_MODE_TYPE_NORMAL;
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYNCEDRV_OM, &syncedrv_om_sem_id);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYNCEDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYNCEDRV_OM_AttachSystemResources(void)
{
    shmem_data_p = (SYNCEDRV_OM_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYNCEDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYNCEDRV_OM, &syncedrv_om_sem_id);
    L_SORT_LST_ShMem_SetCompareFunc(
        L_SORT_LST_SHMEM_COMPARE_FUNC_ID_SYNCEDRV_CLOCK_SOURCE,
        SYNCEDRV_OM_CompareClockSource);
}

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
void SYNCEDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SYNCEDRV_SHMEM_SEGID;
    *seglen_p = SYNCEDRV_OM_SHMEM_TOTAL_SIZE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function will set SYNCEDRV to transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetTransitionMode(void)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterTransitionMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initialize data variables for transition mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterTransitionMode(void)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterMasterMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for master mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterMasterMode(void)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_EnterSlaverMode
 *-----------------------------------------------------------------
 * FUNCTION: This function initial data variables for slave mode operation
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_EnterSlaveMode(void)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetProvisionComplete
 *-----------------------------------------------------------------
 * FUNCTION: This function set provision complete status to OM
 * INPUT   : provision_complete
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetProvisionComplete(BOOL_T provision_complete)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->provision_complete = provision_complete;
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
void SYNCEDRV_OM_GetSyncEEnabledPbmp(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if(pbmp==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid argument\r\n", __FUNCTION__, __LINE__);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(pbmp, shmem_data_p->synce_enabled_pbmp, sizeof(UI8_T)*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}


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
void SYNCEDRV_OM_SetSyncEEnabledPbmp(UI8_T pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if(pbmp==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid argument\r\n", __FUNCTION__, __LINE__);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(shmem_data_p->synce_enabled_pbmp, pbmp , sizeof(UI8_T)*SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
void SYNCEDRV_OM_GetSyncEClockSourceStatus(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T* clock_src_num_p)
{
    SYNCEDRV_TYPE_ClockSource_T entry;

    if ((clock_src_lst==NULL) || (clock_src_num_p==NULL))
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid argument\r\n", __FUNCTION__, __LINE__);
        return;
    }

    *clock_src_num_p=0;

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    if(L_SORT_LST_ShMem_Get_1st(&(shmem_data_p->clock_src_lst), &(clock_src_lst[0]))==TRUE)
    {
        *clock_src_num_p=1;
        memcpy(&entry, &(clock_src_lst[0]), sizeof(SYNCEDRV_TYPE_ClockSource_T));
        while(L_SORT_LST_ShMem_Get_Next(&(shmem_data_p->clock_src_lst), &entry)==TRUE)
        {
            memcpy(&(clock_src_lst[*clock_src_num_p]), &entry, sizeof(SYNCEDRV_TYPE_ClockSource_T));
            (*clock_src_num_p)+=1;
        }
    }
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
BOOL_T SYNCEDRV_OM_GetSyncEClockSourceStatusByPortandPriority(SYNCEDRV_TYPE_ClockSource_T* clock_src_p)
{
    BOOL_T ret;

    if(clock_src_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid argument\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    ret=L_SORT_LST_ShMem_Get(&(shmem_data_p->clock_src_lst),clock_src_p);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

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
void SYNCEDRV_OM_UpdateSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC], UI32_T clock_src_num)
{
    UI32_T i;
    SYNCEDRV_TYPE_ClockSource_T entry;

    if(clock_src_num>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        BACKDOOR_MGR_Printf("%s(%d)Illegal clock_src_num %lu\r\n", __FUNCTION__, __LINE__, clock_src_num);
        return;
    }

    if(clock_src_lst==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid clock_src_lst\r\n", __FUNCTION__, __LINE__);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    for(i=0; i<clock_src_num; i++)
    {
        /* Check whether the clock source port already exists
         * Remove the entry if it already exists
         */
        entry.port=0;
        entry.priority=0;
        while(L_SORT_LST_ShMem_Get_Next(&(shmem_data_p->clock_src_lst), &entry)==TRUE)
        {
            if(entry.port==clock_src_lst[i].port)
            {
                SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Found existing entry(port=%lu, priority=%lu)\r\n", entry.port, entry.priority);
                if(L_SORT_LST_ShMem_Delete(&(shmem_data_p->clock_src_lst), &entry)==FALSE)
                {
                    SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_CRITICAL_MSG,"Fail to delete the entry of index %lu(port %lu)\r\n", i, entry.port);
                }
                break;
            }
        }

        /* Set the entry to the list
         */
        entry=clock_src_lst[i];
        if(L_SORT_LST_ShMem_Set(&(shmem_data_p->clock_src_lst), &entry)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Fail to set the entry of index %lu\r\n", __FUNCTION__, __LINE__, i);
        }
        else
        {
            SYNCEDRV_SHOW_DEBUG_MSG(SYNCEDRV_DEBUG_MSG_LEVEL_VERBOSE_MSG,"Add clock src entry ok(port=%lu, priority=%lu)\r\n", entry.port, entry.priority);
        }

    }
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
void SYNCEDRV_OM_RemoveSyncEClockSource(SYNCEDRV_TYPE_ClockSource_T clock_src_lst[], UI32_T clock_src_lst_len)
{
    UI32_T i;

    if(clock_src_lst==NULL || clock_src_lst_len>SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        BACKDOOR_MGR_Printf("%s(%d)Invalid argument.(clock_src_lst_len=%lu)\r\n", __FUNCTION__, __LINE__, clock_src_lst_len);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    for(i=0; i<clock_src_lst_len; i++)
    {
        if(L_SORT_LST_ShMem_Delete(&(shmem_data_p->clock_src_lst), &(clock_src_lst[i]))==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to remove clock source(port=%lu, priority=%lu)\r\n",
                __FUNCTION__, __LINE__, clock_src_lst[i].port, clock_src_lst[i].priority);
        }
    }
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_RemoveAllSyncEClockSource
 *-----------------------------------------------------------------
 * FUNCTION: This function will remove all of the the synce clock source
 * INPUT   : None
 * RETURN  : None
 * NOTE    : None
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_RemoveAllSyncEClockSource(void)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    if(L_SORT_LST_ShMem_Delete_All(&(shmem_data_p->clock_src_lst))==FALSE)
        printf("%s(%d)Failed to delete all of synce clock source\r\n",
            __FUNCTION__, __LINE__);
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding
 *-----------------------------------------------------------------
 * FUNCTION: This function will get the binding of the given reference
 *           index to clock source port to SYNCEDRV OM.
 * INPUT   : ref_idx        -  input clock reference index
 * RETURN  : clock_src_port -  the user port id of the clock source port
 * NOTE    : No port is binded to given ref_idx if clock_src_port is 0
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_GetRefIdxToClockSrcPortBinding(UI32_T ref_idx, UI32_T *clock_src_port_p)
{
    if(clock_src_port_p==NULL)
    {
        printf("%s(%d):clock_src_port_p is NULL\r\n", __FUNCTION__, __LINE__);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    *clock_src_port_p = shmem_data_p->ref_idx_to_clock_src_port[ref_idx];
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
void SYNCEDRV_OM_SetRefIdxToClockSrcPortBinding(UI32_T ref_idx, UI32_T clock_src_port)
{
    if(ref_idx>=SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC)
    {
        printf("%s(%d):Invalid ref_idx=%lu\r\n", __FUNCTION__, __LINE__, ref_idx);
        return;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->ref_idx_to_clock_src_port[ref_idx]=clock_src_port;
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

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
BOOL_T SYNCEDRV_OM_GetRefIdxByClockSrcPort(UI32_T clock_src_port, UI32_T *ref_idx_p)
{
    BOOL_T ret=FALSE;
    UI8_T  i;

    if(ref_idx_p==NULL)
    {
        printf("%s(%d):ref_idx_p is NULL\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
    for(i=0; i<SYS_HWCFG_SYNCE_MAX_NBR_OF_CLOCK_SRC; i++)
    {
        if(shmem_data_p->ref_idx_to_clock_src_port[i]==clock_src_port)
        {
            *ref_idx_p=i;
            ret=TRUE;
            break;
        }
    }
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

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
int SYNCEDRV_OM_CompareClockSource(void *inlist_element, void *input_element)
{
    SYNCEDRV_TYPE_ClockSource_T *inlist_p, *input_p;

    inlist_p = (SYNCEDRV_TYPE_ClockSource_T*) inlist_element;
    input_p  = (SYNCEDRV_TYPE_ClockSource_T*) input_element;

    /* compare priority value (smaller value get higher priority)
     * L_SORT_LST is sorted ascendingly, in order to let the
     * precedence sorted from the highest to the lowest,
     * need to reverse the comparision
     */
    if (inlist_p->priority < input_p->priority)
    {
        return -1;
    }
    else if (inlist_p->priority == input_p->priority)
    {
        return 0;
    }
    else
    {
        return 1;
    }


    /* compare port value (smaller value get higher priority)
     */
    if (inlist_p->port < input_p->port)
    {
        return -1;
    }
    else if(inlist_p->port == input_p->port)
    {
        return 0;
    }

    return 1;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_SetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: Set the current chip mode to OM
 * INPUT   : mode -- current chip mode
 * OUTPUT  : none
 * RETURN  : none
 * NOTE    : none
 *----------------------------------------------------------------*/
void SYNCEDRV_OM_SetChipMode(SYNCEDRV_CHIP_MODE_TYPE_T mode)
{
    SYNCEDRV_OM_ENTER_CRITICAL_SECTION();
	shmem_data_p->chip_mode = mode;
    SYNCEDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - SYNCEDRV_OM_GetChipMode
 *-----------------------------------------------------------------
 * FUNCTION: Get the current chip mode from OM
 * INPUT   : none
 * OUTPUT  : none
 * RETURN  : current chip mode
 * NOTE    : none
 *----------------------------------------------------------------*/
SYNCEDRV_CHIP_MODE_TYPE_T SYNCEDRV_OM_GetChipMode(void)
{
	return shmem_data_p->chip_mode;
}

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
void SYNCEDRV_OM_SetDebugMsgLevel(UI8_T debug_msg_level)
{
    shmem_data_p->debug_msg_level=debug_msg_level;
}

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
UI8_T SYNCEDRV_OM_GetDebugMsgLevel(void)
{
    return shmem_data_p->debug_msg_level;
}

/* LOCAL SUBPROGRAM BODIES
 */
#endif /*SYS_CPNT_SYNCE == TRUE*/

