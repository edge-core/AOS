/* MODULE NAME: mldsnp_pmgr.H
* PURPOSE:
*	{1. What is covered in this file - function and scope}
*	{2. Related documents or hardware information}
* NOTES:
*	{Something must be known or noticed}
*	{1. How to use these functions - Give an example}
*	{2. Sequence of messages if applicable}
*	{3. Any design limitation}
*	{4. Any performance limitation}
*	{5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/


#ifndef _MLDSNP_PMGR_H
#define _MLDSNP_PMGR_H

/* INCLUDE FILE DECLARATIONS
*/
#include "sys_type.h"
#include "mldsnp_type.h"
#include "mldsnp_mgr.h"
/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/
/* DATA TYPE DECLARATIONS
*/
/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/

/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_PMGR_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for MLDSNP_PMGR.
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
 *------------------------------------------------------------------------------
 */
void MLDSNP_PMGR_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_AddPortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid        - the vlan id
 *          *gip_ap     - the group ip
 *          *sip_ap     - the source ip
 *          lport       - the static port list
 *          mode      - include or exclude
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_AddPortStaticJoinGroup(
                                        UI32_T vid,
                                        UI8_T *gip_ap,
                                        UI8_T *sip_ap,
                                        UI32_T lport,
                                        MLDSNP_TYPE_CurrentMode_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_AddStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the static router port
 * INPUT   : vid   - the vlan id
 *           lport - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_AddStaticRouterPort(
                                       UI32_T vid,
                                       UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_DeletePortStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete static port from group
 * INPUT   : vid        - the vlan id
 *           *gip_ap    - the group ip
 *           *sip_ap    - the source ip
 *           lport      - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_DeletePortStaticJoinGroup(
                                             UI32_T vid,
                                             UI8_T  *gip_ap,
                                             UI8_T  *sip_ap,
                                             UI32_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_DeleteStaticRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to delete the static router port
 * INPUT   : vid   - the vlan id
 *           lport - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_DeleteStaticRouterPort(
                                          UI32_T vid,
                                          UI32_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status
* INPUT  : vid                    - the vlan id
*          immediate_leave_status - the returned router port info
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_PMGR_SetImmediateLeaveStatus(
                                           UI32_T vid,
                                           UI32_T immediate_leave_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetImmediateLeaveStatusByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function Set the immediate leave status by-host-ip
* INPUT  : vid                    - the vlan id
*          immediate_leave_byhost_status - the immediate leave by host status
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_PMGR_SetImmediateLeaveByHostStatus(
                                            UI32_T vid,
                                            UI32_T immediate_leave_byhost_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function set the last listener query interval
* INPUT  : interval  - the interval in second
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetLastListenerQueryInterval(
                                         UI32_T interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function set the mldsno version
* INPUT  : ver
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetMldSnpVer(
                              UI32_T ver);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function set the Mld status
* INPUT  : mldsnp_status - the mldsnp status
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetMldStatus(
                               UI32_T mldsnp_status);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status
* INPUT  : querier_status - the querier status
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_FAIL
*          MLDSNP_TYPE_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQuerierStatus(
                                  UI32_T querier_status);

 /*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion set the query interval
* INPUT  : interval - the interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQueryInterval(
                                  UI32_T interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : interval - the query repsonse interval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetQueryResponseInterval(
                                          UI32_T interval);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function set the robust ess value
* INPUT  : value - the robustness value
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetRobustnessValue(
                                      UI32_T value);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function set the router expire time
* INPUT  : exp_time  - the expire time
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetRouterExpireTime(
                                      UI32_T exp_time);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function Set the unknown multicast data flood behavior
* INPUT  : flood_behavior - the returned router port info
*           vlan_id        - which vlan to configure
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  : vlan_id =0 means all vlan
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetUnknownFloodBehavior(UI32_T vlan_id,
                                           UI32_T flood_behavior);

#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetUnsolicitedReportInterval
*------------------------------------------------------------------------------
* Purpose: This function set the unsolicitedReportInterval
* INPUT  : interval  - the inteval in seconds
* OUTPUT : None
* RETURN : MLDSNP_TYEP_RETURN_FAIL
*          MLDSNP_TYEP_RETURN_SUCCESS
* NOTES  :  7.9, 9.11
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_PMGR_SetUnsolicitedReportInterval(
                                            UI32_T interval);

BOOL_T MLDSNP_PMGR_SetMldSnoopProxyReporting(UI32_T status);

#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetPortListStaticJoinGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid            - the vlan id
 *           *gip_ap        - the group ip
 *           *sip_ap        - the source ip
 *           *port_list_ap  - the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_SetPortListStaticJoinGroup(
                                              UI32_T vid,
                                              UI8_T *gip_ap,
                                              UI8_T *sip_ap,
                                              UI8_T *port_list_ap);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetPortListStaticLeaveGroup
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set static port join group
 * INPUT   : vid          - the vlan id
 *           *gip_ap      - the group ip
 *           *sip_ap      - the source ip
 *           *port_list_ap- the static port list
 * OUTPUT  : None
 * RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
 *           MLDSNP_TYPE_RETURN_FAIL     - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_SetPortListStaticLeaveGroup(
                                               UI32_T vid,
                                               UI8_T *gip_ap,
                                               UI8_T *sip_ap,
                                               UI8_T *port_list_ap);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_DeleteStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : vid           - the vlan id
*          *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN : MLDSNP_TYPE_RETURN_SUCCESS  - success
*          MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_DeleteStaticRouterPortlist(
                                              UI32_T vid,
                                              UI8_T  *port_list_ap);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetStaticRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function set the static router port list
* INPUT  : *port_list_ap  - the static router port list
* OUTPUT : None
* RETURN  : MLDSNP_TYPE_RETURN_SUCCESS  - success
*           MLDSNP_TYPE_RETURN_FAIL     - failure
* NOTES  :SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_PMGR_SetStaticRouterPortlist(
                                           UI32_T vid,
                                           UI8_T  *port_list_ap);

/*------------------------------------------------------------------------------
* Function : MLDSNP_PMGR_SetMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set mroute status
* INPUT  : is_eanbled - mroute is enabled or not
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void  MLDSNP_PMGR_SetMRouteStatus(
                                           BOOL_T is_enabled);

UI32_T MLDSNP_PMGR_SetMldFilter(UI32_T status);
UI32_T MLDSNP_PMGR_CreateMLDProfileEntry(UI32_T profile_id);
UI32_T MLDSNP_PMGR_DestroyMLDProfileEntry(UI32_T profile_id);
UI32_T MLDSNP_PMGR_SetMLDProfileAccessMode(UI32_T pid, UI32_T mode);
UI32_T MLDSNP_PMGR_AddMLDProfileGroup(UI32_T pid, UI8_T mip_begin[], UI8_T mip_end[]);
UI32_T MLDSNP_PMGR_DeleteMLDProfileGroup(UI32_T pid, UI8_T mip_begin[], UI8_T mip_end[]);
UI32_T MLDSNP_PMGR_AddMLDProfileToPort(UI32_T ifindex, UI32_T pid);
UI32_T MLDSNP_PMGR_RemoveMLDProfileFromPort(UI32_T ifindex);
UI32_T MLDSNP_PMGR_SetMLDThrottlingNumberToPort(UI32_T ifindex, UI32_T pid);
UI32_T MLDSNP_PMGR_SetMLDThrottlingActionToPort(UI32_T port, UI32_T action);
UI32_T MLDSNP_PMGR_GetNextPortMLDProfileID(UI32_T *ifindex, UI32_T *pid);

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set igmp report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 *           limit_per_sec - per second limit value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetMldReportLimitPerSec(UI32_T ifindex, UI16_T limit_per_sec);
#endif

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set query quard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetQueryDropStatus(UI32_T lport, UI32_T status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetNextQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get next port query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_GetNextQueryDropStatus(UI32_T *lport, UI32_T  *status);
#endif

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_SetMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set multicast data guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : None
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_SET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_PMGR_SetMulticastDataDropStatus(UI32_T lport, UI32_T status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetNextMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_PMGR_GetNextMulticastDataDropStatus(UI32_T *lport, UI32_T  *status);
#endif


BOOL_T
MLDSNP_PMGR_ClearMldSnoopingDynamicgGroup();
BOOL_T
MLDSNP_PMGR_Clear_Ipv6_Mld_snooping_Statistics(UI32_T ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_PMGR_GetInfStatistics
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get interface statistics entry
 * INPUT   : inf_id       - interface id
 *           is_vlan      - TRUE to indicate the inf_id is VLAN id
 * OUTPUT  : statistics_p - pointer to content of the statistics entry
 *
 * RETUEN  : TRUE  - Success
 *           FALSE - Fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T
MLDSNP_PMGR_GetInfStatistics(
    UI32_T  inf_id,
    BOOL_T  is_vlan,
    MLDSNP_MGR_InfStat_T   *statistics_p);

#endif/* End of mldsnp_pmgr_H */


