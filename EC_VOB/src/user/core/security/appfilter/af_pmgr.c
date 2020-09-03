/* ------------------------------------------------------------------------
 * FILE NAME - AF_PMGR.C
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
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_module.h"

#if (TRUE == SYS_CPNT_APP_FILTER)
#include "af_pmgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */
#define LOCAL_TEST true
/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */
#define AF_PMGR_MAX(a, b) (((a) > (b)) ? (a) : (b))

#ifdef _MSC_VER
#define AF_PMGR_IPC_BUF_DECL(req_sz, rep_sz)                                                  \
    const UI16_T        req_size = req_sz;                                                    \
    const UI16_T        rep_size = rep_sz;                                                    \
    UI8_T               ipc_buf[30];                                                          \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T *)&ipc_buf;                                 \
    AF_MGR_IpcMsg_T     *msg_p = (AF_MGR_IpcMsg_T *)msgbuf_p->msg_buf
#else
#define AF_PMGR_IPC_BUF_DECL(req_sz, rep_sz)                                                  \
    const UI32_T        req_size = req_sz;                                                    \
    const UI32_T        rep_size = rep_sz;                                                    \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(AF_PMGR_MAX(req_sz, rep_sz))];             \
    SYSFUN_Msg_T        *msgbuf_p = (void *)&ipc_buf;                                         \
    AF_MGR_IpcMsg_T     *msg_p = (AF_MGR_IpcMsg_T *)msgbuf_p->msg_buf
#endif /* #ifdef _MSC_VER */


#define AF_PMGR_IPC_RETVAL     \
    AF_MGR_MSG_RETVAL(msgbuf_p)

#define AF_PMGR_IPC_SEND_MSG(cmd_id, ret_val)                       \
    do {                                                            \
        msgbuf_p->cmd = SYS_MODULE_AF;                              \
        msgbuf_p->msg_size = req_size;                              \
        AF_MGR_MSG_CMD(msgbuf_p) = (cmd_id);                        \
                                                                    \
        if (SYSFUN_OK != SYSFUN_SendRequestMsg(ipcmsgq_handle,      \
                                    msgbuf_p,                       \
                                    SYSFUN_TIMEOUT_WAIT_FOREVER,    \
                                    SYSFUN_SYSTEM_EVENT_IPCMSG,     \
                                    rep_size,                       \
                                    msgbuf_p))                      \
            AF_PMGR_IPC_RETVAL = ret_val;                           \
    } while (0)

/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

#if LOCAL_TEST
static AF_MGR_IpcMsg_T *msg_p;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AF_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(AF_TYPE_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_SetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of CDP packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_SetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_PMGR_IPC_BUF_DECL(
        AF_MGR_GET_MSG_SIZE(pkt_status),
        AF_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA());

    /* set IPC input */
    msg_p->data.pkt_status.ifindex = ifindex;
    msg_p->data.pkt_status.status = status;

    AF_PMGR_IPC_SEND_MSG(
        AF_MGR_IPC_SET_PORT_CDP_STATUS,
        AF_TYPE_E_UNKNOWN);

    return AF_PMGR_IPC_RETVAL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_GetPortCdpStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of CDP packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_GetPortCdpStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status)
{
    AF_PMGR_IPC_BUF_DECL(
        AF_MGR_GET_MSG_SIZE(pkt_status),
        AF_MGR_GET_MSG_SIZE(pkt_status));

    /* set IPC input */
    msg_p->data.pkt_status.ifindex = ifindex;

    AF_PMGR_IPC_SEND_MSG(
        AF_MGR_IPC_GET_PORT_CDP_STATUS,
        AF_TYPE_E_UNKNOWN);

    /* set IPC output */
    *status = msg_p->data.pkt_status.status;

    return AF_PMGR_IPC_RETVAL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_SetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Set the status of PVST packet
 * INPUT  : ifindex    -- interface index
 *          status     -- AF_TYPE_STATUS_T
 * OUTPUT : None
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_SetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T status)
{
    AF_PMGR_IPC_BUF_DECL(
        AF_MGR_GET_MSG_SIZE(pkt_status),
        AF_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA());

    /* set IPC input */
    msg_p->data.pkt_status.ifindex = ifindex;
    msg_p->data.pkt_status.status = status;

    AF_PMGR_IPC_SEND_MSG(
        AF_MGR_IPC_SET_PORT_PVST_STATUS,
        AF_TYPE_E_UNKNOWN);

    return AF_PMGR_IPC_RETVAL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: AF_PMGR_GetPortPvstStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: Get the status of PVST packet
 * INPUT  : ifindex    -- interface index
 * OUTPUT : status     -- AF_TYPE_STATUS_T
 * RETURN : AF_TYPE_ErrorCode_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
UI32_T
AF_PMGR_GetPortPvstStatus(
    UI32_T ifindex,
    AF_TYPE_STATUS_T *status)
{
    AF_PMGR_IPC_BUF_DECL(
        AF_MGR_GET_MSG_SIZE(pkt_status),
        AF_MGR_GET_MSG_SIZE(pkt_status));

    /* set IPC input */
    msg_p->data.pkt_status.ifindex = ifindex;

    AF_PMGR_IPC_SEND_MSG(
        AF_MGR_IPC_GET_PORT_PVST_STATUS,
        AF_TYPE_E_UNKNOWN);

    /* set IPC output */
    *status = msg_p->data.pkt_status.status;

    return AF_PMGR_IPC_RETVAL;
}

#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */
