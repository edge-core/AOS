/* MODULE NAME: mldsnp_engine.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*     1. switch the query to mldsnp_querier.c, uknown multicast data to mldsnp_unknown.c
*         and report and done process here.
*     2. process the report and done message
*     3. collec the port jion group information to write to chip
*    {2. Related documents or hardware information}
*     1. msl
*     2. ipmc table in chip
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*     1. Filter mode is registered at the (vid, gip,0).
*     2. Unknonw is register at each (vid, gip, sip)
*     3. when process report need to send the query, it won't update the source timer and filter timer to the LLQI.
*         the query will count the specify-query-send-count to check this port shall leave group or not.
*     4. the chip entry will write (vid, gip, sip, port) when in  include list and unknown list.
*     5. the chip entry will write (vid, gip, sip, r_port) when in exclude list
*     6. om will write (vid, gip, sip, receive-report-port) for include, exclude and request list
*                ++++++++++++
*                +mlsdnp_mgr+
*                ++++++++++++
*
*                 ++++++++++++++++
*                 + mldsnp_engine+
*                 ++++++++++++++++
*
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
#include "mldsnp_engine.h"
#include "mldsnp_querier.h"
#include "mldsnp_unknown.h"
#include "mldsnp_timer.h"
#include "mldsnp_backdoor.h"
#include "l2mux_mgr.h"
#include "vlan_mgr.h"
#include "vlan_om.h"
#include "vlan_lib.h"
#include "l_cvrt.h"
#include "msl_pmgr.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "swctrl.h"

/* NAMING CONSTANT DECLARATIONS
*/
#define GROUP_LEN   MLDSNP_TYPE_IPV6_DST_IP_LEN
#define SRC_IP_LEN  MLDSNP_TYPE_IPV6_SRC_IP_LEN

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
*/

static BOOL_T MLDSNP_ENGINE_ConstructMref(
    L_MM_Mref_Handle_T *org_mref_p,
    L_MM_Mref_Handle_T     **mref_pp,
    UI8_T                  *pdu_p,
    UI16_T                 pdu_len,
    MLDSNP_TYPE_TxReason_T tx_reason);

static BOOL_T MLDSNP_ENGINE_VerifyPdu(
    MLDNSP_ENGINE_Msg_T  *msg_p);
static BOOL_T MLDSNP_ENGINE_DispatchPdu(
    MLDNSP_ENGINE_Msg_T *msg_p);
static BOOL_T MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI16_T input_port,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip);
static BOOL_T MLDSNP_ENGINE_ProcessV1Report(
    MLDNSP_ENGINE_Msg_T *msg_p);

static BOOL_T MLDSNP_ENGINE_GetFileterTimerTime(
    UI16_T       vid,
    UI8_T  *gip_ap,
    UI16_T input_port,
    UI32_T *filter_timer_time);

static UI32_T MLDSNP_ENGINE_ProcessOneRecord(
    UI16_T                   vid,
    UI8_T                    *gip_ap,
    UI16_T                   rcvd_port,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip);
static BOOL_T MLDSNP_ENGINE_ProcessV2Report(
    MLDNSP_ENGINE_Msg_T *msg_p);

static BOOL_T MLDSNP_ENGINE_ProcessPimHello(
    MLDNSP_ENGINE_Msg_T *msg_p);

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
static BOOL_T MLDSNP_ENGINE_NewAndCheckThrottle(UI32_T rcvd_port);
#endif
static void MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow(
    UI16_T vid,
    UI16_T input_port,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI8_T  *src_ip_list_p,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_IncludeModeHandle_IS_EX_and_TO_EX(
    UI16_T                   vid,
    UI16_T                   input_port,
    UI8_T                    *gip_ap,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_IncludeModeHandle_TO_IN(
    UI16_T           vid,
    UI16_T           input_port,
    UI8_T            *gip_ap,
    UI16_T           num_src,
    UI8_T            *src_ip_list_p,
    UI8_T    *host_src_ip);
static  void MLDSNP_ENGINE_IncludeModeHandle_Block(
    UI16_T vid,
    UI16_T input_port,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI8_T  *src_ip_list_p,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_ExcludeModeHandle_TO_IN(
    UI16_T   vid,
    UI16_T   input_port,
    UI8_T    *gip_ap,
    UI16_T   num_src,
    UI8_T    *src_ip_list_p,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_ExcludeModeHandle_IS_EX_and_TO_EX(
    UI16_T                  vid,
    UI16_T                  input_port,
    UI8_T                   *gip_ap,
    UI16_T                  num_src,
    UI8_T                   *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_ExcludeModeHandle_IS_IN_and_Allow(
    UI16_T                   vid,
    UI16_T                   input_port,
    UI8_T                    *gip_ap,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T    *host_src_ip);
static void MLDSNP_ENGINE_ExcludeModeHandle_Block(
    UI16_T  vid,
    UI16_T  input_port,
    UI8_T   *gip_ap,
    UI16_T  num_src,
    UI8_T   *src_ip_list_p,
    UI8_T    *host_src_ip);
static BOOL_T MLDSNP_ENGINE_IsInSourceList(
    UI8_T  *src_ip_ap,
    UI8_T  *src_ip_list_p,
    UI16_T num_of_src);
static BOOL_T MLDSNP_ENGINE_RegisterPortJoinGroup(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_TYPE_ListType_T list_type,
    MLDSNP_OM_PortInfo_T *port_info_p);
static BOOL_T MLDSNP_ENGINE_LeavePortFromGroup(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info_p);
static BOOL_T MLDSNP_ENGINE_ForwardRcvdPdu(
    MLDNSP_ENGINE_Msg_T *msg_p,
    UI8_T               *output_port_list_ap,
    MLDSNP_TYPE_TraceId_T              reason);
#if 0
static BOOL_T MLDSNP_ENGINE_AddPortToChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport);
#endif
static BOOL_T MLDSNP_ENGINE_AddPortBitmapToChipEntry(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI8_T *port_bitmap_ap);
static BOOL_T MLDSNP_ENGINE_DeletePortFromChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport);
static BOOL_T MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_bitmap_ap);

static BOOL_T MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(
    MLDNSP_ENGINE_Msg_T   *msg_p,
    MLDSNP_TYPE_TraceId_T reason);
static BOOL_T MLDSNP_ENGINE_ProcessDone(
    MLDNSP_ENGINE_Msg_T *msg_p);

static BOOL_T MLDSNP_ENGINE_FloodRcvdPDU(
    MLDNSP_ENGINE_Msg_T   *msg_p,
    MLDSNP_TYPE_TraceId_T reason);

static void MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info_p);

static void MLDSNP_ENGINE_MovePortFromIncludeListToRequestList(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info_p);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
static BOOL_T MLDSNP_ENGINE_PackAndSendV2Report(UI16_T vid, UI8_T *data_p, UI32_T data_len, UI8_T nr_of_record);
#endif

/* STATIC VARIABLE DEFINITIONS
*/
static UI8_T src_ip_list_save_aa[MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC][SRC_IP_LEN];
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
typedef struct MLDSNP_ENGINE_GrpSrcNode_S
{
    UI8_T grp[GROUP_LEN];
    struct L_list src_lst;
}MLDSNP_ENGINE_GrpSrcNode_T;

struct L_list mldsnp_engine_join_lst = {NULL,NULL,0,NULL, MLDSNP_OM_LinkListNodeFree};
struct L_list mldsnp_engine_leave_lst = {NULL,NULL,0,NULL, MLDSNP_OM_LinkListNodeFree};
#endif

/* LOCAL SUBPROGRAM BODIES
*/
extern UI8_T mldsnp_om_null_src_ip_a[SRC_IP_LEN];
extern UI8_T mldsnp_om_null_group_ip_a[GROUP_LEN];

static void MLDSNP_ENGINE_OrTwoPortList(UI8_T src[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST],
	                                    UI8_T result[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST])
{
  UI16_T i=0;
  for(; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
  {
    result[i]|=src[i];
  }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ConstructMref
 *-------------------------------------------------------------------------
 * PURPOSE  : To transmit cfm pdu by specified mep.
 * INPUT    : org_mref_p - the original mref
 *            mref_pp   - mref pinter's pointer
 *            pdu_p     - the packet content
 *            pdu_len   - the pdu length
 *            tx_reason - the tx reason
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_ConstructMref(
    L_MM_Mref_Handle_T *org_mref_p,
    L_MM_Mref_Handle_T    **mref_pp,
    UI8_T                 *pdu_p,
    UI16_T                 pdu_len,
    MLDSNP_TYPE_TxReason_T tx_reason)
{
    UI32_T len = 0;
    UI8_T  *buf_p;

    if (NULL == (*mref_pp = L_MM_AllocateTxBufferForPktForwarding(org_mref_p, pdu_len, L_MM_USER_ID2(SYS_MODULE_MLDSNP, tx_reason))))
    {
        return FALSE;
    }

    (*mref_pp)->current_usr_id = SYS_MODULE_MLDSNP;
    (*mref_pp)->next_usr_id    = SYS_MODULE_L2MUX;

    buf_p = (UI8_T *)L_MM_Mref_GetPdu(*mref_pp, &len);

    if (NULL == buf_p)
    {
        if (!L_MM_Mref_Release(mref_pp))
        {
            MLDSNP_BD(TX, "can't free mref");
        }
        return FALSE;
    }

    memcpy(buf_p, pdu_p, pdu_len);

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_GetFileterTimerTime
 *-------------------------------------------------------------------------
 * PURPOSE : This function record the mode change to exclude
 * INPUT   : vid        - vlan id
 *           gia_ap     - group ip array pointer
 *           input_port - the input port
 * OUTPUT  : filter_timer_time - the filter time last
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_GetFileterTimerTime(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T input_port,
    UI32_T *filter_timer_time)
{
    MLDSNP_OM_PortInfo_T port_info;

    /* record filter mode, Filter Timer=MALI*/
    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, input_port, &port_info))
    {
        return FALSE;
    }
    else
    {
        if (NULL == port_info.filter_timer_p)
            return FALSE;

        if (FALSE == MLDSNP_TIMER_QueryTimer(port_info.filter_timer_p, filter_timer_time))
            return FALSE;
    }

    return TRUE;
}


/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow
*-------------------------------------------------------------------------
* PURPOSE : Handle is include record
* INPUT   :  vid         - the input vid
*            *gip_ap     - the group ip address
*            num_src     - the number of src in this record
*            src_ip_list_p- the source ip list pointer
*            rec_type     - the record type to be processed
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static UI32_T MLDSNP_ENGINE_ProcessOneRecord(
    UI16_T                   vid,
    UI8_T                    *gip_ap,
    UI16_T                   rcvd_port,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip)
{
    MLDSNP_TYPE_CurrentMode_T current_mode;
    MLDSNP_OM_PortInfo_T      port_info;

    MLDSNP_BD(TRACE, " ");

    /*if this group addr in this record is not start from ff02, we don't process it
       because it doesn't link local multicast group
      */
    if (0xff != gip_ap[0])
    {
        MLDSNP_BD_SHOW_GROUP(RX, "vid:%d, port%d, This group is not start from ff", gip_ap, vid, rcvd_port);
        return MLDSNP_TYPE_PROC_FAIL;
    }

#if (SYS_CPNT_MLDSNP_V2_ASM == TRUE)
    /* when mroute is enable, let snp accept v2 report, else don't process */
    if(FALSE == MLDSNP_OM_IsMrouteEnabled())
    {
        if (MLDSNP_TYPE_ALLOW_NEW_SOURCES == rec_type
            || MLDSNP_TYPE_BLOCK_OLD_SOURCES == rec_type)
        {
            MLDSNP_BD(RX, "Don't support this record type %d; won't process", rec_type);
            return MLDSNP_TYPE_PROC_FAIL;
        }
        if ((MLDSNP_TYPE_IS_EXCLUDE_MODE == rec_type
                || MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type
                || MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE == rec_type
                || MLDSNP_TYPE_IS_INCLUDE_MODE == rec_type)
                && num_src != 0)
        {
            MLDSNP_BD_ARG(RX, "This record type %d can't have source address list; count %u; won't process\r\n", rec_type, num_src);
            return MLDSNP_TYPE_PROC_FAIL;
        }
    }
#endif

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    {
        UI32_T filer_type;
        if (VAL_mldSnoopProfileAction_deny == (filer_type = MLDSNP_ENGINE_ShallFilterGroup(gip_ap, rcvd_port)))
        {
            MLDSNP_BD(RX, "Drop report due to profile filtered");
            MLDSNP_OM_PortIncStat(rcvd_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
            return TRUE;
        }
        if (VAL_mldSnoopProfileAction_forward == filer_type)
        {
            return MLDSNP_TYPE_PROC_FLOOD;
        }
    }
#endif

    /*get (vid, gip) current mode*/
    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, rcvd_port, &port_info))
    {
        /*dosen't have this report currently
         */
        current_mode = MLDSNP_TYPE_IS_INCLUDE_MODE;
    }
    else
    {
        if (NULL == port_info.filter_timer_p)
            current_mode = MLDSNP_TYPE_IS_INCLUDE_MODE;
        else
            current_mode = MLDSNP_TYPE_IS_EXCLUDE_MODE;
    }

    if (MLDSNP_TYPE_IS_INCLUDE_MODE == current_mode)
    {
        switch (rec_type)
        {
            case MLDSNP_TYPE_IS_INCLUDE_MODE:
            case MLDSNP_TYPE_ALLOW_NEW_SOURCES:
                MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, host_src_ip);
                break;

            case MLDSNP_TYPE_IS_EXCLUDE_MODE:
            case MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE:
                MLDSNP_ENGINE_IncludeModeHandle_IS_EX_and_TO_EX(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, rec_type, host_src_ip);
                break;

            case MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE:
                MLDSNP_ENGINE_IncludeModeHandle_TO_IN(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, host_src_ip);
                break;

            case MLDSNP_TYPE_BLOCK_OLD_SOURCES:
                MLDSNP_ENGINE_IncludeModeHandle_Block(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, host_src_ip);
                break;
        }
    }
    else
    {
        switch (rec_type)
        {
            case MLDSNP_TYPE_IS_INCLUDE_MODE:
            case MLDSNP_TYPE_ALLOW_NEW_SOURCES:
                MLDSNP_ENGINE_ExcludeModeHandle_IS_IN_and_Allow(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, rec_type, host_src_ip);
                break;

            case MLDSNP_TYPE_IS_EXCLUDE_MODE:
            case MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE:
                MLDSNP_ENGINE_ExcludeModeHandle_IS_EX_and_TO_EX(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, rec_type, host_src_ip);
                break;

            case MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE:
                MLDSNP_ENGINE_ExcludeModeHandle_TO_IN(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, host_src_ip);
                break;

            case MLDSNP_TYPE_BLOCK_OLD_SOURCES:
                MLDSNP_ENGINE_ExcludeModeHandle_Block(vid, rcvd_port, gip_ap, num_src, src_ip_list_p, host_src_ip);
                break;
        }
    }

    return MLDSNP_TYPE_PROC_SUCC;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_VerifyPdu
*------------------------------------------------------------------------------
* Purpose: Veriy the PDU is good MLD pdu to process
* INPUT  :*msg_p   - the input parameter.
*         *dmac_ap - the destination mac
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  : This function will assign the icmp packet pointer and ipv6 header pointer
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_VerifyPdu(
    MLDNSP_ENGINE_Msg_T  *msg_p)
{
    MLDSNP_ENGINE_IPv6Header_T *ipv6_header_p = (MLDSNP_ENGINE_IPv6Header_T *)msg_p->ipv6_header_p;

    MLDSNP_BD(TRACE, " ");

    /*verify dst mac*/
    if (msg_p->dst_mac_a[0] != 0x33
            || msg_p->dst_mac_a[1] != 0x33)
    {
        MLDSNP_BD(RX, "dmac is not 33.33 start");
        goto EXIT_FALSE;
    }

    /*check first octect shall be 0xff*/
    if (ipv6_header_p->dip_a[0] != 0xff)
    {
        MLDSNP_BD(RX, "ip header dest ip not ff start address ");
        goto EXIT_FALSE;
    }

    /*check source ip can't be multicast*/
    if (0xff == ipv6_header_p->sip_a[0])
    {
        MLDSNP_BD(RX, "ip header src ip is multicast address ");
        goto EXIT_FALSE;
    }

    /*verify dst mac is the same as gip */
    if (0 != memcmp(&msg_p->dst_mac_a[2], &ipv6_header_p->dip_a[GROUP_LEN- 4], 4))
    {
        MLDSNP_BD(RX, "dmac is not match with dip last four octes");
        goto EXIT_FALSE;
    }

    /*if payload not icmp or udp, it doesn't belong to MLD*/
    if (MLDSNP_TYPE_IPV6_UDP_HEAD == ipv6_header_p->next_header
            || MLDSNP_TYPE_IPV6_OTHER_HEAD == ipv6_header_p->next_header)
    {
        msg_p->icmp_len = 0; /*there is no icmp here, so correct the variable*/
        MLDSNP_BD(RX, "ip next header is not icmp or is udp");
        return TRUE;
    }

    /*verify mldsnp type*/
    if ((msg_p->icmpv6_p->type   != MLDSNP_TYPE_QUERY)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_REPORT_V1)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_REPORT_V2)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_DONE)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_MRD_ADVERTISEMENT)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_MRD_SOLICITATION)
            && (msg_p->icmpv6_p->type != MLDSNP_TYPE_MRD_TERMINATION))
    {
        MLDSNP_BD_ARG(RX, "icmp type %02x is not mld type\r\n", msg_p->icmpv6_p->type);
        /*if it not mld type, just return true for flooding it.*/
        goto EXIT_FALSE;
    }

    if(msg_p->icmpv6_p->type == MLDSNP_TYPE_REPORT_V1
       &&(ipv6_header_p->dip_a[1] ==0x00
       ||ipv6_header_p->dip_a[1] ==0x02)
      )
    {
        MLDSNP_BD(RX, "ip header dest ip ff02 or ff00 start address ");
        goto EXIT_FALSE;
    }

    if (msg_p->icmpv6_p->type == MLDSNP_TYPE_DONE
            && (msg_p->ipv6_header_p->dip_a[GROUP_LEN-1] != 0x02
                || msg_p->ipv6_header_p->dip_a[1] != 0x02
               )
       )
    {
        MLDSNP_BD(RX, "type done but dip not end with 0x02");
        goto EXIT_FALSE;
    }

    if ((msg_p->icmpv6_p->type == MLDSNP_TYPE_MRD_ADVERTISEMENT
            || msg_p->icmpv6_p->type == MLDSNP_TYPE_MRD_SOLICITATION
            || msg_p->icmpv6_p->type == MLDSNP_TYPE_MRD_TERMINATION)
            && msg_p->ipv6_header_p->dip_a[GROUP_LEN-1] != 0x6a
            && msg_p->ipv6_header_p->dip_a[1] != 0x02
            /*&& msg_p->ipv6_header_p->dip_a[0]!=0xff*/)
    {
        MLDSNP_BD_ARG(RX, "type=%d, MRD message but DA is worng\r\n", msg_p->icmpv6_p->type);
        goto EXIT_FALSE;
    }

    /*icmp + 1/2 ip pseudo header*/
    if (0 != MLDSNP_QUERIER_GenerateCheckSum((UI8_T *)msg_p->icmpv6_p, ipv6_header_p->sip_a,  ipv6_header_p->dip_a,
            MLDSNP_TYPE_IPV6_ICMPV6_HAED, msg_p->icmp_len))
    {
        MLDSNP_BD(RX, "checksum calculate wrong");
        goto EXIT_FALSE;
    }

    return TRUE;

EXIT_FALSE:
    MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_INVALID, TRUE);
    return FALSE;
}/*End of MLDSNP_ENGINE_VerifyPdu*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_DispatchPdu
*------------------------------------------------------------------------------
* Purpose: This function dispatch the pdu to other engine to process according
*          to mld type
* INPUT  : *msg_p  - the mld pdu with other infomation structure
* OUTPUT : TRUE    - success
*          FALSE   - fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_DispatchPdu(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    BOOL_T ret = TRUE;

    MLDSNP_BD(TRACE, " ");

    if (0 != msg_p->icmp_len)
    {
        UI16_T  ver = MLDSNP_TYPE_VERSION_1;

        MLDSNP_OM_GetMldSnpVer(&ver);
        /*increase satistics*/
        switch (msg_p->icmpv6_p->type)
        {
            case MLDSNP_TYPE_REPORT_V1:
            case MLDSNP_TYPE_REPORT_V2:
                MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_JOINS, TRUE);
                {
#if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_PORT == TRUE)
                    if (TRUE == MLDSNP_OM_IsPortMldReportRcvdReachLimit(msg_p->recevied_port))
                    {
                        MLDSNP_BD(RX, "Drop; due to reach port limit in a seconds");
                        MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_DROP_RATE_LIMIT, TRUE);
                        return TRUE;
                    }
#endif
#if(SYS_CPNT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_PER_VLAN == TRUE)
                    if (TRUE == MLDSNP_OM_IsVlanMldReportRcvdReachLimit(msg_p->vid))
                    {
                        MLDSNP_BD(RX, "Drop; due to reach VLAN limit in a seconds");
                        MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_DROP_RATE_LIMIT, TRUE);
                        return TRUE;
                    }
#endif
                }
                break;
            case MLDSNP_TYPE_DONE:
                MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_LEAVE, TRUE);
                break;
            case MLDSNP_TYPE_QUERY:
            {
                MLDSNP_ENGINE_Query_T *query_p = (MLDSNP_ENGINE_Query_T *)msg_p->icmpv6_p;

                if (0 == memcmp(query_p->gip_a, mldsnp_om_null_src_ip_a, GROUP_LEN))
                    MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_GQ_RCVD, TRUE);
                else
                    MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_SQ_RCVD, TRUE);
            }
            break;

        }
        /*check configuration is allow to process it*/
        switch (msg_p->icmpv6_p->type)
        {
            case MLDSNP_TYPE_REPORT_V1:
            case MLDSNP_TYPE_REPORT_V2:
            case MLDSNP_TYPE_DONE:
            {
                MLDSNP_TYPE_QuerierStatus_T querier_status;

                /* check querier is enabled or has router port
                 */
                MLDSNP_OM_GetQuerierStauts(&querier_status);

                if ((MLDSNP_TYPE_QUERIER_DISABLED == querier_status)
                        && (0 == MLDSNP_OM_GetVlanRouterPortCount(msg_p->vid))
                        && (FALSE == MLDSNP_OM_IsMrouteEnabled()))
                {
                    MLDSNP_BD(RX, "Querier is disabled or has no router port");
                    return FALSE;
                }

                if (MLDSNP_OM_IsRouterPort(msg_p->vid, msg_p->recevied_port))
                {
                    MLDSNP_BD(RX, "Router port won't process report and done, forward to all router port");

                    if(msg_p->icmpv6_p->type == MLDSNP_TYPE_REPORT_V1)
                        MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_V1_REPORT);
                    else if(msg_p->icmpv6_p->type == MLDSNP_TYPE_REPORT_V2)
                        MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_V2_REPORT);
                    else
                        MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_DONE);

                    return FALSE;
                }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                MLDSNP_OM_SetLastReportHostAddress(msg_p->vid, msg_p->ipv6_header_p->sip_a);
#endif
            }
            break;
        }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

        /*dispach and process the message*/
        switch (msg_p->icmpv6_p->type)
        {
            case MLDSNP_TYPE_QUERY:
            {
                MLDSNP_ENGINE_Query_T *query_p = (MLDSNP_ENGINE_Query_T *)msg_p->icmpv6_p;

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                MLDSNP_OM_RouterPortInfo_T r_info;
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
                {
                    if (TRUE == MLDSNP_OM_GetRouterPortInfo(msg_p->vid, msg_p->recevied_port, &r_info))
                    {/*received on mrouter port, forward to all other mrouter port*/
                        if (0 == memcmp(query_p->gip_a, mldsnp_om_null_group_ip_a, GROUP_LEN))
                            MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GENERAL_QEURY);
                        else
                            MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GROUP_SPECIFIC_QEURY);
                    }
                }
                else
                {
                    if (0 == memcmp(query_p->gip_a, mldsnp_om_null_group_ip_a, GROUP_LEN))
                        MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GENERAL_QEURY);
                    else
                        MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GROUP_SPECIFIC_QEURY);
                }
#else
                /*if chip can only trap to cpu, then this flood function can be used or just mark as comment*/
                if (0 == memcmp(query_p->gip_a, mldsnp_om_null_group_ip_a, GROUP_LEN))
                    MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GENERAL_QEURY);
                else
                    MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_FORWARD_GROUP_SPECIFIC_QEURY);
#endif

                if (MLDSNP_TYPE_VERSION_1 == ver
                        && msg_p->icmp_len != MLDSNP_TYPE_QUERY_V1_LEN)
                {
                    MLDSNP_OM_VlanIncStat(msg_p->vid, MLDSNP_TYPE_STAT_INVALID, TRUE);
                    MLDSNP_BD(RX, "Snoop is version 1 won't process v2 query");
                    break;
                }

                ret = MLDSNP_QUERIER_ProcessQuery(msg_p);

                if (0 != memcmp(query_p->gip_a, mldsnp_om_null_group_ip_a, GROUP_LEN))
                {
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
                      ret = MLDSNP_ENGINE_ProxyReplyGSQuery(msg_p->vid, query_p->gip_a, (UI8_T *)query_p->sip_aa, query_p->num_of_src);
                    #endif
                }
                else
                {
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
                      ret = MLDSNP_ENGINE_ProxySendUnsolicitReports(msg_p->vid);
                    #endif
                }

                break;
            }
            case MLDSNP_TYPE_REPORT_V1:
                ret = MLDSNP_ENGINE_ProcessV1Report(msg_p);
                if (ret == TRUE)
                    MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_JOIN_SUCC, TRUE);
                break;

            case MLDSNP_TYPE_REPORT_V2:
                if (MLDSNP_TYPE_VERSION_1 == ver
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    || MLDSNP_TYPE_PROXY_REPORTING_DISABLE == proxy_status
                    #endif
                   )
                    /*forward this report to router port*/
                    MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_V2_REPORT);

                if (MLDSNP_TYPE_VERSION_2 == ver)
                {
                    ret = MLDSNP_ENGINE_ProcessV2Report(msg_p);
                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
                        MLDSNP_ENGINE_ProcessSendStoreList(msg_p->vid);
                    #endif
                }
                else
                {
                    MLDSNP_BD(RX, "Snoop is version 1 and won't process v2 report");
                }

                if (ret == TRUE)
                    MLDSNP_OM_PortIncStat(msg_p->recevied_port, msg_p->vid, MLDSNP_TYPE_STAT_JOIN_SUCC, TRUE);

                break;

            case MLDSNP_TYPE_DONE:
                ret = MLDSNP_ENGINE_ProcessDone(msg_p);
                if (ret == TRUE)
                    MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_DONE);

                break;
            case MLDSNP_TYPE_MRD_ADVERTISEMENT:
                MLDSNP_QUERIER_ProcessMrdAdvertisement(msg_p);
                break;

            case MLDSNP_TYPE_MRD_SOLICITATION:
                MLDSNP_QUERIER_ProcessMrdSolicitation(msg_p);
                MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_MRD_SOLICITATION);
                break;

            case MLDSNP_TYPE_MRD_TERMINATION:
                MLDSNP_QUERIER_ProcessMrdTermination(msg_p);
                break;

            default: /*icmp type but not mld type, flood*/
                MLDSNP_BD(RX, "Flood Unknown Type ICMP");
                MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_UNKNOWN_PDU);
                break;
        }/*End switch (msg_p->mldsnp_pdu_p->icmpv6.type)*/
    }/*end of if(MLDSNP_TYPE_IPV6_ICMPV6_HAED == msg_p->mldsnp_pdu_p->ipv6_header.next_header)*/
    else
    {
        ret = MLDSNP_UNKNOWN_ProcessUnknownMcastData(msg_p);
    }
    /*after process packet, it shall not have any one in store list*/
    MLDSNP_ENGINE_ClearProxyV2AllocatedMemory();
    return ret;
}/*End of MLDSNP_ENGINE_DispatchPdu*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer
 *-------------------------------------------------------------------------
 * PURPOSE : This function record the mode change to exclude
 * INPUT   : vid        - vlan id
 *           *gia_ap    - group ip array pointer
 *           input_port - the input port
 * OUTPUT  : None
 * RETURN  : TRUE  - success
 *           FALSE - fail
 * NOTE    : if(vid, g, 0) has port register, this is mean this (vid, g, sip_list) is in exclude mode.
 *           the filter_timer is put at port register at (vid, g, 0).
 *           if already in exclude mode, then update the mode fileter timer.
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI16_T input_port,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip)
{
    MLDSNP_OM_PortInfo_T   port_info;
    UI32_T MALI = MLDSNP_OM_GetMALI(vid);

    MLDSNP_BD_SHOW_GROUP(TRACE, "vid=%d, port=%d, MALI=%u", gip_ap, vid, input_port, MALI);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {
        MLDSNP_OM_HisamEntry_T entry_info;
        MLDSNP_TYPE_ProxyReporting_T proxy_status;
        UI16_T ver;

        if (MLDSNP_OM_GetMldSnpVer(&ver)
             && MLDSNP_TYPE_VERSION_2 == ver
             && MLDSNP_OM_GetProxyReporting(&proxy_status)
             && MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
             && FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            /*put into leave_lst about (0,G), because it will sendout IS_EX{}*/
            MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, mldsnp_om_null_src_ip_a, &mldsnp_engine_leave_lst);
        }
    }
#endif

    /* record filter mode, Filter Timer=MALI*/
    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, input_port, &port_info))
    {
        MLDSNP_BD(TRACE, "to exclude mode");

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
        if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
        {
            MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
            return FALSE;
        }
#endif

        MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, mldsnp_om_null_src_ip_a, rec_type);

        MLDSNP_OM_InitialportInfo(input_port,
                                  MLDSNP_TYPE_JOIN_DYNAMIC,
                                  MLDSNP_TYPE_LIST_EXCLUDE,
                                  SYS_TIME_GetSystemTicksBy10ms(),
                                  &port_info);

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_FilterTimerTimeout,
                vid,
                gip_ap,
                mldsnp_om_null_src_ip_a,
                0,
                input_port,
                MALI,
                MLDSNP_TIMER_ONE_TIME,
                &port_info.filter_timer_p))
            return FALSE;

        if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_LIST_INCLUDE, &port_info))
        {
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.filter_timer_p);
            return FALSE;
        }

        MLDSNP_OM_AddPortHostIp(vid, gip_ap, mldsnp_om_null_src_ip_a, host_src_ip, &port_info);
    }
    else if ((MLDSNP_TYPE_IS_EXCLUDE_MODE == rec_type
              || MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type)
             && port_info.join_type != MLDSNP_TYPE_JOIN_STATIC)
    {
        MLDSNP_BD(TRACE, "update timer");

        if (NULL != port_info.filter_timer_p
                && port_info.filter_timer_p->func_p == MLDSNP_ENGINE_FilterTimerTimeout)
        {
            if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(port_info.filter_timer_p, MALI))
            {
                return FALSE;
            }
        }
        else
        {
            if (FALSE == MLDSNP_TIMER_ModifyTimer(&port_info.filter_timer_p,
                                                  MLDSNP_ENGINE_FilterTimerTimeout,
                                                  MALI,
                                                  MLDSNP_TIMER_ONE_TIME))
            {
                return FALSE;
            }
        }

        port_info.last_report_time = SYS_TIME_GetSystemTicksBy10ms();
        port_info.specific_query_count = 0;
        MLDSNP_OM_AddPortHostIp(vid, gip_ap, mldsnp_om_null_src_ip_a, host_src_ip, &port_info);

        if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, &port_info))
        {
            return FALSE;
        }
    }
    return TRUE;
}/*End of MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_ProcessV1Report
*------------------------------------------------------------------------------
* Purpose: This function process the version 1 report
* INPUT  : *msg_p  - the msg parameter
* OUTPUT : TRUE    - success
*          FALSE   - fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_ProcessV1Report(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_OM_PortInfo_T        port_info;
    MLDSNP_OM_HisamEntry_T      hisam_entry;
    MLDSNP_ENGINE_V1Report_T    *report_p = (MLDSNP_ENGINE_V1Report_T *)msg_p->icmpv6_p;
    UI32_T                      time_now  = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T interval;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    BOOL_T already_has = FALSE;
#endif

    MLDSNP_BD(RX, "Process V1 report ");

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {
        MLDSNP_OM_RouterPortInfo_T r_info;
        MLDSNP_OM_GetProxyReporting(&proxy_status);

        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
        {
            if (MLDSNP_OM_GetRouterPortInfo(msg_p->vid, msg_p->recevied_port, &r_info))
            {
                MLDSNP_BD(RX, "Don't learn report; Received on router port");
                MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_V1_REPORT);
                return TRUE; /*mrouter port don't learn gorup*/
            }

            if (TRUE == MLDSNP_OM_GetHisamEntryInfo(msg_p->vid,
                                                    report_p->gip_a,
                                                    NULL,
                                                    MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
                                                    &hisam_entry)
                    && hisam_entry.register_port_list.nbr_of_element != 0)
            {
                already_has = TRUE;
            }
        }
    }
#endif

    MLDSNP_ENGINE_ProcessOneRecord(msg_p->vid, report_p->gip_a, msg_p->recevied_port, 0, NULL,
               MLDSNP_TYPE_IS_EXCLUDE_MODE, msg_p->ipv6_header_p->sip_a);

#if(SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
    if (MLDSNP_TYPE_PROC_FLOOD == proc_ret)
    {
        MLDSNP_ENGINE_FloodRcvdPDU(msg_p, MLDSNP_TYPE_TRACE_FLOOD_V1_REPORT);
        return TRUE;
    }
#endif

    /*below create the old ver host present timer
      */
    if (FALSE == MLDSNP_OM_GetPortInfo(msg_p->vid, report_p->gip_a, NULL, msg_p->recevied_port, &port_info))
    {
        return FALSE;
    }

    if (NULL == port_info.ver1_host_present_timer_p)
    {
        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_V1HostPresentTimeout,
                msg_p->vid,
                report_p->gip_a,
                mldsnp_om_null_src_ip_a, 0,
                msg_p->recevied_port,
                MLDSNP_OM_GetMALI(msg_p->vid),
                MLDSNP_TIMER_ONE_TIME,
                &port_info.ver1_host_present_timer_p))
        {
            return FALSE;
        }
        MLDSNP_OM_AddV1HostPresentPort(port_info.port_no);
    }
    else
    {
        if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(port_info.ver1_host_present_timer_p,
                MLDSNP_OM_GetMALI(msg_p->vid)))
        {
            return FALSE;
        }
    }

    port_info.specific_query_count = 0;

    if (FALSE == MLDSNP_OM_UpdatePortInfo(msg_p->vid, report_p->gip_a, NULL, &port_info))
    {
        MLDSNP_BD_ARG(RX, "can't update port %d with v1 host preset timer\r\n", msg_p->recevied_port);
        return FALSE;
    }

    /*
      *process report suppresion
      */
    if (FALSE == MLDSNP_OM_GetHisamEntryInfo(msg_p->vid,
            report_p->gip_a,
            NULL,
            MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        return FALSE;
    }

    MLDSNP_OM_GetQueryResponseInterval(&interval);

    if ((time_now - hisam_entry.last_fwd_to_router_time) < interval)
    {
        MLDSNP_BD(RX, "report suppresion");
        return TRUE;
    }

    /*update last report send to router time*/
    hisam_entry.last_fwd_to_router_time = time_now;
    MLDSNP_OM_SetHisamEntryInfo(&hisam_entry);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
        && already_has)
    {
        MLDSNP_BD(RX, "proxy reporting; don't forward; already has this group registered");
        return TRUE;
    }
#endif

    /*forward this report to router port
      */
    MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(msg_p, MLDSNP_TYPE_TRACE_FORWARD_V1_REPORT);
    return TRUE;
}/*End of MLDSNP_ENGINE_ProcessV1Report*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_ProcessV1Report
*------------------------------------------------------------------------------
* Purpose: This function process the version 2 report
* INPUT  : *msg_p  - the msg parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_ProcessV2Report(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_TYPE_RecordType_T    rec_type = MLDSNP_TYPE_IS_INCLUDE_MODE;
    MLDSNP_ENGINE_V2Report_T    *report_p = (MLDSNP_ENGINE_V2Report_T *)msg_p->icmpv6_p;
    MLDSNP_ENGINE_GroupRecord_T *rec_p = NULL;
    UI16_T proc_rec = 0, num_rec = 0, num_src = 0, auxlen = 0, offset = 0;
    UI8_T gip_a[GROUP_LEN] = {0}, *src_ip_list_p;

    MLDSNP_BD(RX, "Process V2 report ");

    num_rec = L_STDLIB_Ntoh16(report_p->num_of_group_rec);
    rec_p = (MLDSNP_ENGINE_GroupRecord_T *) & report_p->rec[0];

    for (;proc_rec < num_rec; proc_rec++)
    {
        rec_type = rec_p->rec_type;
        num_src = L_STDLIB_Ntoh16(rec_p->num_of_src);
        auxlen = rec_p->aux_data_len;
        memcpy(gip_a, rec_p->gip_a, GROUP_LEN);
        src_ip_list_p = (UI8_T *)rec_p->sip_aa;

        if ((rec_type == MLDSNP_TYPE_IS_INCLUDE_MODE) && (0 == num_src))
            return FALSE;

        if ((rec_type == MLDSNP_TYPE_ALLOW_NEW_SOURCES) && (0 == num_src))
            return FALSE;

        if ((rec_type == MLDSNP_TYPE_BLOCK_OLD_SOURCES) && (0 == num_src))
            return FALSE;

        MLDSNP_ENGINE_ProcessOneRecord(msg_p->vid, gip_a, msg_p->recevied_port, num_src, src_ip_list_p,
                                       rec_type, msg_p->ipv6_header_p->sip_a);

        /* pointer move to next rec*/
        offset = 20 /*record_type, aux data len, num_of_src, multicast addr*/
                 + num_src * SRC_IP_LEN
                 + auxlen;
        rec_p  = (MLDSNP_ENGINE_GroupRecord_T*)((UI8_T *)rec_p + offset);

        if (L_CVRT_GET_OFFSET(msg_p->icmpv6_p, rec_p) > msg_p->icmp_len)
        {
            MLDSNP_BD_ARG(RX, "error packet size, the recrod pointer offset %lu is over icmp length %d\r\n",
                          L_CVRT_GET_OFFSET(msg_p->icmpv6_p, rec_p), msg_p->icmp_len);
            return FALSE;
        }
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_ProcessV2Report*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_ProcessPimHello
*------------------------------------------------------------------------------
* Purpose: This function process the pim hello message
* INPUT  : *msg_p  - the msg parameter
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_ProcessPimHello(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
    MLDSNP_ENGINE_IPv6Header_T *ipv6_header_p = (MLDSNP_ENGINE_IPv6Header_T *)msg_p->ipv6_header_p;
    MLDSNP_ENGINE_PimHeader_T *pim =  (MLDSNP_ENGINE_PimHeader_T *)msg_p->pim6_header_p;
    MLDSNP_ENGINE_PimOption_T *pim_option;
    I32_T length = L_STDLIB_Ntoh16(msg_p->pim_len);
    UI16_T hello_holdtime = 0;
    UI8_T MLDSNP_PIM_HELLO_ADDR[MLDSNP_TYPE_IPV6_DST_IP_LEN]={0xff,0x02,0x00,0x00,
                                                              0x00,0x00,0x00,0x00,
                                                              0x00,0x00,0x00,0x00,
                                                              0x00,0x00,0x00,0x0d};

    if (length < MLDSNP_TYPE_PIM_HEAD_SIZE)
    {
        return FALSE;
    }

    if (memcmp(ipv6_header_p->dip_a, MLDSNP_PIM_HELLO_ADDR, MLDSNP_TYPE_IPV6_DST_IP_LEN)!=0)
    {
        return FALSE;
    }

    if (pim->vertype != MLDSNP_TYPE_PIM_HELLO)
    {
        MLDSNP_BD_ARG(RX, "Pim Packet, but not Hello in port %ld, vlan %ld\r\n", msg_p->recevied_port, msg_p->vid);
        return FALSE;
    }

    pim_option = (MLDSNP_ENGINE_PimOption_T *)((UI8_T *)pim + MLDSNP_TYPE_PIM_HEAD_SIZE);

    length -= MLDSNP_TYPE_PIM_HEAD_SIZE;
    while (length > 0)
    {
        if (L_STDLIB_Ntoh16(pim_option->type) == MLDSNP_TYPE_PIM_HELLO_HOLDTIME_OPTION)
        {
            hello_holdtime = L_STDLIB_Ntoh16(*(UI16_T *) (pim_option->param));
            break;
        }
        else
        {
            pim_option = (MLDSNP_ENGINE_PimOption_T *)((UI8_T *)pim_option + sizeof(MLDSNP_ENGINE_PimOption_T));
            length -= sizeof(MLDSNP_ENGINE_PimOption_T);
        }
    }

    if (hello_holdtime == 0)
    {
        MLDSNP_BD_ARG(RX, "Pim Hello is zero in port %ld, vlan %ld\r\n", msg_p->recevied_port, msg_p->vid);
        return FALSE;
    }

    MLDSNP_BD_ARG(RX, "Pim Hello received in %ld, vlan %ld\r\n", msg_p->recevied_port, msg_p->vid);

    if (FALSE == MLDSNP_QUERIER_AddDynamicRouterPort(msg_p->vid, msg_p->recevied_port))
    {
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
static BOOL_T MLDSNP_ENGINE_NewAndCheckThrottle(UI32_T rcvd_port)
{
    UI32_T throttling_number, action_mode;

    /* throtting check*/
    if (VAL_mldSnoopThrottlePortRunningStatus_true == MLDSNP_OM_GetThrottleStatus(rcvd_port))
    {
        if (TRUE == MLDSNP_ENGINE_ShallThrottleGroup(rcvd_port))
        {
            MLDSNP_OM_GetMLDThrottlingAction(rcvd_port, &action_mode);
            MLDSNP_OM_GetPortMLDThrottlingNumber(rcvd_port, &throttling_number);

            if ((VAL_mldSnoopThrottlePortAction_deny == action_mode)
                    || (0 == throttling_number))
            {
                MLDSNP_BD(RX, "Drop report due to throttle");
                return TRUE;
            }

            MLDSNP_ENGINE_RemoveDynamicGroupbyCount(rcvd_port, 1);
        }
    }

    return FALSE;
}
#endif


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow
 *-------------------------------------------------------------------------
 * PURPOSE : Handle is include record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :   current state     report received       new state         action
 *                 IN(A)                IS_IN(B)      IN(A+B)          (B)=MALI
  *                IN(A)                ALLOW(B)      IN(A+B)          (B)=MALI
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow(
    UI16_T vid,
    UI16_T input_port,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI8_T  *src_ip_list_p,
    UI8_T *host_src_ip)
{
    MLDSNP_OM_PortInfo_T port_info;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI32_T MALI     = MLDSNP_OM_GetMALI(vid);
    UI16_T proc_src;

    MLDSNP_BD(TRACE, " ");
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif
    /*(B)=MALI*/
    for (proc_src = 0; proc_src < num_src; proc_src++, src_ip_list_p = src_ip_list_p + SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == src_ip_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }

        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
            && (FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, src_ip_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
               || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)
               )
           )
        {
            MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, src_ip_list_p, &mldsnp_engine_join_lst);
        }
        #endif

        /*port info not exit, register new group
          */
        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap,  src_ip_list_p, input_port, &port_info))
        {
            MLDSNP_BD_SHOW_GROUP_SRC(RX, "register new group", gip_ap, src_ip_list_p, 1);

            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif

            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, src_ip_list_p, MLDSNP_TYPE_LIST_INCLUDE);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_INCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                    vid,
                    gip_ap,
                    src_ip_list_p,
                    1,
                    input_port,
                    MALI,
                    MLDSNP_TIMER_ONE_TIME,
                    &port_info.src_timer_p))
                continue;

            port_info.last_report_time = time_now;
            port_info.register_time    = time_now;

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, src_ip_list_p, MLDSNP_TYPE_LIST_INCLUDE, &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
            }

            MLDSNP_OM_AddPortHostIp(vid, gip_ap, src_ip_list_p, host_src_ip, &port_info);

            continue;
        }
        else
        {
            /*already exist, update portinfo timer
              */
            MLDSNP_BD_SHOW_GROUP_SRC(RX, "update the registered group info", gip_ap, src_ip_list_p, 1);

            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
                continue;

            MLDSNP_OM_AddPortHostIp(vid, gip_ap, src_ip_list_p, host_src_ip, &port_info);

            if (FALSE ==  MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, MALI))
                continue;

            port_info.last_report_time     = time_now;
            port_info.specific_query_count = 0;

            if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, src_ip_list_p, &port_info))
            {
                continue;
            }
        }
    }

    return;
}/*End of MLDSNP_ENGINE_IncludeModeHandle_IS_IN_and_Allow*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IncludeModeHandle_IS_EX_and_TO_EX
 *-------------------------------------------------------------------------
 * PURPOSE : Handle in exclude  record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 *           rec_type      - the rec type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :   current state   report received  new state      action
 *             INCLUDE(A)      IS_EX(B)         EX(A*B, B-A)   (B-A) =0, Del(A-B), Filter Timer=MALI
 *             INCLUDE(A)      TO_EX(B)         EX(A*B, B-A)   (B-A) =0, Del(A-B), Filter Timer=MALI, Send Q(MA, A*B)
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_IncludeModeHandle_IS_EX_and_TO_EX(
    UI16_T                   vid,
    UI16_T                   input_port,
    UI8_T                    *gip_ap,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_OM_PortInfo_T          port_info;
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src = 0, saved_src_ip_num = 0, LLQI = 0, robustness_value = 0;
    UI8_T *tmp_src_list_p       = src_ip_list_p;
    UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;

    MLDSNP_BD(TRACE, " ");

    /* record filter mode, Filter Timer=MALI
        it register port at (vid, g, 0).
     */
    if (FALSE == MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(vid, gip_ap, num_src, input_port, rec_type, host_src_ip))
    {
        MLDSNP_BD(RX, "change mode fail");
        return;
    }

    memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
    MLDSNP_OM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);
    MLDSNP_OM_GetRobustnessOperValue(vid, &robustness_value);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    for (proc_src = 0; proc_src < num_src; proc_src++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }
        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p , input_port, &port_info))
        {/* (B-A)=0 , only register in om */
            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif

            /*notify unknonw to clear registered port*/
            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_EXCLUDE);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_EXCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            port_info.register_time = time_now;

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_EXCLUDE, &port_info))
            {
                continue;
            }

            #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            /*if there is port (S,G,V) in inlcude, we can't send leave*/
            /*if there is port (S,G,V) in exclude, we needn't send leave, becaus it ever send it*/
            if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                && entry_info.register_port_list.nbr_of_element == 1)
                MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
            #endif
        }
        else
        {/* (A*B)  timer no change*/

            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
            {
                continue;
            }

            if (MLDSNP_TYPE_IMMEDIATE_DISABLED == immediate_leave_status)
            {
                if (MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type)
                {
                    if (TRUE == MLDSNP_QUERIER_IsQuerierRunning(vid))
                    {
                        port_info.specific_query_count = 1;

                        /*save A*B*/
                        if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
                        {
                            memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                            saved_src_ip_num ++ ;
                        }
                    }
                }
                else
                {
                    port_info.specific_query_count = 0;
                    port_info.last_report_time = time_now;
                }
                port_info.list_type = MLDSNP_TYPE_LIST_REQUEST;

                if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                {
                    continue;
                }

                MLDSNP_ENGINE_MovePortFromIncludeListToRequestList(vid, gip_ap, tmp_src_list_p, &port_info);
            }
            else
            {
                /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE) */
                if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                {
                if (MLDSNP_OM_DeletePortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info) != 0)
                    continue;
                }
                /* #endif */

                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;

                if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                {
                    continue;
                }

                MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(vid, gip_ap, tmp_src_list_p, &port_info);

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                    && TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                    && FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                {
                    MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
                }
                #endif
            }
        }
    }

    /*Delete A-B*/
    {
        MLDSNP_OM_HisamEntry_T entry_info;
        UI16_T nxt_vid             = vid;
        UI8_T nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

        while (TRUE  == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (vid != nxt_vid
                    || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            port_info.port_no = input_port;

            if (FALSE == MLDSNP_ENGINE_IsInSourceList(nxt_sip_a, src_ip_list_p, num_src)
                    && TRUE  == L_SORT_LST_Get(&entry_info.register_port_list, &port_info)
                    && MLDSNP_TYPE_LIST_EXCLUDE != port_info.list_type
                    && MLDSNP_TYPE_JOIN_STATIC  != port_info.join_type
               )
            {
                if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(vid, nxt_gip_a, nxt_sip_a, input_port))
                {
                    continue;
                }

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                    &&(FALSE == MLDSNP_OM_GetHisamEntryInfo(nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                       || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                   )
                {
                    MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(nxt_gip_a, nxt_sip_a, &mldsnp_engine_leave_lst);
                }
                #endif
            }
        }
    }

    /*send Q(MA, A*B), if A*B has element is static, it won't put in save_src_ip_list_aap*/
    if (MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type
            && MLDSNP_TYPE_IMMEDIATE_DISABLED     == immediate_leave_status
            && 0 != saved_src_ip_num
            && TRUE == MLDSNP_QUERIER_IsQuerierRunning(vid))
    {
        MLDSNP_Timer_T *timer_p = NULL;

        MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "src in g-s-s", gip_ap, save_src_ip_list_aap, saved_src_ip_num);

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                vid,
                gip_ap,
                (UI8_T *)save_src_ip_list_aap,
                saved_src_ip_num,
                input_port,
                LLQI,
                MLDSNP_TIMER_ONE_TIME, &timer_p))
        {
            MLDSNP_BD(RX, "create specified query timer fail");
        }

        /*send query*/
        MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);
    }
    return;
}/*End of MLDSNP_ENGINE_IncludeModeHandle_IS_EX_and_TO_EX*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IncludeModeHandle_TO_IN
 *-------------------------------------------------------------------------
 * PURPOSE : Handle change to include  record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :   current state  report received   new state    action
 *             IN(A)          TO_IN(B)          IN(A+B)      B=MALI, SendQ(MA, A-B)
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_IncludeModeHandle_TO_IN(
    UI16_T   vid,
    UI16_T   input_port,
    UI8_T    *gip_ap,
    UI16_T   num_src,
    UI8_T    *src_ip_list_p,
    UI8_T    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    MLDSNP_OM_PortInfo_T          port_info;
    UI32_T MALI     = MLDSNP_OM_GetMALI(vid);
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src,  LLQI = 0;
    UI8_T *tmp_src_list_p    = src_ip_list_p;
    BOOL_T is_querier_runing = FALSE;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    /*(B) = MALI*/
    for (proc_src = 0; proc_src < num_src; proc_src++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }
        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
            && (FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
               || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)
               )
           )
        {
            MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_join_lst);
        }
        #endif
        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info))
        {/*B-A*/
            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif
            /*notify unknonw to clear registered port*/
            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_INCLUDE);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_INCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                    vid,
                    gip_ap,
                    tmp_src_list_p,
                    1,
                    input_port,
                    MALI,
                    MLDSNP_TIMER_ONE_TIME,
                    &port_info.src_timer_p))
                continue;

            port_info.last_report_time = time_now;
            port_info.register_time    = time_now;

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_INCLUDE, &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                continue;
            }

            MLDSNP_OM_AddPortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info);
        }
        /*A*B*/
        else if (MLDSNP_TYPE_JOIN_DYNAMIC == port_info.join_type)
        {
            MLDSNP_OM_AddPortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info);

            if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, MALI))
            {
                continue;
            }

            port_info.last_report_time     = time_now;
            port_info.specific_query_count = 0;

            if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
            {
                continue;
            }
        }
    }

    MLDSNP_OM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);

    is_querier_runing = MLDSNP_QUERIER_IsQuerierRunning(vid);

    /*Send Q(MA, A-B)*/
    {
        MLDSNP_OM_HisamEntry_T entry_info;
        MLDSNP_Timer_T         *timer_p = NULL;
        UI16_T nxt_vid = vid, saved_src_ip_num = 0, LLQI = 0, robust_value = 0;
        UI8_T nxt_gip_a[GROUP_LEN]  = {0}, nxt_sip_a[SRC_IP_LEN] = {0};
        UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;

        MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
        MLDSNP_OM_GetRobustnessOperValue(vid, &robust_value);

        memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

        while (TRUE  == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (vid != nxt_vid || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            port_info.port_no = input_port;
            if (FALSE == MLDSNP_ENGINE_IsInSourceList(nxt_sip_a, src_ip_list_p, num_src)
                    && TRUE  == L_SORT_LST_Get(&entry_info.register_port_list, &port_info))
            {
                if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
                    continue;

                if (MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status)
                {/*if immediate leave enable, this entry needn't to send g-s-s query so that just remove it*/

                    /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                    if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                    {

                    if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, nxt_sip_a, host_src_ip, &port_info))
                        continue;
                    }
                    /* #endif  */

                    MLDSNP_ENGINE_PortLeaveGroup(nxt_vid, nxt_gip_a, nxt_sip_a, port_info.port_no);

                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                        &&(FALSE == MLDSNP_OM_GetHisamEntryInfo(nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                        || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                       )
                    {
                        MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, nxt_sip_a, &mldsnp_engine_leave_lst);
                    }
                    #endif

                    continue;
                }

                /*save (A-B)*/
                if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC
                        && TRUE == is_querier_runing)
                {
                    port_info.specific_query_count = 1;

                    if (FALSE == MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info))
                    {
                        continue;
                    }

                    memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], nxt_sip_a, SRC_IP_LEN);
                    saved_src_ip_num++;
                }
            }
        }

        /*send g-s-s query
          */

        /*if immediate enable, it needn't to send the query*/
        if (MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status
                || 0 == saved_src_ip_num
                || FALSE == is_querier_runing)
        {
            return;
        }

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                vid,
                gip_ap,
                (UI8_T *)save_src_ip_list_aap,
                saved_src_ip_num,
                input_port,
                LLQI,
                MLDSNP_TIMER_ONE_TIME, &timer_p))
        {
            return;
        }

        /*send query*/
        MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);
    }
    return;
}/*End of MLDSNP_ENGINE_IncludeModeHandle_TO_IN*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_IncludeModeHandle_Block
 *-------------------------------------------------------------------------
 * PURPOSE : Handle block old  sources record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :   current state     report received    new state     action
 *             IN(A)             BLOCK(B)           IN(A)         Send Q(MA, A*B)
 *-------------------------------------------------------------------------
 */
static  void MLDSNP_ENGINE_IncludeModeHandle_Block(
    UI16_T vid,
    UI16_T input_port,
    UI8_T  *gip_ap,
    UI16_T num_src,
    UI8_T  *src_ip_list_p,
    UI8_T  *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    MLDSNP_Timer_T *timer_p = NULL;
    MLDSNP_OM_PortInfo_T port_info;
    UI16_T proc_src, saved_src_ip_num = 0, LLQI = 0, robust_value = 0;
    UI8_T *tmp_src_list_p       = src_ip_list_p;
    UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;
    BOOL_T is_querier_runing    = FALSE;

    MLDSNP_BD(TRACE, " ");

    memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
    MLDSNP_OM_GetRobustnessOperValue(vid, &robust_value);
    MLDSNP_OM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    is_querier_runing = MLDSNP_QUERIER_IsQuerierRunning(vid);

    /* A*B */
    for (proc_src = 0; proc_src < num_src; proc_src ++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }

        if (TRUE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info))
        {
            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
                continue;

            if (MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status)
            {
                /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                {
                if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info))
                    continue;
                }
                /* #endif  */

                if (TRUE == MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap, tmp_src_list_p, input_port))
                {
                    continue;
                }

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                    &&(FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                    || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)))
                {
                    MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
                }
                #endif
            }
            /*save src_ip*/
            else if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC
                     && TRUE == is_querier_runing)
            {
                port_info.specific_query_count = 1;

                if (FALSE  == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                {
                    continue;
                }

                memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                saved_src_ip_num ++;
            }
        }
    }

    /*if immediate leave enabled, it needn't to send the g-s-s to this port*/
    if (MLDSNP_TYPE_IMMEDIATE_ENABLED == immediate_leave_status
            || 0 == saved_src_ip_num
            || FALSE == is_querier_runing)
    {
        return;
    }

    /*send Q(MA, A*B)
      */
    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
            vid,
            gip_ap,
            (UI8_T *)save_src_ip_list_aap,
            saved_src_ip_num,
            input_port,
            LLQI,
            MLDSNP_TIMER_ONE_TIME,
            &timer_p))
    {
        MLDSNP_BD(RX, " create specified query timer fail");
    }

    /*send query*/
    MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);
    return;
}/*End of MLDSNP_ENGINE_IncludeModeHandle_Block*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ExcludeModeHandle_TO_IN
 *-------------------------------------------------------------------------
 * PURPOSE : Handle change to include  record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :  current state   report       new state         action
 *            EX(X,Y)         TO_IN(A)     EX(X+A, Y-A)      (A)=MALI, SendQ(MA, X-A), SendQ(MA)
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_ExcludeModeHandle_TO_IN(
    UI16_T   vid,
    UI16_T   input_port,
    UI8_T    *gip_ap,
    UI16_T   num_src,
    UI8_T    *src_ip_list_p,
    UI8_T    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T ver;
#endif
    MLDSNP_TYPE_ImmediateStatus_T imm_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    MLDSNP_OM_PortInfo_T          port_info;
    MLDSNP_Timer_T                *timer_p = NULL;
    UI32_T MALI     = MLDSNP_OM_GetMALI(vid);
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src = 0, LLQI = 0, robustness = 0;
    UI8_T *tmp_src_list_p = src_ip_list_p;

    MLDSNP_BD(TRACE, " ");

    /* record filter mode, Filter Timer=MALI*/
    if (FALSE == MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(vid, gip_ap, num_src, input_port, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE, host_src_ip))
    {
        MLDSNP_BD(TRACE, "change mode fail");
        return;
    }

    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
    MLDSNP_OM_GetRobustnessOperValue(vid, &robustness);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    /*(A)=MALI*/
    for (proc_src = 0; proc_src < num_src; proc_src ++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(TRACE, "register src is multicast addr");
            continue;
        }
        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
            && (FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
               || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)
               )
           )
        {
            MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_join_lst);
        }
        #endif
        /*(A-X-Y) = MALI*/
        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info))
        {
            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif

            /*notify unknonw to clear registered port*/
            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                    vid,
                    gip_ap,
                    tmp_src_list_p,
                    1,
                    input_port,
                    MALI,
                    MLDSNP_TIMER_ONE_TIME,
                    &port_info.src_timer_p))
                continue;

            port_info.last_report_time = time_now;
            port_info.register_time    = time_now;

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST,  &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                continue;
            }

            MLDSNP_OM_AddPortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info);
        }
        else /*(A*X), (A*Y) = MALI*/
        {
            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
                continue;

            MLDSNP_OM_AddPortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info);

            if (MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type) /*(A*Y)*/
            {
                port_info.list_type = MLDSNP_TYPE_LIST_REQUEST;

                if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                        vid,
                        gip_ap,
                        tmp_src_list_p,
                        1,
                        input_port,
                        MALI,
                        MLDSNP_TIMER_ONE_TIME,
                        &port_info.src_timer_p))
                {
                    continue;
                }
            }
            else /*(A*X)*/
            {
                MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, MALI);
            }

            port_info.last_report_time     = time_now;
            port_info.specific_query_count = 0;

            if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
            {
                continue;
            }
        }
    }

    MLDSNP_OM_GetImmediateLeaveStatus(vid, &imm_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);

    /*Send Q(MA, X-A), Q(MA)*/
    {
        MLDSNP_OM_HisamEntry_T entry_info;
        UI16_T nxt_vid = vid, saved_src_ip_num = 0;
        UI8_T nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};
        UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;

        memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

        while (TRUE  == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (nxt_vid != vid
                    || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            port_info.port_no = input_port;

            if (TRUE  == L_SORT_LST_Get(&entry_info.register_port_list, &port_info)
                && FALSE == MLDSNP_ENGINE_IsInSourceList(nxt_sip_a, src_ip_list_p, num_src)
                && MLDSNP_TYPE_JOIN_STATIC != port_info.join_type
                && MLDSNP_TYPE_LIST_EXCLUDE != port_info.list_type
               )
            {
                if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_status)
                {
                    /*save (X-A)*/
                    if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
                    {
                        port_info.specific_query_count = 1;
                        memcpy(&save_src_ip_list_aap [saved_src_ip_num*SRC_IP_LEN], nxt_sip_a, SRC_IP_LEN);
                        saved_src_ip_num ++ ;
                    }

                    if (FALSE == MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info))
                    {
                        continue;
                    }
                }
                else
                {
                    /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                    if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                    {
                    if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, nxt_sip_a, host_src_ip, &port_info))
                        continue;
                    }
                    /* #endif  */

                    /*move to exclude list, because not in (X-A) neen't to send g-s-s query*/
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                    port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;

                    if (FALSE == MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info))
                    {
                        continue;
                    }

                    MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(vid, gip_ap, nxt_sip_a, &port_info);

                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                        && TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                        && FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)
                       )
                    {
                        MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, nxt_sip_a, &mldsnp_engine_leave_lst);
                    }
                    #endif
                }
            }
        }

        if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_status)
        {
            /*send g-s-s. it need have src_ip to send.
               if immediate leave enable, it mean all x-a have bee remove to exclude directly so it needn't to send g-s-s*/
            if (saved_src_ip_num != 0
                && TRUE == MLDSNP_QUERIER_IsQuerierRunning(vid))
            {
                if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                        vid,
                        gip_ap,
                        (UI8_T *)save_src_ip_list_aap,
                        saved_src_ip_num,
                        input_port,
                        LLQI,
                        MLDSNP_TIMER_ONE_TIME, &timer_p))
                {
                    return;
                }

                /*send query*/
                MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);
            }

            /*send Q(MA), it needn't to care immediate leave, beacause A still in request list*/
            if (TRUE == MLDSNP_QUERIER_IsQuerierRunning(vid))
            {
                if (TRUE == MLDSNP_OM_GetPortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, input_port, &port_info))
                {
                    port_info.specific_query_count = 1;

                    MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, NULL, 0);
                    MLDSNP_OM_UpdatePortInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, &port_info);

                    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                            vid,
                            gip_ap,
                            NULL,
                            0,
                            input_port,
                            LLQI,
                            MLDSNP_TIMER_ONE_TIME, &timer_p))
                    {
                        return;
                    }
                }
            }
        }
        else if (0 == num_src) /*port leave (vid, gip, *), immediate leave enabled*/
        {
            UI16_T nxt_vid = vid;
            UI8_T  nxt_gip_a[GROUP_LEN] = {0};
            UI8_T  nxt_sip_a[SRC_IP_LEN] = {0};

            memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

/*            #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE) */
            if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
            {
            if (MLDSNP_TYPE_IMMEDIATE_ENABLED == imm_status
                && MLDSNP_OM_GetPortInfo(vid, nxt_gip_a, nxt_sip_a, input_port, &port_info)
                && (0 != MLDSNP_OM_DeletePortHostIp(nxt_vid, nxt_gip_a, nxt_sip_a, host_src_ip, &port_info)))
                return;
            }
/*            #endif */

            MLDSNP_ENGINE_PortLeaveGroup(nxt_vid, nxt_gip_a, nxt_sip_a, input_port);

            #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                && FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
            {
                MLDSNP_OM_GetMldSnpVer(&ver);
                if(MLDSNP_TYPE_VERSION_2 == ver)
                  MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, nxt_sip_a, &mldsnp_engine_join_lst);
                else
                  MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, nxt_sip_a, &mldsnp_engine_leave_lst);
            }
            #endif

            while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
            {
                if (nxt_vid != vid
                        || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                    break;

                if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, nxt_sip_a, input_port, &port_info))
                    continue;

                MLDSNP_ENGINE_PortLeaveGroup(nxt_vid, nxt_gip_a, nxt_sip_a, port_info.port_no);
            }
        }
    }
    return;
}/*End of MLDSNP_ENGINE_ExcludeModeHandle_TO_IN*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ExcludeModeHandle_IS_EX_and_TO_EX
 *-------------------------------------------------------------------------
 * PURPOSE : Handle change to exclude record
 * INPUT   :  vid              - the input vid
 *                input_port    - record input port
 *                *gip_ap       - the group ip address
 *                num_src      - the number of src in this record
 *                src_ip_list_p- the source ip list pointer
 *                rec_type     - the record type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :  current state       report      new state            action
 *            EX(X,Y)             IS_EX(A)     EX(A-Y, Y*A)      (A-X-Y)=MALI, Del(X-A), Del(Y-A), Filter timer=MALI
 *            EX(X,Y)             TO_EX(A)     EX(A-Y, Y*A)      (A-X-Y) = Filter timer, Del(X-A), Del(Y-A), SendQ(MA, A-Y), Filter timer=MALI
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_ExcludeModeHandle_IS_EX_and_TO_EX(
    UI16_T                  vid,
    UI16_T                  input_port,
    UI8_T                   *gip_ap,
    UI16_T                  num_src,
    UI8_T                   *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_TYPE_ImmediateStatus_T imm_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    MLDSNP_OM_PortInfo_T          port_info;
    UI32_T new_time = 0;
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src = 0, LLQI = 0, robustness = 0;
    UI8_T *tmp_src_list_p = src_ip_list_p, saved_src_ip_num = 0;
    UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;

    MLDSNP_BD(TRACE, " ");

    /* record filter mode, Filter Timer=MALI*/
    if (FALSE == MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(vid, gip_ap, num_src, input_port, rec_type, host_src_ip))
    {
        MLDSNP_BD(RX, "change mode fail");
        return;
    }

    memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
    MLDSNP_OM_GetImmediateLeaveStatus(vid, &imm_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
    MLDSNP_OM_GetRobustnessOperValue(vid, &robustness);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    for (proc_src = 0; proc_src < num_src; proc_src++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }

        /*(A-X-Y) = new_time*/
        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info))
        {
            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif

            /*notify unknonw to clear registered port*/
            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_status)
            {
                port_info.register_time    = time_now;

                if (MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type)
                {
                    port_info.specific_query_count = 1;

                    /*save (A-Y)*/
                    if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
                    {
                        memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                        saved_src_ip_num ++;
                    }
                }

                {/*(A-X-Y)*/
                    port_info.last_report_time = time_now;
                    new_time = MLDSNP_OM_GetMALI(vid);

                    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                            vid,
                            gip_ap,
                            tmp_src_list_p,
                            1,
                            input_port,
                            new_time,
                            MLDSNP_TIMER_ONE_TIME,
                            &port_info.src_timer_p))
                        continue;
                    port_info.specific_query_count = 0;
                }

                if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST, &port_info))
                {
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                    continue;
                }
            }
            else /*just register to exclude list*/
            {
                if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_EXCLUDE, &port_info))
                {
                    continue;
                }
            }

            #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                &&TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                    && entry_info.register_port_list.nbr_of_element == 1)
            {
                MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
            }
            /*if entry exit and have port in include, we can't send leave*/
            /*if entry exit and have port in exclude, we neen't send leave*/
            #endif
        }
        else
        {
            if (MLDSNP_TYPE_LIST_EXCLUDE != port_info.list_type
                    && MLDSNP_TYPE_JOIN_STATIC  != port_info.join_type)
            { /*(A-Y) = (A-X-Y) + (A*X), because (A-X-Y) already been process when it can't get port info from om, so here only process(A*X) */
                if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_status)
                {
                    port_info.specific_query_count = 1;
                    port_info.last_report_time     = time_now;

                    /*save (A*Y)*/
                    if ((MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE == rec_type)
                            && (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC))
                    {
                        memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                        saved_src_ip_num ++;
                    }

                    if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                    {
                        continue;
                    }
                }
                else
                {
                    /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                    if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                    {
                    if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info))
                        continue;
                    }
                    /* #endif  */

                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                    port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
                    if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                    {
                        continue;
                    }

                    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, tmp_src_list_p);

                    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                    /*if port is in inlcude, we can't send leave*/
                    /*if port is in exclude, we needn't send leave*/
                    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                        && entry_info.register_port_list.nbr_of_element == 1)
                      MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
                    #endif
                }
            }
        }
    }

    MLDSNP_BD(TRACE, "delete (X-A), (Y-A) ");

    /*delete (X-A), (Y-A)*/
    {
        MLDSNP_OM_HisamEntry_T entry_info;
        UI16_T nxt_vid = vid;
        UI8_T nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

        memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

        while (TRUE  == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (nxt_vid != vid
                || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
                break;

            port_info.port_no = input_port;
            if (TRUE  == L_SORT_LST_Get(&entry_info.register_port_list, &port_info)
                && FALSE == MLDSNP_ENGINE_IsInSourceList(nxt_sip_a, src_ip_list_p, num_src) /* -A*/
                && MLDSNP_TYPE_JOIN_STATIC != port_info.join_type
               )
            {
                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                /*we shall not send to upstream to block the (X-A) (Y-A),
                  because it will become allow these (X-A) and (Y-A).
                */
                #endif
                if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap,  nxt_sip_a, input_port))
                {
                    continue;
                }
            }
        }
    }

    if (MLDSNP_TYPE_IMMEDIATE_ENABLED == imm_status
        || 0 == saved_src_ip_num)
    {
        return;
    }

    MLDSNP_BD(TRACE, "send Q(MA, A-Y)");

    /*send Q(MA, A-Y)*/
    if (TRUE == MLDSNP_QUERIER_IsQuerierRunning(vid))
    {
        MLDSNP_Timer_T *timer_p = NULL;

        if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
                vid,
                gip_ap,
                (UI8_T *)save_src_ip_list_aap,
                saved_src_ip_num,
                input_port,
                LLQI,
                MLDSNP_TIMER_ONE_TIME, &timer_p))
        {
            MLDSNP_BD(RX, "create specified query timer fail");
        }

        /*send query*/
        MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);
    }
    return;
}/*End of MLDSNP_ENGINE_ExcludeModeHandle_IS_EX_and_TO_EX*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ExcludeModeHandle_IS_IN_and_Allow
 *-------------------------------------------------------------------------
 * PURPOSE : Handle allow new sources record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 *           rec_type      - the record type
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :  current state       report       new state            action
 *                EX(X,Y)         IS_IN(A)     EX(X+A, Y-A)     (A)=MALI,
 *                EX(X,Y)         Allow(A)     EX(X+A, Y-A)     (A)=MALI
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_ExcludeModeHandle_IS_IN_and_Allow(
    UI16_T                   vid,
    UI16_T                   input_port,
    UI8_T                    *gip_ap,
    UI16_T                   num_src,
    UI8_T                    *src_ip_list_p,
    MLDSNP_TYPE_RecordType_T rec_type,
    UI8_T                    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_TYPE_ImmediateStatus_T immediate_leave_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_OM_PortInfo_T          port_info;
    UI32_T MALI     = MLDSNP_OM_GetMALI(vid);
    UI32_T time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src = 0, LLQT = 0;
    UI8_T *tmp_src_list_p = src_ip_list_p;

    MLDSNP_BD(TRACE, " ");

    /* record filter mode, Filter Timer=MALI*/
    if (FALSE == MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(vid, gip_ap, num_src, input_port, rec_type, host_src_ip))
    {
        MLDSNP_BD(RX, "change mode fail");
        return;
    }

    MLDSNP_OM_GetImmediateLeaveStatus(vid, &immediate_leave_status);
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQT);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    /*(A) = MALI
     */
    for (proc_src = 0; proc_src < num_src; proc_src ++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }
        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
            &&( FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
               || FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list)
               )
           )
        {
            MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_join_lst);
        }
        #endif

        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info)) /*not in X, Y, put at request list*/
        {
            #if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            if (TRUE == MLDSNP_ENGINE_NewAndCheckThrottle(input_port))
            {
                MLDSNP_OM_PortIncStat(input_port,vid, MLDSNP_TYPE_STAT_DROP_FILTER, TRUE);
                continue;
            }
            #endif
            /*notify unknonw to clear registered port*/
            MLDSNP_UNKNOWN_HandleJoinReport(vid, input_port, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST);

            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                    vid,
                    gip_ap,
                    tmp_src_list_p,
                    1,
                    input_port,
                    MALI,
                    MLDSNP_TIMER_ONE_TIME,
                    &port_info.src_timer_p))
                continue;

            port_info.last_report_time = time_now;
            port_info.register_time    = time_now;

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_LIST_REQUEST, &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                continue;
            }
        }
        else
        {
            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
                continue;

            if (MLDSNP_TYPE_LIST_EXCLUDE ==  port_info.list_type) /*(Y-A)*/
            {
                if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                        vid,
                        gip_ap,
                        tmp_src_list_p,
                        1,
                        input_port,
                        MALI,
                        MLDSNP_TIMER_ONE_TIME,
                        &port_info.src_timer_p))
                {
                    continue;
                }

                port_info.last_report_time     = time_now;
                port_info.specific_query_count = 0;
                port_info.list_type            = MLDSNP_TYPE_LIST_REQUEST;
            }
            else /*(X*A)*/
            {

                MLDSNP_TIMER_UpdateTimerNewTime(port_info.src_timer_p, MALI);
                port_info.last_report_time     = time_now;
                port_info.specific_query_count = 0;
            }

            if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                continue;
            }
        }

        MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap,  tmp_src_list_p);
        MLDSNP_OM_AddPortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info);
    }

    return;
}/*End of MLDSNP_ENGINE_ExcludeModeHandle_IS_IN_and_Allow*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ExcludeModeHandle_Block
 *-------------------------------------------------------------------------
 * PURPOSE : Handle block old  sources record
 * INPUT   : vid           - the input vid
 *           input_port    - record input port
 *           *gip_ap       - the group ip address
 *           num_src       - the number of src in this record
 *           src_ip_list_p - the source ip list pointer
 * RETURN  : None
 * NOTE    : current state    report       new state          action
 *           EX(X,Y)          Block(A)     EX(X+(A-Y), Y)     (A-X-Y)=Filter Timer, SendQ(MA, A-Y)
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_ExcludeModeHandle_Block(
    UI16_T  vid,
    UI16_T  input_port,
    UI8_T   *gip_ap,
    UI16_T  num_src,
    UI8_T   *src_ip_list_p,
    UI8_T    *host_src_ip)
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif
    MLDSNP_TYPE_ImmediateStatus_T imm_leave_status = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ImmediateByHostStatus_T imm_byhost_status = MLDSNP_TYPE_IMMEDIATE_BYHOST_DISABLED;
    MLDSNP_OM_PortInfo_T          port_info;
    MLDSNP_Timer_T                *timer_p = NULL;
    UI32_T filter_time = 0, time_now = SYS_TIME_GetSystemTicksBy10ms();
    UI16_T proc_src, saved_src_ip_num = 0, LLQI = 0, robustness = 0;
    UI8_T *tmp_src_list_p       = src_ip_list_p;
    UI8_T *save_src_ip_list_aap = (UI8_T *)src_ip_list_save_aa;

    MLDSNP_BD(TRACE, " ");

    memset(save_src_ip_list_aap, 0, sizeof(src_ip_list_save_aa));
    MLDSNP_OM_GetImmediateLeaveStatus(vid, &imm_leave_status);
    MLDSNP_OM_GetImmediateLeaveByHostStatus(vid, &imm_byhost_status);
    MLDSNP_OM_GetLastListenerQueryInterval(&LLQI);
    MLDSNP_OM_GetRobustnessOperValue(vid, &robustness);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    if (FALSE == MLDSNP_ENGINE_GetFileterTimerTime(vid, gip_ap, input_port, &filter_time))
        return;

    for (proc_src = 0; proc_src < num_src; proc_src ++, tmp_src_list_p += SRC_IP_LEN)
    {
        /*if src_ip can't be multiast*/
        if (0xff == tmp_src_list_p[0])
        {
            MLDSNP_BD(RX, "register src is multicast addr");
            continue;
        }

        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, tmp_src_list_p, input_port, &port_info))
        {
            MLDSNP_OM_InitialportInfo(input_port, MLDSNP_TYPE_JOIN_DYNAMIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

            if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_leave_status)
            {
                port_info.last_report_time     = time_now;
                port_info.register_time        = time_now;
                port_info.specific_query_count = 1;

                /*save A in (A-Y)*/
                if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
                {
                    memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                    saved_src_ip_num ++;
                }

                if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_SourceTimerTimeout,
                        vid,
                        gip_ap,
                        tmp_src_list_p,
                        1,
                        input_port,
                        filter_time,
                        MLDSNP_TIMER_ONE_TIME,
                        &port_info.src_timer_p))
                {
                    continue;
                }

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                /*proxy part let timeout decide, to aovide jitter jion/leave*/
                #endif
            }
            else
            {
                /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                {
                if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info))
                    continue;
                }
                /* #endif  */

                port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                     && FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, tmp_src_list_p, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
                {
                   MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(gip_ap, tmp_src_list_p, &mldsnp_engine_leave_lst);
                }
                #endif
            }

            if (FALSE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, tmp_src_list_p, port_info.list_type, &port_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
                continue;
            }
        }
        else /*(A*X + A*Y)*/
        {
            if (MLDSNP_TYPE_JOIN_STATIC ==  port_info.join_type)
            {
                continue;
            }

            if (MLDSNP_TYPE_IMMEDIATE_DISABLED == imm_leave_status)
            {
                /*(A*X) in (A-Y)*/
                if (MLDSNP_TYPE_LIST_REQUEST == port_info.list_type)
                {
                    port_info.specific_query_count = 1;

                    MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info);

                    /*save (A*X)*/
                    if (saved_src_ip_num < MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC)
                    {
                        memcpy(&save_src_ip_list_aap[saved_src_ip_num*SRC_IP_LEN], tmp_src_list_p, SRC_IP_LEN);
                        saved_src_ip_num++;
                    }
                }
            }
            else
            {
                /* #if(SYS_CPNT_MLDSNP_IMMEDIATE_LEAVE_DEPEND_ON_SUBSCRIBER == TRUE)  */
                if (MLDSNP_TYPE_IMMEDIATE_BYHOST_ENABLED == imm_byhost_status)
                {
                if (0 != MLDSNP_OM_DeletePortHostIp(vid, gip_ap, tmp_src_list_p, host_src_ip, &port_info))
                    continue;
                }
                /* #endif  */

                port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                if (FALSE == MLDSNP_OM_UpdatePortInfo(vid, gip_ap, tmp_src_list_p, &port_info))
                {
                    continue;
                }

                MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(vid, gip_ap, tmp_src_list_p, &port_info);

                #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                /*if it have port in inlcude, we can't send out leave*/
                /*if it have port in exclude, we needn't send out leave*/
                #endif
            }
        }
    }


    if (MLDSNP_TYPE_IMMEDIATE_ENABLED == imm_leave_status
            || 0 == saved_src_ip_num
            || FALSE == MLDSNP_QUERIER_IsQuerierRunning(vid))
        return;

    /*send Q(MA, A-Y)
      */

    if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_QUERIER_SpecificQueryTimeOut,
            vid,
            gip_ap,
            (UI8_T *)save_src_ip_list_aap,
            saved_src_ip_num,
            input_port,
            LLQI,
            MLDSNP_TIMER_ONE_TIME, &timer_p))
    {
        MLDSNP_BD(RX, " create specified query timer fail");
    }

    /*send query*/
    MLDSNP_QUERIER_SendSpecificQuery(vid, gip_ap, input_port, save_src_ip_list_aap, saved_src_ip_num);

    return;
}/*End of MLDSNP_ENGINE_ExcludeModeHandle_Block*/


/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_IsInSourceList
*-------------------------------------------------------------------------
* PURPOSE : check if the input src_ip is in the src_ip_list
* INPUT   :  src_ip_ap       -  the source ip want to be checked
*            src_ip_list_p   - the source ip list pointer
*            num_of_src      - the num of src in src_list
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_ENGINE_IsInSourceList(
    UI8_T  *src_ip_ap,
    UI8_T  *src_ip_list_p,
    UI16_T num_of_src)
{
    UI16_T idx;

    for (idx = 0; idx < num_of_src; idx ++, src_ip_list_p += SRC_IP_LEN)
    {
        if (!memcmp(src_ip_ap, src_ip_list_p, SRC_IP_LEN))
            return TRUE;
    }

    return FALSE;
}/*End of MLDSNP_ENGINE_IsInSourceList*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_RegisterPortJoinGroup
*------------------------------------------------------------------------------
* Purpose: this function register the port into group.
*          if the group entry doen't exist, this entry will be created first.
* INPUT  : vid           - the vlan id
*          *gip_ap       - the group ip address
*          *sip_ap       - the source ip address
*          list_type      - Exclude or Include
*          port_info_p   - the port info
* OUTPUT : None
* RETURN : TRUE - success
*          FALSE- fail
* NOTES  :
*------------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_ENGINE_RegisterPortJoinGroup(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_TYPE_ListType_T list_type,
    MLDSNP_OM_PortInfo_T *port_info_p)
{
#if 0
    MLDSNP_OM_HisamEntry_T   hisam_entry;
    UI8_T                    port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, time %lu",
                             gip_ap, sip_ap, 1, vid, port_info_p->port_no, SYS_TIME_GetSystemTicksBy10ms());

    /*if first jion port then writting chip with router port*/
    if (FALSE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &hisam_entry)
            || 0 == hisam_entry.register_port_list.nbr_of_element)
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;
        MLDSNP_OM_VlanInfo_T vlan_info;

        if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
        {
            nxt_r_port_info.port_no = 0;

            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, port_bitmap_a);
            }
        }
    }

    /*if the register port is exclude list, only write the entry with router port,
       because exculde source list won't forward to host but to router port
      */
    if (MLDSNP_TYPE_LIST_EXCLUDE != list_type
            || (NULL == sip_ap
                || 0 == memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN)))
        MLDSNP_TYPE_AddPortIntoPortBitMap(port_info_p->port_no, port_bitmap_a);

    MLDSNP_ENGINE_AddPortBitmapToChipEntry(vid, gip_ap, sip_ap, port_bitmap_a);
#endif

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, time %lu",
                             gip_ap, sip_ap, 1, vid, port_info_p->port_no, SYS_TIME_GetSystemTicksBy10ms());

    /*add to om*/
    if (FALSE  == MLDSNP_OM_AddPortInfo(vid, gip_ap, sip_ap, port_info_p))
    {
        MLDSNP_BD(TRACE, "register group fail");
        return FALSE;
    }

    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, sip_ap);

    return TRUE;
}/*End if MLDSNP_ENGINE_RegisterPortJoinGroup*/


/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_LeavePortFromGroup
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the group
* INPUT  : vid      - the vlan id
*          *gip_ap  - the group ip address
*          *sip_ap  - the source ip address
*          lport    - the logical port
* OUTPUT : None
* RETURN : TRUE    - success
*          FALSE   - fail
* NOTES  : this function won't call the chip to delete the chip entry
*          after call this function, the port info can't be used.
*------------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_ENGINE_LeavePortFromGroup(
    UI16_T               vid,
    UI8_T                *gip_ap,
    UI8_T                *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info_p)
{
    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, time %lu ", gip_ap, sip_ap, 1, vid, port_info_p->port_no, SYS_TIME_GetSystemTicksBy10ms());

    if (NULL != port_info_p->src_timer_p)
    {
        MLDSNP_TIMER_StopAndFreeTimer(&port_info_p->src_timer_p);
    }

    if (NULL != port_info_p->filter_timer_p)
    {
        MLDSNP_TIMER_StopAndFreeTimer(&port_info_p->filter_timer_p);
    }

    if (NULL != port_info_p->ver1_host_present_timer_p)
    {
        MLDSNP_TIMER_StopAndFreeTimer(&port_info_p->ver1_host_present_timer_p);
        MLDSNP_OM_DeleteV1HostPresentPort(port_info_p->port_no);
    }

    if (FALSE == MLDSNP_OM_DeletePortInfo(vid, gip_ap, sip_ap, port_info_p->port_no))
    {
        MLDSNP_BD(TRACE, "Delete port fail ");
        return FALSE;
    }
    return TRUE;
}/*End if MLDSNP_ENGINE_PortLeaveGroup*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ForwardRcvdPdu
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to forward packet to port list.
 * INPUT   : *msg_p               - the msg pointer which from received pdu
 *           *output_port_list_ap - indicate which port this packet to send to
 *           reson                - the packet is forwraded by which reason
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_ForwardRcvdPdu(
    MLDNSP_ENGINE_Msg_T *msg_p,
    UI8_T               *output_port_list_ap,
    MLDSNP_TYPE_TraceId_T              reason)
{
    L_MM_Mref_Handle_T *mref_p = NULL;
    UI16_T   i;
    BOOL_T  have_outports = FALSE;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;

    MLDSNP_BD(TX, "forward received packet");

    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, msg_p->vid, &vlan_om_info) == FALSE)
    {
        MLDSNP_BD(TX, "Failed to get vlan om info");
        return FALSE;
    }


    if (FALSE == MLDSNP_ENGINE_ConstructMref(msg_p->org_mref_p, &mref_p, (UI8_T *)msg_p->ipv6_header_p, L_STDLIB_Ntoh16(msg_p->ipv6_header_p->payload_len) + MLDSNP_TYPE_IPV6_HEADER_LEN, MLDSNP_TYPE_FORWARD))
    {
        return FALSE;
    }

    /* check whether there are ports in input portlist. If no port exist, don't forward it */
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if (output_port_list_ap[i])
        {
            have_outports = TRUE;
            break;
        }
    }

    if (have_outports == FALSE)
    {
        MLDSNP_BD(TX, "no port to send out");
        L_MM_Mref_Release(&mref_p);
        return TRUE;
    }

    /*increase statistics*/
    if (reason == MLDSNP_TYPE_TRACE_FORWARD_GENERAL_QEURY
            || reason == MLDSNP_TYPE_TRACE_SEND_GENERAL_QUERY_PER_PORT)
    {
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_GQ_SEND, output_port_list_ap);
        MLDSNP_OM_VlanIncStat(msg_p->vid, MLDSNP_TYPE_STAT_GQ_SEND, TRUE);
    }
    else if (reason == MLDSNP_TYPE_TRACE_FORWARD_GROUP_SPECIFIC_QEURY
            || reason == MLDSNP_TYPE_TRACE_SEND_GROUP_SPECIFIC_QUERY)
    {
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_SQ_SEND, output_port_list_ap);
        MLDSNP_OM_VlanIncStat(msg_p->vid, MLDSNP_TYPE_STAT_SQ_SEND, TRUE);
    }
    else if (reason == MLDSNP_TYPE_TRACE_FORWARD_V1_REPORT
             || reason == MLDSNP_TYPE_TRACE_FORWARD_V2_REPORT
             || reason == MLDSNP_TYPE_TRACE_SEND_REPORT)
    {
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_JOIN_SEND, output_port_list_ap);
        MLDSNP_OM_VlanIncStat(msg_p->vid, MLDSNP_TYPE_STAT_JOIN_SEND, TRUE);
    }
    else if (reason == MLDSNP_TYPE_TRACE_FORWARD_DONE
             || reason == MLDSNP_TYPE_TRACE_SEND_DONE)
    {
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_LEAVE_SEND, output_port_list_ap);
        MLDSNP_OM_VlanIncStat(msg_p->vid, MLDSNP_TYPE_STAT_LEAVE_SEND, TRUE);
    }

    L2MUX_MGR_SendMultiPacket(mref_p,
                              msg_p->dst_mac_a,
                              msg_p->src_mac_a,
                              MLDSNP_TYPE_IPV6_ETH_TYPE,
                              msg_p->vid,
                              L_STDLIB_Ntoh16(msg_p->ipv6_header_p->payload_len) + MLDSNP_TYPE_IPV6_HEADER_LEN,
                              output_port_list_ap,
                              vlan_om_info.dot1q_vlan_current_untagged_ports,
                              0);

    return TRUE;

}   /* End of MLDSNP_ENGINE_ForwardRcvdPdu() */

#if 0
/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_AddPortToChipEntry
*-------------------------------------------------------------------------
* PURPOSE : add one port into the chip enty
* INPUT   : vid      - the input vid
*           *gip_ap  - the group ip address
*           *sip_ap  - the number of src in this record
*           lport    - the logical port
* OUTPUT  : None
* RETURN  : None
* NOTE    : None
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_ENGINE_AddPortToChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport)
{
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);

    if (0 == lport)
        msl_pmgr_mldsnp_entry_add(0,  group_ip, source_ip, vid, 0, lport);
    else
        msl_pmgr_mldsnp_entry_add(0,  group_ip, source_ip, vid, 1, lport);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port %d", gip_ap, sip_ap, 1, vid, lport);

    return TRUE;
}/*MLDSNP_ENGINE_AddPortToChipEntry*/
#endif
/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_AddPortBitmapToChipEntry
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
static BOOL_T MLDSNP_ENGINE_AddPortBitmapToChipEntry(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    UI8_T *port_bitmap_ap)
{
    UI32_T b = 0, i = 0, j = 0;
    UI16_T out_port_a[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI8_T  group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

    #if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
    #endif

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

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_DeletePortFromChipEntry
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
static BOOL_T MLDSNP_ENGINE_DeletePortFromChipEntry(
    UI16_T  vid,
    UI8_T   *gip_ap,
    UI8_T   *sip_ap,
    UI16_T  lport)
{
    UI8_T group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

    #if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
    #endif

    if (0 == lport)
        msl_pmgr_mldsnp_entry_del(0,  group_ip, source_ip, vid, 0, lport);
    else
        msl_pmgr_mldsnp_entry_del(0,  group_ip, source_ip, vid, 1, lport);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port %d", gip_ap, sip_ap, 1, vid, lport);

    return TRUE;
}/*end of MLDSNP_ENGINE_DeletePortFromChipEntry*/

/*-------------------------------------------------------------------------
* FUNCTION NAME - MLDSNP_ENGINE_DeletePortBitMapFromChipEntry
*-------------------------------------------------------------------------
* PURPOSE : Delete one ports from the chip enty
* INPUT   : vid             - the input vid
*           *gip_ap         - the group ip address
*           *port_bitmap_ap - the port bitmap
* OUTPUT  : None
* RETURN  : None
* NOTE    : If no port to delete it will delete entry.
*           This is used for delete entry directly,
*           current for the case when port in exclude list and no router port
*-------------------------------------------------------------------------
*/
static BOOL_T MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(
    UI16_T vid,
    UI8_T  *gip_ap,
    UI8_T  *sip_ap,
    UI8_T  *port_bitmap_ap)
{
    UI32_T b = 0, i = 0, j = 0;
    UI16_T out_port_a[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    UI8_T  group_ip[GROUP_LEN] = {0}, source_ip[SRC_IP_LEN] = {0};

    memcpy(group_ip, gip_ap, GROUP_LEN);

    #if(SYS_CPNT_MLDSNP_SUPPORT_V2 == TRUE)
    if (NULL != sip_ap)
        memcpy(source_ip, sip_ap, SRC_IP_LEN);
    #endif

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

    if (0 == j)
        msl_pmgr_mldsnp_entry_remove(0, group_ip, source_ip, vid);
    else
        msl_pmgr_mldsnp_entry_del_portlist(0, group_ip, source_ip, vid, j, out_port_a);

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d, port bitmap =%02x%02x%02x%02x%02x%02x",
                             group_ip, source_ip, 1, vid, port_bitmap_ap[0], port_bitmap_ap[1], port_bitmap_ap[2],
                             port_bitmap_ap[3], port_bitmap_ap[4], port_bitmap_ap[5]);

    return TRUE;
}/*end of MLDSNP_ENGINE_DeletePortBitMapFromChipEntry*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ForwardRcvdPduToRouterPort
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to flood to router port
 * INPUT   : *msg_p               - the msg pointer which from received pdu
 *               reson                  - the flood reason
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_ForwardRcvdPduToRouterPort(
    MLDNSP_ENGINE_Msg_T   *msg_p,
    MLDSNP_TYPE_TraceId_T reason)
{
    MLDSNP_OM_RouterPortInfo_T r_port_info;
    MLDSNP_OM_VlanInfo_T       vlan_info;
    UI8_T fw_port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TX, "Forward packet to router port");

    if (FALSE == MLDSNP_OM_GetVlanInfo(msg_p->vid, &vlan_info))
    {
        return FALSE;
    }

    r_port_info.port_no = 0;
    while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
    {
        MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, fw_port_bitmap);
    }

    MLDSNP_TYPE_DeletePortFromPorBitMap(msg_p->recevied_port, fw_port_bitmap);

    if (FALSE == MLDSNP_ENGINE_ForwardRcvdPdu(msg_p, fw_port_bitmap, reason))
    {
        return FALSE;
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_ForwardRcvdPduToRouterPort*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_ProcessDone
*------------------------------------------------------------------------------
* Purpose: This function process the done message
* INPUT  :
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  :
*------------------------------------------------------------------------------*/
static BOOL_T MLDSNP_ENGINE_ProcessDone(
    MLDNSP_ENGINE_Msg_T *msg_p)
{
#if(SYS_CPNT_MLDSNP_PROXY == FALSE)
    MLDSNP_ENGINE_V1Report_T *done_p = (MLDSNP_ENGINE_V1Report_T *)msg_p->icmpv6_p;
    MLDSNP_BD(TRACE, " ");

    MLDSNP_ENGINE_ProcessOneRecord(msg_p->vid, done_p->gip_a, msg_p->recevied_port, 0, NULL,
                                   MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE, msg_p->ipv6_header_p->sip_a);
    return TRUE;
#else
    MLDSNP_ENGINE_V1Report_T *done_p = (MLDSNP_ENGINE_V1Report_T *)msg_p->icmpv6_p;
    MLDSNP_OM_RouterPortInfo_T r_info;
    MLDSNP_OM_PortInfo_T port_info;
    MLDSNP_TYPE_ImmediateStatus_T imm = MLDSNP_TYPE_IMMEDIATE_DISABLED;
    MLDSNP_TYPE_ProxyReporting_T proxy_status = MLDSNP_TYPE_PROXY_REPORTING_DISABLE;
    BOOL_T already_join = FALSE, ret = TRUE;

    MLDSNP_BD(TRACE, " ");

    MLDSNP_OM_GetImmediateLeaveStatus(msg_p->vid, &imm);
    MLDSNP_OM_GetProxyReporting(&proxy_status);

    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
    {
        if (MLDSNP_OM_GetRouterPortInfo(msg_p->vid, msg_p->recevied_port, &r_info))
        {
            return TRUE; /*mrouter port don't learn gorup*/
        }

        ret = FALSE;
        if (MLDSNP_TYPE_IMMEDIATE_ENABLED == imm
             && TRUE == MLDSNP_OM_GetPortInfo(msg_p->vid, done_p->gip_a, mldsnp_om_null_src_ip_a,
                                                 msg_p->recevied_port, &port_info))
            already_join = TRUE;
    }

    MLDSNP_ENGINE_ProcessOneRecord(msg_p->vid, done_p->gip_a, msg_p->recevied_port, 0, NULL,
                                   MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE, msg_p->ipv6_header_p->sip_a);

    if (MLDSNP_TYPE_IMMEDIATE_ENABLED == imm)
    {
        if (already_join == TRUE
            && FALSE == MLDSNP_OM_GetPortInfo(msg_p->vid, done_p->gip_a, mldsnp_om_null_src_ip_a,
                                                  msg_p->recevied_port, &port_info))
        {
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
            { /*last leave and proxy reporting enabled*/
                return TRUE;
            }
        }
        else
        {
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
            { /*not last leave and proxy reporting enabled, don't forwarded*/
                return FALSE;
            }
        }
    }
    return ret;
#endif
}/*End of MLDSNP_ENGINE_ProcessDone*/


/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_ForwardRcvdPdu
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to flood to all port
 * INPUT   : *msg_p  - the msg pointer which from received pdu
 *           reson   - the flood reason
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    : This function will flood pdu to all ports in the sam vlan
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_FloodRcvdPDU(
    MLDNSP_ENGINE_Msg_T   *msg_p,
    MLDSNP_TYPE_TraceId_T reason)
{
    UI8_T out_port_list_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TX, "Flood received packet");

    if (FALSE == MLDSNP_ENGINE_FindAllPortsInVlan(msg_p->vid, out_port_list_a))
    {
        return FALSE;
    }

    MLDSNP_TYPE_DeletePortFromPorBitMap(msg_p->recevied_port, out_port_list_a);

    if (FALSE == MLDSNP_ENGINE_ForwardRcvdPdu(msg_p, out_port_list_a, reason))
    {
        return FALSE;
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_FloodRcvdPDU*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList
 *-------------------------------------------------------------------------
 * PURPOSE : This function handle port move from include to exclude, the chip entry need to update.
 * INPUT   : vid       - vlan id
 *           gip_ap    - group ip array point
 *           sip       - src ip array point
 *           port_info_p - the port will be move
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :we shall write (v,g,s) or the data will be forwarded accroding to (v,g,0).
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info)
{
#if 0
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_RouterPortInfo_T router_port_info;
    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, leave time=%lu",
                             gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid, port_info->port_no, SYS_TIME_GetSystemTicksBy10ms());

    /*directly remove from chip, it needn't to remove router port, because when this port in include list, it has add the router port to chip*/
    MLDSNP_ENGINE_DeletePortFromChipEntry(vid, gip_ap,  sip_ap, port_info->port_no);

    if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, port_bitmap_a);
        }
    }

    MLDSNP_ENGINE_AddPortBitmapToChipEntry(vid, gip_ap, sip_ap, port_bitmap_a);
#endif

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, leave time=%lu",
                             gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid, port_info->port_no, SYS_TIME_GetSystemTicksBy10ms());

    MLDSNP_ENGINE_DeletePortFromChipEntry(vid, gip_ap,  sip_ap, port_info->port_no);

    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, sip_ap);

    return;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_MovePortFromIncludeListToRequestList
 *-------------------------------------------------------------------------
 * PURPOSE : This function handle port move from request to exclude, the chip entry need to update.
 * INPUT   : vid       - vlan id
 *           gip_ap    - group ip array point
 *           sip       - src ip array point
 *           port_info_p - the port will be move
 * OUTPUT  : None
 * RETUEN  : TRUE  - success
 *           FALSE - failure
 * NOTE    :ports in request list needn't to write to chip
 *-------------------------------------------------------------------------
 */
static void MLDSNP_ENGINE_MovePortFromIncludeListToRequestList(
    UI16_T vid,
    UI8_T *gip_ap,
    UI8_T *sip_ap,
    MLDSNP_OM_PortInfo_T *port_info)
{
    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d",
                             gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid, port_info->port_no);

    MLDSNP_ENGINE_DeletePortFromChipEntry(vid, gip_ap,  sip_ap, port_info->port_no);

    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, sip_ap);
    return;
}



#if 0
#endif
/* EXPORTED SUBPROGRAM BODIES
*/
void MLDSNP_ENGINE_ClearProxyV2AllocatedMemory()
{
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    struct L_listnode   *nn;
    struct L_list *free_lst; /*avoid comile warning*/
    MLDSNP_ENGINE_GrpSrcNode_T *g_s_node;

    free_lst = &mldsnp_engine_join_lst;
    L_LIST_LOOP(free_lst, g_s_node, nn)
    {
        L_list_delete_all_node(&g_s_node->src_lst);
    }

    L_list_delete_all_node(free_lst);

    free_lst = &mldsnp_engine_leave_lst;
    L_LIST_LOOP(free_lst, g_s_node, nn)
    {
        L_list_delete_all_node(&g_s_node->src_lst);
    }
    L_list_delete_all_node(free_lst);
#endif
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME -MLDSNP_ENGINE_UpdateAndAddChipPortBitmap
 *-------------------------------------------------------------------------
 * PURPOSE : This function calculate what ports shall be write the (v, g, s)
 *           and wether the (v,g,s) or (v,g,0) shall be removed.
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
    UI8_T  *sip_ap)
{
    MLDSNP_OM_PortInfo_T  port_info;
    MLDSNP_OM_RouterPortInfo_T r_port_info;
    MLDSNP_OM_VlanInfo_T       vlan_info;
    MLDSNP_OM_HisamEntry_T  src_entry_info, grp_entry_info;
    UI8_T port_bitmap[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %d", gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid);

    if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        return FALSE;
    }

    /*add router port*/
    r_port_info.port_no = 0;
    while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &r_port_info))
    {
        MLDSNP_TYPE_AddPortIntoPortBitMap(r_port_info.port_no, port_bitmap);
    }

    if (sip_ap != NULL
            && memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
    {
        /*handle (s,g)*/
        if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &src_entry_info))
        {
            BOOL_T chip_add_entry = FALSE;

            if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &grp_entry_info))
            {
                port_info.port_no = 0;
                while (TRUE == L_SORT_LST_Get_Next(&grp_entry_info.register_port_list, &port_info))
                {
                    MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap);
                }
            }

            port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&src_entry_info.register_port_list, &port_info))
            {
                if (MLDSNP_TYPE_LIST_INCLUDE == port_info.list_type) /*needn't check request, because already in (*,G)*/
                {
                    MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap);
                    chip_add_entry = TRUE;
                }
                else if (MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type) /*exclude list*/
                {
                    MLDSNP_TYPE_DeletePortFromPorBitMap(port_info.port_no, port_bitmap);
                    chip_add_entry = TRUE;
                }
            }

            if (chip_add_entry)
                MLDSNP_ENGINE_AddPortBitmapToChipEntry(vid, gip_ap, sip_ap, port_bitmap);
            else
            {
                msl_pmgr_mldsnp_entry_remove(0, gip_ap, sip_ap, vid);
            }
        }
        else
        {
            msl_pmgr_mldsnp_entry_remove(0, gip_ap, sip_ap, vid);
        }
    }
    else
    {
        BOOL_T has_register_port = FALSE;

        /*handle (*,g)*/
        if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &grp_entry_info))
        {
            port_info.port_no = 0;
            while (TRUE == L_SORT_LST_Get_Next(&grp_entry_info.register_port_list, &port_info))
            {
                has_register_port = TRUE;
                MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap);
            }
        }

        if (has_register_port)
            MLDSNP_ENGINE_AddPortBitmapToChipEntry(vid, gip_ap, mldsnp_om_null_src_ip_a, port_bitmap);
        else
            msl_pmgr_mldsnp_entry_remove(0, gip_ap, mldsnp_om_null_src_ip_a, vid);
    }

    return TRUE;
}

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
    UI16_T lport)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_OM_PortInfo_T      port_info;
    UI16_T nxt_vid = vid;
    UI8_T nxt_sip_a[SRC_IP_LEN] = {0}, nxt_gip_a[GROUP_LEN] = {0};

    MLDSNP_BD_SHOW_GROUP(TRACE, "vid %d lport %d", gip_ap, vid, lport);

    memcpy(nxt_gip_a, gip_ap, GROUP_LEN);

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (vid != nxt_vid
                || memcmp(nxt_gip_a, gip_ap, GROUP_LEN))
        {
            break;
        }

        port_info.port_no = lport;
        if (FALSE ==  L_SORT_LST_Get(&entry_info.register_port_list, &port_info))
        {
            continue;
        }

        if (MLDSNP_TYPE_LIST_EXCLUDE == port_info.list_type)
        {
            MLDSNP_ENGINE_PortLeaveGroup(vid, nxt_gip_a, nxt_sip_a, lport);
        }
        else
        {
            port_info.list_type = MLDSNP_TYPE_LIST_INCLUDE;
            MLDSNP_OM_UpdatePortInfoFromHisam(&entry_info, &port_info);
        }
    }

    if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap, NULL, lport))
    {
        return FALSE;
    }

    return TRUE;
}/* End of MLDSNP_ENGINE_ChangeModeToIncludeMode*/


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
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status)
{
    MLDSNP_TYPE_QuerierStatus_T querier_status;
    BOOL_T ret = FALSE;

    MLDSNP_BD(TRACE, "mld_status=%d", mldsnp_status);

    ret = MLDSNP_OM_SetMldStatus(mldsnp_status);

    MLDSNP_OM_GetQuerierStauts(&querier_status);

    if (MLDSNP_TYPE_MLDSNP_DISABLED == mldsnp_status)
    {
        MLDSNP_OM_RouterPortInfo_T r_port_info;
        UI32_T next_vid = 0;
        UI16_T next_rt_port = 0;
        MLDSNP_OM_VlanInfo_T vlan_info;

        MLDSNP_ENGINE_DeleteAllEntry();

        /*delete dynamic router port from om*/
        while (TRUE == VLAN_OM_GetNextVlanId(0, &next_vid))
        {
            next_rt_port = 0;
            while (TRUE  == MLDSNP_OM_GetNextRouterPortInfo(next_vid, &next_rt_port, &r_port_info))
            {
                if (MLDSNP_TYPE_JOIN_STATIC == r_port_info.attribute)
                {
                    continue;
                }

                MLDSNP_TIMER_StopAndFreeTimer(&r_port_info.router_timer_p);
                MLDSNP_OM_DeleteRouterPort(next_vid, &r_port_info);
                msl_pmgr_mldsnp_rtport_del(0, next_vid, r_port_info.port_no);
            }

            if (TRUE == MLDSNP_OM_GetVlanInfo(next_vid, &vlan_info))
            {
                MLDSNP_TIMER_StopAndFreeTimer(&vlan_info.other_querier_present_timer_p);
                vlan_info.other_querier_uptime = 0;
                vlan_info.querier_uptime = 0;
                MLDSNP_OM_SetVlanInfo(&vlan_info);
            }
        }

        if (MLDSNP_TYPE_QUERIER_ENABLED == querier_status)
        {
            ret = MLDSNP_QUERIER_StopQuerier();
        }

        MLDSNP_QUERIER_SetMrdSolicitationStatus(FALSE);

    }
    else /*join all static entry*/
    {
        if (MLDSNP_TYPE_QUERIER_ENABLED  == querier_status)
        {
            ret = MLDSNP_QUERIER_StartQuerier();
            MLDSNP_ENGINE_RecoveryStaticJoin(0);
        }
        else
        {
            UI32_T next_vid = 0;

            while (VLAN_OM_GetNextVlanId(0, &next_vid))
            {
                MLDSNP_ENGINE_RecoveryStaticJoin(next_vid);
            }
        }
        MLDSNP_QUERIER_SetMrdSolicitationStatus(TRUE);
    }

    return ret;
}/*End of MLDSNP_ENGINE_SetMLdStatus*/

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
    UI16_T    ip_ext_opt_len)
{
    MLDSNP_TYPE_MLDSNP_STATUS_T  mldsnp_status_p;
    MLDNSP_ENGINE_Msg_T          msg;
    UI32_T pdu_len = 0;
    UI8_T *pdu_p   = NULL;

    MLDSNP_BD_ARG(RX, "Process received MLD packet from port %lu vid %lu\r\n", lport, vid);

    pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (NULL == pdu_p)
    {
        MLDSNP_BD(RX, "Can't get the pdu, pdu is null");
        return FALSE;
    }

    memcpy(msg.dst_mac_a, dmac_ap, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(msg.src_mac_a, smac_ap, SYS_ADPT_MAC_ADDR_LEN);
    msg.recevied_port = lport;
    msg.vid           = vid;
    msg.pdu_len       = pkt_len;
    msg.ipv6_header_p = (MLDSNP_ENGINE_IPv6Header_T *)pdu_p;
    msg.org_mref_p = mref_handle_p;

    /*print the pdu content*/
    MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_RX, (UI8_T *)msg.ipv6_header_p, pkt_len);

    if (MLDSNP_TYPE_IPV6_PIM_HEAD == msg.ipv6_header_p->next_header)
    {
        msg.pim6_header_p = L_CVRT_GET_PTR(pdu_p, ip_ext_opt_len + 40)/*ip header fix length*/;
        msg.pim_len = L_STDLIB_Ntoh16(msg.ipv6_header_p->payload_len) - ip_ext_opt_len;

        /* if receive pim hello message and mldsnp enabled, set as mrouter port */
        if (TRUE == MLDSNP_OM_GetMldStatus(&mldsnp_status_p)
            && MLDSNP_TYPE_MLDSNP_ENABLED == mldsnp_status_p)
        {
            if (FALSE == MLDSNP_ENGINE_ProcessPimHello(&msg))
            {
                return FALSE;
            }
            return TRUE;
        }
        else
            return TRUE;
    }
    else
    {
        msg.icmpv6_p = L_CVRT_GET_PTR(pdu_p, ip_ext_opt_len + 40)/*ip header fix length*/;
        msg.icmp_len = L_STDLIB_Ntoh16(msg.ipv6_header_p->payload_len) - ip_ext_opt_len;
    }

#if 0  /*zhimin, 20160622*/
    {
      UI8_T reserved[SYS_ADPT_IPV6_ADDR_LEN] ={0xff, 0x02, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00};
      if((0== memcmp(msg.ipv6_header_p->dip_a, reserved, SYS_ADPT_IPV6_ADDR_LEN-1))
         &&(msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]!=0x1) /*query*/
         &&(msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]!=0x2) /*done*/
         &&(msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]!=0x16) /*v2 report*/
        )
      {
#if (SYS_CPNT_MLDSNP_RESERVE_ADDRESS_PACKET_CHIP_TRAP_TO_CPU_BUT_NOT_FORWARD == TRUE)
        UI8_T portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};

        MLDSNP_ENGINE_FindAllPortsInVlan(vid, portlist);
        MLDSNP_TYPE_DeletePortFromPorBitMap(lport, portlist);
        MLDSNP_ENGINE_ForwardRcvdPdu(&msg, portlist, MLDSNP_TYPE_TRACE_UNKNOWN_PDU);
#endif
        return TRUE;
      }
    }
#endif
    if (TRUE == MLDSNP_OM_GetMldStatus(&mldsnp_status_p)
            && MLDSNP_TYPE_MLDSNP_DISABLED == mldsnp_status_p)
    {
        MLDSNP_BD(RX, "MLDSNP Disable, Drop Packet ");
        #if (SYS_CPNT_MLDSNP_RESERVE_ADDRESS_PACKET_CHIP_TRAP_TO_CPU_BUT_NOT_FORWARD == TRUE)
        /*maybe chip flood it self, here received pdu just wrong trap, we neen't to flood again, or it will cause duplicate packet*/
        if (msg.ipv6_header_p->next_header == 0) /*rule can't identify MLD control packet when hop-by-hop is inserted*/
            MLDSNP_ENGINE_FloodRcvdPDU(&msg, MLDSNP_TYPE_TRACE_UNKNOWN_PDU);
        #endif
        return FALSE;
    }

    /*verify ip+icmp pdu, don't verify the tcp, upd and other type, we take them as data*/
    if (FALSE == MLDSNP_ENGINE_VerifyPdu(&msg))
    {
        /*MLDSNP_ENGINE_FloodRcvdPDU(&msg, MLDSNP_TYPE_TRACE_UNKNOWN_PDU);*/
        return FALSE;
    }

    /*zhimin, 20160622
     *According to RFC4291, 2.7.Multicast Address
        |   8    |  4 |  4 |                  112 bits                   |
        +------ -+----+----+---------------------------------------------+
        |11111111|flgs|scop|                  group ID                   |
        +--------+----+----+---------------------------------------------+

        binary 11111111 at the start of the address identifies the address
        as being a multicast address.

                                     +-+-+-+-+
        flgs is a set of 4 flags:    |0|R|P|T|
                                     +-+-+-+-+

        The high-order flag is reserved, and must be initialized to 0.

         T = 0 indicates a permanently-assigned ("well-known") multicast
         address, assigned by the Internet Assigned Numbers Authority
         (IANA).

         T = 1 indicates a non-permanently-assigned ("transient" or
         "dynamically" assigned) multicast address.
	 */
    {
        UI8_T reserved[SYS_ADPT_IPV6_ADDR_LEN] ={0xff, 0x02, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00,
                                                 0x00, 0x00, 0x00, 0x00};
        if((0== memcmp(msg.ipv6_header_p->dip_a, reserved, SYS_ADPT_IPV6_ADDR_LEN-1))
           &&((msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]==0x1) /*query*/
           ||(msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]==0x2) /*done*/
           ||(msg.ipv6_header_p->dip_a[SYS_ADPT_IPV6_ADDR_LEN-1]==0x16) /*v2 report*/)
          )
        {
            if(msg.icmp_len != 0)/*it is mldsnp query/done/v2 report packet*/
            {

                goto dispatchPdu;
            }
        }

        /* According to RFC4291, 2.7.Multicast Address
         * Nodes must not originate a packet to a multicast address whose scop
         * field contains the reserved value 0; if such a packet is received, it
         * must be silently dropped.  Nodes should not originate a packet to a
         * multicast address whose scop field contains the reserved value F; if
         * such a packet is sent or received, it must be treated the same as
         * packets destined to a global (scop E) multicast address.
         */
        if((msg.ipv6_header_p->dip_a[1]&0x0f) == 0)
        {
            /* scop field contains the reserved value 0, if such a packet is received, it
             * must be silently dropped.  */
            return TRUE;
        }

        /* it's an UDP or other data, and it isn't mldsnp query/done/v2 report packet*/
        if((msg.ipv6_header_p->dip_a[1]&0x10) == 0)
        {
          /* T flags in flgs is 0,  it's well-known multicast packet, should be flood*/
            UI8_T portlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]={0};

            MLDSNP_ENGINE_FindAllPortsInVlan(vid, portlist);
            MLDSNP_TYPE_DeletePortFromPorBitMap(lport, portlist);
            MLDSNP_ENGINE_ForwardRcvdPdu(&msg, portlist, MLDSNP_TYPE_TRACE_WELLKNOWN_PDU);
            return TRUE;
        }
    }
    /*zhimin, 20160622, end*/

dispatchPdu:

    return MLDSNP_ENGINE_DispatchPdu(&msg);
}/*End of MLDSNP_ENIGNE_ProcessRcvdPdu*/


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
    UI8_T *gip_ap)
{
    /*UI16_T  i = 0;*/

    /*check first and second octect shall be 0xff*/
    if (gip_ap[0] != 0xff)
    {
        MLDSNP_BD(TRACE, "not ff start address ");
        return FALSE;
    }

    if (gip_ap[1] == 0x00
        ||gip_ap[1] == 0x02)
    {
        MLDSNP_BD(TRACE, "ff02 or ff00 start address ");
        return FALSE;
    }
#if 0
    /*check last byte is 02, done,  or 01, query,  or 016, report, */
    for (i = 2; i < GROUP_LEN - 1; i++) /*check 2-14 byte, left 15 not check*/
    {
        if (0 != gip_ap[i])
            break;
    }

    if (i == (GROUP_LEN - 1)) /* i stop at last byte*/
    {
        if ((gip_ap[i] == 0
                || gip_ap[i] == 0x01
                || gip_ap[i] == 0x02
                || gip_ap[i] == 0x16
            )
                && gip_ap[1] == 0x02)
        {
            MLDSNP_BD(TRACE, "control packet last byte is %d ", gip_ap[i]);
            return FALSE;
        }
    }
#endif
    return TRUE;
}/*End of MLDSNP_ENGINE_IsLegalGroupIP*/

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
    MLDSNP_TYPE_RecordType_T rec_type)
{
    MLDSNP_OM_PortInfo_T port_info;
    BOOL_T ret = FALSE;

    MLDSNP_BD(TRACE, " ");

    if (TRUE == MLDSNP_ENGINE_IsExcludeMode(vid, gip_ap, lport))
    {
        /*This port is in Exclude mode
         */
        if (MLDSNP_TYPE_IS_INCLUDE_MODE == rec_type)
        { /*join include, put into request list*/
            if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
            {
                MLDSNP_OM_InitialportInfo(lport, MLDSNP_TYPE_JOIN_STATIC, MLDSNP_TYPE_LIST_REQUEST, SYS_TIME_GetSystemTicksBy10ms(), &port_info);
                return MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, sip_ap, MLDSNP_TYPE_LIST_REQUEST, &port_info);
            }
            else
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                port_info.join_type = MLDSNP_TYPE_JOIN_STATIC;
                port_info.list_type = MLDSNP_TYPE_LIST_REQUEST;
                return MLDSNP_OM_UpdatePortInfo(vid, gip_ap, sip_ap, &port_info);
            }
        }
        else/*MLDSNP_TYPE_IS_INCLUDE_MODE*/
        {/*join exluce, put into exclude list*/
            if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
            {
                MLDSNP_OM_InitialportInfo(lport, MLDSNP_TYPE_JOIN_STATIC, MLDSNP_TYPE_LIST_EXCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);
                return MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, sip_ap, MLDSNP_TYPE_LIST_EXCLUDE, &port_info);
            }
            else
            {
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                port_info.join_type = MLDSNP_TYPE_JOIN_STATIC;
                port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
                ret = MLDSNP_OM_UpdatePortInfo(vid, gip_ap, sip_ap, &port_info);
                if (ret)
                    MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(vid, gip_ap, sip_ap, &port_info);
                return ret;
            }
        }
    }
    else
    {
        /*This port is in Include mode
          */
        if (MLDSNP_TYPE_IS_EXCLUDE_MODE == rec_type)
        { /*change to exclude mode and add port into exclude list*/
            UI16_T num_src;

            if (NULL == sip_ap
                    || memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN) == 0)
                num_src = 0;
            else
                num_src = 1;

            if (FALSE == MLDSNP_ENGINE_ChgToExModeOrUpdtFilterTimer(vid, gip_ap, num_src, lport, rec_type, mldsnp_om_null_src_ip_a))
            {
                MLDSNP_BD(TRACE, " change to Exclude mode fail");
                return FALSE;
            }

            if (num_src)
            {
                if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
                {
                    MLDSNP_OM_InitialportInfo(lport, MLDSNP_TYPE_JOIN_STATIC, MLDSNP_TYPE_LIST_EXCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

                    if (TRUE == MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, sip_ap, MLDSNP_TYPE_LIST_EXCLUDE, &port_info))
                    {
                        /*because we register the static src ip, so we remove the filter_timer from timer list*/
                        MLDSNP_OM_PortInfo_T tmp_port_info;

                        if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, NULL, lport, &tmp_port_info))
                        {
                            MLDSNP_BD(TRACE, "already change to exclude mode but can't get (vid, gip, *) port info");
                            return FALSE;
                        }

                        MLDSNP_TIMER_StopAndFreeTimer(&tmp_port_info.filter_timer_p);
                        MLDSNP_OM_UpdatePortInfo(vid, gip_ap, NULL, &tmp_port_info);
                    }
                    else
                        return FALSE;
                }
                else
                {
                    /*this port manbe dynamic or static,
                       so remove the port timer from timer list and change to join static attribute
                      */
                    MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                    port_info.join_type = MLDSNP_TYPE_JOIN_STATIC;
                    port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
                    ret = MLDSNP_OM_UpdatePortInfo(vid, gip_ap, sip_ap, &port_info);

                    if (ret)
                        MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(vid, gip_ap, sip_ap, &port_info);

                    return ret;
                }
            }
            else
            {
                /*because we register the static src ip, so we remove the filter_timer from timer list*/
                MLDSNP_OM_PortInfo_T tmp_port_info;

                if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, NULL, lport, &tmp_port_info))
                {
                    MLDSNP_BD(TRACE, "already change to exclude mode but can't get (vid, gip, *) port info");
                    return FALSE;
                }

                tmp_port_info.join_type = MLDSNP_TYPE_JOIN_STATIC;
                MLDSNP_TIMER_StopAndFreeTimer(&tmp_port_info.filter_timer_p);

                MLDSNP_OM_UpdatePortInfo(vid, gip_ap, NULL, &tmp_port_info);
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
                {
                    MLDSNP_OM_HisamEntry_T entry_info;
                    MLDSNP_TYPE_ProxyReporting_T proxy_status;
                    UI16_T ver;

                    MLDSNP_OM_GetProxyReporting(&proxy_status);
                    if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                            && MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, NULL, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info)
                            && entry_info.register_port_list.nbr_of_element == 1)
                    {
                        MLDSNP_OM_GetMldSnpVer(&ver);
                        if (MLDSNP_TYPE_VERSION_1 == ver)
                        {
                            MLDSNP_ENGINE_ProxySendV1Report(vid, gip_ap);
                        }
                        else
                        {
                            MLDSNP_ENGINE_ProxySendV2Report(vid, gip_ap, NULL, 0, MLDSNP_TYPE_CHANGE_TO_EXCLUDE_MODE);
                        }
                    }
                }
#endif
            }
        }
        else /*MLDSNP_TYPE_IS_INCLUDE_MODE*/
        {
            if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
            {
                MLDSNP_OM_InitialportInfo(lport, MLDSNP_TYPE_JOIN_STATIC, MLDSNP_TYPE_LIST_INCLUDE, SYS_TIME_GetSystemTicksBy10ms(), &port_info);

                return MLDSNP_ENGINE_RegisterPortJoinGroup(vid, gip_ap, sip_ap, MLDSNP_TYPE_LIST_INCLUDE, &port_info);
            }
            else
            {
                /*this port manbe dynamic or static,
                   so remove the port timer from timer list and change to join static attribute
                  */
                MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);

                port_info.join_type = MLDSNP_TYPE_JOIN_STATIC;
                port_info.list_type = MLDSNP_TYPE_LIST_INCLUDE;

                return MLDSNP_OM_UpdatePortInfo(vid, gip_ap, sip_ap, &port_info);
            }
        }
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_AddPortStaticJoinGroup*/

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
    UI16_T  lport)
{
    MLDSNP_OM_PortInfo_T port_info;

    MLDSNP_BD(TRACE, " ");

    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
        return TRUE; /*don't exist, return true to say it delete success becuase there is no entry left*/

    if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(vid, gip_ap, sip_ap, lport))
        return FALSE;

    return TRUE;
}/*End of MLDSNP_ENGINE_DeletePortStaticJoinGroup*/


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
    UI32_T  to_ifindex)
{
    MLDSNP_TYPE_RecordType_T out_rec_type;
    MLDSNP_OM_RouterPortInfo_T r_port_info;
    UI32_T next_vid = 0;
    UI16_T next_rt_port = 0;
    UI16_T next_id = 0, out_vid, out_lport;
    UI8_T out_gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0}, out_sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0};

    while (TRUE == VLAN_OM_GetNextVlanId(0, &next_vid))
    {
        next_rt_port = 0;
        while (TRUE  == MLDSNP_OM_GetNextRouterPortInfo(next_vid, &next_rt_port, &r_port_info))
        {
            if(r_port_info.port_no == from_ifindex
                && r_port_info.attribute == MLDSNP_TYPE_JOIN_STATIC)
            {
                MLDSNP_QUERIER_AddStaticRouterPort(next_vid, to_ifindex);
            }
        }
    }

    while (TRUE == MLDSNP_OM_GetNextStaticPortJoinGroup(&next_id, &out_vid, out_gip_a, out_sip_a, &out_lport, &out_rec_type))
    {
        if (out_lport == from_ifindex)
        {
            MLDSNP_ENGINE_AddPortStaticJoinGroup(out_vid, out_gip_a, out_sip_a, to_ifindex, out_rec_type);
            MLDSNP_OM_ReplaceStaticJoinGroup(next_id, from_ifindex, to_ifindex);
        }
    }

    return TRUE;
}/* End of MLDSNP_ENGINE_CarryConfigBetweenPort*/

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
    UI32_T  trunk_ifindex)
{
    MLDSNP_TYPE_RecordType_T out_rec_type;
    MLDSNP_OM_RouterPortInfo_T r_port_info;
    UI32_T next_vid = 0;
    UI16_T next_rt_port = 0;
    UI16_T next_id = 0, out_vid, out_lport;
    UI8_T out_gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0}, out_sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0};

    while (TRUE == VLAN_OM_GetNextVlanId(0, &next_vid))
    {
        next_rt_port = 0;
        while (TRUE  == MLDSNP_OM_GetNextRouterPortInfo(next_vid, &next_rt_port, &r_port_info))
        {
            if(r_port_info.port_no == trunk_ifindex
                && r_port_info.attribute == MLDSNP_TYPE_JOIN_STATIC)
            {
                MLDSNP_QUERIER_AddStaticRouterPort(next_vid, member_ifindex);
            }
        }
    }

    while (TRUE == MLDSNP_OM_GetNextStaticPortJoinGroup(&next_id, &out_vid, out_gip_a, out_sip_a, &out_lport, &out_rec_type))
    {
        if (out_lport == trunk_ifindex)
        {
            /* if there is enough empty entry to save the static group, register the static group to the port */
            if (TRUE == MLDSNP_OM_AddStaticPortJoinGroup(out_vid, out_gip_a, out_sip_a, member_ifindex, out_rec_type))
            {
                MLDSNP_ENGINE_AddPortStaticJoinGroup(out_vid, out_gip_a, out_sip_a, member_ifindex, out_rec_type);
            }
        }
    }
    return TRUE;
}/* End of MLDSNP_ENGINE_InheritConfigFromTrunkToPort*/


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
    UI8_T *linear_portlist_ap)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T    vlan_info;

    if (FALSE == VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info))
    {
        MLDSNP_BD(ERR, "Get vlan entry fail");
        return FALSE;
    }

    memcpy(linear_portlist_ap, vlan_info.dot1q_vlan_current_egress_ports, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    return TRUE;
}   /* End of MLDSNP_ENGINE_FindAllPortsInVlan() */


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
    UI16_T lport)
{
#if 0
    MLDSNP_OM_PortInfo_T   port_info_p;
    MLDSNP_OM_HisamEntry_T entry_info;
    UI8_T port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    BOOL_T has_router_port = FALSE;

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, leave time=%lu",
                             gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid, lport, SYS_TIME_GetSystemTicksBy10ms());

    /*check has port register but not router port
      */
    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        if (entry_info.register_port_list.nbr_of_element <= 1)
        {/*this port is this group entry's last member , remove all router port*/
            MLDSNP_OM_VlanInfo_T vlan_info;

            if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
            {
                MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;

                nxt_r_port_info.port_no = 0;
                while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
                {
                    MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, port_bitmap_a);
                    has_router_port = TRUE;
                }
            }
        }
        else if (TRUE == MLDSNP_OM_IsRouterPort(vid, lport))
        {/*this port is not this group entry's last member and this port is router port so that don't remove this router port from chip*/
            /*remove port from om
             */
            if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info_p))
                return TRUE;

            MLDSNP_ENGINE_LeavePortFromGroup(vid, gip_ap, sip_ap, &port_info_p);

            return TRUE;
        }
    }

    /*remove port from om
     */
    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info_p))
        return TRUE;

    /*delet om*/
    MLDSNP_ENGINE_LeavePortFromGroup(vid, gip_ap, sip_ap, &port_info_p);

    /*Delete port from chip
      */
    if (port_info_p.list_type == MLDSNP_TYPE_LIST_EXCLUDE
            && (NULL != sip_ap
                && memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN)
               )
       )
    {/*when port already in exclude list, chip entry won't have this port in forward egress portlist.
       we needn't to add this port, then chip can delete entry or if we put this port into portbitmap, this entry in chip won't be deleted.
       ref MLDSNP_ENGINE_ChangeModeToIncludeMode.
       The case the port move from include to exclude list will be handled by MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList
     */
        /*MLDSNP_TYPE_AddPortIntoPortBitMap(lport, port_bitmap_a); it needn't to do this*/
        MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_ap, sip_ap, port_bitmap_a);
    }
    else
    {
        MLDSNP_TYPE_AddPortIntoPortBitMap(lport, port_bitmap_a);
        MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_ap, sip_ap, port_bitmap_a);
    }
#endif
    MLDSNP_OM_PortInfo_T   port_info;

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid=%d, port=%d, leave time=%lu",
                             gip_ap, sip_ap, (sip_ap != NULL ? 1 : 0), vid, lport, SYS_TIME_GetSystemTicksBy10ms());

    /*remove port from om
     */
    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, sip_ap, lport, &port_info))
        return TRUE;

    /*delet om*/
    MLDSNP_ENGINE_LeavePortFromGroup(vid, gip_ap, sip_ap, &port_info);

    MLDSNP_ENGINE_DeletePortFromChipEntry(vid, gip_ap, sip_ap, lport);

    MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, gip_ap, sip_ap);

    return TRUE;
}/*End if MLDSNP_ENGINE_PortLeaveGroup*/

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
    UI16_T lport)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T  return_port_info;
    UI16_T next_vid = 0;
    UI8_T  sip_a[SRC_IP_LEN] = {0}, gip_a[SRC_IP_LEN] = {0};
    BOOL_T exit_unknown = FALSE;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T ver;
#endif

    MLDSNP_BD(TRACE, "lport %d ", lport);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
    MLDSNP_OM_GetMldSnpVer(&ver);
#endif

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        return_port_info.port_no = lport;
        if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &return_port_info))
        {
            if (MLDSNP_TYPE_JOIN_STATIC == return_port_info.join_type)
            {
                continue;
            }
            else if (MLDSNP_TYPE_JOIN_UNKNOWN == return_port_info.join_type)
            {
                exit_unknown = TRUE;
            }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                 && hisam_entry.register_port_list.nbr_of_element == 1)
            {
                if (MLDSNP_TYPE_VERSION_1 == ver)
                    MLDSNP_ENGINE_ProxySendV1Done(next_vid, gip_a);
                else if (MLDSNP_TYPE_VERSION_2 == ver)
                    MLDSNP_ENGINE_ProxySendV2Report(next_vid, gip_a, NULL, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
            }
#endif

            if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(next_vid, gip_a, sip_a, lport))
            {
                MLDSNP_BD(TRACE, "port leave group fail");
            }
        }

        if (TRUE == exit_unknown)
            MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(next_vid, gip_a, sip_a);
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_PortLeaveAllGroup*/


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
    UI16_T vid)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    MLDSNP_OM_VlanInfo_T   vlan_info;
    UI16_T next_vid = vid;
    UI8_T  sip_a[SRC_IP_LEN] = {0}, gip_a[SRC_IP_LEN] = {0};
    UI8_T  port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  router_port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    BOOL_T have_static_join = FALSE, have_port_join=FALSE;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
#endif

    MLDSNP_BD(TRACE, " vid %d", vid);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
#endif

    /*get all router port*/
    if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;

        nxt_r_port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, router_port_bitmap_a);
        }
    }

    /*get (vid, gip, sip) stop the timer */
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        if (next_vid != vid)
        {
            break;
        }

        have_port_join = FALSE;
        have_static_join = FALSE;
        /*delete the timer of each port*/
        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&hisam_entry.register_port_list, &port_info))
        {
            if(port_info.join_type == MLDSNP_TYPE_JOIN_STATIC
               ||port_info.join_type == MLDSNP_TYPE_JOIN_UNKNOWN)
            {
                have_static_join = TRUE;
                continue;
            }

            have_port_join = TRUE;
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.ver1_host_present_timer_p);
            MLDSNP_OM_DeleteV1HostPresentPort(port_info.port_no);
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap_a);
            L_list_delete_all_node(&port_info.host_sip_lst);
            L_SORT_LST_Delete(&hisam_entry.register_port_list, &port_info);
            #if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            MLDSNP_OM_PortDynamicGroupCountSubtractOne(port_info.port_no);
            #endif
            MLDSNP_OM_PortIncStat(port_info.port_no, next_vid, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
        }

        /*write to chip*/
        if(have_static_join == TRUE)
        {
            if(have_port_join == TRUE)
            {
                MLDSNP_OM_SetHisamEntryInfo(&hisam_entry);
                MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_a, sip_a, port_bitmap_a);
            }
        }
        else
        {
          #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
          if(proxy_status == VAL_mldSnoopProxyReporting_enabled)
              MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(hisam_entry.gip_a,
                                                    hisam_entry.sip_a,
                                                    &mldsnp_engine_leave_lst);
          #endif

          /*add router port bitmap*/
          MLDSNP_ENGINE_OrTwoPortList(router_port_bitmap_a, port_bitmap_a);
          MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_a, sip_a, port_bitmap_a);
          MLDSNP_OM_DeleteHisamEntryInfo(&hisam_entry);
        }
    }

    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    if(proxy_status == VAL_mldSnoopProxyReporting_enabled)
    {
        MLDSNP_ENGINE_ProcessSendStoreList(vid);
        MLDSNP_ENGINE_ClearProxyV2AllocatedMemory();
    }
    #endif
    return TRUE;
}/*End of MLDSNP_ENGINE_DeleteAllGroupInVlan*/

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_PortLeaveVlanAllGroup
*------------------------------------------------------------------------------
* Purpose: This function process the port leave the all group in the input vlan
* INPUT  : vid      - the vlan id
*          port     - the logical port
* OUTPUT : TRUE - success
*          FALSE- fail
* RETURN : None
* NOTES  : will delete all the dynamic, static, and unknown group entry
*------------------------------------------------------------------------------*/
BOOL_T MLDSNP_ENGINE_PortLeaveVlanAllGroup(
    UI16_T vid,
    UI16_T lport)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    UI16_T next_vid = vid;
    UI8_T  sip_a[SRC_IP_LEN] = {0}, gip_a[SRC_IP_LEN] = {0};
    BOOL_T exit_unknown = FALSE;
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    UI16_T ver;
#endif

    MLDSNP_BD(TRACE, "vid %d, lport %d ", vid , lport);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetProxyReporting(&proxy_status);
    MLDSNP_OM_GetMldSnpVer(&ver);
#endif

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        if (next_vid != vid)
        {
            /*hisam entry jump out the vid range, so break*/
            break;
        }

        port_info.port_no = lport;
        if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &port_info))
        {
            /* when the port is remove from this vlan, also delete the static group */
            #if 0
            if (MLDSNP_TYPE_JOIN_STATIC == port_info.join_type)
            {
                continue;
            }
            #endif
            if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type)
            {
                exit_unknown = TRUE;
            }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
            if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status
                 && hisam_entry.register_port_list.nbr_of_element == 1)
            {
                if (MLDSNP_TYPE_VERSION_1 == ver)
                    MLDSNP_ENGINE_ProxySendV1Done(next_vid, gip_a);
                else if (MLDSNP_TYPE_VERSION_2 == ver)
                    MLDSNP_ENGINE_ProxySendV2Report(next_vid, gip_a, NULL, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
            }
#endif

            if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(vid, gip_a, sip_a, lport))
            {
                MLDSNP_BD(TRACE, " ");
            }
        }

        if (TRUE == exit_unknown)
            MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(next_vid, gip_a, sip_a);
    }

    /* delete the static goup joined on that port in the vlan*/
    MLDSNP_OM_DeleteAllStaticPortJoinGroupInVlan(vid, lport);

    return TRUE;
}/*End of MLDSNP_ENGINE_PortLeaveVlanAllGroup*/

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
    UI16_T vid)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    MLDSNP_OM_PortInfo_T   port_info;
    MLDSNP_OM_VlanInfo_T   vlan_info;
    UI16_T next_vid = vid, tmp_vid = 0;
    UI8_T  sip_a[SRC_IP_LEN] = {0}, gip_a[SRC_IP_LEN] = {0};
    UI8_T  port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    UI8_T  router_port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, " vid %d", vid);

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {
        MLDSNP_TYPE_ProxyReporting_T proxy_status;
        MLDSNP_OM_GetProxyReporting(&proxy_status);

        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE == proxy_status)
        {
          MLDSNP_ENGINE_ProxyLeaveAllGroup(vid);
        }
    }
#endif

    /*get all router port*/
    if (TRUE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;

        nxt_r_port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
        {
            MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, router_port_bitmap_a);
        }
    }

    /*add router port bitmap*/
    memcpy(port_bitmap_a, router_port_bitmap_a, sizeof(router_port_bitmap_a));

    /*get (vid, gip, 0)*/
    if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &hisam_entry))
    {
        /*delete the timer of each port*/
        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&hisam_entry.register_port_list, &port_info))
        {
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.ver1_host_present_timer_p);
            MLDSNP_OM_DeleteV1HostPresentPort(port_info.port_no);
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap_a);
            L_list_delete_all_node(&port_info.host_sip_lst);
            #if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            MLDSNP_OM_PortDynamicGroupCountSubtractOne(port_info.port_no);
            #endif
            MLDSNP_OM_PortIncStat(port_info.port_no, 0, MLDSNP_TYPE_STAT_GRROUPS, FALSE);

        }

        /*write to chip*/
        MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_a, sip_a, port_bitmap_a);
    }

    memcpy(port_bitmap_a, router_port_bitmap_a, sizeof(router_port_bitmap_a));
    /*get (vid, gip, sip) stop the timer */
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        if (next_vid != vid)
        {
            break;
        }

        /*delete the timer of each port*/
        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&hisam_entry.register_port_list, &port_info))
        {
            if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type
                    && next_vid != tmp_vid /*make sure this vlan only search unknown one times*/)
            {
                tmp_vid = next_vid;
                MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp(next_vid, port_info.port_no, port_bitmap_a);
            }

            MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.ver1_host_present_timer_p);
            MLDSNP_OM_DeleteV1HostPresentPort(port_info.port_no);
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap_a);
            L_list_delete_all_node(&port_info.host_sip_lst);
            MLDSNP_OM_PortIncStat(port_info.port_no, 0, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
            #if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            MLDSNP_OM_PortDynamicGroupCountSubtractOne(port_info.port_no);
            #endif
        }

        /*write to chip*/
        MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(vid, gip_a, sip_a, port_bitmap_a);
        /*add router port bitmap*/
        memcpy(port_bitmap_a, router_port_bitmap_a, sizeof(router_port_bitmap_a));
    }

    /*remove the entries*/
    MLDSNP_OM_RemoveAllGroupsInVlan(vid);

    return TRUE;
}/*End of MLDSNP_ENGINE_DeleteAllGroupInVlan*/

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
BOOL_T MLDSNP_ENGINE_DeleteAllEntry()
{
    MLDSNP_OM_HisamEntry_T        hisam_entry;
    MLDSNP_OM_PortInfo_T          port_info;
    MLDSNP_OM_VlanInfo_T          vlan_info;
    UI16_T next_vid = 0, un_tmp_vid = 0, re_tmp_vid = 0;
    UI8_T  sip_a[SRC_IP_LEN] = {0}, gip_a[SRC_IP_LEN] = {0};
    UI8_T  port_bitmap_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, " ");

    /*stop the timer*/
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
            &hisam_entry))
    {
        /*get all router port*/
        if (next_vid != re_tmp_vid/*make sure this vlan only search unknown one times*/
                && TRUE == MLDSNP_OM_GetVlanInfo(next_vid, &vlan_info))
        {
            MLDSNP_OM_RouterPortInfo_T nxt_r_port_info;

            re_tmp_vid              = next_vid;
            nxt_r_port_info.port_no = 0;

            while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &nxt_r_port_info))
            {
                MLDSNP_TYPE_AddPortIntoPortBitMap(nxt_r_port_info.port_no, port_bitmap_a);
            }
        }


        port_info.port_no = 0;
        while (TRUE == L_SORT_LST_Get_Next(&hisam_entry.register_port_list, &port_info))
        {
            if (MLDSNP_TYPE_JOIN_UNKNOWN == port_info.join_type
                    && next_vid != un_tmp_vid /*make sure this vlan only search unknown one times*/)
            {
                un_tmp_vid = next_vid;
                MLDSNP_UNKNOWN_GetUnknownForwardPortbitmp(next_vid, port_info.port_no, port_bitmap_a);
            }

            MLDSNP_TIMER_StopAndFreeTimer(&port_info.src_timer_p);
            MLDSNP_TIMER_StopAndFreeTimer(&port_info.ver1_host_present_timer_p);
            MLDSNP_OM_DeleteV1HostPresentPort(port_info.port_no);
            MLDSNP_TYPE_AddPortIntoPortBitMap(port_info.port_no, port_bitmap_a);
            L_list_delete_all_node(&port_info.host_sip_lst);
            #if ( SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
            MLDSNP_OM_PortDynamicGroupCountSubtractOne(port_info.port_no);
            #endif
            MLDSNP_OM_PortIncStat(port_info.port_no, 0, MLDSNP_TYPE_STAT_GRROUPS, FALSE);
        }

        MLDSNP_ENGINE_DeletePortBitMapFromChipEntry(next_vid, gip_a, sip_a, port_bitmap_a);
    }

    /*remove the entries*/
    MLDSNP_OM_RemoveAllHisamEntry();

    return TRUE;
}/*End of MLDSNP_ENGINE_DeleteAllGroupInVlan*/

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
    UI16_T vid)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_BD(TRACE, " vid %d ", vid);

    MLDSNP_OM_InitVlanInfo(vid, &vlan_info);

    if (FALSE == MLDSNP_OM_SetVlanInfo(&vlan_info))
        return FALSE;

    return TRUE;
}/*End of MLDSNP_ENIGNE_VlanCreated*/

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
    UI16_T vid)
{
    MLDSNP_OM_VlanInfo_T vlan_info;

    MLDSNP_BD(TRACE, "vid %d ", vid);

    if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        MLDSNP_BD(TRACE, " ");
        return FALSE;
    }

    MLDSNP_TIMER_StopAndFreeTimer(&vlan_info.other_querier_present_timer_p);

    if (FALSE == MLDSNP_OM_DeleteVlanInfo(&vlan_info))
    {
        MLDSNP_BD(TRACE, " ");
        return FALSE;
    }

    if (FALSE == MLDSNP_ENGINE_DeleteAllGroupInVlan(vid))
    {
        MLDSNP_BD(TRACE, " ");
        return FALSE;
    }

    /* delete the static goup joined in the vlan*/
    MLDSNP_OM_DeleteAllStaticJoinGroupInVlan(vid);

    return TRUE;
}/*End of MLDSNP_ENIGNE_VlanDestroy*/


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
    UI16_T input_port)
{
    MLDSNP_OM_PortInfo_T port_info;

    if (FALSE == MLDSNP_OM_GetPortInfo(vid, gip_ap, NULL, input_port, &port_info))
    {
        return FALSE;
    }

    /*identifer MLDSNP_ENGINE_FilterTimerTimeout, because v1 port timer will associate another function*/
    if (NULL != port_info.filter_timer_p)
        return TRUE;

    return FALSE;
}/*End of MLDSNP_ENGINE_IsExcludeMode*/

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
    void * para_p)
{
    MLDSNP_TIMER_TimerPara_T    *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)para_p;
    MLDSNP_TYPE_QuerierStatus_T q_running_status = MLDSNP_TYPE_QUERIER_DISABLED;
    MLDSNP_OM_PortInfo_T        port_info;
    UI16_T robust_value = 0;
    BOOL_T ret = TRUE;
    UI8_T tmp_gip_a[SYS_ADPT_IPV6_ADDR_LEN] = {0};
#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    UI8_T  next_gip_a[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI8_T  next_src_a[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    UI16_T tmp_vid, next_vid;

    tmp_vid = next_vid = timer_para_p->vid;
    IPV6_ADDR_COPY(next_gip_a, timer_para_p->gip_a);
#endif
    IPV6_ADDR_COPY(tmp_gip_a, timer_para_p->gip_a);

    MLDSNP_BD_SHOW_GROUP(TRACE, "vid=%d, lport=%d, time at=%lu ", timer_para_p->gip_a, timer_para_p->vid, timer_para_p->lport, SYS_TIME_GetSystemTicksBy10ms());

    /*if querier is not running, keep run this timer. So, the outside will look like dosn't timeout
       becuase we not runing querier, so the ageout shall through received query count
      */
    MLDSNP_OM_GetQuerierRunningStatus(timer_para_p->vid, &q_running_status);
    MLDSNP_OM_GetRobustnessOperValue(timer_para_p->vid, &robust_value);

    if (FALSE == MLDSNP_OM_GetPortInfo(timer_para_p->vid, timer_para_p->gip_a, NULL, timer_para_p->lport, &port_info))
    {
        MLDSNP_BD(TRACE, "can't get port info");
        ret = FALSE;
        goto EXIT;
    }
#if 0
    /*don't wait g count reach robust value, let it has its owner timer*/
    if (MLDSNP_TYPE_QUERIER_DISABLED == q_running_status
        && port_info.specific_query_count  < robust_value)  /*when specific_query_count is the same as robustness value, it means received enough query(g, g-s)*/
    {
        MLDSNP_TIMER_StartTimer(timer_para_p->self_p);
        return TRUE;
    }
#endif
    if (FALSE == MLDSNP_ENGINE_ChangeModeToIncludeMode(timer_para_p->vid, tmp_gip_a, timer_para_p->lport))
    {
        MLDSNP_BD(TRACE, "change mode to include mode fail");
        ret = FALSE;
        goto EXIT;
    }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    {
        MLDSNP_TYPE_ProxyReporting_T proxy_status = MLDSNP_TYPE_PROXY_REPORTING_DISABLE;
        MLDSNP_OM_HisamEntry_T entry;
        UI16_T ver;

        /*NOTE: timer_para_p alrady been freed in MLDSNP_ENGINE_ChangeModeToIncludeMode
          you can't use it.
         */

        MLDSNP_OM_GetMldSnpVer(&ver);
        MLDSNP_OM_GetProxyReporting(&proxy_status);

        if (MLDSNP_TYPE_PROXY_REPORTING_ENABLE != proxy_status)
            goto EXIT;

        if (MLDSNP_TYPE_VERSION_2 == ver)
        {
            /*here next_src_a is null*/
            if (FALSE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, next_gip_a, next_src_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry))
            {
                MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, mldsnp_om_null_src_ip_a, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
            }
            else
            {
                if (tmp_vid != next_vid
                     || !IPV6_ADDR_SAME(next_gip_a, tmp_gip_a))
                {
                    MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, mldsnp_om_null_src_ip_a, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
                    goto EXIT;
                }

                /*same vid and group, checking src
                 */
                MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(entry.gip_a, entry.sip_a, &mldsnp_engine_join_lst);
                IPV6_ADDR_COPY(next_src_a, entry.sip_a);

                while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, next_gip_a, next_src_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry))
                {
                    if (tmp_vid != next_vid
                            || !IPV6_ADDR_SAME(next_gip_a, tmp_gip_a))
                        break;

                    MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(entry.gip_a, entry.sip_a, &mldsnp_engine_join_lst);
                }

                MLDSNP_ENGINE_ProcessSendStoreList(tmp_vid);
            }
        }
        else
        {/*ver 1*/
            MLDSNP_ENGINE_ProxySendV1Done(tmp_vid, tmp_gip_a);
        }
    }
#endif

EXIT:
    return ret;
}/*End of MLDSNP_ENGINE_FilterTimerTimeout*/

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
    void * para_p)
{
    MLDSNP_TIMER_TimerPara_T    *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)para_p;
    MLDSNP_TYPE_QuerierStatus_T q_running_status = MLDSNP_TYPE_QUERIER_DISABLED;
    MLDSNP_OM_PortInfo_T        port_info;
    UI16_T robust_value = 0;
    UI8_T tmp_gip_a[GROUP_LEN], tmp_sip_a[SRC_IP_LEN];
    UI16_T tmp_vid;
    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_TYPE_ProxyReporting_T proxy_status;
    MLDSNP_OM_HisamEntry_T entry;
    UI16_T next_vid;
    UI8_T nxt_gip_a[GROUP_LEN], nxt_sip_a[SRC_IP_LEN];
    #endif

    tmp_vid= timer_para_p->vid;
    IPV6_ADDR_COPY(tmp_gip_a, timer_para_p->gip_a);
    IPV6_ADDR_COPY(tmp_sip_a, timer_para_p->sip_list_a);
    #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    next_vid = timer_para_p->vid;
    IPV6_ADDR_COPY(nxt_gip_a, timer_para_p->gip_a);
    #endif

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "timer=%lu, vid=%d, lport =%d",
                             timer_para_p->gip_a, timer_para_p->sip_list_a, 1, SYS_TIME_GetSystemTicksBy10ms(), timer_para_p->vid, timer_para_p->lport);

    /*if querier is not running, keep run this timer. So, the outside will look like dosn't timeout
       becuase we not runing querier, so the ageout shall through received query count
      */
    MLDSNP_OM_GetQuerierRunningStatus(timer_para_p->vid, &q_running_status);
    MLDSNP_OM_GetRobustnessOperValue(timer_para_p->vid, &robust_value);

    if (FALSE == MLDSNP_OM_GetPortInfo(timer_para_p->vid, timer_para_p->gip_a,  timer_para_p->sip_list_a, timer_para_p->lport, &port_info))
    {
        return TRUE;
    }
#if 0
    if (MLDSNP_TYPE_QUERIER_DISABLED == q_running_status
            && port_info.specific_query_count  < robust_value)  /*when specific_query_count is the same as robustness value, it means received enough query(g, g-s)*/
    {
        MLDSNP_TIMER_StartTimer(timer_para_p->self_p);
        return TRUE;
    }
#endif
    if (FALSE == MLDSNP_ENGINE_IsExcludeMode(timer_para_p->vid, timer_para_p->gip_a, timer_para_p->lport))
    {
        MLDSNP_ENGINE_PortLeaveGroup(timer_para_p->vid, timer_para_p->gip_a,  timer_para_p->sip_list_a, timer_para_p->lport);

        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        {
          MLDSNP_OM_GetProxyReporting(&proxy_status);

          if (proxy_status == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
          {
            if (FALSE == MLDSNP_OM_GetHisamEntryInfo(tmp_vid, tmp_gip_a, tmp_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry))
            {
                IPV6_ADDR_SET(nxt_sip_a);
                if(FALSE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, nxt_gip_a, nxt_sip_a,MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry))
                { /*not (S), send TO_IN{}*/
                  MLDSNP_BD_SHOW_GROUP(TRACE, "vid %u Send TO_IN{}", tmp_gip_a, tmp_vid);
                  MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, mldsnp_om_null_src_ip_a, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
                }
                else if(next_vid != tmp_vid
                        || !IPV6_ADDR_SAME(tmp_gip_a, nxt_gip_a))
                { /*not (S), send TO_IN{}*/
                  MLDSNP_BD_SHOW_GROUP(TRACE, "vid %u Send TO_IN{}", tmp_gip_a, NULL, 0, tmp_vid);
                  MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, mldsnp_om_null_src_ip_a, 0, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE);
                }
                else
                {
                  MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "vid %u Send block{S}", tmp_gip_a, tmp_sip_a, 1, tmp_vid);
                  MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, tmp_sip_a, 1, MLDSNP_TYPE_BLOCK_OLD_SOURCES);
                }
            }
          }
        }
        #endif
    }
    else
    {
        MLDSNP_TIMER_FreeTimer(&port_info.src_timer_p);
        port_info.list_type = MLDSNP_TYPE_LIST_EXCLUDE;
        MLDSNP_OM_UpdatePortInfo(tmp_vid, tmp_gip_a, tmp_sip_a, &port_info);

        MLDSNP_ENGINE_MovePortFromIncludeListToExcludeList(tmp_vid, tmp_gip_a, tmp_sip_a, &port_info);

        #if(SYS_CPNT_MLDSNP_PROXY == TRUE)
        {
          MLDSNP_OM_GetProxyReporting(&proxy_status);

          if (proxy_status == MLDSNP_TYPE_PROXY_REPORTING_ENABLE)
          {
            MLDSNP_OM_HisamEntry_T entry;
            if (TRUE == MLDSNP_OM_GetHisamEntryInfo(tmp_vid, tmp_gip_a, tmp_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry))
            {
                if (FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry.register_port_list))
                {
                  MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "Vid %u Send block (S)", tmp_gip_a, tmp_sip_a, 1, tmp_vid);
                  MLDSNP_ENGINE_ProxySendV2Report(tmp_vid, tmp_gip_a, tmp_sip_a, 1, MLDSNP_TYPE_BLOCK_OLD_SOURCES);
                }
            }
          }
        }
        #endif
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_SourceTimerTimeout*/

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
    void * para_p)
{
    MLDSNP_TIMER_TimerPara_T *timer_para_p = (MLDSNP_TIMER_TimerPara_T *)para_p;
    MLDSNP_OM_PortInfo_T port_info;

    MLDSNP_BD_SHOW_GROUP(TRACE, "vid=%d, lport =%d, time=%lu",
                         timer_para_p->gip_a, timer_para_p->vid, timer_para_p->lport, SYS_TIME_GetSystemTicksBy10ms());

    if (FALSE == MLDSNP_OM_GetPortInfo(timer_para_p->vid, timer_para_p->gip_a,  mldsnp_om_null_src_ip_a, timer_para_p->lport, &port_info))
    {
        return FALSE;
    }

    MLDSNP_TIMER_FreeTimer(&port_info.ver1_host_present_timer_p);
    MLDSNP_OM_DeleteV1HostPresentPort(port_info.port_no);
    MLDSNP_OM_UpdatePortInfo(timer_para_p->vid, timer_para_p->gip_a, mldsnp_om_null_src_ip_a, &port_info);

    return TRUE;
}/*End of MLDSNP_ENGINE_V1HostPresentTimeout*/


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
BOOL_T MLDSNP_ENGINE_DeletePort(UI16_T lport)
{
    MLDSNP_OM_HisamEntry_T hisam_entry;
    UI16_T next_vid = 0;
    UI8_T  nxt_gip_a[GROUP_LEN] = {0}, nxt_sip_a[SRC_IP_LEN] = {0};

    MLDSNP_BD(TRACE, "lport %d ", lport);

    {/*remove group*/
        MLDSNP_OM_PortInfo_T  return_port_info;
        BOOL_T exit_unknown = FALSE;

        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid, nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC,
                &hisam_entry))
        {
            return_port_info.port_no = lport;
            if (TRUE == L_SORT_LST_Get(&hisam_entry.register_port_list, &return_port_info))
            {
                if (MLDSNP_TYPE_JOIN_UNKNOWN == return_port_info.join_type)
                {
                    exit_unknown = TRUE;
                }

                if (FALSE == MLDSNP_ENGINE_PortLeaveGroup(next_vid, nxt_gip_a, nxt_sip_a, lport))
                {
                    MLDSNP_BD(TRACE, "port leave group fail");
                }
            }

            if (TRUE == exit_unknown)
                MLDSNP_UNKNOWN_DeleteAllUnknownExtraJoinedPort(next_vid, nxt_gip_a, nxt_sip_a);
        }

        /* delete the static goup joined on that port */
        MLDSNP_OM_DeleteAllStaticPortJoinGroup(lport);
    }

    {/*remove router port*/
        UI32_T nxt_vid = 0;
        MLDSNP_OM_RouterPortInfo_T router_port_info;

        while (TRUE == VLAN_OM_GetNextVlanId(0, &nxt_vid))
        {
            if (FALSE == MLDSNP_OM_GetRouterPortInfo(nxt_vid, lport, &router_port_info))
            {
                continue;
            }

            /*delete from chip*/
            {
                UI16_T next_vid = nxt_vid;

                while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&next_vid,  nxt_gip_a, nxt_sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &hisam_entry))
                {
                    if (next_vid != nxt_vid)
                        break;

                    msl_pmgr_mldsnp_entry_del(0,  nxt_gip_a, nxt_sip_a, next_vid, 1, lport);
                }
            }

            router_port_info.port_no = lport;

            if (FALSE == MLDSNP_OM_DeleteRouterPort(nxt_vid, &router_port_info))
            {
                continue;
            }

            MLDSNP_TIMER_StopAndFreeTimer(&router_port_info.router_timer_p);

            {/*check this vid can learn group or not, if can't learn group anymore remove all group
                */
                MLDSNP_TYPE_QuerierStatus_T querier_status;
                MLDSNP_OM_VlanInfo_T        vlan_info;

                MLDSNP_OM_GetQuerierStauts(&querier_status);
                if (FALSE == MLDSNP_OM_GetVlanInfo(nxt_vid, &vlan_info))
                {
                    continue;
                }

                if ((0 == vlan_info.router_port_list.nbr_of_element)
                        && (MLDSNP_TYPE_QUERIER_DISABLED == querier_status)
                        && (FALSE == MLDSNP_OM_IsMrouteEnabled()))
                {
                    MLDSNP_ENGINE_DeleteAllGroupInVlan(nxt_vid);
                }
            }

            msl_pmgr_mldsnp_rtport_del(0, nxt_vid, lport);
        }
    }

    return TRUE;
}/*End of MLDSNP_ENGINE_DeletePort*/


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
BOOL_T MLDSNP_ENGINE_RecoveryStaticJoin(UI16_T vid)
{
    UI16_T next_id = 0, out_vid, out_lport;
    UI8_T out_gip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0}, out_sip_a[MLDSNP_TYPE_IPV6_SRC_IP_LEN] = {0};
    MLDSNP_TYPE_RecordType_T out_rec_type;

    while (TRUE == MLDSNP_OM_GetNextStaticPortJoinGroup(&next_id, &out_vid, out_gip_a, out_sip_a, &out_lport, &out_rec_type))
    {
        if ((vid == out_vid /*firt router port*/
                && MLDSNP_OM_GetVlanRouterPortCount(vid) == 1)
                || (0 == vid /*only querier enable and no router port exsit*/
                    && MLDSNP_OM_GetVlanRouterPortCount(out_vid) == 0
                   )
           )
            MLDSNP_ENGINE_AddPortStaticJoinGroup(out_vid, out_gip_a, out_sip_a, out_lport, out_rec_type);
    }

    return TRUE;
}/* End of MLDSNP_ENGINE_RecoveryStaticJoin*/


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
BOOL_T MLDSNP_ENGINE_IsVlanActive(UI16_T vid)
{
    MLDSNP_TYPE_QuerierStatus_T querier_status;
    MLDSNP_TYPE_MLDSNP_STATUS_T mldsnp_status;

    MLDSNP_OM_GetMldStatus(&mldsnp_status);
    MLDSNP_OM_GetQuerierStauts(&querier_status);

    if (MLDSNP_TYPE_MLDSNP_ENABLED == mldsnp_status
            && (
                MLDSNP_OM_GetVlanRouterPortCount(vid) > 0
                || MLDSNP_TYPE_QUERIER_ENABLED == querier_status
                || MLDSNP_OM_IsMrouteEnabled()
            )
       )
        return TRUE;


    return FALSE;
}
#if (SYS_CPNT_FILTER_THROOTTLE_MLDSNP == TRUE)
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
BOOL_T MLDSNP_ENGINE_RemoveDynamicGroupbyCount(UI32_T lport, UI32_T remove_count)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    MLDSNP_OM_PortInfo_T   port_info;
    UI16_T vid = 0;
    UI8_T gip[GROUP_LEN] = {0};
    UI8_T sip[SRC_IP_LEN] = {0};

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&vid, gip, sip, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
        while (TRUE == MLDSNP_OM_GetPortInfo(vid, gip, sip, lport, &port_info))
        {
            MLDSNP_ENGINE_LeavePortFromGroup(vid, entry_info.gip_a, entry_info.sip_a, &port_info);

            MLDSNP_ENGINE_DeletePortFromChipEntry(vid, entry_info.gip_a, entry_info.sip_a, lport);

            MLDSNP_ENGINE_UpdateAndAddChipPortBitmap(vid, entry_info.gip_a, entry_info.sip_a);

            if (--remove_count == 0)
                return TRUE;
        }
    }

    return TRUE;
}


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
UI32_T MLDSNP_ENGINE_ShallFilterGroup(UI8_T *grp_ap, UI32_T lport)
{
    UI32_T mld_filter_status = VAL_mldSnoopFilterStatus_disabled, pid = 0, access_mode = VAL_mldSnoopProfileAction_permit;
    BOOL_T has_group = FALSE;

    /* IGMP filter information  */
    MLDSNP_OM_GetMldFilterStatus(&mld_filter_status);
    MLDSNP_OM_GetPortMldProfileID(lport, &pid);
    if (pid != SYS_DFLT_MLD_PROFILE_ID_NULL)
    {
        MLDSNP_OM_GetMldProfileAccessMode(pid, &access_mode);
    }

    if (mld_filter_status == VAL_igmpSnoopFilterStatus_enabled)
    {
        if (pid != SYS_DFLT_MLD_PROFILE_ID_NULL)
        {
            switch (access_mode)
            {
                case VAL_mldSnoopProfileAction_deny:
                    if (TRUE == MLDSNP_OM_IsProfileGroup(pid, grp_ap, &has_group))
                    {
                        return VAL_mldSnoopProfileAction_deny;
                    }

                    if (has_group == FALSE)
                        return VAL_mldSnoopProfileAction_deny;

                    break;

                case VAL_mldSnoopProfileAction_permit:
                    if (FALSE == MLDSNP_OM_IsProfileGroup(pid, grp_ap, &has_group))
                    {
                        if(has_group == FALSE)
                            return VAL_mldSnoopProfileAction_permit;
                         else
                            return VAL_mldSnoopProfileAction_deny;
                    }

                    break;

                case VAL_mldSnoopProfileAction_forward:
                    if (TRUE == MLDSNP_OM_IsProfileGroup(pid, grp_ap, &has_group))
                    {
                        return VAL_mldSnoopProfileAction_forward;
                    }

                    if (has_group == FALSE)
                        return VAL_mldSnoopProfileAction_forward;

                    break;

                default:
                    break;
            }
        }
    }

    return FALSE;
}

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
BOOL_T MLDSNP_ENGINE_ShallThrottleGroup(UI32_T lport)
{
    UI32_T throttling_number, current_count;

    MLDSNP_OM_GetPortMLDThrottlingNumber(lport, &throttling_number);
    MLDSNP_OM_GetPortDynamicGroupCount(lport, &current_count);

    if (throttling_number != 0 && current_count < throttling_number)
    {
        return FALSE;
    }

    return TRUE;
}
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_AddPortCounterByPortlist
 *-------------------------------------------------------------------------
 * PURPOSE : Loop each port and the port statistic
 * INPUT   : type               - which type statistics need to increase
 *           output_portlist_ap - portbitmap
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : add port statistics in port list
 *-------------------------------------------------------------------------
 */
void MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_Statistics_T type, UI8_T *output_portlist_ap)
{
    UI16_T i = 0, j = 0;

    for (;i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        /*send v1 query to this port*/
        if (output_portlist_ap[i] != 0)
        {
            for (j = 1; j <= 8; j++)
            {
                if (output_portlist_ap[i]&(0x80 >> (j - 1) % 8))
                    MLDSNP_OM_PortIncStat(i*8 + j, 0, type, TRUE);
            }
        }
    }
    return;
}

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
    UI8_T *addr_ap)
{
#if(SYS_CPNT_IPV6 == TRUE)
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI32_T                      vid_ifindex;
    /*init value*/
    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);

    rif_config.ifindex = vid_ifindex;
    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6Z;

    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetLinkLocalRifFromInterface(&rif_config))
    {
        MLDSNP_BD_SHOW_SRC(TX, "sip from linklocal", rif_config.addr.addr, 1);
        memcpy(addr_ap, rif_config.addr.addr, SRC_IP_LEN);
        return TRUE;
    }

    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;

    if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextInetRifOfInterface(&rif_config))
    {
        MLDSNP_BD_SHOW_SRC(TX, "sip from global", rif_config.addr.addr, 1);
        memcpy(addr_ap, rif_config.addr.addr, SRC_IP_LEN);
        return TRUE;
    }

    MLDSNP_BD_ARG(TRACE, "vid %d can't get the IPv6 address, use last host reported\r\n", vid);
    memset(addr_ap, 0, SRC_IP_LEN);
    return FALSE;

#else /*#if(SYS_CPNT_IPV6 == TRUE)*/
    IPV6_ADDR_SET(addr_ap);
    return TRUE;
#endif/*#if(SYS_CPNT_IPV6 == TRUE)*/
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_GetReportSrcIpv6Address
 *-------------------------------------------------------------------------
 * PURPOSE : This function get the ipv6 address
 * INPUT    : vid -the vlan id to get ip
 * OUTPUT  : *addr_ap - the ipv6 address
 * RETURN  : TRUE  - has to ipv6 address
 *           FALSE - can't get ipv6 address
 * NOTE    : If it can't get local, it will use laster reporter's
 *-------------------------------------------------------------------------
 */
BOOL_T MLDSNP_ENGINE_GetReportSrcIpv6Address(
    UI16_T vid,
    UI8_T *addr_ap)
{
    if (TRUE == MLDSNP_ENGINE_GetConfiguredIPv6Address(vid, addr_ap))
    {
        return TRUE;
    }

#if(SYS_CPNT_MLDSNP_PROXY == TRUE)
    MLDSNP_OM_GetLastReportHostAddress(vid, addr_ap);
#endif
    return TRUE;    /*if we return fail here then return, it will cause some vlan can't send out query on l2 or l3 switch.*/
}

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
BOOL_T MLDSNP_ENGINE_IsPortInIncludeMode(L_SORT_LST_List_T *port_list)
{
    MLDSNP_OM_PortInfo_T port_info;

    port_info.port_no = 0;
    while (L_SORT_LST_Get_Next(port_list, &port_info))
    {
        if (port_info.list_type != MLDSNP_TYPE_LIST_EXCLUDE)
            return TRUE;
    }

    return FALSE;
}

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
void MLDSNP_ENGINE_AddGrpSrcNodeToStoreLst(UI8_T *grp_ap, UI8_T *src_ap, struct L_list *list_p)
{
    MLDSNP_ENGINE_GrpSrcNode_T *g_s_node;
    void *p;

    MLDSNP_BD_SHOW_GROUP_SRC(TRACE, "Add to store list %s", grp_ap, src_ap, 1, \
                             (list_p == &mldsnp_engine_join_lst ? "join" : "leave"));

    if (list_p->tail != NULL)
    {
        g_s_node = (MLDSNP_ENGINE_GrpSrcNode_T *)L_GETDATA(list_p->tail);

        if (IPV6_ADDR_CMP(g_s_node->grp, grp_ap))
        {/*new group*/
            g_s_node = L_MM_Malloc(sizeof(MLDSNP_ENGINE_GrpSrcNode_T), \
                                   L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_GRP_SRC_NODE));

            if (!g_s_node)
                return;

            memset(g_s_node, 0, sizeof(MLDSNP_ENGINE_GrpSrcNode_T));
            g_s_node->src_lst.del = MLDSNP_OM_LinkListNodeFree;
            IPV6_ADDR_COPY(g_s_node->grp, grp_ap);
            L_listnode_add(list_p, (void *)(g_s_node));

            if (src_ap == NULL
                    || IPV6_ADDR_SAME(src_ap, mldsnp_om_null_src_ip_a))
                return;

            /*add source
             */
            p = L_MM_Malloc(SRC_IP_LEN, \
                            L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_GRP_SRC_NODE));
            if (!p)
                return;

            IPV6_ADDR_COPY(p, src_ap);
            L_listnode_add(&g_s_node->src_lst, (void *)(p));
        }
        else
        {/*same group, add new source*/
            if (src_ap == NULL
                    || IPV6_ADDR_SAME(src_ap, mldsnp_om_null_src_ip_a))
                return;

            p = L_MM_Malloc(SRC_IP_LEN, \
                            L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_GRP_SRC_NODE));
            if (!p)
                return;

            IPV6_ADDR_COPY(p, src_ap);
            L_listnode_add(&g_s_node->src_lst, (void *)(p));
        }
    }
    else
    {
        g_s_node = L_MM_Malloc(sizeof(MLDSNP_ENGINE_GrpSrcNode_T), \
                               L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_GRP_SRC_NODE));

        if (!g_s_node)
            return;

        memset(g_s_node, 0, sizeof(MLDSNP_ENGINE_GrpSrcNode_T));
        g_s_node->src_lst.del = MLDSNP_OM_LinkListNodeFree;
        IPV6_ADDR_COPY(g_s_node->grp, grp_ap);
        L_listnode_add(list_p, (void *)(g_s_node));

        if (src_ap == NULL
                || IPV6_ADDR_SAME(src_ap, mldsnp_om_null_src_ip_a))
            return;
        /*add source
        */
        p = L_MM_Malloc(SRC_IP_LEN, \
                        L_MM_USER_ID2(SYS_MODULE_MLDSNP, MLDSNP_TYPE_TRACE_GRP_SRC_NODE));
        if (!p)
            return;

        IPV6_ADDR_COPY(p, src_ap);
        L_listnode_add(&g_s_node->src_lst, (void *)(p));
    }
    return;
}

/*------------------------------------------------------------------------------
* Function : MLDSNP_ENGINE_ConstructIpHeader
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
static BOOL_T MLDSNP_ENGINE_ConstructIpHeader(
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
    UI8_T *linear_portlist_ap)
{
    MLDSNP_OM_VlanInfo_T vlan_info;
    MLDSNP_OM_RouterPortInfo_T router_port_info;

    if (FALSE == MLDSNP_OM_GetVlanInfo(vid, &vlan_info))
    {
        return FALSE;
    }

    router_port_info.port_no = 0;
    while (TRUE == L_SORT_LST_Get_Next(&vlan_info.router_port_list, &router_port_info))
    {
        MLDSNP_TYPE_AddPortIntoPortBitMap(router_port_info.port_no, linear_portlist_ap);
    }

    return TRUE;
}   /* End of MLDSNP_ENGINE_FindAllPortsInVlan() */

#define MLDSNP_ENGINE_SENDSTORELIST_INIT()do{ memset(frame, 0, sizeof(frame));\
                                   next_rec_p  = frame; \
                                   pdu_len     = 0; \
                                   nr_of_record = 0;}while(0)
static void MLDSNP_ENGINE_SendJoinStoreList(UI16_T vid, struct L_list *loop_grp_lst_p)
{
  MLDSNP_ENGINE_GrpSrcNode_T *g_s_node_p;
  MLDSNP_OM_HisamEntry_T entry_info;
  MLDSNP_TYPE_RecordType_T rec_type;
  struct L_listnode   *nn, *nnn;
  UI32_T pdu_len = 0, nr_of_record = 0;
  UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE], *next_rec_p;
  BOOL_T is_new_rec = FALSE;
  void *p;

  MLDSNP_BD(TRACE, "Send join list");

  next_rec_p = frame;

  L_LIST_LOOP(loop_grp_lst_p, g_s_node_p, nn) /*loop group*/
  {
      is_new_rec = TRUE;
      nr_of_record ++;

      if (g_s_node_p->src_lst.count == 0)
      {  /*loop group without source*/
          if (FALSE == MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, g_s_node_p->grp, mldsnp_om_null_src_ip_a,
                  nr_of_record, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE, TRUE))
          {
              MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
              MLDSNP_ENGINE_SENDSTORELIST_INIT();
          }
          continue;
      }

      if(MLDSNP_OM_GetHisamEntryInfo(vid, g_s_node_p->grp, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        rec_type = MLDSNP_TYPE_ALLOW_NEW_SOURCES;
      else
        rec_type = MLDSNP_TYPE_IS_INCLUDE_MODE; /*use to_in(B) will cause (A-B) send Q*/

      /*loop group with source*/
      L_LIST_LOOP(&g_s_node_p->src_lst, p, nnn) /*loop group*/
      {
          if (nr_of_record == 0)
            break; /*it shall not reach here, but for protection*/

          if (FALSE == MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, g_s_node_p->grp, p,
                  nr_of_record, rec_type, is_new_rec))
          {
              if (nnn != g_s_node_p->src_lst.tail)
              {
                  is_new_rec = FALSE;
                  continue; /*A:although allow pud size reach, we shall kepp put all src in it*/
              }

              MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
              MLDSNP_ENGINE_SENDSTORELIST_INIT();
          }
          is_new_rec = FALSE;
      }
      L_list_delete_all_node(&g_s_node_p->src_lst);
      /*A:check pdu already over limited transmit length*/
      if (pdu_len >= MLDSNP_TYPE_MAX_PAYLOAD_SIZE)
      {
        MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
        MLDSNP_ENGINE_SENDSTORELIST_INIT();
      }
  }

  L_list_delete_all_node(loop_grp_lst_p);

  if (pdu_len < MLDSNP_TYPE_MAX_PAYLOAD_SIZE && pdu_len != 0)
      MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);

  return;
}

static void MLDSNP_ENGINE_SendLeaveStoreList(UI16_T vid, struct L_list *loop_grp_lst_p)
{
  MLDSNP_ENGINE_GrpSrcNode_T *g_s_node_p;
  MLDSNP_OM_HisamEntry_T entry_info;
  MLDSNP_TYPE_RecordType_T rec_type;
  struct L_listnode   *nn, *nnn;
  UI32_T pdu_len = 0, nr_of_record = 0;
  UI16_T nxt_vid=0;
  UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE], *next_rec_p;
  UI8_T nxt_gip[GROUP_LEN], nxt_sip[SRC_IP_LEN];
  BOOL_T is_new_rec = FALSE;
  void *p;

  MLDSNP_BD(TRACE, "Send leave list");

  next_rec_p = frame;

  L_LIST_LOOP(loop_grp_lst_p, g_s_node_p, nn) /*loop group*/
  {
      is_new_rec = TRUE;
      nr_of_record ++;

      if (g_s_node_p->src_lst.count == 0)
      {  /*loop group without source*/
          if (FALSE == MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, g_s_node_p->grp, mldsnp_om_null_src_ip_a,
               nr_of_record, MLDSNP_TYPE_IS_EXCLUDE_MODE, TRUE))
          {
              MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
              MLDSNP_ENGINE_SENDSTORELIST_INIT();
          }
          continue;
      }

      if(MLDSNP_OM_GetHisamEntryInfo(vid, g_s_node_p->grp, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
      {
        rec_type = MLDSNP_TYPE_IS_EXCLUDE_MODE;

        IPV6_ADDR_COPY(nxt_gip, g_s_node_p->grp);
        IPV6_ADDR_SET(nxt_sip);

        while(TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, nxt_gip, nxt_sip, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
          if(nxt_vid != vid
             || !IPV6_ADDR_SAME(nxt_gip, g_s_node_p->grp))
            break;

            if(MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
              continue;

            /*A: fill all source, don't care over MLDSNP_TYPE_MAX_PAYLOAD_SIZE*/
            MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, g_s_node_p->grp, nxt_sip,
                                            nr_of_record, rec_type, is_new_rec);

            is_new_rec = FALSE;
        }
      }
      else
      {
        rec_type = MLDSNP_TYPE_BLOCK_OLD_SOURCES;

        /*loop group with source*/
        L_LIST_LOOP(&g_s_node_p->src_lst, p, nnn) /*loop group*/
        {
            if (nr_of_record == 0)
              break; /*it shall not reach here, but for protection*/

            /*A: fill all source, don't care over MLDSNP_TYPE_MAX_PAYLOAD_SIZE*/
            MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, g_s_node_p->grp, p,
                    nr_of_record, rec_type, is_new_rec);

            is_new_rec = FALSE;
        }
      }

      L_list_delete_all_node(&g_s_node_p->src_lst);

      /*A:check pdu already over limited transmit length*/
      if (pdu_len >= MLDSNP_TYPE_MAX_PAYLOAD_SIZE)
      {
        MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
        MLDSNP_ENGINE_SENDSTORELIST_INIT();
      }
  }

  L_list_delete_all_node(loop_grp_lst_p);

  if (pdu_len < MLDSNP_TYPE_MAX_PAYLOAD_SIZE && pdu_len != 0)
      MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);

  return;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_ProcessSendStoreList
 *-------------------------------------------------------------------------
 * PURPOSE : Process store the leave (S,G) to send report or done
 * INPUT   : vid   - which vlan
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : you shall make usre {G1,S1} won't mix with {G2, S2} to call this api
 *-------------------------------------------------------------------------
 */
void MLDSNP_ENGINE_ProcessSendStoreList(UI16_T vid)
{
    UI16_T ver;

    MLDSNP_OM_GetMldSnpVer(&ver);

    MLDSNP_BD(TRACE, "vid %u ver %u", vid, ver);

    if (MLDSNP_TYPE_VERSION_2 == ver)
    {
        /*process mldsnp join
         */
        MLDSNP_ENGINE_SendJoinStoreList(vid, &mldsnp_engine_join_lst);

        /*process mldsnp leave
         */
        MLDSNP_ENGINE_SendLeaveStoreList(vid, &mldsnp_engine_leave_lst);
    }
    else /*ver 1*/
    {
        MLDSNP_ENGINE_GrpSrcNode_T *g_s_node_p;
        struct L_listnode   *nn;
        struct L_list *loop_grp_lst_p; /*avoid comile warning*/
        UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE];

        memset(frame, 0, sizeof(frame));

        loop_grp_lst_p = &mldsnp_engine_join_lst;
        L_LIST_LOOP(loop_grp_lst_p, g_s_node_p, nn) /*loop group*/
        {
            L_list_delete_all_node(&g_s_node_p->src_lst);  /*we shall not have src list*/
            MLDSNP_ENGINE_ProxySendV1Report(vid, g_s_node_p->grp);
        }
        L_list_delete_all_node(loop_grp_lst_p);

        loop_grp_lst_p = &mldsnp_engine_leave_lst;
        L_LIST_LOOP(loop_grp_lst_p, g_s_node_p, nn) /*loop group*/
        {
            L_list_delete_all_node(&g_s_node_p->src_lst);  /*we shall not have src list*/
            MLDSNP_ENGINE_ProxySendV1Done(vid, g_s_node_p->grp);
        }
        L_list_delete_all_node(loop_grp_lst_p);

    }
    return;
}

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
BOOL_T MLDSNP_ENGINE_ProxySendV1Report(UI16_T vid, UI8_T *gip_ap)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p   = NULL;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = NULL;
    MLDSNP_ENGINE_V1Report_T   *report_p    = NULL;
    L_MM_Mref_Handle_T   *mref_p = NULL;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len = MLDSNP_TYPE_REPORT_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    UI8_T *pdu_p = NULL;
    UI8_T sip_a[SRC_IP_LEN] = {0};
    UI8_T output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};
    MLDSNP_BD(TRACE, " ");

    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_om_info) == FALSE)
    {
        MLDSNP_BD(TX, "Failed to get vlan om info");
        return FALSE;
    }

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_REPORT))))
        return FALSE;

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        L_MM_Mref_Release(&mref_p);
        return FALSE;
    }

    memset(pdu_p, 0, pdu_len);

    mld_pdu_p   = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    report_p     = (MLDSNP_ENGINE_V1Report_T *) & mld_pdu_p->icmpv6;

    MLDSNP_ENGINE_GetReportSrcIpv6Address(vid, sip_a);

    /*assign the vlaue of mld query
     */
    report_p->type = MLDSNP_TYPE_REPORT_V1;
    memcpy(report_p->gip_a, gip_ap, GROUP_LEN);

    /*assign the IPv6 header*/
    MLDSNP_ENGINE_ConstructIpHeader(ipv6_head_p, gip_ap, sip_a, MLDSNP_TYPE_REPORT_V1_LEN);

    report_p->checksum = L_STDLIB_Hton16(MLDSNP_QUERIER_GenerateCheckSum((UI8_T *) & mld_pdu_p->icmpv6, sip_a, gip_ap,
                                         MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                         MLDSNP_TYPE_REPORT_V1_LEN));
    {/*send the query pdu to driver*/
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};
        UI8_T dst_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0x33, 0x33, 0, 0, 0, 0};

        SWCTRL_GetCpuMac(src_mac_a);

        memcpy(&dst_mac_a[2], &gip_ap[GROUP_LEN-4], 4);

        MLDSNP_ENGINE_FindAllRouterPortsInVlan(vid, output_portlist_a);
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_JOIN_SEND, output_portlist_a);
        MLDSNP_OM_VlanIncStat(vid, MLDSNP_TYPE_STAT_JOIN_SEND, TRUE);
        mref_p->next_usr_id = SYS_MODULE_L2MUX;

        L2MUX_MGR_SendMultiPacket(mref_p,
                                  dst_mac_a,
                                  src_mac_a,
                                  MLDSNP_TYPE_IPV6_ETH_TYPE,
                                  vid,
                                  pdu_len,
                                  output_portlist_a,
                                  vlan_om_info.dot1q_vlan_current_untagged_ports,
                                  0);
    }
    return TRUE;
}
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
BOOL_T MLDSNP_ENGINE_ProxySendV1Done(UI16_T vid, UI8_T *gip_ap)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p   = NULL;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = NULL;
    MLDSNP_ENGINE_V1Report_T   *report_p    = NULL;
    L_MM_Mref_Handle_T   *mref_p = NULL;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len = MLDSNP_TYPE_REPORT_V1_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    UI8_T *pdu_p = NULL;
    UI8_T sip_a[SRC_IP_LEN] = {0}, dip_a[GROUP_LEN] = {0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x02};
    UI8_T output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TRACE, " ");

    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_om_info) == FALSE)
    {
        MLDSNP_BD(TX, "Failed to get vlan om info");
        return FALSE;
    }

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_REPORT))))
    {
        return FALSE;
    }
    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        L_MM_Mref_Release(&mref_p);
        return FALSE;
    }

    memset(pdu_p, 0, pdu_len);

    mld_pdu_p   = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    ipv6_head_p = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    report_p    = (MLDSNP_ENGINE_V1Report_T *) & mld_pdu_p->icmpv6;

    MLDSNP_ENGINE_GetReportSrcIpv6Address(vid, sip_a);

    /*assign the vlaue of mld query
     */
    report_p->type = MLDSNP_TYPE_DONE;
    memcpy(report_p->gip_a, gip_ap, GROUP_LEN);

    /*assign the IPv6 header*/
    MLDSNP_ENGINE_ConstructIpHeader(ipv6_head_p, dip_a, sip_a, MLDSNP_TYPE_REPORT_V1_LEN);

    report_p->checksum = L_STDLIB_Hton16(MLDSNP_QUERIER_GenerateCheckSum((UI8_T *) & mld_pdu_p->icmpv6, sip_a, dip_a,
                                         MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                         MLDSNP_TYPE_REPORT_V1_LEN));

    {/*send the query pdu to driver*/
        UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};
        UI8_T dst_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0x33, 0x33, 0, 0, 0, 0x02};

        SWCTRL_GetCpuMac(src_mac_a);

        MLDSNP_ENGINE_FindAllRouterPortsInVlan(vid, output_portlist_a);
        MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_LEAVE_SEND, output_portlist_a);
        MLDSNP_OM_VlanIncStat(vid, MLDSNP_TYPE_STAT_LEAVE_SEND, TRUE);

        mref_p->next_usr_id = SYS_MODULE_L2MUX;

        L2MUX_MGR_SendMultiPacket(mref_p,
                                  dst_mac_a,
                                  src_mac_a,
                                  MLDSNP_TYPE_IPV6_ETH_TYPE,
                                  vid,
                                  pdu_len,
                                  output_portlist_a,
                                  vlan_om_info.dot1q_vlan_current_untagged_ports,
                                  0);
    }

    return TRUE;
}
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
                                       BOOL_T new_record)
{
    MLDSNP_ENGINE_GroupRecord_T *rec_p = (MLDSNP_ENGINE_GroupRecord_T *)(*cur_rec_pp);

    if (!new_record)
    {
        IPV6_ADDR_COPY(&rec_p->sip_aa[rec_p->num_of_src], sip_ap);
        rec_p->num_of_src++;
        *pdu_len_p += SRC_IP_LEN;
    }
    else
    {
        if (num_of_rec > 1)
            /*move to next record start pointer*/
            rec_p = (MLDSNP_ENGINE_GroupRecord_T *)(*cur_rec_pp + 4/*rec_type, aux_data_len num_of_src*/ \
                                                    + GROUP_LEN + ((MLDSNP_ENGINE_GroupRecord_T *) * cur_rec_pp)->num_of_src * SRC_IP_LEN);
        else
            rec_p = (MLDSNP_ENGINE_GroupRecord_T *) * cur_rec_pp;

        *cur_rec_pp = (UI8_T *)rec_p;
        rec_p->rec_type     = rec_type;
        rec_p->aux_data_len = 0;
        rec_p->num_of_src   = 0;
        *pdu_len_p += 4;

        IPV6_ADDR_COPY(rec_p->gip_a, gip_ap);
        *pdu_len_p += GROUP_LEN;

        if (0 == memcmp(sip_ap, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
            goto EXIT;

        IPV6_ADDR_COPY(&rec_p->sip_aa[rec_p->num_of_src], sip_ap);
        rec_p->num_of_src++;
        *pdu_len_p += SRC_IP_LEN;
    }

EXIT:
    /*A:ust define a max payload length only consider group,
      it shall have more buff for src list. so set MLDSNP_TYPE_MAX_PAYLOAD_SIZE
      < MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE - MLDSNP_TYPE_MAX_NBR_OF_SRC_IP_A_REC*16 at least
    */
    if (*pdu_len_p >= MLDSNP_TYPE_MAX_PAYLOAD_SIZE)
        return FALSE;
    else
        return TRUE;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_ENGINE_PackAndSendV2Report
 *-------------------------------------------------------------------------
 * PURPOSE : Add group record to tx pdu and add report header
 * INPUT   : vid          - which vlan to send
 *           data_p       - group record poninter
 *           data_len     - group record len
 *           nr_of_record - number of group record
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 *-------------------------------------------------------------------------
 */
static BOOL_T MLDSNP_ENGINE_PackAndSendV2Report(UI16_T vid,
        UI8_T *data_p,
        UI32_T data_len,
        UI8_T nr_of_record)
{
    MLDSNP_ENGINE_MldPdu_T     *mld_pdu_p   = NULL;
    MLDSNP_ENGINE_IPv6Header_T *ipv6_head_p = NULL;
    MLDSNP_ENGINE_V2Report_T   *report_p    = NULL;
    L_MM_Mref_Handle_T   *mref_p = NULL;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_om_info;
    UI32_T pdu_len = data_len + MLDSNP_TYPE_REPORT_V2_LEN + MLDSNP_TYPE_IPV6_HEADER_LEN + MLDSNP_TYPE_HOP_BY_HOP_LEN;
    UI8_T *pdu_p;
    UI8_T sip_a[SRC_IP_LEN] = {0}, dip_a[GROUP_LEN] = {0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x16};
    UI8_T src_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T dst_mac_a[SYS_ADPT_MAC_ADDR_LEN] = {0x33, 0x33, 0, 0, 0, 0x16};
    UI8_T output_portlist_a[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST] = {0};

    MLDSNP_BD(TX, "Send V2 Report");
    MLDSNP_BACKDOOR_PrintPdu(MLDSNP_BD_FLAG_TRACE, data_p, data_len);

    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_om_info) == FALSE)
    {
        MLDSNP_BD(TX, "Failed to get vlan om info");
        return FALSE;
    }

    if (NULL == (mref_p = L_MM_AllocateTxBuffer(pdu_len,
                          L_MM_USER_ID2(SYS_MODULE_MLDSNP,
                                        MLDSNP_TYPE_TRACE_SEND_V2REPORT))))
        return FALSE;

    mref_p->current_usr_id = SYS_MODULE_MLDSNP;

    if (NULL == (pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_p, &pdu_len)))
    {
        L_MM_Mref_Release(&mref_p);
        return FALSE;
    }

    memset(pdu_p, 0, pdu_len);
    mld_pdu_p      = (MLDSNP_ENGINE_MldPdu_T *)pdu_p;
    ipv6_head_p    = (MLDSNP_ENGINE_IPv6Header_T *) & mld_pdu_p->ipv6_header;
    report_p       = (MLDSNP_ENGINE_V2Report_T *) & mld_pdu_p->icmpv6;

    report_p->type = MLDSNP_TYPE_REPORT_V2;
    report_p->num_of_group_rec = nr_of_record;
    /*copy data*/
    memcpy(report_p->rec, data_p, data_len);

    MLDSNP_ENGINE_GetReportSrcIpv6Address(vid, sip_a);
    /*assign the IPv6 header*/
    MLDSNP_ENGINE_ConstructIpHeader(ipv6_head_p, dip_a, sip_a, data_len + MLDSNP_TYPE_REPORT_V2_LEN);

    report_p->check_sum = L_STDLIB_Hton16(MLDSNP_QUERIER_GenerateCheckSum((UI8_T *) & mld_pdu_p->icmpv6, sip_a, dip_a,
                                          MLDSNP_TYPE_IPV6_ICMPV6_HAED,
                                          data_len + MLDSNP_TYPE_REPORT_V2_LEN));
    SWCTRL_GetCpuMac(src_mac_a);

    MLDSNP_ENGINE_FindAllRouterPortsInVlan(vid, output_portlist_a);
    MLDSNP_ENGINE_AddPortCounterByPortlist(MLDSNP_TYPE_STAT_JOIN_SEND, output_portlist_a);
    MLDSNP_OM_VlanIncStat(vid, MLDSNP_TYPE_STAT_JOIN_SEND, TRUE);

    mref_p->next_usr_id = SYS_MODULE_L2MUX;

    L2MUX_MGR_SendMultiPacket(mref_p,
                              dst_mac_a,
                              src_mac_a,
                              MLDSNP_TYPE_IPV6_ETH_TYPE,
                              vid,
                              pdu_len,
                              output_portlist_a,
                              vlan_om_info.dot1q_vlan_current_untagged_ports,
                              0);
    return TRUE;
}
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
BOOL_T MLDSNP_ENGINE_ProxySendV2Report(UI16_T vid,
                                       UI8_T *gip_ap,
                                       UI8_T *sip_list_ap,
                                       UI16_T num_of_src,
                                       MLDSNP_TYPE_RecordType_T rec_type)
{
    MLDSNP_ENGINE_GroupRecord_T *rec_p;
    UI32_T pdu_len = 0, i;
    UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE] = {0};

    MLDSNP_BD_ARG(TX, "Proxy Send report, record type %u", rec_type);

    rec_p = (MLDSNP_ENGINE_GroupRecord_T *)frame;

    IPV6_ADDR_COPY(rec_p->gip_a, gip_ap);
    rec_p->num_of_src = num_of_src;
    rec_p->rec_type   = rec_type;

    pdu_len = (GROUP_LEN + 4 + num_of_src * SRC_IP_LEN);

    if (pdu_len > MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE)
        return FALSE;

    for (i = 0; i < num_of_src; i++)
        IPV6_ADDR_COPY(&rec_p->sip_aa[i], sip_list_ap + (i*SRC_IP_LEN));

    MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, 1);
    return TRUE;
}

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
BOOL_T MLDSNP_ENGINE_ProxyLeaveAllGroup(UI16_T vid)
{
  MLDSNP_OM_HisamEntry_T  entry_info;
  UI16_T ver, nxt_vid = 0;
  UI8_T  gip_a[GROUP_LEN] = {0}, sip_a[SRC_IP_LEN] = {0}, tmp_gip[GROUP_LEN] = {0};

  MLDSNP_OM_GetMldSnpVer(&ver);

  if(MLDSNP_TYPE_VERSION_2 == ver)
  {
    UI32_T pdu_len = 0;
    UI8_T nr_of_record = 0;
    UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE] = {0}, *next_rec_p;

    next_rec_p = frame;

    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
      if(vid != nxt_vid)
        break;

      if(!IPV6_ADDR_SAME(tmp_gip, gip_a)
         || IPV6_ADDR_SAME(tmp_gip, mldsnp_om_null_group_ip_a)/*first*/)
      {
        nr_of_record ++;

        if (FALSE == MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, gip_a, mldsnp_om_null_src_ip_a,
                nr_of_record, MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE, TRUE))
        {
            MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
            MLDSNP_ENGINE_SENDSTORELIST_INIT();
        }
        IPV6_ADDR_COPY(tmp_gip, gip_a);
      }
    }

    if(pdu_len!=0)
      MLDSNP_ENGINE_PackAndSendV2Report(vid, frame, pdu_len, nr_of_record);
  }
  else
  {
    while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, gip_a, sip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
    {
      if(vid != nxt_vid)
        break;

      if(!IPV6_ADDR_SAME(sip_a, mldsnp_om_null_group_ip_a))
        continue;

      MLDSNP_ENGINE_ProxySendV1Done(nxt_vid, gip_a);
    }
  }

  return TRUE;
}

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
BOOL_T MLDSNP_ENGINE_ProxyReplyGSQuery(UI16_T vid, UI8_T check_gip_ap[], UI8_T *src_list_p, UI16_T num_src)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    UI8_T  gip_ap[GROUP_LEN] = {0}, sip_ap[SRC_IP_LEN] = {0};
    UI16_T ver;

    MLDSNP_BD(TX, "Proxy report for answering G-S/G-S-S query");

    MLDSNP_OM_GetMldSnpVer(&ver);

    if (ver == MLDSNP_TYPE_VERSION_1)
    {
        if (TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, check_gip_ap, mldsnp_om_null_src_ip_a, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
          IPV6_ADDR_COPY(gip_ap, check_gip_ap);
          MLDSNP_ENGINE_ProxySendV1Report(vid, gip_ap);
        }
    }
    else if (ver == MLDSNP_TYPE_VERSION_2)
    {
        UI32_T pdu_len = 0;
        UI8_T nr_of_record = 0;
        UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE] = {0}, *next_rec_p;
        BOOL_T new_rec = TRUE;

        next_rec_p = frame;

        IPV6_ADDR_COPY(gip_ap, check_gip_ap);

        if(TRUE == MLDSNP_OM_GetHisamEntryInfo(vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        { /*exclude mode*/
          MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, entry_info.gip_a, entry_info.sip_a,
                                          nr_of_record, MLDSNP_TYPE_IS_EXCLUDE_MODE, new_rec);
          new_rec = FALSE;
          while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
          { /*although specify s, but we shall answer all S so or upsream will think we only exclude s but not S*/
              if (vid != entry_info.vid
                  || !IPV6_ADDR_SAME(gip_ap, check_gip_ap))
                  break;

              if (TRUE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                  continue;
              /*A*/
              MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, entry_info.gip_a, entry_info.sip_a,
                                              nr_of_record, MLDSNP_TYPE_IS_INCLUDE_MODE, new_rec);
          }/*while*/
        }
        else /*include mode*/
        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (vid != entry_info.vid
                || !IPV6_ADDR_SAME(gip_ap, check_gip_ap))
                break;

            if(FALSE == MLDSNP_ENGINE_IsInSourceList(sip_ap, src_list_p, num_src))
              continue;

            /*only exclude mode need to check this*/
            if (FALSE == MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                continue;
            /*A*/
            MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, entry_info.gip_a, entry_info.sip_a,
                                            nr_of_record, MLDSNP_TYPE_IS_INCLUDE_MODE, new_rec);
            new_rec = FALSE;
        }/*while*/

        if (pdu_len != 0)
            MLDSNP_ENGINE_PackAndSendV2Report(entry_info.vid, frame, pdu_len, nr_of_record);

    }/*else if(ver == MLDSNP_TYPE_VERSION_2)*/

    return TRUE;
}

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
BOOL_T MLDSNP_ENGINE_ProxySendUnsolicitReports(UI16_T vid)
{
    MLDSNP_OM_HisamEntry_T entry_info;
    UI8_T  gip_ap[GROUP_LEN] = {0}, sip_ap[SRC_IP_LEN] = {0};
    UI16_T ver, nxt_vid = vid;

    MLDSNP_BD(TX, "Proxy send unsolicit report");

    MLDSNP_OM_GetMldSnpVer(&ver);

    if (ver == MLDSNP_TYPE_VERSION_1)
    {
        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if (memcmp(mldsnp_om_null_src_ip_a, entry_info.sip_a, SRC_IP_LEN))
                continue;

            if (vid != 0 && vid != entry_info.vid)
                break;

            if (vid != 0 && entry_info.vid > vid)
                break; /*skip search because vid is sorted*/

            MLDSNP_ENGINE_ProxySendV1Report(nxt_vid, gip_ap);
        }
    }
    else if (ver == MLDSNP_TYPE_VERSION_2)
    {
        /*we shall send inlcude when there are source list
          or exclude when there are no source list
         */
        MLDSNP_TYPE_RecordType_T rec_type = MLDSNP_TYPE_CHANGE_TO_INCLUDE_MODE;
        UI32_T pdu_len = 0;
        UI16_T tmp_vid = vid;
        UI8_T nr_of_record = 0;
        UI8_T frame[MLDSNP_TYPE_MAX_ALOCATE_FRAME_SIZE] = {0}, *next_rec_p;
        UI8_T tmp_gip[GROUP_LEN] = {0};

        next_rec_p = frame;

        while (TRUE == MLDSNP_OM_GetNextHisamEntryInfo(&nxt_vid, gip_ap, sip_ap, MLDSNP_TYPE_HISAM_KEY_VID_GROUP_SRC, &entry_info))
        {
            if(vid == 0 && tmp_vid!=vid)
            {
              if (pdu_len != 0)
              {
                  MLDSNP_ENGINE_PackAndSendV2Report(tmp_vid, frame, pdu_len, nr_of_record);
                  MLDSNP_ENGINE_SENDSTORELIST_INIT();
              }
            }

            if (vid != 0 && vid != entry_info.vid)
                break;

            if (vid != 0 && entry_info.vid > vid)
                break; /*skip search because vid is sorted*/

            tmp_vid = nxt_vid;

            if (memcmp(entry_info.gip_a, tmp_gip, GROUP_LEN))
            {
                if (pdu_len > MLDSNP_TYPE_MAX_PAYLOAD_SIZE && pdu_len != 0)
                {
                    MLDSNP_ENGINE_PackAndSendV2Report(entry_info.vid, frame, pdu_len, nr_of_record);
                    MLDSNP_ENGINE_SENDSTORELIST_INIT();
                }

                nr_of_record ++;
                memcpy(tmp_gip, entry_info.gip_a, GROUP_LEN);

                if (memcmp(entry_info.sip_a, mldsnp_om_null_src_ip_a, SRC_IP_LEN))
                    rec_type = MLDSNP_TYPE_IS_INCLUDE_MODE;
                else
                    rec_type = MLDSNP_TYPE_IS_EXCLUDE_MODE;

                /*A*/
                MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, entry_info.gip_a, entry_info.sip_a,
                        nr_of_record, rec_type, TRUE);
            }
            else
            {
                /*only exclude mode need to check this*/
                if (rec_type == MLDSNP_TYPE_IS_EXCLUDE_MODE
                        && MLDSNP_ENGINE_IsPortInIncludeMode(&entry_info.register_port_list))
                    continue;

                MLDSNP_ENGINE_EncodeGroupRecord(&next_rec_p, &pdu_len, entry_info.gip_a, entry_info.sip_a,
                        nr_of_record, rec_type, FALSE);
            }
        }/*while*/

        if (pdu_len < MLDSNP_TYPE_MAX_PAYLOAD_SIZE && pdu_len != 0)
            MLDSNP_ENGINE_PackAndSendV2Report(tmp_vid, frame, pdu_len, nr_of_record);

    }/*else if(ver == MLDSNP_TYPE_VERSION_2)*/

    return TRUE;
}
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
    void * para_p)
{
    MLDSNP_BD(TRACE, "Unsolicit report timeout");

    return MLDSNP_ENGINE_ProxySendUnsolicitReports(0);
}
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
BOOL_T MLDSNP_ENGINE_StartUnsolicittimer(BOOL_T enabled)
{
    MLDSNP_Timer_T *timer_p = NULL;
    UI32_T interval;

    MLDSNP_OM_GetUnsolicitTimer(&timer_p);

    if (enabled)
    {
        MLDSNP_OM_GetUnsolicitedReportInterval(&interval);

        if (NULL == timer_p)
        {
            if (FALSE == MLDSNP_TIMER_CreateAndStartTimer(MLDSNP_ENGINE_UnsolicitTimeout,
                    0, NULL, NULL, 0, 0,
                    interval, MLDSNP_TIMER_CYCLIC,
                    &timer_p))
                return FALSE;
        }
        else
        {
            if (FALSE == MLDSNP_TIMER_UpdateTimerNewTime(timer_p, interval))
                return FALSE;
        }
    }
    else
    {
        if (timer_p != NULL)
            MLDSNP_TIMER_StopAndFreeTimer(&timer_p);
    }

    MLDSNP_OM_SetUnsolicitTimer(timer_p);

    return TRUE;
}

#endif

