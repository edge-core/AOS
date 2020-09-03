/* MODULE NAME: mldsnp_pom.H
* PURPOSE:
*   {1. What is covered in this file - function and scope}
*   {2. Related documents or hardware information}
* NOTES:
*   {Something must be known or noticed}
*   {1. How to use these functions - Give an example}
*   {2. Sequence of messages if applicable}
*   {3. Any design limitation}
*   {4. Any performance limitation}
*   {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007    Macauley_Cheng Create
* Copyright(C)      Accton Corporation, 2007
*/


#ifndef _MLDSNP_POM_H
#define _MLDSNP_POM_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_om.h"


/* NAMING CONSTANT DECLARATIONS
*/


/* MACRO FUNCTION DECLARATIONS
*/
/* DATA TYPE DECLARATIONS
*/


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/*------------------------------------------------------------------------------
 * ROUTINE NAME : MLDSNP_POM_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
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
void MLDSNP_POM_InitiateProcessResources(void);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the global configuration
* INPUT  : None
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetGlobalConf(
                         MLDSNP_OM_Cfg_T *mldsnp_global_conf_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the group's port list
* INPUT  : vid            - the vlan id
*          *gip_ap        - the group ip address
*          *sip_ap        - the source ip address
* OUTPUT : *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only rovide to UI to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetGroupPortlist(
                                   UI16_T                vid,
                                   UI8_T                 *gip_ap,
                                   UI8_T                 *sip_ap,
                                   MLDSNP_OM_GroupInfo_T *group_info_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetHisamEntryInfo
*------------------------------------------------------------------------------
* Purpose: This function get the info in hisam entry
* INPUT  : vid           - the vlan id
*          gip_ap        - the group ip
*          sip_ap        - the source ip
*          key_idx       - use which key to get
* OUTPUT : *entry_info   - the infomation store in hisam
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetHisamEntryInfo(
                                    UI32_T                 vid,
                                    UI8_T                  *gip_ap,
                                    UI8_T                  *sip_ap,
                                    UI32_T                 key_idx,
                                    MLDSNP_OM_HisamEntry_T *entry_info);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave status
* INPUT  : vid                      - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetImmediateLeaveStatus(
                                          UI16_T vid,
                                          MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetImmediateLeaveStatusByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate leave by-host-ip status
* INPUT  : vid                      - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave by-host-ip status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetImmediateLeaveByHostStatus(
                                          UI16_T vid,
                                          MLDSNP_TYPE_ImmediateByHostStatus_T *immediate_leave_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's immediate leave status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave status
*         *vid_p                    - the next vlan id
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextImmediateLeaveStatus(
                                              UI16_T *vid_p,
                                              MLDSNP_TYPE_ImmediateStatus_T *immediate_leave_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextImmediateLeaveByHostStatus
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's immediate leave by-host-ip status
* INPUT  : *vid_p                   - the vlan id
* OUTPUT : immediate_leave_status_p - the immediate leave by-host-ip status
*          *vid_p                    - the next vlan id
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextImmediateLeaveByHostStatus(
                                              UI16_T *vid_p,
                                              MLDSNP_TYPE_ImmediateByHostStatus_T* immediate_leave_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetLastListenerQueryInterval
*------------------------------------------------------------------------------
* Purpose: This function Get the last listener query interval
* INPUT  : None
* OUTPUT : *interval_p  - the interval in second
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.8, 9.8
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetLastListenerQueryInterval(
                                         UI16_T *interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetListenerInterval
*------------------------------------------------------------------------------
* Purpose: This function get the listner interval
* INPUT  : None
* OUTPUT : *interval_p - the listerner interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :7.4 in RFC2710,  9.4 in RFC 3810
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetListenerInterval(
                              UI16_T *interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetMldSnpVer
*------------------------------------------------------------------------------
* Purpose: This function get the mldsno version
* INPUT  : None
* OUTPUT : *ver_p - the version
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetMldSnpVer(
                         UI16_T *ver_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetMldStatus
*------------------------------------------------------------------------------
* Purpose: This function get the Mld status
* INPUT  : None
* OUTPUT : *mldsnp_status_p - the mldsnp status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetMldStatus(
                               MLDSNP_TYPE_MLDSNP_STATUS_T *mldsnp_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the next group's port list
* INPUT  : vid_p    - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
* OUTPUT :  vid_p   - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          *group_info_p  - the group informaiton
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextGroupPortlist(
                                       UI16_T                *vid_p,
                                       UI8_T                 *gip_ap,
                                       UI8_T                 *sip_ap,
                                       MLDSNP_OM_GroupInfo_T *group_info_p);
/*------------------------------------------------------------------------------

* Function : MLDSNP_POM_GetNextPortGroupSourceList
*------------------------------------------------------------------------------

* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid   - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT : port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*          This frunction only provide to user to access

*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSourceList(
                                        MLDSNP_OM_PortSourceListInfo_T *port_source_list_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetPortGroupSourceList
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_src_lst_p->vid    - the next vlan id
*          port_src_lst_p->port  - the port to get
* OUTPUT :  port_src_lst_p - the next vid, gip and source list
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetPortGroupSourceList(
                                        MLDSNP_OM_PortSourceListInfo_T *port_source_list_info_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSource
*------------------------------------------------------------------------------
* Purpose: This function get the port joined all group
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSource(
    MLDSNP_OM_PortSourceInfo_T *port_source_linfo_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortGroupSourceHost
*------------------------------------------------------------------------------
* Purpose: This function get the port joined (S,G, host ip)
* INPUT  : port_source_linfo_p->vid    - the next vlan id
*          port_source_linfo_p->port   - the port to get
*          port_source_linfo_p->gip_a  - current group
*          port_source_linfo_p->sip_a  - current group
*          port_source_linfo_p->host_a  - current group
* OUTPUT :  port_src_lst_p - the next vid, gip, sip
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This frunction only provide to user to access
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_POM_GetNextPortGroupSourceHost(
    MLDSNP_OM_PortHostInfo_T *port_host_p);
/*------------------------------------------------------------------------------

* Function : MLDSNP_POM_GetNextRunningPortStaticGroup
*------------------------------------------------------------------------------

* Purpose: This function get the static join group port bit list
* INPUT  : *nxt_id   - current entry id
* OUTPUT : *nxt_id - the next entry id
*          *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *port   -the port
*          *rec_type - ther record type
* RETUEN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default

*           SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure

*           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default

* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetNextRunningPortStaticGroup(
                                             UI16_T *id,
                                             UI16_T *vid,
                                             UI8_T  *gip_ap,
                                             UI8_T  *sip_ap,
                                             UI16_T  *port,
                                             MLDSNP_TYPE_RecordType_T *rec_type);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextPortStaticGroup
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *nxt_id   - current entry id
* OUTPUT : *nxt_id       - the next entry id
*           *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *port  -the port
*          *rec_type - ther record type
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextPortStaticGroup(
                                             UI16_T *nxt_id,
                                             UI16_T *vid,
                                             UI8_T  *gip_ap,
                                             UI8_T  *sip_ap,
                                             UI16_T  *port,
                                             MLDSNP_TYPE_RecordType_T *rec_type);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetNextStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *vid   - current vlan id
* OUTPUT : *vid    - the next vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
*          *group_info_p  - the group informaiton
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetNextStaticGroupPortlist(
    UI16_T                *vid_p,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portbitmap_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetStaticGroupPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the static join group port bit list
* INPUT  : *vid    - the  vlan id
*          *gip_ap - the gruop ip
*          *sip_ap - the source ip
* OUTPUT : *group_info_p  - the group informaiton
* RETUEN  : TRUE/FALSE
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetStaticGroupPortlist(
    UI16_T                vid,
    UI8_T                 *gip_ap,
    UI8_T                 *sip_ap,
    UI8_T                 *static_portbitmap_p);


/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetOldVerQuerierPresentTimeOut
*------------------------------------------------------------------------------
* Purpose: This function get the old version querier present time out value
* INPUT  : vid	- the vlan id
* OUTPUT : *time_out_p   - the time out in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :9.12 in RFC3180
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetOldVerQuerierPresentTimeOut(
                                             UI16_T *time_out_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetOtherQueryPresentInterval
*------------------------------------------------------------------------------
* Purpose: This function get the other querier present interval
* INPUT  : None
* OUTPUT : *interval_p   - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.5, 9.5 robust * query_interval + query_rsponse_interval
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetOtherQueryPresentInterval(
                                           UI32_T *interval_p);

/*------------------------------------------------------------------------------

* Function : MLDSNP_POM_GetQuerierRunningStatus
*------------------------------------------------------------------------------

* Purpose: This function get the querier running status of the input vlan

* INPUT  : vid        - the vlan id
* OUTPUT :
           *status_p  - the querier running status

* RETURN : TRUE - success

*          FALSE- fail

* NOTES  :

*------------------------------------------------------------------------------*/

BOOL_T MLDSNP_POM_GetQuerierRunningStatus(
                                          UI16_T vid,
                                          MLDSNP_TYPE_QuerierStatus_T *status_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQuerierStatus
*------------------------------------------------------------------------------
* Purpose: This function set the querier status of input vlan
* INPUT  : none
* OUTPUT : *status  - the querier status
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQuerierStatus(
                                MLDSNP_TYPE_QuerierStatus_T *status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQueryInterval
*------------------------------------------------------------------------------
* Purpose: This funcgtion get the query interval
* INPUT  : None
* OUTPUT : *interval_p - the interval in seconds
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.2, 9.2
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQueryInterval(
                              UI16_T *interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetQueryResponseInterval
*------------------------------------------------------------------------------
* Purpose: This function get the query response interval
* INPUT  : None
* OUTPUT : *interval_p  - the query response interval
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :   7.3, 9.3
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetQueryResponseInterval(
                                       UI16_T *interval_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRobustnessValue
*------------------------------------------------------------------------------
* Purpose: This function get the robust ess value
* INPUT  : None
* OUTPUT : *value_p - the robustness value
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :  7.1, 7.9, 9.1, 9.9
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRobustnessValue(
                                  UI16_T *value_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRouterExpireTime
*------------------------------------------------------------------------------
* Purpose: This function get the router expire time
* INPUT  :
* OUTPUT : exp_time_p  - the expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRouterExpireTime(
                                  UI16_T *exp_time_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningGlobalConf
*------------------------------------------------------------------------------
* Purpose: This function get the running configuration
* INPUT  : None
* OUTPUT : *mldsnp_global_conf_p - the global configuration
* RETUEN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*          SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningGlobalConf(
                                 MLDSNP_OM_RunningCfg_T *mldsnp_global_conf_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningImmediateLeaveStatus
*------------------------------------------------------------------------------
* Purpose: This function get the immediate running status
* INPUT  : vid       - the vlan id
* OUTPUT : *status_p - the immediate status
* RETUEN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*          SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningImmediateLeaveStatus(
                                                UI32_T vid,
                                                MLDSNP_TYPE_ImmediateStatus_T *status_p,
                                                MLDSNP_TYPE_ImmediateByHostStatus_T *imme_byhost_status_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningRouterPortList
*------------------------------------------------------------------------------
* Purpose: This function get next the run static router port bit list
* INPUT  : *vid - the vlan id to get next vlan id
* OUTPUT : *router_port_bitmap_p  - the router port bit map pointer
* RETUEN : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   - not same as default
*          SYS_TYPE_GET_RUNNING_CFG_FAIL	   - get failure
*          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE - same as default
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningRouterPortList(
                                           UI16_T vid,
                                           UI8_T  *router_port_bitmap_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id          - which vlan to get
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetUnknownFloodBehavior(
    UI32_T vlan_id,
    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRunningUnknownFloodBehavior
*------------------------------------------------------------------------------
* Purpose: This function get the unknown multicast data flood behavior
* INPUT  : flood_behavior_p - the returned router port info
*          vlan_id          - which vlan to get
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
UI32_T MLDSNP_POM_GetRunningUnknownFloodBehavior(
    UI32_T vlan_id,
                                    MLDSNP_TYPE_UnknownBehavior_T *flood_behavior_p);

BOOL_T MLDSNP_POM_GetNextRunningVlanConfig(
    UI32_T *next_vlan_p,
    MLDSNP_OM_VlanRunningCfg_T *vlan_cfg_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetVlanRouterPortlist
*------------------------------------------------------------------------------
* Purpose: This function get the vlan's router list
* INPUT  : vid               - the vlan id
* OUTPUT : router_port_list  - the router port info
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetVlanRouterPortlist(
                                        UI16_T                     vid,
                                        MLDSNP_OM_RouterPortList_T *router_port_list);
/*------------------------------------------------------------------------------

* Function : MLDSNP_POM_GetNextVlanRouterPortlist
*------------------------------------------------------------------------------

* Purpose: This function get the next vlan's router list
* INPUT  : *vid_p            - the vlan id

* OUTPUT : router_port_list  - the router port info
*          *vid_p            - the next vlan id
* RETURN : TRUE - success

*          FALSE- fail
* NOTES  : This function is only provide to UI to access

*------------------------------------------------------------------------------*/

BOOL_T MLDSNP_POM_GetNextVlanRouterPortlist(
                                            UI16_T                     *vid_p,
                                            MLDSNP_OM_RouterPortList_T *router_port_list);

/*------------------------------------------------------------------------------
* Function : MLDSNP_POM_GetRouterPortExpireInterval
*------------------------------------------------------------------------------
* Purpose: This function get the next vlan's router list
* INPUT  : vid_p  - the vlan id
*          lport  - which port
* OUTPUT : *expire_p - expire time
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  : This function is only provide to UI to access
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_POM_GetRouterPortExpireInterval(
    UI16_T vid,
    UI32_T lport,
    UI32_T *expire_p);

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDFilterStatus
 *-------------------------------------------------------------------------
 * PURPOSE : Get MLD filter status
 * INPUT   : None
 * OUTPUT  : *status - filter status
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDFilterStatus(UI32_T *status);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetNextMLDProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get next profile id
 * INPUT   : *pid - current profile id
 * OUTPUT  : *pid - next profile id
  * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetNextMLDProfileID(UI32_T *pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDProfileAccessMode
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile access mode
 * INPUT   : pid - which profile di to get
 * OUTPUT  : *mode  - profile access mode
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDProfileAccessMode(UI32_T pid, UI32_T *mode);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetNextMLDProfileGroupbyPid
 *-------------------------------------------------------------------------
 * PURPOSE : Get profile next group
 * INPUT   : pid  - profile id
 * OUTPUT  : *start_ip  - group start ip
 *           *end_ip    - group end ip
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetNextMLDProfileGroupbyPid(UI32_T pid, UI8_T *start_ip, UI8_T *end_ip);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetPortMLDProfileID
 *-------------------------------------------------------------------------
 * PURPOSE : Get port bind profile id
 * INPUT   : port - which port to get
 * OUTPUT  : *pid - which profile id bind to this port
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetPortMLDProfileID(UI32_T port, UI32_T *pid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_IsMLDProfileExist
 *-------------------------------------------------------------------------
 * PURPOSE : Check this profile id exist
 * INPUT   : profile_id  - profile id to check
 * OUTPUT  : None
 * RETURN  : TRUE - exist
 *           FALSE- not exist
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_IsMLDProfileExist(UI32_T profile_id);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMLDThrottlingInfo
 *-------------------------------------------------------------------------
 * PURPOSE : Get throttle information
 * INPUT   : port - which port to get
 * OUTPUT  : *info- throttle information
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMLDThrottlingInfo(UI32_T port, MLDSNP_OM_Throttle_T *info);
#endif

#if (SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT== TRUE || SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to set mld report limit value per second
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetMldReportLimitPerSec(UI32_T ifindex, UI16_T  *limit_per_sec);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunnningMldReportLimitPerSec
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get mld report limit value per second configuration
 * INPUT   : ifindex       - which port or vlan to get
 * OUTPUT  : limit_per_sec - per second limit value
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunnningMldReportLimitPerSec(UI32_T ifindex, UI16_T *limit_per_sec);
#endif

#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_OM_GetQueryDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get port query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : MVR_MGR_OK  - success
 *           MVR_MGR_CAN_NOT_GET - failure
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetQueryDropStatus(UI32_T lport, UI32_T  *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunningQuaryGuardStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get query guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunningQuaryGuardStatus(UI32_T lport, UI32_T *status);
#endif /*#if (SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)*/

#if (SYS_CPNT_IPV6_MULTICAST_DATA_DROP== TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetMulticastDataDropStatus
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
BOOL_T MLDSNP_POM_GetMulticastDataDropStatus(UI32_T lport, UI32_T  *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetRunningMulticastDataDropStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get multicast data guard status
 * INPUT   : lport  - which port to get information
 *           status - the enable or diable status
 * OUTPUT  : status  - the enabled or disable status
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_POM_GetRunningMulticastDataDropStatus(UI32_T lport, UI32_T *status);
#endif

#if (SYS_CPNT_MLDSNP_PROXY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetProxyReporting
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get MLDSNP status.
 * INPUT   : *mldsnp_status - MLDSNP status output buffer
 * OUTPUT  : *mldsnp_status - MLDSNP status
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetProxyReporting(MLDSNP_TYPE_ProxyReporting_T *status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetUnsolicitedReportInterval
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get MLDSNP unsolicited report interval.
 * INPUT   : Unsolicit_report_interval - getting value
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *    FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetUnsolicitedReportInterval(UI32_T *Unsolicit_report_interval);

#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_POM_GetTotalEntry
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to get total created entries in om
 * INPUT   : None
 * OUTPUT  : total_p  - total entries
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE  - no change
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS - not default value
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_POM_GetTotalEntry(UI32_T *total_p);

#endif/* End of pom_H */


