/* MODULE NAME:  amtr_pom.c
 * PURPOSE:
 *     For implementations of AMTR POM APIs.
 *
 * NOTES:
 *     N/A
 *
 * HISTORY
 *    5/3/2010 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2010
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "amtr_pom.h"
#include "amtr_om.h"
#include "swctrl.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */
/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AMTR_POM_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AMTR_POM in the calling process.
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
BOOL_T AMTR_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for AMTR POM
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;

}

/*-----------------------------------------------------------------------------
 * Function : AMTR_POM_GetPortInfo
 *-----------------------------------------------------------------------------
 * Purpose  : This function get the port Infomation
 * INPUT    : ifindex
 * OUTPUT   : port_info(learning mode, life time, count, protocol)
 * RETURN   : BOOL_T - True : successs, False : failed
 * NOTE     : 
 *-----------------------------------------------------------------------------
 */
BOOL_T  AMTR_POM_GetPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info)
{
    /* message buffer is used for both request and response
     * its size must be max(buffer for request, buffer for response)
     */
    const UI32_T res_msg_size = AMTR_OM_GET_MSG_SIZE(arg_grp_ui32_portinfo);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(res_msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    AMTR_OM_IpcMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_AMTR;
    msgbuf_p->msg_size = AMTR_OM_IPCMSG_TYPE_SIZE + 4;

    msg_p = (AMTR_OM_IpcMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = AMTR_OM_IPC_GETPORTINFO;
    msg_p->data.arg_grp_ui32_portinfo.arg_ui32 = ifindex;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, res_msg_size,
            msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *port_info = msg_p->data.arg_grp_ui32_portinfo.arg_portinfo;

    return msg_p->type.ret_bool;
} /* End of AMTR_POM_GetPortInfo */

/* LOCAL SUBPROGRAM BODIES
 */
