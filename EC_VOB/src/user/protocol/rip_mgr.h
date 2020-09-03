/* MODULE NAME:  rip_mgr.h
 * PURPOSE:
 *     This module provides APIs for RIP CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    05/12/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
 
#ifndef RIP_MGR_H
#define RIP_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "rip_type.h"
#include "l_radix.h"
#define RIP_MGR_IPCMSG_TYPE_SIZE sizeof(union RIP_MGR_IPCMsg_Type_U)

#define RIP_MGR_MSG_HEADER_SIZE ((UI32_T)(&(((RIP_MGR_IPCMsg_T*)0)->data)))


#define RIP_MGR_GET_MSGBUFSIZE(msg_data_type) \
    (RIP_MGR_IPCMSG_TYPE_SIZE + sizeof(msg_data_type))

#define RIP_MGR_GET_MSG_SIZE(field_name)                       \
            (RIP_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((RIP_MGR_IPCMsg_T *)0)->data.field_name))

/***************************************************
 **    rip_mgr ipc request command definitions    **
 ***************************************************
 */
enum
{
    RIP_MGR_IPCCMD_DEBUG= 0,
    RIP_MGR_IPCCMD_CONFIGDEBUG,
    RIP_MGR_IPCCMD_UNDEBUG,
    RIP_MGR_IPCCMD_CONFIGUNDEBUG,
    RIP_MGR_IPCCMD_DEBUGEVENT,
    RIP_MGR_IPCCMD_CONFIGDEBUGEVENT,
    RIP_MGR_IPCCMD_UNDEBUGEVENT,
    RIP_MGR_IPCCMD_CONFIGUNDEBUGEVENT,
    RIP_MGR_IPCCMD_DEBUGPACKET,
    RIP_MGR_IPCCMD_CONFIGDEBUGPACKET,
    RIP_MGR_IPCCMD_UNDEBUGPACKET,
    RIP_MGR_IPCCMD_CONFIGUNDEBUGPACKET,
    RIP_MGR_IPCCMD_DEBUGNSM,
    RIP_MGR_IPCCMD_CONFIGDEBUGNSM,
    RIP_MGR_IPCCMD_UNDEBUGNSM,
    RIP_MGR_IPCCMD_CONFIGUNDEBUGNSM,  
    RIP_MGR_IPCCMD_RECVPACKETSET,
    RIP_MGR_IPCCMD_RECVPACKETUNSET,
    RIP_MGR_IPCCMD_SENDPACKETSET,
    RIP_MGR_IPCCMD_SENDPACKETUNSET,
    RIP_MGR_IPCCMD_RECVVERSIONTYPESET,
    RIP_MGR_IPCCMD_RECVVERSIONUNSET,
    RIP_MGR_IPCCMD_SENDVERSIONTYPESET,
    RIP_MGR_IPCCMD_SENDVERSIONUNSET,
    RIP_MGR_IPCCMD_AUTHMODESET,
    RIP_MGR_IPCCMD_AUTHMODEUNSET,
    RIP_MGR_IPCCMD_AUTHSTRINGSET,
    RIP_MGR_IPCCMD_AUTHSTRINGUNSET,
    RIP_MGR_IPCCMD_AUTHKEYCHAINSET,
    RIP_MGR_IPCCMD_AUTHKEYCHAINUNSET,
    RIP_MGR_IPCCMD_SPLITHORIZONSET,
    RIP_MGR_IPCCMD_SPLITHORIZONUNSET,
    RIP_MGR_IPCCMD_ROUTERRIPSET,
    RIP_MGR_IPCCMD_ROUTERRIPUNSET,
    RIP_MGR_IPCCMD_VERSIONSET,
    RIP_MGR_IPCCMD_VERSIONUNSET,
    RIP_MGR_IPCCMD_NETWORKSETBYVID,
    RIP_MGR_IPCCMD_NETWORKSETBYADDRESS,
    RIP_MGR_IPCCMD_NETWORKUNSETBYVID,
    RIP_MGR_IPCCMD_NETWORKUNSETBYADDRESS,
    RIP_MGR_IPCCMD_NEIGHBORSET,
    RIP_MGR_IPCCMD_NEIGHBORUNSET,
    RIP_MGR_IPCCMD_PASSIVEIFADD,
    RIP_MGR_IPCCMD_PASSIVEIFDELETE,
    RIP_MGR_IPCCMD_DEFAULTADD,
    RIP_MGR_IPCCMD_DEFAULTDELETE,
    RIP_MGR_IPCCMD_DEFAULTMETRICSET,
    RIP_MGR_IPCCMD_DEFAULTMETRICUNSET,
    RIP_MGR_IPCCMD_DISTRIBUTELISTADD,
    RIP_MGR_IPCCMD_DISTRIBUTELISTDELETE,
    RIP_MGR_IPCCMD_TIMERSET,
    RIP_MGR_IPCCMD_TIMERUNSET,
    RIP_MGR_IPCCMD_DISTANCEDEFAULTSET,
    RIP_MGR_IPCCMD_DISTANCEDEFAULTUNSET,
    RIP_MGR_IPCCMD_DISTANCESET,
    RIP_MGR_IPCCMD_DISTANCEUNSET,
    RIP_MGR_IPCCMD_MAXPREFIXSET,
    RIP_MGR_IPCCMD_MAXPREFIXUNSET,
    RIP_MGR_IPCCMD_RECVBUFFSIZESET,
    RIP_MGR_IPCCMD_RECVBUFFSIZEUNSET,
    RIP_MGR_IPCCMD_REDISTRIBUTESET,
    RIP_MGR_IPCCMD_REDISTRIBUTEMETRICSET,
    RIP_MGR_IPCCMD_REDISTRIBUTERMAPSET,
    RIP_MGR_IPCCMD_REDISTRIBUTEALLSET,
    RIP_MGR_IPCCMD_REDISTRIBUTEUNSET,
    RIP_MGR_IPCCMD_CLEARROUTE,
    RIP_MGR_IPCCMD_CLEARSTATISTICS,
    RIP_MGR_IPCCMD_INTERFACEADD,
    RIP_MGR_IPCCMD_INTERFACEDELETE,
    RIP_MGR_IPCCMD_LOOPBACKINTERFACEADD,
    RIP_MGR_IPCCMD_INTERFACEUP,
    RIP_MGR_IPCCMD_INTERFACEDOWN,
    RIP_MGR_IPCCMD_IPADDRESSADD,
    RIP_MGR_IPCCMD_IPADDRESSDELET,
    RIP_MGR_IPCCMD_RIFUP,
    RIP_MGR_IPCCMD_RIFDOWN,
    RIP_MGR_IPCCMD_GETNEXTROUTEENTRY,
    RIP_MGR_IPCCMD_GETNEXTTHREADTIMER,
    RIP_MGR_IPCCMD_GETPEERENTRY,
    RIP_MGR_IPCCMD_GETNEXTPEERENTRY,
    RIP_MGR_IPCCMD_GETGLABLASTATISTICS,
    RIP_MGR_IPCCMD_GETRIP2IFSTATTABLE,
    RIP_MGR_IPCCMD_GETRIP2IFCONFTABLE,
    RIP_MGR_IPCCMD_GETNEXTRIP2IFSTATTABLE,
    RIP_MGR_IPCCMD_GETNEXTRIP2IFCONFTABLE,
    RIP_MGR_IPCCMD_GETIFADDRESS,
    RIP_MGR_IPCCMD_GETIFRECVBADPACKET,
    RIP_MGR_IPCCMD_GETIFRECVBADROUTE,
    RIP_MGR_IPCCMD_GETIFSENDUPDATE
};

typedef struct
{
    UI32_T vr_id;
    UI32_T vrf_id;
    UI32_T ifindex;
    UI16_T if_flags;
} RIP_MGR_IPC_Interface_T;

/*****************************************
 **      rip_mgr ipc msg structure      **
 *****************************************
 */
typedef struct RIP_MGR_IPCMsg_S
{
    union RIP_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;
    
    union
    {
        struct    
        {
            UI32_T      arg1;
            UI32_T      arg2;
        } arg_grp1;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
        } arg_grp2;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            struct pal_in4_addr arg3;
        } arg_grp3;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_grp4;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[20];
        } arg_grp5;
        
        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            UI32_T  arg3;
            char    arg4[17];
        } arg_grp6;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            char    arg3[20];
            char    arg4[16];
            UI32_T  arg5;
            UI32_T  arg6;
        } arg_grp7;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            RIP_TYPE_Timer_T arg3;
        } arg_grp8;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            UI32_T  arg3;
            struct pal_in4_addr arg4;
            UI32_T  arg5;
            char    arg6[20];
        } arg_grp9;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            RIP_TYPE_Route_Entry_T  arg3;
        } arg_grp10;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            RIP_TYPE_Peer_Entry_T  arg3;
        } arg_grp11;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            RIP_TYPE_Global_Statistics_T  arg3;
        } arg_grp12;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            UI32_T  arg3;
            UI32_T  arg4;
            UI32_T  arg5;
        } arg_grp13;

        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            Rip2IfStatEntry_T  arg3;
        } arg_grp14;
        struct    
        {
            UI32_T  arg1;
            UI32_T  arg2;
            Rip2IfConfEntry_T  arg3;
        } arg_grp15;

        struct
        {
            UI32_T  arg1;
            UI32_T  arg2;
            L_PREFIX_T arg3;
        } arg_grp16;

        RIP_MGR_IPC_Interface_T interface;
    } data;
} RIP_MGR_IPCMsg_T;

/* FUNCTION NAME:  RIP_MGR_SetTransitionMode
* PURPOSE:
*    This function will set transition state flag.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void RIP_MGR_SetTransitionMode(void);
  
/* FUNCTION NAME:  RIP_MGR_EnterTransitionMode
* PURPOSE:
*    This function will force RIP to enter transition state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void RIP_MGR_EnterTransitionMode(void);
  
/* FUNCTION NAME:  RIP_MGR_EnterMasterMode
* PURPOSE:
*    This function will force RIP to enter master state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void RIP_MGR_EnterMasterMode(void);
  
/* FUNCTION NAME:  RIP_MGR_EnterSlaveMode
* PURPOSE:
*    This function will force RIP to enter slave state.
*
* INPUT:
*    None.
*
* OUTPUT:
*    None.
*
* RETURN:
*    None.
*
* NOTES:
*    None.
*
*/
void RIP_MGR_EnterSlaveMode(void);

/* FUNCTION NAME : RIP_MGR_HandleIPCReqMsg
* PURPOSE:
*      Handle the ipc request received from mgr queue.
*
* INPUT:
*      sysfun_msg_p  --  The ipc request for RIP_MGR.
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
BOOL_T RIP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *sysfun_msg_p);

/* FUNCTION NAME : RIP_MGR_Debug
* PURPOSE:
*      RIP debug on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/

UI32_T RIP_MGR_Debug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigDebug
* PURPOSE:
*      RIP debug on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_UnDebug
* PURPOSE:
*      RIP undebug on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_UnDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigUnDebug
* PURPOSE:
*      RIP undebug on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigUnDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigDebugEvent
* PURPOSE:
*      RIP debug event on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DebugEvent
* PURPOSE:
*      RIP debug event on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_UnDebugEvent
* PURPOSE:
*      RIP undebug event on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_UnDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigUnDebugEvent
* PURPOSE:
*      RIP debug event on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigUnDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DebugPacket
* PURPOSE:
*      RIP debug packet on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_MGR_ConfigDebugPacket
* PURPOSE:
*      RIP debug packet on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_MGR_UnDebugPacket
* PURPOSE:
*      RIP undebug packet on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_UnDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_MGR_ConfigUnDebugPacket
* PURPOSE:
*      RIP undebug packet on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigUnDebugPacket(UI32_T vr_id, UI32_T instance,RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_MGR_DebugNsm
* PURPOSE:
*      RIP debug nsm on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigDebugNsm
* PURPOSE:
*      RIP debug nsm on config mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_UnDebugNsm
* PURPOSE:
*      RIP undebug nsm on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_UnDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ConfigUnDebugNsm
* PURPOSE:
*      RIP undebug nsm on exec mode.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ConfigUnDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ReceivePacketSet
* PURPOSE:
*     Set to let RIP reveive packet.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ReceivePacketSet(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_ReceivePacketUnset
* PURPOSE:
*     Set to not let RIP reveive packet.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ReceivePacketUnset(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_SendPacketSet
* PURPOSE:
*     Set to let RIP send packet.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SendPacketSet(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_SendPacketUnset
* PURPOSE:
*     Set to not let RIP send packet.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SendPacketUnset(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_ReceiveVersionSet
* PURPOSE:
*     Set the RIP receive version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      type.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ReceiveVersionSet(UI32_T vr_id, UI32_T instance, UI32_T ifindex, enum RIP_TYPE_Version_E type);

/* FUNCTION NAME : RIP_MGR_ReceiveVersionUnset
* PURPOSE:
*     Set RIP to reveive default version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_ReceiveVersionUnset(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_SendVersionSet
* PURPOSE:
*     Set the RIP send version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      type.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SendVersionSet(UI32_T vr_id, UI32_T instance, UI32_T ifindex, enum RIP_TYPE_Version_E type);

/* FUNCTION NAME : RIP_MGR_SendVersionUnset
* PURPOSE:
*     Set RIP to send default version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SendVersionUnset(UI32_T vr_id, UI32_T instance, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_AuthModeSet
* PURPOSE:
*     Set the RIP authentication mode.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      mode.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthModeSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, enum RIP_TYPE_Auth_Mode_E mode);

/* FUNCTION NAME : RIP_MGR_AuthModeUnset
* PURPOSE:
*     Set RIP authentication mode to default.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthModeUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_AuthStringSet
* PURPOSE:
*     Set the RIP authentication string.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      str.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthStringSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str);

/* FUNCTION NAME : RIP_MGR_AuthStringUnset
* PURPOSE:
*     Set RIP authentication string to default.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthStringUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_AuthKeyChainSet
* PURPOSE:
*     Set the RIP authentication key string.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      str.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthKeyChainSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str);

/* FUNCTION NAME : RIP_MGR_AuthKeyChainUnset
* PURPOSE:
*     Set RIP authentication key string to default.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AuthKeyChainUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_SplitHorizonSet
* PURPOSE:
*     Set the RIP split horizon.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      type.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SplitHorizonSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, enum RIP_TYPE_Split_Horizon_E type);

/* FUNCTION NAME : RIP_MGR_SplitHorizonUnset
* PURPOSE:
*     Not set RIP split horison.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_SplitHorizonUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_EnableRouterRip
* PURPOSE:
*      Enable RIP for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_EnableRouterRip(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DisableRouterRip
* PURPOSE:
*      Disable RIP for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DisableRouterRip(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_VersionSet
* PURPOSE:
*      Set RIP global version for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      version
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_VersionSet(UI32_T vr_id, UI32_T instance,enum RIP_TYPE_Global_Version_E version);

/* FUNCTION NAME : RIP_MGR_VersionUnset
* PURPOSE:
*      Set RIP into default value for a instance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id .
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_VersionUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_NetworkSetByVid
* PURPOSE:
*     Enable a network by vlan index.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      vid: vlan index
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_NetworkSetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid);

/* FUNCTION NAME : RIP_MGR_NetworkSetByAddress
* PURPOSE:
*     Enable a network by network address.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      address: network address
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_NetworkSetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address);

/* FUNCTION NAME : RIP_MGR_NetworkUnsetByVid
* PURPOSE:
*     Disable a network by vlan index
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      vid: vlan index
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_NetworkUnsetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid);

/* FUNCTION NAME : RIP_MGR_NetworkUnsetByAddress
* PURPOSE:
*     Disable a network by network address
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      address: network address
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_NetworkUnsetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address);

/* FUNCTION NAME : RIP_MGR_AddPassiveInterface
* PURPOSE:
*     Add a passive interface.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AddPassiveInterface(UI32_T vr_id, UI32_T instance, UI32_T vid);

/* FUNCTION NAME : RIP_MGR_DeletePassiveInterface
* PURPOSE:
*     Delete a passive interface.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      vid
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DeletePassiveInterface(UI32_T vr_id, UI32_T instance, UI32_T vid);

/* FUNCTION NAME : RIP_MGR_AddNeighbor
* PURPOSE:
*     Add a neighbor.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      addr:neighbor address
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AddNeighbor(UI32_T vr_id, UI32_T instance, struct pal_in4_addr * addr);

/* FUNCTION NAME : RIP_MGR_DeleteNeighbor
* PURPOSE:
*     Delete a neighbor.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      addr:neighbor address
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DeleteNeighbor(UI32_T vr_id, UI32_T instance, struct pal_in4_addr * addr);

/* FUNCTION NAME : RIP_MGR_DefaultInfoOriginate
* PURPOSE:
*     Originate default information.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DefaultInfoOriginate(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DeleteDefaultInfoOriginate
* PURPOSE:
*     Delete originate default information.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DeleteDefaultInfoOriginate(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DefaultMetricSet
* PURPOSE:
*     Set default metric.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DefaultMetricSet(UI32_T vr_id, UI32_T instance, UI32_T metric);

/* FUNCTION NAME : RIP_MGR_DefaultMetricUnset
* PURPOSE:
*     Set default metric to default value.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DefaultMetricUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DistributeListAdd
* PURPOSE:
*     Set distribute list.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      ifname,
*      list_name
*      type: in /out
*      list_type:access/prefix.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistributeListAdd(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                          enum RIP_TYPE_Distribute_Type_E type,
                                          enum RIP_TYPE_Distribute_List_Type_E list_type);

/* FUNCTION NAME : RIP_MGR_DistributeListDelete
* PURPOSE:
*     Delete distribute list.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      ifname,
*      list_name
*      type: in /out
*      list_type:access/prefix.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistributeListDelete(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                             enum RIP_TYPE_Distribute_Type_E type,
                                             enum RIP_TYPE_Distribute_List_Type_E list_type);

/* FUNCTION NAME : RIP_MGR_RedistributeSet
* PURPOSE:
*     Redistribute route by type.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      type_str.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RedistributeSet(UI32_T vr_id, UI32_T instance,char *type_str);

/* FUNCTION NAME : RIP_MGR_RedistributeMetricSet
* PURPOSE:
*     Redistribute route and set metric .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      type_str,
*      metric.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RedistributeMetricSet(UI32_T vr_id, UI32_T instance,char *type_str,UI32_T metric);

/* FUNCTION NAME : RIP_MGR_RedistributeRmapSet
* PURPOSE:
*     Redistribute route and set route map .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      type_str,
*      name.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RedistributeRmapSet(UI32_T vr_id, UI32_T instance,char *type_str,char *name);

/* FUNCTION NAME : RIP_MGR_RedistributeMetricRmapSet
* PURPOSE:
*     Redistribute route and set route map and set metric .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      type_str,
*      metric,
*      name.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RedistributeMetricRmapSet(UI32_T vr_id, UI32_T instance,char *type_str,UI32_T metric,char *name);

/* FUNCTION NAME : RIP_MGR_RedistributeUnset
* PURPOSE:
*     Not redistribute route by type.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      type_str.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RedistributeUnset(UI32_T vr_id, UI32_T instance,char *type_str);

/* FUNCTION NAME : RIP_MGR_TimerSet
* PURPOSE:
*     Set RIP timers value, include update time, timeout time,carbage collection time .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      RIP_TYPE_Timer_T.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_TimerSet(UI32_T vr_id, UI32_T instance, RIP_TYPE_Timer_T *timers);

/* FUNCTION NAME : RIP_MGR_TimerUnset
* PURPOSE:
*     Set RIP timers value to default value, include update time, timeout time,carbage collection time .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_TimerUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DistanceDefaultSet
* PURPOSE:
*     Set RIP distance value.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      distance.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistanceDefaultSet(UI32_T vr_id, UI32_T instance,UI32_T distance);

/* FUNCTION NAME : RIP_MGR_DistanceDefaultUnset
* PURPOSE:
*     Set RIP distance value to default.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistanceDefaultUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_DistanceSet
* PURPOSE:
*     Set RIP distance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      distance,
*      addr,
*      plen,
*      alist.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistanceSet(UI32_T vr_id, UI32_T instance,UI32_T distance,
                                    struct pal_in4_addr *addr, UI32_T plen, char *alist);

/* FUNCTION NAME : RIP_MGR_DistanceUnset
* PURPOSE:
*     Not set RIP distance.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      distance,
*      addr,
*      plen.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DistanceUnset(UI32_T vr_id, UI32_T instance,struct pal_in4_addr * addr,UI32_T plen);

/* FUNCTION NAME : RIP_MGR_MaxPrefixSet
* PURPOSE:
*     Set RIP max prefix value.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      pmax.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_MaxPrefixSet(UI32_T vr_id, UI32_T instance,UI32_T pmax);

/* FUNCTION NAME : RIP_MGR_MaxPrefixUnset
* PURPOSE:
*     Set max prefix value to default.
*
* INPUT:
*      vr_id,
*      instance : vrf_id.
*      
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_MaxPrefixUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_RecvBufSizeSet
* PURPOSE:
*     Set RIP receive buffer size.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      bufsize.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RecvBufSizeSet(UI32_T vr_id, UI32_T instance,UI32_T bufsize);

/* FUNCTION NAME : RIP_MGR_RecvBufSizeUnset
* PURPOSE:
*     Set RIP receive buffer size to default value.
*
* INPUT:
*      vr_id,
*      instance : vrf_id.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_RecvBufSizeUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_ClearRoute
* PURPOSE:
*     Clear rip router by type or by network address.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_MGR_ClearRoute(UI32_T vr_id, UI32_T instance,char *arg);

/* FUNCTION NAME : RIP_MGR_ClearStatistics
* PURPOSE:
*     Clear rip statistics.
*
* INPUT:
*      vr_id,
*      instance
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
UI32_T RIP_MGR_ClearStatistics(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_MGR_InterfaceAdd
* PURPOSE:
*     signal rip when interface add.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      interface
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
UI32_T RIP_MGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_InterfaceDelete
* PURPOSE:
*     signal rip when interface delete.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      interface
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
UI32_T RIP_MGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_LoopbackInterfaceAdd
* PURPOSE:
*     signal rip when loopback interface add.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex,
*      if_flags
*
* OUTPUT:
*      None.
*
* RETURN:
*       RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      
*/
UI32_T RIP_MGR_LoopbackInterfaceAdd(RIP_MGR_IPCMsg_T* ipcmsg_p);

/* FUNCTION NAME : RIP_MGR_InterfaceUp
 * PURPOSE:
 *     signal rip when interface up.
 *
 * INPUT:
 *      vr_id   -- VR ID
 *      vrf_id  -- VRF ID
 *      ifindex -- Interface ifindex
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
UI32_T RIP_MGR_InterfaceUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_InterfaceDown
 * PURPOSE:
 *     signal rip when interface down.
 *
 * INPUT:
 *      vr_id   -- VR ID
 *      vrf_id  -- VRF ID
 *      ifindex -- Interface ifindex
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
UI32_T RIP_MGR_InterfaceDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_MGR_IpAddressAdd
* PURPOSE:
*     signal rip when add an primary IP address.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      interface
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
UI32_T RIP_MGR_IpAddressAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                     struct pal_in4_addr *addr, UI32_T pfx_len);

/* FUNCTION NAME : RIP_MGR_IpAddressDelete
* PURPOSE:
*     signal rip when delete an primary IP address.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      interface
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
UI32_T RIP_MGR_IpAddressDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                       struct pal_in4_addr *addr, UI32_T pfx_len);

/* FUNCTION NAME : RIP_MGR_RifUp
 * PURPOSE:
 *     signal rip when rif up.
 *
 * INPUT:
 *      vr_id    --  VR ID
 *      vrf_id   --  VRF ID
 *      ifindex  --  interface index 
 *      ip_addr  --  RIF address
 *      ip_mask  --  RIF address mask
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
UI32_T RIP_MGR_RifUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : RIP_MGR_RifDown
 * PURPOSE:
 *     signal rip when rif down
 *
 * INPUT:
 *      vr_id    --  VR ID
 *      vrf_id   --  VRF ID
 *      ifindex  --  interface index 
 *      ip_addr  --  RIF address
 *      ip_mask  --  RIF address mask
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
UI32_T RIP_MGR_RifDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : RIP_MGR_GetNextRouteEntry
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
*       RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*       
*/
UI32_T RIP_MGR_GetNextRouteEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Route_Entry_T *entry);

/* FUNCTION NAME : RIP_MGR_GetNextThreadTimer
* PURPOSE:
*     Get next thread timer.
*
* INPUT:
*      vr_id,
*      vrf_id
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
BOOL_T RIP_MGR_GetNextThreadTimer(UI32_T vr_id, UI32_T vrf_id, UI32_T *nexttime);

/* FUNCTION NAME : RIP_MGR_GetNextPeerEntry
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
BOOL_T RIP_MGR_GetNextPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry);

/* FUNCTION NAME : RIP_MGR_GetPeerEntry
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
BOOL_T RIP_MGR_GetPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry);

/* FUNCTION NAME : RIP_MGR_GetGlobalStatistics
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
BOOL_T RIP_MGR_GetGlobalStatistics(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Global_Statistics_T *stat);
/* FUNCTION NAME : RIP_MGR_GetRip2IfStatTable
* PURPOSE:
*     Get interface state table
*
* INPUT:
*      vrf_id
*      Rip2IfStatEntry_T  data 
*
* OUTPUT:
*      Rip2IfStatEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_MGR_GetRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id,  Rip2IfStatEntry_T *data);

/* FUNCTION NAME : RIP_MGR_GetNextRip2IfStatTable
* PURPOSE:
*     Getnext interface state table
*
* INPUT:
*      vrf_id
*      Rip2IfStatEntry_T  data 
*
* OUTPUT:
*      Rip2IfStatEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_MGR_GetNextRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id,  Rip2IfStatEntry_T *data);
/* FUNCTION NAME : RIP_MGR_GetRip2IfConfTable
* PURPOSE:
*     Get interface configure table
*
* INPUT:
*      vrf_id
*      Rip2IfConfEntry_T  data 
*
* OUTPUT:
*      Rip2IfConfEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_MGR_GetRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id,  Rip2IfConfEntry_T *data);

/* FUNCTION NAME : RIP_MGR_GetNextRip2IfConfTable
* PURPOSE:
*     Getnext interface configure table
*
* INPUT:
*      vrf_id
*      Rip2IfConfEntry_T  data 
*
* OUTPUT:
*      Rip2IfConfEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_MGR_GetNextRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id,  Rip2IfConfEntry_T *data);



/* FUNCTION NAME : RIP_MGR_GetIfAddress
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
BOOL_T RIP_MGR_GetIfAddress(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *out_addr);

/* FUNCTION NAME : RIP_MGR_GetIfRecvBadPacket
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
BOOL_T RIP_MGR_GetIfRecvBadPacket(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : RIP_MGR_GetIfRecvBadRoute
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
BOOL_T RIP_MGR_GetIfRecvBadRoute(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : RIP_MGR_GetIfSendUpdate
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
BOOL_T RIP_MGR_GetIfSendUpdate(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value);
#if 0
/* FUNCTION NAME : RIP_MGR_AddRoute
* PURPOSE:
*     Add a route .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      addr:route address
*      plen: route prefix length
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_AddRoute(UI32_T vr_id,UI32_T instance,struct pal_in4_addr * addr,int plen);

/* FUNCTION NAME : RIP_MGR_DeleteRoute
* PURPOSE:
*     Delete a route .
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      addr:route address
*      plen: route prefix length
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_DeleteRoute(UI32_T vr_id,UI32_T instance,struct pal_in4_addr * addr,int plen);

/* FUNCTION NAME : RIP_MGR_OffsetListSet
* PURPOSE:
*     Set Offset list.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      alist,
*      direct_str,
*      metric,
*      ifname.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_OffsetListSet(UI32_T vr_id, UI32_T instance, char *alist, char *direct_str, int metric, char *ifname);

/* FUNCTION NAME : RIP_MGR_OffsetListUnset
* PURPOSE:
*     Delete Offset list.
*
* INPUT:
*      vr_id,
*      instance : vrf_id,
*      alist,
*      direct_str,
*      metric,
*      ifname.
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_OK/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_MGR_OffsetListUnset(UI32_T vr_id, UI32_T instance, char *alist, char *direct_str, int metric, char *ifname);
#endif


#endif

