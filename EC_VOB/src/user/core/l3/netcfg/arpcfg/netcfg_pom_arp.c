/* MODULE NAME:  netcfg_pom_arp.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_OM_ARP service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_POM_XXX for APIs NETCFG_OM_XXX provided by NETCFG, and same as NETCFG_PMGR_ARP for
 *    NETCFG_MGR_ARP APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "netcfg_type.h"
#include "netcfg_om_arp.h"
#include "netcfg_pom_arp.h"


/* STATIC VARIABLE DECLARATIONS
 */
static SYSFUN_MsgQ_T netcfg_om_arp_ipcmsgq_handle;

/* FUNCTION NAME : NETCFG_POM_ARP_InitiateProcessResource
 * PURPOSE:
 *    Initiate resource for CSCA_POM in the calling process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *    Before other CSC use NETCFG_POM_ARP, it should initiate the resource (get the message queue handler internally)
 
 *
 */
BOOL_T NETCFG_POM_ARP_InitiateProcessResource(void)
{
    /* Given that CSCA is run in XXX_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_arp_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : NETCFG_POM_ARP_GetIpNetToMediaTimeout
 * PURPOSE:
 *    Get ARP timeout
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    age_time.
 *
 * RETURN:
 *    TRUE  --  Sucess
 *    FALSE --  Error
 *
 * NOTES:
 *
 */
BOOL_T NETCFG_POM_ARP_GetIpNetToMediaTimeout(UI32_T *age_time)
{
    const UI32_T msg_size = NETCFG_OM_ARP_GET_MSG_SIZE(ui32_v);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_ARP_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_ARPCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_ARP_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_ARP_IPC_GETTIMEOUT;

    if (SYSFUN_SendRequestMsg(netcfg_om_arp_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *age_time = msg_p->data.ui32_v;
    return msg_p->type.result_bool;

}

