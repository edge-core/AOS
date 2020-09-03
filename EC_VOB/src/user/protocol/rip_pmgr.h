/* MODULE NAME:  rip_pmgr.h
 * PURPOSE:
 *     RIP PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    05/12/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef RIP_PMGR_H
#define RIP_PMGR_H

#include "sys_type.h"
#include "rip_mgr.h"
#include "rip_type.h"

/* FUNCTION NAME : RIP_PMGR_InitiateProcessResource
* PURPOSE:
*      Initiate process resources for RIP_PMGR.
*
* INPUT:
*      None.
*
* OUTPUT:
*      None.
*
* RETURN:
*      TRUE   -- Success
*      FALSE  -- Fail
*
* NOTES:
*      None.
*/
BOOL_T RIP_PMGR_InitiateProcessResource(void);

/* FUNCTION NAME : RIP_PMGR_Debug
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
UI32_T RIP_PMGR_Debug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigDebug
* PURPOSE:
*     RIP debug on config mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_UnDebug
* PURPOSE:
*     RIP undebug on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_UnDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigUnDebug
* PURPOSE:
*     RIP undebug on CONFIG mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigUnDebug(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DebugEvent
* PURPOSE:
*     RIP debug event on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigDebugEvent
* PURPOSE:
*     RIP debug event on config mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_UnDebugEvent
* PURPOSE:
*     RIP undebug event on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_UnDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigUnDebugEvent
* PURPOSE:
*     RIP undebug event on config mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigUnDebugEvent(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
*      type
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_DebugPacket(UI32_T vr_id, UI32_T instance, RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_PMGR_ConfigDebugPacket
* PURPOSE:
*     RIP debug packet on config mode.
*
* INPUT:
*      vr_id,
*      instance,
*       type
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
UI32_T RIP_PMGR_ConfigDebugPacket(UI32_T vr_id, UI32_T instance,
                                             RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_PMGR_UnDebugPacket
* PURPOSE:
*     RIP undebug packet on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
*       type
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_UnDebugPacket(UI32_T vr_id, UI32_T instance,
                                         RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_PMGR_ConfigUnDebugPacket
* PURPOSE:
*     RIP undebug packet on config mode.
*
* INPUT:
*      vr_id,
*      instance,
*      type
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_ConfigUnDebugPacket(UI32_T vr_id, UI32_T instance,
                                                RIP_TYPE_Packet_Debug_Type_T type);

/* FUNCTION NAME : RIP_PMGR_DebugNsm
* PURPOSE:
*     RIP debug nsm on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigDebugNsm
* PURPOSE:
*     RIP debug nsm on config mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_UnDebugNsm
* PURPOSE:
*     RIP undebug nsm on EXEC mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_UnDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_ConfigUnDebugNsm
* PURPOSE:
*     RIP undebug nsm on config mode.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_ConfigUnDebugNsm(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_RecvPacketSet
* PURPOSE:
*     Set receive packet on an interface.
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
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RecvPacketSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_RecvPacketUnset
* PURPOSE:
*     Unset receive packet on an interface.
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
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RecvPacketUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_SendPacketSet
* PURPOSE:
*     set send packet on an interface.
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
*       RIP_TYPE_RESULT/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_SendPacketSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_SendPacketUnset
* PURPOSE:
*     Unset send packet on an interface.
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
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_SendPacketUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_RecvVersionTypeSet
* PURPOSE:
*     Set RIP receive version.
*
* INPUT:
*      vr_id,
*      instance,
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
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RecvVersionTypeSet(UI32_T vr_id, UI32_T instance,
                                              UI32_T ifindex, enum RIP_TYPE_Version_E type);

/* FUNCTION NAME : RIP_PMGR_RecvVersionUnset
* PURPOSE:
*     unset RIP receive version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*
* OUTPUT:
*      None.
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RecvVersionUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_SendVersionTypeSet
* PURPOSE:
*     Set RIP send version.
*
* INPUT:
*      vr_id,
*      instance,
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
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_SendVersionTypeSet(UI32_T vr_id, UI32_T instance,
                                              UI32_T ifindex, enum RIP_TYPE_Version_E type);

/* FUNCTION NAME : RIP_PMGR_SendVersionUnset
* PURPOSE:
*     unset RIP send version.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*
* OUTPUT:
*      None
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_SendVersionUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_AuthModeSet
* PURPOSE:
*     Set RIP authentication mode.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      mode:
*        text----- 1
*        md5------2
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
UI32_T RIP_PMGR_AuthModeSet(UI32_T vr_id, UI32_T instance,
                                      UI32_T ifindex, enum RIP_TYPE_Auth_Mode_E mode);

/* FUNCTION NAME : RIP_PMGR_AuthModeUnset
* PURPOSE:
*     Unset RIP authentication mode.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
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
UI32_T RIP_PMGR_AuthModeUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_AuthStringSet
* PURPOSE:
*     set RIP authentication string.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*     If the string is null string, please input "".
*/
UI32_T RIP_PMGR_AuthStringSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str);

/* FUNCTION NAME : RIP_PMGR_AuthStringUnset
* PURPOSE:
*     Unset RIP authentication string.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
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
UI32_T RIP_PMGR_AuthStringUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_AuthKeyChainSet
* PURPOSE:
*     set RIP authentication key-chain.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      str
*
* OUTPUT:
*      None.
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*     If the string is null string, please input "".
*/
UI32_T RIP_PMGR_AuthKeyChainSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex, char *str);

/* FUNCTION NAME : RIP_PMGR_AuthKeyChainUnset
* PURPOSE:
*     Unset RIP authentication key-chain.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
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
UI32_T RIP_PMGR_AuthKeyChainUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_SplitHorizonSet
* PURPOSE:
*     Set RIP split horizon.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex,
*      mode:
*        split horizon----- 1
*        split horizon poisoned------2
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
UI32_T RIP_PMGR_SplitHorizonSet(UI32_T vr_id, UI32_T instance,UI32_T ifindex,
                                        enum RIP_TYPE_Split_Horizon_E type);

/* FUNCTION NAME : RIP_PMGR_SplitHorizonUnset
* PURPOSE:
*     unset rip spliet horizon.
*
* INPUT:
*      vr_id,
*      instance,
*      ifindex.
*
* OUTPUT:
*      None
*
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_SplitHorizonUnset(UI32_T vr_id, UI32_T instance,UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_RouterRipSet
* PURPOSE:
*     Set router rip.
*
* INPUT:
*      vr_id,
*      instance,
*
* OUTPUT:
*      None
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RouterRipSet(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_RouterRipUnset
* PURPOSE:
*     unset router rip.
*
* INPUT:
*      vr_id,
*      instance,
*
* OUTPUT:
*      None,
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_RouterRipUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_VersionSet
* PURPOSE:
*     set rip version.
*
* INPUT:
*      vr_id,
*      instance,
*      version.
*
* OUTPUT:
*      None
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_VersionSet(UI32_T vr_id, UI32_T instance,enum RIP_TYPE_Global_Version_E version);

/* FUNCTION NAME : RIP_PMGR_VersionUnset
* PURPOSE:
*     unset rip version.
*
* INPUT:
*      vr_id,
*      instance,
*
* OUTPUT:
*      None,
* RETURN:
*      RIP_TYPE_RESULT_SUCCESS/RIP_TYPE_RESULT_FAIL
*
* NOTES:
*      None.
*/
UI32_T RIP_PMGR_VersionUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_NetworkSetByVid
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
UI32_T RIP_PMGR_NetworkSetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid);

/* FUNCTION NAME : RIP_PMGR_NetworkSetByAddress
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
UI32_T RIP_PMGR_NetworkSetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address);

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
UI32_T RIP_PMGR_NetworkUnsetByVid(UI32_T vr_id, UI32_T instance,UI32_T vid);

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
UI32_T RIP_PMGR_NetworkUnsetByAddress(UI32_T vr_id, UI32_T instance,L_PREFIX_T network_address);

/* FUNCTION NAME : RIP_PMGR_NeighborSet
* PURPOSE:
*     set rip neighbor
*
* INPUT:
*      vr_id,
*      instance,
*      addr.
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
UI32_T RIP_PMGR_NeighborSet(UI32_T vr_id, UI32_T instance,struct pal_in4_addr *addr);

/* FUNCTION NAME : RIP_PMGR_NeighborUnset
* PURPOSE:
*     unset rip neighbor
*
* INPUT:
*      vr_id,
*      instance,
*      addr.
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
UI32_T RIP_PMGR_NeighborUnset(UI32_T vr_id, UI32_T instance,struct pal_in4_addr *addr);

/* FUNCTION NAME : RIP_PMGR_PassiveIfAdd
* PURPOSE:
*     add passive interface.
*
* INPUT:
*      vr_id,
*      instance,
*      vid
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
UI32_T RIP_PMGR_PassiveIfAdd(UI32_T vr_id, UI32_T instance,UI32_T vid);

/* FUNCTION NAME : RIP_PMGR_PassiveIfDelete
* PURPOSE:
*     delete passive interface.
*
* INPUT:
*      vr_id,
*      instance,
*      vid
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
UI32_T RIP_PMGR_PassiveIfDelete(UI32_T vr_id, UI32_T instance,UI32_T vid);

/* FUNCTION NAME : RIP_PMGR_DefaultAdd
* PURPOSE:
*     originate rip default information.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DefaultAdd(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DefaultDelete
* PURPOSE:
*     not originate rip default information.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DefaultDelete(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DefaultMetricSet
* PURPOSE:
*     set rip default metric.
*
* INPUT:
*      vr_id,
*      instance,
*      metric.
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
UI32_T RIP_PMGR_DefaultMetricSet(UI32_T vr_id, UI32_T instance,UI32_T metric);

/* FUNCTION NAME : RIP_PMGR_DefaultMetricUnset
* PURPOSE:
*     unset rip default metric.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DefaultMetricUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DistributeListAdd
* PURPOSE:
*     add distribute list.
*
* INPUT:
*      vr_id,
*      instance,
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
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
UI32_T RIP_PMGR_DistributeListAdd(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                          enum RIP_TYPE_Distribute_Type_E type,
                                          enum RIP_TYPE_Distribute_List_Type_E list_type);

/* FUNCTION NAME : RIP_PMGR_DistributeListDelete
* PURPOSE:
*     delete distribute list.
*
* INPUT:
*      vr_id,
*      instance,
*      ifname.
*      list_name,
*      distribute type: in/out,
*      list type: access/prefix
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
UI32_T RIP_PMGR_DistributeListDelete(UI32_T vr_id, UI32_T instance,char *ifname, char *list_name,
                                             enum RIP_TYPE_Distribute_Type_E type,
                                             enum RIP_TYPE_Distribute_List_Type_E list_type);

/* FUNCTION NAME : RIP_PMGR_TimerSet
* PURPOSE:
*     set timer value.
*
* INPUT:
*      vr_id,
*      instance,
*      timer: update, timeout,carbage.
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
UI32_T RIP_PMGR_TimerSet(UI32_T vr_id, UI32_T instance, RIP_TYPE_Timer_T *timer);

/* FUNCTION NAME : RIP_PMGR_TimerUnset
* PURPOSE:
*     unset timer.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_TimerUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DistanceDefaultSet
* PURPOSE:
*     set distance value.
*
* INPUT:
*      vr_id,
*      instance,
*      distance.
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
UI32_T RIP_PMGR_DistanceDefaultSet(UI32_T vr_id, UI32_T instance,UI32_T distance);

/* FUNCTION NAME : RIP_PMGR_DistanceDefaultUnset
* PURPOSE:
*     unset distance value.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_DistanceDefaultUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_DistanceSet
* PURPOSE:
*     set distance .
*
* INPUT:
*      vr_id,
*      instance,
*      distance,
*      add,
*      plen: prefix length
*      alist.
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
UI32_T RIP_PMGR_DistanceSet(UI32_T vr_id, UI32_T instance,UI32_T distance,
                                    struct pal_in4_addr *addr, UI32_T plen, char *alist);

/* FUNCTION NAME : RIP_PMGR_DistanceUnset
* PURPOSE:
*     unset distance .
*
* INPUT:
*      vr_id,
*      instance,
*      add,
*      plen: prefix length
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
UI32_T RIP_PMGR_DistanceUnset(UI32_T vr_id, UI32_T instance,
                                       struct pal_in4_addr *addr, UI32_T plen);

/* FUNCTION NAME : RIP_PMGR_MaxPrefixSet
* PURPOSE:
*     set max prefix value.
*
* INPUT:
*      vr_id,
*      instance,
*      pmax.
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
UI32_T RIP_PMGR_MaxPrefixSet(UI32_T vr_id, UI32_T instance,UI32_T pmax);

/* FUNCTION NAME : RIP_PMGR_MaxPrefixUnset
* PURPOSE:
*     unset max prefix.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_MaxPrefixUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_RecvBuffSizeSet
* PURPOSE:
*     set reveiver buffer size.
*
* INPUT:
*      vr_id,
*      instance,
*      buff_size.
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
UI32_T RIP_PMGR_RecvBuffSizeSet(UI32_T vr_id, UI32_T instance,UI32_T buff_size);

/* FUNCTION NAME : RIP_PMGR_RecvBuffSizeUnset
* PURPOSE:
*     unset receive buffer size.
*
* INPUT:
*      vr_id,
*      instance,
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
UI32_T RIP_PMGR_RecvBuffSizeUnset(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_RedistributeSet
* PURPOSE:
*     set redistribute.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string.
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
UI32_T RIP_PMGR_RedistributeSet(UI32_T vr_id, UI32_T instance,char *protocol);

/* FUNCTION NAME : RIP_PMGR_RedistributeMetricSet
* PURPOSE:
*     set redistribute with metric.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string,
*      metric.
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
UI32_T RIP_PMGR_RedistributeMetricSet(UI32_T vr_id, UI32_T instance,char *protocol, UI32_T metric);

/* FUNCTION NAME : RIP_PMGR_RedistributeRmapSet
* PURPOSE:
*     set redistribute with route map.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string,
*      rmap.
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
UI32_T RIP_PMGR_RedistributeRmapSet(UI32_T vr_id, UI32_T instance,char *protocol, char *rmap);

/* FUNCTION NAME : RIP_PMGR_RedistributeAllSet
* PURPOSE:
*     set redistribute with metric and route map.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string,
*      metric
*      rmap.
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
UI32_T RIP_PMGR_RedistributeAllSet(UI32_T vr_id, UI32_T instance,char *protocol, UI32_T metric, char *rmap);

/* FUNCTION NAME : RIP_PMGR_RedistributeUnset
* PURPOSE:
*     unset redistribute.
*
* INPUT:
*      vr_id,
*      instance,
*      protocol: protocol string,
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
UI32_T RIP_PMGR_RedistributeUnset(UI32_T vr_id, UI32_T instance,char *protocol);

/* FUNCTION NAME : RIP_PMGR_ClearRoute
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
UI32_T RIP_PMGR_ClearRoute(UI32_T vr_id, UI32_T instance,char *arg);

/* FUNCTION NAME : RIP_PMGR_ClearStatistics
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
UI32_T RIP_PMGR_ClearStatistics(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : RIP_PMGR_InterfaceAdd
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
UI32_T RIP_PMGR_InterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_InterfaceDelete
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
UI32_T RIP_PMGR_InterfaceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_LoopbackInterfaceAdd
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
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      
*/
UI32_T RIP_PMGR_LoopbackInterfaceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI16_T if_flags);

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
UI32_T RIP_PMGR_InterfaceUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_InterfaceDown
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
UI32_T RIP_PMGR_InterfaceDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : RIP_PMGR_IpAddressAdd
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
UI32_T RIP_PMGR_IpAddressAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                      struct pal_in4_addr *addr, UI32_T pfx_len);

/* FUNCTION NAME : RIP_PMGR_IpAddressDelete
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
UI32_T RIP_PMGR_IpAddressDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                         struct pal_in4_addr *addr, UI32_T pfx_len);

/* FUNCTION NAME : RIP_PMGR_RifUp
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
UI32_T RIP_PMGR_RifUp(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : RIP_PMGR_RifDown
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
UI32_T RIP_PMGR_RifDown(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : RIP_PMGR_GetNextRouteEntry
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
UI32_T RIP_PMGR_GetNextRouteEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Route_Entry_T *entry);

/* FUNCTION NAME : RIP_PMGR_GetNextThreadTimer
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
BOOL_T RIP_PMGR_GetNextThreadTimer(UI32_T vr_id, UI32_T vrf_id, UI32_T *nexttime);

/* FUNCTION NAME : RIP_PMGR_GetNextPeerEntry
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
BOOL_T RIP_PMGR_GetNextPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry);

/* FUNCTION NAME : RIP_PMGR_GetPeerEntry
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
BOOL_T RIP_PMGR_GetPeerEntry(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Peer_Entry_T *entry);

/* FUNCTION NAME : RIP_PMGR_GetGlobalStatistics
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
BOOL_T RIP_PMGR_GetGlobalStatistics(UI32_T vr_id, UI32_T vrf_id, RIP_TYPE_Global_Statistics_T *stat);
/* FUNCTION NAME : RIP_PMGR_GetRip2IfStatTable
* PURPOSE:
*     Get interface state table .
*
* INPUT:
*     Rip2IfStatEntry_T  data
*
* OUTPUT:
*     Rip2IfStatEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/

BOOL_T RIP_PMGR_GetRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfStatEntry_T *data);

/* FUNCTION NAME : RIP_PMGR_GetNextRip2IfStatTable
* PURPOSE:
*     Getnext interface state table .
*
* INPUT:
*     Rip2IfStatEntry_T  data
*
* OUTPUT:
*     Rip2IfStatEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_PMGR_GetNextRip2IfStatTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfStatEntry_T *data);
/* FUNCTION NAME : RIP_PMGR_GetRip2IfConfTable
* PURPOSE:
*     Get interface configure table .
*
* INPUT:
*     Rip2IfConfEntry_T  data
*
* OUTPUT:
*     Rip2IfConfEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_PMGR_GetRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfConfEntry_T *data);

/* FUNCTION NAME : RIP_PMGR_GetNextRip2IfConfTable
* PURPOSE:
*     Getnext interface configure table .
*
* INPUT:
*     Rip2IfConfEntry_T  data
*
* OUTPUT:
*     Rip2IfConfEntry_T  data
*
* RETURN:
*       TRUE/FALSE
*
* NOTES:
*       
*/
BOOL_T RIP_PMGR_GetNextRip2IfConfTable(UI32_T vr_id, UI32_T vrf_id, Rip2IfConfEntry_T *data);



/* FUNCTION NAME : RIP_PMGR_GetIfAddress
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
BOOL_T RIP_PMGR_GetIfAddress(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *out_addr);

/* FUNCTION NAME : RIP_PMGR_GetIfRecvBadPacket
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
BOOL_T RIP_PMGR_GetIfRecvBadPacket(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : RIP_PMGR_GetIfRecvBadRoute
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
BOOL_T RIP_PMGR_GetIfRecvBadRoute(UI32_T vr_id, UI32_T vrf_id, UI32_T exact, UI32_T in_addr, UI32_T *value);

/* FUNCTION NAME : RIP_PMGR_GetIfSendUpdate
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
BOOL_T RIP_PMGR_GetIfSendUpdate(UI32_T vr_id, UI32_T vrf_id,UI32_T exact, UI32_T in_addr, UI32_T *value);

#endif    /* End of RIP_PMGR_H */

