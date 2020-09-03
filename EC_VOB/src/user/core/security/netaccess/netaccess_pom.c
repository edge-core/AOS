/* MODULE NAME : netaccess_pom.c
 * PURPOSE  :
 *   This file implements the APIs for NETACCESS OM IPC.
 * NOTES    :
 *
 * HISTORY  :
 *   mm/dd/yy (A.D.)
 *    6/23/2008 - Squid Ro, Created
 *
 *
 * Copyright(C)     Accton Corporation, 2008
 */

/* INCLUDE FILE DECLARATIONS
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)
#include "sys_module.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "netaccess_om.h"

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
/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - NETACCESS_POM_InitiateProcessResource
 * ------------------------------------------------------------------------
 *  PURPOSE: Initiate resource for NETACCESS POM in the calling process.
 *  INPUT:   None.
 *  OUTPUT:  None.
 *  RETURN:  None.
 *  NOTE:    None.
 * ------------------------------------------------------------------------
 */
BOOL_T NETACCESS_POM_InitiateProcessResource(void)
{
    /* get the ipc message queues for DOT1X SUP OM
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_POM_IsSecurityPort
 * ---------------------------------------------------------------------
 * PURPOSE: Check if security related function is enabled on lport
 * INPUT:   lport.
 * OUTPUT:  is_enabled.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T NETACCESS_POM_IsSecurityPort(UI32_T lport, BOOL_T *is_enabled)
{
    const UI32_T            msg_size = NETACCESS_OM_GET_MSG_SIZE(lport_bdata);
    SYSFUN_Msg_T            *msg_p;
    NETACCESS_OM_IpcMsg_T   *msg_data_p;
    UI8_T                   space_msg[SYSFUN_SIZE_OF_MSG(msg_size)];

    msg_p=(SYSFUN_Msg_T *) &space_msg;
    msg_p->cmd = SYS_MODULE_NETACCESS;
    msg_p->msg_size = msg_size;

    msg_data_p=(NETACCESS_OM_IpcMsg_T *)msg_p->msg_buf;
    msg_data_p->type.cmd = NETACCESS_OM_IPC_IS_SECURITY_PORT;

    /*assign input
     */
    msg_data_p->data.lport_bdata.lport=lport;

    /*send ipc
     */
    if(SYSFUN_SendRequestMsg(ipcmsgq_handle, msg_p, SYSFUN_TIMEOUT_WAIT_FOREVER,
        0,msg_size,msg_p)!=SYSFUN_OK)
    {
        return FALSE;
    }

    /*assign output
     */
    *is_enabled=msg_data_p->data.lport_bdata.bdata;

    /*return result
     */
    return msg_data_p->type.ret_bool;
}

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
