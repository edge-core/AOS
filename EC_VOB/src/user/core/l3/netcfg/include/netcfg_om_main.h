/* Module Name: NETCFG_OM_MAIN.H
 * Purpose: 
 *
 * Notes:
 *      1. 
 *
 *
 * History:
 *       Date       -- 	Modifier,  	Reason
 *      01/29/2008  --  Vai Wang,   Created
 *
 * Copyright(C)      Accton Corporation, 2008.
 */

#ifndef NETCFG_OM_MAIN_H
#define NETCFG_OM_MAIN_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
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
void NETCFG_OM_MAIN_InitateProcessResources(void);


/* FUNCTION NAME : NETCFG_OM_MAIN_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for NETCFG_OM_MAIN.
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
BOOL_T NETCFG_OM_MAIN_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /* NETCFG_OM_MAIN_H */

