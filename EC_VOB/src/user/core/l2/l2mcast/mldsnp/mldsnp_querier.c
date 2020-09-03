/* MODULE NAME: mldsnp_querier.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*                ++++++++++++
*                +mlsdnp_mgr+
*                ++++++++++++

*                 ++++++++++++++++
*                 + mldsnp_engine+
*                 ++++++++++++++++

*         ++++++++++++++++++        ++++++++++++++++++
*         + mldsnp_unknown +        +mldsnp_querier  +
*         ++++++++++++++++++        ++++++++++++++++++
*
* +++++++++++
* +mldsnp_om+
* +++++++++++
*                              +++++
*                              +msl+
*                              +++++
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/

/* INCLUDE FILE DECLARATIONS
*/
#include "mldsnp_querier.h"
#include "mldsnp_timer.h"
#include "mldsnp_om.h"
#include "mldsnp_engine.h"
#include "mldsnp_backdoor.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "swctrl.h"
#include "sys_time.h"
#include "l2mux_mgr.h"
#include "msl_pmgr.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "xstp_mgr.h"
#include "l_cvrt.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define GROUP_LEN           MLDSNP_TYPE_IPV6_DST_IP_LEN
#define SRC_IP_LEN          MLDSNP_TYPE_IPV6_SRC_IP_LEN

/* MACRO FUNCTION DECLARATIONS
*/
/* DATA TYPE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
static BOOL_T MLDSNP_QUERIER_DeleteDynamicRouterPort(
    UI16_T vid,
    UI16_T lport);

/* STATIC VARIABLE DEFINITIONS
*/
static  UI8_T general_query_dip_ag[GROUP_LEN] = {  0xff, 0x02, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01
                                                };
static UI8_T general_query_dst_mac_ag[SYS_ADPT_MAC_ADDR_LEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01};
static UI8_T src_ip_list_save_aa[MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC][SRC_IP_LEN];
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
static UI8_T src_ip_block_list_save_aa[MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC][SRC_IP_LEN];
#endif
extern UI8_T mldsnp_om_null_src_ip_a[SYS_ADPT_IPV6_ADDR_LEN];
extern UI8_T mldsnp_om_null_group_ip_a[MLDSNP_TYPE_IPV6_DST_IP_LEN];

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
extern struct L_list mldsnp_engine_join_lst;
extern struct L_list mldsnp_engine_leave_lst;
#endif

/* LOCAL SUBPROGRAM BODIES
*/
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_GetIpv6Address
 *-------------------------------------------------------------------------
 * PURPOSE : This function get the ipv6 address
 * INPUT    : vid -the vlan id to get ip
 * OUTPUT  : *addr_ap - the ipv6 address
 * RETURN  : TRUE  - has to ipv6 address
 *           FALSE - can't get ipv6 address
 * NOTE    : If it can't get local, it will get other querier's
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_QUERIER_GetIpv6Address(
    UI16_T vid,
    UI8_T *addr_ap)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    if (TRUE == MLDSNP_ENGINE_GetConfiguredIPv6Address(vid, addr_ap))
    {
        return TRUE;
    }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (proxy_status == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
    {
        MLDSNP_OM_VlanInfo_T vlan_info;
        if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            IPV6_ADDR_COPY(addr_ap, vlan_info.other_querier_src_ip);
            return TRUE;
        }
    }
#endif
    return TRUE;
    /* return false will cause packet won't be send due to null source ip. */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_IsQueryFromLowerIp
 *-------------------------------------------------------------------------
 * PURPOSE : Check if the Query is from lower IP
 * INPUT   : vid     - the vlan id
 *           *sip_ap - the source ip
 * OUTPUT  : None
 * RETURN  : TRUE  - is from lower IP
 *           FALSE - is not lower IP
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_QUERIER_IsQueryFromLowerIp(
    UI16_T vid,
    UI8_T *sip_ap)
{
    UI8_T src_ip_a[SRC_IP_LEN] = {0};

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, src_ip_a))
    {
        return TRUE;
    }

    if (memcmp(src_ip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN) != 0 /*if myself is null src ip, always loss election*/
            && memcmp(sip_ap, src_ip_a, SRC_IP_LEN) >= 0)
    {
        MLDSNP_BD(TRACE, "query is not from lower src ip address");
        return FALSE;
    }

    MLDSNP_BD(TRACE, "query is from lower src ip address");

    return TRUE;
}/* End MLDSNP_QUERIER_IsQueryFromLowerIp() */
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_CalQQI
*------------------------------------------------------------------------------
* Purpose: This function calculate the QQI field in query packet
* INPUT  : vid   - vlan id
*          rport - router port
* OUTPUT :None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static UI8_T MLDSNP_QUERIER_CalQQI(
    UI8_T value)
{
    return L_CVRT_16bitsInt_to_8bitsFloat(value);
#if 0
    UI8_T code = 0, exp = 0;
    UI8_T mant = value;

    if (mant < 0x80)
        return mant;

    while (mant > 0x1f)
    {
        exp ++;
        mant = mant >> 1;
    }
    mant -= 0x10;
    exp  -= 3;

    code = 0x80 + (exp << 4) + mant;

    return code;
#endif
}/*End of MLDSNP_QUERIER_CalQQI*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_CalMaxRespCode
*------------------------------------------------------------------------------
* Purpose: This function calculate the max response code
* INPUT  : vid   - vlan id
*          rport - router port
* OUTPUT :None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static UI16_T MLDSNP_QUERIER_CalMaxRespCode(
    UI16_T value)
{
    return L_CVRT_32bitsInt_to_16BitsFloat(value*1000);

#if 0
    UI16_T code, mant = value * 1000;
    UI8_T  exp = 0;

    if (mant < 0x8000)
        return mant;

    while (mant > 0x1fff)
    {
        exp ++;
        mant = mant >> 1;
    }
    mant -= 0x1000;
    exp -= 3;

    code = 0x8000 + (exp << 12) + mant;

    return code;
#endif
}/*End of MLDSNP_QUERIER_CalMaxRespCode*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_AddRouterPortToAllChipEntry
*------------------------------------------------------------------------------
* Purpose: This function add router port to all entry in chip
* INPUT  : vid  - vlan id
*          rport - router port
* OUTPUT :None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static void MLDSNP_QUERIER_AddRouterPortToAllChipEntry(
    UI16_T vid,
    UI16_T rport)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    UI16_T nxt_vid = vid;
    UI8_T  nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

    /*(vid, gip, sip)*/
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid,  nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (nxt_vid != vid)
            break;

#if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
        msl_pmgr_mldsnp_entry_add(0,  nxt_gip_a, nxt_sip_a, vid, 1, rport);
#else
        msl_pmgr_mldsnp_entry_add(0,  nxt_gip_a, mldsnp_om_null_src_ip_a, vid, 1, rport);
#endif
    }

    return;
}/*End of MLDSNP_QUERIER_AddRouterPortToAllChipEntry*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_AddRouterPortToAllChipEntry
*------------------------------------------------------------------------------
* Purpose: This function add router port to all entry in chip
* INPUT  : vid   - vlan id
*          rport - router port
* OUTPUT :None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static void MLDSNP_QUERIER_DeleteRouterPortFromAllChipEntry(
    UI16_T vid,
    UI16_T rport)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    UI16_T nxt_vid = vid;
    UI8_T  nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

    /*(vid, gip, *)*/
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid,  nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (nxt_vid != vid)
            break;

        msl_pmgr_mldsnp_entry_del(0,  nxt_gip_a, nxt_sip_a, vid, 1, rport);
    }

    return;
}/*End of MLDSNP_QUERIER_AddRouterPortToAllChipEntry*/

#if 0
/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_QUERIER_AddPortBitmapToChipEntry
*-------------------------------------------------------------------------
* PURPOSE : add one port into the chip enty
* INPUT   : vid           - the input vid
*           *gip_ap       - the group ip address
*           *sip_ap       - the number of src in this record
*           port_bitmap_a - the port bitmap
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_QUERIER_AddPortBitmapToChipEntry(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI8_T *port_bitmap_ap)
{
    UI32_T b = 0, i = 0, j = 0;
    UI16_T out_port_a[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI8_T  group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);

    for (; b < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; b++)
    {
        if (0 != port_bitmap_ap[b])
        {
            for (i = 1; i <= 8; i++)
            {
                if (port_bitmap_ap[b]&(0x80 >> (i - 1) % 8))
                {
                    out_port_a[j] = b * 8 + i;
                    j++;
                }
            }
        }
    }

    msl_pmgr_mldsnp_entry_add_portlist(0, group_ip, source_ip, vid, j, out_port_a);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port bitmap: %02x%02x%02x",
                             group_ip, source_ip, 1, vid, port_bitmap_ap[0], port_bitmap_ap[1], port_bitmap_ap[2]);
    return TRUE;
}/*MLDSNP_ENGINE_AddPortBitmapToChipEntry*/
#endif
/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_QUERIER_DeletePortFromChipEntry
*-------------------------------------------------------------------------
* PURPOSE : Delete one port from the chip enty
* INPUT   : vid        - the input vid
*           *gip_ap    - the group ip address
*           *sip_ap    - the number of src in this record
*           lport      - the logical port
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_QUERIER_DeletePortFromChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport)
{
    UI8_T source_ip[SRC_IP_LEN] = {0};

    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);

    msl_pmgr_mldsnp_entry_del(0,  gip_ap, source_ip, vid, 1, lport);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port =%d", gip_ap, sip_ap, 1, vid, lport);

    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, sip_ap);

    return TRUE;
}/*end of MLDSNP_QUERIER_DeletePortFromChipEntry*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructIpHeader
*------------------------------------------------------------------------------
* Purpose: This function contruct the ipv6 header
* INPUT  : *ipv6_header_p - the pointer start after ether type
*          *dip_ap        - the destination ipv6 address
*          *sip_ap        - the source ipv6 address
*          payload_len    - the payload length
* OUTPUT : *ipv6_header_p - the assigned content pdu
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_QUERIER_ConstructIpHeader(
    MLDSNP_ENGINE_IPv6Header_T *ipv6_header_p,
    UI8_T                      *dip_ap,
    UI8_T                      *sip_ap,
    UI16_T                     payload_len)
{
    ipv6_header_p->ver_class_label = L_STDLIB_Hton32(0x60300000); /*ver, traffic class, flow label*/
    ipv6_header_p->payload_len     = L_STDLIB_Hton16(payload_len + MLDSNP_TYPE_HOP_BY_HOP_LEN);
    ipv6_header_p->next_header     = MLDSNP_TYPE_IPV6_HOP_BY_HOP_HEAD;    /*next header is hop-by-hop header*/
    ipv6_header_p->hop_lim         = 1;
    memcpy(ipv6_header_p->sip_a, sip_ap, SRC_IP_LEN);
    memcpy(ipv6_header_p->dip_a, dip_ap, GROUP_LEN);
    ipv6_header_p->hop_by_hop[0] = MLDSNP_TYPE_IPV6_ICMPV6_HAED;
    ipv6_header_p->hop_by_hop[1] = 0x00;
    ipv6_header_p->hop_by_hop[2] = 0x00;
    ipv6_header_p->hop_by_hop[3] = 0x00;
    ipv6_header_p->hop_by_hop[4] = 0x05;
    ipv6_header_p->hop_by_hop[5] = 0x02;
    ipv6_header_p->hop_by_hop[6] = 0x00;
    ipv6_header_p->hop_by_hop[7] = 0x00;
    return TRUE;
}/*End of MLDSNP_QUERIER_ConstructIpHeader*/




/* EXPORTED SUBPROGRAM BODIES
*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructGeneralQuery
*------------------------------------------------------------------------------
* Purpose: This function construct the group qeury
* INPUT  : *pdu_p  - the pdu content to assign the value
*              vid - the vlan to send genery query
* OUTPUT : *pdu_p  - the assing valued pdu
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_QUERIER_ConstructGeneralQuery(
    UI8_T *pdu_p,
    UI16_T vid,
    UI16_T snp_ver)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p   = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    MLDSNP_ENGINE_Query_T      *query_p     = (MLDSNP_ENGINE_Query_T *) & mld_pdu_p->icmpv6;
    MLDSNP_OM_VlanInfo_T       vlan_info;
    UI16_T max_res_time = 0;
    UI8_T sip_a[SRC_IP_LEN] = {0};/*this vlan configured ip, but now still doesn't know how to get*/

    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, sip_a))
        return FALSE;

    /*assign the vlaue of mld query
     */
    query_p->type = MLDSNP_TYPE_QUERY;
    /*the gip address of general query in icmp is 0*/
    memset(query_p->gip_a, 0, GROUP_LEN);

    MLDSNP_OM_GetQueryResponseInterval(&max_res_time);
    query_p->max_rsp_code = L_STDLIB_Hton16(MLDSNP_QUERIER_CalMaxRespCode(max_res_time));

    MLDSNP_OM_GetVlanInfo(vid, &vlan_info);

    if (MLDSNP_TYPE_QUERIER_DISABLED == vlan_info.querier_runing_status)
    { /*rfc4541, page 2.1.1 4) page 5, if not querier then send null sip*/
        MLDSNP_BD_ARG(TX, "vid %d querier is not running so set the sip to null\r\n", vid);
        memset(sip_a, 0, SRC_IP_LEN);
    }


    if (MLDSNP_TYPE_VERSION_1 == snp_ver
            || vlan_info.old_ver_querier_exist)
    {
        MLDSNP_BD(TX, "create v1 general query ");
        /*assign the IPv6 header*/
        MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, general_query_dip_ag, sip_a, MLDSNP_TYPE_QUERY_V1_LEN);

        query_p->check_sum = MLDSNP_QUERIER_GenerateCheckSum((UI8_T *) & mld_pdu_p->icmpv6, sip_a, general_query_dip_ag,
                                 MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                 MLDSNP_TYPE_QUERY_V1_LEN);
    }
    else /*V2*/
    {
        UI16_T value = 0;

        MLDSNP_BD(TX, "create v2 general query ");

        MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, general_query_dip_ag, sip_a, MLDSNP_TYPE_QUERY_V2_LEN);

        memset(query_p->gip_a, 0, GROUP_LEN);

        MLDSNP_OM_GetRobustnessValue(&value);
        if(value > 7) /*Ref 5.1.8, exceed 7, the QRV field is set to zero*/
          query_p->qrv = 0;
        else
        query_p->qrv = value;
        query_p->s   = FALSE;

        MLDSNP_OM_GetQueryInterval(&value);
        query_p->qqic       = MLDSNP_QUERIER_CalQQI(value);
        query_p->num_of_src = 0;

        query_p->check_sum  = MLDSNP_QUERIER_GenerateCheckSum(
                                  (UI8_T *) & mld_pdu_p->icmpv6,
                                  sip_a,
                                  general_query_dip_ag,
                                  MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                  MLDSNP_TYPE_QUERY_V2_LEN);
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_ConstructGeneralQuery*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructGroupSpecifyQuery
*------------------------------------------------------------------------------
* Purpose: This function construct the group specify qeury
* INPUT  : *pdu_p         - the pdu content to assign the value
*          vid - the vlan id to send th packet
*          *specify_gip_p - the specify group ip
*          version     - the snooping version now
* OUTPUT : *pdu_p         - the assing valued pdu
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_QUERIER_ConstructGroupSpecifyQuery(
    UI8_T  *pdu_p,
    UI16_T vid,
    UI8_T  *specify_gip_p,
    MLDSNP_TYPE_Version_T version)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    MLDSNP_ENGINE_Query_T      *query_p = (MLDSNP_ENGINE_Query_T *) & mld_pdu_p->icmpv6;
    MLDSNP_OM_VlanInfo_T       vlan_info;
    UI16_T llqi = 0;
    UI8_T sip_a[GROUP_LEN] = {0};

    MLDSNP_BD(TRACE, " ");

    memset(pdu_p, 0, sizeof(MLDSNP_ENGINE_MldPdu_T));

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, sip_a))
        return FALSE;

    /*assign the vlaue of mld query
     */
    query_p->type = 130;
    /*the gip address of general query in icmp is 0*/
    memcpy(query_p->gip_a, specify_gip_p, GROUP_LEN);

    MLDSNP_OM_GetLastListenerQueryInterval(&llqi);
    query_p->max_rsp_code = L_STDLIB_Hton16(MLDSNP_QUERIER_CalMaxRespCode(llqi));

    MLDSNP_OM_GetVlanInfo(vid, &vlan_info);

    if (MLDSNP_TYPE_VERSION_1 == version
            || vlan_info.old_ver_querier_exist)
    {
        MLDSNP_BD(TX, "create v1 group specify query ");
        /*assign the IPv6 header*/
        MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, specify_gip_p, sip_a, MLDSNP_TYPE_QUERY_V1_LEN);

        query_p->check_sum = MLDSNP_QUERIER_GenerateCheckSum(
                                 (UI8_T *) & mld_pdu_p->icmpv6,
                                 sip_a,
                                 specify_gip_p,
                                 MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                 MLDSNP_TYPE_QUERY_V1_LEN);
    }
    else /*V2*/
    {
        UI16_T value = 0;

        MLDSNP_BD(TX, "create v2 group specify query ");
        /*assign the IPv6 header, +4 mean Resv, S, QRV, QQIC, Numb of src*/
        MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, specify_gip_p, sip_a, MLDSNP_TYPE_QUERY_V2_LEN);

        MLDSNP_OM_GetRobustnessValue(&value);
        if(value > 7) /*Ref 5.1.8, exceed 7, the QRV field is set to zero*/
          query_p->qrv = 0;
        else
        query_p->qrv = value;
        query_p->s   = FALSE;

        MLDSNP_OM_GetQueryInterval(&value);
        query_p->qqic = MLDSNP_QUERIER_CalQQI(value);

        query_p->check_sum = MLDSNP_QUERIER_GenerateCheckSum(
                                 (UI8_T *) & mld_pdu_p->icmpv6,
                                 sip_a,
                                 specify_gip_p,
                                 MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                 MLDSNP_TYPE_QUERY_V2_LEN);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructGroupSourceSpecifyQuery
*------------------------------------------------------------------------------
* Purpose: This function construct the group source specify qeury
* INPUT  : *pdu_p          - the pdu content to assign the value
*          vid                   - the vlan id
*          *specify_gip_p  - the group ip
*          *specify_sip_p  - the source address ip
*          specify_sip_num - the number of spcified source ip
*          pdu_len             - the pdu len
* OUTPUT : *pdu_p     - the assing valued pdu
* RETURN : TRUE   - Construct Success
*          FALSE  - Consruct Fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_QUERIER_ConstructGroupSourceSpecifyQuery(
    UI8_T  *pdu_p,
    UI16_T vid,
    UI8_T  *specify_gip_p,
    UI8_T  *specify_sip_p,
    UI16_T specify_sip_num,
    UI32_T pdu_len)
{
    MLDSNP_ENGINE_MldPdu_T      *mld_pdu_p = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    MLDSNP_ENGINE_IPv6Header_T  *ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    MLDSNP_ENGINE_Query_T       *query_p = (MLDSNP_ENGINE_Query_T *) & mld_pdu_p->icmpv6;
    UI16_T llqi = 0, value = 0, idx;
    UI8_T  sip_a[SRC_IP_LEN] = {0};

    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, sip_a))
        return FALSE;

    MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, specify_gip_p, sip_a, MLDSNP_TYPE_ICMPV6_COMMON_LEN + 4 + specify_sip_num*SRC_IP_LEN);

    /*assign the vlaue of mld query
     */
    query_p->type = 130;
    query_p->code = 0;

    MLDSNP_OM_GetLastListenerQueryInterval(&llqi);
    query_p->max_rsp_code = L_STDLIB_Hton16(MLDSNP_QUERIER_CalMaxRespCode(llqi));
    query_p->num_of_src   = specify_sip_num;

    memcpy(query_p->gip_a, specify_gip_p, GROUP_LEN);

    /*assign  flag*/
    MLDSNP_OM_GetRobustnessValue(&value);
    if(value > 7) /*Ref 5.1.8, exceed 7, the QRV field is set to zero*/
      query_p->qrv = 0;
    else
    query_p->qrv  = value;
    query_p->s    = FALSE;

    MLDSNP_OM_GetQueryInterval(&value);
    query_p->qqic = MLDSNP_QUERIER_CalQQI(value);

    /*assign src_ip*/
    for (idx = 0; idx < specify_sip_num; idx++)
    {
        memcpy(&query_p->sip_aa[idx], specify_sip_p + idx*SRC_IP_LEN, SRC_IP_LEN);
    }

    query_p->check_sum    = 0;
    mld_pdu_p->icmpv6.checksum = MLDSNP_QUERIER_GenerateCheckSum(
                                     (UI8_T *)& mld_pdu_p->icmpv6,
                                     sip_a,
                                     specify_gip_p,
                                     MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                     pdu_len);

    return TRUE;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructMrdSolicitation
*------------------------------------------------------------------------------
* Purpose: This function construct MRD solicitation message
* INPUT  : vid  - vlan id
*          *pdu_p - pdu ponter to asssign value
* OUTPUT : *pdu_p - the pdu content
* RETURN : TRUE   - Construct Success
*          FALSE  - Consruct Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ConstructMrdSolicitation(
    UI16_T vid,
    UI8_T *pdu_p)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    UI8_T sip_a[SRC_IP_LEN] = {0};
    UI8_T mrd_dip[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0xff, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x6a
                                                 };
    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, sip_a))
        return FALSE;

    /*assign the vlaue of multicast router solicitation
     */
    mld_pdu_p->icmpv6.type = MLDSNP_TYPE_MRD_SOLICITATION;
    /*assign the IPv6 header*/
    MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, mrd_dip, sip_a, MLDSNP_TYPE_MRD_SOLICITAION_LEN);

    mld_pdu_p->icmpv6.checksum = MLDSNP_QUERIER_GenerateCheckSum(
                                     (UI8_T *) & mld_pdu_p->icmpv6,
                                     sip_a,
                                     mrd_dip,
                                     MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                     MLDSNP_TYPE_MRD_SOLICITAION_LEN);
    return TRUE;
}/*End of MLDSNP_QUERIER_ConstructMrdSolicitation*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ConstructMrdAdvertisement
*------------------------------------------------------------------------------
* Purpose: This function contruct the MRD advertisement message
* INPUT  : vid  - vlan id
*          *pdu_p - pdu ponter to asssign value
* OUTPUT : *pdu_p - the pdu content
* RETURN : TRUE   - Construct Success
*          FALSE  - Consruct Fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ConstructMrdAdvertisement(
    UI16_T vid,
    UI8_T *pdu_p)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    UI8_T sip_a[SRC_IP_LEN] = {0};
    UI8_T mrd_dip[MLDSNP_TYPE_IPV6_DST_IP_LEN] = {0xff, 0x02, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x6a
                                                 };
    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_QUERIER_GetIpv6Address(vid, sip_a))
        return FALSE;

    /*assign the vlaue of multicast router solicitation
     */
    mld_pdu_p->icmpv6.type = MLDSNP_TYPE_MRD_ADVERTISEMENT;
    mld_pdu_p->icmpv6.code = 20; /*default advertisement interval is 20s*/

    MLDSNP_OM_GetQueryInterval((UI16_T *)mld_pdu_p->icmpv6.head_a);
    MLDSNP_OM_GetRobustnessValue((UI16_T *)&mld_pdu_p->icmpv6.head_a[2]);

    /*assign the IPv6 header*/
    MLDSNP_QUERIER_ConstructIpHeader(ipv6_head_p, mrd_dip, sip_a, MLDSNP_TYPE_MRD_ADVERTISEMENT_LEN);

    mld_pdu_p->icmpv6.checksum = MLDSNP_QUERIER_GenerateCheckSum(
                                     (UI8_T *) & mld_pdu_p->icmpv6,
                                     sip_a,
                                     mrd_dip,
                                     MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                     MLDSNP_TYPE_MRD_ADVERTISEMENT_LEN);
    return TRUE;
}/*End of MLDSNP_QUERIER_ConstructMrdSolicitation*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendGeneralQeuryToSpecifyPort
*------------------------------------------------------------------------------
* Purpose: This function send the general query to the specified port
* INPUT  : vid            - the vlan id
*              lport          - the specified port
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static void MLDSNP_QUERIER_SendGeneralQeuryToSpecifyPort(
    UI16_T vid,
    UI16_T lport,
    MLDSNP_TYPE_Version_T ver)
{
    L_MM_Mref_Handle_T *mref_p = NULL;
    UI32_T             pdu_len = 0;
    UI32_T             vlan_ifindex;
    UI8_T              *pdu_p  = NULL;
    BOOL_T            is_tagged = FALSE;

    MLDSNP_BD(TRACE, "vid:%d, port:%d, ver:%d", vid, lport, ver);

    if (MLDSNP_TYPE_VERSION_1 == ver)
        pdu_len = MLDSNP_TYPE_QUERY_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    else
        pdu_len = MLDSNP_TYPE_QUERY_V2_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_GENERAL_QUERY_PER_PORT))))
    {
        MLDSNP_BD(TX, "mref is null");
        return;
    }

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        MLDSNP_BD(TX, "pdu_p is null");
        L_MM_Mref_Release(&mref_p);
        return;
    }

    memset(pdu_p, 0, pdu_len);

    /*construct the query packet content*/
    if (FALSE == MLDSNP_QUERIER_ConstructGeneralQuery(pdu_p, vid, ver))
    {
        L_MM_Mref_Release(&mref_p);
        return;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);
    is_tagged = !(VLAN_OM_IsVlanUntagPortListMember(vlan_ifindex, lport));

    MLDSNP_BD_ARG(TX, "vid=%d, lport=%d, dmac=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                  vid, lport, general_query_dst_mac_ag[0], general_query_dst_mac_ag[1], general_query_dst_mac_ag[2],
                  general_query_dst_mac_ag[3], general_query_dst_mac_ag[4], general_query_dst_mac_ag[5]);
    MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);

    {/*send the query pdu to driver*/
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};

        SWCTRL_GetCpuMac(src_mac_a);

        mref_p->next_usr_id = SYS_MODULE_L2MUX;

        L2MUX_MGR_SendPacket(mref_p,
                             general_query_dst_mac_ag,
                             src_mac_a,
                             MLDSNP_TYPE_IPV6_ETH_TYPE,
                             vid,
                             pdu_len,
                             lport,
                             is_tagged,
                             0,
                             FALSE);
    }
    return;
}/*ENd of MLDSNP_QUERIER_SendGeneralQeuryToSpecifyPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendMrdAdvertisement
*------------------------------------------------------------------------------
* Purpose: This function send the MRD advertisement
* INPUT  : vid            - the vlan id
*              lport          - the specified port
* OUTPUT : None
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static void MLDSNP_QUERIER_SendMrdAdvertisement(
    UI16_T vid,
    UI16_T lport)
{
    L_MM_Mref_Handle_T *mref_p = NULL;
    UI32_T             pdu_len = MLDSNP_TYPE_MRD_ADVERTISEMENT_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    UI32_T             vlan_ifindex;
    UI8_T              *pdu_p  = NULL;
    BOOL_T            is_tagged = FALSE;


    MLDSNP_BD(TRACE, "vid:%d, port:%d", vid, lport);

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_MRD_ADVERTISEMENT))))
    {
        MLDSNP_BD(TX, "mref is null");
        return;
    }

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        MLDSNP_BD(TX, "pdu_p is null");
        L_MM_Mref_Release(&mref_p);
        return;
    }

    memset(pdu_p, 0, pdu_len);

    /*construct the query packet content*/
    if (FALSE == MLDSNP_QUERIER_ConstructMrdAdvertisement(vid, pdu_p))
    {
        L_MM_Mref_Release(&mref_p);
        return;
    }

    VLAN_VID_CONVERTTO_IFINDEX(vid, vlan_ifindex);
    is_tagged = !(VLAN_OM_IsVlanUntagPortListMember(vlan_ifindex, lport));

    MLDSNP_BD_ARG(TX, "vid=%d, lport=%d, dmac=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                  vid, lport, general_query_dst_mac_ag[0], general_query_dst_mac_ag[1], general_query_dst_mac_ag[2],
                  general_query_dst_mac_ag[3], general_query_dst_mac_ag[4], general_query_dst_mac_ag[5]);
    MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);

    {/*send the query pdu to driver*/
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};

        SWCTRL_GetCpuMac(src_mac_a);

        mref_p->next_usr_id = SYS_MODULE_L2MUX;

        L2MUX_MGR_SendPacket(mref_p,
                             general_query_dst_mac_ag,
                             src_mac_a,
                             MLDSNP_TYPE_IPV6_ETH_TYPE,
                             vid,
                             pdu_len,
                             lport,
                             is_tagged,
                             0,
                             FALSE);
    }
    return;
}/*ENd of MLDSNP_QUERIER_SendMrdAdvertisement*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_AddDynamicRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to add the dynamic router port
 * INPUT   : vid   - the vlan id
 *           lport - the logical port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_QUERIER_AddDynamicRouterPort(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    UI16_T router_port_expire_time = 0;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetRouterExpireTime(&router_port_expire_time);

    if (FALSE == MLDSNP_OM_GetRouterPortInfo(vid, lport, &router_port_info))
    {
        MLDSNP_BD_ARG(RX, "Doesn't exist this router port %d, add this router port, time %lu",
                      lport, SYS_TIME_GetSystemTicksBy10ms());
        memset(&router_port_info, 0, sizeof(MLDSNP_OM_RouterPortInfo_T));

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_RouterPortTimeout,
                vid,
                mldsnp_om_null_group_ip_a,
                mldsnp_om_null_src_ip_a,
                0,
                lport,
                router_port_expire_time,
                MLDSNP_TIMER_ONE_TIME,
                &router_port_info.router_timer_p))
        {
            return FALSE;
        }

        router_port_info.attribute     = MLDSNP_TYPE_JOIN_DYNAMIC;
        router_port_info.register_time = SYS_TIME_GetSystemTicksBy10ms();
        router_port_info.port_no       = lport;

        if (FALSE == MLDSNP_OM_AddRouterPort(vid, &router_port_info))
        {
            MLDSNP_TIMER_StopAndFreeTimer(&router_port_info.router_timer_p);
            MLDSNP_BD_ARG(RX, "vid=%d, lport=%d add router port fail\r\n", vid, lport);
            return FALSE;
        }

        {/*add static port jion group*/
            UI16_T next_id = 0, out_vid, out_lport;
            UI8_T out_gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0}, out_sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0};
            MLDSNP_TYPE_RecordType_T out_rec_type;
            MLDSNP_TYPE_QuerierStatus_T querier_status;

            MLDSNP_OM_GetQuerierStauts(&querier_status);

            if (MLDSNP_OM_GetVlanRouterPortCount(vid) == 1
                    && MLDSNP_TYPE_QUERIER_DISABLED == querier_status
               )
            {
                while (TRUE == MLDSNP_OM_GetNextStaticPortJoinGroup(&next_id, &out_vid, out_gip_a, out_sip_a, &out_lport, &out_rec_type))
                {
                    if (vid == out_vid)
                        MLDSNP_ENGINE_AddPortStaticJoinGroup(out_vid, out_gip_a, out_sip_a, out_lport, out_rec_type);
                }
            }
        }

        /*because router port doesn't not record in om with joined group
           , it only used when add register port to chip, so here call msl directly to add the router port
           Or, we need search all group and call chip to add entry with this router port*/
        MLDSNP_QUERIER_AddRouterPortToAllChipEntry(vid, lport);

        msl_pmgr_mldsnp_rtport_add(0, vid, lport);
    }
    else
    {
        if (MLDSNP_TYPE_JOIN_STATIC == router_port_info.attribute)
        {
            MLDSNP_BD_ARG(RX, " This port %d is static router port\r\n", lport);
            return TRUE;
        }

        if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(router_port_info.router_timer_p, router_port_expire_time))
        {
            MLDSNP_BD_ARG(RX, "update router port %d timer fail\r\n", lport);
            return FALSE;
        }
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_AddDynamicRouterPort*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_UpdateOtherQuereirPresentTimer
 *-------------------------------------------------------------------------
 * PURPOSE : This function update other querier
 * INPUT   :  msg_p - the msg pointer
 * OUTPUT  : querier_running_status  - the querier running status
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static BOOL_T  MLDSNP_QUERIER_UpdateOtherQuereirPresentTimer(
    MLDNSP_ENGINE_Msg_T *msg_p,
    MLDSNP_TYPE_QuerierStatus_T *new_q_run_status_p)
{
    MLDSNP_TYPE_QuerierStatus_T org_q_run_status  = MLDSNP_TYPE_QUERIER_DISABLED, querier_status = MLDSNP_TYPE_QUERIER_DISABLED;
    MLDSNP_OM_VlanInfo_T        vlan_info;
    BOOL_T                      create_other_querier_present_timer = FALSE;

    MLDSNP_OM_GetQuerierStauts(&querier_status);

    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_OM_GetQuerierRunningStatus(msg_p->vid, &org_q_run_status))
    {
        return FALSE;
    }

    if (FALSE == MLDSNP_OM_GetVlanInfo(msg_p->vid, &vlan_info))
    {
        return FALSE;
    }

    *new_q_run_status_p = org_q_run_status;

    if (MLDSNP_TYPE_QUERIER_ENABLED == org_q_run_status) /*now queirer is running*/
    {
        /*check the other high ip address querier exist or not*/
        if (TRUE == MLDSNP_QUERIER_IsQueryFromLowerIp(msg_p->vid, msg_p->ipv6_header_p->sip_a))
        {
            create_other_querier_present_timer = TRUE;
        }
    }
    else  /*already loss election to be the querier, so update the other querier present timer*/
    {
        /*check still loss or not*/
        if (MLDSNP_TYPE_QUERIER_ENABLED == querier_status
            &&FALSE == MLDSNP_QUERIER_IsQueryFromLowerIp(msg_p->vid, msg_p->ipv6_header_p->sip_a))
        {
            /*now this switch become lower src ip, so change running status to enabled*/
            MLDSNP_TIMER_StopAndFreeTimer(&vlan_info.other_querier_present_timer_p);

            MLDSNP_OM_GetQueryInterval(&vlan_info.query_oper_interval);
            MLDSNP_OM_GetRobustnessValue(&vlan_info.robust_oper_value);

            /*vlan_info.other_querier_present_timer_p = NULL;*/
            vlan_info.querier_runing_status         = MLDSNP_TYPE_QUERIER_ENABLED;
            *new_q_run_status_p                     = MLDSNP_TYPE_QUERIER_ENABLED;

            if (FALSE == MLDSNP_OM_SetVlanInfo(&vlan_info))
            {
                return FALSE;
            }
            /*we are querier, set the querier_src_ip to this vlan configured */
            //MLDSNP_QUERIER_GetIpv6Address(msg_p->vid, vlan_info.other_querier_src_ip);
            MLDSNP_OM_SetVlanInfo(&vlan_info);
            return TRUE;
        }
        else
        {
            if (NULL == vlan_info.other_querier_present_timer_p)
            {
                create_other_querier_present_timer = TRUE;
            }
            else
            {
                if (!vlan_info.old_ver_querier_exist /*not exist old ver querier*/
                        && (msg_p->icmp_len == MLDSNP_TYPE_QUERY_V1_LEN ? TRUE : FALSE)) /*but receive v1 querier */
                {/*record the old ver src ip to check it will change to new ver or not*/
                    vlan_info.old_ver_querier_exist = TRUE;

                    /*if(vlan_info.old_ver_querier_exist)*/ /*if older ver exist then record the src ip to check the query from src ip is change to new version or not*/
                    memcpy(vlan_info.old_ver_querier_src_ip, msg_p->ipv6_header_p->sip_a, SRC_IP_LEN);
                }
                else if (0 == memcmp(vlan_info.old_ver_querier_src_ip, msg_p->ipv6_header_p->sip_a, SRC_IP_LEN)/*last is old ver*/
                         && (msg_p->icmp_len == MLDSNP_TYPE_QUERY_V2_LEN ? TRUE : FALSE) /*but receive v2 querier */)
                {/*clear the old ver querier record*/
                    vlan_info.old_ver_querier_exist = FALSE;
                    memset(vlan_info.old_ver_querier_src_ip, 0, SRC_IP_LEN);
                }

                MLDSNP_TIMER_UpdateTimerNewTime(vlan_info.other_querier_present_timer_p, MLDSNP_OM_GetOtherQueryPresentInterval());
                MLDSNP_OM_SetVlanInfo(&vlan_info);

                return TRUE;
            }
        }
    }

    /*create other querier present timer*/
    if (TRUE == create_other_querier_present_timer)
    {
        MLDSNP_Timer_T *other_querier_timer_p = NULL;

        *new_q_run_status_p = MLDSNP_TYPE_QUERIER_DISABLED;

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_OtherQuerierPrentTimeout,
                msg_p->vid,
                NULL,
                NULL,
                0,
                msg_p->recevied_port,
                MLDSNP_OM_GetOtherQueryPresentInterval(),
                MLDSNP_TIMER_ONE_TIME,
                &other_querier_timer_p))
        {
            return FALSE;
        }

        /*store other querier present timer
         */

        vlan_info.old_ver_querier_exist         = (msg_p->icmp_len == MLDSNP_TYPE_QUERY_V1_LEN ? TRUE : FALSE); /*v1 querier exist*/
        if (vlan_info.old_ver_querier_exist) /*if older ver exist then record the src ip to check the query from src ip is change to new version or not*/
            memcpy(vlan_info.old_ver_querier_src_ip, msg_p->ipv6_header_p->sip_a, SRC_IP_LEN);

        vlan_info.other_querier_present_timer_p = other_querier_timer_p;
        vlan_info.querier_runing_status         = MLDSNP_TYPE_QUERIER_DISABLED;
        vlan_info.other_querier_uptime          = SYSFUN_GetSysTick();
        vlan_info.querier_uptime = 0;

        if (vlan_info.old_ver_querier_exist)
        {
            MLDSNP_ENGINE_Query_T *query_p = (MLDSNP_ENGINE_Query_T *)msg_p->icmpv6_p;

            vlan_info.query_oper_interval = (query_p->qqic & 0x0f) << (((query_p->qqic & 0x70) >> 3) + 3);
            vlan_info.robust_oper_value   = query_p->qrv;
        }

        if (FALSE == MLDSNP_OM_SetVlanInfo(&vlan_info))
        {
            MLDSNP_TIMER_StopAndFreeTimer(&other_querier_timer_p);
            return FALSE;
        }

    }

    return TRUE;
}/*End of MLDSNP_QUERIER_UpdateOtherQuereirPresentTimer*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_UpdateOtherQuereirPresentTimer
 *-------------------------------------------------------------------------
 * PURPOSE : This function update other querier
 * INPUT   : vid      - the vlan id
 *           gip_ap   - the group address
 *           sip_ap   - the source ip array pointer
 *           num_src  - the number of source in sip_ap
 * OUTPUT  : None
 * RETURN  : TRUE   - success
 *           FALSE  - fail
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_QUERIER_IncreaseSpecificQueryCount(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI16_T num_of_src)
{
    MLDSNP_OM_PortInfo_T   port_info;
    MLDSNP_OM_HisamEntry_T entry_info;
    UI16_T robust_value = 0, LLQI = 0, idx, nxt_lport = 0;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T delete_count = 0;
#endif
    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetRobustnessOperValue(vid, &robust_value);
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    if (num_of_src != 0)
    {
        /*only check the src specified
          */
        for (idx = 0;idx < num_of_src; idx++, sip_ap += idx * SRC_IP_LEN)
        {
            if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
            {
                nxt_lport = 0;
                while (TRUE == MLDSNP_OM_GetNextPortInfo(vid, gip_ap, sip_ap, &nxt_lport, &port_info))
                {
                    if (MLDSNP_TYPE_JOIN_DYNAMIC != port_info.join_type)
                        continue;

                    port_info.specific_query_count ++;

                    if (port_info.specific_query_count < robust_value)
                    {
                        /*set (robust_value- port_info.specific_query_count) times LLQI ,
                         because if it don't receive g-s query,
                         it will still timeout accroding standard.
                         */
                        MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p,
                                                        LLQI * (robust_value - port_info.specific_query_count));
                        MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
                    }
                    else
                    {/*receive over robustnees g-s query, delete it directly*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                        delete_count++;
#endif
                        MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap, sip_ap, port_info.port_no);
                    }
                }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                        && entry_info.register_port_list.nbr_of_element == delete_count)
                {
                    MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, sip_ap, &mldsnp_engine_leave_lst);
                    delete_count = 0;
                }
#endif
            }
        }
    }
    else
    {
        /*check all entry
        */
        MLDSNP_OM_HisamEntry_T entry_info;
        UI16_T nxt_vid = vid;
        UI8_T nxt_gip_a[GROUP_LEN] = {0}, nxt_src_ip[SRC_IP_LEN] = {0};

        /*check(vid, gip, 0)*/
        if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,                                                        &entry_info))
        {
            nxt_lport = 0;
            while (TRUE == MLDSNP_OM_GetNextPortInfo(vid, gip_ap, sip_ap, &nxt_lport, &port_info))
            {
                if (MLDSNP_TYPE_JOIN_DYNAMIC != port_info.join_type)
                    continue;

                port_info.specific_query_count ++;

                if (port_info.specific_query_count < robust_value)
                {
                    /*set (robust_value- port_info.specific_query_count) times LLQI ,
                      because if it don't receive g-s query,
                      it will still timeout accroding standard.
                     */
                    MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p,
                                                    LLQI * (robust_value - port_info.specific_query_count + 1));
                    MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
                }
                else
                {   /*receive over robustnees g-s query, delete it directly
                     here we shall use MLDSNP_ENGINE_ChangeModeToIncludeMode, because this (vid, gip, 0) have to
                     consider the mode change.
                    */
                    MLDSNP_ENGINE_ChangeModeToIncludeMode(vid, gip_ap, port_info.port_no);
                }
            }
        }

        /*check(vid, gip, *)*/
        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);
        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_src_ip, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (nxt_vid != vid
                    || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            nxt_lport = 0;
            while (TRUE == MLDSNP_OM_GetNextPortInfo(vid, gip_ap, sip_ap, &nxt_lport, &port_info))
            {
                if (MLDSNP_TYPE_JOIN_DYNAMIC != port_info.join_type)
                    continue;

                if(MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type)
                {
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    delete_count++;
                    #endif
                    continue;
                }

                port_info.specific_query_count ++;

                if (port_info.specific_query_count < robust_value)
                {
                    /*set (robust_value- port_info.specific_query_count) times LLQI , because if it don't receive g-s query,
                        it will still timeout accroding standard.
                      */
                    MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, LLQI * (robust_value - port_info.specific_query_count + 1));
                    MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
                }
                else
                {/*receive over robustnees g-s query, delete it directly*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    delete_count++;
#endif
                    MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap, sip_ap, port_info.port_no);
                }
            }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                 && entry_info.register_port_list.nbr_of_element == delete_count)
            {
                MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, sip_ap, &mldsnp_engine_leave_lst );
                delete_count = 0;
            }
#endif
        }
    }
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
    {
        MLDSNP_ENGINE_ProcessSendStoreList(vid);
    }
#endif
    return TRUE;
}/*End of MLDSNP_QUERIER_IncreaseSpecificQueryCount*/



/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_IncreaseVlanAllGrupsSpecificQueryCount
 *-------------------------------------------------------------------------
 * PURPOSE : This function update other querier
 * INPUT   : vid      - the vlan id
 * OUTPUT  : None
 * RETURN  : TRUE   - success
 *           FALSE  - fail
 * NOTE    : we use the specific_query_count to count the received general query
 *               beause g query work as g-s query to coun the group timeout
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_QUERIER_IncreaseVlanAllGrupsSpecificQueryCount(
    UI16_T check_vid)
{
    MLDSNP_OM_PortInfo_T   port_info;
    MLDSNP_OM_HisamEntry_T entry_info;
    UI16_T robust_value = 0, LLQI = 0;
    UI16_T nxt_vid = check_vid, nxt_lport = 0;
    UI8_T  nxt_gip_ap[GROUP_LEN] = {0}, nxt_sip_ap[SRC_IP_LEN] = {0};
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T delete_count = 0;
#endif

    MLDSNP_OM_GetRobustnessOperValue(check_vid, &robust_value);
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    MLDSNP_BD(TRACE, " ");

    /*check all entry
    */
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_ap, nxt_sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (nxt_vid != check_vid)
            break;

        /*check(vid, gip, 0)*/
        if (memcmp(nxt_sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN) == 0)
        {
            nxt_lport = 0;
            while (TRUE == MLDSNP_OM_GetNextPortInfo(nxt_vid, nxt_gip_ap, nxt_sip_ap, &nxt_lport, &port_info))
            {
                if (MLDSNP_TYPE_JOIN_DYNAMIC != port_info.join_type)
                    continue;

                port_info.specific_query_count ++;

                if (port_info.specific_query_count <= robust_value)
                {
#if 0 /*not g-s quey, needn't to update timeout timer*/
                    /*set (robust_value- port_info.specific_query_count) times LLQI ,
                      because if it don't receive g-s query,
                      it will still timeout accroding standard.
                     */
                    MLDSNP_TIMER_UpdateTimerNewTime(port_info.filter_timer_p,
                                                    LLQI * (robust_value - port_info.specific_query_count + 1));
#endif
                    MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
                }
                else
                {   /*receive over robustnees g-s query, delete it directly
                     here we shall use MLDSNP_ENGINE_ChangeModeToIncludeMode, because this (check_vid, gip, 0) have to
                     consider the mode change.
                    */
                    MLDSNP_ENGINE_ChangeModeToIncludeMode(nxt_vid, nxt_gip_ap, port_info.port_no);
                }
            }
        }
        else /*check(vid, gip, *)*/
        {
            nxt_lport = 0;
            while (TRUE == MLDSNP_OM_GetNextPortInfo(nxt_vid, nxt_gip_ap, nxt_sip_ap, &nxt_lport, &port_info))
            {
                if (MLDSNP_TYPE_JOIN_DYNAMIC != port_info.join_type)
                    continue;

                if(MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type)
                {
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    delete_count++;
                    #endif
                    continue;
                }

                port_info.specific_query_count ++;

                if (port_info.specific_query_count <= robust_value)
                {
#if 0 /*not g-s quey, needn't to update timeout timer*/
                    /*set (robust_value- port_info.specific_query_count) times LLQI , because if it don't receive g-s query,
                        it will still timeout accroding standard.
                      */
                    MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, LLQI * (robust_value - port_info.specific_query_count + 1));
#endif
                    MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
                }
                else
                {/*receive over robustnees g-s query, delete it directly*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    delete_count++;
#endif
                    MLDSNP_ENGINE_PortLeaveGroup(check_vid, nxt_gip_ap, nxt_sip_ap, port_info.port_no);
                }
            }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                 && entry_info.register_port_list.nbr_of_element == delete_count)
            {
                MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(nxt_gip_ap, nxt_sip_ap, &mldsnp_engine_leave_lst);
                delete_count = 0;
            }
#endif
        }
    }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
    {
        MLDSNP_ENGINE_ProcessSendStoreList(check_vid);
    }
#endif
    return TRUE;
}/*End of MLDSNP_QUERIER_IncreaseSpecificQueryCount*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_DeleteDynamicRouterPort
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
static BOOL_T MLDSNP_QUERIER_DeleteDynamicRouterPort(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;

    MLDSNP_BD(TRACE, "vid %d, lport %d", vid, lport);

    if (FALSE == MLDSNP_OM_GetRouterPortInfo(vid, lport, &router_port_info))
    {
        MLDSNP_BD(TRACE, "can't get this router port");
        return FALSE;
    }

    /*if the router port is not static*/
    if (MLDSNP_TYPE_JOIN_STATIC == router_port_info.attribute)
    {
        return FALSE;
    }

    /*delete from chip*/
    MLDSNP_QUERIER_DeleteRouterPortFromAllChipEntry(vid, lport);

    router_port_info.port_no = lport;

    if (FALSE == MLDSNP_OM_DeleteRouterPort(vid, &router_port_info))
    {
        MLDSNP_BD(TRACE, " delete router port %d from om fail ", lport);
        return FALSE;
    }

    MLDSNP_TIMER_StopAndFreeTimer(&router_port_info.router_timer_p);

    {/*check this vid can learn group or not, if can't learn group anymore remove all group
        */
        MLDSNP_TYPE_QuerierStatus_T querier_status;
        MLDSNP_OM_VlanInfo_T        vlan_info;

        MLDSNP_OM_GetQuerierStauts(&querier_status);
        if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            MLDSNP_BD(TRACE, " can't get vlan %d info ", vid);
            return FALSE;
        }

        if ((0 == vlan_info.router_port_list.nbr_of_element)
                && (MLDSNP_TYPE_QUERIER_DISABLED == querier_status)
                && (FALSE == MLDSNP_OM_IsMrouteEnabled()))
        {
            MLDSNP_ENGINE_DeleteAllGroupInVlan(vid);
        }
#if 0
        /*when register group from router port, we still take it as normal port register.
           so when router port delete, we just remove router port,
           won't leave all group on this rotuer port*/
        else /*just remove this port registered group*/
            if (FALSE == MLDSNP_ENGINE_PortLeaveVlanAllGroup(vid, lport))
            {
                MLDSNP_BD(TRACE, " prot %d leave all group fail", lport);
                return FALSE;
            }
#endif
    }

    msl_pmgr_mldsnp_rtport_del(0, vid, lport);

    return TRUE;
}/*MLDSNP_QUERIER_DeleteDynamicRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendMrdSocitation
*------------------------------------------------------------------------------
* Purpose: This function send the MRD solicitation message
* INPUT  : specify_vid      - spcify send to this vlan
*          skip_router_port - skip router port
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_QUERIER_SendMrdSocitation(
    UI16_T specify_vid,
    BOOL_T skip_router_port)
{
    L_MM_Mref_Handle_T   *mref_p = NULL;
    MLDSNP_OM_VlanInfo_T vlan_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len  = MLDSNP_TYPE_MRD_SOLICITAION_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    UI16_T next_vid = 0;
    UI8_T *pdu_p    = NULL;
    UI8_T output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, "specify vid %d, skip router port %d", specify_vid, skip_router_port);

    while (TRUE)
    {
        if (0 == specify_vid
                && FALSE == MLDSNP_OM_GetNextVlanInfo(&next_vid, &vlan_info))
            return TRUE;
        else if (0 != specify_vid
                 && FALSE == MLDSNP_OM_GetVlanInfo(specify_vid, &vlan_info))
            return TRUE;


        MLDSNP_BD_ARG(TX, "specify vid %d, vid %d\r\n", specify_vid, next_vid);

        /*when running status is disabled, it mean some querier exist in this vlan and better than ourself
         */
        if (MLDSNP_TYPE_QUERIER_ENABLED == vlan_info.querier_runing_status)
        {
            if (0 == specify_vid)
                continue;
            else
                break;
        }

        /*find the output port to forward the query*/
        if (MLDSNP_ENGINE_FindAllPortsInVlan(vlan_info.vid, output_portlist_a) == FALSE)
        {
            if (0 == specify_vid)
                continue;
            else
                break;
        }

        /*skip transmit to router port*/
        if (skip_router_port)
        {
            MLDSNP_OM_RouterPortInfo_T r_port_info;
            r_port_info.port_no =0;

            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
            {
                MLDSNP_TYPE_DeletePortFromPorBitMap(r_port_info.port_no, output_portlist_a);
            }
        }

        if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                              L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                            MLDSNP_TYPE_TRACE_MRD_SOLICITATION))))
        {
            MLDSNP_BD(TX, "mref is null");
            break;
        }

        mref_p->current_usr_id = SYS_MODULE_MLDSNP;

        if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
        {
            MLDSNP_BD(TX, "pdu_p is null");
            L_MM_Mref_Release(&mref_p);
            break;
        }

        mref_p->current_usr_id = SYS_MODULE_MLDSNP;
        memset(pdu_p, 0, pdu_len);

        if (FALSE  == MLDSNP_QUERIER_ConstructMrdSolicitation(next_vid, pdu_p))
        {
            L_MM_Mref_Release(&mref_p);
            break;
        }


        MLDSNP_BD_ARG(TX, "%s:%d, vid=%d\r\n", __FUNCTION__, __LINE__, vlan_info.vid);
        MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);


        {/*send the query pdu to driver*/
            UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};

            SWCTRL_GetCpuMac(src_mac_a);

            mref_p->next_usr_id = SYS_MODULE_L2MUX;

            if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vlan_info.vid&0x0fff, &vlan_om_info) == FALSE)
           {
               MLDSNP_BD(TX, "Failed to get vlan om info");
               L_MM_Mref_Release(&mref_p);
               return FALSE;
           }


            L2MUX_MGR_SendMultiPacket(mref_p,
                                      general_query_dst_mac_ag,
                                      src_mac_a,
                                      MLDSNP_TYPE_IPV6_ETH_TYPE,
                                      (UI16_T) vlan_info.vid&0x0fff,
                                      pdu_len,
                                      output_portlist_a,
                                      vlan_om_info.dot1q_vlan_current_untagged_ports,
                                      0);
        }

        if (0 != specify_vid)  /* specify vid only process one time*/
            break;
    }
    return TRUE;
}/*ENd of MLDSNP_QUERIER_SendMrdSocitation*/

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
static void MLDSNP_QUERIER_ProxySendOutLeave(UI16_T vid, UI8_T *gip_ap, UI8_T *sip_list_ap, UI16_T num_of_src)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T ver;

    if (MLDSNP_OM_GetProxyReporting(&proxy_status)
        && MLDSNP_TYPE_PROXY_REPORTING_DISABLE == proxy_status)
        return;

    if (num_of_src == 0)
    {
        if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (entry_info.register_port_list.nbr_of_element != 0)
            {
                return; /*still have port in this entry*/
            }
        }

        if (MLDSNP_OM_GetMldSnpVer(&ver)
             && MLDSNP_TYPE_VERSION_1 == ver)
        {
            MLDSNP_BD(TX, "Proxy send out done message");
            MLDSNP_ENGINE_ProxySendV1Done(vid, gip_ap);
        }
        else
        {
            MLDSNP_BD(TX, "Proxy send out to_in{}");
            MLDSNP_ENGINE_ProxySendV2Report(vid, gip_ap, sip_list_ap, num_of_src, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
        }
    }
    else
    {
        UI8_T *save_p = (UI8_T *)src_ip_list_save_aa;
        UI16_T i, num_save = 0;

        if (MLDSNP_OM_GetMldSnpVer(&ver)
            && MLDSNP_TYPE_VERSION_2 != ver)
            return;

        if(MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {/*exclude mode, block exclude source means allow this source, we shall collect all exclude source to upstream,
           Shall inhibit partial exclude list*/
           UI16_T next_vid = vid;
           UI8_T nxt_grp[GROUP_LEN], nxt_src[SRC_IP_LEN];

           IPV6_ADDR_COPY(nxt_grp, gip_ap);
           IPV6_ADDR_SET(nxt_src);

           while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, nxt_grp, nxt_src, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
           {
             if (vid != next_vid
                 || !IPV6_ADDR_SAME(gip_ap, nxt_grp))
               break;

             if(MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
               continue;

              IPV6_ADDR_COPY(save_p + (num_save*SRC_IP_LEN), nxt_src);
              num_save++;
          }

          MLDSNP_BD(TX, "Proxy send out IS_EX{S}");
          MLDSNP_ENGINE_ProxySendV2Report(vid, gip_ap, save_p, num_save, MLDSNP_TYPE_IS_EXCLUDE_MODE);
        }
        else
        { /*include mode*/
          for (i = 0; i < num_of_src; i++)
          {
              if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_list_ap + (i*SRC_IP_LEN), MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
              {
                if(MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                  continue; /*have port in exclude list, we can block this src, but not include list*/
              }

              IPV6_ADDR_COPY(save_p+(i*SRC_IP_LEN), sip_list_ap + (i*SRC_IP_LEN));
              num_save++;
          }

          if (num_save)
          {
            MLDSNP_BD(TX, "Proxy send out block{S}");
            MLDSNP_ENGINE_ProxySendV2Report(vid, gip_ap, save_p, num_save, MLDSNP_TYPE_BLOCK_OLD_SOURCES);
          }
       }
    }

    return;
}
#endif

#if 0
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_AddStaticRouterPort
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
BOOL_T MLDSNP_QUERIER_AddStaticRouterPort(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;

    MLDSNP_BD(TRACE, " ");

    router_port_info.attribute     = MLDSNP_TYPE_JOIN_STATIC;
    router_port_info.register_time = SYS_TIME_GetSystemTicksBy10ms();
    router_port_info.port_no       = lport;
    router_port_info.router_timer_p = NULL;

    if (FALSE == MLDSNP_OM_AddRouterPort(vid, &router_port_info))
    {
        MLDSNP_BD(TRACE, "vid=%d, lport=%d", vid, lport);
        return FALSE;
    }

    /*because router port doesn't not record in om with joined group
       , it only used when add register port to chip, so here call msl directly to add the router port
       Or, we need search all group and call chip to add entry with this router port*/
    MLDSNP_QUERIER_AddRouterPortToAllChipEntry(vid, lport);

    msl_pmgr_mldsnp_rtport_add(0, vid, lport);

    return TRUE;
}/*End of MLDSNP_QUERIER_AddStaticRouterPort*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_QUERIER_DeleteStaticRouterPort
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
BOOL_T MLDSNP_QUERIER_DeleteStaticRouterPort(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_RouterPortInfo_T router_port_info;

    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_OM_GetRouterPortInfo(vid, lport, &router_port_info))
    {
        MLDSNP_BD(TRACE, "can't get this router port");
        return FALSE;
    }

    /*if the router port is not static*/
    if (MLDSNP_TYPE_JOIN_STATIC != router_port_info.attribute)
    {
        return FALSE;
    }

    /*delete from chip*/
    MLDSNP_QUERIER_DeleteRouterPortFromAllChipEntry(vid, lport);

#if 0
    /*when register group from router port, we still take it as normal port register.
           so when router port delete, we just remove router port,
           won't leave all group on this rotuer port*/
    if (FALSE == MLDSNP_ENGINE_PortLeaveVlanAllGroup(vid, lport))
    {
        MLDSNP_BD(TRACE, " ");
        return FALSE;
    }
#endif
    router_port_info.port_no = lport;

    if (FALSE == MLDSNP_OM_DeleteRouterPort(vid, &router_port_info))
    {
        MLDSNP_BD(TRACE, " ");
        return FALSE;
    }

    {/*check this vid can learn group or not, if can't learn group anymore remove all group
        */
        MLDSNP_TYPE_QuerierStatus_T querier_status;
        MLDSNP_OM_VlanInfo_T        vlan_info;

        MLDSNP_OM_GetQuerierStauts(&querier_status);
        if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            MLDSNP_BD(TRACE, " can't get vlan %d info ", vid);
            return FALSE;
        }

        if ((0 == vlan_info.router_port_list.nbr_of_element)
                && (MLDSNP_TYPE_QUERIER_DISABLED == querier_status)
                && (FALSE == MLDSNP_OM_IsMrouteEnabled()))
        {
            MLDSNP_ENGINE_DeleteAllGroupInVlan(vid);
        }
    }

    msl_pmgr_mldsnp_rtport_del(0, vid, lport);

    return TRUE;
}/*MLDSNP_QUERIER_DeleteStaticRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendGeneralQeury
*------------------------------------------------------------------------------
* Purpose: This function send the general query to all querying running status enabled vlan
* INPUT  : specify_vid - spcify send to this vlan
*          skip_router_port - skip router port
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendGeneralQeury(
    UI16_T specify_vid,
    BOOL_T skip_router_port)
{
    L_MM_Mref_Handle_T   *mref_p = NULL;
    MLDSNP_OM_VlanInfo_T vlan_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len = 0;
    UI16_T next_vid = 0, snp_ver = 0;
    UI8_T *pdu_p = NULL;
    UI8_T output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]    = {0};
    UI8_T v1_present_bitmap_ap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, "specify vid %d ", specify_vid);

    MLDSNP_OM_GetMldSnpVer(&snp_ver);

    if (MLDSNP_TYPE_VERSION_1 == snp_ver)
    {
        pdu_len = MLDSNP_TYPE_QUERY_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    }
    else
    {
        pdu_len = MLDSNP_TYPE_QUERY_V2_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
        MLDSNP_OM_GetV1HostPresentPortbitmap(v1_present_bitmap_ap);
    }

    while (TRUE)
    {
        if (0 == specify_vid
                && FALSE == MLDSNP_OM_GetNextVlanInfo(&next_vid, &vlan_info))
            return;
        else if (0 != specify_vid
                 && FALSE == MLDSNP_OM_GetVlanInfo(specify_vid, &vlan_info))
            return;

        MLDSNP_BD_ARG(TX, "specify vid %d, vid %d\r\n", specify_vid, next_vid);

        /*when running status is disabled, it mean some querier exist in this vlan and better than ourself
         */
        if (MLDSNP_TYPE_QUERIER_DISABLED == vlan_info.querier_runing_status)
        {
            MLDSNP_BD_ARG(TX, "vlan=%d is querier disable\r\n", vlan_info.vid);
            if (0 == specify_vid)
                continue;
            else
                break;
        }

        /*find the output port to forward the query*/
        if (MLDSNP_ENGINE_FindAllPortsInVlan(vlan_info.vid, output_portlist_a) == FALSE)
        {
            if (0 == specify_vid)
                continue;
            else
                break;
        }

        /*skip transmit to router port*/
        if (skip_router_port)
        {
            MLDSNP_OM_RouterPortInfo_T r_port_info;
            r_port_info.port_no =0;

            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
            {
                MLDSNP_TYPE_DeletePortFromPorBitMap(r_port_info.port_no, output_portlist_a);
            }
        }

        if (MLDSNP_TYPE_VERSION_2 == snp_ver)
        {
            UI16_T i = 0, j = 0;

            for (;i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
            {
                /*clear v1 host present port from output port*/
                output_portlist_a[i] &= ~v1_present_bitmap_ap[i];
                /*send v1 query to this port*/
                if (v1_present_bitmap_ap[i] != 0)
                {
                    for (j = 1; j <= 8; j++)
                    {
                        if (v1_present_bitmap_ap[i]&(0x80 >> (j - 1) % 8))
                        {
                            MLDSNP_QUERIER_SendGeneralQeuryToSpecifyPort(vlan_info.vid, i*8 + j, MLDSNP_TYPE_VERSION_1);
                            MLDSNP_OM_PortIncStat(i*8 + j, 0, MLDSNP_TYPE_STAT_GQ_SEND, TRUE);
                        }
                    }
                }
            }
        }

        if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                              L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                            MLDSNP_TYPE_TRACE_SEND_GENERAL_QUERY))))
        {
            MLDSNP_BD(TX, "mref is null");

            if (0 == specify_vid)
                continue;
            else
                break;
        }

        mref_p->current_usr_id = SYS_MODULE_MLDSNP;

        if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
        {
            MLDSNP_BD(TX, "pdu_p is null");
            L_MM_Mref_Release(&mref_p);
            if (0 == specify_vid)
                continue;
            else
                break;
        }

        memset(pdu_p, 0, pdu_len);

        /*construct the query packet content*/
        if (FALSE == MLDSNP_QUERIER_ConstructGeneralQuery(pdu_p, vlan_info.vid, snp_ver))
        {
            L_MM_Mref_Release(&mref_p);

            if (0 == specify_vid)
                continue;
            else
                break;
        }

        MLDSNP_BD_ARG(TX, "vid=%d, dmac=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                      vlan_info.vid, general_query_dst_mac_ag[0], general_query_dst_mac_ag[1], general_query_dst_mac_ag[2],
                      general_query_dst_mac_ag[3], general_query_dst_mac_ag[4], general_query_dst_mac_ag[5]);
        MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);

        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_GQ_SEND, output_portlist_a);

        MLDSNP_OM_VlanIncStat(vlan_info.vid&0x0fff, MLDSNP_TYPE_STAT_GQ_SEND, TRUE);

        {/*send the query pdu to driver*/
            UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};

            SWCTRL_GetCpuMac(src_mac_a);

            mref_p->next_usr_id = SYS_MODULE_L2MUX;

            if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vlan_info.vid&0x0fff, &vlan_om_info) == FALSE)
            {
                MLDSNP_BD(TX, "Failed to get vlan om info");
                L_MM_Mref_Release(&mref_p);
                break;
            }

            L2MUX_MGR_SendMultiPacket(mref_p,
                                      general_query_dst_mac_ag,
                                      src_mac_a,
                                      MLDSNP_TYPE_IPV6_ETH_TYPE,
                                      (UI16_T) vlan_info.vid&0x0fff,
                                      pdu_len,
                                      output_portlist_a,
                                      vlan_om_info.dot1q_vlan_current_untagged_ports,
                                      0);
        }

        if (0 != specify_vid)  /* specify vid only process one time*/
            break;
    }/*end while(MLDSNP_OM_GetNextVlanInfo(&next_vid, &vlan_info))*/

    return;
}/*End of MLDSNP_QUERIER_SendGeneralQeury*/

#if 0
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendGeneralQeuryToSpcificVlan
*------------------------------------------------------------------------------
* Purpose: This function send the general query to specific vlan id
* INPUT  : vid - the vlan id
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendGeneralQeuryToSpcificVlan(
    UI16_T vid)
{
    L_MM_Mref_Handle_T *mref_p = NULL;
    UI32_T pdu_len = 0;
    UI16_T snp_ver = MLDSNP_TYPE_VERSION_1;
    UI8_T  *pdu_p  = NULL;
    UI8_T  output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  v1_present_bitmap_ap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, " ");

    if (FALSE  == MLDSNP_QUERIER_IsQuerierRunning(vid))
    {
        MLDSNP_BD_ARG(TX, "vlan=%d is querier disable\r\n",  vid);
        return; /*the querier running status is disable, means this switch is not querier*/
    }

    MLDSNP_OM_GetMldSnpVer(&snp_ver);

    if (MLDSNP_TYPE_VERSION_1 == snp_ver)
    {
        pdu_len = MLDSNP_TYPE_QUERY_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    }
    else
    {
        pdu_len = MLDSNP_TYPE_QUERY_V2_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
        MLDSNP_OM_GetV1HostPresentPortbitmap(v1_present_bitmap_ap);
    }

    /*find the output port to forward the query*/
    if (MLDSNP_ENGINE_FindAllPortsInVlan(vid, output_portlist_a) == FALSE)
    {
        return;
    }

    if (MLDSNP_TYPE_VERSION_2 == snp_ver)
    {
        UI16_T i = 0, j = 0;

        for (; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
        {
            /*clear v1 host present port from output port*/
            output_portlist_a[i] &= ~v1_present_bitmap_ap[i];
            /*send v1 query to this port*/
            if (v1_present_bitmap_ap[i] != 0)
            {
                for (j = 0; j < 8;j++)
                {
                    if (v1_present_bitmap_ap[i]&(0x80 >> (j - 1) % 8))
                        MLDSNP_QUERIER_SendGeneralQeuryToSpecifyPort(vid, (i + 1)*8 + j + 1, MLDSNP_TYPE_VERSION_1);
                }
            }
        }
    }

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_GROUP_SPECIFIC_QUERY))))
    {
        MLDSNP_BD(TX, "mref is null");
        return;
    }

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        MLDSNP_BD(TX, "pdu_p is null");
        L_MM_Mref_Release(&mref_p);
        return;
    }

    memset(pdu_p, 0, pdu_len);

    /*construct the query packet content*/
    if (FALSE == MLDSNP_QUERIER_ConstructGeneralQuery(pdu_p, vid, snp_ver))
    {
        L_MM_Mref_Release(&mref_p);
        return;
    }


    MLDSNP_BD_ARG(TX, "vid=%d, dmac=%02x.%02x.%02x.%02x.%02x.%02x\r\n",
                  vid, general_query_dst_mac_ag[0], general_query_dst_mac_ag[1], general_query_dst_mac_ag[2],
                  general_query_dst_mac_ag[3], general_query_dst_mac_ag[4], general_query_dst_mac_ag[5]);
    MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);

    {/*send the query pdu to driver*/
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};

        SWCTRL_GetCpuMac(src_mac_a);

        mref_p->next_usr_id = SYS_MODULE_L2MUX;

        L2MUX_MGR_SendMultiPacket(mref_p,
                                  general_query_dst_mac_ag,
                                  src_mac_a,
                                  MLDSNP_TYPE_IPV6_ETH_TYPE,
                                  (UI16_T) vid&0x0fff,
                                  pdu_len,
                                  output_portlist_a,
                                  output_portlist_a,
                                  0);
    }

    return;
}/*End of MLDSNP_QUERIER_SendGeneralQeuryToSpcificVlan*/
#endif
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SendSpecificQuery
*------------------------------------------------------------------------------
* Purpose: This function send the group spcific qeury
* INPUT  : vid    - the vlan id
*          gip_ap - the group ip address
*          lport  - the port to send the g-s query
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SendSpecificQuery(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T lport,
    UI8_T  *src_ip_list_ap,
    UI16_T num_of_src)
{
    MLDSNP_TYPE_QuerierStatus_T  querier_running_status = MLDSNP_TYPE_QUERIER_DISABLED;
    L_MM_Mref_Handle_T *mref_p   = NULL;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len = 0;
    UI16_T snp_ver = 0;
    UI8_T *pdu_p = NULL, tmp_pdu[SYS_ADPT_MAX_FRAME_SIZE] = {0};
    UI8_T output_port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T v1_present_bitmap_ap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d", gip_ap, src_ip_list_ap, num_of_src, vid, lport);

    /*if > allow process src list, we don't process it, acutally it shall not happen*/
    if (num_of_src > MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
        return;

    MLDSNP_OM_GetQuerierRunningStatus(vid, &querier_running_status);
    if (MLDSNP_TYPE_QUERIER_DISABLED == querier_running_status)
        return; /*the querier running status is disable, means this switch is not querier*/

    MLDSNP_OM_GetMldSnpVer(&snp_ver);

    if (MLDSNP_TYPE_VERSION_1 == snp_ver)
    {
        pdu_len = MLDSNP_TYPE_QUERY_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    }
    else
    {
        pdu_len = MLDSNP_TYPE_QUERY_V2_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    }

    /*configure v1*/
    if (MLDSNP_TYPE_VERSION_1 == snp_ver)
    {
        if (FALSE == MLDSNP_QUERIER_ConstructGroupSpecifyQuery(tmp_pdu, vid, gip_ap, MLDSNP_TYPE_VERSION_1))
            return;
    }
    /*configure v2*/
    else
    {
        if (0 == num_of_src)
        {
            MLDSNP_OM_GetV1HostPresentPortbitmap(v1_present_bitmap_ap);

            if (MLDSNP_TYPE_IsPortInPortBitMap(lport, v1_present_bitmap_ap))
            {
                if (FALSE == MLDSNP_QUERIER_ConstructGroupSpecifyQuery(tmp_pdu, vid, gip_ap, MLDSNP_TYPE_VERSION_1))
                    return;
            }
            else
            {
                if (FALSE == MLDSNP_QUERIER_ConstructGroupSpecifyQuery(tmp_pdu, vid, gip_ap, MLDSNP_TYPE_VERSION_2))
                    return;
            }
        }
        else
        {
            pdu_len += num_of_src * SRC_IP_LEN;
            if (FALSE == MLDSNP_QUERIER_ConstructGroupSourceSpecifyQuery(tmp_pdu, vid, gip_ap, src_ip_list_ap, num_of_src,
                                                                         MLDSNP_TYPE_QUERY_V2_LEN+ num_of_src *SRC_IP_LEN))
                return;
        }
    }
    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_GROUP_SPECIFIC_QUERY))))
    {
        MLDSNP_BD(TX, "mref is null");
        return;
    }

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        MLDSNP_BD(TX, "pdu_p is null");
        L_MM_Mref_Release(&mref_p);
        return;
    }

    memcpy(pdu_p, tmp_pdu, pdu_len);

    /*assign the send out port*/
    MLDSNP_TYPE_AddPortIntoPortBitMap(lport, output_port_bitmap_a);
    MLDSNP_OM_PortIncStat(lport, vid, MLDSNP_TYPE_STAT_SQ_SEND, TRUE);
    {
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0}, dst_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};
        /*send the query pdu to driver*/
        SWCTRL_GetCpuMac(src_mac_a);

        /*construct the query packet content*/
        MLDSNP_TYPE_ipv6ToMac(gip_ap, dst_mac_a);

        MLDSNP_BD_ARG(TX, "%s:%d, vid=%d, dmac=%02x.%02x.%02x.%02x.%02x.%02x\r\n",
                      __FUNCTION__, __LINE__, vid, dst_mac_a[0], dst_mac_a[1], dst_mac_a[2], dst_mac_a[3], dst_mac_a[4], dst_mac_a[5]);
        MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TX, pdu_p, pdu_len);

        if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid&0x0fff, &vlan_om_info) == FALSE)
        {
            MLDSNP_BD(TX, "Failed to get vlan om info");
            L_MM_Mref_Release(&mref_p);
            return;
        }

        L2MUX_MGR_SendMultiPacket(mref_p,
                                  dst_mac_a,
                                  src_mac_a,
                                  MLDSNP_TYPE_IPV6_ETH_TYPE,
                                  (UI16_T) vid&0x0fff,
                                  pdu_len,
                                  output_port_bitmap_a,
                                  vlan_om_info.dot1q_vlan_current_untagged_ports,
                                  0);
    }
    return;
}/*End of MLDSNP_QUERIER_SendSpecificQuery*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_EnableQuerier
*------------------------------------------------------------------------------
* Purpose: This function enable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_EnableQuerier()
{
    MLDSNP_Timer_T *timer_p = NULL;
    UI16_T   interval = 0;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetStartupQueryInterval(&interval);

    /*add timer to timer list.*/
    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_QuerierStartTimeout,
            0,
            mldsnp_om_null_group_ip_a,
            mldsnp_om_null_src_ip_a,
            0,
            0,
            interval,
            MLDSNP_TIMER_ONE_TIME,
            &timer_p))
    {
        return FALSE;
    }

    MLDSNP_OM_StoreQuerierTimer(timer_p);

    MLDSNP_OM_SetQuerierStatus(MLDSNP_TYPE_QUERIER_ENABLED);

    if (!MLDSNP_OM_IsMrouteEnabled())
    {
        MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_ENABLED);

        MLDSNP_OM_SetStartupQuerySentCount(1);

        {
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            MLDSNP_TYPE_ProxyReporting_T proxy_staus;

            if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
                 && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
                MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
            else
#endif
                MLDSNP_QUERIER_SendGeneralQeury(0, FALSE);
        }
    }
    return TRUE;
}/*End of MLDSNP_QUERIER_EnableQuerier*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_DisableQuerier
*------------------------------------------------------------------------------
* Purpose: This function disable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_DisableQuerier()
{
    MLDSNP_Timer_T *timer_p = NULL;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_SetQuerierStatus(MLDSNP_TYPE_QUERIER_DISABLED);

    MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_DISABLED);

    MLDSNP_OM_GetQuerierTimer(&timer_p);

    MLDSNP_TIMER_StopAndFreeTimer(&timer_p);

    MLDSNP_OM_StoreQuerierTimer(NULL);

    return TRUE;
}/*End of MLDSNP_QUERIER_EnableQuerier*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_StartQuerier
*------------------------------------------------------------------------------
* Purpose: This function start the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_StartQuerier()
{
    MLDSNP_Timer_T *timer_p = NULL;
    UI16_T   interval = 0;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetStartupQueryInterval(&interval);

    /*add timer to timer list.*/
    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_QuerierStartTimeout,
            0,
            mldsnp_om_null_group_ip_a,
            mldsnp_om_null_src_ip_a,
            0,
            0,
            interval,
            MLDSNP_TIMER_ONE_TIME,
            &timer_p))
    {
        return FALSE;
    }

    MLDSNP_OM_StoreQuerierTimer(timer_p);

    if (!MLDSNP_OM_IsMrouteEnabled())
    {
        MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_ENABLED);

        MLDSNP_OM_SetStartupQuerySentCount(1);

        {
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            MLDSNP_TYPE_ProxyReporting_T proxy_staus;

            if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
                    && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
                MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
            else
#endif
                MLDSNP_QUERIER_SendGeneralQeury(0, FALSE);
        }
    }
    return TRUE;
}/*End of MLDSNP_QUERIER_EnableQuerier*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_StopQuerier
*------------------------------------------------------------------------------
* Purpose: This function stop the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_StopQuerier()
{
    MLDSNP_Timer_T *timer_p = NULL;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_DISABLED);

    MLDSNP_OM_GetQuerierTimer(&timer_p);

    MLDSNP_TIMER_StopAndFreeTimer(&timer_p);

    MLDSNP_OM_StoreQuerierTimer(NULL);

    return TRUE;
}/*End of MLDSNP_QUERIER_EnableQuerier*/
/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_EnableMrdSolicitation
*------------------------------------------------------------------------------
* Purpose: This function enable the querier
* INPUT  :
* OUTPUT :
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_SetMrdSolicitationStatus(BOOL_T enabled)
{
    MLDSNP_Timer_T *timer_p = NULL;

    MLDSNP_BD(TRACE, "status %d", enabled);

    if (TRUE == enabled)
    {
        /*add timer to timer list.*/
        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_MrdSolicitationTimeout,
                0,
                mldsnp_om_null_group_ip_a,
                mldsnp_om_null_src_ip_a,
                0,
                0,
                MLDSNP_TYPE_DFLT_SOLICITATION_INTERVAL, /*not follow rfc*/
                MLDSNP_TIMER_CYCLIC,
                &timer_p))
        {
            return FALSE;
        }

        MLDSNP_OM_StoreMrdSolicitationTimer(timer_p);

        /*send first mrd solicitation*/
        MLDSNP_QUERIER_SendMrdSocitation(0, FALSE);
    }
    else
    {
        MLDSNP_OM_GetMrdSolicitationTimer(&timer_p);

        MLDSNP_TIMER_StopAndFreeTimer(&timer_p);
        MLDSNP_OM_StoreMrdSolicitationTimer(NULL);
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_EnableQuerier*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessQuery
*------------------------------------------------------------------------------
* Purpose: This function change the query inteval
* INPUT  : new_interval  - new query interval
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUEIRER_ChangeQueryInterval(
    UI32_T new_interval)
{
    MLDSNP_Timer_T *querier_timer_p = NULL;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetQuerierTimer(&querier_timer_p);

    /*if in MLDSNP_QUERIER_QuerierStartTimeout, it mean querier still in start, so don't update the query interval*/
    if (NULL != querier_timer_p
            && querier_timer_p->func_p != MLDSNP_QUERIER_QuerierStartTimeout)
    {
        if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(querier_timer_p, new_interval))
        {
            MLDSNP_BD(TRACE, "set query interval fail");
            return FALSE;
        }
    }

    MLDSNP_OM_SetQueryInterval(new_interval);

    return TRUE;
}/*MLDSNP_QUEIRER_ChangeQueryInterval*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessQuery
*------------------------------------------------------------------------------
* Purpose: This function process the query
* INPUT  : *msg_p   - the message pointer
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ProcessQuery(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_OM_VlanInfo_T        vlan_info;
    /*MLDSNP_TYPE_QuerierStatus_T querier_status         = MLDSNP_TYPE_QUERIER_DISABLED;*/
    MLDSNP_ENGINE_Query_T       *query_p               = (MLDSNP_ENGINE_Query_T *)msg_p->icmpv6_p;
    MLDSNP_TYPE_QuerierStatus_T querier_runninn_status = MLDSNP_TYPE_QUERIER_DISABLED;

    MLDSNP_BD(TRACE, " ");

    /*register router port*/
    if (FALSE == MLDSNP_QUERIER_AddDynamicRouterPort(msg_p->vid, msg_p->recevied_port))
    {
        return FALSE;
    }

    /*check the null src ip*/
    if (!memcmp(msg_p->ipv6_header_p->sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
    {
        MLDSNP_BD(RX, "Recevied null src ip query, won't join querier election and update received query count");
        return TRUE;
    }

    /*record other querier's source ip address
     */
    if (TRUE == MLDSNP_OM_GetVlanInfo(msg_p->vid, &vlan_info))
    {
        /*use querier_src_ip to store other querier's address*/
        if(IPV6_ADDR_SAME(vlan_info.other_querier_src_ip, mldsnp_om_null_src_ip_a)
           ||
           (IPV6_ADDR_CMP(msg_p->ipv6_header_p->sip_a, vlan_info.other_querier_src_ip) < 0)
          )
        {
            IPV6_ADDR_COPY(vlan_info.other_querier_src_ip, msg_p->ipv6_header_p->sip_a);
            MLDSNP_OM_SetVlanInfo(&vlan_info);
        }
    }

    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {/*we allow add mrouter port, but not allow do other querier part*/
      MLDSNP_TYPE_ProxyReporting_T proxy_status;
      MLDSNP_OM_GetProxyReporting(&proxy_status);

      if(MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
        return TRUE;
    }
    #endif

    /*if admin querier is not enabled, just return*/
    /* if (MLDSNP_OM_GetQuerierStauts(&querier_status) we always record other querier
        && MLDSNP_TYPE_QUERIER_ENABLED == querier_status)*/
    {
        if (FALSE == MLDSNP_QUERIER_UpdateOtherQuereirPresentTimer(msg_p, &querier_runninn_status))
        {
            MLDSNP_BD(RX, "update other querier timer  fail");
            return FALSE;
        }
    }

    if (MLDSNP_TYPE_QUERIER_DISABLED == querier_runninn_status)
    {
        if (L_STDLIB_Ntoh16(msg_p->icmp_len) == MLDSNP_TYPE_QUERY_V1_LEN)
        {/*v1*/
            if (0 != memcmp(query_p->gip_a, mldsnp_om_null_src_ip_a, GROUP_LEN))
            {/*g-s query*/
                MLDSNP_QUERIER_IncreaseSpecificQueryCount(msg_p->vid, query_p->gip_a, mldsnp_om_null_src_ip_a, 0);
            }
            else
            {
                MLDSNP_QUERIER_IncreaseVlanAllGrupsSpecificQueryCount(msg_p->vid);
            }
        }
        else
        {/*v2*/
            /*set/clear S flag*/
            MLDSNP_OM_SetS_QRV_QQIC(msg_p->vid, (query_p->s ? TRUE : FALSE), query_p->qrv, query_p->qqic);

            if (0 != memcmp(query_p->gip_a, mldsnp_om_null_src_ip_a, GROUP_LEN))
            { /*g-s query*/
                if (0 == query_p->num_of_src)
                {/*g-squery*/
                    MLDSNP_QUERIER_IncreaseSpecificQueryCount(msg_p->vid, query_p->gip_a, mldsnp_om_null_src_ip_a, 0);
                }
                else
                { /*g-s-s query*/
                    MLDSNP_QUERIER_IncreaseSpecificQueryCount(msg_p->vid, query_p->gip_a, (UI8_T *)query_p->sip_aa, query_p->num_of_src);
                }
            }
            else
            {
                MLDSNP_QUERIER_IncreaseVlanAllGrupsSpecificQueryCount(msg_p->vid);
            }
        }
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_ProcessQuery*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_IsQuerierRunning
*------------------------------------------------------------------------------
* Purpose: Is querier running?
* INPUT  :vid - the vlan id to check querier running status
* OUTPUT : TRUE - running
*          FALSE- not running
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_IsQuerierRunning(
    UI16_T vid)
{
    MLDSNP_TYPE_QuerierStatus_T q_run_status = MLDSNP_TYPE_QUERIER_DISABLED;

    MLDSNP_OM_GetQuerierRunningStatus(vid, &q_run_status);

    if (MLDSNP_TYPE_QUERIER_DISABLED == q_run_status)
        return FALSE;

    return TRUE;
}/*End of MLDSNP_QUERIER_IsQuerierRunning*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_GenerateCheckSum
*------------------------------------------------------------------------------
* Purpose: This function genereate the checksum
* INPUT  :*icmp_p     - the icmp start address
*         *sip_ap     - the source ip first four octect
*         *dip_ap     - the dest ip first fourt octect
*         icmp_length - the icmp pdu length
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
UI16_T  MLDSNP_QUERIER_GenerateCheckSum(
    UI8_T  *icmp_p,
    UI8_T  *sip_ap,
    UI8_T  *dip_ap,
    UI8_T  next_header,
    UI32_T icmp_length)
{
#define PSEUDO_LEN 40
    MLDSNP_ENGINE_Icmpv6Checksum_T *icmpv6_checksum_p;
    UI16_T ret = 0xffff;
    UI8_T *buffer_p;

    if (NULL == (buffer_p = (UI8_T *) malloc(icmp_length + PSEUDO_LEN)))
        return 0xffff;

    icmpv6_checksum_p = (MLDSNP_ENGINE_Icmpv6Checksum_T *)buffer_p;

    memset(icmpv6_checksum_p, 0, icmp_length + PSEUDO_LEN);
    memcpy(icmpv6_checksum_p->dst_ip, dip_ap, GROUP_LEN);
    memcpy(icmpv6_checksum_p->src_ip, sip_ap, SRC_IP_LEN);
    icmpv6_checksum_p->next_header = next_header;
    icmpv6_checksum_p->packet_leng = L_STDLIB_Hton32(icmp_length);

    memcpy(&icmpv6_checksum_p->icmpv6_pdu, icmp_p, icmp_length);

    ret = L_MATH_CheckSum16((UI16_T *)icmpv6_checksum_p, (icmp_length + PSEUDO_LEN));
    free(buffer_p);
    return ret;
}/*End of MLDSNP_QUERIER_GenerateCheckSum*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_GeneralQueryTimeOut
*------------------------------------------------------------------------------
* Purpose: This function process the quereir time out to send the query to all port in all vlan
* INPUT  : *timer_para_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_GeneralQueryTimeOut(
    void *parameter_p)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T  proxy_staus = MLDSNP_TYPE_PROXY_REPORTING_DISABLE;
#endif

    MLDSNP_BD(TRACE, " ");

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
        && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
        MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
    else
#endif
        MLDSNP_QUERIER_SendGeneralQeury(0, FALSE);

    return TRUE;
}/*End of MLDSNP_QUERIER_GeneralQueryTimeOut*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SpecificQueryTimeOut
*------------------------------------------------------------------------------
* Purpose: This function process the specific quereir time out to send the specific group query to
*          the want to leave port
* INPUT  : *timer_para_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_SpecificQueryTimeOut(
    void *para_p)
{
    MLDSNP_TIMER_TimerPara_T *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)para_p;
    MLDSNP_OM_PortInfo_T     port_info;
    UI16_T robustness = 0, idx = 0, num_src_ip = 0;
    UI8_T *sip_list_aap = (UI8_T *)src_ip_list_save_aa, *sip_tmp_list_aap = timer_para_p->sip_list_a;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI8_T *sip_block_save = (UI8_T *)src_ip_block_list_save_aa;
    UI16_T saved_block = 0;
#endif

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, lport =%d", timer_para_p->gip_a, timer_para_p->sip_list_a,
                             timer_para_p->num_of_src, timer_para_p->vid, timer_para_p->lport);

    if (FALSE == MLDSNP_QUERIER_IsQuerierRunning(timer_para_p->vid))
    {
        MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/
        return TRUE; /*the querier running status is disable, means this switch is not querier*/
    }
    memset(sip_list_aap, 0, MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC*SRC_IP_LEN);

    MLDSNP_OM_GetRobustnessOperValue(timer_para_p->vid, &robustness);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
    memset(src_ip_block_list_save_aa, 0, sizeof(src_ip_block_list_save_aa));
#endif

    for (idx = 0; idx < timer_para_p->num_of_src; idx++, sip_tmp_list_aap += SRC_IP_LEN)
    {
        MLDSNP_BD_SHOW_SRC(TRACE, "process", sip_tmp_list_aap, 1);

        if (FALSE == MLDSNP_OM_GetPortInfo(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, timer_para_p->lport, &port_info))
        {
            MLDSNP_BD(TRACE, "get port info fail");
            continue;
        }

        /*if this (vid,gip, sip) has been update after timer create*/
        if (port_info.last_report_time > timer_para_p->create_time)
        {
            MLDSNP_BD(TRACE, "port_info.last_report_time > timer_para_p->create_time");
            continue;
        }

        port_info.specific_query_count ++;

        if (port_info.specific_query_count > robustness)
        {
            MLDSNP_BD(TRACE, "port_info.specific_query_count > robustness");
            #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            {
                IPV6_ADDR_COPY(sip_block_save + (saved_block*SRC_IP_LEN), sip_tmp_list_aap);
                saved_block++;
            }
            #endif

            if (FALSE == MLDSNP_ENGINE_IsExcludeMode(timer_para_p->vid, timer_para_p->gip_a, timer_para_p->lport))
            {
                if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, timer_para_p->lport))
                {
                    MLDSNP_BD(TRACE, "port leave group fail");
                }
            }
            else
            {
                MLDSNP_BD(TRACE, "move to exclue list");

                port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                if (FALSE == MLDSNP_OM_UpdatePortInfo(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, &port_info))
                {
                    MLDSNP_BD(TRACE, "update port info fail");
                }

                /*Delete port from chip
                 */
                MLDSNP_QUERIER_DeletePortFromChipEntry(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, timer_para_p->lport);
                #if 0
                /*we shall write (v,g,s) or the data will be forwarded accroding to (v,g,0).*/
                {
                    MLDSNP_OM_VlanInfo_T vlan_info;
                    MLDSNP_OM_RouterPortInfo_T router_port_info;
                    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

                    if (TRUE == MLDSNP_OM_GetVlanInfo(timer_para_p->vid, &vlan_info))
                    {
                        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
                        {
                            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, port_bitmap_a);
                        }
                    }

                    MLDSNP_QUERIER_AddPortBitmapToChipEntry(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, port_bitmap_a);
                }
                #endif
            }
        }
        else
        {
            memcpy(sip_list_aap + num_src_ip*SRC_IP_LEN, sip_tmp_list_aap, SRC_IP_LEN);
            num_src_ip ++;

            if (FALSE == MLDSNP_OM_UpdatePortInfo(timer_para_p->vid, timer_para_p->gip_a, sip_tmp_list_aap, &port_info))
            {
                MLDSNP_BD(TRACE, "update port info fail");
            }
        }
    }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (saved_block)
    {
        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
            MLDSNP_QUERIER_ProxySendOutLeave(timer_para_p->vid, timer_para_p->gip_a, sip_block_save, saved_block);
    }
#endif

    /*if there is no source need to send, after process*/
    if (timer_para_p->num_of_src > 0
        && 0 == num_src_ip)
    {
        MLDSNP_BD(TRACE, "needn't to send g-s-s query, because all src has been updated or deleted");
        MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/
        return TRUE;
    }
    else if (0 == timer_para_p->num_of_src)
    {/*Q(MA)*/
        MLDSNP_BD(TRACE, "process Q(MA)");

        if (FALSE == MLDSNP_OM_GetPortInfo(timer_para_p->vid, timer_para_p->gip_a, mldsnp_om_null_src_ip_a, timer_para_p->lport, &port_info))
        {
            MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/
            return TRUE;
        }

        /*if this (vid,gip, sip) has been update after timer create*/
        if (port_info.last_report_time >= timer_para_p->create_time)
        {
            MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/
            return TRUE;
        }

        port_info.specific_query_count ++;

        if (port_info.specific_query_count > robustness)
        {
            MLDSNP_BD(TRACE, "port_info.specific_query_count > robustness");
            MLDSNP_ENGINE_ChangeModeToIncludeMode(timer_para_p->vid, timer_para_p->gip_a, timer_para_p->lport);
            MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/

            #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
                MLDSNP_QUERIER_ProxySendOutLeave(timer_para_p->vid, timer_para_p->gip_a, NULL, 0);
            #endif
            return TRUE;
        }

        if (FALSE == MLDSNP_OM_UpdatePortInfo(timer_para_p->vid, timer_para_p->gip_a, mldsnp_om_null_src_ip_a, &port_info))
        {
            MLDSNP_BD(TRACE, "update port info fail");
        }
    }


    MLDSNP_QUERIER_SendSpecificQuery(timer_para_p->vid, timer_para_p->gip_a, timer_para_p->lport,  sip_list_aap, num_src_ip);

    {/*start the Q(MA) or Q(MA, A) again*/
        MLDSNP_Timer_T *timer_p = NULL;
        UI16_T LLQT = 0;

        MLDSNP_OM_GetLastListenerQueryInterval(&LLQT);

        MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                                         timer_para_p->vid,
                                         timer_para_p->gip_a,
                                         (UI8_T *)sip_list_aap,
                                         num_src_ip,
                                         timer_para_p->lport,
                                         LLQT,
                                         MLDSNP_TIMER_ONE_TIME,
                                         &timer_p);

    }

    MLDSNP_TIMER_FreeTimer(&timer_para_p->self_p);  /*free self timer*/
    return TRUE;
}/*End of MLDSNP_QUERIER_SpecificQueryTimeOut*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_OtherQuerierPrentTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the other querier timer timeout
* INPUT  : *parameter_p  - the parameter pointer
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_OtherQuerierPrentTimeout(
    void *parameter_p)
{
    MLDSNP_TYPE_QuerierStatus_T querier_status;
    MLDSNP_TIMER_TimerPara_T *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)parameter_p;
    MLDSNP_OM_VlanInfo_T     vlan_info;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_staus;
#endif
    UI16_T tmp_vid = timer_para_p->vid;

    MLDSNP_BD_SHOW_GROUP(TRACE,  "vid=%d, lport =%d", timer_para_p->gip_a, timer_para_p->vid, timer_para_p->lport);

    if (FALSE == MLDSNP_OM_GetVlanInfo(timer_para_p->vid, &vlan_info))
    {
        return FALSE;
    }

    MLDSNP_TIMER_FreeTimer(&vlan_info.other_querier_present_timer_p);

    if (MLDSNP_OM_GetQuerierStauts(&querier_status)
        && querier_status == MLDSNP_TYPE_QUERIER_ENABLED)
    {
        vlan_info.querier_runing_status = MLDSNP_TYPE_QUERIER_ENABLED;
    }
    vlan_info.other_querier_uptime  = 0;

    /* if original self querier is enabled, restart the uptime */
    if (MLDSNP_OM_GetQuerierStauts(&querier_status)
        && querier_status == MLDSNP_TYPE_QUERIER_ENABLED)
    {
        vlan_info.querier_uptime = SYSFUN_GetSysTick();
    }

    vlan_info.old_ver_querier_exist = FALSE;
    memset(vlan_info.old_ver_querier_src_ip, 0, SRC_IP_LEN);
    memset(vlan_info.other_querier_src_ip, 0, SRC_IP_LEN);

    MLDSNP_OM_GetQueryInterval(&vlan_info.query_oper_interval);
    MLDSNP_OM_GetRobustnessValue(&vlan_info.robust_oper_value);

    if (FALSE == MLDSNP_OM_SetVlanInfo(&vlan_info))
    {
        return FALSE;
    }

    if (MLDSNP_OM_GetQuerierStauts(&querier_status)
        && querier_status == MLDSNP_TYPE_QUERIER_ENABLED)/*zhimin add 20160530*/
    {
    /*send general query to this vlan*/
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
            && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
        {
            MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
        }
    else
#endif
        MLDSNP_QUERIER_SendGeneralQeury(tmp_vid, FALSE);
    }
    return TRUE;
}/*End of MLDSNP_QUERIER_OtherQuerierPrentTimeout*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_QuerierStartTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the querier startup timeout
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_QuerierStartTimeout(
    void *parameter_p)
{
    MLDSNP_Timer_T *querier_timer_p = NULL;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_staus;
#endif
    UI16_T already_sent_counts = 0, need_send_count = 0;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetStartupQuerySentCount(&already_sent_counts);
    MLDSNP_OM_GetStartupQueryCount(&need_send_count);
    MLDSNP_OM_GetQuerierTimer(&querier_timer_p);

    if (already_sent_counts < (need_send_count-1))
    {
        MLDSNP_BD(TRACE, "already sent(%d) < need send(%d) ", already_sent_counts, need_send_count);
        MLDSNP_OM_SetStartupQuerySentCount(already_sent_counts + 1);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
             && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
            MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
        else
#endif
            MLDSNP_QUERIER_SendGeneralQeury(0, FALSE);

        if (FALSE == MLDSNP_TIMER_StartTimer(querier_timer_p))
        {
            MLDSNP_TIMER_FreeTimer(&querier_timer_p);
            MLDSNP_OM_StoreQuerierTimer(NULL);
        }
    }
    else
    {
        UI16_T query_interval = 0;

        MLDSNP_BD(TRACE, "normal general query ");

        MLDSNP_OM_GetQueryInterval(&query_interval);

        /*change the querier timer value*/
        querier_timer_p->type         = MLDSNP_TIMER_CYCLIC;
        querier_timer_p->orgianl_time = query_interval;
        querier_timer_p->func_p       = MLDSNP_QUERIER_GeneralQueryTimeOut;

        if (FALSE == MLDSNP_TIMER_StartTimer(querier_timer_p))
        {
            MLDSNP_TIMER_FreeTimer(&querier_timer_p);
            MLDSNP_OM_StoreQuerierTimer(NULL);
        }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
                && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
            MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
        else
#endif
            MLDSNP_QUERIER_SendGeneralQeury(0, FALSE);
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_QuerierStartTimeout*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_RouterPortTimeout
*------------------------------------------------------------------------------
* Purpose: This function process the dynamic learned router port expired
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_RouterPortTimeout(
    void * para_p)
{
    MLDSNP_TIMER_TimerPara_T *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)para_p;

    MLDSNP_BD(TRACE, "vid=%d, lport=%d, time=%ld",
              timer_para_p->vid, timer_para_p->lport, SYS_TIME_GetSystemTicksBy10ms());

    if (FALSE == MLDSNP_QUERIER_DeleteDynamicRouterPort(timer_para_p->vid, timer_para_p->lport))
    {
        MLDSNP_BD(TRACE, "vid=%d, lport=%d, delete dyanmic router port fail",
                  timer_para_p->vid, timer_para_p->lport);
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_RouterPortTimeout*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort
*------------------------------------------------------------------------------
* Purpose: This function delete the dynamic learned router port from all vlan
* INPUT  :  vlan - the vlan id
*           lport - the router port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :if vid =0 means all vlan, else specify the vlan.
*         This function is provided for caller not in mldsnp_querier.c
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort(
    UI16_T specify_vid,
    UI16_T lport)
{
    MLDSNP_BD(TRACE, "specify_vid %d, lport %d", specify_vid, lport);

    if (0 == specify_vid)
    {
        UI32_T next_vid = 0;

        while (TRUE == VLAN_OM_GetNextVlanId(0, &next_vid))
        {
            MLDSNP_QUERIER_DeleteDynamicRouterPort(next_vid, lport);
        }

        return TRUE;
    }
    else
    {
        return MLDSNP_QUERIER_DeleteDynamicRouterPort(specify_vid, lport);
    }

    /*return FALSE;*/
}/*End of MLDSNP_QUERIER_RemoveDynamicRouterPortOnSpecifyPort*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessTopologyChange
*------------------------------------------------------------------------------
* Purpose: This function process topology change
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_ProcessTopologyChange(UI32_T xstp_id, UI16_T lport)
{
    XSTP_MGR_MstpInstanceEntry_T mstp_instance_entry;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_staus;
#endif
    UI32_T vid = 0, vlan_ifindex = 0;
    BOOL_T is_exist = FALSE;

    MLDSNP_BD(TRACE, "xstp id %ld, port %d ", xstp_id, lport);

    if (FALSE == XSTP_OM_GetMstpInstanceVlanConfiguration(xstp_id, &mstp_instance_entry))
        return FALSE;

    /* give msti, then get vlan list from xstp for shrink queue used size
     */
    for (vid = 1; vid <= SYS_DFLT_DOT1QMAXVLANID ; vid++)
    {
        is_exist = FALSE;

        if ((0 < vid) && (vid < 1024))
        {
            if (mstp_instance_entry.mstp_instance_vlans_mapped[vid>>3] & (0x01 << (vid % 8)))
                is_exist = TRUE;
        }
        else if ((1023 < vid) && (vid < 2048))
        {
            if (mstp_instance_entry.mstp_instance_vlans_mapped2k[(vid>>3)-128] & (0x01 << (vid % 8)))
                is_exist = TRUE;
        }
        else if ((2047 < vid) && (vid < 3072))
        {
            if (mstp_instance_entry.mstp_instance_vlans_mapped3k[(vid>>3)-256] & (0x01 << (vid % 8)))
                is_exist = TRUE;
        }
        else if ((3071 < vid) && (vid < 4096))
        {
            if (mstp_instance_entry.mstp_instance_vlans_mapped4k[(vid>>3)-384] & (0x01 << (vid % 8)))
                is_exist = TRUE;
        }

        if (is_exist)
        {
            if (FALSE == VLAN_OM_ConvertToIfindex(vid, &vlan_ifindex))
                continue;

            if (FALSE == VLAN_OM_IsPortVlanMember(vlan_ifindex, lport))
                continue;

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_OM_GetProxyReporting(&proxy_staus)
                 && proxy_staus == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
            {
                MLDSNP_QUERIER_SendGeneralQeury(0, TRUE);
            }
            else
#endif
                MLDSNP_QUERIER_SendGeneralQeury(vid, TRUE);
        }

        MLDSNP_QUERIER_SendMrdSocitation(vid, TRUE);
    }

    return TRUE;
}/*End of MLDSNP_QUERIER_ProcessTopologyChange*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_MrdSolicitationTimeout
*------------------------------------------------------------------------------
* Purpose: This function send the mrd solicitation to all port
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_QUERIER_MrdSolicitationTimeout(
    void *para_p)
{
    MLDSNP_BD(TRACE, " ");

    if (MLDSNP_OM_IsMrouteEnabled())
        return TRUE;

    MLDSNP_QUERIER_SendMrdSocitation(0, FALSE);

    return TRUE;
}/*End of MLDSNP_QUERIER_MrdSolicitationTimeout*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdAdvertisement
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router advertisement message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdAdvertisement(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_BD(TRACE, " ");

    /*register router port*/
    if (FALSE == MLDSNP_QUERIER_AddDynamicRouterPort(msg_p->vid, msg_p->recevied_port))
    {
        return;
    }

    return;
}/*End of MLDSNP_QUERIER_ProcessMrdAdvertisement*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdSolicitation
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router solicitation message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdSolicitation(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_BD(TRACE, " ");

    MLDSNP_QUERIER_SendMrdAdvertisement(msg_p->vid, msg_p->recevied_port);
    return;
}/*End of MLDSNP_QUERIER_ProcessMrdSolicitation*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_ProcessMrdTermination
*------------------------------------------------------------------------------
* Purpose: This function process the multicast router termination message
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_ProcessMrdTermination(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_BD(TRACE, " ");

    /*register router port*/
    if (FALSE == MLDSNP_QUERIER_DeleteDynamicRouterPort(msg_p->vid, msg_p->recevied_port))
    {
        return;
    }

    return;
}/*End of MLDSNP_QUERIER_ProcessMrdTermination*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_QUERIER_SetMRouteStatus
*------------------------------------------------------------------------------
* Purpose: This function set multicast routing status
* INPUT  : *para_p      - the parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
void MLDSNP_QUERIER_SetMRouteStatus(BOOL_T is_enabled)

{
    MLDSNP_TYPE_QuerierStatus_T querier_status = MLDSNP_TYPE_QUERIER_DISABLED;
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status = MLDSNP_TYPE_MLDSNP_DISABLED;

    MLDSNP_BD(UI, " status %s ", is_enabled ? "Enabled" : "Disabled");

    MLDSNP_OM_SetMroutStatus(is_enabled);

    MLDSNP_OM_GetQuerierStauts(&querier_status);

    MLDSNP_OM_GetMldStatus(&mldsnp_status);

    if (is_enabled
            && querier_status == MLDSNP_TYPE_QUERIER_ENABLED)
        MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_DISABLED);
    else if (querier_status == MLDSNP_TYPE_QUERIER_ENABLED)
        MLDSNP_OM_InitialAllVlanQuerierRunningStatus(MLDSNP_TYPE_QUERIER_ENABLED);

    /* enable/disable mroute will clear the entry in chip,
       therefore also clear the mldsnp record */
    if(mldsnp_status == MLDSNP_TYPE_MLDSNP_ENABLED)
        MLDSNP_ENGINE_DeleteAllEntry();

    return;
}/*End of MLDSNP_QUERIER_SetMRouteStatus*/


