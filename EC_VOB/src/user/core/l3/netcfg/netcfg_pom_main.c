/* MODULE NAME:  netcfg_pom_main.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to access NETCFG_OM service.
 *    In Linux platform, the communication between CSC group are done via IPC.
 *    Other CSC can call NETCFG_POM_XXX for APIs NETCFG_OM_XXX provided by NETCFG, and same as NETCFG_PMGR for
 *    NETCFG_MGR APIs
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    07/10/2007 - Max Chen, Created
 *    02/21/2008 - Vai Wang, Modified
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>

#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_cpnt.h"

#include "netcfg_pom_main.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pom_route.h"
#include "netcfg_pom_nd.h"
#include "netcfg_type.h"
#include "l_mm.h"

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
static SYSFUN_MsgQ_T netcfg_om_main_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : NETCFG_POM_MAIN_InitiateProcessResource
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
 *    Before other CSC use NETCFG_POM_XXX, it should initiate the resource (get the message queue handler internally)

 *
 */
BOOL_T NETCFG_POM_MAIN_InitiateProcessResource(void)
{
    /* Given that CSCA is run in XXX_PROC
     */
    if(SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY,SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_main_ipcmsgq_handle)!=SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    NETCFG_POM_IP_InitiateProcessResource();
    NETCFG_POM_ROUTE_InitiateProcessResource();
    NETCFG_POM_ND_InitiateProcessResource();

    return TRUE;
}


