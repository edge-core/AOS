/* MODULE NAME:  VRRP_POM.C
 * PURPOSE:
 *    POM implement for VRRP.
 *
 * NOTES:
 *
 *
 * History:
 *          Date    --  Modifier,  Reason
 *    2014/ 3/ 10    -- Jimi Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */

/* INCLUDE FILE DECLARATIONS 
 */
#include "sys_cpnt.h"
#if (SYS_CPNT_VRRP == TRUE)
#include <string.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "vrrp_om.h"

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void VRRP_POM_SendMsg(UI32_T cmd, SYSFUN_Msg_T *msg_p, UI32_T req_size, UI32_T res_size, UI32_T ret_val);

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES 
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_InitiateProcessResource
 *-------------------------------------------------------------------------
 * PURPOSE : Initiate resource used in the calling process.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T VRRP_POM_InitiateProcessResource(void)
{
    if (SYSFUN_GetMsgQ(VRRP_OM_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;

    }
    
    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_POM_GetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_OM_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = VRRP_OM_GET_MSGBUFSIZE(msg_p->data.oper_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = msg_size;
    msg_p = (VRRP_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_OM_IPC_GET_OPER_ENTRY;
    
    memset(&(msg_p->data.oper_entry), 0, sizeof(VRRP_OPER_ENTRY_T));
    memcpy(&(msg_p->data.oper_entry), vrrp_oper_entry, sizeof(VRRP_OPER_ENTRY_T));
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(vrrp_oper_entry, &(msg_p->data.oper_entry), sizeof(VRRP_OPER_ENTRY_T));
    
    return msg_p->type.result;
    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetNextVrrpAssoIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function returns the next availabe associated IP address of the
 *            vrrp on the specific interface.
 * INPUT    : associated_info.ifindex -- specify which ifindex info to be deleted.
 *            associated_info.vrid -- specify which vrid to be deleted
 *            associated_info.ip_addr -- specify which ip address
 * OUTPUT   : returns the next associated ip address info
 * RETURN   : VRRP_TYPE_OK/
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    : none
 *--------------------------------------------------------------------------*/
UI32_T VRRP_POM_GetNextVrrpAssoIpAddress(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_entry)
{
    VRRP_OM_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_AssocIpEntry_T);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(NULL == assoc_ip_entry)
        return VRRP_TYPE_PARAMETER_ERROR;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = msg_size;
    msg_p = (VRRP_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_OM_IPC_GET_NEXT_ASSOC_IP_ADDRESS;
    
    memcpy(&(msg_p->data.assoc_ip_entry), assoc_ip_entry, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(assoc_ip_entry, &(msg_p->data.assoc_ip_entry), sizeof(VRRP_ASSOC_IP_ENTRY_T));
    return msg_p->type.result;    
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr
 *--------------------------------------------------------------------------
 * PURPOSE  : This function get specified vrrp associated IP entry by ifindex and ip address
 * INPUT    : associated_info.ip_addr -- specify which ip address to search
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK /
 *            VRRP_TYPE_PARAMETER_ERROR/
 *            VRRP_TYPE_ASSO_IP_ADDR_ENTRY_NOT_EXIST
 * NOTES    : None
 *--------------------------------------------------------------------------*/
UI32_T VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(VRRP_ASSOC_IP_ENTRY_T *assoc_ip_info)
{
    VRRP_OM_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_AssocIpEntry_T);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(NULL == assoc_ip_info)
        return VRRP_TYPE_PARAMETER_ERROR;
    
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = msg_size;
    msg_p = (VRRP_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_OM_IPC_GET_ASSOC_IP_ADDRESS_BY_IP;
    
    memcpy(&(msg_p->data.assoc_ip_entry), assoc_ip_info, sizeof(VRRP_ASSOC_IP_ENTRY_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    memcpy(assoc_ip_info, &(msg_p->data.assoc_ip_entry), sizeof(VRRP_ASSOC_IP_ENTRY_T));
    return msg_p->type.result;    
}

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_POM_GetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_POM_GetPingStatus(UI32_T *ping_status)
{
    VRRP_OM_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = VRRP_OM_GET_MSGBUFSIZE(VRRP_OM_IPCMsg_UI32_T);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    
    if (NULL == ping_status)
        return VRRP_TYPE_RESULT_FAIL;
        
    msgbuf_p->cmd = SYS_MODULE_VRRP;
    msgbuf_p->msg_size = msg_size;
    msg_p = (VRRP_OM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = VRRP_OM_IPC_GET_PING_STATUS;
    
    msg_p->data.u32.value = *ping_status;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                              msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return VRRP_TYPE_RESULT_FAIL;
    }
    
    *ping_status = msg_p->data.u32.value;
    
    return msg_p->type.result;
}
#endif
#endif
