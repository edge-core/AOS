/* ------------------------------------------------------------------------
 * FILE NAME - AF_MGR.C
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

/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "string.h"
#include "sysfun.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "swctrl_pmgr.h"
#include "swctrl.h"

#if (TRUE == SYS_CPNT_APP_FILTER)
#include "af_mgr.h"
#include "af_om.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */
#ifndef ASSERT
#define ASSERT(a)
#endif /* #ifndef ASSERT(a) */

/* LOCAL FUNCTIONS DECLARATIONS
 */
static AF_TYPE_ErrorCode_T
AF_MGR_CheckIfIndex(
    UI32_T ifindex
);

/* LOCAL VARIABLES DECLARATIONS
 */
SYSFUN_DECLARE_CSC

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
void AF_MGR_InitiateSystemResources(void)
{
    AF_OM_Init();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - AF_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void AF_MGR_Create_InterCSC_Relation(void)
{
}

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
void AF_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

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
void AF_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
    AF_OM_Init();
}

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
void AF_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    AF_OM_Init();
}

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
void AF_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

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
    BOOL_T in_use_default)
{
}

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
    UI32_T in_number_of_port)
{
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_CheckIfIndex
 *-----------------------------------------------------------------------------
 * PURPOSE: Check the ifindex is validaated for AF to access
 * INPUT  : ifindex    -- interface index
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
static AF_TYPE_ErrorCode_T
AF_MGR_CheckIfIndex(
    UI32_T ifindex)
{
    UI32_T unit,port,trunk_id;
    SWCTRL_Lport_Type_T  port_type;

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
    if(port_type != SWCTRL_LPORT_NORMAL_PORT)
    {
        return AF_TYPE_E_PARAMETER;
    }

    return AF_TYPE_SUCCESS;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of CDP packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_TYPE_ErrorCode_T result;
    BOOL_T discard;

    result = AF_MGR_CheckIfIndex(ifindex);
    if (AF_TYPE_SUCCESS != result)
    {
        return result;
    }

    switch (status)
    {
        case AF_TYPE_DEFAULT:
            discard = FALSE;
            break;
        case AF_TYPE_DISCARD:
            discard = TRUE;
            break;
        default :
            return  AF_TYPE_E_PARAMETER;
    }

    if (FALSE == SWCTRL_PMGR_DropPortCdpPacket(discard, ifindex))
    {
        return  AF_TYPE_E_SET_SWCTRL;
    }

    result = AF_OM_SetPortCdpStatus(ifindex, status);

    return result;
}

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
    AF_TYPE_STATUS_T *status)
{
    AF_TYPE_ErrorCode_T result;

    result = AF_MGR_CheckIfIndex(ifindex);
    if (AF_TYPE_SUCCESS != result)
    {
        return result;
    }

    result = AF_OM_GetPortCdpStatus(ifindex, status);

    return result;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_MGR_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of PVST packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
AF_TYPE_ErrorCode_T
AF_MGR_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_TYPE_ErrorCode_T result;
    BOOL_T discard;

    result = AF_MGR_CheckIfIndex(ifindex);
    if (AF_TYPE_SUCCESS != result)
    {
        return result;
    }

    switch (status)
    {
        case AF_TYPE_DEFAULT:
            discard = FALSE;
            break;
        case AF_TYPE_DISCARD:
            discard = TRUE;
            break;
        default :
            return  AF_TYPE_E_PARAMETER;
    }

    if (FALSE == SWCTRL_PMGR_DropPortPvstPacket(discard, ifindex))
    {
        return  AF_TYPE_E_SET_SWCTRL;
    }

    result = AF_OM_SetPortPvstStatus(ifindex, status);

    return result;
}

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
    AF_TYPE_STATUS_T *status)
{
    AF_TYPE_ErrorCode_T result;

    result = AF_MGR_CheckIfIndex(ifindex);
    if (AF_TYPE_SUCCESS != result)
    {
        return result;
    }

    result = AF_OM_GetPortPvstStatus(ifindex, status);

    return result;
}

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
BOOL_T AF_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *msgbuf_p)
{
    AF_MGR_IpcMsg_T *msg_p;

    if (msgbuf_p == NULL)
    {
        return FALSE;
    }

    msg_p = (AF_MGR_IpcMsg_T *)msgbuf_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        msg_p->type.result_ui32 = AF_TYPE_E_NOT_READY;
        msgbuf_p->msg_size = AF_MGR_IPCMSG_TYPE_SIZE;
        return TRUE;
    }

    /* dispatch IPC message and call the corresponding MGR function
     */
    switch (msg_p->type.cmd)
    {
#if (TRUE == SYS_CPNT_APP_FILTER_CDP)
        case AF_MGR_IPC_SET_PORT_CDP_STATUS:
            msgbuf_p->msg_size = AF_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            msg_p->type.result_ui32 = AF_MGR_SetPortCdpStatus(
                msg_p->data.pkt_status.ifindex,
                msg_p->data.pkt_status.status);
            break;
        case AF_MGR_IPC_GET_PORT_CDP_STATUS:
            msgbuf_p->msg_size =  AF_MGR_GET_MSG_SIZE(pkt_status);
            msg_p->type.result_ui32 = AF_MGR_GetPortCdpStatus(
                msg_p->data.pkt_status.ifindex,
                &msg_p->data.pkt_status.status);
            break;
#endif /* #if (TRUE == SYS_CPNT_APP_FILTER_CDP) */

#if (TRUE == SYS_CPNT_APP_FILTER_PVST)
        case AF_MGR_IPC_SET_PORT_PVST_STATUS:
            msgbuf_p->msg_size = AF_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            msg_p->type.result_ui32 = AF_MGR_SetPortPvstStatus(
                msg_p->data.pkt_status.ifindex,
                msg_p->data.pkt_status.status);
            break;
        case AF_MGR_IPC_GET_PORT_PVST_STATUS:
            msgbuf_p->msg_size = AF_MGR_GET_MSG_SIZE(pkt_status);
            msg_p->type.result_ui32 = AF_MGR_GetPortPvstStatus(
                msg_p->data.pkt_status.ifindex,
                &msg_p->data.pkt_status.status);
            break;
#endif /* #if (TRUE == SYS_CPNT_APP_FILTER_PVST) */
        default:
            ASSERT(0);
            msg_p->type.result_ui32 = AF_TYPE_E_IPC_CMD_INVALID;
            msgbuf_p->msg_size = AF_MGR_IPCMSG_TYPE_SIZE;
    }

    return TRUE;
}
#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */
