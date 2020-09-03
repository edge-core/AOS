/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_OSPF.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_MGR_OSPF and NETCFG_OM_OSPF service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#include <string.h>
#include <sys/types.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "netcfg_mgr_ospf.h"
#include "netcfg_pmgr_ospf.h"

static SYSFUN_MsgQ_T ipcmsgq_handle;

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_OSPF_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_OSPF in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_OSPF_InitiateProcessResource(void)/*init in netcfg_main*/
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterOspfSet
* PURPOSE:
*     Set router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_ROUTEROSPFSET;

    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = vrf_id;
    msg_p->data.arg_grp2.arg3 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterOspfUnset
* PURPOSE:
*     Unset router ospf.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_ROUTEROSPFUNSET;

    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = vrf_id;
    msg_p->data.arg_grp2.arg3 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  NETCFG_PMGR_OSPF_IfAuthenticationTypeSet
* PURPOSE:
*     Set OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPESET;

    msg_p->data.arg_grp7.arg1 = vr_id;
    msg_p->data.arg_grp7.arg2 = ifindex;
    msg_p->data.arg_grp7.arg3 = type;
    msg_p->data.arg_grp7.arg4 = addr_flag;
    msg_p->data.arg_grp7.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  NETCFG_PMGR_OSPF_IfAuthenticationTypeUnset
* PURPOSE:
*     Unset OSPF interface authentication type.
*
* INPUT:
*     vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPEUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfAuthenticationKeySet
* PURPOSE:
*      Set OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp10);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYSET;

    msg_p->data.arg_grp10.arg1 = vr_id;
    msg_p->data.arg_grp10.arg2 = ifindex;
    memcpy(msg_p->data.arg_grp10.arg3, auth_key, NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE + 1);
    msg_p->data.arg_grp10.arg4 = addr_flag;
    msg_p->data.arg_grp10.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfAuthenticationKeyUnset
* PURPOSE:
*      Unset OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfMessageDigestKeySet
* PURPOSE:
*      Set OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp11);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYSET;

    msg_p->data.arg_grp11.arg1 = vr_id;
    msg_p->data.arg_grp11.arg2 = ifindex;
    msg_p->data.arg_grp11.arg3 = key_id;
    memcpy(msg_p->data.arg_grp11.arg4, auth_key, NETCFG_TYPE_OSPF_AUTH_MD5_SIZE + 1);
    msg_p->data.arg_grp11.arg5 = addr_flag;
    msg_p->data.arg_grp11.arg6 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfMessageDigestKeyUnset
* PURPOSE:
*      Unset OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYUNSET;

    msg_p->data.arg_grp7.arg1 = vr_id;
    msg_p->data.arg_grp7.arg2 = ifindex;
    msg_p->data.arg_grp7.arg3 = key_id;
    msg_p->data.arg_grp7.arg4 = addr_flag;
    msg_p->data.arg_grp7.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfPrioritySet
* PURPOSE:
*      Set OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      priority
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp7);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFPRIORITYSET;

    msg_p->data.arg_grp7.arg1 = vr_id;
    msg_p->data.arg_grp7.arg2 = ifindex;
    msg_p->data.arg_grp7.arg3 = priority;
    msg_p->data.arg_grp7.arg4 = addr_flag;
    msg_p->data.arg_grp7.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfPriorityUnset
* PURPOSE:
*      Unset OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFPRIORITYUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfCostSet
* PURPOSE:
*      Set OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      cost
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFCOSTSET;

    msg_p->data.arg_grp9.arg1 = vr_id;
    msg_p->data.arg_grp9.arg2 = ifindex;
    msg_p->data.arg_grp9.arg3 = cost;
    msg_p->data.arg_grp9.arg4 = addr_flag;
    msg_p->data.arg_grp9.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfCostUnset
* PURPOSE:
*      Unset OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFCOSTUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfDeadIntervalSet
* PURPOSE:
*      Set OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFDEADINTERVALSET;

    msg_p->data.arg_grp9.arg1 = vr_id;
    msg_p->data.arg_grp9.arg2 = ifindex;
    msg_p->data.arg_grp9.arg3 = interval;
    msg_p->data.arg_grp9.arg4 = addr_flag;
    msg_p->data.arg_grp9.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfDeadIntervalUnset
* PURPOSE:
*      Unset OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFDEADINTERVALUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfHelloIntervalSet
* PURPOSE:
*      Set OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALSET;

    msg_p->data.arg_grp9.arg1 = vr_id;
    msg_p->data.arg_grp9.arg2 = ifindex;
    msg_p->data.arg_grp9.arg3 = interval;
    msg_p->data.arg_grp9.arg4 = addr_flag;
    msg_p->data.arg_grp9.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfHelloIntervalUnset
* PURPOSE:
*      Unset OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfRetransmitIntervalSet
* PURPOSE:
*      Set OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALSET;

    msg_p->data.arg_grp9.arg1 = vr_id;
    msg_p->data.arg_grp9.arg2 = ifindex;
    msg_p->data.arg_grp9.arg3 = interval;
    msg_p->data.arg_grp9.arg4 = addr_flag;
    msg_p->data.arg_grp9.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfRetransmitIntervalUnset
* PURPOSE:
*      Unset OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfTransmitDelaySet
* PURPOSE:
*      Set OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      delay
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp9);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYSET;

    msg_p->data.arg_grp9.arg1 = vr_id;
    msg_p->data.arg_grp9.arg2 = ifindex;
    msg_p->data.arg_grp9.arg3 = delay;
    msg_p->data.arg_grp9.arg4 = addr_flag;
    msg_p->data.arg_grp9.arg5 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME :  	NETCFG_PMGR_OSPF_IfTransmitDelayUnset
* PURPOSE:
*      Unset OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp8);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYUNSET;

    msg_p->data.arg_grp8.arg1 = vr_id;
    msg_p->data.arg_grp8.arg2 = ifindex;
    msg_p->data.arg_grp8.arg3 = addr_flag;
    msg_p->data.arg_grp8.arg4 = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
   
    return msg_p->type.result_ui32;

}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_NetworkSet
* PURPOSE:
*     Set ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_NETWORKSET;

    msg_p->data.arg_grp5.arg1 = vr_id;
    msg_p->data.arg_grp5.arg2 = proc_id;
    msg_p->data.arg_grp5.arg3 = network_addr;
    msg_p->data.arg_grp5.arg4 = masklen;
    msg_p->data.arg_grp5.arg5 = area_id;
    msg_p->data.arg_grp5.arg6 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_PMGR_OSPF_NetworkUnset
* PURPOSE:
*     Unset ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_NETWORKUNSET;

    msg_p->data.arg_grp5.arg1 = vr_id;
    msg_p->data.arg_grp5.arg2 = proc_id;
    msg_p->data.arg_grp5.arg3 = network_addr;
    msg_p->data.arg_grp5.arg4 = masklen;
    msg_p->data.arg_grp5.arg5 = area_id;
    msg_p->data.arg_grp5.arg6 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterIdSet
* PURPOSE:
*     Set ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*      router_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_ROUTERIDSET;

    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    msg_p->data.arg_grp2.arg3 = router_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RouterIdUnset
* PURPOSE:
*     Unset ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_ROUTERIDUNSET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_TimerSet
* PURPOSE:
*     Set ospf timer.
*
* INPUT:
*      vr_id,
*      proc_id,
*      delay,
*      hold.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_TIMERSET;
    
    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = delay;
    msg_p->data.arg_grp3.arg4 = hold;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_TimerUnset
* PURPOSE:
*     Set ospf timer to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_TIMERUNSET;
    
    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTMETRICSET;
    
    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    msg_p->data.arg_grp2.arg3 = metric;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTMETRICUNSET;
    
    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(ifname == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_PASSIVEIFSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = addr;
    strcpy(msg_p->data.arg_grp6.arg4, ifname);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(ifname == NULL)
        return NETCFG_TYPE_INVALID_ARG;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_PASSIVEIFUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = addr;
    strcpy(msg_p->data.arg_grp6.arg4, ifname);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583SET;
    
    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583UNSET;
    
    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OspfSummaryAddressSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfSummaryAddressSet(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_SUMMARY_ADDRSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = address;
    msg_p->data.arg_grp3.arg4 = mask;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OspfSummaryAddressUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      address.
*      mask
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfSummaryAddressUnset(UI32_T vr_id, UI32_T proc_id, UI32_T address, UI32_T mask)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_SUMMAYR_ADDRUNSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = address;
    msg_p->data.arg_grp3.arg4 = mask;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OspfAutoCostSet
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ref_bandwidth.
*      
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfAutoCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T ref_bandwidth)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AUTOCOSTSET;

    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    msg_p->data.arg_grp2.arg3 = ref_bandwidth;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OspfAutoCostUnset
* PURPOSE:
*     Set ospf summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OspfAutoCostUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AUTOCOSTUNSET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeProtoSet
* PURPOSE:
*     Set ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeProtoUnset
* PURPOSE:
*     Unset ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPESET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = metric_type;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPEUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = metric;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeTagSet
* PURPOSE:
*     Set ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      tag
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = tag;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeTagUnset
* PURPOSE:
*     Unset ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp_route_map);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPSET;
    
    memset(&(msg_p->data.arg_grp_route_map), 0, sizeof(msg_p->data.arg_grp_route_map));
    msg_p->data.arg_grp_route_map.arg1 = vr_id;
    msg_p->data.arg_grp_route_map.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp_route_map.arg3, proto_type);
    strcpy(msg_p->data.arg_grp_route_map.arg4, route_map);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_RedistributeRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp6.arg4, proto_type);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric_type
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPESET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = metric_type;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricTypeUnset(UI32_T vr_id, UI32_T proc_id )
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPEUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    msg_p->data.arg_grp6.arg3 = metric;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id )
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      route_map,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp_route_map);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPSET;
    
    memset(&(msg_p->data.arg_grp_route_map), 0, sizeof(msg_p->data.arg_grp_route_map));
    msg_p->data.arg_grp_route_map.arg1 = vr_id;
    msg_p->data.arg_grp_route_map.arg2 = proc_id;
    strcpy(msg_p->data.arg_grp_route_map.arg3, route_map);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoRoutemapUnset(UI32_T vr_id, UI32_T proc_id )
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp6);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPUNSET;
    
    memset(&(msg_p->data.arg_grp6), 0, sizeof(msg_p->data.arg_grp6));
    msg_p->data.arg_grp6.arg1 = vr_id;
    msg_p->data.arg_grp6.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoAlwaysSet
* PURPOSE:
*     Set ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWAYSSET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoAlwaysUnset
* PURPOSE:
*     Unset ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWWAYSUNSET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}
/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoSet
* PURPOSE:
*     Set ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_SET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_DefaultInfoUnset
* PURPOSE:
*     Unset ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_DEFAULTINFO_UNSET;

    msg_p->data.arg_grp1.arg1 = vr_id;
    msg_p->data.arg_grp1.arg2 = proc_id;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetOspfIfEntry
* PURPOSE:
*     Get ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRY;
    
    msg_p->data.arg_grp12.arg1 = vr_id;
    memset(&msg_p->data.arg_grp12.arg2, 0, sizeof(OSPF_TYPE_OspfInterfac_T));
    memcpy(&msg_p->data.arg_grp12.arg2, entry, sizeof(OSPF_TYPE_OspfInterfac_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &msg_p->data.arg_grp12.arg2, sizeof(OSPF_TYPE_OspfInterfac_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetOspfIfEntryByIfindex
* PURPOSE:
*     Get ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRYBYIFINDEX;
    
    msg_p->data.arg_grp12.arg1 = vr_id;
    memset(&msg_p->data.arg_grp12.arg2, 0, sizeof(OSPF_TYPE_OspfInterfac_T));
    memcpy(&msg_p->data.arg_grp12.arg2, entry, sizeof(OSPF_TYPE_OspfInterfac_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &msg_p->data.arg_grp12.arg2, sizeof(OSPF_TYPE_OspfInterfac_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextOspfIfEntry
* PURPOSE:
*     Get nex ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetNextOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRY;
    
    msg_p->data.arg_grp12.arg1 = vr_id;
    memset(&msg_p->data.arg_grp12.arg2, 0, sizeof(OSPF_TYPE_OspfInterfac_T));
    memcpy(&msg_p->data.arg_grp12.arg2, entry, sizeof(OSPF_TYPE_OspfInterfac_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &msg_p->data.arg_grp12.arg2, sizeof(OSPF_TYPE_OspfInterfac_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextOspfIfEntryByIfindex
* PURPOSE:
*     Get nex ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp12);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRYBYIFINDEX;
    
    msg_p->data.arg_grp12.arg1 = vr_id;
    memset(&msg_p->data.arg_grp12.arg2, 0, sizeof(OSPF_TYPE_OspfInterfac_T));
    memcpy(&msg_p->data.arg_grp12.arg2, entry, sizeof(OSPF_TYPE_OspfInterfac_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &msg_p->data.arg_grp12.arg2, sizeof(OSPF_TYPE_OspfInterfac_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp13);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETRUNNINGIFENTRYBYIFINDEX;
    
    msg_p->data.arg_grp13.arg1 = vr_id;
    memset(&msg_p->data.arg_grp13.arg2, 0, sizeof(NETCFG_TYPE_OSPF_IfConfig_T));
    memcpy(&msg_p->data.arg_grp13.arg2, entry, sizeof(NETCFG_TYPE_OSPF_IfConfig_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    memcpy(entry, &msg_p->data.arg_grp13.arg2, sizeof(NETCFG_TYPE_OSPF_IfConfig_T));
    return msg_p->type.result_ui32;
}
/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextSummaryAddress
* PURPOSE:
*     Getnext summary address.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetNextSummaryAddress(UI32_T vr_id, UI32_T proc_id, UI32_T *addr, UI32_T *masklen)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.summary_addr_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETNEXT_SUMMARY_ADDR;

    msg_p->data.summary_addr_entry.vr_id = vr_id;
    msg_p->data.summary_addr_entry.proc_id = proc_id;
    msg_p->data.summary_addr_entry.addr = *addr;
    msg_p->data.summary_addr_entry.masklen = *masklen;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *addr = msg_p->data.summary_addr_entry.addr;
    *masklen = msg_p->data.summary_addr_entry.masklen;

    return msg_p->type.result_ui32;
}
/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetAutoCostRefBandwidth
* PURPOSE:
*      Get auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T *refbw)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GET_REDIST_AUTOCOST;

    msg_p->data.arg_grp2.arg1 = vr_id;
    msg_p->data.arg_grp2.arg2 = proc_id;
    msg_p->data.arg_grp2.arg3 = *refbw;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *refbw = msg_p->data.arg_grp2.arg3;

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetRedistributeConfig
* PURPOSE:
*      Get redistribute configuration inormation.
*
* INPUT:
*            
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetRedistributeConfig(NETCFG_MGR_REDIST_CONFIG_T *redist_config)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.redist_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GET_REDIST_CONFIG;

    msg_p->data.redist_entry = *redist_config;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *redist_config = msg_p->data.redist_entry;

    return msg_p->type.result_ui32;
}


/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetDefaultInfoConfig
* PURPOSE:
*      Get default information configuration.
*
* INPUT:
*      redist_config
*      
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetDefaultInfoConfig(NETCFG_MGR_REDIST_CONFIG_T *redist_config)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.redist_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GET_DEFAULT_INFO_ORIGINATE;

    msg_p->data.redist_entry = *redist_config;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    *redist_config = msg_p->data.redist_entry;

    return msg_p->type.result_ui32;
}



/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREASTUBSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = area_id;
    msg_p->data.arg_grp3.arg4 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubUnset
* PURPOSE:
*     Unset ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREASTUBUNSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = area_id;
    msg_p->data.arg_grp3.arg4 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubNoSummarySet
* PURPOSE:
*     Set ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = area_id;
    msg_p->data.arg_grp3.arg4 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaStubNoSummaryUnset
* PURPOSE:
*     Unset ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYUNSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = area_id;
    msg_p->data.arg_grp3.arg4 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTSET;

    msg_p->data.arg_grp4.arg1 = vr_id;
    msg_p->data.arg_grp4.arg2 = proc_id;
    msg_p->data.arg_grp4.arg3 = area_id;
    msg_p->data.arg_grp4.arg4 = format;
    msg_p->data.arg_grp4.arg5 = cost;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTUNSET;

    msg_p->data.arg_grp3.arg1 = vr_id;
    msg_p->data.arg_grp3.arg2 = proc_id;
    msg_p->data.arg_grp3.arg3 = area_id;
    msg_p->data.arg_grp3.arg4 = format;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeSet
* PURPOSE:
*     Set ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREARANGESET;

    msg_p->data.arg_grp5.arg1 = vr_id;
    msg_p->data.arg_grp5.arg2 = proc_id;
    msg_p->data.arg_grp5.arg3 = area_id;
    msg_p->data.arg_grp5.arg4 = format;
    msg_p->data.arg_grp5.arg5 = range_addr;
    msg_p->data.arg_grp5.arg6 = range_masklen;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeNoAdvertiseSet
* PURPOSE:
*     Set ospf area range no advertise.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREARANGENOADVERTISESET;

    msg_p->data.arg_grp5.arg1 = vr_id;
    msg_p->data.arg_grp5.arg2 = proc_id;
    msg_p->data.arg_grp5.arg3 = area_id;
    msg_p->data.arg_grp5.arg4 = format;
    msg_p->data.arg_grp5.arg5 = range_addr;
    msg_p->data.arg_grp5.arg6 = range_masklen;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaRangeUnset
* PURPOSE:
*     Unset ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp5);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREARANGEUNSET;

    msg_p->data.arg_grp5.arg1 = vr_id;
    msg_p->data.arg_grp5.arg2 = proc_id;
    msg_p->data.arg_grp5.arg3 = area_id;
    msg_p->data.arg_grp5.arg4 = format;
    msg_p->data.arg_grp5.arg5 = range_addr;
    msg_p->data.arg_grp5.arg6 = range_masklen;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaNssaSet
* PURPOSE:
*     Set ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      nssa_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Nssa_Para_T *nssa_para)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp14);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREANSSASET;

    memset(&(msg_p->data.arg_grp14),0, sizeof(msg_p->data.arg_grp14));
    msg_p->data.arg_grp14.arg1 = vr_id;
    msg_p->data.arg_grp14.arg2 = proc_id;
    msg_p->data.arg_grp14.arg3 = area_id;
    msg_p->data.arg_grp14.arg4 = format;
    memcpy(&(msg_p->data.arg_grp14.arg5),nssa_para, sizeof(NETCFG_TYPE_OSPF_Area_Nssa_Para_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaNssaUnset
* PURPOSE:
*     Unset ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      flag.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREANSSAUNSET;

    msg_p->data.arg_grp4.arg1 = vr_id;
    msg_p->data.arg_grp4.arg2 = proc_id;
    msg_p->data.arg_grp4.arg3 = area_id;
    msg_p->data.arg_grp4.arg4 = format;
    msg_p->data.arg_grp4.arg5 = flag;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaVirtualLinkSet
* PURPOSE:
*     Set ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp17);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREAVLINKSET;

    memset(&(msg_p->data.arg_grp17),0, sizeof(msg_p->data.arg_grp17));
    msg_p->data.arg_grp17.arg1 = vr_id;
    msg_p->data.arg_grp17.arg2 = proc_id;
    msg_p->data.arg_grp17.arg3 = area_id;
    msg_p->data.arg_grp17.arg4 = format;
    memcpy(&(msg_p->data.arg_grp17.arg5),vlink_para, sizeof(NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T));

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_AreaVirtualLinkUnset
* PURPOSE:
*     Unset ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp17);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_AREAVLINKUNSET;

    memset(&(msg_p->data.arg_grp17),0, sizeof(msg_p->data.arg_grp17));
    msg_p->data.arg_grp17.arg1 = vr_id;
    msg_p->data.arg_grp17.arg2 = proc_id;
    msg_p->data.arg_grp17.arg3 = area_id;
    msg_p->data.arg_grp17.arg4 = format;
    memcpy(&(msg_p->data.arg_grp17.arg5),vlink_para, sizeof(NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    //printf("%s,%d,result:%x\n",__FUNCTION__,__LINE__,msg_p->type.result_ui32);
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_instance_para);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;


    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETINSTANCEPARA;

    memset(&(msg_p->data.arg_instance_para),0,sizeof(NETCFG_TYPE_OSPF_Instance_Para_T));   
    memcpy(&(msg_p->data.arg_instance_para),entry,sizeof(NETCFG_TYPE_OSPF_Instance_Para_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    memcpy(entry,&(msg_p->data.arg_instance_para),sizeof(NETCFG_TYPE_OSPF_Instance_Para_T));
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetNextPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp15);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETNEXTPASSIVEIF;

    memset(&(msg_p->data.arg_grp15),0,sizeof(msg_p->data.arg_grp15));
    msg_p->data.arg_grp15.arg1 = vr_id;
    msg_p->data.arg_grp15.arg2 = proc_id;
    memcpy(&(msg_p->data.arg_grp15.arg3),entry,sizeof(NETCFG_TYPE_OSPF_Passive_If_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    memcpy(entry,&(msg_p->data.arg_grp15.arg3),sizeof(NETCFG_TYPE_OSPF_Passive_If_T));
    
    return msg_p->type.result_ui32;
}

/* FUNCTION NAME : NETCFG_PMGR_OSPF_GetNextNetwork
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_PMGR_OSPF_GetNextNetwork(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry)
{
    NETCFG_MGR_OSPF_IPCMsg_T *msg_p = NULL;
    UI32_T msg_size = NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_p->data.arg_grp16);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;

    if(entry == NULL)
        return NETCFG_TYPE_INVALID_ARG;
    
    msgbuf_p->cmd = SYS_MODULE_OSPFCFG;
    msgbuf_p->msg_size = msg_size;
    msg_p = (NETCFG_MGR_OSPF_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_OSPF_IPC_GETNEXTNETWORK;

    memset(&(msg_p->data.arg_grp16),0,sizeof(msg_p->data.arg_grp16));
    msg_p->data.arg_grp16.arg1 = vr_id;
    msg_p->data.arg_grp16.arg2 = proc_id;
    memcpy(&(msg_p->data.arg_grp16.arg3),entry,sizeof(NETCFG_TYPE_OSPF_Network_T));
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }
    
    memcpy(entry,&(msg_p->data.arg_grp16.arg3),sizeof(NETCFG_TYPE_OSPF_Network_T));

    return msg_p->type.result_ui32;
}

