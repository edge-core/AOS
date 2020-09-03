/* MODULE NAME:  netcfg_pom_pbr.c
 * PURPOSE:
 *    This file provides APIs for other process or CSC group to 
 *    access NETCFG_OM_PBR service.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY:
 *    2015/07/16     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sysfun.h"
#include "sys_bld.h"
#include "netcfg_pom_pbr.h"
#include "netcfg_om_pbr.h"
#include "netcfg_type.h"
#include "sys_module.h"
#include "sys_dflt.h"

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
static SYSFUN_MsgQ_T netcfg_om_pbr_ipcmsgq_handle;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : NETCFG_POM_PBR_InitiateProcessResource
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
 *    Before other CSC use NETCFG_POM_PBR, it should initiate the resource (get the message queue handler internally)
 *
 */
BOOL_T NETCFG_POM_PBR_InitiateProcessResources(void)
{
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &netcfg_om_pbr_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_POM_PBR_GetNextBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_POM_PBR_GetNextBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p)
{
    const UI32_T msg_size = NETCFG_OM_PBR_GET_MSG_SIZE(binding_entry);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_PBR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PBRCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_PBR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_PBR_IPCCMD_GETNEXTBINDINGENTRY;
    
    memcpy(&msg_p->data.binding_entry, binding_entry_p, sizeof(msg_p->data.binding_entry));
    
    if (SYSFUN_SendRequestMsg(netcfg_om_pbr_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return FALSE;
    }
    
    memcpy(binding_entry_p, &msg_p->data.binding_entry, sizeof(msg_p->data.binding_entry));
    return msg_p->type.result_bool;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetRunningBindingRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Get the binding route-map for a vlan
 * INPUT:    vid       -- vlan id
 * OUTPUT:   rmap_name -- bing route-map name
 * RETURN:   
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE 
 * NOTES:    
 * -------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T NETCFG_POM_PBR_GetRunningBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    const UI32_T msg_size = NETCFG_OM_PBR_GET_MSG_SIZE(binding_rmap);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    NETCFG_OM_PBR_IPCMsg_T *msg_p;

    msgbuf_p->cmd = SYS_MODULE_PBRCFG;
    msgbuf_p->msg_size = msg_size;

    msg_p = (NETCFG_OM_PBR_IPCMsg_T*)msgbuf_p->msg_buf;
    msg_p->type.cmd = NETCFG_OM_PBR_IPCCMD_GETRUNNINGBINDINGROUTEMAP;
    
    msg_p->data.binding_rmap.vid = vid;
    
    if (SYSFUN_SendRequestMsg(netcfg_om_pbr_ipcmsgq_handle, msgbuf_p,
            SYSFUN_TIMEOUT_WAIT_FOREVER, 0,
            msg_size, msgbuf_p) != SYSFUN_OK)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (msg_p->type.result_running_cfg == SYS_TYPE_GET_RUNNING_CFG_SUCCESS)
        strncpy(rmap_name, msg_p->data.binding_rmap.rmap_name, SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1);

    return msg_p->type.result_running_cfg;
}

