/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_TXRX.c
 *
 * PURPOSE: This package provides the service routines to manage VRRP packets send/receive
 *          procedeure
 * NOTES:   The key functions of this module are to provide receving and sending VRRP packets
 *          function and handle packet
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 * -------------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#include "string.h"
#include "sys_module.h"
#include "sysfun.h"
#include "vrrp_type.h"
#include "vrrp_om.h"
#include "vrrp_mgr.h"
#include "syslog_type.h"
#include "syslog_pmgr.h"
#include "syslog_task.h"
#include "sys_module.h"
#include "ip_lib.h"
#include "iml_pmgr.h"
#include "vlan_pmgr.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "l_stdlib.h"

#if (SYS_CPNT_VRRP == TRUE)

/* ----------------------------
 * NAMING CONSTANT
 * ----------------------------*/
#define VRRP_ARP_HW_TYPE     0X1
#define VRRP_ARP_PT_TYPE     0X0800
#define VRRP_ARP_HL          0X6
#define VRRP_ARP_PL          0X4
#define VRRP_ARP_REQUEST     0X1


/* ----------------------------
 * LOCAL DATATYPE DECLARATION
 * ----------------------------*/
typedef struct ETHER_HEADER_S
{
    UI8_T   ether_dhost[6];
    UI8_T   ether_shost[6];
    UI16_T 	ether_type;
} ETHER_HEADER_T;

typedef struct	ARP_HEADER_S
{
    UI16_T  hardware_type;
    UI16_T  protocol_type;
    UI8_T   hlen;
    UI8_T   plen;
    UI16_T  operation;
    UI8_T   sender_ha[6];
    UI8_T   sender_ip[VRRP_IP_ADDR_LENGTH];
    UI8_T   target_ha[6];
    UI8_T   target_ip[VRRP_IP_ADDR_LENGTH];
} ARP_HEADER_T;

/* ----------------------------
 * STATIC VARIABLE DECLARATIONS
 * ---------------------------- */
static UI16_T vrrpPktId = 0;
static BOOL_T vrrpTxrxErrorLogFlag[8];

/* ----------------------------
 * LOCAL SUBPROGRAM DECLARATION
 * ----------------------------*/
static BOOL_T VRRP_TXRX_BuildVrrpPkt(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, ETHER_HEADER_T   *eth);
static BOOL_T VRRP_TXRX_BuildEtherHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, ETHER_HEADER_T *eth);
static BOOL_T VRRP_TXRX_BuildIpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame);
static BOOL_T VRRP_TXRX_BuildVrrpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame);
static BOOL_T VRRP_TXRX_BuildAndSendGraArp(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *ipaddr_p);
static BOOL_T VRRP_TXRX_BuildArpEtherHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, ETHER_HEADER_T   *eth);
static BOOL_T VRRP_TXRX_BuildArpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, UI8_T *ipaddr_p);
static BOOL_T VRRP_TXRX_SendPkt(VRRP_OPER_ENTRY_T *vrrp_info, ETHER_HEADER_T   *eth, UI32_T pkt_len, L_MM_Mref_Handle_T  *mref_handle_p);
static BOOL_T VRRP_TXRX_GetPrimaryIp(UI32_T ifindex, UI8_T *SrcIp);
static UI16_T VRRP_TXRX_ChkSumCalculate(UI16_T *calculateDataBufferPtr, UI32_T calculateLengthInHalfWord);
static BOOL_T VRRP_TXRX_BuildArpHeaderWithVlanMac(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, UI8_T *ipaddr_p, UI8_T *vlan_mac_p);



/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_SendAdvertisement
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will send ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_TXRX_SendAdvertisement(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    UI8_T    *buffer;
    UI32_T   pkt_len = 0;
    UI32_T   pdu_len;
    L_MM_Mref_Handle_T       *mref_handle_p;
    ETHER_HEADER_T   eth;

    SYSLOG_OM_RecordOwnerInfo_T vrrpErrorLogS;

    if (vrrp_oper_entry == 0)
        return FALSE;

    /* Ether: 14, IP:20, VRRP:8, Auth_data:8, 4 is reserved for vlan tag */
    pkt_len = sizeof(VRRP_IP_PKT_HEADER_T) + sizeof(VRRP_PKT_T) +
              vrrp_oper_entry->ip_addr_count * 4 + VRRP_MGR_AUTH_DATA_LENGTH;

    mref_handle_p = L_MM_AllocateTxBufferFromDedicatedBufPool(
                        L_MM_TX_BUF_POOL_ID_VRRP_TX,
                        L_MM_USER_ID2(SYS_MODULE_VRRP,
                                      VRRP_TYPE_TRACE_ID_VRRP_SEND_ADVERTISEMENT));

    if (NULL == mref_handle_p)
    {
        VRRP_BD(DBG, "Failed to allocate mref.");
        return FALSE;
    }

    buffer = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (buffer == 0)
    {
        if (vrrpTxrxErrorLogFlag[0] != 1)
        {
            vrrpErrorLogS.level = SYSLOG_LEVEL_WARNING;
            vrrpErrorLogS.module_no = SYS_MODULE_VRRP;
            vrrpErrorLogS.function_no = VRRP_TYPE_TXRX_SEND_ADVERTISEMENT;
            vrrpErrorLogS.error_no    = VRRP_TYPE_EH_GET_PDU_ERROR;
            SYSLOG_PMGR_AddFormatMsgEntry(&vrrpErrorLogS, VRRP_TXRX_SENDADVERTISEMENT_L_MEM_ALLOCATE_ERROR_INDEX , 0, 0, 0);
            vrrpTxrxErrorLogFlag[0] = 1;
        }

        VRRP_BD(DBG, "Failed to get pdu.");
        return FALSE;
    }

    if (VRRP_TXRX_BuildVrrpPkt(vrrp_oper_entry, buffer, &eth) != TRUE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        VRRP_BD(DBG, "Failed to build vrrp packet.");
        return FALSE;
    }

    if (VRRP_TXRX_SendPkt(vrrp_oper_entry, &eth, pkt_len, mref_handle_p) != TRUE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        VRRP_BD(DBG, "Failed to send packet.");
        return FALSE;
    }

    return TRUE;
} /* VRRP_TXRX_SendAdvertisement() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_SendGratuitousArp
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will send gratuitous ARP packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_TXRX_SendGratuitousArp(VRRP_OPER_ENTRY_T *vrrp_oper_entry)
{
    VRRP_ASSOC_IP_ENTRY_T vrrp_assoc_ip_entry;
    UI8_T    ipaddr[VRRP_IP_ADDR_LENGTH];
    UI32_T   i;
    L_INET_AddrIp_T dst_addr_p;

    if (vrrp_oper_entry == 0)
        return FALSE;

    vrrp_assoc_ip_entry.ifindex = vrrp_oper_entry->ifindex;
    vrrp_assoc_ip_entry.vrid = vrrp_oper_entry->vrid;
    memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
    for (i = 0; i < vrrp_oper_entry->ip_addr_count; i++)
    {
        if (VRRP_TYPE_OK != VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry))
        {
            VRRP_BD(DBG, "Failed to get associated ip address.");
            return FALSE;

        }

        memcpy(ipaddr, vrrp_assoc_ip_entry.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);

        if (VRRP_TXRX_BuildAndSendGraArp(vrrp_oper_entry, ipaddr) != TRUE)
        {
            VRRP_BD(DBG, "Failed to build and send gratitous arp.");
            return FALSE;
        }
    }

    return TRUE;
} /* VRRP_TXRX_SendGratuitousArp */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildVrrpPkt
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct a ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildVrrpPkt(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, ETHER_HEADER_T   *eth)
{

    if ((vrrp_oper_entry == NULL) || (frame == NULL) || (eth == NULL))
        return FALSE;
    if (VRRP_TXRX_BuildEtherHeader(vrrp_oper_entry, eth) == FALSE)
    {
        VRRP_BD(DBG, "Failed to build ethernet header.");
        return FALSE;
    }

    if (VRRP_TXRX_BuildIpHeader(vrrp_oper_entry, frame) == FALSE)
    {
        VRRP_BD(DBG, "Failed to build ip header.");
        return FALSE;
    }
    frame += sizeof(VRRP_IP_PKT_HEADER_T);

    if (VRRP_TXRX_BuildVrrpHeader(vrrp_oper_entry, frame) == FALSE)
    {
        VRRP_BD(DBG, "Failed to build vrrp header.");
        return FALSE;
    }
    return TRUE;
} /* VRRP_TXRX_BuildVrrpPkt() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildAndSendGraArp
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct a ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildAndSendGraArp(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *ipaddr_p)
{
    UI8_T    *buffer;
    UI32_T   pkt_len = 0;
    UI32_T   pdu_len;
    L_MM_Mref_Handle_T       *mref_handle_p;
    ETHER_HEADER_T   eth;

    SYSLOG_OM_RecordOwnerInfo_T vrrpErrorLogS;

    if (vrrp_oper_entry == 0)
        return FALSE;

    /* Ether: 14, IP:20, VRRP:8, Auth_data:8, 4 is reserved for vlan tag */
    pkt_len = sizeof(ETHER_HEADER_T) + sizeof(ARP_HEADER_T);

    mref_handle_p = L_MM_AllocateTxBufferFromDedicatedBufPool(
                        L_MM_TX_BUF_POOL_ID_VRRP_TX,
                        L_MM_USER_ID2(SYS_MODULE_VRRP,
                                      VRRP_TYPE_TRACE_ID_VRRP_SEND_ADVERTISEMENT));

    if (NULL == mref_handle_p)
    {
        VRRP_BD(DBG, "Failed to allocate mref.");
        return FALSE;
    }

    buffer = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (buffer == 0)
    {
        if (vrrpTxrxErrorLogFlag[0] != 1)
        {
            vrrpErrorLogS.level = SYSLOG_LEVEL_WARNING;
            vrrpErrorLogS.module_no = SYS_MODULE_VRRP;
            vrrpErrorLogS.function_no = VRRP_TYPE_TXRX_BUILD_AND_SEND_GRAARP;
            vrrpErrorLogS.error_no    = VRRP_TYPE_EH_GET_PDU_ERROR;
            SYSLOG_PMGR_AddFormatMsgEntry(&vrrpErrorLogS, VRRP_TXRX_L_MREF_CONSTRUCTOR_ERROR_INDEX, 0, 0, 0);
            vrrpTxrxErrorLogFlag[0] = 1;
        }


        return FALSE;
    }

    if (VRRP_TXRX_BuildArpEtherHeader(vrrp_oper_entry, &eth) == FALSE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    if (VRRP_TXRX_BuildArpHeader(vrrp_oper_entry, buffer, ipaddr_p) == FALSE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    if (VRRP_TXRX_SendPkt(vrrp_oper_entry, &eth, pkt_len, mref_handle_p) != TRUE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }
    return TRUE;

} /* VRRP_TXRX_BuildAndSendGraArp() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildEtherHeader
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct a ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildEtherHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, ETHER_HEADER_T *eth)
{

    if ((vrrp_oper_entry == 0) || (eth == NULL))
        return FALSE;

    eth->ether_dhost[0]	= 0x01;
    eth->ether_dhost[1]	= 0x00;
    eth->ether_dhost[2]	= 0x5E;
    eth->ether_dhost[3]	= 0x00;
    eth->ether_dhost[4]	= 0x00;
    eth->ether_dhost[5]	= 0x12;
    eth->ether_shost[0]	= 0x00;
    eth->ether_shost[1]	= 0x00;
    eth->ether_shost[2]	= 0x5E;
    eth->ether_shost[3]	= 0x00;
    eth->ether_shost[4]	= 0x01;
    eth->ether_shost[5]	= (UI16_T)vrrp_oper_entry->vrid;
    eth->ether_type = VRRP_ETHER_FRAME_IP_TYPE;
    return TRUE;
} /* VRRP_TXRX_BuildEtherHeader() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildIpHeader
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct a ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildIpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame)
{
    VRRP_IP_PKT_HEADER_T *ipPktStructure = (VRRP_IP_PKT_HEADER_T*)frame;

    if ((vrrp_oper_entry == 0) || (frame == 0))
        return FALSE;

    ipPktStructure->verHlen = 0x45;
    ipPktStructure->serviceType = 0;
    ipPktStructure->totalLen = L_STDLIB_Hton16(sizeof(VRRP_IP_PKT_HEADER_T) + sizeof(VRRP_PKT_T) +
                               vrrp_oper_entry->ip_addr_count * sizeof(UI32_T) +
                               VRRP_MGR_AUTH_DATA_LENGTH);
    vrrpPktId++;
    ipPktStructure->identification = L_STDLIB_Hton16(vrrpPktId);
    ipPktStructure->flagFragOffset = 0;
    ipPktStructure->ttl = VRRP_IP_TTL;
    ipPktStructure->protocol = VRRP_PROTOCOL_NUMBER;
    ipPktStructure->headerChksum = 0;
    ipPktStructure->desIp[0] = 224;
    ipPktStructure->desIp[1] = 0;
    ipPktStructure->desIp[2] = 0;
    ipPktStructure->desIp[3] = 18;
    memcpy(ipPktStructure->srcIp, vrrp_oper_entry->primary_ip_addr, SYS_ADPT_IPV4_ADDR_LEN);

    ipPktStructure->headerChksum =
        VRRP_TXRX_ChkSumCalculate((UI16_T *)ipPktStructure, sizeof(VRRP_IP_PKT_HEADER_T) / 2);
    return TRUE;
} /* VRRP_TXRX_BuildIpHeader() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildVrrpHeader
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct a ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildVrrpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame)
{
    VRRP_PKT_T             *vrrpPkt = (VRRP_PKT_T*)frame;
    VRRP_ASSOC_IP_ENTRY_T  vrrp_assoc_ip_entry;
    UI32_T                 i = 0;
    UI8_T                 *ipaddr_p = (UI8_T *)(((UI8_T*)vrrpPkt) + sizeof(VRRP_PKT_T));
    UI8_T                 *auth_data = (UI8_T *)(((UI8_T*)vrrpPkt) + sizeof(VRRP_PKT_T) + vrrp_oper_entry->ip_addr_count * 4);

    vrrpPkt->verType = (VRRP_VERSION << 4) | VRRP_ADVER_TYPE;
    vrrpPkt->vrid = vrrp_oper_entry->vrid;
    vrrpPkt->priority = vrrp_oper_entry->priority;
    vrrpPkt->countIpAddr = vrrp_oper_entry->ip_addr_count;
    /* The authentication type in MIB is from 1~3, but it is from 0~2 in VRRP control packets */
    vrrpPkt->authType = vrrp_oper_entry->auth_type - 1;
    vrrpPkt->adverInt = vrrp_oper_entry->advertise_interval;
    vrrpPkt->checksum = 0;
    /* copy the ip addresses */
    vrrp_assoc_ip_entry.ifindex = vrrp_oper_entry->ifindex;
    vrrp_assoc_ip_entry.vrid = vrrp_oper_entry->vrid;
    /* use 0.0.0.0 to get the first ip of the specific vrrp on the interface */
    memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);

    while (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry))
    {
        memcpy(ipaddr_p, vrrp_assoc_ip_entry.assoc_ip_addr, VRRP_IP_ADDR_LENGTH);
        ipaddr_p += 4;
        i++;
    }

    if (i != vrrp_oper_entry->ip_addr_count)
    {
        VRRP_BD(DBG, "ip count[%lu] is not equal to configuration[%lu].",
                (unsigned long)i, (unsigned long)vrrp_oper_entry->ip_addr_count);
    }

    if (vrrp_oper_entry->auth_type == VAL_vrrpOperAuthType_simpleTextPassword)
    {
        memcpy(auth_data, vrrp_oper_entry->auth_key, VRRP_MGR_AUTH_DATA_LENGTH);
    }
    else
        memset(auth_data, 0, VRRP_MGR_AUTH_DATA_LENGTH);

    vrrpPkt->checksum = VRRP_TXRX_ChkSumCalculate((UI16_T *)vrrpPkt,
                        (sizeof(VRRP_PKT_T) + vrrp_oper_entry->ip_addr_count * 4 + VRRP_MGR_AUTH_DATA_LENGTH) / 2);

    return TRUE;
} /* VRRP_TXRX_BuildVrrpHeader() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildArpEtherHeader
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct the Ether header of a ARP packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildArpEtherHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, ETHER_HEADER_T   *eth)
{
    /* hardcoded for ethernet */

    if (vrrp_oper_entry == 0)
        return FALSE;

    eth->ether_dhost[0]	= 0xFF;
    eth->ether_dhost[1]	= 0xFF;
    eth->ether_dhost[2]	= 0xFF;
    eth->ether_dhost[3]	= 0xFF;
    eth->ether_dhost[4]	= 0xFF;
    eth->ether_dhost[5]	= 0xFF;

    /* use virtual mac as source mac
     */
    eth->ether_shost[0] = 0x00;
    eth->ether_shost[1] = 0x00;
    eth->ether_shost[2] = 0x5e;
    eth->ether_shost[3] = 0x00;
    eth->ether_shost[4] = 0x01;
    eth->ether_shost[5] = vrrp_oper_entry->vrid;
    eth->ether_type = VRRP_ETHER_FRAME_ARP_TYPE;
    return TRUE;
} /* VRRP_TXRX_BuildArpEtherHeader() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_BuildArpHeader
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will contruct the header of a ARP packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_BuildArpHeader(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, UI8_T *ipaddr_p)
{
    ARP_HEADER_T *arpPkt = (ARP_HEADER_T*)frame;

    if ((vrrp_oper_entry == 0) || (frame == 0) || (ipaddr_p == 0))
        return FALSE;

    arpPkt->hardware_type = L_STDLIB_Hton16(VRRP_ARP_HW_TYPE);
    arpPkt->protocol_type = L_STDLIB_Hton16(VRRP_ARP_PT_TYPE);
    arpPkt->hlen = VRRP_ARP_HL;
    arpPkt->plen = VRRP_ARP_PL;
    arpPkt->operation = L_STDLIB_Hton16(VRRP_ARP_REQUEST);
    arpPkt->sender_ha[0] = 0x00;
    arpPkt->sender_ha[1] = 0x00;
    arpPkt->sender_ha[2] = 0x5E;
    arpPkt->sender_ha[3] = 0x00;
    arpPkt->sender_ha[4] = 0x01;
    arpPkt->sender_ha[5] = vrrp_oper_entry->vrid;
    memset(arpPkt->target_ha, 0, 6);
    memcpy(arpPkt->sender_ip, ipaddr_p, VRRP_IP_ADDR_LENGTH);
    memcpy(arpPkt->target_ip, ipaddr_p, VRRP_IP_ADDR_LENGTH);
    return TRUE;
} /* VRRP_TXRX_BuildArpHeader() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TXRX_SendPkt
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will send a contructed ADVERTISEMENT packet
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static BOOL_T VRRP_TXRX_SendPkt(VRRP_OPER_ENTRY_T *vrrp_info, ETHER_HEADER_T  *eth, UI32_T pkt_len, L_MM_Mref_Handle_T  *mref_handle_p)
{
#define PRIMARY VAL_netConfigPrimaryInterface_primary

    SYSLOG_OM_RecordOwnerInfo_T vrrpErrorLogS;

    if (vrrp_info == 0)
        return FALSE;

#if 0
    mref_handle_p = L_MM_Mref_Construct(buffer, buflen, 0, buflen - sizeof(ETHER_HEADER_T) - 4,
                                        L_MM_MREF_FREE_FUN_CUSTOM,
                                        NULL);
#endif

    if (mref_handle_p == NULL)
    {
        if (vrrpTxrxErrorLogFlag[2] != 1)
        {
            vrrpErrorLogS.level = SYSLOG_LEVEL_WARNING;
            vrrpErrorLogS.module_no = SYS_MODULE_VRRP;
            vrrpErrorLogS.function_no = VRRP_TYPE_TXRX_SEND_PKT;
            vrrpErrorLogS.error_no    = VRRP_TYPE_EH_LMREF_ERROR;
            SYSLOG_PMGR_AddFormatMsgEntry(&vrrpErrorLogS, VRRP_TXRX_L_MREF_CONSTRUCTOR_ERROR_INDEX , 0, 0, 0);
            vrrpTxrxErrorLogFlag[2] = 1;
        }
        return FALSE;
    }

    if (eth->ether_type == VRRP_ETHER_FRAME_IP_TYPE)
    {
        IML_PMGR_SendPkt(mref_handle_p, vrrp_info->ifindex, pkt_len, eth->ether_dhost, eth->ether_shost, VRRP_ETHER_FRAME_IP_TYPE, FALSE);
    }

    if (eth->ether_type == VRRP_ETHER_FRAME_ARP_TYPE)
    {
        IML_PMGR_SendPkt(mref_handle_p, vrrp_info->ifindex, pkt_len, eth->ether_dhost, eth->ether_shost, VRRP_ETHER_FRAME_ARP_TYPE, FALSE);
    }

    return TRUE;
}

/* FUNCTION : VRRP_TXRX_IpChkSumCalculate
 * PURPOSE  : The function calculate vrrp checksum
 * INPUT    : *calculateDataBufferPtr     --  Data buffer which want to calculate checksum.
 *            calculateLengthInHalfWord   --  length of data buffer (in 2bytes length).
 * OUTPUT   : None.
 * RETUEN   : checksum result.
 * NOTES    : None.
 */
static UI16_T VRRP_TXRX_ChkSumCalculate(UI16_T *calculateDataBufferPtr, UI32_T calculateLengthInHalfWord)
{
    UI32_T  i, temp;
    UI16_T  checkSum;

    checkSum = *calculateDataBufferPtr;
    calculateDataBufferPtr++;
    for (i = 1; i < calculateLengthInHalfWord; i++)
    {
        temp = checkSum + *calculateDataBufferPtr;
        if (temp > 0xffff)
        {   /* carry */
            checkSum = checkSum + *calculateDataBufferPtr + 1;
        }
        else
        {
            checkSum = (UI16_T)temp;
        }
        calculateDataBufferPtr++;
    }
    checkSum = checkSum ^ 0xffff;
    return checkSum;
} /* VRRP_TXRX_ChkSumCalculate() */


/* FUNCTION : VRRP_TXRX_BuildArpHeaderWithVlanMac
 * PURPOSE  : Build the header of the ARP request. Put the Mac address of the VLAN
 *            as source mac address.
 * USES     : When the Virtual Router is the ip owner, and when it shutdown,
 *            an ARP request needs to be sent to tell the local network that the
 *            Vrrp Mac address for this DUT IP address is not valid anymore. The
 *            DUT mac address has to be use instead. This function is here to help
 *            the ARP request header.
 * PARAM.   : VRRP_OPER_ENTRY_T *vrrp_oper_entry
 *            UI8_T *frame
 *            UI8_T *ipaddr_p
 *            UI8_T *vlan_mac_p
 * OUTPUT   :
 * RETURNS  : TRUE if we succed to the operation, FALSE if the parameters are not valid
 * NOTES    : Added to solve EPR:ES3628C-PoE-20-00181
 * AUTHOR   : Aris Michael MORGENSTERN
 */
static BOOL_T VRRP_TXRX_BuildArpHeaderWithVlanMac(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI8_T *frame, UI8_T *ipaddr_p, UI8_T *vlan_mac_p)
{
    ARP_HEADER_T *arpPkt = (ARP_HEADER_T*)frame;

    if ((vrrp_oper_entry == 0) || (frame == 0) || (ipaddr_p == 0))
        return FALSE;

    arpPkt->hardware_type = VRRP_ARP_HW_TYPE;
    arpPkt->protocol_type = VRRP_ARP_PT_TYPE;
    arpPkt->hlen = VRRP_ARP_HL;
    arpPkt->plen = VRRP_ARP_PL;
    arpPkt->operation = VRRP_ARP_REQUEST;
    arpPkt->sender_ha[0] = *vlan_mac_p;
    arpPkt->sender_ha[1] = *(vlan_mac_p + 1);
    arpPkt->sender_ha[2] = *(vlan_mac_p + 2);
    arpPkt->sender_ha[3] = *(vlan_mac_p + 3);
    arpPkt->sender_ha[4] = *(vlan_mac_p + 4);
    arpPkt->sender_ha[5] = *(vlan_mac_p + 5);
    memset(arpPkt->target_ha, 0, 6);
    memcpy(arpPkt->sender_ip, ipaddr_p, VRRP_IP_ADDR_LENGTH);
    memcpy(arpPkt->target_ip, ipaddr_p, VRRP_IP_ADDR_LENGTH);
    return TRUE;

}

#endif



