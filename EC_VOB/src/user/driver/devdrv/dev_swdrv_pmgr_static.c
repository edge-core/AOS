#include <stdio.h>
#include <string.h>
#include "dev_swdrv_pmgr.h"
#include "dev_swdrv.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_cpnt.h"

#define DEV_SWDRV_PMGR_CHECK_INIT() do { \
        if (!init) { \
            DEV_SWDRV_PMGR_STATIC_InitiateProcessResource(); \
            init = 1; \
        } \
    } while (0)

#define DEV_SWDRV_PMGR_FUNC_BEGIN(req_sz, rep_sz, cmd_id)   \
    const UI32_T        req_size = req_sz;                  \
    const UI32_T        rep_size = rep_sz;                  \
    UI8_T               ipc_buf[SYSFUN_SIZE_OF_MSG((req_sz>rep_size)?req_sz:rep_size)]; \
    SYSFUN_Msg_T        *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf; \
    DEV_SWDRV_PMGR_IPCMSG_T *ds_msg_p;                      \
                                                            \
    DEV_SWDRV_PMGR_CHECK_INIT();                            \
                                                            \
    msgbuf_p->cmd = SYS_MODULE_DEV_SWDRV;                   \
    msgbuf_p->msg_size = req_size;                          \
    ds_msg_p = (DEV_SWDRV_PMGR_IPCMSG_T *)msgbuf_p->msg_buf;   \
    ds_msg_p->type.cmd = cmd_id;


static SYSFUN_MsgQ_T ipcmsgq_handle;
static BOOL_T init;

/*------------------------------------------------------------------------|
 * ROUTINE NAME - DEV_SWDRV_PMGR_STATIC_InitiateProcessResource
 *------------------------------------------------------------------------|
 * FUNCTION: Initialize for getting the ipc message queue
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *------------------------------------------------------------------------*/
static void DEV_SWDRV_PMGR_STATIC_InitiateProcessResource()
{
    if (SYSFUN_GetMsgQ(SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
    }
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - DEV_SWDRV_PMGR_SetPortStatusForLicense
 * -------------------------------------------------------------------------
 * FUNCTION: To set set port administration status
 * INPUT   : unit
 *           port
 *           status  - TRUE/FALSE
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T DEV_SWDRV_PMGR_SetPortStatusForLicense(UI32_T unit, UI32_T port, BOOL_T status)
{
    DEV_SWDRV_PMGR_FUNC_BEGIN(
        (DEV_SWDRV_PMGR_REQ_CMD_SIZE +
         sizeof(((DEV_SWDRV_PMGR_IPCMSG_T *)0)->data.setportstatusforlicense.req)),
        (DEV_SWDRV_PMGR_REQ_CMD_SIZE +
         sizeof(((DEV_SWDRV_PMGR_IPCMSG_T *)0)->data.setportstatusforlicense.resp)),
        DEV_SWDRV_IPCCMD_SETPORTSTATUSFORLICENSE);

    {
        BOOL_T  ret = FALSE;

        ds_msg_p->data.setportstatusforlicense.req.unit = unit;
        ds_msg_p->data.setportstatusforlicense.req.port = port;
        ds_msg_p->data.setportstatusforlicense.req.status = status;

        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG,
                rep_size, msgbuf_p) == SYSFUN_OK)
        {
            ret = ds_msg_p->type.result_bool;
        }

        return ret;
    }
}

