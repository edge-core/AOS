/* MODULE NAME:  netcfg_mgr_pbr.h
 * PURPOSE:
 *     NETCFG_MGR_PBR APIs.
 *
 * NOTES:
 *
 * HISTORY:
 *    2015/07/14     KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef _NETCFG_MGR_PBR_H
#define _NETCFG_MGR_PBR_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "sys_type.h"
#include "sysfun.h"
#include "amtrl3_type.h"

#define NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_PBR_IPCMsg_Type_U)

#define NETCFG_MGR_PBR_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_PBR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_PBR_IPCMsg_T *)0)->data.field_name))

/* netcfg_mgr_pbr ipc request command definitions
 */
enum
{
    NETCFG_MGR_PBR_IPCCMD_BIND_ROUTE_MAP,
    NETCFG_MGR_PBR_IPCCMD_UNBIND_ROUTE_MAP,
};

/* netcfg_mgr_pbr ipc msg structure
 */
typedef struct NETCFG_MGR_PBR_IPCMsg_S
{
    union NETCFG_MGR_PBR_IPCMsg_Type_U
    {
        UI32_T cmd;             /* for sending IPC request command */
        BOOL_T result_bool;     /*respond bool return*/
        UI32_T result_ui32;     /*respond ui32 return*/
    } type;

    union
    {
        UI32_T vid;
        struct
        {
            UI32_T vid;
            char  rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1];
        } arg_grp_bind_routemap;
        struct
        {
            UI32_T ifindex;
            L_INET_AddrIp_T addr;
        } arg_grp_signal_rif;
    } data;
} NETCFG_MGR_PBR_IPCMsg_T;

/* FUNCTION NAME : NETCFG_MGR_PBR_HandleIPCReqMsg
 * PURPOSE:
 *      Handle the ipc request received from mgr queue.
 *
 * INPUT:
 *      sysfun_msg_p  --  The ipc request for NETCFG_MGR_PBR.
 *
 * OUTPUT:
 *      sysfun_msg_p  --  The ipc response to send when return value is TRUE
 *
 * RETURN:
 *      TRUE   --  A response is required to send
 *      FALSE  --  Need not to send response.
 *
 * NOTES:
 *      1. The buffer length in sysfun_msg_p must be large enough for sending
 *         all possible response messages.
 */
BOOL_T NETCFG_MGR_PBR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ERPS_MGR_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: This function sets the component to temporary transition mode.
 * INPUT  : none
 * OUTPUT : none
 * RETURN : none
 * NOTES  : none
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE: It's the temporary transition mode between system into master
 *          mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE: Enable PBR operation while in master mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE: Disable the PBR operation while in slave mode.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE: This function will initialize system resouce for PBR.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_InitiateSystemResources(void);

/*------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_PBR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : None
 *------------------------------------------------------------------------*/
void NETCFG_MGR_PBR_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_HostRouteChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback function when host route changed
 * INPUT:    addr_p         -- the host route ip address
 *           is_unresolved  -- is host route unresolved
 * OUTPUT:    
 * RETURN:   
 * NOTES:    
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_HostRouteChangedCallbackHandler(L_INET_AddrIp_T *addr_p, BOOL_T is_unresolved);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_AclChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback handler function for acl change
 * INPUT:    acl_index -- acl index
 *           acl_name  -- acl name
 *           type      -- change type (ACL_CHANGE_TYPE_ADD/DELETE/MODIFY)
 * OUTPUT:    
 * RETURN:   
 * NOTES:    
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_AclChangedCallbackHandler(UI32_T acl_index, char acl_name[SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1], UI8_T type);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_MGR_PBR_RoutemapChangedCallbackHandler
 * -------------------------------------------------------------------------
 * PURPOSE:  callback function when modify acl.
 * INPUT:    rmap_name -- route-map name
 *           seq_num    -- sequence number
 *           is_deleted -- whether it is deleted
 * OUTPUT:   None
 * RETURN:   None
 * NOTES:    
 * -------------------------------------------------------------------------
 */
void NETCFG_MGR_PBR_RoutemapChangedCallbackHandler(char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1], UI32_T seq_num, BOOL_T is_deleted);

void NETCFG_MGR_PBR_SignalL3IfRifUp(UI32_T ifindex, L_INET_AddrIp_T *addr_p);
void NETCFG_MGR_PBR_SignalL3IfRifDown(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

#endif


