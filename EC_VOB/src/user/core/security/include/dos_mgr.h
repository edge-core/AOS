/* ------------------------------------------------------------------------
 * FILE NAME - DOS_MGR.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             24/01/2011      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2011
 * ------------------------------------------------------------------------
 */
#ifndef DOS_MGR_H
#define DOS_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "sys_type.h"
#include "dos_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DOS_MGR_IPCMSG_TYPE_SIZE    sizeof(DOS_MGR_IpcMsg_Type_T)

/* command used in IPC message
 */
enum
{
    DOS_MGR_IPC_SET_DATA_BY_FIELD,
    DOS_MGR_IPC_GET_DATA_BY_FIELD,
    DOS_MGR_IPC_GET_RUNNING_DATA_BY_FIELD,
};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in DOS_MGR_IpcMsg_T.data
 */
#define DOS_MGR_GET_MSG_SIZE(field_name)                       \
            (DOS_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((DOS_MGR_IpcMsg_T *)0)->data.field_name))

#define DOS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
            DOS_MGR_IPCMSG_TYPE_SIZE

#define DOS_MGR_MSG_CMD(msg_p)      (((DOS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define DOS_MGR_MSG_RETVAL(msg_p)   (((DOS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.result)
#define DOS_MGR_MSG_DATA(msg_p)     ((void *)&((DOS_MGR_IpcMsg_T *)(msg_p)->msg_buf)->data)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T field_id;
    BOOL_T use_dflt;
    DOS_TYPE_FieldDataBuf_T data;
} DOS_MGR_IpcMsg_FldData_T;

typedef union
{
    UI32_T  cmd;        /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result;     /* for response */
} DOS_MGR_IpcMsg_Type_T;

typedef union
{
    BOOL_T                      bool_data;
    UI32_T                      ui32_data;
    DOS_MGR_IpcMsg_FldData_T    fld_data;
} DOS_MGR_IpcMsg_Data_T;

typedef struct
{
    DOS_MGR_IpcMsg_Type_T type;
    DOS_MGR_IpcMsg_Data_T data;
} DOS_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_InitiateSystemResources(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - DOS_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void DOS_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Set component mode to Transition.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void DOS_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter transition mode.
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void DOS_MGR_EnterSlaveMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE: This function will initialize the port_p OM of the module ports
 *          when the option module is inserted.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   inserted
 *          number_of_port        -- the number of ports on the inserted
 *                                   module
 *          use_default           -- the flag indicating the default
 *                                   configuration is used without further
 *                                   provision applied; TRUE if a new module
 *                                   different from the original one is
 *                                   inserted
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void DOS_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DOS_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE: This function will clear the port_p OM of the module ports when
 *          the option module is removed.
 * INPUT  : starting_port_ifindex -- the ifindex of the first module port_p
 *                                   removed
 *          number_of_port        -- the number of ports on the removed
 *                                   module
 * OUTPUT : None
 * RETURN : None
 * NOTE   : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void DOS_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *msgbuf_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_SetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set data by field
 * INPUT  : field_id -- DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value to set
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 *                      specifies NULL to set to default value
 * OUTPUT : None.
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_MGR_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_MGR_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_MGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T DOS_MGR_GetRunningDataByField(DOS_TYPE_FieldId_T field_id, void *data_p);

#endif /* DOS_MGR_H */

