/* FILE NAME: NETCFG_PMGR_PBR.C
 * PURPOSE:
 *    Implements the APIs for IPCs of NETCFG_PMGR_PBR
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2015/07/14     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_cpnt.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "netcfg_mgr_pbr.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */
#define NETCFG_PMGR_PBR_DECLARE_MSG_P(data_type) \
    const UI32_T msg_size = NETCFG_MGR_PBR_GET_MSG_SIZE(data_type);\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    NETCFG_MGR_PBR_IPCMsg_T *msg_p = (NETCFG_MGR_PBR_IPCMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_PBRCFG;\
    msgbuf_p->msg_size = msg_size;

#define NETCFG_PMGR_PBR_DECLARE_MSG_VOID_P() \
    const UI32_T msg_size = NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE;\
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];\
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;\
    NETCFG_MGR_PBR_IPCMsg_T *msg_p = (NETCFG_MGR_PBR_IPCMsg_T*)msgbuf_p->msg_buf;\
    msgbuf_p->cmd = SYS_MODULE_PBRCFG;\
    msgbuf_p->msg_size = msg_size;

#define NETCFG_PMGR_PBR_SEND_WAIT_MSG_P() \
do{\
    if(SYSFUN_OK!=SYSFUN_SendRequestMsg(pbr_ipcmsgq_handle, msgbuf_p, SYSFUN_TIMEOUT_WAIT_FOREVER,\
        SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size , msgbuf_p))\
    {\
        SYSFUN_Debug_Printf("%s():SYSFUN_SendRequestMsg fail\n", __FUNCTION__);\
        return NETCFG_TYPE_FAIL;\
    }\
}while(0)


/* STATIC VARIABLE DEFINITIONS
 */
static SYSFUN_MsgQ_T pbr_ipcmsgq_handle;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: NETCFG_PMGR_PBR_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for PBR_PMGR in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T NETCFG_PMGR_PBR_InitiateProcessResources(void)
{
    /* get the ipc message queues for pbr MGR
     */
    if (SYSFUN_GetMsgQ(SYS_BLD_NETCFG_GROUP_IPCMSGQ_KEY,
                        SYSFUN_MSGQ_BIDIRECTIONAL, &pbr_ipcmsgq_handle) != SYSFUN_OK)
    {
        printf("\n%s(): SYSFUN_GetMsgQ fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_PMGR_PBR_BindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  binding vlan with route-map
 * INPUT:    vid       --  VLAN ID
 *           rmap_name --  route-map name
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    
 * -------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_PBR_BindRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1])
{
    NETCFG_PMGR_PBR_DECLARE_MSG_P(arg_grp_bind_routemap);

    msg_p->type.cmd = NETCFG_MGR_PBR_IPCCMD_BIND_ROUTE_MAP;

    msg_p->data.arg_grp_bind_routemap.vid = vid;
    memcpy(msg_p->data.arg_grp_bind_routemap.rmap_name, rmap_name, sizeof(msg_p->data.arg_grp_bind_routemap.rmap_name));

    NETCFG_PMGR_PBR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_UnbindRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  unbinding vlan with route-map
 * INPUT:    vid     --  VLAN ID
 * OUTPUT:   none.
 * RETURN:   success - NETCFG_TYPE_OK
 *           fail    - Error code
 * NOTES:    
 * -------------------------------------------------------------------------
 */
UI32_T NETCFG_PMGR_PBR_UnbindRouteMap(UI32_T vid)
{
    NETCFG_PMGR_PBR_DECLARE_MSG_P(vid);

    msg_p->type.cmd = NETCFG_MGR_PBR_IPCCMD_UNBIND_ROUTE_MAP;
    msg_p->data.vid = vid;

    NETCFG_PMGR_PBR_SEND_WAIT_MSG_P();
    
    return msg_p->type.result_ui32;
}

