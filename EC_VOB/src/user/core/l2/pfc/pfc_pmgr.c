/* MODULE NAME: pfc_pmgr.c
 * PURPOSE:
 *   Definitions of MGR IPC APIs for PFC
 *   (Priority-based Flow Control).
 *
 * NOTES:
 *   None
 *
 * HISTORY:
 *   mm/dd/yy (A.D.)
 *   10/15/12    -- Squid Ro, Create
 *
 * Copyright(C)      Accton Corporation, 2012
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"

#if (SYS_CPNT_PFC == TRUE)

#include "sysfun.h"
#include "pfc_pmgr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define PFC_PMGR_FUNC_BEGIN(req_sz, rep_sz, cmd_id)         \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(PFC_TYPE_MAX(req_sz, rep_size))]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    PFC_MGR_IpcMsg_T   *msg_p;                              \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_PFC;                         \
    msgbuf_p->msg_size = req_size;                          \
    msg_p = (PFC_MGR_IpcMsg_T *)msgbuf_p->msg_buf;          \
    msg_p->type.cmd = cmd_id;

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
 * FUNCTION NAME - PFC_PMGR_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for PFC_PMGR in the calling process.
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
BOOL_T PFC_PMGR_InitiateProcessResource(void)
{
    /* get the ipc message queues for PFC MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_DCB_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_PMGR_GetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To get the data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to output data
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTE   : 1. all output data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
BOOL_T PFC_PMGR_GetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    PFC_PMGR_FUNC_BEGIN(
        PFC_MGR_GET_MSG_SIZE(lport_fld_data),
        PFC_MGR_GET_MSG_SIZE(lport_fld_data),
        PFC_MGR_IPC_GET_DATA_BY_FLD);

    {
        BOOL_T  ret = FALSE;

        if (NULL != data_p)
        {
            msg_p->data.lport_fld_data.lport  = lport;
            msg_p->data.lport_fld_data.fld_id = field_id;

            if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                    rep_size, msgbuf_p) == SYSFUN_OK)
            {
                ret = msg_p->type.ret_bool;
                if (TRUE == ret)
                {
                    *((UI32_T *) data_p) = msg_p->data.lport_fld_data.ui32_data;
                }
            }
        }

        return ret;
    }
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PFC_MGR_GetRunningDataByField
 *--------------------------------------------------------------------------
 * PURPOSE: To get running data by specified field id and lport.
 * INPUT  : lport    - which lport to get
 *                       0 - global data to get
 *                      >0 - lport  data to get
 *          field_id - field id to get (PFC_TYPE_FieldId_E)
 * OUTPUT : data_p   - pointer to output data
 * RETURN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE   : 1. all output data are represented in UI32_T
 *--------------------------------------------------------------------------
 */
UI32_T PFC_PMGR_GetRunningDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    PFC_PMGR_FUNC_BEGIN(
        PFC_MGR_GET_MSG_SIZE(lport_fld_data),
        PFC_MGR_GET_MSG_SIZE(lport_fld_data),
        PFC_MGR_IPC_GET_RUNNING_DATA_BY_FLD);

    {
        UI32_T  ret = SYS_TYPE_GET_RUNNING_CFG_FAIL;

        if (NULL != data_p)
        {
            msg_p->data.lport_fld_data.lport  = lport;
            msg_p->data.lport_fld_data.fld_id = field_id;

            if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                    rep_size, msgbuf_p) == SYSFUN_OK)
            {
                ret = msg_p->type.ret_ui32;

                if (SYS_TYPE_GET_RUNNING_CFG_SUCCESS == ret)
                {
                    *((UI32_T *) data_p) = msg_p->data.lport_fld_data.ui32_data;
                }
            }
        }

        return ret;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PFC_PMGR_SetDataByField
 *-------------------------------------------------------------------------
 * PURPOSE: To set the data by specified field id and lport.
 * INPUT  : lport    - which lport to set
 *                      0 - global data to set
 *                     >0 - lport  data to set
 *          field_id - field id to set (PFC_TYPE_FieldId_E)
 *          data_p   - pointer to input data
 * OUTPUT : None
 * RETURN : PFC_TYPE_ReturnCode_E
 * NOTE   : 1. all input data are represented in UI32_T
 *-------------------------------------------------------------------------
 */
UI32_T PFC_PMGR_SetDataByField(
    UI32_T  lport,
    UI32_T  field_id,
    void    *data_p)
{
    PFC_PMGR_FUNC_BEGIN(
        PFC_MGR_GET_MSG_SIZE(lport_fld_data),
        PFC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA(),
        PFC_MGR_IPC_SET_DATA_BY_FLD);

    {
        UI32_T  ret = PFC_TYPE_RCE_UNKNOWN;

        if (NULL != data_p)
        {
            msg_p->data.lport_fld_data.lport  = lport;
            msg_p->data.lport_fld_data.fld_id = field_id;

            switch (field_id)
            {
            default:
                msg_p->data.lport_fld_data.ui32_data = *((UI32_T *) data_p);
                break;
            }

            if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                    SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                    rep_size, msgbuf_p) == SYSFUN_OK)
            {
                ret = msg_p->type.ret_ui32;
            }
        }

        return ret;
    }
}

#endif /* #if (SYS_CPNT_PFC == TRUE) */

