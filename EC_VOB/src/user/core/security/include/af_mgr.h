/* ------------------------------------------------------------------------
 * FILE NAME - AF_MGR.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Ezio             14/03/2013      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2013
 * ------------------------------------------------------------------------
 */
#ifndef _AF_MGR_H
#define _AF_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "af_type.h"
#include "sysfun.h"

/* NAMING CONSTANT DECLARARTIONS
 */
#define AF_MGR_IPCMSG_TYPE_SIZE    sizeof(AF_MGR_IpcMsg_Type_T)

/* command used in IPC message
 */
enum
{
    AF_MGR_IPC_SET_PORT_CDP_STATUS,
    AF_MGR_IPC_GET_PORT_CDP_STATUS,
    AF_MGR_IPC_SET_PORT_PVST_STATUS,
    AF_MGR_IPC_GET_PORT_PVST_STATUS,
};

/* MACRO DEFINITIONS
 */
#define AF_MGR_GET_MSG_SIZE(field_name)                       \
            (AF_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((AF_MGR_IpcMsg_T *)0)->data.field_name))

#define AF_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
            AF_MGR_IPCMSG_TYPE_SIZE

#define AF_MGR_MSG_CMD(msg_p)      (((AF_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define AF_MGR_MSG_RETVAL(msg_p)   (((AF_MGR_IpcMsg_T *)(msg_p)->msg_buf)->type.result_ui32)
#define AF_MGR_MSG_DATA(msg_p)     ((void *)&((AF_MGR_IpcMsg_T *)(msg_p)->msg_buf)->data)

/* TYPE DEFINITIONS
 */
typedef union
{
    UI32_T cmd;          /*cmd fnction id*/
    BOOL_T result_bool;  /*respond bool return*/
    UI32_T result_ui32;  /*respond ui32 return*/
    UI32_T result_i32;  /*respond i32 return*/
} AF_MGR_IpcMsg_Type_T;

typedef struct
{
    AF_MGR_IpcMsg_Type_T  type;

    union
    {
        struct
        {
            UI32_T     ifindex;
            AF_TYPE_STATUS_T status;
        } pkt_status;
    } data;
} AF_MGR_IpcMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void AF_MGR_InitiateSystemResources(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AF_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void AF_MGR_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Set component mode to Transition.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void AF_MGR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter transition mode.
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void AF_MGR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void AF_MGR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enter slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void AF_MGR_EnterSlaveMode(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_HandleHotInsertion
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
void AF_MGR_HandleHotInsertion(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port,
    BOOL_T in_use_default
);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AF_MGR_HandleHotRemoval
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
void AF_MGR_HandleHotRemoval(
    UI32_T in_starting_port_ifindex,
    UI32_T in_number_of_port
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of CDP packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_GetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of CDP packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_GetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of PVST packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_E
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_GetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of PVST packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_GetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status
);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE: Handle the ipc request message.
 * INPUT  : msgbuf_p -- input request ipc message buffer
 * OUTPUT : msgbuf_p -- output response ipc message buffer
 * RETURN : TRUE  - there is a  response required to be sent
 *          FALSE - there is no response required to be sent
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T
AF_MGR_HandleIPCReqMsg(
    SYSFUN_Msg_T *msgbuf_p
);

#endif /* #ifndef _AF_MGR_H */

