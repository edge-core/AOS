/* MODULE NAME: mldsnp_engine.H
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


#ifndef _MLDSNP_ENGINE_H
#define _MLDSNP_ENGINE_H

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_type.h"
#include "mldsnp_om.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

typedef struct MLDSNP_ENGINE_GroupRecord_S
{
    UI8_T  rec_type;
    UI8_T  aux_data_len;
    UI16_T num_of_src;
    UI8_T  gip_a[MLDSNP_TYPE_ICMPV6_GROUP_ADDR_LEN];
    UI8_T  sip_aa[1][MLDSNP_TYPE_IPV6_SRC_IP_LEN];
}MLDSNP_ENGINE_GroupRecord_T;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct MLDSNP_ENGINE_IPv6Header_S
{
    UI32_T ver_class_label;               /*4 bits,8bits, 20bits*/
    UI16_T payload_len;
    UI8_T  next_header;
    UI8_T  hop_lim;
    UI8_T  sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T  dip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];
    UI8_T  hop_by_hop[MLDSNP_TYPE_HOP_BY_HOP_LEN];
}MLDSNP_ENGINE_IPv6Header_T;


typedef struct MLDSNP_ENGINE_V1Report_S
{
    UI8_T  type;
    UI8_T  code;
    UI16_T checksum;
    UI16_T max_resp;
    UI16_T reserved;
    UI8_T  gip_a[MLDSNP_TYPE_ICMPV6_GROUP_ADDR_LEN];
}MLDSNP_ENGINE_V1Report_T;

typedef struct MLDSNP_ENGINE_V2Report_S
{
    UI8_T  type;
    UI8_T  reserved1;
    UI16_T check_sum;
    UI16_T reserved2;
    UI16_T num_of_group_rec;             /*v1 is reerved*/
    MLDSNP_ENGINE_GroupRecord_T rec[1];
}MLDSNP_ENGINE_V2Report_T;

typedef struct MLDSNP_ENGINE_Query_S
{
    UI8_T  type;
    UI8_T  code;
    UI16_T check_sum;
    UI16_T max_rsp_code;
    UI16_T reserved;
    UI8_T  gip_a[MLDSNP_TYPE_ICMPV6_GROUP_ADDR_LEN];
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU!=TRUE)
    UI8_T  resv: 4;
    UI8_T  s   : 1;
    UI8_T  qrv : 3;
    #else
    UI8_T  qrv : 3;
    UI8_T  s   : 1;
    UI8_T  resv: 4;
#endif
    UI8_T  qqic;
    UI16_T num_of_src;
    UI8_T  sip_aa[0][MLDSNP_TYPE_IPV6_SRC_IP_LEN];
}MLDSNP_ENGINE_Query_T;


typedef struct MLDSNP_ENGINE_Icmpv6_S
{
    UI8_T  type;
    UI8_T  code;
    UI16_T checksum;
    UI8_T  head_a[4];
    UI8_T  addr_a[16];
}MLDSNP_ENGINE_Icmpv6_T;

typedef struct MLDSNP_ENGINE_Icmpv6Checksum_S
{
    /*ipv6 pseduo header*/
    UI8_T   src_ip[MLDSNP_TYPE_IPV6_SRC_IP_LEN];
    UI8_T   dst_ip[MLDSNP_TYPE_IPV6_DST_IP_LEN];
    UI32_T  packet_leng;
    UI8_T   reserv[3];
    UI8_T   next_header;
    struct  MLDSNP_ENGINE_Icmpv6_S icmpv6_pdu;
} MLDSNP_ENGINE_Icmpv6Checksum_T;

typedef struct MLDSNP_ENGINE_MldPdu_S
{
    MLDSNP_ENGINE_IPv6Header_T ipv6_header;
    MLDSNP_ENGINE_Icmpv6_T     icmpv6;
}MLDSNP_ENGINE_MldPdu_T;
#pragma pack(pop)   /* restore original alignment from stack */

typedef struct MLDSNP_ENGINE_PimHeader_S
{
  UI8_T vertype;
  UI8_T resv;
  UI16_T cksum;
}MLDSNP_ENGINE_PimHeader_T;

typedef struct MLDSNP_ENGINE_PimOption_S
{
  UI16_T  type;			/* type of option */
  UI16_T  len;			/* len of param */
  UI8_T  param[4];
}MLDSNP_ENGINE_PimOption_T;

typedef struct MLDNSP_ENGINE_Msg_S
{
    UI16_T                     recevied_port;
    UI16_T                     pdu_len;
    UI16_T                     vid;
    UI16_T                     icmp_len;
    UI16_T                     pim_len;

    UI8_T                      src_mac_a[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                      dst_mac_a[SYS_ADPT_MAC_ADDR_LEN];

    MLDSNP_ENGINE_IPv6Header_T *ipv6_header_p;  /* ipv6 header is not fix length, so use pointer */
    MLDSNP_ENGINE_Icmpv6_T     *icmpv6_p;
    MLDSNP_ENGINE_PimHeader_T  *pim6_header_p;
    L_MM_Mref_Handle_T *org_mref_p;
    
}MLDNSP_ENGINE_Msg_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
void MLDSNP_ENGINE_ClearProxyV2AllocatedMemory();
/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_UpdateAndAddChipPortBitmap
 *-------------------------------------------------------------------------
 * PURPOSE : This function calculate what ports shall be write the (v, g, s)
 * INPUT   : vid       - vlan id
 *           gip_ap    - group ip array point
 *           sip       - src ip array point
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :if there is (v,g,0), write port to (v,g,s) need add ports in (v,g,0).
 *          or the ports in (v,g,0) won't received any data.
 *-------------------------------------------------------------------------
*/
BOOL_T MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ChangeModeToIncludeMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function record the mode change to include mode
 * INPUT   : vid        - vlan id
 *           gia_ap     - group ip array pointer
 *           input_port - the input port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    : because chagne mode, delete all src list in exclude list
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ChangeModeToIncludeMode(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T lport);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_SetMLdStatus
 *-------------------------------------------------------------------------
 * PURPOSE : This function set the mldsnp status
 * INPUT   : mldsnp_status -  the mldsnp status
 * OUTPUT  : None
 * RETURN  : checksum result
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_SetMLdStatus(
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENIGNE_ProcessRcvdPdu
*------------------------------------------------------------------------------
* Purpose: This function process the recevied mld pdu
* INPUT  :
*               mref_handle_p  - mref reference
*               dmac_ap        - destenation mac address array pointer
*               smac_ap        - source mac address array pointer
*               vid            - input vlan id
*               lport          - input port
*               pkt_len        - packet length
*               ip_ext_opt_len - extension length in ip packet
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENIGNE_ProcessRcvdPdu(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T     *dmac_ap,
    UI8_T     *smac_ap,
    UI16_T    vid,
    UI16_T    lport,
    UI16_T    pkt_len,
    UI16_T    ip_ext_opt_len);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_IsLegalGroupIP
*------------------------------------------------------------------------------
* Purpose: This function check group ip is legal
* INPUT  : *gip_ap      - the group ip
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_IsLegalGroupIP(
    UI8_T *gip_ap);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_AddPortStaticJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function join the port to group statically.
* INPUT   : vid          - the vlan id
*           *gip_ap      - the group ip
*           *sip_ap      - the source ip
*           lport        - the logical port
*           rec_type     - the include or exclude mode
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_AddPortStaticJoinGroup(
    UI16_T                   vid,
    UI8_T                    *gip_ap,
    UI8_T                    *sip_ap,
    UI16_T                   lport,
    MLDSNP_TYPE_RecordType_T rec_type);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DeletePortStaticJoinGroup
*------------------------------------------------------------------------------
* Purpose: This function remove the port statically join the group
* INPUT  :vid        - the vlan id
*         *gip_ap    - the group ip
*         *sip_ap    - the source ip
*         lport      - the logical port
* OUTPUT :TRUE - success
*         FALSE- fail
* RETURN :None
* NOTES  : Don't care the fileter mode. just remove the group.
*          when filter mode time out, it will delete itself if it find no port exist in (vid,group,*)
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_DeletePortStaticJoinGroup(
    UI16_T   vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_CarryConfigFromOnePortToAnother
*------------------------------------------------------------------------------
* Purpose: This function carry the configuration from member port to trunk port, or from trunk to member.
* INPUT  :from_ifindex        - carry config from which port
*         *to_ifindex    - to config on which port
* OUTPUT :TRUE - success
*         FALSE- fail
* RETURN :None
* NOTES  : 
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_CarryConfigFromOnePortToAnother(
    UI32_T  from_ifindex,
    UI32_T  to_ifindex);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_InheritConfigFromTrunkToPort
*------------------------------------------------------------------------------
* Purpose: This function inherit the configuration from trunk port to member port
* INPUT  :member_ifindex        - member port
*         *trunk_ifindex    - trunk port
* OUTPUT :TRUE - success
*         FALSE- fail
* RETURN :None
* NOTES  : 
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_InheritConfigFromTrunkToPort(
    UI32_T  member_ifindex,
    UI32_T  trunk_ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_FindAllPortsInVlan
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to find all ports on this vlan.
 * INPUT   : *vid                - vlan id
 *           *linear_portlist_ap - linear port list buffer
 * OUTPUT  : *linear_portlist    - linear port list
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_FindAllPortsInVlan(
    UI16_T vid,
    UI8_T *linear_portlist_ap);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_FindAllRouterPortsInVlan
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to find all ports on this vlan.
 * INPUT   : *vid                - vlan id
 *           *linear_portlist_ap - linear port list buffer
 * OUTPUT  : *linear_portlist    - linear port list
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_FindAllRouterPortsInVlan(
    UI16_T vid,
    UI8_T *linear_portlist_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_GetConfiguredIPv6Address
 *-------------------------------------------------------------------------
 * PURPOSE : This function get this switch configured ipv6 address
 * INPUT    : vid -the vlan id to get ip
 * OUTPUT  : *addr_ap - the ipv6 address
 * RETURN  : TRUE  - has to ipv6 address
 *           FALSE - can't get ipv6 address
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_GetConfiguredIPv6Address(
    UI16_T vid,
    UI8_T *addr_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_GetReportSrcIpv6Address
 *-------------------------------------------------------------------------
 * PURPOSE : This function get the ipv6 address
 * INPUT    : vid -the vlan id to get ip
 * OUTPUT  : *addr_ap - the ipv6 address
 * RETURN  : TRUE  - has to ipv6 address
 *           FALSE - can't get ipv6 address
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_GetReportSrcIpv6Address(
    UI16_T vid,
    UI8_T *addr_ap);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_PortLeaveGroup
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the group
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          lport    - the logical port
* OUTPUT : None
* RETURN : TRUE    - success
*          FALSE   - fail
* NOTES  :after call this function, the port info can't be used.
*------------------------------------------------------------------------------
*/
BOOL_T MLDSNP_ENGINE_PortLeaveGroup(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_PortLeaveAllGroup
*------------------------------------------------------------------------------
* Purpose: This function delete all the entry learned on the input port
* INPUT  : lport- the logical port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_PortLeaveAllGroup(
    UI16_T lport);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DeleteAllDynamicGroupInVlan
*------------------------------------------------------------------------------
* Purpose: This function process delete all the vlan's entry
* INPUT  : vid  - the vlan id
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :because delete all, so 1. stop all timer, 2. write chip, 3.call om to delete it all
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_DeleteAllDynamicGroupInVlan(
    UI16_T vid);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_PortLeaveVlanAllGroup
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the all group in the input vlan
* INPUT  : vid      - the vlan id
*          port     - the logical port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :shall we remove the router port registered group? it may exit other router port.
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_PortLeaveVlanAllGroup(
    UI16_T vid,
    UI16_T lport);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DeleteAllGroupInVlan
*------------------------------------------------------------------------------
* Purpose: This function process delete all the vlan's entry
* INPUT  : vid  - the vlan id
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :because delete all, so 1. stop all timer, 2. write chip, 3.call om to delete it all
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_DeleteAllGroupInVlan(
    UI16_T vid);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DeleteAllGroup
*------------------------------------------------------------------------------
* Purpose: This function process delete all entry
* INPUT  : None
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_DeleteAllEntry();

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENIGNE_VlanCreated
*------------------------------------------------------------------------------
* Purpose: This function process create the vlan info
* INPUT  : vid  - the vlan id
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENIGNE_VlanCreated(
    UI16_T vid);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENIGNE_VlanDestroy
*------------------------------------------------------------------------------
* Purpose: This function process vlan destroy
* INPUT  : vid  - the vlan id
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENIGNE_VlanDestroy(
    UI16_T vid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IsExcludeMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function record the mode change to exclude
 * INPUT   : vid        - vlan id
 *           gia_ap     - group ip array pointer
 *           input_port - the input port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    : if(vid, g, 0) has port register, this is mean this (vid, g, sip_list) is in exclude mode.
 *               the filter_timer is put at port register at (vid, g, 0)
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_IsExcludeMode(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T input_port);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_FilterTimerTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the exclude mode time out
* INPUT  :
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :change mode to include mode
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_FilterTimerTimeout(
    void * para_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_SourceTimerTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the source ip timer timeout
* INPUT  :
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_SourceTimerTimeout(
    void * para_p);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_V1HostPresentTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the v1 host present time out
* INPUT  :
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_V1HostPresentTimeout(
    void * para_p);
/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DeletePort
*------------------------------------------------------------------------------
* Purpose: This function process a port is deleted, it means it won't exist.
* INPUT  : lport - the port will be deleted
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_DeletePort(UI16_T lport);


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_RecoveryStaticJoin
*------------------------------------------------------------------------------
* Purpose: when querier is enabled or has mrouter port, all static entry shalall be re jion again.
* INPUT  : vid - which vlan id the join static group.
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  : vid =0 means all vlan, only querier enabled shall use this vid =0
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_RecoveryStaticJoin(UI16_T vid);

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_IsVlanActive
*------------------------------------------------------------------------------
* Purpose: This function check mldsnp and querier status and router port count.
* INPUT  : vid - vlan id
* OUTPUT : None
* RETURN : TRUE -- success
*          FALSE -- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_IsVlanActive(UI16_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_RemoveDynamicGroupbyCount
 *-------------------------------------------------------------------------
 * PURPOSE : Remove group by input remove_count
 * INPUT   : lport        - which port to delete group
 *           remove_count - remove how many groups
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_RemoveDynamicGroupbyCount(UI32_T lport, UI32_T remove_count);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ShallFilterGroup
 *-------------------------------------------------------------------------
 * PURPOSE :check this group shall be filtered
 * INPUT   : *grp_ap - group address array
 *           lport   - which port to check
 * OUTPUT  : None
 * RETURN  : deny/permit/flood
 * NOTE    :
 *-------------------------------------------------------------------------
 */
UI32_T MLDSNP_ENGINE_ShallFilterGroup(UI8_T *groupAddress, UI32_T inPortNo);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ShallThrottleGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Check shall throttle the new report on this port
 * INPUT   : lport  - which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ShallThrottleGroup(UI32_T lport);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_AddPortCounterByPortlist
 *-------------------------------------------------------------------------
 * PURPOSE : Loop each port and the port statistic
 * INPUT   : type               - which type statistics need to increase
 *           output_portlist_ap - portbitmap
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_Statistics_T type, UI8_T *output_portlist_ap);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IsPortInIncludeMode
 *-------------------------------------------------------------------------
 * PURPOSE : Check join list has a port in include mode
 * INPUT   : port_list - (S,G)register list
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_IsPortInIncludeMode(L_SORT_LST_List_T *port_list);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst
 *-------------------------------------------------------------------------
 * PURPOSE : add leaved G or (S,G) to linklist for later used
 * INPUT   : grp_ap - group array
 *           src_ap - source address array
 *           list_p - which link list to store
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(UI8_T *grp_ap, UI8_T *src_ap, struct L_list *list_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ProcessSendStoreList
 *-------------------------------------------------------------------------
 * PURPOSE : Process store the leave (S,G) to send report or done
 * INPUT   : vid   - which vlan
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
void MLDSNP_ENGINE_ProcessSendStoreList(UI16_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ProxySendV1Report
 *-------------------------------------------------------------------------
 * PURPOSE : Send v1 report
 * INPUT   : vid   - which vlan
 *           gip_ap- group array
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxySendV1Report(UI16_T vid, UI8_T *gip_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ProxySendV1Done
 *-------------------------------------------------------------------------
 * PURPOSE : Send done message
 * INPUT   : vid   - which vlan
 *           gip_ap- group array
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxySendV1Done(UI16_T vid, UI8_T *gip_ap);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ProxySendV2Report
 *-------------------------------------------------------------------------
 * PURPOSE : Send V2 report
 * INPUT   : vid         - which vlan
 *           gip_ap      - group array
 *           sip_list_ap - source list array
 *           num_of_src  - number of source
 *           rec_type    - record type
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxySendV2Report(UI16_T vid, UI8_T *gip_ap, UI8_T *sip_list_ap, UI16_T num_of_src, MLDSNP_TYPE_RecordType_T rec_type);
/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ProxyLeaveAllGroup
 *-------------------------------------------------------------------------
 * PURPOSE : Proxy send leave all group
 * INPUT   : vid - which vlan to send report
 * OUTPUT  : None
 * RETURN  : TRUE -- success
 *           FALSE-- fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxyLeaveAllGroup(UI16_T vid);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_EncodeGroupRecord
 *-------------------------------------------------------------------------
 * PURPOSE : encode a group record
 * INPUT   : cur_rec_pp- current record pointer
 *           pdu_len_p - current packet length
 *           gip_ap    - group array
 *           sip_ap    - source array
 *           rec_type  - record type
 *           new_record- need to create new record
 * OUTPUT  : pdu_len_p - current packet length
 * RETURN  : TRUE  -  sueccess
 *           FALSE -  no more space to store
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_EncodeGroupRecord(UI8_T **cur_rec_pp,
                                       UI32_T *pdu_len_p,
                                       UI8_T *gip_ap,
                                       UI8_T *sip_ap,
                                       UI16_T num_of_rec,
                                       MLDSNP_TYPE_RecordType_T rec_type,
                                       BOOL_T new_record);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_UnsolicitTimeout
 *-------------------------------------------------------------------------
 * PURPOSE : Process unsolicite timer timeout action
 * INPUT   : para_p  - timer parameter
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_UnsolicitTimeout(
    void * para_p);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_StartUnsolicittimer
 *-------------------------------------------------------------------------
 * PURPOSE : start/stop unsolicit report timer
 * INPUT   : enabled - start or stop
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_StartUnsolicittimer(BOOL_T enabled);
/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ProxyReplyGSQuery
 *-------------------------------------------------------------------------
 * PURPOSE : Proxy reply g-s/g-s-s query
 * INPUT   : vid - which vlan to send report
 *           gip - which group
 *           sip_list- source list
 *           num_src - number of source
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxyReplyGSQuery(UI16_T vid, UI8_T gip_ap[], UI8_T *src_list_p, UI16_T num_src);
/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ProxySendUnsolicitReports
 *-------------------------------------------------------------------------
 * PURPOSE : Send unsolicit report for specify vlan
 * INPUT   : vid - which vlan to send report
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_ProxySendUnsolicitReports(UI16_T vid);
#endif
#endif/* End of mldsnp_engine_H */


