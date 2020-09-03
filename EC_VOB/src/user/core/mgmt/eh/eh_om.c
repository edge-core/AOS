/* MODULE NAME:  eh_om.c
 * PURPOSE:
 *     OM of EH(Error Handler).
 *
 * NOTES:
 *
 * HISTORY:
 *      Date        -- Modifier,        Reason
 *      2013.02.06  -- Charlie Chen     Create.
 * Copyright(C)      Edge-Core Networks, 2013
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "eh_om.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define EH_OM_EnterCriticalSection() SYSFUN_TakeSem(eh_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define EH_OM_LeaveCriticalSection() SYSFUN_GiveSem(eh_om_sem_id)

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* shared memory variables
 */
static EH_OM_ShmemData_T *eh_om_shmem_data_p;
static UI32_T eh_om_sem_id;

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/*---------------------------------------------------------------------------------
 * FUNCTION : void EH_OM_InitiateSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void EH_OM_InitiateSystemResources(void)
{
    int idx;

    eh_om_shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_EH_OM_SHMEM_SEGID);

    for (idx=0; idx<SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK; idx++)
    {
        /* need to init ui_msg_no as EH_TYPE_INVALID_UI_MSG_NO(=0xFFFFFFFFUL)
         * so memset as 0xFF here
         */
        memset(&(eh_om_shmem_data_p->eh_buf[idx]), 0xFF, sizeof(eh_om_shmem_data_p->eh_buf[idx]));
        eh_om_shmem_data_p->xml_msg_buf_p_ar[idx] = NULL;
    }
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: EH_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
void EH_OM_AttachSystemResources(void)
{
    eh_om_shmem_data_p = SYSRSC_MGR_GetShMem(SYSRSC_MGR_EH_OM_SHMEM_SEGID);
    
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_EH_OM, &eh_om_sem_id)
        != SYSFUN_OK)
    {
        printf("%s: get om sem id fail.\r\n", __FUNCTION__);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : This function gets the size for share memory
 * INPUT    : *segid_p
 *            *seglen_p
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void EH_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_EH_OM_SHMEM_SEGID;
    *seglen_p = sizeof(EH_OM_ShmemData_T);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_SetErrorMsgEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the given error message entry to the buffer of the
 *            specified eh buffer index.
 * INPUT    : eh_buf_idx  --  the index of the eh message buffer to be set.
 *            entry_p     --  the error message entry to be set.
 * OUTPUT   : None
 * RETURN   :
 *            EH_TYPE_RET_OK                -- Set entry to OM successfully.
 *            EH_TYPE_RET_INVALID_INPUT_ARG -- Invalid input argument.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
EH_TYPE_RET_T EH_OM_SetErrorMsgEntry(UI8_T eh_buf_idx, EH_OM_ErrorMsgEntry_T *entry_p)
{
    /* sanity check
     */
    if (eh_buf_idx > SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printf("%s:Invalid eh_buf_idx %hu\r\n", __FUNCTION__, eh_buf_idx);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    if (entry_p==NULL)
    {
        printf("%s:entry_p is NULL\r\n", __FUNCTION__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    EH_OM_EnterCriticalSection();
    eh_om_shmem_data_p->eh_buf[eh_buf_idx] = *entry_p;
    EH_OM_LeaveCriticalSection();
    return EH_TYPE_RET_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_GetErrorMsgEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the given error message entry in the buffer of the
 *            specified eh buffer index.
 * INPUT    : eh_buf_idx  --  the index of the eh message buffer to be set.
 * OUTPUT   : entry_p     --  the error message entry.
 * RETURN   :
 *            EH_TYPE_RET_OK                -- Get the entry from OM
 *                                             successfully.
 *            EH_TYPE_RET_INVALID_INPUT_ARG -- Invalid input argument.
 *            EH_TYPE_RET_NO_ERR_MSG        -- No error message info in the
 *                                             specified eh_buf_idx.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
EH_TYPE_RET_T EH_OM_GetErrorMsgEntry(UI8_T eh_buf_idx, EH_OM_ErrorMsgEntry_T *entry_p)
{
    /* sanity check
     */
    if (eh_buf_idx > SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printf("%s:Invalid eh_buf_idx %hu\r\n", __FUNCTION__, eh_buf_idx);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    if (entry_p==NULL)
    {
        printf("%s:entry_p is NULL\r\n", __FUNCTION__);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }
    if (eh_om_shmem_data_p->eh_buf[eh_buf_idx].ui_msg_no == EH_TYPE_INVALID_UI_MSG_NO)
    {
        return EH_TYPE_RET_NO_ERR_MSG;
    }

    EH_OM_EnterCriticalSection();
    *entry_p = eh_om_shmem_data_p->eh_buf[eh_buf_idx];
    EH_OM_LeaveCriticalSection();

    return EH_TYPE_RET_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_ClearErrorMsgEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Clear the error message info in buffer of the specified eh buffer
 *            index.
 * INPUT    : eh_buf_idx  --  the index of the eh message buffer to be cleared.
 * OUTPUT   : None
 * RETURN   :
 *            EH_TYPE_RET_OK                -- Set entry to OM successfully.
 *            EH_TYPE_RET_INVALID_INPUT_ARG -- Invalid input argument.
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
EH_TYPE_RET_T EH_OM_ClearErrorMsgEntry(UI8_T eh_buf_idx)
{
    /* sanity check
     */
    if (eh_buf_idx > SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printf("%s:Invalid eh_buf_idx %hu\r\n", __FUNCTION__, eh_buf_idx);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    EH_OM_EnterCriticalSection();
    eh_om_shmem_data_p->eh_buf[eh_buf_idx].ui_msg_no = EH_TYPE_INVALID_UI_MSG_NO;
    EH_OM_LeaveCriticalSection();
    return EH_TYPE_RET_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_SetXmlBufPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the XML buffer pointer to the given the index of the eh
 *            message buffer
 * INPUT    : eh_buf_idx  --  the index of the eh message buffer.
 *            entry_p     --  the error message entry to be set.
 * OUTPUT   : None
 * RETURN   :
 *            EH_TYPE_RET_OK                -- Set XML Buffer pointer to OM
 *                                             successfully.
 *            EH_TYPE_RET_INVALID_INPUT_ARG -- Invalid input argument.
 *            EH_TYPE_RET_SET_OM_FAILED     -- Failed to set XML Buffer pointer
 *                                             to OM.
 * NOTE     : Only UI thread will call this function to setup the XML Buffer
 *            pointer into its unique entry in OM. No need to do critical
 *            section protection in this function.
 *-------------------------------------------------------------------------
 */
EH_TYPE_RET_T EH_OM_SetXmlBufPtr(UI8_T eh_buf_idx, void *xml_buf_p)
{
    if (eh_buf_idx > SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printf("%s:Invalid eh_buf_idx %hu\r\n", __FUNCTION__, eh_buf_idx);
        return EH_TYPE_RET_INVALID_INPUT_ARG;
    }

    if(eh_om_shmem_data_p->xml_msg_buf_p_ar[eh_buf_idx]!=NULL)
    {
        printf("%s:XML Buffer Ptr for eh_buf_idx %hu is not NULL\r\n", __FUNCTION__, eh_buf_idx);
        return EH_TYPE_RET_SET_OM_FAILED;
    }

    eh_om_shmem_data_p->xml_msg_buf_p_ar[eh_buf_idx] = xml_buf_p;

    return EH_TYPE_RET_OK;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_OM_GetXmlBufPtr
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the XML buffer pointer to the given the index of the eh
 *            message buffer
 * INPUT    : eh_buf_idx  --  the index of the eh message buffer.
 * OUTPUT   : None
 * RETURN   : XML Buffer pointer for the given eh_buf_idx.
 * NOTE     : Only UI thread will call this function to get the XML Buffer
 *            pointer into its unique entry in OM. No need to do critical
 *            section protection in this function.
 *-------------------------------------------------------------------------
 */
void* EH_OM_GetXmlBufPtr(UI8_T eh_buf_idx)
{
    if (eh_buf_idx > SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printf("%s:Invalid eh_buf_idx %hu\r\n", __FUNCTION__, eh_buf_idx);
        return NULL;
    }

    return eh_om_shmem_data_p->xml_msg_buf_p_ar[eh_buf_idx];
}

/* LOCAL SUBPROGRAM BODIES
 */
