/* Module Name:NETCFG_OM_MAIN.C
 * Purpose: .
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *       01/29/2008 --  Vai Wang    Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "netcfg_om_main.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS 
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : NETCFG_OM_MAIN_InitateProcessResources
 * PURPOSE:
 *          
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void NETCFG_OM_MAIN_InitateProcessResources(void)
{
    return;
}


/* FUNCTION NAME : NETCFG_MGR_MAIN_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_MGR_MAIN.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *
 */
BOOL_T NETCFG_OM_MAIN_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
//    ipcmsg_p->msg_size = NETCFG_OM_MAIN_IPCMSG_TYPE_SIZE;
    return TRUE;
}

