/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_MGR_OSPF.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_MGR_OSPF_H
#define NETCFG_MGR_OSPF_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "l_radix.h"
#include "ospf_type.h"

#define NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE sizeof(union NETCFG_MGR_OSPF_IPCMsg_Type_U)

#define NETCFG_MGR_OSPF_AUTH_SIMPLE_SIZE           8
#define NETCFG_MGR_OSPF_AUTH_MD5_SIZE              16
/* command used in IPC message
 */
enum
{
    NETCFG_MGR_OSPF_IPC_ROUTEROSPFSET,
    NETCFG_MGR_OSPF_IPC_ROUTEROSPFUNSET,
    NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPESET,
    NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONTYPEUNSET,
    NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYSET,
    NETCFG_MGR_OSPF_IPC_IFAUTHENTICATIONKEYUNSET,
    NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYSET,
    NETCFG_MGR_OSPF_IPC_IFMESSAGEDIGESTKEYUNSET,
    NETCFG_MGR_OSPF_IPC_IFPRIORITYSET,
    NETCFG_MGR_OSPF_IPC_IFPRIORITYUNSET,
    NETCFG_MGR_OSPF_IPC_IFCOSTSET,
    NETCFG_MGR_OSPF_IPC_IFCOSTUNSET,
    NETCFG_MGR_OSPF_IPC_IFDEADINTERVALSET,
    NETCFG_MGR_OSPF_IPC_IFDEADINTERVALUNSET,
    NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALSET,
    NETCFG_MGR_OSPF_IPC_IFHELLOINTERVALUNSET,
    NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALSET,
    NETCFG_MGR_OSPF_IPC_IFRETRANSMITINTERVALUNSET,
    NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYSET,
    NETCFG_MGR_OSPF_IPC_IFTRANSMITDELAYUNSET,
    NETCFG_MGR_OSPF_IPC_NETWORKSET,
    NETCFG_MGR_OSPF_IPC_NETWORKUNSET,
    NETCFG_MGR_OSPF_IPC_ROUTERIDSET,
    NETCFG_MGR_OSPF_IPC_ROUTERIDUNSET,
    NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583SET,
    NETCFG_MGR_OSPF_IPC_COMPATIBLERFC1583UNSET,
    NETCFG_MGR_OSPF_IPC_PASSIVEIFSET,
    NETCFG_MGR_OSPF_IPC_PASSIVEIFUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTMETRICSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTMETRICUNSET,
    NETCFG_MGR_OSPF_IPC_TIMERSET,
    NETCFG_MGR_OSPF_IPC_TIMERUNSET,
    NETCFG_MGR_OSPF_IPC_AREASTUBSET,
    NETCFG_MGR_OSPF_IPC_AREASTUBUNSET,
    NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYSET,
    NETCFG_MGR_OSPF_IPC_AREASTUBNOSUMMARYUNSET,
    NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTSET,
    NETCFG_MGR_OSPF_IPC_AREADEFAULTCOSTUNSET,
    NETCFG_MGR_OSPF_IPC_AREARANGESET,
    NETCFG_MGR_OSPF_IPC_AREARANGENOADVERTISESET,
    NETCFG_MGR_OSPF_IPC_AREARANGEUNSET,  
	NETCFG_MGR_OSPF_IPC_AREANSSASET,
	NETCFG_MGR_OSPF_IPC_AREANSSAUNSET,
    NETCFG_MGR_OSPF_IPC_AREAVLINKSET,
    NETCFG_MGR_OSPF_IPC_AREAVLINKUNSET,
	NETCFG_MGR_OSPF_IPC_GETINSTANCEPARA,
    NETCFG_MGR_OSPF_IPC_GETNEXTPASSIVEIF,
    NETCFG_MGR_OSPF_IPC_GETNEXTNETWORK,

    NETCFG_MGR_OSPF_IPC_SUMMARY_ADDRSET,
    NETCFG_MGR_OSPF_IPC_SUMMAYR_ADDRUNSET,
    NETCFG_MGR_OSPF_IPC_AUTOCOSTSET,
    NETCFG_MGR_OSPF_IPC_AUTOCOSTUNSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_PROTOUNSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICUNSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPESET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_METRICTYPEUNSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_ROUTEMAPUNSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGSET,
    NETCFG_MGR_OSPF_IPC_REDISTRIBUTE_TAGUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPESET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_METRICTYPEUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ROUTEMAPUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWAYSSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_ALWWAYSUNSET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_SET,
    NETCFG_MGR_OSPF_IPC_DEFAULTINFO_UNSET,
    NETCFG_MGR_OSPF_IPC_GETNEXT_NBR_ENTRY,
    NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRY,
    NETCFG_MGR_OSPF_IPC_GETOSPFIFENTRYBYIFINDEX,
    NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRY,
    NETCFG_MGR_OSPF_IPC_GETNEXTOSPFIFENTRYBYIFINDEX,
    NETCFG_MGR_OSPF_IPC_GETRUNNINGIFENTRYBYIFINDEX,
    NETCFG_MGR_OSPF_IPC_GETNEXT_SUMMARY_ADDR,  
    NETCFG_MGR_OSPF_IPC_GET_REDIST_AUTOCOST,
    NETCFG_MGR_OSPF_IPC_GET_REDIST_CONFIG,
    NETCFG_MGR_OSPF_IPC_GET_DEFAULT_INFO_ORIGINATE,

};

/* Macro function for computation of IPC msg_buf size based on field name
 * used in NETCFG_MGR_OSPF_IPCMsg_T.data
 */
#define NETCFG_MGR_OSPF_GET_MSGBUF_SIZE(msg_data_type) \
    (NETCFG_MGR_OSPF_IPCMSG_TYPE_SIZE + sizeof(msg_data_type))

typedef struct NETCFG_MGR_REDIST_CONFIG_S
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI8_T  proto[20];
    UI32_T flags;
    UI32_T metric;
    UI32_T metric_type;
    UI32_T tag;
    UI8_T  route_map[20];
    UI8_T  default_origin;
}NETCFG_MGR_REDIST_CONFIG_T;

typedef struct NETCFG_MGR_SUMMARY_CONFIG_S
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T addr;
    UI32_T masklen;
}NETCFG_MGR_SUMMARY_CONFIG_T;

/*****************************************
 **      netcfg_mgr_ospf  ipc msg structure      **
 *****************************************
 */
typedef struct NETCFG_MGR_OSPF_IPCMsg_S
{
    union NETCFG_MGR_OSPF_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request command */
        BOOL_T result_bool;      /*respond bool return*/
        UI32_T result_ui32;      /*respond ui32 return*/ 
        SYS_TYPE_Get_Running_Cfg_T         result_running_cfg; /* For running config API */
    } type;
    
    union
    {
        NETCFG_MGR_SUMMARY_CONFIG_T summary_addr_entry;
        NETCFG_TYPE_OSPF_Instance_Para_T arg_instance_para;

        NETCFG_MGR_REDIST_CONFIG_T redist_entry;
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
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
        } arg_grp3;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
        } arg_grp4;
        
         struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            UI32_T   arg5;
            UI32_T   arg6;
        } arg_grp5;
       
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            char     arg4[40];
        } arg_grp6;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI8_T    arg3;          
            BOOL_T   arg4;
            struct pal_in4_addr arg5;
        } arg_grp7;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;        
            BOOL_T   arg3;
            struct pal_in4_addr arg4;
        } arg_grp8;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;            
            BOOL_T   arg4;
            struct pal_in4_addr arg5;
        } arg_grp9;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE + 1];
            BOOL_T   arg4;
            struct pal_in4_addr arg5;           
        } arg_grp10;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI8_T    arg3;
            char     arg4[NETCFG_TYPE_OSPF_AUTH_MD5_SIZE + 1];
            BOOL_T   arg5;
            struct pal_in4_addr arg6;           
        } arg_grp11;

        struct    
        {
            UI32_T   arg1;
            OSPF_TYPE_OspfInterfac_T arg2;       
        } arg_grp12;

        struct    
        {
            UI32_T   arg1;
            NETCFG_TYPE_OSPF_IfConfig_T arg2;       
        } arg_grp13;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            NETCFG_TYPE_OSPF_Area_Nssa_Para_T   arg5;
        } arg_grp14;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            NETCFG_TYPE_OSPF_Passive_If_T   arg3;
        } arg_grp15;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            NETCFG_TYPE_OSPF_Network_T arg3;
        } arg_grp16;
        
        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            UI32_T   arg3;
            UI32_T   arg4;
            NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T arg5;
        } arg_grp17;

        struct    
        {
            UI32_T   arg1;
            UI32_T   arg2;
            char     arg3[20];
            char     arg4[20];
        } arg_grp_route_map;
    } data;
} NETCFG_MGR_OSPF_IPCMsg_T;


/* FUNCTION NAME : NETCFG_MGR_OSPF_InitiateProcessResources
 * PURPOSE:
 *      Initialize NETCFG_MGR_OSPF used system resource, eg. protection semaphore.
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
BOOL_T NETCFG_MGR_OSPF_InitiateProcessResources(void);

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterMasterMode
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
void NETCFG_MGR_OSPF_EnterMasterMode (void); 

/* FUNCTION NAME : NETCFG_MGR_OSPF_ProvisionComplete
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
void NETCFG_MGR_OSPF_ProvisionComplete(void); 

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterSlaveMode
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
void NETCFG_MGR_OSPF_EnterSlaveMode (void);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SetTransitionMode
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
void NETCFG_MGR_OSPF_SetTransitionMode(void);

/* FUNCTION NAME : NETCFG_MGR_OSPF_EnterTransitionMode
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
void NETCFG_MGR_OSPF_EnterTransitionMode (void);   

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - NETCFG_MGR_OSPF_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for OSPF MGR.
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
BOOL_T NETCFG_MGR_OSPF_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p); 

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterOspfSet
* PURPOSE:
*     Set router ospf.
*
* INPUT:
*      msg_p -- ospf message buf.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterOspfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterOspfUnset
* PURPOSE:
*     Unset router ospf.
*
* INPUT:
*      msg_p -- ospf message buf.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterOspfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalInterfaceAdd
* PURPOSE:
*     When add an l3 interface signal OSPF.
*
* INPUT:
*      ifindex.
*      mtu
*      bandwidth
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
UI32_T NETCFG_MGR_OSPF_SignalInterfaceAdd(UI32_T ifindex, UI32_T mtu, UI32_T bandwidth, UI32_T if_flags);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalInterfaceDelete
* PURPOSE:
*     When delete an l3 interface signal OSPF.
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
UI32_T NETCFG_MGR_OSPF_SignalInterfaceDelete(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalLoopbackInterfaceAdd
* PURPOSE:
*     When add a loopback interface signal OSPF.
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
UI32_T NETCFG_MGR_OSPF_SignalLoopbackInterfaceAdd(UI32_T ifindex, UI16_T if_flags);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifCreate
* PURPOSE:
*     When an IP address create signal OSPF.
*
* INPUT:
*      ifindex,
*      ipaddr,
*      ipmask,
*      primary: flag for if is primary rif
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
UI32_T NETCFG_MGR_OSPF_SignalRifCreate(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifDelete
* PURPOSE:
*     When an IP address Delete signal OSPF.
*
* INPUT:
*      ifindex,
*      ipaddr,
*      ipmask,
*      primary: flag for if is primary rif
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
UI32_T NETCFG_MGR_OSPF_SignalRifDelete(UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask, UI32_T primary);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifUp
* PURPOSE:
*     When an l3 interface primary rif up signal OSPF.
*
* INPUT:
*      ifindex,
*     
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
UI32_T NETCFG_MGR_OSPF_SignalRifUp(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_OSPF_SignalRifDown
* PURPOSE:
*     When an l3 interface primary rif down signal OSPF.
*
* INPUT:
*      ifindex,
*     
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
UI32_T NETCFG_MGR_OSPF_SignalRifDown(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_MGR_OSPF_IfAuthenticationTypeSet
* PURPOSE:
*     Set OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_MGR_OSPF_IfAuthenticationTypeUnset
* PURPOSE:
*     Unset OSPF interface authentication type.
*
* INPUT:
*      vr_id,
*      ifindex 
*      type
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfAuthenticationKeySet
* PURPOSE:
*      Set OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfAuthenticationKeyUnset
* PURPOSE:
*      Unset OSPF interface authentication key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfMessageDigestKeySet
* PURPOSE:
*      Set OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      auth_key
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfMessageDigestKeyUnset
* PURPOSE:
*      Unset OSPF interface message digest key.
*
* INPUT:
*      vr_id,
*      ifindex 
*      key_id
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfPrioritySet
* PURPOSE:
*      Set OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      priority
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfPriorityUnset
* PURPOSE:
*      Unset OSPF interface priority.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfCostSet
* PURPOSE:
*      Set OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      cost
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfCostUnset
* PURPOSE:
*      Unset OSPF interface cost.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfDeadIntervalSet
* PURPOSE:
*      Set OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfDeadIntervalUnset
* PURPOSE:
*      Unset OSPF interface dead interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfHelloIntervalSet
* PURPOSE:
*      Set OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfHelloIntervalUnset
* PURPOSE:
*      Unset OSPF interface hello interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfRetransmitIntervalSet
* PURPOSE:
*      Set OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      interval
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfRetransmitIntervalUnset
* PURPOSE:
*      Unset OSPF interface retransmit interval.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfTransmitDelaySet
* PURPOSE:
*       Set OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      delay
*      addr_ flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_MGR_OSPF_IfTransmitDelayUnset
* PURPOSE:
*      Unset OSPF interface transmit delay.
*
* INPUT:
*      vr_id,
*      ifindex 
*      addr_flag
*      addr
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_MGR_OSPF_NetworkSet
* PURPOSE:
*     Set ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_NetworkUnset
* PURPOSE:
*     Delete ospf network.
*
* INPUT:
*      vr_id,
*      proc_id,
*      network_addr,
*      masklen,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T network_addr, UI32_T masklen, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterIdSet
* PURPOSE:
*     Set ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*      router_id.
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
UI32_T NETCFG_MGR_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_RouterIdUnset
* PURPOSE:
*     Unset ospf router id.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_TimerSet
* PURPOSE:
*     Set ospf timer.
*
* INPUT:
*      vr_id,
*      proc_id,
*      delay,
*      hold.
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
UI32_T NETCFG_MGR_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, UI32_T delay, UI32_T hold);

/* FUNCTION NAME : NETCFG_MGR_OSPF_TimerUnset
* PURPOSE:
*     Set ospf timer to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
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
UI32_T NETCFG_MGR_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
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
UI32_T NETCFG_MGR_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : NETCFG_MGR_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
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
UI32_T NETCFG_MGR_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
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
UI32_T NETCFG_MGR_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr);

/* FUNCTION NAME : NETCFG_MGR_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf passive interface.
*
* INPUT:
*      vr_id,
*      proc_id,
*      ifname,
*      addr.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, char *ifname, UI32_T addr);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetNextPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
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
UI32_T NETCFG_MGR_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
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
UI32_T NETCFG_MGR_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubSet
* PURPOSE:
*     Set ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubUnset
* PURPOSE:
*     Unset ospf area stub.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubNoSummarySet
* PURPOSE:
*     Set ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubNoSummarySet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaStubNoSummaryUnset
* PURPOSE:
*     Unset ospf area stub no summary.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaStubNoSummaryUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaDefaultCostUnset
* PURPOSE:
*     Unset ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaDefaultCostUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeSet
* PURPOSE:
*     Set ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeNoAdvertiseSet
* PURPOSE:
*     Set ospf area range no advertise.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeNoAdvertiseSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaRangeUnset
* PURPOSE:
*     Unset ospf area range.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      range_addr,
*      range_masklen.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaRangeUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T range_addr, UI32_T range_masklen);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaNssaSet
* PURPOSE:
*     Set ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      nssa_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaNssaSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Nssa_Para_T *nssa_para);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaNssaUnset
* PURPOSE:
*     Unset ospf area nssa.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      flag.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaNssaUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T flag);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextNetwork
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_MGR_OSPF_GetNextNetwork(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaVirtualLinkSet
* PURPOSE:
*     Set ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaVirtualLinkSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : NETCFG_MGR_OSPF_AreaVirtualLinkUnset
* PURPOSE:
*     Unset ospf area virtual link.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      vlink_para.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_MGR_OSPF_AreaVirtualLinkUnset(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, NETCFG_TYPE_OSPF_Area_Virtual_Link_Para_T *vlink_para);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetOspfIfEntry
* PURPOSE:
*     Get ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
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
UI32_T NETCFG_MGR_OSPF_GetOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetOspfIfEntryByIfindex
* PURPOSE:
*     Get ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry
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
UI32_T NETCFG_MGR_OSPF_GetOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextOspfIfEntry
* PURPOSE:
*     Get next ospf interface entry.
*
* INPUT:
*      vr_id,
*      entry
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
UI32_T NETCFG_MGR_OSPF_GetNextOspfIfEntry(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetNextOspfIfEntryByIfindex
* PURPOSE:
*     Get next ospf interface entry by ifindex.
*
* INPUT:
*      vr_id,
*      entry
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
UI32_T NETCFG_MGR_OSPF_GetNextOspfIfEntryByIfindex(UI32_T vr_id, OSPF_TYPE_OspfInterfac_T *entry);

/* FUNCTION NAME : NETCFG_MGR_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
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
UI32_T NETCFG_MGR_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry);

#endif /* NETCFG_MGR_OSPF_H */

