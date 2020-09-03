/* MODULE NAME: NETCFG_OM_PBR.H
 * PURPOSE:
 *   Definitions of OM APIs for PBR
 *
 * NOTES:
 *   None
 *
* HISTORY:
 *    2015/07/09     --- KH Shi, Create
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#ifndef NETCFG_OM_PBR_H
#define NETCFG_OM_PBR_H

#include <stdlib.h>
#include <stdio.h>
#include "sysfun.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "l_inet.h"

/* NAME CONSTANT DECLARATIONS
 */
#define NETCFG_OM_PBR_MSGBUF_TYPE_SIZE sizeof(union NETCFG_OM_PBR_IPCMsg_Type_U)

enum
{
    NETCFG_OM_PBR_IPCCMD_GETNEXTBINDINGENTRY,
    NETCFG_OM_PBR_IPCCMD_GETRUNNINGBINDINGROUTEMAP,
};


#define NETCFG_OM_PBR_BINDING_OPT_DSCP             (1 << 0)
#define NETCFG_OM_PBR_BINDING_OPT_NEXTHOP          (1 << 1)
#define NETCFG_OM_PBR_BINDING_OPT_ALL              (NETCFG_OM_PBR_BINDING_OPT_DSCP|NETCFG_OM_PBR_BINDING_OPT_NEXTHOP)

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in CSCA_OM_IPCMsg_T.data
 */
#define NETCFG_OM_PBR_GET_MSG_SIZE(field_name)                       \
            (NETCFG_OM_PBR_MSGBUF_TYPE_SIZE +                        \
            sizeof(((NETCFG_OM_PBR_IPCMsg_T*)0)->data.field_name))

/* DATA TYPE DECLARATIONS
 */
typedef struct NETCFG_OM_PBR_BindingEntry_S
{
    UI32_T vid;
    char  rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1];
    UI32_T seq_num;
    char acl_name[SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH+1];
    UI32_T dscp;
    L_INET_AddrIp_T nexthop;
    UI32_T nexthop_ifindex;
    UI32_T nexthop_hwinfo;
    UI8_T  rmap_type;
    UI8_T  option;
    BOOL_T in_amtrl3;
    BOOL_T in_chip;
} NETCFG_OM_PBR_BindingEntry_T;

typedef struct NETCFG_OM_PBR_BindingRouteMap_S
{
    UI32_T vid;
    char  rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1];
} NETCFG_OM_PBR_BindingRouteMap_T;

/* structure for the request/response ipc message in csca pom and om
 */
typedef struct
{
    union NETCFG_OM_PBR_IPCMsg_Type_U
    {
        UI32_T cmd;          /* for sending IPC request. CSCA_OM_IPC_CMD1 ... */
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/ 
        UI32_T result_i32;   /*respond i32 return*/
        SYS_TYPE_Get_Running_Cfg_T result_running_cfg; /* For running config API */
    } type;

    union
    {
        NETCFG_OM_PBR_BindingEntry_T binding_entry;
        NETCFG_OM_PBR_BindingRouteMap_T binding_rmap;
    } data; /* contains the supplemntal data for the corresponding cmd */
} NETCFG_OM_PBR_IPCMsg_T;


/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Init the OM system resouces.
 * INPUT:    none
 * OUTPUT:   none.
 * RETURN:   
 * NOTES:    
 * -------------------------------------------------------------------------*/
void NETCFG_OM_PBR_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_Init
 *--------------------------------------------------------------------------
 * PURPOSE: To create the binding table
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_OM_PBR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_ClearAll
 *--------------------------------------------------------------------------
 * PURPOSE: To clear the om database.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *--------------------------------------------------------------------------
 */
void NETCFG_OM_PBR_ClearAll(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_AddBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Add a binding entry to OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_AddBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get a binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete a binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntry(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by nexthop
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByNextHop(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByExactNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by nexthop
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES: 1. if the nexthop retrieved is not the same as input, return false
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByExactNextHop(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingEntryByAclName
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding entry from OM by ACL name
 * INPUT:    binding_entry_p -- NETCFG_OM_PBR_BindingEntry_T
 * OUTPUT:   none
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingEntryByAclName(NETCFG_OM_PBR_BindingEntry_T *binding_entry_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetBindingRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Get binding route-map name by vlan id
 * INPUT:    vid        -- VLAN ID
 * OUTPUT:   rmap_name  -- route-map name
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetBindingRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetBindingVlan
 * -------------------------------------------------------------------------
 * PURPOSE:  Get binding vlan id by route-map name
 * INPUT:    rmap_name  -- route-map name
 * OUTPUT:   vid        -- VLAN ID
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetBindingVlan(char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1], UI32_T *vid);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_GetNextBindingVlan
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next binding vlan id by route-map name
 * INPUT:    rmap_name  -- route-map name
 * OUTPUT:   vid        -- VLAN ID
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_GetNextBindingVlan(char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1], UI32_T *vid);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteEntriesByVidAndRouteMap
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete binding entries by vid and route-map name
 * INPUT:    vid        -- VLAN ID
 *           rmap_name  -- route-map name
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTES:    1. won't delete the seq_num = 0 entry
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteEntriesByVidAndRouteMap(UI32_T vid, char rmap_name[SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH+1]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: NETCFG_OM_PBR_DeleteEntriesByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete binding entries by vid
 * INPUT:    vid        -- VLAN ID
 * OUTPUT:   None
 * RETURN:   TRUE/FALSE
 * NOTES:    
 * -------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_DeleteEntriesByVid(UI32_T vid);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_OM_PBR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for PBR OM.
 *
 * INPUT   : ipcmsg_p -- input request ipc message buffer
 *
 * OUTPUT  : ipcmsg_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_OM_PBR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif
 
