/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_RIP.H
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
#ifndef NETCFG_OM_RIP_H
#define NETCFG_OM_RIP_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "l_radix.h"


typedef struct NETCFG_OM_RIP_Distance_S
{
    struct prefix p;
    UI32_T  distance;
    char    alist_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
}NETCFG_OM_RIP_Distance_T;

typedef struct NETCFG_OM_RIP_Instance_S
{
    L_RADIX_Table_T             network_table;
    L_RADIX_Table_T             distance_table;
    L_RADIX_Table_T             neignbor_table;
    NETCFG_TYPE_RIP_Instance_T  instance_value;

}NETCFG_OM_RIP_Instance_T;


/* FUNCTION NAME : NETCFG_OM_RIP_Init
 * PURPOSE:Init NETCFG_OM_RIP database, create semaphore
 *
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
 *      None.
 */
void NETCFG_OM_RIP_Init(void);

/* FUNCTION NAME : NETCFG_OM_RIP_AddVr
 * PURPOSE:
 *      Add a RIP master entry when add a VR.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddVr(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteVr
 * PURPOSE:
 *      Delete a RIP master entry when delete a VR.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteVr(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteAllRipMasterEntry
 * PURPOSE:
 *          Remove all RIP master entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_RIP_DeleteAllRipMasterEntry(void);

/* FUNCTION NAME : NETCFG_OM_RIP_AddInstance
 * PURPOSE:
 *      Add a RIP instance entry when router rip enable in a instance.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddInstance(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteInstanceEntry
 * PURPOSE:
 *      Delete a RIP instance entry when router rip disable in a instance.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteInstanceEntry(UI32_T vr_id, UI32_T instance);

/* FUNCTION NAME : NETCFG_OM_RIP_GetInstanceEntry
 * PURPOSE:
 *      Get a rip instance entry.
 *
 * INPUT:
 *      vr_id
 *      instance: vrf_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetInstanceEntry(UI32_T vr_id,UI32_T instance,NETCFG_OM_RIP_Instance_T *entry);

/* FUNCTION NAME : NETCFG_OM_RIP_AddInterface
 * PURPOSE:
 *      Make a link-up l3 interface up in the RIP
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_AddInterface(UI32_T vr_id,UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_DeleteInterface
 * PURPOSE:
 *      Make a interface down in the RIP
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
void NETCFG_OM_RIP_DeleteInterface(UI32_T vr_id,UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_GetInterfaceEntry
 * PURPOSE:
 *      Get a rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextInterfaceEntry
 * PURPOSE:
 *      Get next rip interface entry.
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_GetNextInterfaceEntry(UI32_T vr_id,NETCFG_TYPE_RIP_If_T *entry);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvPacketSet
 * PURPOSE:
 *      Make RIP receive packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvPacketSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvPacketUnset
 * PURPOSE:
 *      Make RIP not receive packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvPacketUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_SendPacketSet
 * PURPOSE:
 *      Make RIP not send packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendPacketSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_SendPacketUnset
 * PURPOSE:
 *      Make RIP not send packet.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendPacketUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvVersionTypeSet
 * PURPOSE:
 *      Set RIP receive version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvVersionTypeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                                       enum NETCFG_TYPE_RIP_Version_E type);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvVersionUnset
 * PURPOSE:
 *      Set RIP receive version type to default version type.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvVersionUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_SendVersionTypeSet
 * PURPOSE:
 *      Set RIP send version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendVersionTypeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                                       enum NETCFG_TYPE_RIP_Version_E type);

/* FUNCTION NAME : NETCFG_OM_RIP_SendVersionUnset
 * PURPOSE:
 *      Set RIP send version type to default version type.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SendVersionUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthModeSet
 * PURPOSE:
 *      Set RIP authentication mode.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      mode 
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthModeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex,
                                              enum NETCFG_TYPE_RIP_Auth_Mode_E mode);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthModeUnset
 * PURPOSE:
 *      Set RIP authentication mode to default mode.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthModeUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthStringSet
 * PURPOSE:
 *      Set RIP authentication string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      str
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthStringSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, char *str);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthStringUnset
 * PURPOSE:
 *      Set RIP authentication string to null string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthStringUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthKeyChainSet
 * PURPOSE:
 *      Set RIP key-chain string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      str
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthKeyChainSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, char *str);

/* FUNCTION NAME : NETCFG_OM_RIP_AuthKeyChainUnset
 * PURPOSE:
 *      Set RIP Key-chain string to null string.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_AuthKeyChainUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_SplitHorizonSet
 * PURPOSE:
 *      Set RIP split horizon.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      type
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SplitHorizonSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, enum NETCFG_TYPE_RIP_Split_Horizon_E type);

/* FUNCTION NAME : NETCFG_OM_RIP_SplitHorizonUnset
 * PURPOSE:
 *      Set RIP split horizon to default.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SplitHorizonUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_VersionSet
 * PURPOSE:
 *      Set RIP global version.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      version
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_VersionSet(UI32_T vr_id, UI32_T vrf_id,
                                           enum NETCFG_TYPE_RIP_Global_Version_E version);

/* FUNCTION NAME : NETCFG_OM_RIP_VersionUnset
 * PURPOSE:
 *      Set RIP global version to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_VersionUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultAdd
 * PURPOSE:
 *      Make RIP to orignate default route.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultAdd(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultDelete
 * PURPOSE:
 *      Not make RIP to orignate default route.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultDelete(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultMetricSet
 * PURPOSE:
 *      Set RIP default metric value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      metric
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultMetricSet(UI32_T vr_id, UI32_T vrf_id, UI32_T metric);

/* FUNCTION NAME : NETCFG_OM_RIP_DefaultMetricUnset
 * PURPOSE:
 *      Set RIP default metric value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DefaultMetricUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_TimerSet
 * PURPOSE:
 *      Set RIP timer value include update, timeout, garbage.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      timer
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_TimerSet(UI32_T vr_id, UI32_T vrf_id, NETCFG_TYPE_RIP_Timer_T *timer);

/* FUNCTION NAME : NETCFG_OM_RIP_TimerUnset
 * PURPOSE:
 *      Set RIP timer include update, timeout,garbage value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_TimerUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceDefaultSet
 * PURPOSE:
 *      Set RIP default distance value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      distance
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DistanceDefaultSet(UI32_T vr_id, UI32_T vrf_id, UI32_T distance);

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceDefaultUnset
 * PURPOSE:
 *      Set RIP default distance value to default value.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_DistanceDefaultUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_MaxPrefixSet
 * PURPOSE:
 *      Set RIP max prefix number.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      pmax
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_MaxPrefixSet(UI32_T vr_id, UI32_T vrf_id, UI32_T pmax);

/* FUNCTION NAME : NETCFG_OM_RIP_MaxPrefixUnset
 * PURPOSE:
 *      Set RIP max prefix number to default number.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_MaxPrefixUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvBuffSizeSet
 * PURPOSE:
 *      Set RIP receive buffer size.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      buff_size
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvBuffSizeSet(UI32_T vr_id, UI32_T vrf_id, UI32_T buff_size);

/* FUNCTION NAME : NETCFG_OM_RIP_RecvBuffSizeUnset
 * PURPOSE:
 *      Set RIP receive buffer size to default size.
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_RecvBuffSizeUnset(UI32_T vr_id, UI32_T vrf_id);

/* FUNCTION NAME : NETCFG_OM_RIP_RedistributeSet
* PURPOSE:
*     set redistribute.
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
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_RedistributeSet(UI32_T vr_id, UI32_T vrf_id,UI32_T pro_type, UI32_T metric, char *rmap);

/* FUNCTION NAME : NETCFG_OM_RIP_RedistributeUnset
* PURPOSE:
*     unset redistribute.
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
*      None
*
* NOTES:
*      None.
*/
void NETCFG_OM_RIP_RedistributeUnset(UI32_T vr_id, UI32_T vrf_id,UI32_T pro_type);

/* FUNCTION NAME : NETCFG_OM_RIP_RifUp
 * PURPOSE:
 *      Singnal rif up
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_RifUp(UI32_T vr_id,UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_RifDown
 * PURPOSE:
 *      Singnal rif down
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_RifDown(UI32_T vr_id,UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_GetDistanceTableEntry
 * PURPOSE:
 *      Get distance table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      distance_entry. 
 *
 * OUTPUT:
 *      distance_entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_GetDistanceTableEntry(UI32_T vr_id, UI32_T vrf_id,
                                                         NETCFG_OM_RIP_Distance_T *distance_entry);

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceSet
* PURPOSE:
*     Set distance table.
*
* INPUT:
*      vr_id,
*      instance,
*      p,
*      distance,
*      alist.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_DistanceSet(UI32_T vr_id, UI32_T vrf_id, struct prefix *p,
                                            UI32_T distance, char *alist);

/* FUNCTION NAME : NETCFG_OM_RIP_DistanceUnset
* PURPOSE:
*     Unset distance table.
*
* INPUT:
*      vr_id,
*      instance,
*      p.
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/

UI32_T NETCFG_OM_RIP_DistanceUnset(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_GetDistributeTableEntry
 * PURPOSE:
 *      Get distribute table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id.
 *
 * OUTPUT:
 *      distribute_entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
 BOOL_T NETCFG_OM_RIP_GetDistributeTableEntry(UI32_T vr_id, UI32_T vrf_id,
                                                           NETCFG_TYPE_RIP_Distribute_T *distribute_entry);

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeSet
* PURPOSE:
*     Set distribute table.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeSet(UI32_T vr_id, UI32_T vrf_id,
                                             UI32_T ifindex, char *list_name,
                                             enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                             enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeSetAllInterface
* PURPOSE:
*     Set distribute table for all of l3 interface.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeSetAllInterface(UI32_T vr_id, UI32_T vrf_id, char *list_name,
                                                           enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                           enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeUnset
* PURPOSE:
*     Unset distribute table.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeUnset(UI32_T vr_id, UI32_T vrf_id,
                                                UI32_T ifindex, char *list_name,
                                                enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_OM_RIP_DistributeUnsetAllInterface
* PURPOSE:
*     Unset distribute table for all of l3 interface.
*
* INPUT:
*      vr_id,
*      vrf_id,
*      list_name,
*      type,
*      list_type.
*
* OUTPUT:
*      None.
*
* RETURN: TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DistributeUnsetAllInterface(UI32_T vr_id, UI32_T vrf_id, char *list_name,
                                                           enum NETCFG_TYPE_RIP_Distribute_Type_E type,
                                                           enum NETCFG_TYPE_RIP_Distribute_List_Type_E list_type);

/* FUNCTION NAME : NETCFG_OM_RIP_PassiveIfSet
* PURPOSE:
*     Set passive inteface
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_PassiveIfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_PassiveIfUnset
* PURPOSE:
*     Unset passive inteface
*
* INPUT:
*      vr_id,
*      vrf_id,
*      ifindex
*
* OUTPUT:
*      None.
*
* RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_RIP_PassiveIfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_CheckNeighbor
 * PURPOSE:
 *      Check if the neighbor exist.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_CheckNeighbor(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr);

/* FUNCTION NAME : NETCFG_OM_RIP_NeighborSet
 * PURPOSE:
 *      Set neighbor table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NeighborSet(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr);

/* FUNCTION NAME : NETCFG_OM_RIP_NeighborUnset
 * PURPOSE:
 *      Unset neighbor table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NeighborUnset(UI32_T vr_id, UI32_T vrf_id, struct pal_in4_addr *addr);

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkIfSet
 * PURPOSE:
 *      Set network ifname status.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkIfSet(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkIfUnset
 * PURPOSE:
 *      Unset network ifname status.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkIfUnset(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkTableSet
 * PURPOSE:
 *      Set network table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkTableSet(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_NetworkTableUnset
 * PURPOSE:
 *      Unset network table.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_NetworkTableUnset(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_CheckNetworkTable
 * PURPOSE:
 *      check if the network table is exist
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      p
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_RIP_CheckNetworkTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_SignalIpAddrAdd
 * PURPOSE:
 *      When add a primary IP address signal RIP.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      ip_addr,
 *      ip_mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SignalIpAddrAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : NETCFG_OM_RIP_SignalIpAddrDelete
 * PURPOSE:
 *      When delete a primary IP address signal RIP.
 *
 * INPUT:
 *      vr_id
 *      vrf_id,
 *      ifindex,
 *      ip_addr,
 *      ip_mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
 *
 * NOTES:
 *      
 */
UI32_T NETCFG_OM_RIP_SignalIpAddrDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextNetworkTableEntry
 * PURPOSE:
 *      Get next network table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id
 *
 * OUTPUT:
 *      p.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (p.prefixlen, p.u.prefix4.s_addr).
 *      If key is (0, 0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextNetworkTableEntry(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextNeighborTable
 * PURPOSE:
 *      Get next neighbor table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id
 *      p
 *
 * OUTPUT:
 *      p.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (p.u.prefix4.s_addr).
 *      If key is (0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextNeighborTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

/* FUNCTION NAME : NETCFG_OM_RIP_GetNextDistanceTable
 * PURPOSE:
 *      Get next distance table.
 *
 * INPUT:
 *      vr_id, 
 *      vrf_id
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      Key is (entry.p.prefixlen,entry.p.u.prefix4.s_addr).
 *      If key is (0,0), get first one.
 */
BOOL_T NETCFG_OM_RIP_GetNextDistanceTable(UI32_T vr_id, UI32_T vrf_id, NETCFG_OM_RIP_Distance_T *entry);

/* FUNCTION NAME : NETCFG_OM_RIP_DebugSet
* PURPOSE:
*     RIP debug on config mode.
*
* INPUT:
*      None.
*
* OUTPUT:
*      vr_id.
*
* RETURN:
*      TRUE/FALSE
*
* NOTES:
*      None.
*/
BOOL_T NETCFG_OM_RIP_DebugSet(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_DebugUnset
* PURPOSE:
*     RIP undebug on config mode.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_DebugUnset(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_EventDebugSet
* PURPOSE:
*     RIP event debug on config mode.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_EventDebugSet(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_EventDebugUnset
* PURPOSE:
*     RIP event undebug on config mode.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_EventDebugUnset(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_NsmDebugSet
* PURPOSE:
*     RIP nsm debug on config mode.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_NsmDebugSet(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_NsmDebugUnset
* PURPOSE:
*     RIP nsm undebug on config mode.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_NsmDebugUnset(UI32_T vr_id);

/* FUNCTION NAME : NETCFG_OM_RIP_PacketDebugSet
* PURPOSE:
*     RIP packet debug on config mode.
*
* INPUT:
*      vr_id,
*      type.
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
BOOL_T NETCFG_OM_RIP_PacketDebugSet(UI32_T vr_id, NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_OM_RIP_PacketDebugUnset
* PURPOSE:
*     RIP packet undebug on config mode.
*
* INPUT:
*      vr_id,
*      type.
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
BOOL_T NETCFG_OM_RIP_PacketDebugUnset(UI32_T vr_id, NETCFG_TYPE_RIP_Packet_Debug_Type_T type);

/* FUNCTION NAME : NETCFG_OM_RIP_GetDebugStatus
* PURPOSE:
*     Get RIP debug status.
*
* INPUT:
*      vr_id.
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
BOOL_T NETCFG_OM_RIP_GetDebugStatus(UI32_T vr_id, NETCFG_TYPE_RIP_Debug_Status_T *status);

BOOL_T NETCFG_OM_RIP_GetNextNetworkTable(UI32_T vr_id, UI32_T vrf_id, struct prefix *p);

#endif/*NETCFG_OM_RIP_H*/
