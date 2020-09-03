/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_ARP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/01/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_MGR_ARP_H
#define NETCFG_MGR_ARP_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"

#define NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_ARP_IPCMsg_Type_U)

/* command used in IPC message
 */
enum
{
    NETCFG_MGR_ARP_IPC_CREATESTATICARP,
    NETCFG_MGR_ARP_IPC_DELETESTATICARP,
    NETCFG_MGR_ARP_IPC_SETSTATICARP,
    NETCFG_MGR_ARP_IPC_GETARPENTRY,
    NETCFG_MGR_ARP_IPC_GETNEXTARPENTRY,
    NETCFG_MGR_ARP_IPC_DELETEALLDYNAMIC,
    NETCFG_MGR_ARP_IPC_SETTIMEOUT,
    NETCFG_MGR_ARP_IPC_GETSTATISTICS,
    NETCFG_MGR_ARP_IPC_GETRUNNINGTIMEOUT,
    NETCFG_MGR_ARP_IPC_GETNEXTRUNNINGSTATICARPENTRY,
    NETCFG_MGR_ARP_IPC_SIGNALRIFUP,
    NETCFG_MGR_ARP_IPC_SIGNALRIFDOWN,
    NETCFG_MGR_ARP_IPC_SIGNALRIFCREATE,
    NETCFG_MGR_ARP_IPC_SIGNALRIFDESTORY
};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_MGR_ARP_IPCMsg_T.data
 */
#define NETCFG_MGR_ARP_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_ARP_IPCMSG_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_ARP_IPCMsg_T*)0)->data.field_name))




typedef struct
{
    union NETCFG_MGR_ARP_IPCMsg_Type_U
    {
        UI32_T cmd;                /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T        ui32_v;
        NETCFG_TYPE_IpNetToMediaEntry_T           arg_arp_entry; /*all type ARP entry*/
        NETCFG_TYPE_StaticIpNetToMediaEntry_T     arg_arp_static_entry;/*static ARP entry*/
        NETCFG_TYPE_IpNetToMedia_Statistics_T    arg_arp_stat;
        struct
        {
            UI32_T arg1;
            UI32_T arg2;
            UI32_T arg3;
            UI8_T  arg4[NETCFG_TYPE_PHY_ADDRESEE_LENGTH];
        } arg_grp1;

        struct    
        {
            UI32_T arg1;
            UI32_T arg2;
        } arg_grp2;
        
        struct
        {
            NETCFG_TYPE_StaticIpNetToMediaEntry_T arg1;
            UI32_T arg2;
        } arg_grp3;

    } data;
}NETCFG_MGR_ARP_IPCMsg_T;

/* FUNCTION NAME : NETCFG_MGR_ARP_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_ARP used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  - Success
 *      FALSE - Fail
 */
BOOL_T NETCFG_MGR_ARP_InitiateProcessResources(void);

/* FUNCTION NAME : NETCFG_MGR_ARP_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 */
void NETCFG_MGR_ARP_Create_InterCSC_Relation(void); 

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterMasterMode
 * PURPOSE:
 *      Make Routing Engine enter master mode, handling all TCP/IP configuring requests,
 *      and receiving packets.
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
 *
 */
void NETCFG_MGR_ARP_EnterMasterMode (void); 

/* FUNCTION NAME : NETCFG_MGR_ARP_ProvisionComplete
 * PURPOSE:
 *      1. Let default gateway CFGDB into route when provision complete.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 *
 * NOTES:
 *
 */
void NETCFG_MGR_ARP_ProvisionComplete(void); 

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterSlaveMode
 * PURPOSE:
 *      Make Routing Engine enter slave mode, discarding all TCP/IP configuring requests,
 *      and receiving packets.
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
 *      1. In slave mode, just rejects function request and discard incoming message.
 */
void NETCFG_MGR_ARP_EnterSlaveMode (void);

/* FUNCTION NAME : NETCFG_MGR_ARP_SetTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ARP_SetTransitionMode(void);

/* FUNCTION NAME : NETCFG_MGR_ARP_EnterTransitionMode
 * PURPOSE:
 *      Make Routing Engine enter transition mode, releasing all allocateing resource in master mode,
 *      discarding TCP/IP configuring requests, and receiving packets.
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
 *      1. In Transition Mode, must make sure all messages in queue are read
 *         and dynamic allocated space is free, resource set to INIT state.
 *      2. All function requests and incoming messages should be dropped.
 */
void NETCFG_MGR_ARP_EnterTransitionMode (void); 

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_ARP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for ARP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ARP_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Add a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr, phy_addr_len and phy_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_AddStaticIpNetToMediaEntry(UI32_T vid_ifIndex,
                                                                         UI32_T ip_addr,
                                                                         UI32_T phy_addr_len,
                                                                         UI8_T *phy_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_DeleteStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete a static ARP entry.
 *
 * INPUT   : vid_ifindex, ip_addr.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_DeleteStaticIpNetToMediaEntry(UI32_T vid_ifIndex, UI32_T ip_addr);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetNextStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE :Lookup next static ARP entry.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_GetNextStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SetIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE : Set ARP timeout.
 *
 * INPUT   : age_time.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SetIpNetToMediaTimeout(UI32_T age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_DeleteAllDynamicIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Delete all dynamic ARP entries.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_DeleteAllDynamicIpNetToMediaEntry(void);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_GetNextIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetStatistics
 *-----------------------------------------------------------------------------
 * PURPOSE : Get ARP packet statistics.
 *
 * INPUT   : None.
 *
 * OUTPUT  : stat.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_GetStatistics(NETCFG_TYPE_IpNetToMedia_Statistics_T *stat);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetRunningIpNetToMediaTimeout
 *-----------------------------------------------------------------------------
 * PURPOSE :Get running ARP timeout.
 *
 * INPUT   : None.
 *
 * OUTPUT  : age_time.
 *
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS/
 *                  SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *               SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  = 3
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T
NETCFG_MGR_ARP_GetRunningIpNetToMediaTimeout(UI32_T *age_time);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_GetIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Get an ARP entry information.
 *
 * INPUT   : entry.
 *
 * OUTPUT  : entry.
 *
 * RETURN  :TRUE/
 *                 FALSE
 *
 * NOTES   :key are ifindex and ip address.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_MGR_ARP_GetIpNetToMediaEntry(NETCFG_TYPE_IpNetToMediaEntry_T *entry);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SetStaticIpNetToMediaEntry
 *-----------------------------------------------------------------------------
 * PURPOSE : Set a static ARP entry invalid or valid.
 *
 * INPUT   : entry, type.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SetStaticIpNetToMediaEntry(NETCFG_TYPE_StaticIpNetToMediaEntry_T *entry, int type);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifActive
 *-----------------------------------------------------------------------------
 * PURPOSE : when an exist rif active signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifActive(UI32_T ip_addr, UI32_T ip_mask);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifDown
 *-----------------------------------------------------------------------------
 * PURPOSE : when a rif down signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifDown(UI32_T ip_addr, UI32_T ip_mask);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifCreate
 *-----------------------------------------------------------------------------
 * PURPOSE : when create a rif signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :When rif create only update ifindex.
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifCreate(UI32_T ip_addr, UI32_T ip_mask);

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_MGR_ARP_SignalRifDestroy
 *-----------------------------------------------------------------------------
 * PURPOSE : when destroy a rif signal ARP .
 *
 * INPUT   : ip_addr, ip_mask.
 *
 * OUTPUT  : None.
 *
 * RETURN  :NETCFG_TYPE_OK/
 *                 NETCFG_TYPE_FAIL
 *
 * NOTES   :
 *-----------------------------------------------------------------------------
 */
UI32_T NETCFG_MGR_ARP_SignalRifDestroy(UI32_T ip_addr, UI32_T ip_mask);
#endif    /* End of NETCFG_MGR_ARP_H */

