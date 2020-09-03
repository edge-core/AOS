/*-----------------------------------------------------------------------------
 * FILE NAME: PSEC_POM.C
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file implements the APIs for PSEC OM IPC.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    20015/02/09     --- Jerry, Create
 *
 * Copyright(C)      Accton Corporation, 2015
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
#include "psec_om.h"
#include "psec_pom.h"

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
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_POM_InitiateProcessResource
 * ---------------------------------------------------------------------
 * PURPOSE  : Initiate resource for PSEC_POM in the calling process.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETURN   : TRUE, success; else return FALSE;
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_POM_InitiateProcessResource(void)
{
    /* Given that CSCA is run in L2_L4_PROC
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
} /* End of VLAN_POM_InitiateProcessResource */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_POM_GetMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : This api will call PSEC_OM_GetMaxMacCount through the IPC msgq.
 * INPUT    : lport           -- Logic port
 * OUTPUT   : mac_count       -- Max MAC count
 * RETURN   : TRUE, success; else return FALSE;
 * NOTES    : None.
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_POM_GetMaxMacCount(UI32_T lport, UI32_T *mac_count)
{
    UI8_T         msgbuf[SYSFUN_SIZE_OF_MSG(PSEC_OM_GET_MSGBUFSIZE(PSEC_OM_IPCMsg_GetPortSecurityMacCount_T))];
    SYSFUN_Msg_T  *msg_p = (SYSFUN_Msg_T *)msgbuf;
    PSEC_OM_IPCMsg_GetPortSecurityMacCount_T *data_p;

    if (mac_count == NULL)
        return FALSE;

    data_p = PSEC_OM_MSG_DATA(msg_p);
    data_p->ifindex = lport;

    msg_p->cmd = SYS_MODULE_PSEC;
    msg_p->msg_size = PSEC_OM_GET_MSGBUFSIZE(PSEC_OM_IPCMsg_GetPortSecurityMacCount_T);

    PSEC_OM_MSG_CMD(msg_p) = PSEC_OM_IPC_GET_MAX_MAC_COUNT;

    if (SYSFUN_SendRequestMsg(ipcmsgq_handle,
                              msg_p,
                              SYSFUN_TIMEOUT_WAIT_FOREVER,
                              SYSFUN_SYSTEM_EVENT_IPCMSG,
                              PSEC_OM_GET_MSGBUFSIZE(PSEC_OM_IPCMsg_GetPortSecurityMacCount_T),
                              msg_p) != SYSFUN_OK)
    {
        return FALSE;
    }

    *mac_count = data_p->mac_count;

    return (BOOL_T) PSEC_OM_MSG_RETVAL(msg_p);
} /* End of PSEC_POM_GetMaxMacCount */

