/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_RIP.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/05/18     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_MGR_RIP_H
#define NETCFG_MGR_RIP_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "l_radix.h"

#define NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_RIP_IPCMsg_Type_U)

/* command used in IPC message
 */
enum
{
    NETCFG_MGR_RIP_IPC_DEBUG,
    NETCFG_MGR_RIP_IPC_CONFIGDEBUG,
    NETCFG_MGR_RIP_IPC_UNDEBUG,
    NETCFG_MGR_RIP_IPC_CONFIGUNDEBUG,
    NETCFG_MGR_RIP_IPC_DEBUGEVENT,
    NETCFG_MGR_RIP_IPC_CONFIGDEBUGEVENT,
    NETCFG_MGR_RIP_IPC_UNDEBUGEVENT,
    NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGEVENT,
    NETCFG_MGR_RIP_IPC_DEBUGPACKET,
    NETCFG_MGR_RIP_IPC_CONFIGDEBUGPACKET,
    NETCFG_MGR_RIP_IPC_UNDEBUGPACKET,
    NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGPACKET,
    NETCFG_MGR_RIP_IPC_DEBUGNSM,
    NETCFG_MGR_RIP_IPC_CONFIGDEBUGNSM,
    NETCFG_MGR_RIP_IPC_UNDEBUGNSM,
    NETCFG_MGR_RIP_IPC_CONFIGUNDEBUGNSM,
    NETCFG_MGR_RIP_IPC_GETDEBUGSTATUS,
    NETCFG_MGR_RIP_IPC_RECVPACKETSET,
    NETCFG_MGR_RIP_IPC_RECVPACKETUNSET,
    NETCFG_MGR_RIP_IPC_SENDPACKETSET,
    NETCFG_MGR_RIP_IPC_SENDPACKETUNSET,
    NETCFG_MGR_RIP_IPC_RECVVERSIONTYPESET,
    NETCFG_MGR_RIP_IPC_RECVVERSIONUNSET,
    NETCFG_MGR_RIP_IPC_SENDVERSIONTYPESET,
    NETCFG_MGR_RIP_IPC_SENDVERSIONUNSET,
    NETCFG_MGR_RIP_IPC_AUTHMODESET,
    NETCFG_MGR_RIP_IPC_AUTHMODEUNSET,
    NETCFG_MGR_RIP_IPC_AUTHSTRINGSET,
    NETCFG_MGR_RIP_IPC_AUTHSTRINGUNSET,
    NETCFG_MGR_RIP_IPC_AUTHKEYCHAINSET,
    NETCFG_MGR_RIP_IPC_AUTHKEYCHAINUNSET,
    NETCFG_MGR_RIP_IPC_SPLITHORIZONSET,
    NETCFG_MGR_RIP_IPC_SPLITHORIZONUNSET,
    NETCFG_MGR_RIP_IPC_ROUTERRIPSET,
    NETCFG_MGR_RIP_IPC_ROUTERRIPUNSET,
    NETCFG_MGR_RIP_IPC_VERSIONSET,
    NETCFG_MGR_RIP_IPC_VERSIONUNSET,
    NETCFG_MGR_RIP_IPC_NETWORKSETBYVID,
    NETCFG_MGR_RIP_IPC_NETWORKSETBYADDRESS,
    NETCFG_MGR_RIP_IPC_NETWORKUNSETBYVID,
    NETCFG_MGR_RIP_IPC_NETWORKUNSETBYADDRESS,
    NETCFG_MGR_RIP_IPC_NEIGHBORSET,
    NETCFG_MGR_RIP_IPC_NEIGHBORUNSET,
    NETCFG_MGR_RIP_IPC_PASSIVEIFADD,
    NETCFG_MGR_RIP_IPC_PASSIVEIFDELETE,
    NETCFG_MGR_RIP_IPC_DEFAULTADD,
    NETCFG_MGR_RIP_IPC_DEFAULTDELETE,
    NETCFG_MGR_RIP_IPC_DEFAULTMETRICSET,
    NETCFG_MGR_RIP_IPC_DEFAULTMETRICUNSET,
    NETCFG_MGR_RIP_IPC_DISTRIBUTELISTADD,
    NETCFG_MGR_RIP_IPC_DISTRIBUTELISTDELETE,
    NETCFG_MGR_RIP_IPC_TIMERSET,
    NETCFG_MGR_RIP_IPC_TIMERUNSET,
    NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTSET,
    NETCFG_MGR_RIP_IPC_DISTANCEDEFAULTUNSET,
    NETCFG_MGR_RIP_IPC_DISTANCESET,
    NETCFG_MGR_RIP_IPC_DISTANCEUNSET,
    NETCFG_MGR_RIP_IPC_MAXPREFIXSET,
    NETCFG_MGR_RIP_IPC_MAXPREFIXUNSET,
    NETCFG_MGR_RIP_IPC_RECVBUFFSIZESET,
    NETCFG_MGR_RIP_IPC_RECVBUFFSIZEUNSET,
    NETCFG_MGR_RIP_IPC_REDISTRIBUTESET,
    NETCFG_MGR_RIP_IPC_REDISTRIBUTEMETRICSET,
    NETCFG_MGR_RIP_IPC_REDISTRIBUTERMAPSET,
    NETCFG_MGR_RIP_IPC_REDISTRIBUTEALLSET,
    NETCFG_MGR_RIP_IPC_REDISTRIBUTEUNSET,
    NETCFG_MGR_RIP_IPC_CLEARROUTE,
    NETCFG_MGR_RIP_IPC_CLEARSTATISTICS,
    NETCFG_MGR_RIP_IPC_GETINSTANCE,
    NETCFG_MGR_RIP_IPC_GETINTERFACE,
    NETCFG_MGR_RIP_IPC_GETNEXTINTERFACE,
    NETCFG_MGR_RIP_IPC_GETNETWORKTABLE,
    NETCFG_MGR_RIP_IPC_GETNEXTNETWORKTABLE,
    NETCFG_MGR_RIP_IPC_GETNEIGHBOR,
    NETCFG_MGR_RIP_IPC_GETNEXTNEIGHBOR,
    NETCFG_MGR_RIP_IPC_GETDISTANCETABLE,
    NETCFG_MGR_RIP_IPC_GETNEXTDISTANCETABLE,
    NETCFG_MGR_RIP_IPC_GETNEXTRIPROUTE,
    NETCFG_MGR_RIP_IPC_GETNEXTTHREADTIMER,
    NETCFG_MGR_RIP_IPC_GETPEERENTRY,
    NETCFG_MGR_RIP_IPC_GETNEXTPEERENTRY,
    NETCFG_MGR_RIP_IPC_GETGLOBALSTATISTICS,
    NETCFG_MGR_RIP_IPC_GETIFADDRESS,
    NETCFG_MGR_RIP_IPC_GETIFRECVBADPACKET,
    NETCFG_MGR_RIP_IPC_GETIFRECVBADROUTE,
    NETCFG_MGR_RIP_IPC_GETIFSENDUPDATE,
    NETCFG_MGR_RIP_IPC_GETNEXTREDISTRIBUTETABLE,
    NETCFG_MGR_RIP_IPC_GETREDISTRIBUTETABLE,
    NETCFG_MGR_RIP_IPC_GETNEXTACTIVERIFBYVLANIFINDEX,   
};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_MGR_RIP_IPCMsg_T.data
 */
#define NETCFG_MGR_RIP_GET_MSG_SIZE(field_name)                       \
            (NETCFG_MGR_RIP_IPCMSG_TYPE_SIZE +                        \
            sizeof(((NETCFG_MGR_RIP_IPCMsg_T*)0)->data.field_name))




typedef struct
{
    union NETCFG_MGR_RIP_IPCMsg_Type_U
    {
        UI32_T cmd;                /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;

    union
    {
        UI32_T        ui32_v;
        int           int_v;
        char          char_v[20];
        struct pal_in4_addr     addr_v;
        NETCFG_TYPE_RIP_Packet_Debug_Type_T    pdebug_v;
        NETCFG_TYPE_RIP_Debug_Status_T status_v;
        NETCFG_TYPE_RIP_Timer_T  timer_v;
        enum NETCFG_TYPE_RIP_Global_Version_E version_v;
        NETCFG_TYPE_RIP_Instance_T    instance_v;
        NETCFG_TYPE_RIP_If_T          if_v;
        NETCFG_TYPE_RIP_Network_T     prefix_v;
        NETCFG_TYPE_RIP_Distance_T    distance_v;
        NETCFG_TYPE_RIP_Route_T       route_v;
        NETCFG_TYPE_RIP_Peer_Entry_T  peer_v;
        NETCFG_TYPE_RIP_Global_Statistics_T global_stat_v;
        NETCFG_TYPE_RIP_Redistribute_Table_T redistribute_v;
        struct
        {
            UI32_T  arg1;
            UI32_T  arg2;
        } arg_grp1;

        struct    
        {
            UI32_T  arg1;
            char    arg2[17];
        } arg_grp2;
        
        struct
        {
            char    arg1[20];
            char    arg2[16];
            enum NETCFG_TYPE_RIP_Distribute_Type_E          arg3;
            enum NETCFG_TYPE_RIP_Distribute_List_Type_E     arg4;
        } arg_grp3;

        struct
        {
            UI32_T                 arg1;
            UI32_T    arg2;
            UI32_T                 arg3;
            char                   arg4[16];
        } arg_grp4;
    
        struct
        {
            char    arg1[10];
            UI32_T  arg2;
            char    arg3[16];
        } arg_grp5;
        
        struct
        {
            UI32_T                          arg1;
            enum NETCFG_TYPE_RIP_Version_E  arg2;
        } arg_grp6;

        struct
        {
            UI32_T                            arg1;
            enum NETCFG_TYPE_RIP_Auth_Mode_E  arg2;
        } arg_grp7;

        struct
        {
            UI32_T                            arg1;
            UI32_T                            arg2;
            UI32_T                            arg3;
        } arg_grp8;
    
        struct 
        {
            L_PREFIX_T                        arg1;
        } arg_grp9;
    
        struct 
        {
            UI32_T ui32;
            L_INET_AddrIp_T addr;
        } arg_grp10;
    
    } data;
}NETCFG_MGR_RIP_IPCMsg_T;

/* FUNCTION NAME : NETCFG_MGR_RIP_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_RIP used system resource, eg. protection semaphore.
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
BOOL_T NETCFG_MGR_RIP_InitiateProcessResources(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterMasterMode
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
void NETCFG_MGR_RIP_EnterMasterMode (void); 

/* FUNCTION NAME : NETCFG_MGR_RIP_ProvisionComplete
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
void NETCFG_MGR_RIP_ProvisionComplete(void); 

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterSlaveMode
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
void NETCFG_MGR_RIP_EnterSlaveMode (void);

/* FUNCTION NAME : NETCFG_MGR_RIP_SetTransitionMode
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
void NETCFG_MGR_RIP_SetTransitionMode(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_EnterTransitionMode
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
void NETCFG_MGR_RIP_EnterTransitionMode (void);   

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_RIP_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RIP MGR.
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
BOOL_T NETCFG_MGR_RIP_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p); 

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebug
* PURPOSE:
*     RIP debug on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebug(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_Debug
* PURPOSE:
*     RIP debug on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_Debug(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebug
* PURPOSE:
*     RIP undebug on CONFIG mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebug(void);


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebug
* PURPOSE:
*     RIP undebug on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebug(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugEvent
* PURPOSE:
*     RIP debug event on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugEvent(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugEvent
* PURPOSE:
*     RIP undebug event on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugEvent(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DebugEvent
* PURPOSE:
*     RIP debug event on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugEvent(void);


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugEvent
* PURPOSE:
*     RIP undebug event on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugEvent(void);


/* FUNCTION NAME : NETCFG_MGR_RIP_DebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);


/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugPacket
* PURPOSE:
*     RIP debug packet on config mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugPacket
* PURPOSE:
*     RIP undebug packet on config mode.
*
* INPUT:
*       type:
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE = 1,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL,
*       NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);


/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigDebugNsm
* PURPOSE:
*     RIP debug nsm on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigDebugNsm(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_ConfigUnDebugNsm
* PURPOSE:
*     RIP undebug nsm on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_ConfigUnDebugNsm(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DebugNsm
* PURPOSE:
*     RIP debug nsm on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_DebugNsm(void);


/* FUNCTION NAME : NETCFG_MGR_RIP_UnDebugNsm
* PURPOSE:
*     RIP undebug nsm on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_UnDebugNsm(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetDebugStatus
* PURPOSE:
*     Get RIP debug status on EXEC mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      status.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_MGR_RIP_GetDebugStatus(NETCFG_TYPE_RIP_Debug_Status_T *status);

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvPacketSet
* PURPOSE:
*     Set receive packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvPacketSet(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvPacketUnset
* PURPOSE:
*     Unset receive packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvPacketUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SendPacketSet
* PURPOSE:
*     set send packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendPacketSet(UI32_T ifindex);


/* FUNCTION NAME : NETCFG_MGR_RIP_SendPacketUnset
* PURPOSE:
*     Unset send packet on an interface.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendPacketUnset(UI32_T ifindex);



/* FUNCTION NAME : NETCFG_MGR_RIP_RecvVersionTypeSet
* PURPOSE:
*     Set RIP receive version.
*
* INPUT:
*      ifindex,
*      type:
*        rip version 1----- 1
*        rip version 2------2
*        rip version 1 and 2----3
*        rip version 1 compatible-----4
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type);


/* FUNCTION NAME : NETCFG_MGR_RIP_RecvVersionUnset
* PURPOSE:
*     unset RIP receive version.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvVersionUnset(UI32_T ifindex);


/* FUNCTION NAME : NETCFG_MGR_RIP_SendVersionTypeSet
* PURPOSE:
*     Set RIP send version.
*
* INPUT:
*      ifindex,
*      type:
*        rip version 1----- 1
*        rip version 2------2
*        rip version 1 and 2----3
*        rip version 1 compatible-----4
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type);


/* FUNCTION NAME : NETCFG_MGR_RIP_SendVersionUnset
* PURPOSE:
*     unset RIP send version.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SendVersionUnset(UI32_T ifindex);


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthModeSet
* PURPOSE:
*     Set RIP authentication mode.
*
* INPUT:
*      ifindex,
*      mode.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthModeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Auth_Mode_E mode);


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthModeUnset
* PURPOSE:
*     Unset RIP authentication mode.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthModeUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_AuthStringSet
* PURPOSE:
*     set RIP authentication string.
*
* INPUT:
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthStringSet(UI32_T ifindex, char *str);


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthStringUnset
* PURPOSE:
*     Unset RIP authentication string.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthStringUnset(UI32_T ifindex);


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthKeyChainSet
* PURPOSE:
*     set RIP authentication key-chain.
*
* INPUT:
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthKeyChainSet(UI32_T ifindex, char *str);


/* FUNCTION NAME : NETCFG_MGR_RIP_AuthKeyChainUnset
* PURPOSE:
*     Unset RIP authentication key-chain.
*
* INPUT:
*      ifindex,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_AuthKeyChainUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SplitHorizonSet
* PURPOSE:
*     Set RIP split horizon.
*
* INPUT:
*      ifindex,
*      mode:
*        split horizon----- 0
*        split horizon poisoned------1
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SplitHorizonSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type);

/* FUNCTION NAME : NETCFG_MGR_RIP_SplitHorizonUnset
* PURPOSE:
*     unset rip spliet horizon.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_SplitHorizonUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_RouterRipSet
* PURPOSE:
*     Set router rip.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RouterRipSet(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_RouterRipUnset
* PURPOSE:
*     unset router rip.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None,
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RouterRipUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_VersionSet
* PURPOSE:
*     set rip version.
*
* INPUT:
*      version.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_VersionSet(enum NETCFG_TYPE_RIP_Global_Version_E version);

/* FUNCTION NAME : NETCFG_MGR_RIP_VersionUnset
* PURPOSE:
*     unset rip version.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None,
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_VersionUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkSetByVid
* PURPOSE:
*     set rip network.
*
* INPUT:
*      vid: vlan index
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkSetByVid(UI32_T vid);

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkSetByAddress
* PURPOSE:
*     set rip network.
*
* INPUT:
*      network address -- network address
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkSetByAddress(L_PREFIX_T network_address);

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkUnsetByVid
* PURPOSE:
*     unset rip network.
*
* INPUT:
*      vid  --  vlan index
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkUnsetByVid(UI32_T vid);

/* FUNCTION NAME : NETCFG_MGR_RIP_NetworkUnsetByAddress
* PURPOSE:
*     unset rip network.
*
* INPUT:
*      network address   --  network address
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NetworkUnsetByAddress(L_PREFIX_T network_address);

/* FUNCTION NAME : NETCFG_MGR_RIP_NeighborSet
* PURPOSE:
*     set rip neighbor
*
* INPUT:
*      ip_addr.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NeighborSet(UI32_T ip_addr);

/* FUNCTION NAME : NETCFG_MGR_RIP_NeighborUnset
* PURPOSE:
*     unset rip neighbor
*
* INPUT:
*      ip_addr.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_NeighborUnset(UI32_T ip_addr);

/* FUNCTION NAME : NETCFG_MGR_RIP_PassiveIfAdd
* PURPOSE:
*     add passive interface.
*
* INPUT:
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_PassiveIfAdd(UI32_T vid);

/* FUNCTION NAME : NETCFG_MGR_RIP_PassiveIfDelete
* PURPOSE:
*     delete passive interface.
*
* INPUT:
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_PassiveIfDelete(UI32_T vid);

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultAdd
* PURPOSE:
*     originate rip default information.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultAdd(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultDelete
* PURPOSE:
*     not originate rip default information.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultDelete(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultMetricSet
* PURPOSE:
*     set rip default metric.
*
* INPUT:
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultMetricSet(UI32_T metric);

/* FUNCTION NAME : NETCFG_MGR_RIP_DefaultMetricUnset
* PURPOSE:
*     unset rip default metric.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DefaultMetricUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistributeListAdd
* PURPOSE:
*     add distribute list.
*
* INPUT:
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistributeListAdd(char *ifname, char *list_name,
                                                      enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                      enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistributeListDelete
* PURPOSE:
*     delete distribute list.
*
* INPUT:
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistributeListDelete(char *ifname, char *list_name,
                                                         enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                         enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_MGR_RIP_TimerSet
* PURPOSE:
*     set timer value.
*
* INPUT:
*      timer: update, timeout,carbage.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_TimerSet(NETCFG_TYPE_RIP_Timer_T *timer);

/* FUNCTION NAME : NETCFG_MGR_RIP_TimerUnset
* PURPOSE:
*     unset timer.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_TimerUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceDefaultSet
* PURPOSE:
*     set distance value.
*
* INPUT:
*      distance.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceDefaultSet(UI32_T distance);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceDefaultUnset
* PURPOSE:
*     unset distance value.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceDefaultUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceSet
* PURPOSE:
*     set distance .
*
* INPUT:
*      distance,
*      ip_add,
*      plen: prefix length
*      alist.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceSet(UI32_T distance, UI32_T ip_addr, UI32_T plen, char *alist);

/* FUNCTION NAME : NETCFG_MGR_RIP_DistanceUnset
* PURPOSE:
*     unset distance .
*
* INPUT:
*      ip_add,
*      plen: prefix length
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_DistanceUnset(UI32_T ip_addr, UI32_T plen);

/* FUNCTION NAME : NETCFG_MGR_RIP_MaxPrefixSet
* PURPOSE:
*     set max prefix value.
*
* INPUT:
*      pmax.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_MaxPrefixSet(UI32_T pmax);

/* FUNCTION NAME : NETCFG_MGR_RIP_MaxPrefixUnset
* PURPOSE:
*     unset max prefix.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_MaxPrefixUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvBuffSizeSet
* PURPOSE:
*     set reveiver buffer size.
*
* INPUT:
*      buff_size.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvBuffSizeSet(UI32_T buff_size);

/* FUNCTION NAME : NETCFG_MGR_RIP_RecvBuffSizeUnset
* PURPOSE:
*     unset receive buffer size.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RecvBuffSizeUnset(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeSet
* PURPOSE:
*     set redistribute.
*
* INPUT:
*      protocol: protocol string.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeSet(char *protocol);

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeMetricSet
* PURPOSE:
*     set redistribute with metric.
*
* INPUT:
*      protocol: protocol string,
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeMetricSet(char *protocol, UI32_T metric);

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeRmapSet
* PURPOSE:
*     set redistribute with route map.
*
* INPUT:
*      protocol: protocol string,
*      rmap.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeRmapSet(char *protocol, char *rmap);

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeAllSet
* PURPOSE:
*     set redistribute with metric and route map.
*
* INPUT:
*      protocol: protocol string,
*      metric
*      rmap.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeAllSet(char *protocol, UI32_T metric, char *rmap);

/* FUNCTION NAME : NETCFG_MGR_RIP_RedistributeUnset
* PURPOSE:
*     unset redistribute.
*
* INPUT:
*      protocol: protocol string,
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_RIP_RedistributeUnset(char *protocol);

/* FUNCTION NAME : NETCFG_MGR_RIP_ClearRoute
* PURPOSE:
*     Clear rip router by type or by network address.
*
* INPUT:
*      arg
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      Input parameter 'arg', please input string "connected","static","ospf","rip","static","all" or "A.B.C.D/M".
*/
UI32_T NETCFG_MGR_RIP_ClearRoute(char *arg);

/* FUNCTION NAME : NETCFG_MGR_RIP_ClearStatistics
* PURPOSE:
*     Clear rip statistics.
*
* INPUT:
*      None
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_ClearStatistics(void);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceAdd
* PURPOSE:
*     When add an l3 interface signal RIP.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalInterfaceAdd(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceDelete
* PURPOSE:
*     When delete an l3 interface signal RIP.
*
* INPUT:
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalInterfaceDelete(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalLoopbackInterfaceAdd
* PURPOSE:
*     When add a loopback interface signal RIP.
*
* INPUT:
*      ifindex.
*      if_flags
*
* OUTPUT:
*      None.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_SignalLoopbackInterfaceAdd(UI32_T ifindex, UI16_T if_flags);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceUp
 * PURPOSE:
 *     When a l3 interface up, signal RIP.
 *
 * INPUT:
 *      ifindex  -- Interface ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalInterfaceUp(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalInterfaceDown
 * PURPOSE:
 *     When a l3 interface down, signal RIP.
 *
 * INPUT:
 *      ifindex -- Interface ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalInterfaceDown(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalRifUp
 * PURPOSE:
 *     When a l3 interface rif up signal RIP.
 *
 * INPUT:
 *      ifindex  -- Interface ifindex
 *      ipaddr   -- RIF address
 *      ipmask   -- RIF address mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalRifUp(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : NETCFG_MGR_RIP_SignalRifDown
 * PURPOSE:
 *     When a l3 interface rif down signal RIP.
 *
 * INPUT:
 *      ifindex  -- Interface ifindex
 *      ipaddr   -- RIF address
 *      ipmask   -- RIF address mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T NETCFG_MGR_RIP_SignalRifDown(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetInstanceEntry
* PURPOSE:
*     Get rip instance entry.
*
* INPUT:
*      None
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_GetInstanceEntry(NETCFG_TYPE_RIP_Instance_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetInterfaceEntry
* PURPOSE:
*     Get rip interface entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T NETCFG_MGR_RIP_GetInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextInterfaceEntry
* PURPOSE:
*     Getnext rip interface entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the entry.ifindex == 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNetworkTable
* PURPOSE:
*     Get rip network table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetNetworkTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextNetworkTable
* PURPOSE:
*     Getnext rip network table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the p.prefix and p.u.prefix4.s_addr are 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextNetworkTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNeighborTable
* PURPOSE:
*     Get rip Neighbor table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetNeighborTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextNeighborTable
* PURPOSE:
*     Getnext rip Neighbor table.
*
* INPUT:
*      p
*
* OUTPUT:
*      p.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      if the p.u.prefix4.s_addr is 0, get first
*/
UI32_T NETCFG_MGR_RIP_GetNextNeighborTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetDistanceTable
* PURPOSE:
*     Get rip distance table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*     
*/
UI32_T NETCFG_MGR_RIP_GetDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextDistanceTable
* PURPOSE:
*     Getnext rip distance table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*       if entry.p.prefixlen and entry.p.u.prefix4.s_addr are 0 ,get first
*/
UI32_T NETCFG_MGR_RIP_GetNextDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextRouteEntry
* PURPOSE:
*     Getnext rip route table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*       
*/
UI32_T NETCFG_MGR_RIP_GetNextRouteEntry(NETCFG_TYPE_RIP_Route_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextThreadTimer
* PURPOSE:
*     Get next thread timer.
*
* INPUT:
*      None
*
* OUTPUT:
*      nexttime.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetNextThreadTimer(UI32_T *nexttime);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextPeerEntry
* PURPOSE:
*     Get next peer entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetNextPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetPeerEntry
* PURPOSE:
*     Get peer entry.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetGlobalStatistics
* PURPOSE:
*     Get rip global statistics, include global route changes and blobal queries.
*
* INPUT:
*      None
*
* OUTPUT:
*      stat.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetGlobalStatistics(NETCFG_TYPE_RIP_Global_Statistics_T *stat);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfAddress
* PURPOSE:
*     Get or getnext interface IP address.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      out_addr.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfAddress(UI32_T exact, UI32_T in_addr, UI32_T *out_addr);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfRecvBadPacket
* PURPOSE:
*     Get or getnext interface receive bad packet.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfRecvBadPacket(UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfRecvBadRoute
* PURPOSE:
*     Get or getnext interface receive bad route.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfRecvBadRoute(UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetRedistributeTable
* PURPOSE:
*     Get rip redistribute table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      key is entry->protocol
*/
UI32_T NETCFG_MGR_RIP_GetRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetIfSendUpdate
* PURPOSE:
*     Get or getnext interface send update.
*
* INPUT:
*      exact: 0- getnext/1 - get,
*      in_addr
*
* OUTPUT:
*      value.
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T NETCFG_MGR_RIP_GetIfSendUpdate(UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextRedistributeTable
* PURPOSE:
*     Get next rip redistribute table.
*
* INPUT:
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      key is entry->protocol, if the protocol is NETCFG_TYPE_RIP_Redistribute_Max , get first
*/
UI32_T NETCFG_MGR_RIP_GetNextRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry);

/* FUNCTION NAME : NETCFG_MGR_RIP_GetNextActiveRifByVlanIfIndex
 * PURPOSE:
 *     Get next rif which is rip enabled.
 *
 * INPUT:
 *      ifindex -- ifindex of vlan
 *
 * OUTPUT:
 *      addr_p  -- ip address of rif.
 *
 * RETURN:
 *       NETCFG_TYPE_OK / NETCFG_TYPE_NO_MORE_ENTRY
 *
 * NOTES:
 *
 */
UI32_T NETCFG_MGR_RIP_GetNextActiveRifByVlanIfIndex(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

#endif /* NETCFG_MGR_RIP_H */

