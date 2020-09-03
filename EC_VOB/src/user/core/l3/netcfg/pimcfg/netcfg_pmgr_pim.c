/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_PMGR_PIM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_MGR_PIM and NETCFG_OM_PIM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/05/18     --- Hongliang, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "netcfg_mgr_pim.h"
#include "netcfg_pmgr_pim.h"

static SYSFUN_MsgQ_T ipcmsgq_handle;

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_PIM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_PIM in the calling process.
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
BOOL_T NETCFG_PMGR_PIM_InitiateProcessResource(void)/*init in netcfg_main*/
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_PMGR_EnablePimDenseModeOnIf
* PURPOSE:
*     Enable Dense Mode Pim on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnablePimDenseModeOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_DENSE_MODE;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    if( TRUE == msg_p->type.result_bool )
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;

}


/* FUNCTION NAME : NETCFG_PMGR_DisablePimDenseModeOnIf
* PURPOSE:
*     Disable Dense Mode Pim on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisablePimDenseModeOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_DENSE_MODE;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}


/* FUNCTION NAME : NETCFG_PMGR_SetPimIfHelloIntv
* PURPOSE:
*     Set pim hello interval on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
*      interval -- the hello timer interval
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimIfHelloIntv(UI32_T ifindex, UI32_T interval)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_HELLO_INTERVAL;
    msg_p->data.arg_grp1.arg1 = ifindex;
    msg_p->data.arg_grp1.arg2 = interval;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimIfHelloHoldtime
* PURPOSE:
*     Set pim hold interval on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
*      interval -- the hello timer interval
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimIfHelloHoldtime(UI32_T ifindex, UI32_T interval)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_HELLO_HOLD_INTERVAL;
    msg_p->data.arg_grp1.arg1 = ifindex;
    msg_p->data.arg_grp1.arg2 = interval;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimIfNeighborFilter
* PURPOSE:
*     Set pim neighbor filter on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
*      filter     -- the neighbor filter ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimIfNeighborFilter(UI32_T ifindex, UI8_T *filter)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_NEIGHBOR_FILTER;
    msg_p->data.arg_grp2.arg1 = ifindex;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(filter) > SYS_ADPT_ACL_MAX_NAME_LEN )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp2.arg2, filter);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimIfNeighborFilter
* PURPOSE:
*     UnSet pim neighbor filter on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimIfNeighborFilter(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_NEIGHBOR_FILTER;
    msg_p->data.ui32_v = ifindex;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_PimExcludeGenid
* PURPOSE:
*     Exclude generation ID or not on some l3 interface.
*
* INPUT:
*      ifindex       -- the interface index
*      exclude_b   -- the neighbor filter ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_PimExcludeGenid(UI32_T ifindex, BOOL_T exclude_b)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp3);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_PIM_EXCLUDE_GENID;
    msg_p->data.arg_grp3.arg1 = ifindex;
    msg_p->data.arg_grp3.arg2 = exclude_b;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimIfDrPriority
* PURPOSE:
*     Set pim neighbor filter on some l3 interface.
*
* INPUT:
*      ifindex     -- the interface index
*      priority     -- the neighbor filter ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimIfDrPriority(UI32_T ifindex, UI32_T priority)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_DR_PRIORITY;
    msg_p->data.arg_grp1.arg1 = ifindex;
    msg_p->data.arg_grp1.arg2 = priority;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimDrPriority
* PURPOSE:
*     UnSet pim DR priority on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimIfDrPriority(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_DR_PRIORITY;
    msg_p->data.ui32_v = ifindex;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_EnablePimSparseModeOnIf
* PURPOSE:
*     Enable Sparse Mode Pim on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnablePimSparseModeOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_SPARSE_MODE;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}


/* FUNCTION NAME : NETCFG_PMGR_DisablePimSparseModeOnIf
* PURPOSE:
*     Disable Sparse Mode Pim on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisablePimSparseModeOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_SPARSE_MODE;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_EnablePimStateRefreshOnIf
* PURPOSE:
*     Enable state refresh on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnablePimStateRefreshOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_STATE_REFRESH;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    if( TRUE == msg_p->type.result_bool )
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;

}


/* FUNCTION NAME : NETCFG_PMGR_DisablePimStateRefreshOnIf
* PURPOSE:
*     Disable state refresh on some l3 interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisablePimStateRefreshOnIf(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_STATE_REFRESH;
    msg_p->data.ui32_v= ifindex;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimIfStateRefreshOriginalInterval
* PURPOSE:
*     Set pim state refresh origination interval on some l3 interface.
*
* INPUT:
*      ifindex     -- the interface index
*      interval    -- the interval
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimIfStateRefreshOriginalInterval(UI32_T ifindex, UI32_T interval)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp1);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_STATE_REFRESH_INTERVAL;
    msg_p->data.arg_grp1.arg1 = ifindex;
    msg_p->data.arg_grp1.arg2 = interval;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimIfStateRefreshOriginalInterval
* PURPOSE:
*     UnSet pim state refresh original interval on some l3 interface.
*
* INPUT:
*      ifindex  -- the interface index
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimIfStateRefreshOriginalInterval(UI32_T ifindex)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_STATE_REFRESH_INTERVAL;
    msg_p->data.ui32_v = ifindex;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimAcceptReigsterList
* PURPOSE:
*     Set pim accept register list.
*
* INPUT:
*      filter     -- the neighbor filter ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimAcceptReigsterList(UI8_T *filter)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_ACCEPT_REGISTER_LIST;
    msg_p->data.arg_grp2.arg1 = NULL;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(filter) > SYS_ADPT_ACL_MAX_NAME_LEN )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp2.arg2, filter);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimAcceptReigsterList
* PURPOSE:
*     UnSet pim accept register list.
*
* INPUT:
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimAcceptReigsterList(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_ACCEPT_REGISTER_LIST;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_EnablePimCrpPrefix
* PURPOSE:
*     Enable CRP prefix Pim.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnablePimCrpPrefix(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_CRP_PREFIX;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    if( TRUE == msg_p->type.result_bool )
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;

}


/* FUNCTION NAME : NETCFG_PMGR_DisablePimCrpPrefix
* PURPOSE:
*     Disable CRP prefix.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisablePimCrpPrefix(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_CRP_PREFIX;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_PimIgnoreRpSetPriority
* PURPOSE:
*     Ignore RP set priority on some l3 interface.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_PimIgnoreRpSetPriority(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_IGNORE_RP_PRIORITY;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    if( TRUE == msg_p->type.result_bool )
        return NETCFG_TYPE_OK;
    else
        return NETCFG_TYPE_FAIL;

}


/* FUNCTION NAME : NETCFG_PMGR_PimNoIgnoreRpSetPriority
* PURPOSE:
*     Don't ignore RP set priority.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_PimNoIgnoreRpSetPriority(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_NO_IGNORE_RP_PRIORITY;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimJoinPruneInterval
* PURPOSE:
*     Set pim joine/prune interval.
*
* INPUT:
*      interval     -- the interval.
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimJoinPruneInterval(UI32_T interval)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_JOIN_PRUNE_INTERVAL;
    msg_p->data.ui32_v = interval;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimJoinPruneInterval
* PURPOSE:
*     UnSet pim join/prune interval
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimJoinPruneInterval(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_JOIN_PRUNE_INTERVAL;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRegisterRateLimit
* PURPOSE:
*     Set pim register rate limit.
*
* INPUT:
*      limit     -- the register rate limit.
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRegisterRateLimit(UI32_T limit)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_RATE_LIMIT;
    msg_p->data.ui32_v = limit;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRegisterRateLimit
* PURPOSE:
*     UnSet pim register rate limit
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRegisterRateLimit(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_RATE_LIMIT;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRegisterSuppressionTime
* PURPOSE:
*     Set pim register suppression time.
*
* INPUT:
*      limit     -- the register suppression time.
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRegisterSuppressionTime(UI32_T time)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_SUPPRESSION_TIME;
    msg_p->data.ui32_v = time;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRegisterSuppressionTime
* PURPOSE:
*     UnSet pim register suppression time
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRegisterSuppressionTime(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_REGISTER_SUPPRESSION_TIME;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRpAddress
* PURPOSE:
*     Set pim RP address.
*
* INPUT:
*      addr     -- the RP address.
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRpAddress(UI32_T addr)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_RP_ADDRESS;
    msg_p->data.ui32_v = addr;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRpAddress
* PURPOSE:
*     UnSet pim RP address
*
* INPUT:
*      addr  -- the RP address
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRpAddress(UI32_T addr)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_RP_ADDRESS;
    msg_p->data.ui32_v = addr;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRegisterKAT
* PURPOSE:
*     Set pim KAT(keep alive time).
*
* INPUT:
*      time     -- the KAT time.
*      
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRegisterKAT(UI32_T time)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_KAT_TIME;
    msg_p->data.ui32_v = time;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRegisterKAT
* PURPOSE:
*     UnSet pim KAT(keep alive time)
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRegisterKAT(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_PIM_KAT_TIME;
   
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRegisterChecksumGroupList
* PURPOSE:
*     Set pim  register checksum.
*
* INPUT:
*      list     -- the ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRegisterChecksumGroupList(UI8_T *list)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_PIM_REGISTER_CHECKSUM;
    msg_p->data.arg_grp2.arg1 = NULL;
    if ( list == NULL )
    {
        msg_p->data.arg_grp2.arg2[0] = 0;
    }
    else
    {       
        /* check the string length again although it has been checked in CLI */
        if ( strlen(list) > SYS_ADPT_ACL_MAX_NAME_LEN )
            return NETCFG_TYPE_FAIL;
        strcpy(msg_p->data.arg_grp2.arg2, list);
    }
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}
/* FUNCTION NAME : NETCFG_PMGR_DisablePimRegisterChecksum
* PURPOSE:
*     Disable register checksum.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisablePimRegisterChecksum(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_REGISTER_CHECKSUM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}
/* FUNCTION NAME : NETCFG_PMGR_EnablePimRegisterChecksum
* PURPOSE:
*     Enable register checksum.
*
* INPUT:
*      
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnablePimRegisterChecksum(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;
    
    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_REGISTER_CHECKSUM;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRpCandidate
* PURPOSE:
*     Set pim rp candidate.
*
* INPUT:
*      ifName     -- the interface name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRpCandidate(UI8_T *ifName)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimRpCandidateGroupAddr
* PURPOSE:
*     Set pim rp candidate with group address and mask
*
* INPUT:
*      ifname                 -- the interface name
*      groupAddr            -- the group address
*      maskAddr             -- the mask address
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimRpCandidateGroupAddr(UI8_T *ifName,
                                                                UI32_T groupAddr,
                                                                UI32_T maskAddr)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_RP_CANDIDATE_GROUP_ADDR;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    msg_p->data.arg_grp4.arg2 = groupAddr;
    msg_p->data.arg_grp4.arg3 = maskAddr;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRpCandidate
* PURPOSE:
*     Unset pim rp candidate.
*
* INPUT:
*      ifName     -- the interface name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRpCandidate(UI8_T *ifName)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimRpCandidateGroupAddr
* PURPOSE:
*     Unset pim rp candidate.
*
* INPUT:
*      ifName     -- the interface name.
*      groupAddr -- the group address.
*      maskAddr  -- the mask address.
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimRpCandidateGroupAddr( UI8_T *ifName,
                                                                    UI32_T groupAddr, 
                                                                    UI32_T maskAddr )
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_RP_CANDIDATE_GROUP_ADDR;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    msg_p->data.arg_grp4.arg2 = groupAddr;
    msg_p->data.arg_grp4.arg3 = maskAddr;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_EnableCandidateBsr
* PURPOSE:
*     Set bsr candidate.
*
* INPUT:
*      ifName     -- the interface name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_EnableCandidateBsr(UI8_T *ifName)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_ENABLE_BSR_CANDIDATE;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_DisableCandidateBsr
* PURPOSE:
*     Disable bsr candidate.
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_DisableCandidateBsr(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_DISABLE_BSR_CANDIDATE;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetCandidateBsrHash
* PURPOSE:
*     Set bsr candidate hash length.
*
* INPUT:
*      ifName     -- the interface name
*      hash        -- the hash length
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetCandidateBsrHash(UI8_T *ifName, UI32_T hash)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_HASH;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    msg_p->data.arg_grp4.arg2 = hash;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetCandidateBsrPriority
* PURPOSE:
*     Set bsr candidate priority value.
*
* INPUT:
*      ifName     -- the interface name
*      priority     -- the priority value
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetCandidateBsrPriority(UI8_T *ifName, UI32_T priority)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp4);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_BSR_CANDIDATE_PRIORITY;
    /* check the string length again although it has been checked in CLI */
    if ( strlen(ifName) > INTERFACE_NAMSIZ )
        return NETCFG_TYPE_FAIL;
    strcpy(msg_p->data.arg_grp4.arg1, ifName);
    msg_p->data.arg_grp4.arg2 = priority;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_SetPimSptThresholdInfinity
* PURPOSE:
*     Set the SPT threshold infinity value
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimSptThresholdInfinity(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

/* FUNCTION NAME : NETCFG_PMGR_UnSetPimSptThresholdInfinity
* PURPOSE:
*     Unset the SPT threshold infinity.
*
* INPUT:
*      None
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_UnSetPimSptThresholdInfinity(void)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_IPCMSG_TYPE_SIZE;
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_UNSET_SPT_THRESHOLD_INFINITY;
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}
/* FUNCTION NAME : NETCFG_PMGR_SetPimSptThresholdInfinityGroupList
* PURPOSE:
*     Set pim  spt threshold infinity with group list.
*
* INPUT:
*      list     -- the ACL name
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_PMGR_SetPimSptThresholdInfinityGroupList(UI8_T *list)
{
    const UI32_T msg_size = NETCFG_MGR_PIM_GET_MSG_SIZE(arg_grp2);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_MGR_PIM_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PIMCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_MGR_PIM_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_MGR_PIM_IPC_SET_SPT_THRESHOLD_INFINITY_WITH_GROUP_LIST;
    msg_p->data.arg_grp2.arg1 = NULL;
    if ( list == NULL )
    {
        msg_p->data.arg_grp2.arg2[0] = 0;
    }
    else
    {       
        /* check the string length again although it has been checked in CLI */
        if ( strlen(list) > SYS_ADPT_ACL_MAX_NAME_LEN )
            return NETCFG_TYPE_FAIL;
        strcpy(msg_p->data.arg_grp2.arg2, list);
    }
    
    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return NETCFG_TYPE_FAIL;
    }

    return msg_p->type.result_bool;

}

