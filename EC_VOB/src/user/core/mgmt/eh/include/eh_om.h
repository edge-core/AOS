/* MODULE NAME:  eh_om.h
 * PURPOSE:
 *  OM definitions for EH(Error Handler).
 *
 * NOTES:
 *
 * HISTORY:
 *      Date        -- Modifier,        Reason
 *      2013.02.04  -- Charlie Chen     Create.
 * Copyright(C)      Edge-Core Networks, 2013
 */
#ifndef EH_OM_H
#define EH_OM_H

/* INCLUDE FILE DECLARATIONS
 */
#include "eh_type.h"
#include "sysfun_type.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T   module_id;      /* module id defined in sys_module.h */
    UI8_T   msg_flag;       /* Reserved. Must be set as 0 now. */
    UI8_T   function_no;    /* Caller's function number, defined by each module itself */
    UI8_T   num_of_arg;     /* Number of argument in args[] */
    UI32_T  args[EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG]; /* an array of arguments for printf like format string */
    EH_TYPE_UIMsgNumber_T ui_msg_no; /* UI Message Number. Refer to file header comment of eh_type.h for details. */
} EH_OM_ErrorMsgEntry_T;

typedef struct
{
    EH_OM_ErrorMsgEntry_T eh_buf[SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK];
    void*                 xml_msg_buf_p_ar[SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK];  /* Buffer for handling the UI message excerpted from XML file */
} EH_OM_ShmemData_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void EH_OM_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: EH_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
void EH_OM_AttachSystemResources(void);

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
void EH_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

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
EH_TYPE_RET_T EH_OM_SetErrorMsgEntry(UI8_T eh_buf_idx, EH_OM_ErrorMsgEntry_T *entry_p);

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
EH_TYPE_RET_T EH_OM_GetErrorMsgEntry(UI8_T eh_buf_idx, EH_OM_ErrorMsgEntry_T *entry_p);

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
EH_TYPE_RET_T EH_OM_ClearErrorMsgEntry(UI8_T eh_buf_idx);

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
EH_TYPE_RET_T EH_OM_SetXmlBufPtr(UI8_T eh_buf_idx, void *xml_buf_p);

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
void* EH_OM_GetXmlBufPtr(UI8_T eh_buf_idx);

#endif    /* End of EH_OM_H */
