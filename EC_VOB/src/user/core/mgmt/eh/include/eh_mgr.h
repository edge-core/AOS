/* Module Name: eh_mgr.h
 * Purpose:
 *    EH is a short name for "Error Handler".  
 *    EH provides processing request for display of clear and meaningful error
 *    message due to failure of an operation executed by UI.
 *
 *    EH supports to output error message in different languages and encoding. 
 *
 * Notes: 
 *      1. An system-wised EH message buffer is allocated and binded to a UI
 *         thread when a UI thread is spawned. 
 *      2. Each UI thread can hold one and only one UI error message in the
 *         system-wised EH message buffer. Note that the UI error message will 
 *         be cleared after processing the request for display error message.
 *     	3. Scenario of usage.
 *         (1)in caller (executed in UI thread context)  :
 *            call service function...
 *            if return an error status
 *            then
 *               EH_MGR_Process_Error_Info  (...);
 *               show_ui_message (...);
 *         (2)in service routine :
 *            if there is an error can't continuously processing...
 *               EH_MGR_Handle_Exception (...);
 *               return (error_status);
 *
 * History:                                                               
 *        Date          -- Modifier,    Reason
 * 00.00  2002.09.06    -- William      Create.
 * 00.00  2002.10.04    -- Erica        Modify.
 * 00.00  2013.02.04    -- Charlie Chen Porting from Vxworks to Linux. Refine
 *                                      the design and add support for display
 *                                      of multiple language and encoding.
 *                                      Change prototype of
 *                                      EH_MGR_Handle_Exception() to consolidate
 *                                      three APIs,
 *                                      EH_MGR_Handle_Exception1(),
 *                                      EH_MGR_Handle_Exception2(),
 *                                      and EH_MGR_Handle_Exception3().
 *                                      Remove EH_MGR_Get_Exception_Info() and
 *                                      replace by EH_MGR_Process_Error_Info().         
 *
 * Copyright(C)      Accton Corporation, 2002.
 */

#ifndef 	_EH_MGR_H
#define 	_EH_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>

#include "sys_cpnt.h"
#include "sys_type.h"
#include "eh_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#if !defined(EH_TEST_WORK_AROUND)
#if (SYS_CPNT_EH == FALSE)
#define EH_MGR_Process_Error_Info(cookie, output_func_p, lang, encoding) ({EH_TYPE_RET_OK;})
#define EH_MGR_Handle_Error(module_id, function_no, ui_msg_no, msg_flag, num_of_arg, args) ({EH_TYPE_RET_OK;})

#if 1 /* These macros are defined for compile error workaround. Can be removed after all CSCs have modified for current EH design */
#define EH_MGR_Handle_Exception(module_id, function_no, format_id, msg_flag)
#define EH_MGR_Handle_Exception1(module_id, function_no, format_id, msg_flag, arg)
#define EH_MGR_Handle_Exception2(module_id, function_no, format_id, msg_flag, arg1, arg2)
#define EH_MGR_Handle_Exception3(module_id, function_no, format_id, msg_flag, arg1, arg2, arg3)
#endif /* end of #if 1 */

#endif /* SYS_CPNT_EH == TRUE */
#endif /* end of #if !defined(EH_TEST_WORK_AROUND) */

/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* EH_TEST_WORK_AROUND is for EH unit test only, it can be removed when
 * the platform has full support for EH on all of the CSCs
 */
#if (SYS_CPNT_EH == TRUE) || defined(EH_TEST_WORK_AROUND)
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: EH_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   None
 *---------------------------------------------------------------------------------*/
void EH_MGR_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_Process_Error_Info 
 *-------------------------------------------------------------------------
 * PURPOSE  : Process the request to output error message in specified language
 *            and encoding. 
 * INPUT    : cookie        -- A cookie that will be passed as an argument when
 *                             the specified function pointer output_func_p is
 *                             called.   
 *            output_fun_p  -- A function pointer that will be called with
 *                             cookie and the error message to be output.
 *                             In usual, output_fun_p is responsible for display
 *                             the error message on the appropriate UI.
 *            lang          -- The language of the output error message.
 *            encoding      -- The character encoding of the output error
 *                             message.
 * OUTPUT   : None
 * RETURN   : EH_TYPE_RET_OK                  --
 *                An error message is got from EH message buffer which is binded
 *                to the caller UI thread. The error message in this EH message
 *                buffer will be cleared after returning from this function. 
 *            EH_TYPE_RET_NO_ERR_MSG          --
 *                No error message is found in EH message buffer which is binded
 *                to the caller UI thread.
 *            EH_TYPE_RET_NO_BOUND_EH_MSG_BUF --
 *                Cannot locate the EH message buffer which is bound to the
 *                calling thread.
 *            EH_TYPE_RET_INVALID_INPUT_ARG --
 *                An error occurred due to invalid one or more input arguments.
 *            EH_TYPE_RET_NOT_SUPPORT       --
 *                The requested operation is not supported.
 *            EH_TYPE_RET_INVALID_UI_MSG_NO --
 *                The specified UI message number is invalid.
 *            EH_TYPE_RET_GENERATE_MSG_ERROR--
 *                An error occurred when generated the error message.
 * NOTE     : 1. This function shall only be invoked by CLI and WEB
 *            2. Current support language:
 *               EH_TYPE_LANGUAGE_EN_EN
 *               EH_TYPE_LANGUAGE_ZH_CN
 *            3. Current support encoding:
 *               EH_TYPE_ENCODING_UTF_8
 *            4. The EH message buffer which is binded to the caller UI thread
 *               will be cleared no matter the return status is successful or
 *               failure.
 *-------------------------------------------------------------------------*/
 EH_TYPE_RET_T EH_MGR_Process_Error_Info(void* cookie,
    EH_TYPE_OutputMessageFunction_T output_func_p,
    EH_TYPE_LANGUAGE_T lang,
    EH_TYPE_ENCODING_T encoding);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - EH_MGR_Handle_Error 
 *-------------------------------------------------------------------------
 * PURPOSE  : When an error occurs in the execution of the operation, it is
 *            able to call this function to set a clear and specific error
 *            message to OM for the reason of failure. This error message will
 *            be displayed on UI by a UI thread finally.
 * INPUT    : module_id           -- the module id of the caller.
 *            function_no         -- the caller's function no, defined by each
 *                                   module itself.
 *            ui_msg_no           -- Refer to "UI Message ID" in header comment
 *                                   of eh_type.h
 *            msg_flag            -- Not used now. Reserved for future
 *                                   development. Set as 0 now.
 *            num_of_arg          -- Number of element in array args.
 *            args                -- Array of arguments for printf like format
 *                                   string. Can be NULL if num_of_arg is 0.  
 * OUTPUT   : None
 * RETURN   :
 *  EH_TYPE_RET_OK                   --  The error info is set to EH successfully.
 *  EH_TYPE_RET_INVALID_INPUT_ARG    --  Invalid input argument.
 *  EH_TYPE_RET_NO_BOUND_EH_MSG_BUF  --  Cannot locate the EH message buffer
 *                                       bound to the caller thread.
 *  EH_TYPE_RET_SET_OM_FAILED        --  Failed to set OM.
 * NOTE     : The maximum number of argument allowed for the API is
 *            EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG. The requirement for
 *            increasing EH_TYPE_MAX_NUMBER_OF_ARG_FOR_UI_MSG needs to be
 *            reviewed by SA.
 *-------------------------------------------------------------------------*/
EH_TYPE_RET_T EH_MGR_Handle_Error(UI32_T module_id, UI32_T function_no,
    EH_TYPE_UIMsgNumber_T ui_msg_no, UI32_T msg_flag, UI32_T num_of_arg,
    UI32_T args[]);


#endif /* SYS_CPNT_EH == TRUE */

#if defined(EH_UNIT_TEST)
void eh_unit_test_register_backdoor(UI32_T ipc_msgq_key);
#endif /* end of #if defined(EH_UNIT_TEST) */

#endif   /* _EH_MGR_H */

