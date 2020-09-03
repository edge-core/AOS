/* ------------------------------------------------------------------------
 * FILE NAME - DOS_PMGR.C
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
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_DOS == TRUE)
#include "dos_type.h"
#include "dos_om.h"
#include "dos_pmgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */

/* TYPE DEFINITIONS
 */

/* MACRO DEFINITIONS
 */
#define DOS_PMGR_MAX(a, b) (((a) > (b)) ? (a) : (b))

#define DOS_PMGR_IPC_BUF_DECL(req_sz, rep_sz)               \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG(DOS_PMGR_MAX(req_size, rep_size))];  \
    SYSFUN_Msg_T        *msgbuf_p = (void *)&ipc_buf;       \
    DOS_MGR_IpcMsg_T    *msg_p = (DOS_MGR_IpcMsg_T *)msgbuf_p->msg_buf

#define DOS_PMGR_IPC_RETVAL     \
    DOS_MGR_MSG_RETVAL(msgbuf_p)

#define DOS_PMGR_IPC_SEND_MSG(cmd_id, ret_val)                      \
    do {                                                            \
        msgbuf_p->cmd = SYS_MODULE_DOS;                             \
        msgbuf_p->msg_size = req_size;                              \
        DOS_MGR_MSG_CMD(msgbuf_p) = (cmd_id);                       \
                                                                    \
        if (SYSFUN_OK != SYSFUN_SendRequestMsg(ipcmsgq_handle,      \
                                    msgbuf_p,                       \
                                    SYSFUN_TIMEOUT_WAIT_FOREVER,    \
                                    SYSFUN_SYSTEM_EVENT_IPCMSG,     \
                                    rep_size,                       \
                                    msgbuf_p))                      \
            DOS_PMGR_IPC_RETVAL = ret_val;                          \
    } while (0)

/* LOCAL FUNCTIONS DECLARATIONS
 */

/* LOCAL VARIABLES DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

#if LOCAL_TEST
static DOS_MGR_IpcMsg_T *msg_p;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate resource used in the calling process.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE/FALSE
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T DOS_PMGR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(DOS_TYPE_IPCMSG_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_SetDataByField
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
DOS_TYPE_Error_T DOS_PMGR_SetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_PMGR_IPC_BUF_DECL(
        DOS_MGR_GET_MSG_SIZE(fld_data),
        DOS_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA());

    msg_p->data.fld_data.field_id = field_id;

    if (data_p == NULL)
    {
        msg_p->data.fld_data.use_dflt = TRUE;
    }
    else
    {
        msg_p->data.fld_data.use_dflt = FALSE;

        switch (DOS_OM_GetDataTypeByField(field_id))
        {
            case DOS_TYPE_FTYPE_UI32:
                msg_p->data.fld_data.data.u32 = *(UI32_T *)data_p;
                break;
            default:
                return DOS_TYPE_E_UNKNOWN;
        }
    }

    DOS_PMGR_IPC_SEND_MSG(
        DOS_MGR_IPC_SET_DATA_BY_FIELD,
        DOS_TYPE_E_UNKNOWN);

    return DOS_PMGR_IPC_RETVAL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_GetDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : DOS_TYPE_Error_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
DOS_TYPE_Error_T DOS_PMGR_GetDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_PMGR_IPC_BUF_DECL(
        DOS_MGR_GET_MSG_SIZE(fld_data),
        DOS_MGR_GET_MSG_SIZE(fld_data));

    if (data_p == NULL)
    {
        return DOS_TYPE_E_INVALID;
    }

    msg_p->data.fld_data.field_id = field_id;

    DOS_PMGR_IPC_SEND_MSG(
        DOS_MGR_IPC_GET_DATA_BY_FIELD,
        DOS_TYPE_E_UNKNOWN);

    if (DOS_PMGR_IPC_RETVAL == DOS_TYPE_E_OK)
    {
        switch (DOS_OM_GetDataTypeByField(field_id))
        {
            case DOS_TYPE_FTYPE_UI32:
                *(UI32_T *)data_p = msg_p->data.fld_data.data.u32;
                break;
            default:
                DOS_PMGR_IPC_RETVAL = DOS_TYPE_E_UNKNOWN;
        }
    }

    return DOS_PMGR_IPC_RETVAL;
}

/*-----------------------------------------------------------------------------
 * ROUTINE NAME: DOS_PMGR_GetRunningDataByField
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get data by field
 * INPUT  : field_id - DOS_TYPE_FieldId_T
 *          data_p   -- pointer to buffer that contains value of specified field
 *                      use can also use DOS_TYPE_FieldDataBuf_T as data buffer
 * RETURN : SYS_TYPE_Get_Running_Cfg_T
 * NOTES  : None.
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T DOS_PMGR_GetRunningDataByField(DOS_TYPE_FieldId_T field_id, void *data_p)
{
    DOS_PMGR_IPC_BUF_DECL(
        DOS_MGR_GET_MSG_SIZE(fld_data),
        DOS_MGR_GET_MSG_SIZE(fld_data));

    if (data_p == NULL)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    msg_p->data.fld_data.field_id = field_id;

    DOS_PMGR_IPC_SEND_MSG(
        DOS_MGR_IPC_GET_RUNNING_DATA_BY_FIELD,
        SYS_TYPE_GET_RUNNING_CFG_FAIL);

    if (DOS_PMGR_IPC_RETVAL != SYS_TYPE_GET_RUNNING_CFG_FAIL)
    {
        switch (DOS_OM_GetDataTypeByField(field_id))
        {
            case DOS_TYPE_FTYPE_UI32:
                *(UI32_T *)data_p = msg_p->data.fld_data.data.u32;
                break;
            default:
                DOS_PMGR_IPC_RETVAL = SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }
    }

    return DOS_PMGR_IPC_RETVAL;
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

#endif /* (SYS_CPNT_DOS == TRUE) */



