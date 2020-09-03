/*-----------------------------------------------------------------------------
 * FILE NAME: PPPOE_IA_PMGR.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for PPPOE_IA MGR IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2009/11/26     --- Squid Ro, Create for Linux platform
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "pppoe_ia_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DEFINITIONS
 */

static SYSFUN_MsgQ_T ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : PPPOE_IA_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for PPPOE_IA_PMGR in the calling process.
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
BOOL_T PPPOE_IA_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for PPPOE_IA MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_PPPOE_IA_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of PPPOE_IA_PMGR_InitiateProcessResource */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_ClearPortStatistics
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the statistics entry for the specified ifindex.
 * INPUT  : lport - 1-based ifindex to clear
 *                  (0 to clear all)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_ClearPortStatistics(
    UI32_T  lport)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(ui32_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd         = PPPOE_IA_MGR_IPC_CLEAR_PORT_STATISTICS;
    msg_p->data.ui32_data   = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetAccessNodeId
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global access node id. (operation string)
 * INPUT  : is_oper   - TRUE to get operation access node id
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : a. access node id may be
 *           1. user configured id
 *           2. first ip address if 1 is not available
 *           3. cpu mac if 1 & 2 are not available
 *          b. set API - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetAccessNodeId(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_ACCESS_NODE_ID_LEN+1])
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_ACCESS_NODE_ID;
    msg_p->data.bool_str_data.bool_data = is_oper;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        strcpy((char *)outbuf_ar, (char *)msg_p->data.bool_str_data.str_data);
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetGenericErrMsg
 *-------------------------------------------------------------------------
 * PURPOSE: To get the global generic error message. (operation string)
 * INPUT  : is_oper   - TRUE to get operation generic error message
 * OUTPUT : outbuf_ar - pointer to output buffer
 *                      >= PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1
 * RETURN : TRUE/FALSE
 * NOTE   : 1. set API - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetGenericErrMsg(
    BOOL_T  is_oper,
    UI8_T   outbuf_ar[PPPOE_IA_TYPE_MAX_GENERIC_ERMSG_LEN+1])
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_str_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_GENERIC_ERR_MSG;
    msg_p->data.bool_str_data.bool_data = is_oper;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        strcpy((char *)outbuf_ar, (char *)msg_p->data.bool_str_data.str_data);
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To get global enable status
 * INPUT  : None
 * OUTPUT : is_enable_p - pointer to output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetGlobalEnable(
    BOOL_T  *is_enable_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_GLOBAL_ENABLE;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *is_enable_p = msg_p->data.bool_data;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetNextPortConfigEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port config entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          pcfg_p  - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetNextPortConfigEntry(
    UI32_T                          *lport_p,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_NEXT_PORT_OPRCFG_ENTRY;
    msg_p->data.lport_cfg_data.lport = *lport_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *lport_p = msg_p->data.lport_cfg_data.lport;
        *pcfg_p  = msg_p->data.lport_cfg_data.pcfg_entry;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetNextPortStatisticsEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get next PPPoE IA port statistic entry
 * INPUT  : lport_p - pointer to lport used to get next
 * OUTPUT : lport_p - pointer to lport got
 *          psts_p  - pointer to statistic entry got
 * RETURN : TRUE/FALSE
 * NOTE   : None
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetNextPortStatisticsEntry(
    UI32_T                      *lport_p,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_NEXT_PORT_STATS_ENTRY;
    msg_p->data.lport_sts_data.lport = *lport_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *lport_p = msg_p->data.lport_sts_data.lport;
        *psts_p  = msg_p->data.lport_sts_data.psts_entry;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get boolean data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  *val_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_PORT_BOOL_DATA_BY_FLD;
    msg_p->data.lport_fld_bool_data.lport = lport;
    msg_p->data.lport_fld_bool_data.fld_id= fld_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *val_p = msg_p->data.lport_fld_bool_data.bool_data;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get ui32 data for specified ifindex and field id.
 * INPUT  : lport  - 1-based ifindex to get
 *          fld_id - PPPOE_IA_TYPE_FLDID_E
 *                   field id to get
 * OUTPUT : val_p  - pointer to output value
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  *val_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_PORT_UI32_DATA_BY_FLD;
    msg_p->data.lport_fld_ui32_data.lport = lport;
    msg_p->data.lport_fld_ui32_data.fld_id= fld_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *val_p = msg_p->data.lport_fld_ui32_data.ui32_data;
    }

    return msg_p->type.ret_bool;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortOprCfgEntry
 *-------------------------------------------------------------------------
 * PURPOSE: To get the PPPoE IA port oper config entry for specified lport.
 * INPUT  : lport  - lport to get
 * OUTPUT : pcfg_p - pointer to config entry got
 * RETURN : TRUE/FALSE
 * NOTE   : 1. will get the operation string for the port
 *-------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortOprCfgEntry(
    UI32_T                          lport,
    PPPOE_IA_OM_PortOprCfgEntry_T   *pcfg_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_cfg_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_PORT_OPRCFG_ENTRY;
    msg_p->data.lport_cfg_data.lport = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *pcfg_p = msg_p->data.lport_cfg_data.pcfg_entry;
    }

    return msg_p->type.ret_bool;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortStatisticsEntry
 *--------------------------------------------------------------------------
 * PURPOSE: To get the port statistics entry for specified lport.
 * INPUT  : lport      - 1-based lport
 * OUTPUT : psts_ent_p - pointer to the output buffer
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortStatisticsEntry(
    UI32_T                      lport,
    PPPOE_IA_OM_PortStsEntry_T  *psts_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_sts_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_PORT_STATS_ENTRY;
    msg_p->data.lport_sts_data.lport = lport;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *psts_p = msg_p->data.lport_sts_data.psts_entry;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To get string data for specified ifindex and field id.
 * INPUT  : lport     - 1-based ifindex to get
 *          fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to get
 *          is_oper   - TRUE to get operation string
 *          str_len_p - length of input buffer
 *                      (including null terminator)
 * OUTPUT : str_p     - pointer to output string data
 *          str_len_p - length of output buffer used
 *                      (not including null terminator)
 * RETURN : TRUE/FALSE
 * NOTES  : 1. set API - PPPOE_IA_PMGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_GetPortStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  is_oper,
    I32_T   *str_len_p,
    UI8_T   *str_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_PORT_STR_DATA_BY_FLD;
    msg_p->data.lport_fld_str_data.lport        = lport;
    msg_p->data.lport_fld_str_data.fld_id       = fld_id;
    msg_p->data.lport_fld_str_data.str_len_max  = *str_len_p;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    if (TRUE == msg_p->type.ret_bool)
    {
        *str_len_p = msg_p->data.lport_fld_str_data.str_len_max;
        memcpy(str_p, msg_p->data.lport_fld_str_data.str_data, *str_len_p);
    }

    return msg_p->type.ret_bool;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningBoolDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running bool data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : bool_flag_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningBoolDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    BOOL_T  *bool_flag_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_RUNNING_BOOL_DATA_BY_FLD;
    msg_p->data.lport_fld_bool_data.lport = lport;
    msg_p->data.lport_fld_bool_data.fld_id= field_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (TRUE == msg_p->type.ret_ui32)
    {
        *bool_flag_p = msg_p->data.lport_fld_bool_data.bool_data;
    }

    return msg_p->type.ret_ui32;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningUi32DataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running ui32 data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 * OUTPUT : ui32_data_p - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningUi32DataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  *ui32_data_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_RUNNING_UI32_DATA_BY_FLD;
    msg_p->data.lport_fld_ui32_data.lport = lport;
    msg_p->data.lport_fld_ui32_data.fld_id= field_id;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (TRUE == msg_p->type.ret_ui32)
    {
        *ui32_data_p = msg_p->data.lport_fld_ui32_data.ui32_data;
    }

    return msg_p->type.ret_ui32;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_GetRunningStrDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running string data by field id from port or global config entry.
 * INPUT  : lport       - lport
 *                         0, get from the global config entry
 *                        >0, get from the port config entry
 *          field_id    - PPPOE_IA_TYPE_FLDID_E
 *          str_len_max - maximum length of buffer to receive the string
 *                        (including null terminator)
 * OUTPUT : string_p    - pointer to content of output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
UI32_T PPPOE_IA_PMGR_GetRunningStrDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    UI32_T  str_len_max,
    UI8_T   *string_p)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_GET_RUNNING_STR_DATA_BY_FLD;
    msg_p->data.lport_fld_str_data.lport        = lport;
    msg_p->data.lport_fld_str_data.fld_id       = field_id;
    msg_p->data.lport_fld_str_data.str_len_max  = str_len_max;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (TRUE == msg_p->type.ret_ui32)
    {
        memcpy(string_p, msg_p->data.lport_fld_str_data.str_data, str_len_max);
    }

    return msg_p->type.ret_ui32;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetGlobalEnable
 * ------------------------------------------------------------------------
 * PURPOSE: To enable or disable globally.
 * INPUT  : is_enable - TRUE to enable
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetGlobalEnable
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetGlobalEnable(
    BOOL_T  is_enable)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(bool_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd         = PPPOE_IA_MGR_IPC_SET_GLOBAL_ENABLE;
    msg_p->data.bool_data   = is_enable;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetGlobalAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set global string data for specified field id.
 * INPUT  : fld_id    - PPPOE_IA_TYPE_FLDID_E
 *                      field id to set
 *          str_p     - pointer to input string data
 *          str_len   - length of input string data
 *                      (not including null terminator, 0 to reset to default)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetAccessNodeId/
 *                       PPPOE_IA_PMGR_GetGenericErrMsg
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetGlobalAdmStrDataByField(
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_SET_GLOBAL_ADM_STR_DATA_BY_FLD;
    msg_p->data.lport_fld_str_data.lport        = 0;
    msg_p->data.lport_fld_str_data.fld_id       = fld_id;
    msg_p->data.lport_fld_str_data.str_len_max  = str_len;
    if (NULL != str_p)
        memcpy(msg_p->data.lport_fld_str_data.str_data, str_p, str_len+1);
    else
        msg_p->data.lport_fld_str_data.str_data[0] = '\0';

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortBoolDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set boolean data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetPortBoolDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortBoolDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    BOOL_T  new_val)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_bool_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_SET_PORT_BOOL_DATA_BY_FLD;
    msg_p->data.lport_fld_bool_data.lport    = lport;
    msg_p->data.lport_fld_bool_data.fld_id   = fld_id;
    msg_p->data.lport_fld_bool_data.bool_data= new_val;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortUi32DataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set ui32 data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          new_val - new value to set
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. get API - PPPOE_IA_PMGR_GetPortUi32DataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortUi32DataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI32_T  new_val)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_ui32_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_SET_PORT_UI32_DATA_BY_FLD;
    msg_p->data.lport_fld_ui32_data.lport    = lport;
    msg_p->data.lport_fld_ui32_data.fld_id   = fld_id;
    msg_p->data.lport_fld_ui32_data.ui32_data= new_val;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}
/* ------------------------------------------------------------------------
 * FUNCTION NAME - PPPOE_IA_PMGR_SetPortAdmStrDataByField
 * ------------------------------------------------------------------------
 * PURPOSE: To set string data for specified ifindex and field id.
 * INPUT  : lport   - 1-based ifindex to set
 *          fld_id  - PPPOE_IA_TYPE_FLDID_E
 *                    field id to set
 *          str_p   - pointer to input string data
 *          str_len - length of input string data
 *                    (not including null terminator)
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : 1. use engine API to apply setting on trunk.
 *          2. get API - PPPOE_IA_PMGR_GetPortStrDataByField
 * ------------------------------------------------------------------------
 */
BOOL_T PPPOE_IA_PMGR_SetPortAdmStrDataByField(
    UI32_T  lport,
    UI32_T  fld_id,
    UI8_T   *str_p,
    UI32_T  str_len)
{
    const UI32_T            req_size = PPPOE_IA_MGR_GET_MSG_SIZE(lport_fld_str_data);
    const UI32_T            rep_size = PPPOE_IA_MGR_IPCMSG_TYPE_SIZE;
    UI8_T                   ipc_buf[SYSFUN_SIZE_OF_MSG(PPPOE_IA_MAX(rep_size, req_size))];
    SYSFUN_Msg_T            *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    PPPOE_IA_MGR_IpcMsg_T   *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PPPOE_IA;
    msgbuf_p->msg_size = req_size;

    msg_p = (PPPOE_IA_MGR_IpcMsg_T *)msgbuf_p->msg_buf;
    msg_p->type.cmd = PPPOE_IA_MGR_IPC_SET_PORT_ADM_STR_DATA_BY_FLD;
    msg_p->data.lport_fld_str_data.lport        = lport;
    msg_p->data.lport_fld_str_data.fld_id       = fld_id;
    msg_p->data.lport_fld_str_data.str_len_max  = str_len;
    if (NULL != str_p)
        memcpy(msg_p->data.lport_fld_str_data.str_data, str_p, str_len+1);
    else
        msg_p->data.lport_fld_str_data.str_data[0] = '\0';

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
            rep_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    return msg_p->type.ret_bool;
}


