/* ------------------------------------------------------------------------
 * FILE NAME - BACKDOOR_MGR.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Charles Cheng     02/21/2002      new created
 *   Wakka             05/09/2007      design change.
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2002
 * ------------------------------------------------------------------------
 */
#ifndef  BACKDOOR_MGR_H
#define  BACKDOOR_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* To specify which mgr msgq to receive backdoor mgr messages.
 */
#define BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH 15
#define BACKDOOR_MGR_MIN_CSC_NAME_STRING_LENTH 1

#define BACKDOOR_MGR_RESERVED_WORD_FOR_EXIT    "exit"

#define BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME     SYS_BLD_TICKS_PER_SECOND

#define BACKDOOR_MGR_INVALID_MSGQ_KEY          SYSFUN_MSGQKEY_PRIVATE

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* IPC command used for communication between backdoor threads.
 *
 *      (direct call or new thread)       (async. IPC msg and suspense BACKDOOR_MAIN)
 *      ENTER_DEBUG_MODE                  INVOKE_BACKDOOR
 * CLI ------------------> BACKDOOR_MAIN ------------------> CSC
 *     <------------------               <------------------
 *      LEAVE_DEBUG_MODE                  REGISTER_BACKDOOR (direct call)
 *      (callback for new thread)         LEAVE_BACKDOOR    (resume BACKDOOR_MAIN)
 *
 */
typedef enum
{
    BACKDOOR_MGR_CMD_INVOKE_BACKDOOR
} BACKDOOR_MGR_Cmd_T;

/* IPC message for communication between backdoor threads.
 */
typedef struct
{
    BACKDOOR_MGR_Cmd_T backdoor_cmd;
    UI32_T session_idx;
    UI32_T callback_idx;
} BACKDOOR_MGR_Msg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Initiate system resource for BACKDOOR_MGR
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Attach system resource for BACKDOOR_MGR in the context of the
 *      calling process.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Provide shared memory information of BACKDOOR_MGR for SYSRSC.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      segid_p  --  shared memory segment id
 *      seglen_p --  length of the shared memroy segment
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack
 *-----------------------------------------------------------------
 * FUNCTION: To register which function wish to appear in backdoor
 *           of CLI.
 * INPUT   : csc_name   - The name of the csc (computer software compoment)
 *                        module that wish to hook to backdoor, and this
 *                        name will be appeared in the main menu of the
 *                        backdoor. The string length of the name should
 *                        <= BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH
 *                        and >= BACKDOOR_MGR_MIN_CSC_NAME_STRING_LENTH.
 *                        If the length > BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH
 *                        The name will be truncated.
 *                        If the length < BACKDOOR_MGR_MIN_CSC_NAME_STRING_LENTH
 *                        registering will fail.
 *                        The name should not be BACKDOOR_MGR_RESERVED_WORD because
 *                        this is reserved word and registering will fail.
 *           msgq_key   - The message queue that waits for backdoor message.
 *           func       - The functional pointer of the function
 *                        that wish to register.
 *
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------------*/
void BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(char *csc_name,
                                                       UI32_T msgq_key,
                                                       void (*func)(void));

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Main
 *-----------------------------------------------------------------
 * FUNCTION: This function is to enter debug mode.
 * INPUT   : forked       - TRUE - specify a thread will be spawned
 *                          to run debug mode.
 *                          FALSE - debug mode will run in caller's context.
 *           msgq_key     - The msgq key to handle I/O.
 *                          specify BACKDOOR_MGR_INVALID_MSGQ_KEY
 *                          to use stdin/stdout instead of IPC.
 *           func_getchar - The callback func to get char.
 *                          similar to getchar().
 *                          return negative number on failure.
 *           func_print   - The callback func to print string.
 *                          similar to printf("%s", s).
 *                          return negative number on failure.
 *           func_leave   - if a callback func is specified,
 *                          specified func will be called when debug
 *                          mode finish.
 *           cookie       - cookie for func_getchar/func_print/func_leave
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Main(
    BOOL_T forked,
    UI32_T msgq_key,
    int (*func_getchar)(void *cookie),
    int (*func_print)(void *cookie, const char *s),
    void (*func_leave)(void *cookie),
    void *cookie);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCMsg
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle backdoor messages.
 * INPUT   : msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_HandleIPCMsg(SYSFUN_Msg_T *msg_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCIOMsg
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle L_IPCIO messages.
 * INPUT   : msgq_key - the key of msgq that received the message.
 *           msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : should only be call by IO service thread.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_HandleIPCIOMsg(UI32_T msgq_key, SYSFUN_Msg_T *msg_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetChar
 *-----------------------------------------------------------------
 * FUNCTION: To get a character from UI.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : a character or EOF
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
int BACKDOOR_MGR_GetChar(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_RequestKeyIn
 *-----------------------------------------------------------------
 * FUNCTION: To get the string until enter is pressed from UI.
 * INPUT   : key_in_string - The pointer to the buffer to store
 *                           the key in string.
 *                           The size of buffer must reserve additional
 *                           space for '\0' (null terminate).
 *           max_key_len   - Maxium character user can key in.
 * OUTPUT  : key_in_string - Got string.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_RequestKeyIn(void *key_in_string, UI32_T max_key_len);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Print
 *-----------------------------------------------------------------
 * FUNCTION: To print a string.
 * INPUT   : str - The string to print.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : it may spend at most BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME
 *           on IPC.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Print(char *str);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Printf
 *-----------------------------------------------------------------
 * FUNCTION: To print a formatted string.
 * INPUT   : fmt_p - The format string.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : it may spend at most BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME
 *           on IPC.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Printf(char *fmt_p, ...);

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_DumpHex
 *-----------------------------------------------------------------
 * FUNCTION: Dump heximal code.
 * INPUT   : title   -- text displays on the output.
 *           len     -- data length to be displayed.
 *           buffer  -- buffer holding displaying data.
 * OUTPUT  : None.
 * RETURN  : see BACKDOOR_MGR_Print/BACKDOOR_MGR_Printf
 * NOTE    : see BACKDOOR_MGR_Print/BACKDOOR_MGR_Printf 
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_DumpHex(char *title, int len, void *buf);

#endif /* BACKDOOR_MGR_H */

