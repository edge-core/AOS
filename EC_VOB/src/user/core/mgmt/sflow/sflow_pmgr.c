/* -------------------------------------------------------------------------------------
 * FILE NAME: sflow_pmgr.c
 * -------------------------------------------------------------------------------------
 * PURPOSE:  This package provides the sevices to manage the RFC3176 MIB.
 * NOTE:  1. The key functions of this module are to provide interfaces for the
 *           upper layer to configure sFlow, update database information base on the
 *           confiugration, and configure the lower layers(swctrl).
 *        2. This package shall be a reusable package for all the L2/L3 switchs.
 *
 * MODIFICATION HISTORY:
 * Modifier      Date          Version      Description
 * -------------------------------------------------------------------------------------
 * Nelson Dai    09-09-2009    V1.0         First Created
 * Nelson Dai    12-03-2009    V1.1         Support RFC-3176 MIB
 * -------------------------------------------------------------------------------------
 * Copyright(C)                 Accton Technology Corp. 2009
 * -------------------------------------------------------------------------------------*/

#include "string.h"
#include "stdio.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "l_stdlib.h"

#include "sflow_mgr.h"
#include "sflow_pmgr.h"

#if (SYS_CPNT_SFLOW == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */
static SYSFUN_MsgQ_T sflow_ipcmsgq_handle;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE  : Initiate resource used in the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE  - Success,
 *            FALSE - Error
 * NOTES    : Before other CSC use SFLOW_PMGR, it should initiate the resource (get the message queue handler internally)
 * ------------------------------------------------------------------------
 */
BOOL_T SFLOW_PMGR_InitiateProcessResource(void)
{
    /* Given that CSCA PMGR requests are handled in CSCGROUP1 of XXX_PROC
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_SFLOW_GROUP_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &sflow_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_PMGR_GetAgentAddress
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address.
 * INPUT   : addr_p -- agent's address
 * OUTPUT  : None
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetAgentAddress(
    L_INET_AddrIp_T *addr_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(addr_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if (addr_p == NULL)
        return SFLOW_MGR_RETURN_FAIL;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* copy back */
    *addr_p = msg_p->data.addr_v;
    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_PMGR_GetAgentAddressType
 * -------------------------------------------------------------------------
 * FUNCTION: This function will get sFlow agent's address type.
 * INPUT   : None
 * OUTPUT  : type  -- agent's address type
 * RETURN  : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetAgentAddressType(
    UI32_T *type)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if (type == NULL)
        return SFLOW_MGR_RETURN_FAIL;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_AGENT_ADDRESS_TYPE;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    /* copy back */
    *type = msg_p->data.ui32_v;
    return msg_p->type.result_ui32;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverOwner
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver owner.
 * INPUT    : receiver_index  -- receiver index
 *            owner_name_p    -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverOwner(
    UI32_T receiver_index,
    char *owner_name_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ownername);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_OWNER;
    msg_p->data.ui32_ownername.ui32_a1 = receiver_index;
    strncpy(msg_p->data.ui32_ownername.owner_name_a2,
        owner_name_p,
        sizeof(msg_p->data.ui32_ownername.owner_name_a2));
    msg_p->data.ui32_ownername.owner_name_a2[sizeof(msg_p->data.ui32_ownername.owner_name_a2)-1] = '\0';

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverOwner */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverTimeout
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver timeout.
 * INPUT    : receiver_index  -- receiver index
 *            timeout         -- timeout value
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverTimeout(
    UI32_T receiver_index,
    UI32_T timeout)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_TIMEOUT;
    msg_p->data.ui32_ui32.ui32_a1 = receiver_index;
    msg_p->data.ui32_ui32.ui32_a2 = timeout;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverTimeout */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverDestination
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver destination IP address.
 * INPUT    : receiver_index  -- receiver index
 *            address_p       -- destination IP address
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverDestination(
    UI32_T receiver_index,
    L_INET_AddrIp_T *address_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ip);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_DESTINATION;
    msg_p->data.ui32_ip.ui32_a1 = receiver_index;
    memcpy(&(msg_p->data.ui32_ip.ip_a2),
        address_p,
        sizeof(msg_p->data.ui32_ip.ip_a2));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverDestination */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverSockPort
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver sock port.
 * INPUT    : receiver_index -- receiver index
 *            udp_port       -- udp port number
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverSockPort(
    UI32_T receiver_index,
    UI32_T udp_port)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_SOCK_PORT;
    msg_p->data.ui32_ui32.ui32_a1 = receiver_index;
    msg_p->data.ui32_ui32.ui32_a2 = udp_port;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverSockPort */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverMaxDatagramSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver maximum datagram size.
 * INPUT    : receiver_index     -- receiver index
 *            max_datagram_size  -- maximum datagram size
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverMaxDatagramSize(
    UI32_T receiver_index,
    UI32_T max_datagram_size)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_MAX_DATAGRAM_SIZE;
    msg_p->data.ui32_ui32.ui32_a1 = receiver_index;
    msg_p->data.ui32_ui32.ui32_a2 = max_datagram_size;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverMaxDatagramSize */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetReceiverDatagramVersion
 *-------------------------------------------------------------------------
 * PURPOSE  : Set receiver datagram version.
 * INPUT    : receiver_index    -- receiver index
 *            datagram_version  -- datagram version
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetReceiverDatagramVersion(
    UI32_T receiver_index,
    UI32_T datagram_version)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_RECEIVER_DATAGRAM_VERSION;
    msg_p->data.ui32_ui32.ui32_a1 = receiver_index;
    msg_p->data.ui32_ui32.ui32_a2 = datagram_version;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetReceiverDatagramVersion */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreateReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create receiver entry or update receiver timeout.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreateReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_CREATE_RECEIVER_ENTRY;
    memcpy(&(msg_p->data.receiver.entry),
        receiver_entry_p,
        sizeof(msg_p->data.receiver.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(receiver_entry_p,
        &(msg_p->data.receiver.entry),
        sizeof(SFLOW_MGR_Receiver_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_CreateReceiverEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroyReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy receiver entry by owner name.
 * INPUT    : owner_name_p  -- owner name
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_DestroyReceiverEntryByOwnerName(
    char *owner_name_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ownername);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_DESTROY_RECEIVER_ENTRY_BY_OWNER_NAME;
    strncpy(msg_p->data.ownername.owner_name,
        owner_name_p,
        sizeof(msg_p->data.ownername.owner_name));
    msg_p->data.ownername.owner_name[sizeof(msg_p->data.ownername.owner_name)-1] = '\0';

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_DestroyReceiverEntryByOwnerName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_RECEIVER_ENTRY;
    memcpy(&(msg_p->data.receiver.entry),
        receiver_entry_p,
        sizeof(msg_p->data.receiver.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(receiver_entry_p,
        &(msg_p->data.receiver.entry),
        sizeof(SFLOW_MGR_Receiver_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetReceiverEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_RECEIVER_ENTRY;
    memcpy(&(msg_p->data.receiver.entry),
        receiver_entry_p,
        sizeof(msg_p->data.receiver.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(receiver_entry_p,
        &(msg_p->data.receiver.entry),
        sizeof(SFLOW_MGR_Receiver_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextReceiverEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveReceiverEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveReceiverEntry(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_RECEIVER_ENTRY;
    memcpy(&(msg_p->data.receiver.entry),
        receiver_entry_p,
        sizeof(msg_p->data.receiver.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(receiver_entry_p,
        &(msg_p->data.receiver.entry),
        sizeof(SFLOW_MGR_Receiver_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextActiveReceiverEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActiveReceiverEntryByOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active receiver entry.
 * INPUT    : receiver_entry_p  -- receiver entry
 * OUTPUT   : receiver_entry_p  -- receiver entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_index is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActiveReceiverEntryByOwnerName(
    SFLOW_MGR_Receiver_T *receiver_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(receiver);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_ACTIVE_RECEIVER_ENTRY_BY_OWNER_NAME;
    msg_p->data.receiver.entry.receiver_index = receiver_entry_p->receiver_index;

    strncpy(msg_p->data.receiver.entry.owner_name,
        receiver_entry_p->owner_name,
        sizeof(msg_p->data.receiver.entry.owner_name));
    msg_p->data.receiver.entry.owner_name[sizeof(msg_p->data.receiver.entry.owner_name)-1] = '\0';

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(receiver_entry_p,
        &(msg_p->data.receiver.entry),
        sizeof(SFLOW_MGR_Receiver_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetActiveReceiverEntryByOwnerName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingRate
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling rate.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            rate         -- sampling rate
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetSamplingRate(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T rate)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_SAMPLING_RATE;
    msg_p->data.ui32_ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32_ui32.ui32_a2 = instance_id;
    msg_p->data.ui32_ui32_ui32.ui32_a3 = rate;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetSamplingRate */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingMaxHeaderSize
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling maximum header size.
 * INPUT    : ifindex          -- interface index
 *            instance_id      -- instance id
 *            max_header_size  -- sampling maximum header size
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetSamplingMaxHeaderSize(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T max_header_size)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_SAMPLING_MAX_HEADER_SIZE;
    msg_p->data.ui32_ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32_ui32.ui32_a2 = instance_id;
    msg_p->data.ui32_ui32_ui32.ui32_a3 = max_header_size;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetSamplingMaxHeaderSize */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetSamplingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set sampling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- sampling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetSamplingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_SAMPLING_RECEIVER_INDEX;
    msg_p->data.ui32_ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32_ui32.ui32_a2 = instance_id;
    msg_p->data.ui32_ui32_ui32.ui32_a3 = receiver_index;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetSamplingReceiverIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreateSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreateSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_CREATE_SAMPLING_ENTRY;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* end of SFLOW_PMGR_CreateSamplingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroySamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy sampling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_DestroySamplingEntry(
    UI32_T ifindex,
    UI32_T instance_id)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_DESTROY_SAMPLING_ENTRY;
    msg_p->data.ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32.ui32_a2 = instance_id;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* end of SFLOW_PMGR_DestroySamplingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_SAMPLING_ENTRY;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetSamplingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActiveSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActiveSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_ACTIVE_SAMPLING_ENTRY;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetActiveSamplingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextSamplingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next sampling entry.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextSamplingEntry(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_SAMPLING_ENTRY;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextSamplingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveSamplingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by receiver owner name.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveSamplingEntryByReveiverOwnerName(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_RECEIVER_OWNER_NAME;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextActiveSamplingEntryByReveiverOwnerName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActiveSamplingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active sampling entry by data source.
 * INPUT    : sampling_entry_p  -- sampling entry
 * OUTPUT   : sampling_entry_p  -- sampling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActiveSamplingEntryByDatasource(
    SFLOW_MGR_Sampling_T *sampling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(sampling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_SAMPLING_ENTRY_BY_DATASOURCE;
    memcpy(&(msg_p->data.sampling.entry),
        sampling_entry_p,
        sizeof(msg_p->data.sampling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(sampling_entry_p,
        &(msg_p->data.sampling.entry),
        sizeof(SFLOW_MGR_Sampling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextActiveSamplingEntryByDatasource */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetPollingInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling interval.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 *            interval     -- polling interval
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetPollingInterval(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T interval)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_POLLING_INTERVAL;
    msg_p->data.ui32_ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32_ui32.ui32_a2 = instance_id;
    msg_p->data.ui32_ui32_ui32.ui32_a3 = interval;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetPollingInterval */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_SetPollingReceiverIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Set polling receiver index.
 * INPUT    : ifindex         -- interface index
 *            instance_id     -- instance id
 *            receiver_index  -- polling receiver index
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_SetPollingReceiverIndex(
    UI32_T ifindex,
    UI32_T instance_id,
    UI32_T receiver_index)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_SET_POLLING_RECEIVER_INDEX;
    msg_p->data.ui32_ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32_ui32.ui32_a2 = instance_id;
    msg_p->data.ui32_ui32_ui32.ui32_a3 = receiver_index;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_SetPollingReceiverIndex */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_CreatePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_CreatePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_CREATE_POLLING_ENTRY;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* end of SFLOW_PMGR_CreatePollingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_DestroyPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Destroy polling entry.
 * INPUT    : ifindex      -- interface index
 *            instance_id  -- instance id
 * OUTPUT   : None
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_DestroyPollingEntry(
    UI32_T ifindex,
    UI32_T instance_id)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(ui32_ui32);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_DESTROY_POLLING_ENTRY;
    msg_p->data.ui32_ui32.ui32_a1 = ifindex;
    msg_p->data.ui32_ui32.ui32_a2 = instance_id;

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    return msg_p->type.result_ui32;
} /* end of SFLOW_PMGR_DestroyPollingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_POLLING_ENTRY;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetPollingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetActivePollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get active polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetActivePollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GET_ACTIVE_POLLING_ENTRY;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetActivePollingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextPollingEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next polling entry.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex and instance_id is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextPollingEntry(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_POLLING_ENTRY;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextPollingEntry */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActivePollingEntryByReveiverOwnerName
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by receiver owner name.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : receiver_owner_name is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActivePollingEntryByReveiverOwnerName(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_RECEIVER_OWNER_NAME;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextActivePollingEntryByReveiverOwnerName */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_PMGR_GetNextActivePollingEntryByDatasource
 *-------------------------------------------------------------------------
 * PURPOSE  : Get next active polling entry by data source.
 * INPUT    : polling_entry_p  -- polling entry
 * OUTPUT   : polling_entry_p  -- polling entry
 * RETUEN   : SFLOW_MGR_RETURN_SUCCESS/SFLOW_MGR_RETURN_FAIL
 * NOTES    : ifindex is index
 * ------------------------------------------------------------------------
 */
SFLOW_MGR_RETURN_VALUE_T
SFLOW_PMGR_GetNextActivePollingEntryByDatasource(
    SFLOW_MGR_Polling_T *polling_entry_p)
{
    SFLOW_MGR_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = SFLOW_MGR_GET_MSG_SIZE(polling);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_SFLOW;
    msgbuf_p->msg_size = msg_size;

    msg_p = (SFLOW_MGR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = SFLOW_MGR_IPCCMD_GETNEXT_ACTIVE_POLLING_ENTRY_BY_DATASOURCE;
    memcpy(&(msg_p->data.polling.entry),
        polling_entry_p,
        sizeof(msg_p->data.polling.entry));

    if (SYSFUN_SendRequestMsg(sflow_ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SFLOW_MGR_RETURN_FAIL;
    }

    memcpy(polling_entry_p,
        &(msg_p->data.polling.entry),
        sizeof(SFLOW_MGR_Polling_T));

    return msg_p->type.result_ui32;
} /* End of SFLOW_PMGR_GetNextActivePollingEntryByDatasource */

#endif /* #if (SYS_CPNT_SFLOW == TRUE) */
