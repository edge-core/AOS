/* Module Name: CLI_OM.H
 * Purpose: Initialize the database resources and provide some Get/Set function
 *          for accessing the system log database.
 *
 * Notes:
 *
 * History:
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


#ifndef CLI_OM_H
#define CLI_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "stktplg_om.h"
#include "sysfun.h"
#include "cli_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

#define CLI_OM_GET_MSGBUFSIZE(field_name)                       \
            (CLI_OM_MSGBUF_TYPE_SIZE +                        \
            sizeof(((CLI_OM_IPCMsg_T*)0)->data.field_name))

#define CLI_OM_MSGBUF_TYPE_SIZE sizeof(union CLI_OM_IpcMsg_Type_U)
/* IPC message structure
 */
typedef struct
{
    union CLI_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        UI8_T ret_ui8;
        BOOL_T ret_bool;
    } type; /* the intended action or return value */

    union
    {
        struct CLI_OM_IPCMSG_OM_Data_S
        {
            STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
        }CLI_stacking_db_data;

        struct CLI_OM_IPCMSG_OM_Data_1_S
        {
            BOOL_T arg1;
        }CLI_om_arg1;

        struct
        {
            UI32_T module_types[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
        }CLI_module_types;

        struct
        {
            UI32_T uart_handler;
        }CLI_uart_handler;

        struct
        {
            CLI_TYPE_WorkingFlag_T flag;
        }get_working_flag;

    } data;
} CLI_OM_IPCMsg_T;

enum
{
    CLI_OM_IPC_GETSTACKINGDB,
    CLI_OM_IPC_GETCONFIGUARTIONMODULETYPES,
    CLI_IO_IPC_GETUARTHANDLER,
};

 /*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_Get_OperatingMode
 *-----------------------------------------------------------------------------
 * PURPOSE : CLI_OM_Get_OperatingMode
 *
 * INPUT   : NONE
 *
 * OUTPUT  : NONE
 *
 * RETURN  : SYS_TYPE_Stacking_Mode_T
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T CLI_OM_GetOperatingMode(void);

  /*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : DEBUG_OM_SetTransitionMode
 *
 * INPUT   : NONE
 *
 * OUTPUT  : NONE
 *
 * RETURN  : NONE
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void CLI_OM_SetTransitionMode(void);

 /*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE : CLI_OM_EnterTransitionMode
 *
 * INPUT   : NONE
 *
 * OUTPUT  : NONE
 *
 * RETURN  : NONE
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void CLI_OM_EnterTransitionMode(void);

 /*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE : CLI_OM_EnterMasterMode
 *
 * INPUT   : NONE
 *
 * OUTPUT  : NONE
 *
 * RETURN  : NONE
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void CLI_OM_EnterMasterMode(void);

 /*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE : CLI_OM_EnterSlaveMode
 *
 * INPUT   : NONE
 *
 * OUTPUT  : NONE
 *
 * RETURN  : NONE
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void CLI_OM_EnterSlaveMode(void);

void	CLI_OM_InitOmSem(void);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for CLI OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
CLI_TYPE_ReturnValue_T CLI_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/* FUNCTION NAME: CLI_MGR_GetStackingDB
 * PURPOSE: This API is used to get the mac and unit id and device type
 * in CLI configuration file header
 * INPUT:   None.
 * OUTPUT:  stacking_db : array of structure.
 * RETUEN:  0  -- failure
 *          otherwise -- success. the returned value is the number of entries
 * NOTES:   None.
 */

UI8_T CLI_OM_GetStackingDB(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - CLI_OM_GetConfiguartionModuleTypes
 * ------------------------------------------------------------------------
 * PURPOSE  : This function is used to get module type in config file
 *
 * INPUT    : module_types
 * OUTPUT   : module_types
 * RETURN   : TRUE/FALSE
 * NOTE     :
 * ------------------------------------------------------------------------
 */
BOOL_T CLI_OM_GetConfiguartionModuleTypes(UI32_T  module_types[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

/* ------------------------------------------------------------------------
 * FUNCTION NAME: CLI_OM_GetUartHandler
 * ------------------------------------------------------------------------
 * PURPOSE : This API is used to get uart Handler
 * INPUT   : None.
 * OUTPUT  : uart_handler_p
 * RETUEN  : TRUE/FALSE
 * NOTES   : None.
 * ------------------------------------------------------------------------
 */
BOOL_T  CLI_OM_GetUartHandler(UI32_T *uart_handler_p);

#endif

