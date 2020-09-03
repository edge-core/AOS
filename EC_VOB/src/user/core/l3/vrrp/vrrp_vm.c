/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP.c
 *
 * PURPOSE: This package provides the service routines to manage VRRP state machine
 * NOTES:   The key functions of this module are to provide receving VRRP packets
 *          function and handle packet
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 * -------------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */
#include <leaf_1213.h>
#include "sysfun.h"
#include "string.h"
#include "netcfg_mgr_arp.h"
#include "vlan_mgr.h"
#include "vrrp_mgr.h"
#include "vrrp_vm.h"
#include "vrrp_om.h"
#include "vrrp_txrx.h"
#include "amtrl3_mgr.h"
#include "l_prefix.h"
#include "ipal_types.h"
#include "amtrl3_pmgr.h"
#include "netcfg_pmgr_nd.h"
#include "ipal_vrrp.h"
#include "ip_lib.h"
#include "vrrp_sys_adpt.h"
#include "syslog_type.h"
#include "syslog_pmgr.h"
#include "syslog_task.h"
#include "sys_module.h"
#include "swctrl.h"
#include "leaf_4001.h"
#include "l_mm.h"

#if (SYS_CPNT_NSM == TRUE)
    #include "nsm_pmgr.h"
#endif

#if (SYS_CPNT_VRRP == TRUE)

/*
 * MACRO DEFINITION
 */

/*
 * NAMING CONSTANT
 */

#define VRRP_HEADER 1
#define IP_HEADER   2

/* LOCAL DATATYPE DECLARATION
 */
/* Willy Owner test*/

#define VrrpOperEntryAddrOwner_true	        1L
#define VrrpOperEntryAddrOwner_false	    2L
/*end owner test code*/

/* MACRO FUNCTIONS DECLARACTION
 */
#define VRRP_VM_IS_IP_EQUAL(ip1, ip2)  !memcmp((ip1), (ip2), VRRP_IP_ADDR_LENGTH)
#define VRRP_VM_IS_MULTICAST_VIRTUAL_MAC(mac) \
    ((mac)[0] == 0x01) && \
    ((mac)[1] == 0x00) && \
    ((mac)[2] == 0x5E) && \
    ((mac)[3] == 0x00) && \
    ((mac)[4] == 0x00) && \
    ((mac)[5] == 0x12)

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI16_T VRRP_VM_ChkSumCalculate(UI16_T *calculateDataBufferPtr,
                                      UI32_T calculateLengthInHalfWord, UI8_T type);
static BOOL_T VRRP_VM_OperationStateChange(VRRP_OPER_ENTRY_T *vrrp_info, BOOL_T to_master);
static BOOL_T VRRP_VM_ProcessOperationState(
    VRRP_PKT_T *vrrpPkt,
    VRRP_IP_PKT_T *ipPkt,
    VRRP_OPER_ENTRY_T *vrrp_info
);

static void VRRP_VM_ProcessVrrpPkt(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T    ifindex,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN]
);

static UI32_T VRRP_VM_vrrpPktCheck(
    VRRP_PKT_T *vrrpPktPtr,
    VRRP_IP_PKT_T *ipPktBufferPtr,
    VRRP_OPER_ENTRY_T *vrrp_info
);

static void VRRP_VM_AddSystemLog(
    UI8_T level,
    UI8_T function_no,
    UI8_T error_no,
    UI32_T msg_idx,
    void *msg
);

/* STATIC VARIABLE DECLARATIONS
*/
static BOOL_T vrrpVmErrorLogFlag[VRRP_TYPE_EH_MAX_NUMBER] = {0};

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will init VRRP virtual machine
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VRRP_VM_Init(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_RxVrrpPkt
 *--------------------------------------------------------------------------
 * PURPOSE  : When receive ADVERTISEMENT packet, this function handle mref release
 * INPUT    : mref_handle_p     --  mref
 *            ifindex           --  VLAN ifindex
 *            dst_mac           --  destination MAC address
 *            src_mac           --  source MAC address
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
void VRRP_VM_RxVrrpPkt(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T    ifindex,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    VRRP_VM_ProcessVrrpPkt(mref_handle_p, ifindex, dst_mac, src_mac);
    L_MM_Mref_Release(&mref_handle_p);
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_ProcessVrrpPkt
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process ADVERTISEMENT packet
 * INPUT    : mref_handle_p     --  mref
 *            ifindex           --  VLAN ifindex
 *            dst_mac           --  destination MAC address
 *            src_mac           --  source MAC address
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static void VRRP_VM_ProcessVrrpPkt(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T    ifindex,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    VRRP_PKT_T *vrrpPktPtr;
    VRRP_IP_PKT_T *ipPktBufferPtr;
    UI32_T ipHeaderLen;
    UI32_T pkt_length = 0;
    VRRP_OPER_ENTRY_T vrrp_info;
    UI32_T result;

    if(!VRRP_VM_IS_MULTICAST_VIRTUAL_MAC(dst_mac))
    {
        VRRP_VM_AddSystemLog(
            SYSLOG_LEVEL_NOTICE,
            VRRP_TYPE_VM_PROCESS_VRRP_PKT,
            VRRP_TYPE_EH_RX_PKT_DA_ERROR,
            VRRP_VM_RECEIVE_PACKET_DA_ERROR_INDEX,
            0);

        return;
    }

    ipPktBufferPtr = (VRRP_IP_PKT_T *)L_MM_Mref_GetPdu(mref_handle_p, &pkt_length);
    if (ipPktBufferPtr == 0)
    {
        VRRP_VM_AddSystemLog(
            SYSLOG_LEVEL_NOTICE,
            VRRP_TYPE_VM_PROCESS_VRRP_PKT,
            VRRP_TYPE_EH_GET_PDU_ERROR,
            VRRP_VM_L_MREF_GETPDU_ERROR_INDEX,
            0);

        return;
    }

    ipHeaderLen = (0x0f & ipPktBufferPtr->verHlen) * 4;
    if (VRRP_VM_ChkSumCalculate((UI16_T *)ipPktBufferPtr, ipHeaderLen / 2, IP_HEADER) != ipPktBufferPtr->headerChksum)
    {
        return;
    }

    vrrpPktPtr = (VRRP_PKT_T*)((UI8_T*)ipPktBufferPtr + ipHeaderLen);

    /* get before process VRRP entry
     */
    memset(&vrrp_info, 0, sizeof(vrrp_info));
    vrrp_info.ifindex = ifindex;
    vrrp_info.vrid = vrrpPktPtr->vrid;
    if(VRRP_TYPE_OK != VRRP_OM_GetVrrpOperEntry(&vrrp_info))
    {
        return;
    }

    vrrp_info.vrrp_statistic.vrrpStatsAdvertiseRcvd++;

    if(vrrpPktPtr->priority == 0)
    {
        vrrp_info.vrrp_statistic.vrrpStatsPriorityZeroPktsRcvd++;
    }

    result = VRRP_VM_vrrpPktCheck(vrrpPktPtr, ipPktBufferPtr, &vrrp_info);
    if (result != VRRP_TYPE_OK)
    {
        if (result == VRRP_TYPE_MISCONFIGURATION)
        {
            VRRP_VM_AddSystemLog(
                SYSLOG_LEVEL_WARNING,
                VRRP_TYPE_VM_PROCESS_VRRP_PKT,
                VRRP_TYPE_EH_MISCONFIGURATION,
                VRRP_VM_MISCONFIGURATION_INDEX,
                0);
        }

        return;
    }

    if(!VRRP_VM_ProcessOperationState(vrrpPktPtr, ipPktBufferPtr, &vrrp_info))
    {
        return;
    }

    /* save modified VRRP entry
     */
    VRRP_OM_SetVrrpOperEntry(&vrrp_info);
} /* VRRP_VM_ProcessVrrpPkt() */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_Startup
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with Startup event
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_VM_Startup(UI32_T if_index, UI8_T vrid)
{
    VRRP_OPER_ENTRY_T vrrp_info;
    SYSLOG_OM_RecordOwnerInfo_T owner_info;

    vrrp_info.ifindex = if_index;
    vrrp_info.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_info) != VRRP_TYPE_OK)
        return FALSE;

    VRRP_BD(INFO, "vid[%lu], vrid[%u], priority[%lu]", (unsigned long)if_index, vrid, (unsigned long)vrrp_info.priority);

    if (vrrp_info.oper_state == VAL_vrrpOperState_initialize)
    {
        VRRP_ASSOC_IP_ENTRY_T assoc_ip_info;

        VRRP_BD(EVENT, "Initialize with priority = %lu", (unsigned long)vrrp_info.priority);

        memset(&assoc_ip_info, 0, sizeof(assoc_ip_info));
        assoc_ip_info.ifindex = vrrp_info.ifindex;
        assoc_ip_info.vrid = vrrp_info.vrid;
        memset(assoc_ip_info.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
        while (VRRP_OM_GetNextVrrpAssoIpAddress(&assoc_ip_info) == VRRP_TYPE_OK)
        {
            if((if_index != assoc_ip_info.ifindex)||(vrid != assoc_ip_info.vrid))
                break;

            if(VRRP_VM_IS_IP_EQUAL(assoc_ip_info.assoc_ip_addr, vrrp_info.primary_ip_addr))
            {
                vrrp_info.priority = VRRP_TYPE_MAX_PRIORITY;
                vrrp_info.master_priority = vrrp_info.priority;
                vrrp_info.owner = VrrpOperEntryAddrOwner_true;
                break;
            }
        }

        if (vrrp_info.priority == VRRP_TYPE_MAX_PRIORITY)
        {
            VRRP_BD(EVENT, "Become master router.");
            VRRP_TXRX_SendAdvertisement(&vrrp_info);
            VRRP_TXRX_SendGratuitousArp(&vrrp_info);
            vrrp_info.master_advertise_int = vrrp_info.advertise_interval;
            vrrp_info.master_down_int = 3 * vrrp_info.advertise_interval + ((256 - vrrp_info.priority)/ 256);
            vrrp_info.transmit_expire = VRRP_TYPE_SEC_TO_TICK(vrrp_info.advertise_interval);
            vrrp_info.oper_state = VAL_vrrpOperState_master;
            vrrp_info.vrrp_statistic.vrrpStatsBecomeMaster++;
            memcpy(vrrp_info.master_ip_addr, vrrp_info.primary_ip_addr, VRRP_IP_ADDR_LENGTH);

            if (!VRRP_VM_OperationStateChange(&vrrp_info, TRUE))
            {
                return FALSE;
            }

            /* Add operation state change to syslog */
            VRRP_VM_AddSystemLog(
                SYSLOG_LEVEL_NOTICE,
                VRRP_TYPE_VM_STARTUP,
                VRRP_TYPE_EH_STATE_CHANGE,
                VRRP_VM_OPER_STATE_CHANGE_INDEX,
                "master");
        }
        else
        {
            UI32_T skew_time=0;

            VRRP_BD(EVENT, "Become backup router.");
            skew_time = (VRRP_TYPE_SEC_TO_TICK(256 - vrrp_info.priority))/256;
            vrrp_info.master_down_expire =
                VRRP_TYPE_SEC_TO_TICK(3 * vrrp_info.advertise_interval + vrrp_info.preempt_delay) + skew_time;
            vrrp_info.oper_state = VAL_vrrpOperState_backup;

            /* Add operation state change to syslog */
            VRRP_VM_AddSystemLog(
                SYSLOG_LEVEL_NOTICE,
                VRRP_TYPE_VM_STARTUP,
                VRRP_TYPE_EH_STATE_CHANGE,
                VRRP_VM_OPER_STATE_CHANGE_INDEX,
                "backup");
        }
    }

    if (VRRP_OM_SetVrrpOperEntry(&vrrp_info) != VRRP_TYPE_OK)
        return FALSE;
    return TRUE;
} /* VRRP_VM_Startup() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_ShutDown
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with ShutDown event
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_VM_ShutDown(UI32_T if_index, UI8_T vrid)
{
    VRRP_OPER_ENTRY_T vrrp_info;
    SYSLOG_OM_RecordOwnerInfo_T owner_info;

    vrrp_info.ifindex = if_index;
    vrrp_info.vrid = vrid;
    if (VRRP_OM_GetVrrpOperEntry(&vrrp_info) != VRRP_TYPE_OK)
        return FALSE;

    if (vrrp_info.oper_state == VAL_vrrpOperState_master)
    {

        if (!VRRP_VM_OperationStateChange(&vrrp_info, FALSE))
        {
            return FALSE;
        }

        vrrp_info.transmit_expire = VRRP_TYPE_NO_EXPIRE_TIME;

        if (vrrp_info.priority == VRRP_TYPE_MAX_PRIORITY)
        {
            vrrp_info.priority = 0;
            VRRP_TXRX_SendAdvertisement(&vrrp_info);
            vrrp_info.priority = vrrp_info.pre_priority;
        }
        else
        {
            vrrp_info.priority = 0;
            VRRP_TXRX_SendAdvertisement(&vrrp_info);
            vrrp_info.priority = vrrp_info.pre_priority;
        }
        vrrp_info.vrrp_statistic.vrrpStatsPriorityZeroPktsSent++;
    }
    else if (vrrp_info.oper_state == VAL_vrrpOperState_backup)
    {
        vrrp_info.master_down_expire = VRRP_TYPE_NO_EXPIRE_TIME;
    }

    vrrp_info.oper_state = VAL_vrrpOperState_initialize;

    memset(vrrp_info.master_ip_addr, 0, sizeof(vrrp_info.master_ip_addr));
    vrrp_info.master_priority = 0;
    vrrp_info.owner = VrrpOperEntryAddrOwner_false;
    vrrp_info.master_advertise_int = 0;
    vrrp_info.master_down_int = 0;

    if (VRRP_OM_SetVrrpOperEntry(&vrrp_info) != VRRP_TYPE_OK)
        return FALSE;


    /* Add operation state change to syslog */
    VRRP_VM_AddSystemLog(
        SYSLOG_LEVEL_NOTICE,
        VRRP_TYPE_VM_SHUTDOWN,
        VRRP_TYPE_EH_STATE_CHANGE,
        VRRP_VM_OPER_STATE_CHANGE_INDEX,
        "initialize");
    return TRUE;
} /* VRRP_VM_ShutDown() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_Check_Master_Down_Timer
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with Master_Down_Timer Expire event
 * INPUT    : vrrp_oper_entry   --  VRRP operation entry
 *            pass_time         --  passed time interval in ticks
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_VM_Check_Master_Down_Timer(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T pass_time)
{
    SYSLOG_OM_RecordOwnerInfo_T owner_info;

    if (vrrp_oper_entry->oper_state != VAL_vrrpOperState_backup)
        return VRRP_VM_INTERNAL_ERROR;

    if(vrrp_oper_entry->master_down_expire == VRRP_TYPE_NO_EXPIRE_TIME)
        return VRRP_VM_MASTER_DOWN_TIMER_NOT_EXPIRE;

    if(vrrp_oper_entry->master_down_expire > pass_time)
    {
        vrrp_oper_entry->master_down_expire -= pass_time;
        if(VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry) != VRRP_TYPE_OK)
            return VRRP_VM_INTERNAL_ERROR;

        return VRRP_VM_MASTER_DOWN_TIMER_NOT_EXPIRE;
    }
    else
    {
        VRRP_BD(EVENT, "Master route down, backup become master router.");
        VRRP_TXRX_SendAdvertisement(vrrp_oper_entry);
        VRRP_TXRX_SendGratuitousArp(vrrp_oper_entry);
        vrrp_oper_entry->transmit_expire = VRRP_TYPE_SEC_TO_TICK(vrrp_oper_entry->advertise_interval);
        vrrp_oper_entry->oper_state = VAL_vrrpOperState_master;
        memcpy(vrrp_oper_entry->master_ip_addr, vrrp_oper_entry->primary_ip_addr, VRRP_IP_ADDR_LENGTH);
        vrrp_oper_entry->master_priority = vrrp_oper_entry->priority;
        vrrp_oper_entry->master_advertise_int = vrrp_oper_entry->advertise_interval;
        vrrp_oper_entry->master_down_int = (3 * vrrp_oper_entry->advertise_interval) + ((256 - vrrp_oper_entry->priority) / 256);
        vrrp_oper_entry->vrrp_statistic.vrrpStatsBecomeMaster++;
        vrrp_oper_entry->preempt_delay_start = FALSE;
        vrrp_oper_entry->master_down_expire = VRRP_TYPE_NO_EXPIRE_TIME;

        if (VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry) != VRRP_TYPE_OK)
            return VRRP_VM_INTERNAL_ERROR;

        if (!VRRP_VM_OperationStateChange(vrrp_oper_entry, TRUE))
            return VRRP_VM_INTERNAL_ERROR;

        /* Add operation state change to syslog */
        VRRP_VM_AddSystemLog(
            SYSLOG_LEVEL_NOTICE,
            VRRP_TYPE_VM_CHECK_MASTER_DOWN_TIMER,
            VRRP_TYPE_EH_STATE_CHANGE,
            VRRP_VM_OPER_STATE_CHANGE_INDEX,
            "master");
        return VRRP_VM_MASTER_DOWN_TIMER_EXPIRE;
    }

} /* VRRP_VM_Check_Master_Down_Timer() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_Check_Adver_Timer
 *--------------------------------------------------------------------------
 * PURPOSE  : This function actions with Adver_Timer Expire event
 * INPUT    : vrrp_oper_entry   --  VRRP operation entry
 *            pass_time         --  passed time interval in ticks
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
UI32_T VRRP_VM_Check_Adver_Timer(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T pass_time)
{
    UI32_T ret = VRRP_VM_ADVER_TIMER_NOT_EXPIRE;

    if (vrrp_oper_entry->oper_state != VAL_vrrpOperState_master)
        return VRRP_VM_INTERNAL_ERROR;

    if(vrrp_oper_entry->transmit_expire == VRRP_TYPE_NO_EXPIRE_TIME)
        return VRRP_VM_ADVER_TIMER_NOT_EXPIRE;

    if(vrrp_oper_entry->transmit_expire > pass_time)
    {
        vrrp_oper_entry->transmit_expire -= pass_time;
        ret = VRRP_VM_ADVER_TIMER_NOT_EXPIRE;
    }
    else
    {
        VRRP_TXRX_SendAdvertisement(vrrp_oper_entry);
        vrrp_oper_entry->transmit_expire = VRRP_TYPE_SEC_TO_TICK(vrrp_oper_entry->advertise_interval);
        ret = VRRP_VM_ADVER_TIMER_EXPIRE;
    }

    if (VRRP_OM_SetVrrpOperEntry(vrrp_oper_entry) != VRRP_TYPE_OK)
    {
        /* Should log error! (set vrrpoperentry error!) */
        ret = VRRP_VM_INTERNAL_ERROR;
    }

    return ret;
} /* VRRP_VM_Check_Adver_Timer() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_vrrpPktCheck
 *--------------------------------------------------------------------------
 * PURPOSE  : This function checks packets
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : 1. If "Ip addr Count" or the list of Ip addr does not match
 *               local configuration, we should return "misconfiguration"
 *--------------------------------------------------------------------------*/
static UI32_T VRRP_VM_vrrpPktCheck(
    VRRP_PKT_T *vrrpPktPtr,
    VRRP_IP_PKT_T *ipPktBufferPtr,
    VRRP_OPER_ENTRY_T *vrrp_info)
{
    UI8_T   mip[VRRP_IP_ADDR_LENGTH];
    UI8_T   *auth_data_p = NULL;
    UI8_T   *addr_pointer = NULL;
    UI16_T  vrrp_header_length;
    UI32_T	i = 0;
    VRRP_ASSOC_IP_ENTRY_T assoc_ip_info;
    UI32_T  result = VRRP_TYPE_OK;

    /* The ip must be 224.0.0.18 */
    memcpy(mip, ipPktBufferPtr->desIp, VRRP_IP_ADDR_LENGTH);
    if ((mip[0] != 224) || (mip[1] != 0) || (mip[2] != 0) || (mip[3] != 18))
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /* The protocol of the vrrp must be 112 */
    if (ipPktBufferPtr->protocol != VRRP_PROTOCOL_NUMBER)
    {
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /* The Virtaul Router Id must between 1~255*/
    if ((vrrpPktPtr->vrid < SYS_ADPT_MIN_VRRP_ID) || (vrrpPktPtr->vrid > SYS_ADPT_MAX_VRRP_ID))
    {
        VRRP_OM_IncreaseRouterStatisticCounter(VRRP_ROUTER_VRID_ERROR);
        return VRRP_TYPE_PARAMETER_ERROR;
    }
    else if (VRRP_OM_VridExistOnIf(vrrp_info->ifindex, vrrpPktPtr->vrid) == FALSE)
    { /* The vrid must exist on the receiving interface */
        VRRP_OM_IncreaseRouterStatisticCounter(VRRP_ROUTER_VRID_ERROR);
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /* The version of the vrrp packet must be 2 */
    if ((0xF0 & vrrpPktPtr->verType) != 0x20)
    {
        VRRP_OM_IncreaseRouterStatisticCounter(VRRP_ROUTER_VERSION_ERROR);
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /* Check the csum of the packet*/
    vrrp_header_length = sizeof(VRRP_PKT_T) + vrrpPktPtr->countIpAddr * sizeof(UI32_T) +
                         VRRP_MGR_AUTH_DATA_LENGTH;

    if (VRRP_VM_ChkSumCalculate((UI16_T *)vrrpPktPtr, vrrp_header_length / 2, VRRP_HEADER) != vrrpPktPtr->checksum)
    {
        VRRP_OM_IncreaseRouterStatisticCounter(VRRP_ROUTER_CHECKSUM_ERROR);
        return VRRP_TYPE_PARAMETER_ERROR;
    }

    /* The TTL of the vrrp must be 255*/
    if (ipPktBufferPtr->ttl != VRRP_IP_TTL)
    {
        vrrp_info->vrrp_statistic.vrrpStatsIpTtlErrors++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The length of the packet must be greater than or equal to the VRRP header */
    /* In our design, the length should be greater than or equal to 40 byte(20bytes IP + 20bytes VRRP).*/
    if ((ipPktBufferPtr->totalLen - (0x0F & ipPktBufferPtr->verHlen) * 4) < (sizeof(VRRP_PKT_T) + sizeof(UI32_T) + VRRP_MGR_AUTH_DATA_LENGTH))
    {
        vrrp_info->vrrp_statistic.vrrpStatsPacketLengthErrors++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The type of the vrrp packet must be 1 for advertisement packet */
    if ((0x0F & vrrpPktPtr->verType) != 0x1)
    {
        vrrp_info->vrrp_statistic.vrrpStatsInvalidTypePktsRcvd++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The version of vrrp packet must be 2 */
    if((0xF0 & vrrpPktPtr->verType) != 0x20)
    {
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The authentication type in MIB is from 1~3, but it is from 0~2 in VRRP control packets */
    (vrrpPktPtr->authType)++;
    if ((vrrpPktPtr->authType != VAL_vrrpOperAuthType_noAuthentication) &&
        (vrrpPktPtr->authType != VAL_vrrpOperAuthType_simpleTextPassword) &&
        (vrrpPktPtr->authType != VAL_vrrpOperAuthType_ipAuthenticationHeader))
    {
        vrrp_info->vrrp_statistic.vrrpStatsInvalidAuthType++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The advertisement interval of each Virtaul Router must between 1~255*/
    if ((vrrpPktPtr->adverInt < SYS_ADPT_MIN_VRRP_ADVER_INTERVAL) ||
        (vrrpPktPtr->adverInt > SYS_ADPT_MAX_VRRP_ADVER_INTERVAL))
    {
        vrrp_info->vrrp_statistic.vrrpStatsAdvertiseIntervalErrors++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The authentication type of packet must match with local configuration */
    if (vrrpPktPtr->authType != vrrp_info->auth_type)
    {
        vrrp_info->vrrp_statistic.vrrpStatsAuthTypeMismatch++;
        return VRRP_TYPE_MISCONFIGURATION;
    }
    /* The authentication data must match with local configuration */
    if (vrrp_info->auth_type == VAL_vrrpOperAuthType_simpleTextPassword)
    {
        auth_data_p = (UI8_T *)vrrpPktPtr + sizeof(VRRP_PKT_T) + vrrpPktPtr->countIpAddr * sizeof(UI32_T);
        if (memcmp(vrrp_info->auth_key, auth_data_p, VRRP_MGR_AUTH_DATA_LENGTH) != 0)
        {
            vrrp_info->vrrp_statistic.vrrpStatsAuthFailures++;
            return VRRP_TYPE_MISCONFIGURATION;
        }
    }

    /* The advertisement interval must match with local configuration */
    if (vrrpPktPtr->adverInt != vrrp_info->advertise_interval)
    {
        vrrp_info->vrrp_statistic.vrrpStatsAdvertiseIntervalErrors++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    /* The associated IP addresses and amount must match with local configuration */
    if (vrrpPktPtr->countIpAddr != vrrp_info->ip_addr_count)
    {
        vrrp_info->vrrp_statistic.vrrpStatsAddressListErrors++;
        return VRRP_TYPE_MISCONFIGURATION;
    }

    assoc_ip_info.ifindex = vrrp_info->ifindex;
    assoc_ip_info.vrid = vrrpPktPtr->vrid;
    addr_pointer = (UI8_T *)vrrpPktPtr + sizeof(VRRP_PKT_T);
    for (i = 0; i < vrrpPktPtr->countIpAddr; i++)
    {
        memcpy(assoc_ip_info.assoc_ip_addr, addr_pointer, VRRP_IP_ADDR_LENGTH);
        if (VRRP_OM_SearchVrrpAssoIpAddrEntry(&assoc_ip_info) != VRRP_TYPE_OK)
        {
            vrrp_info->vrrp_statistic.vrrpStatsAddressListErrors++;
            return VRRP_TYPE_MISCONFIGURATION;
        }

        if(VRRP_VM_IS_IP_EQUAL(addr_pointer, ipPktBufferPtr->srcIp))
        {
            /* If the packet was not generated by the address owner(priority does not equal 255),
             * , the receiver must drop the packet, otherwise continue processing.
             * (RFC3768, p.19)
             * It shouldn't filter priority is 0, because shutdown event will send out it.
             */
            if((vrrpPktPtr->priority != VRRP_TYPE_MAX_PRIORITY)&&
               (vrrpPktPtr->priority != 0))
            {
                return VRRP_TYPE_PARAMETER_ERROR;
            }
        }

        addr_pointer += 4;
    }

    return VRRP_TYPE_OK;
} /* VRRP_VM_vrrpPktCheck() */

/* FUNCTION : VRRP_VM_IpChkSumCalculate
 * PURPOSE  : The function calculate vrrp checksum
 * INPUT    : *calculateDataBufferPtr     --  Data buffer which want to calculate checksum.
 *            calculateLengthInHalfWord   --  length of data buffer (in 2bytes length).
 * OUTPUT   : None.
 * RETUEN   : checksum result.
 * NOTES    : None.
 */
static UI16_T VRRP_VM_ChkSumCalculate(UI16_T *calculateDataBufferPtr, UI32_T calculateLengthInHalfWord, UI8_T type)
{
    UI32_T  i, temp;
    UI16_T  checkSum;

    checkSum = *calculateDataBufferPtr;
    calculateDataBufferPtr++;
    for (i = 1; i < calculateLengthInHalfWord; i++)
    {
        if ((i == 3) && (type == VRRP_HEADER)) /* checksum must be 0x0000 for calculating vrrp checksum */
            temp = checkSum + 0x0000;
        else if ((i == 5) && (type == IP_HEADER)) /* checksum must be 0x0000 for calculating ip checksum */
            temp = checkSum + 0x0000;
        else
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
} /* VRRP_VM_ChkSumCalculate() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_OperationStateChange
 *--------------------------------------------------------------------------
 * PURPOSE  : change VRRP operation status
 * INPUT    : vrrp_info     --  VRRP entry info
 *            to_master     --  change to master or not
 * OUTPUT   : None
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T VRRP_VM_OperationStateChange(VRRP_OPER_ENTRY_T *vrrp_info, BOOL_T to_master)
{
    VRRP_ASSOC_IP_ENTRY_T vrrp_assoc_ip_entry;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    L_PREFIX_IPv4_T vip_prefix;
    BOOL_T is_owner = FALSE;
    UI32_T ipaddr = 0;
    UI32_T i = 0;

    if (NULL == vrrp_info)
        return FALSE;

    /* get associated ip address
     */
    memset(&vrrp_assoc_ip_entry, 0, sizeof(vrrp_assoc_ip_entry));
    vrrp_assoc_ip_entry.ifindex = vrrp_info->ifindex;
    vrrp_assoc_ip_entry.vrid = vrrp_info->vrid;
    memset(vrrp_assoc_ip_entry.assoc_ip_addr, 0, VRRP_IP_ADDR_LENGTH);
    for (i = 0; i < vrrp_info->ip_addr_count; i++)
    {
        if (VRRP_TYPE_OK == VRRP_OM_GetNextVrrpAssoIpAddress(&vrrp_assoc_ip_entry))
        {
            IP_LIB_ArraytoUI32(vrrp_assoc_ip_entry.assoc_ip_addr, &ipaddr);
            break;
        }
        else
        {
            return FALSE;
        }

    }

    /* check if priroity is ip owner
     */
    if (vrrp_info->priority == VRRP_TYPE_MAX_PRIORITY)
    {
        is_owner = TRUE;
    }

    memset(&vip_prefix, 0, sizeof(vip_prefix));
    vip_prefix.prefix.s_addr = ipaddr;
    vip_prefix.prefixlen = 32;

    if (to_master)
    {
        /* change to master state
         */
        VRRP_BD(EVENT, "change to master state.");
        VRRP_BD(EVENT, "Add virtual mac[%02X-%02X-%02X-%02X-%02X-%02X] on ifindex[%lu].",
                L_INET_EXPAND_MAC(vrrp_info->virtual_mac),
                (unsigned long)vrrp_info->ifindex);
        if (!AMTRL3_PMGR_AddL3Mac(vrrp_info->virtual_mac, vrrp_info->ifindex))
        {
            return FALSE;
        }

#if (SYS_CPNT_NSM == TRUE)
        {
            L_INET_AddrIp_T vip_info;

            memset(&vip_info, 0, sizeof(vip_info));
            vip_info.type = L_INET_ADDR_TYPE_IPV4;
            vip_info.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            memcpy(vip_info.addr, vrrp_assoc_ip_entry.assoc_ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
            if(NSM_TYPE_RESULT_OK != NSM_PMGR_AddVrrpVirtualIp(&vip_info))
            {
                VRRP_BD(EVENT, "Failed to add virtual IP %u.%u.%u.%u to NSM",
                    L_INET_EXPAND_IP(vip_info.addr));
            }
        }
#endif

        /* clear AMTRL3 dynamic ARP entry */
        memset(&host_entry, 0, sizeof(host_entry));
        host_entry.dst_inet_addr.type = VAL_InetAddressType_ipv4;
        memcpy(host_entry.dst_inet_addr.addr, &ipaddr, SYS_ADPT_IPV4_ADDR_LEN);
        VRRP_BD(EVENT, "Clear dynamic ARP[%u.%u.%u.%u]", L_INET_EXPAND_IP(ipaddr));
        AMTRL3_PMGR_DeleteHostRoute(AMTRL3_TYPE_FLAGS_IPV4, 0, &host_entry);

        VRRP_BD(EVENT, "Add virtual ip[%u.%u.%u.%u],vrid[%u],ifindex[%lu],is_owner[%s].",
                L_INET_EXPAND_IP(ipaddr),
                vrrp_info->vrid,
                (unsigned long)vrrp_info->ifindex,
                is_owner ? "YES" : "NO");

        if(!is_owner)
        {
            if(IPAL_RESULT_OK !=
                IPAL_VRRP_AddVrrpVirturalIp(
                    vrrp_info->ifindex,
                    vrrp_info->vrid,
                    &vip_prefix))
            {
                VRRP_BD(DBG, "Failed to IPAL_VRRP_AddVrrpVirturalIp");
                return FALSE;
            }
        }
    }
    else
    {
        /* change from master state to initialize/backup
         */
        VRRP_BD(EVENT, "change from master state to initialize/backup.");
        VRRP_BD(EVENT, "Delete virtual mac[%02X-%02X-%02X-%02X-%02X-%02X] on ifindex[%lu].",
                L_INET_EXPAND_MAC(vrrp_info->virtual_mac),
                (unsigned long)vrrp_info->ifindex);
        if (!AMTRL3_PMGR_DeleteL3Mac(vrrp_info->virtual_mac, vrrp_info->ifindex))
        {
            return FALSE;
        }

#if (SYS_CPNT_NSM == TRUE)
        {
            L_INET_AddrIp_T vip_info;

            memset(&vip_info, 0, sizeof(vip_info));
            vip_info.type = L_INET_ADDR_TYPE_IPV4;
            vip_info.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            memcpy(vip_info.addr, vrrp_assoc_ip_entry.assoc_ip_addr, SYS_ADPT_IPV4_ADDR_LEN);
            if(NSM_TYPE_RESULT_OK != NSM_PMGR_DelVrrpVirtualIp(&vip_info))
            {
                VRRP_BD(EVENT, "Failed to remove virtual IP %u.%u.%u.%u to NSM",
                    L_INET_EXPAND_IP(vip_info.addr));
            }
        }
#endif

        VRRP_BD(EVENT, "Delete virtual ip[%u.%u.%u.%u],vrid[%u],ifindex[%lu],is_owner[%s].",
                L_INET_EXPAND_IP(ipaddr),
                vrrp_info->vrid,
                (unsigned long)vrrp_info->ifindex,
                is_owner ? "YES" : "NO");


        if(!is_owner)
        {
            if(IPAL_RESULT_OK !=
                IPAL_VRRP_DeleteVrrpVirturalIp(
                    vrrp_info->ifindex,
                    vrrp_info->vrid,
                    &vip_prefix))
            {
                /* If virtual ip is not existed, regard as success
                 */
                VRRP_BD(DBG, "Warning:Failed to IPAL_VRRP_DeleteVrrpVirturalIp");
            }
        }
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_ProcessOperationState
 *--------------------------------------------------------------------------
 * PURPOSE  : process VRRP packet to check operation state
 * INPUT    : vrrpPkt       --  VRRP packet
 *            ipPkt         --  IP packet
 *            vrrp_info     --  VRRP operation entry
 * OUTPUT   : vrrp_info
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static BOOL_T VRRP_VM_ProcessOperationState(
    VRRP_PKT_T *vrrpPkt,
    VRRP_IP_PKT_T *ipPkt,
    VRRP_OPER_ENTRY_T *vrrp_info)
{
    UI32_T skew_time;

    if (vrrp_info->oper_state == VAL_vrrpOperState_backup)
    {
        UI32_T expire_interval=0;

        if (vrrpPkt->priority == 0)
        {
            VRRP_BD(EVENT, "Backup receive vrrp packet with priority = 0.");

            skew_time = (VRRP_TYPE_SEC_TO_TICK(256 - vrrp_info->priority)) / 256;
            vrrp_info->master_down_expire = skew_time +
                                           VRRP_TYPE_SEC_TO_TICK(vrrp_info->preempt_delay);
        }
        else
        {
            if ((vrrp_info->preempt_mode == VAL_vrrpOperPreemptMode_false) ||
                    (vrrpPkt->priority >= vrrp_info->priority))
            {
                VRRP_BD(EVENT, "Backup receive vrrp packet priority %lu higher than local priority %lu/preempt mode is %lu",
                        (unsigned long)vrrpPkt->priority, (unsigned long)vrrp_info->priority, (unsigned long)vrrp_info->preempt_mode);
                skew_time = (VRRP_TYPE_SEC_TO_TICK(256 - vrrp_info->priority))/ 256;
                vrrp_info->master_down_expire =
                    VRRP_TYPE_SEC_TO_TICK(3 * vrrp_info->advertise_interval + vrrp_info->preempt_delay) + skew_time;
            }

        }

        memcpy(vrrp_info->master_ip_addr, ipPkt->srcIp, VRRP_IP_ADDR_LENGTH);
        vrrp_info->master_priority = vrrpPkt->priority;
        vrrp_info->master_advertise_int = vrrpPkt->adverInt;
        vrrp_info->master_down_int = (3 * vrrp_info->advertise_interval) + ((256 - vrrp_info->priority) / 256);
        /* the timer may less than one second,
         * update timer trigger interval
         */
        expire_interval = vrrp_info->master_down_expire;
        VRRP_BD(EVENT, "expire time %lu ticks", (unsigned long)expire_interval);

        if(expire_interval < VRRP_TYPE_DFLT_TIMER_TICKS)
        {
            void *vrrp_timer_id_p = VRRP_OM_GetTimerId();
            UI32_T prev_time_interval;

            /* only restart perioric timer if new interval is shorter than previous interval
             */
            if(!SYSFUN_PeriodicTimer_Get(vrrp_timer_id_p, &prev_time_interval))
                VRRP_BD(TIMER, "failed to get periodic timer");

            if(expire_interval < prev_time_interval)
            {
                VRRP_BD(TIMER, "restart periodic timer to %lu ticks", (unsigned long)expire_interval);
                if(expire_interval == 0)
                {
                    SYSFUN_PeriodicTimer_Restart(vrrp_timer_id_p, VRRP_TYPE_MIN_SYS_TICK);
                }
                else
                {
                    SYSFUN_PeriodicTimer_Restart(vrrp_timer_id_p, expire_interval);
                }

            }
        }
    }
    else if (vrrp_info->oper_state == VAL_vrrpOperState_master)
    {
        if (vrrpPkt->priority == 0)
        {
            VRRP_BD(EVENT, "Master receive vrrp packet with priority = 0.");

            VRRP_TXRX_SendAdvertisement(vrrp_info);
            vrrp_info->transmit_expire = VRRP_TYPE_SEC_TO_TICK(vrrp_info->advertise_interval);
        }
        else
        {
            if ((vrrpPkt->priority > vrrp_info->priority) ||
                    ((vrrpPkt->priority == vrrp_info->priority) &&
                     (memcmp(ipPkt->srcIp, vrrp_info->master_ip_addr, VRRP_IP_ADDR_LENGTH) > 0)))
            {
                VRRP_BD(EVENT, "Master receive vrrp packet priority %lu higher than local priority %lu/sip[%u.%u.%u.%u],local ip[%u.%u.%u.%u].",
                        (unsigned long)vrrpPkt->priority, (unsigned long)vrrp_info->priority,
                        L_INET_EXPAND_IP(ipPkt->srcIp), L_INET_EXPAND_IP(vrrp_info->master_ip_addr));

                if (!VRRP_VM_OperationStateChange(vrrp_info, FALSE))
                {
                    return FALSE;
                }

                vrrp_info->transmit_expire = VRRP_TYPE_NO_EXPIRE_TIME;
                skew_time = (VRRP_TYPE_SEC_TO_TICK(256 - vrrp_info->priority))/ 256;
                vrrp_info->master_down_expire =
                    VRRP_TYPE_SEC_TO_TICK(3 * vrrp_info->advertise_interval + vrrp_info->preempt_delay) + skew_time;
                vrrp_info->oper_state = VAL_vrrpOperState_backup;
                memcpy(vrrp_info->master_ip_addr, ipPkt->srcIp, VRRP_IP_ADDR_LENGTH);
                vrrp_info->master_priority = vrrpPkt->priority;
                vrrp_info->master_advertise_int = vrrpPkt->adverInt;
                vrrp_info->master_down_int = 3 * vrrp_info->advertise_interval + ((256 - vrrp_info->priority)/ 256);

                /* Add operation state change to syslog */
                VRRP_VM_AddSystemLog(
                    SYSLOG_LEVEL_NOTICE,
                    VRRP_TYPE_VM_PROCESS_VRRP_PKT,
                    VRRP_TYPE_EH_STATE_CHANGE,
                    VRRP_VM_OPER_STATE_CHANGE_INDEX,
                    "backup");
            }
        }
    }

    return TRUE;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_VM_AddSystemLog
 *--------------------------------------------------------------------------
 * PURPOSE  : Add VRRP log message to system log
 * INPUT    : level         --  system log level
 *            function_no   --  logging function number
 *            error_no      --  logging error number
 *            msg_idx       --  logging message index
 *            msg           --  log message
 * OUTPUT   : None
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : None
 *--------------------------------------------------------------------------
 */
static void VRRP_VM_AddSystemLog(
    UI8_T level,
    UI8_T function_no,
    UI8_T error_no,
    UI32_T msg_idx,
    void *msg)
{
    SYSLOG_OM_RecordOwnerInfo_T syslog_record;
    UI32_T flag_cnt = sizeof(vrrpVmErrorLogFlag)/sizeof(vrrpVmErrorLogFlag[0]);

    if(error_no > flag_cnt)
        return;

    if(!vrrpVmErrorLogFlag[error_no])
    {
        memset(&syslog_record, 0, sizeof(syslog_record));
        syslog_record.level = level;
        syslog_record.module_no = SYS_MODULE_VRRP;
        syslog_record.function_no = function_no;
        syslog_record.error_no = error_no;
        SYSLOG_PMGR_AddFormatMsgEntry(&syslog_record, msg_idx, msg, 0, 0);

        switch(error_no)
        {
            case VRRP_TYPE_EH_RX_PKT_DA_ERROR:
            case VRRP_TYPE_EH_GET_PDU_ERROR:
            case VRRP_TYPE_EH_MISCONFIGURATION:
                vrrpVmErrorLogFlag[error_no] = TRUE;
                break;
        }
    }
}
#endif

