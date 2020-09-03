/*-----------------------------------------------------------------------------
* FILE NAME: NETCFG_PMGR_RIP.H
*-----------------------------------------------------------------------------
* PURPOSE:
*    This file provides APIs for other process or CSC group to access NETCFG_MGR_RIP and NETCFG_OM_RIP service.
*    In Linux platform, the communication between CSC group are done via IPC.
*    Other CSC can call NETCFG_PMGR_XXX for APIs NETCFG_MGR_XXX provided by NETCFG
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
#ifndef NETCFG_PMGR_RIP_H
#define NETCFG_PMGR_RIP_H

#include <sys/types.h>
#include "sys_pal.h"
#include "netcfg_type.h"
#include "l_prefix.h"

/*-----------------------------------------------------------------------------
 * ROUTINE NAME : NETCFG_PMGR_RIP_InitiateProcessResource
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for NETCFG_PMGR_RIP in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  --  Sucess
 *           FALSE --  Error
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T NETCFG_PMGR_RIP_InitiateProcessResource(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebug
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebug(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_Debug
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
BOOL_T NETCFG_PMGR_RIP_Debug(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebug
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebug(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebug
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
BOOL_T NETCFG_PMGR_RIP_UnDebug(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugEvent(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugEvent(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugEvent
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
BOOL_T NETCFG_PMGR_RIP_DebugEvent(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugEvent
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
BOOL_T NETCFG_PMGR_RIP_UnDebugEvent(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugPacket
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
BOOL_T NETCFG_PMGR_RIP_DebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_UnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugPacket
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugPacket(NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_ConfigDebugNsm(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ConfigUnDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_ConfigUnDebugNsm(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DebugNsm
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
BOOL_T NETCFG_PMGR_RIP_DebugNsm(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_UnDebugNsm
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
BOOL_T NETCFG_PMGR_RIP_UnDebugNsm(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetDebugStatus
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
BOOL_T NETCFG_PMGR_RIP_GetDebugStatus(NETCFG_TYPE_RIP_Debug_Status_T *status);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvPacketSet
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
UI32_T NETCFG_PMGR_RIP_RecvPacketSet(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvPacketUnset
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
UI32_T NETCFG_PMGR_RIP_RecvPacketUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendPacketSet
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
UI32_T NETCFG_PMGR_RIP_SendPacketSet(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendPacketUnset
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
UI32_T NETCFG_PMGR_RIP_SendPacketUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvVersionTypeSet
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
UI32_T NETCFG_PMGR_RIP_RecvVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvVersionUnset
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
UI32_T NETCFG_PMGR_RIP_RecvVersionUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendVersionTypeSet
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
UI32_T NETCFG_PMGR_RIP_SendVersionTypeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Version_E type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SendVersionUnset
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
UI32_T NETCFG_PMGR_RIP_SendVersionUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthModeSet
* PURPOSE:
*     Set RIP authentication mode.
*
* INPUT:
*      ifindex,
*      mode:
*        text----- 1
*        md5------2
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
UI32_T NETCFG_PMGR_RIP_AuthModeSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Auth_Mode_E mode);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthModeUnset
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
UI32_T NETCFG_PMGR_RIP_AuthModeUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthStringSet
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
*     If the string is null string, please input "".
*/
UI32_T NETCFG_PMGR_RIP_AuthStringSet(UI32_T ifindex, char *str);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthStringUnset
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
UI32_T NETCFG_PMGR_RIP_AuthStringUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthKeyChainSet
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
*     If the string is null string, please input "".
*/
UI32_T NETCFG_PMGR_RIP_AuthKeyChainSet(UI32_T ifindex, char *str);

/* FUNCTION NAME : NETCFG_PMGR_RIP_AuthKeyChainUnset
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
UI32_T NETCFG_PMGR_RIP_AuthKeyChainUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SplitHorizonSet
* PURPOSE:
*     Set RIP split horizon.
*
* INPUT:
*      ifindex,
*      mode:
*        split horizon----- 1
*        split horizon poisoned------2
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
UI32_T NETCFG_PMGR_RIP_SplitHorizonSet(UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_SplitHorizonUnset
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
UI32_T NETCFG_PMGR_RIP_SplitHorizonUnset(UI32_T ifindex);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RouterRipSet
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
UI32_T NETCFG_PMGR_RIP_RouterRipSet(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RouterRipUnset
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
UI32_T NETCFG_PMGR_RIP_RouterRipUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_VersionSet
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
UI32_T NETCFG_PMGR_RIP_VersionSet(enum NETCFG_TYPE_RIP_Global_Version_E version);

/* FUNCTION NAME : NETCFG_PMGR_RIP_VersionUnset
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
UI32_T NETCFG_PMGR_RIP_VersionUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkSetByVid
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
UI32_T NETCFG_PMGR_RIP_NetworkSetByVid(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkSetByAddress
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
UI32_T NETCFG_PMGR_RIP_NetworkSetByAddress(L_PREFIX_T network_address);



/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkUnsetByVid
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
UI32_T NETCFG_PMGR_RIP_NetworkUnsetByVid(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_RIP_NetworkUnsetByAddress
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
UI32_T NETCFG_PMGR_RIP_NetworkUnsetByAddress(L_PREFIX_T network_address);

/* FUNCTION NAME : NETCFG_PMGR_RIP_NeighborSet
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
UI32_T NETCFG_PMGR_RIP_NeighborSet(UI32_T ip_addr);

/* FUNCTION NAME : NETCFG_PMGR_RIP_NeighborUnset
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
UI32_T NETCFG_PMGR_RIP_NeighborUnset(UI32_T ip_addr);

/* FUNCTION NAME : NETCFG_PMGR_RIP_PassiveIfAdd
* PURPOSE:
*     add passive interface.
*
* INPUT:
*     vid
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
UI32_T NETCFG_PMGR_RIP_PassiveIfAdd(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_RIP_PassiveIfDelete
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
UI32_T NETCFG_PMGR_RIP_PassiveIfDelete(UI32_T vid);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultAdd
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
UI32_T NETCFG_PMGR_RIP_DefaultAdd(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultDelete
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
UI32_T NETCFG_PMGR_RIP_DefaultDelete(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultMetricSet
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
UI32_T NETCFG_PMGR_RIP_DefaultMetricSet(UI32_T metric);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DefaultMetricUnset
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
UI32_T NETCFG_PMGR_RIP_DefaultMetricUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistributeListAdd
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
UI32_T NETCFG_PMGR_RIP_DistributeListAdd(char *ifname, char *list_name,
                                                      enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                      enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistributeListDelete
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
UI32_T NETCFG_PMGR_RIP_DistributeListDelete(char *ifname, char *list_name,
                                                         enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                         enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_PMGR_RIP_TimerSet
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
UI32_T NETCFG_PMGR_RIP_TimerSet(NETCFG_TYPE_RIP_Timer_T *timer);

/* FUNCTION NAME : NETCFG_PMGR_RIP_TimerUnset
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
UI32_T NETCFG_PMGR_RIP_TimerUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceDefaultSet
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
UI32_T NETCFG_PMGR_RIP_DistanceDefaultSet(UI32_T distance);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceDefaultUnset
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
UI32_T NETCFG_PMGR_RIP_DistanceDefaultUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceSet
* PURPOSE:
*     set distance .
*
* INPUT:
*      distance,
*      add,
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
UI32_T NETCFG_PMGR_RIP_DistanceSet(UI32_T distance, UI32_T addr, UI32_T plen, char *alist);

/* FUNCTION NAME : NETCFG_PMGR_RIP_DistanceUnset
* PURPOSE:
*     unset distance .
*
* INPUT:
*      add,
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
UI32_T NETCFG_PMGR_RIP_DistanceUnset(UI32_T addr, UI32_T plen);

/* FUNCTION NAME : NETCFG_PMGR_RIP_MaxPrefixSet
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
UI32_T NETCFG_PMGR_RIP_MaxPrefixSet(UI32_T pmax);

/* FUNCTION NAME : NETCFG_PMGR_RIP_MaxPrefixUnset
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
UI32_T NETCFG_PMGR_RIP_MaxPrefixUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvBuffSizeSet
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
UI32_T NETCFG_PMGR_RIP_RecvBuffSizeSet(UI32_T buff_size);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RecvBuffSizeUnset
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
UI32_T NETCFG_PMGR_RIP_RecvBuffSizeUnset(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeSet(char *protocol);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeMetricSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeMetricSet(char *protocol, UI32_T metric);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeRmapSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeRmapSet(char *protocol, char *rmap);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeAllSet
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
UI32_T NETCFG_PMGR_RIP_RedistributeAllSet(char *protocol, UI32_T metric, char *rmap);

/* FUNCTION NAME : NETCFG_PMGR_RIP_RedistributeUnset
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
UI32_T NETCFG_PMGR_RIP_RedistributeUnset(char *protocol);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ClearRoute
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
UI32_T NETCFG_PMGR_RIP_ClearRoute(char *arg);

/* FUNCTION NAME : NETCFG_PMGR_RIP_ClearStatistics
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
UI32_T NETCFG_PMGR_RIP_ClearStatistics(void);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetInstanceEntry
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
* NOTES: key is entry.instance
*      
*/
UI32_T NETCFG_PMGR_RIP_GetInstanceEntry(NETCFG_TYPE_RIP_Instance_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetInterfaceEntry
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
* NOTES:key is entry.ifindex
*      
*/
UI32_T NETCFG_PMGR_RIP_GetInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextInterfaceEntry
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
UI32_T NETCFG_PMGR_RIP_GetNextInterfaceEntry(NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNetworkTable
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
* NOTES:key are p.prefix and p.u.prefix4.s_addr
*     
*/
UI32_T NETCFG_PMGR_RIP_GetNetworkTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextNetworkTable
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
UI32_T NETCFG_PMGR_RIP_GetNextNetworkTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNeighborTable
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
* NOTES: key is p.u.prefix4.s_addr 
*     
*/
UI32_T NETCFG_PMGR_RIP_GetNeighborTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextNeighborTable
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
UI32_T NETCFG_PMGR_RIP_GetNextNeighborTable(NETCFG_TYPE_RIP_Network_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetDistanceTable
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
*      key are entry.p.prefixlen and entry.p.u.prefix4.s_addr
*/
UI32_T NETCFG_PMGR_RIP_GetDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextDistanceTable
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
UI32_T NETCFG_PMGR_RIP_GetNextDistanceTable(NETCFG_TYPE_RIP_Distance_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextRouteEntry
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
UI32_T NETCFG_PMGR_RIP_GetNextRouteEntry(NETCFG_TYPE_RIP_Route_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextThreadTimer
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
BOOL_T NETCFG_PMGR_RIP_GetNextThreadTimer(UI32_T *nexttime);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextPeerEntry
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
*       key is peer address.
*/
BOOL_T NETCFG_PMGR_RIP_GetNextPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetPeerEntry
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
*       key is peer address.
*/
BOOL_T NETCFG_PMGR_RIP_GetPeerEntry(NETCFG_TYPE_RIP_Peer_Entry_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetGlobalStatistics
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
BOOL_T NETCFG_PMGR_RIP_GetGlobalStatistics(NETCFG_TYPE_RIP_Global_Statistics_T *stat);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfAddress
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
BOOL_T NETCFG_PMGR_RIP_GetIfAddress(UI32_T exact, UI32_T in_addr, UI32_T *out_addr);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfRecvBadPacket
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
BOOL_T NETCFG_PMGR_RIP_GetIfRecvBadPacket(UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfRecvBadRoute
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
BOOL_T NETCFG_PMGR_RIP_GetIfRecvBadRoute(UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetIfSendUpdate
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
BOOL_T NETCFG_PMGR_RIP_GetIfSendUpdate(UI32_T exact, UI32_T in_addr, UI32_T *value);
/* FUNCTION NAME : NETCFG_PMGR_RIP_GetRedistributeTable
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
UI32_T NETCFG_PMGR_RIP_GetRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextRedistributeTable
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
UI32_T NETCFG_PMGR_RIP_GetNextRedistributeTable(NETCFG_TYPE_RIP_Redistribute_Table_T *entry);

/* FUNCTION NAME : NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex
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
UI32_T NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

#endif /* NETCFG_PMGR_RIP_H */

