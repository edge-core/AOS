/*-----------------------------------------------------------------------------
 * Module Name: L2MUX_MGR.C   (LAYER 2 MULTIPLEXING)
 *-----------------------------------------------------------------------------
 * PURPOSE: This module provides the LAN interface between core layer and
 *          driver layer.
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. The main function of this file is to provide the function of
 *    mapping uport to lport.
 * 2. This header file provides the interface to upper layer component, e.g.,
 *    IML, STA, GVRP and IGMPSNP, to access LAN interface.
 * 3. LACP and 802.1x don't need this function, since they use uport only.
 * 4. This module also provides the function for upper layer component to
 *    register their callback function when received packet from LAN interface.
 * 5. When receiving IP packet, L2MUX will check DA to see if this packet is a
 *    multicast packet. If so, L2MUX will send packet to IGMPSNP, as well as IML;
 *    otherwise, L2MUX will ONLY send packet(unicast) to IML.
 *
 *-----------------------------------------------------------------------------
 *
 *       --------------------       ----------------------------
 *      |   LACP, 802.1X     |     | IML(IP),IGMPSNP, GVRP, STA |
 *       --------------------       ----------------------------
 *                \                             |
 *                 \                            | (transmit, receive, register)
 *                  \                           |
 *                   \                 --------------------
 *                    \               |     L2MUX_MGR      |
 *                     \               --------------------
 *      (transmit, receive, register)          /
 *                       \                    / (transmit, receive, register)
 *                        \                  /
 *                        --------------------
 *                       |       LAN.c        |
 *                        --------------------
 *
 *-----------------------------------------------------------------------------
 * - For receiving packet:
 *
 *       --------          --------      -----------
 *      |  IML   |        | IGMPSNP|    | GVRP, STA |
 *       --------          --------      -----------
 *           \                /             /
 *         unicast,          /             /
 *         multicast     multicast        /
 *              \          /             /
 *               \        /             /
 *                --------             /
 *               |   IP   |           /
 *                --------           /
 *                   |              /
 *                   |             /
 *          ------------------------------
 *         |          L2MUX_MGR           |
 *          ------------------------------
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    01/03/2002 - Benson Hsu, Created
 *    07/02/2002 - Benson Hsu, Revised
 *    07/05/2002 - Check DA for multicast(IGMPSNP), Benson Hsu.
 *    10/11/2002 - Merge l2mux_tx.c and l2mux_rx.c to one l2mux_mgr.c.
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_ipcmem.h"
#include "l_bitmap.h"
#include "l_stdlib.h"
#include "backdoor_mgr.h"
#include "swctrl.h"
#include "trk_pmgr.h"
#include "lan.h"
#include "l2mux_mgr.h"
#include "l2mux_type.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_om.h"
#include "xstp_om.h"
#include "sys_callback_mgr.h"
#include "l2mux_group.h"

/* NAMING CONSTANT DECLARACTIONS
 */
#define BPDU_MAC_BYTE1                                 0x01
#define BPDU_MAC_BYTE2                                 0x80
#define BPDU_MAC_BYTE3                                 0xC2
#define BPDU_MAC_BYTE4                                 0x00
#define BPDU_MAC_BYTE5                                 0x00
#define BPDU_MAC_BYTE6                                 0x00

#define L2MUX_MGR_DEBUG

enum {
    L2MUX_TRACE_ID_L2PT,
};

#define L2MUX_IPV4_FORMAT        0x0800
#define L2MUX_IPV6_FORMAT        0x86DD

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef L2MUX_MGR_DEBUG
#define L2MUX_MGR_ERROR_PRINT(args...) (l2mux_mgr_error_msg ? BACKDOOR_MGR_Printf(args) : (void)0)
#define L2MUX_MGR_DEBUG_PRINT(args...) (l2mux_mgr_debug_msg ? BACKDOOR_MGR_Printf(args) : (void)0)
#define L2MUX_MGR_DEBUG_DUMP(title, len, buf) (l2mux_mgr_debug_msg ? BACKDOOR_MGR_DumpHex((title), (len), (buf)) : (void)0)
#else
#define L2MUX_MGR_ERROR_PRINT(args...)
#define L2MUX_MGR_DEBUG_PRINT(args...)
#define L2MUX_MGR_DEBUG_DUMP(title, len, buf)
#endif


/* TYPE DEFINITIONS
 */
/*----------------------------------------------------------------------------------
 * RETURN:  TURE  - MREF processing is finished and released
 *          FALSE - MREF still need to be processed
 *----------------------------------------------------------------------------------
 */
typedef BOOL_T (*L2MUX_MGR_PreprocessPktFunction_T)(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              lport,
    UI32_T              packet_class);

typedef struct L2MUX_MGR_Udphdr_S
{
    UI16_T  uh_sport;           /* source port       */
    UI16_T  uh_dport;           /* destination port  */
    UI16_T  uh_ulen;            /* udp length        */
    UI16_T  uh_sum;             /* udp checksum      */
} __attribute__((packed, aligned(1)))L2MUX_MGR_Udphdr_T;

typedef struct L2MUX_MGR_Ipv4PktFormat_S
{
    UI8_T   ver_len;
    UI8_T   tos;
    UI16_T  length;
    UI16_T  identification;
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU==TRUE)
    UI16_T  fragment_offset:13,
            flags:3;
#else
    UI16_T  flags:3,
            fragment_offset:13;
#endif
    UI8_T   ttl;
    UI8_T   protocol;
    UI16_T  checksum;
    UI32_T  srcIp;
    UI32_T  dstIp;
} __attribute__((packed, aligned(1)))L2MUX_MGR_Ipv4PktFormat_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T L2MUX_MGR_GetPortOperStatus(UI32_T unit_no, UI32_T port_no, UI32_T *lport);

/*
 * Callback Functions Declaration
 */
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
static BOOL_T L2MUX_MGR_IntrusionMac_CallBack(
        L_MM_Mref_Handle_T    *mref_handle_p,
        UI8_T 	    dst_mac[SYS_ADPT_MAC_ADDR_LEN],
        UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN],
        UI16_T      tag_info,       /* raw tagged info      */
        UI16_T      ether_type,     /* packet type          */
        UI32_T      pdu_length,     /* pdu length           */
        UI32_T      src_unit,       /* source unit number   */
        UI32_T      src_port,       /* source port number   */
        UI32_T      reason);        /* why packet trap to CPU */
#endif

static void L2MUX_MGR_SendPkt(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,
                          BOOL_T      is_tagged,
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members,
                          UI16_T      vid);

static void L2MUX_MGR_SendPktPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port);
                          
#if (SYS_CPNT_ITRI_MIM == TRUE)
static BOOL_T L2MUX_MGR_ITRI_MIM_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              lport,
    UI32_T              packet_class);
#endif

/* For debugging purpose ONLY
 */
#ifdef L2MUX_MGR_DEBUG

static void L2MUX_MGR_BackdoorMenu(void);

static void DebugReceivePacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                      UI8_T       dst_mac[6],
                                      UI8_T       src_mac[6],
                                      UI16_T      tag_info,
                                      UI16_T      type,
                                      UI32_T      packet_length,
                                      UI32_T      lport,
                                      UI8_T       protocol);

static void DebugSinglePacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                     UI8_T       dst_mac[6],
                                     UI8_T       src_mac[6],
                                     UI16_T      type,
                                     UI16_T      tag_info,
                                     UI32_T      packet_length,
                                     UI32_T      unit,
                                     UI32_T      port,
                                     BOOL_T      is_tagged,
                                     UI32_T      cos_value);

static void DebugMultiPacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T             dst_mac[6],
                                    UI8_T             src_mac[6],
                                    UI16_T            type,
                                    UI16_T            tag_info,
                                    UI32_T            packet_length,
                                    UI8_T             *uport_list,
                                    UI8_T             *untagged_list,
                                    UI8_T             *tx_port_count_per_unit,
                                    UI32_T            cos_value);

#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
static BOOL_T
L2MUX_MGR_RxSnoopDhcp_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              ether_type,
    UI32_T              pkt_length,
    UI32_T              lport,
    UI32_T              packet_class
);
#endif


/* LOCAL VARIABLE DECLARATIONS
 */
/* The macro of declared variables, used for stacking mode
 */
SYSFUN_DECLARE_CSC

L2MUX_MGR_AnnouncePktFunction_T	            L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_MAX_CLIENT_NO];

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
static SYS_TYPE_CallBack_T *IntrusionMsg_callbacklist = NULL;
/* static L2MUX_MGR_IntrusionCallBackFunction_T L2MUX_MGR_IntrusionCallBackFunc=NULL;*/
#endif/* SYS_CPNT_INTRUSION_MSG_TRAP */

static L2MUX_MGR_PreprocessPktFunction_T l2mux_mgr_preprocess_func_list[] = {
#if (SYS_CPNT_ITRI_MIM == TRUE)
    L2MUX_MGR_ITRI_MIM_CallbackFunc,
#endif
#if (SYS_CPNT_DHCPSNP == TRUE)
    L2MUX_MGR_RxSnoopDhcp_CallbackFunc,
#endif
    NULL
};

#if (SYS_CPNT_ITRI_MIM == TRUE)
static BOOL_T l2mux_mgr_itri_mim_status[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif

#ifdef L2MUX_MGR_DEBUG
static UI32_T l2mux_mgr_counter_rx = 0;
static BOOL_T l2mux_mgr_debug_rx = FALSE;
static BOOL_T l2mux_mgr_debug_tx = FALSE;
static BOOL_T l2mux_mgr_debug_msg = FALSE;
static BOOL_T l2mux_mgr_error_msg = FALSE;
static BOOL_T l2mux_mgr_drop_rx = FALSE;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: L2MUX_MGR_InitiateProcessResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_InitiateProcessResources(void)
{
    return;
}

/* FUNCTION NAME: L2MUX_MGR_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_InitiateSystemResources(void)
{
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: L2MUX_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for L2MUX in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void L2MUX_MGR_AttachSystemResources(void)
{
    return;
}

/* FUNCTION NAME: L2MUX_MGR_Create_InterCSC_Relation
 *---------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:
 */
void L2MUX_MGR_Create_InterCSC_Relation()
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("l2mux", SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY, L2MUX_MGR_BackdoorMenu);
}


/* FUNCTION NAME: L2MUX_MGR_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set L2MUX to transition mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}


/* FUNCTION NAME: L2MUX_MGR_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter transition mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE();

#if (SYS_CPNT_ITRI_MIM == TRUE)
    memset(l2mux_mgr_itri_mim_status, 0, sizeof(l2mux_mgr_itri_mim_status));
#endif

}


/* FUNCTION NAME: L2MUX_MGR_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter master mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
}


/* FUNCTION NAME: L2MUX_MGR_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter slave mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
}

/* FUNCTION NAME: L2MUX_MGR_SendBPDU
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to a specified logical port without checking the STA state.
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          dst_mac    -- destination MAC
 *          src_mac    -- source MAC
 *          type       -- Ethernet frame type or IEEE802.3 length field
 *          tag_info   -- 2 bytes tagged information excluding 0x8100 tag type
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          lport      -- the logical port ( trunk port or NM port )
 *          is_tagged  -- if the packet needs to be sent with tagged
 *          cos_value  -- the cos value when sending this packet
 *          is_send_to_trunk_members -- to tell if this packet is for all trunk members
 *                                      or only for the primary port in this trunk.
 *                                      TRUE: send this packet to all trunk members
 *                                      FALSE: send this packet only to primary port of this trunk
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:  1. This function will translate lport to uport. If this lport is a trunk port,
 *           it will also get all trunk members or a primary port for this trunk port.
 *        2. If packet will be sent to all trunk members, this function will regenerate
 *           a new untagged list and multicast the packet.
 */
void L2MUX_MGR_SendBPDU(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,
                          BOOL_T      is_tagged,
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members)
{
    L2MUX_MGR_SendPkt(mref_handle_p,
                      dst_mac,
                      src_mac,
                      type,
                      tag_info,
                      packet_length,
                      lport,
                      is_tagged,
                      cos_value,
                      is_send_to_trunk_members,
                      SYS_TYPE_IGNORE_VID_CHECK);
}


/* FUNCTION NAME: L2MUX_MGR_SendPacket
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to a specified logical port.
 *          (this function will check the STA state, packet will be drop if not in forwarding state)
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p  -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          dst_mac    -- destination MAC
 *          src_mac    -- source MAC
 *          type       -- Ethernet frame type or IEEE802.3 length field
 *          tag_info   -- 2 bytes tagged information excluding 0x8100 tag type
 *                        this function will not check STA state if tag_info=SYS_TYPE_IGNORE_VID_CHECK
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          lport      -- the logical port ( trunk port or NM port )
 *          is_tagged  -- if the packet needs to be sent with tagged
 *          cos_value  -- the cos value when sending this packet
 *          is_send_to_trunk_members -- to tell if this packet is for all trunk members
 *                                      or only for the primary port in this trunk.
 *                                      TRUE: send this packet to all trunk members
 *                                      FALSE: send this packet only to primary port of this trunk
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:  1. This function will translate lport to uport. If this lport is a trunk port,
 *           it will also get all trunk members or a primary port for this trunk port.
 *        2. If packet will be sent to all trunk members, this function will regenerate
 *           a new untagged list and multicast the packet.
 */
void L2MUX_MGR_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,
                          BOOL_T      is_tagged,
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members)
{
    UI16_T      vid;
    if ( SYS_TYPE_IGNORE_VID_CHECK == tag_info )
    {
        L_MM_Mref_Release(&mref_handle_p);
        printf("L2MUX_MGR_SendPacket:tag_info error.\n");
        return;
    }
    vid = tag_info & 0x0fff;
    /* L2 snooping filter */
    if(FALSE == SYS_CALLBACK_MGR_HandleSendNdPacket(SYS_MODULE_L2MUX,
                                        mref_handle_p,
                                        dst_mac,
                                        src_mac,
                                        type,
                                        packet_length,
                                        vid))
    {
        L_MM_Mref_Release(&mref_handle_p);
        printf("L2MUX_MGR_SendPacket:handle send nd packet error.\n");
        return;
    }

    L2MUX_MGR_SendPkt(mref_handle_p,
                      dst_mac,
                      src_mac,
                      type,
                      tag_info,
                      packet_length,
                      lport,
                      is_tagged,
                      cos_value,
                      is_send_to_trunk_members,
                      vid);
}

/* FUNCTION NAME: L2MUX_MGR_SendPacketPipeline
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to ASIC's packet processing pipeline.
 *          (this function will check the STA state, packet will be drop if not in forwarding state)
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p  -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          in_port    -- the physical port we pretend the packet arrived on.(not consider stacking here)
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:  
 */
void L2MUX_MGR_SendPacketPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port)
{

    /* L2 snooping filter */
#if 0	/* need to modify here if we want to support this feature for OF packetout to pipeline */
    if(FALSE == SYS_CALLBACK_MGR_HandleSendNdPacket(SYS_MODULE_L2MUX,
                                        mref_handle_p,
                                        dst_mac,
                                        src_mac,
                                        type,
                                        packet_length,
                                        vid))
    {
        L_MM_Mref_Release(&mref_handle_p);
        printf("L2MUX_MGR_SendPacketPipeline:handle send nd packet error.\n");
        return;
    }
#endif	
    L2MUX_MGR_SendPktPipeline(mref_handle_p,
                      packet_length,
                      in_port);
}

/* FUNCTION NAME: L2MUX_MGR_SendMultiPacket
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to a specified port list.
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          dst_mac    -- destination MAC
 *          src_mac    -- source MAC
 *          type       -- Ethernet frame type or IEEE802.3 length field
 *          tag_info   -- 2 bytes tagged information excluding 0x8100 tag type
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          lport_list -- port list including unit and port number
 *                        It's a bit map and port 1 is LSB of first byte.
 *                        tagged_list - port tagged list for the corresponding port list.
 *                        It's a bit map and port 1 is LSB of first byte.
 *          untagged_list -- tell the specified uport_POE_list should be sent with tagged or not
 *          tx_port_count_per_unit -- how many ports per unit to transmit packet.
 *          cos_value  -- the cos value when sending this packet
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:
 *  1. This function is used for upper layer to multicast packet.
 *  2. if tag_info is SYS_TYPE_IGNORE_VID_CHECK, untagged_list shall be the same as lport_list
 */
void L2MUX_MGR_SendMultiPacket(L_MM_Mref_Handle_T *mref_handle_p,
                               UI8_T             dst_mac[6],
                               UI8_T             src_mac[6],
                               UI16_T            type,
                               UI16_T            tag_info,
                               UI32_T            packet_length,
                               UI8_T             *lport_list,
                               UI8_T             *untagged_list,
                               UI32_T            cos_value)

{
    UI32_T     i, j;
    UI8_T      uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T      new_untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T      temp_untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T      active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T      active_untag_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI16_T     vid;
    UI32_T     rx_ifindex;

    /* BODY */

    /* Check VID type to see if need to check STA state
     */
    if ( SYS_TYPE_IGNORE_VID_CHECK == tag_info )
    {
         vid = SYS_TYPE_IGNORE_VID_CHECK;
    }
    else
    {
         vid = tag_info & 0x0fff;                /* 12-bit vid */
    }

    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; i++)
    {
        if (lport_list[i] == 0)
        {
            continue;
        }

        for (j = 0; j < 8; j++)
        {
            if (lport_list[i] & (1 << (7-j)))
            {
                UI32_T lport;
                UI32_T verify_vid;

                lport = i * 8 + j + 1;

                if (vid == SYS_TYPE_IGNORE_VID_CHECK)
                {
                    UI32_T pvid_ifindex, pvid;

                    if (VLAN_OM_GetVlanPortPvid(lport, &pvid_ifindex) == FALSE)
                    {
                        continue;
                    }
                    VLAN_IFINDEX_CONVERTTO_VID(pvid_ifindex, pvid);

                    verify_vid = pvid;
                }
                else
                {
                    verify_vid = vid;
                }

                if (VLAN_OM_IsPortVlanInactiveMember(verify_vid, lport))
                {
                    lport_list[i] &= ~(1 << (7 - j));
                }
            }
        }
    }

    /* Mapping lport list to uport list.
     * If a lport in lport list is a trunk port, following function will
     * return the primary port for that trunk port.
     */
    if ( SWCTRL_LportListToActiveUportListExt( vid,
                                            lport_list,
                                            active_uport_count_per_unit,
                                            uport_list) == FALSE )
    {
         /* cannot get uport list from lport list
          */
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

    if (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_UserPortToLogicalPort(
            mref_handle_p->pkt_info.rx_unit,
            mref_handle_p->pkt_info.rx_port,
            &rx_ifindex))
    {
        if (SWCTRL_GetActiveUportListByPortEgressBlock(rx_ifindex, active_uport_count_per_unit, uport_list))
        {
            L2MUX_MGR_DEBUG_PRINT("%s:%d: EgressBlock: rx_ifindex:%lu uport_list is modified!\n", __func__, __LINE__, rx_ifindex);
        }
    }

    /* Regenerate untagged list for ports that could be trunk port in lport list.
     * If a port is a trunk port, we will have its primary port in uport list
     * after mapping lport list to uport list.
     */
    /* clear new untagged list
     */
    memset(temp_untagged_list, 0, sizeof(temp_untagged_list));
    memset(new_untagged_list, 0, sizeof(new_untagged_list));

    /* Inverting the value of untagged list (only if untagged list is used)
     */
    for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;i++)
         temp_untagged_list[i] = ~untagged_list[i];

    /* Regenerate untagged list. This will handle untagged bit of a trunk port
     * in untagged list, since if a port is a trunk port, we have to transfer
     * its untagged bit to the position of its primary port.
     */
    if ( SWCTRL_LportListToActiveUportList( vid,
                                            temp_untagged_list,
                                            active_untag_uport_count_per_unit,
                                            new_untagged_list ) == FALSE )
    {
         /* cannot get new untagged list from original untagged list
          */
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

    /* Inverting the value of untagged list back. This will be the new
     * untagged list we need for multicasting.
     */
    for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;i++)
         new_untagged_list[i] = ~new_untagged_list[i];

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */
             DebugMultiPacketMessage (mref_handle_p,
                                       dst_mac,
                                       src_mac,
                                       type,
                                       tag_info,
                                       packet_length,
                                       uport_list,
                                       new_untagged_list,
                                       active_uport_count_per_unit,
                                       cos_value );
#endif

    /* L2 snooping filter */
    if(FALSE == SYS_CALLBACK_MGR_HandleSendNdPacket(SYS_MODULE_L2MUX,
                                        mref_handle_p,
                                        dst_mac,
                                        src_mac,
                                        type,
                                        packet_length,
                                        vid))
    {
        L_MM_Mref_Release(&mref_handle_p);
        printf("L2MUX_MGR_SendPacket:handle send nd packet error.\n");
        return;
    }

    /* send out packet to all of the uports in this list(multicast)
     */
    LAN_SendMultiPacket (mref_handle_p,
                          dst_mac,
                          src_mac,
                          type,            /* tag type */
                          tag_info,        /* tag control information */
                          packet_length,
                          uport_list,
                          new_untagged_list,
                          active_uport_count_per_unit,
                          cos_value );

    /* Return successfully
     */
    return;
}  /* End of L2MUX_MGR_SendMultiPacket() */

/* FUNCTION NAME: L2MUX_MGR_SendMultiPacketByVlan
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet by vlan membership config.
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          dst_mac    -- destination MAC
 *          src_mac    -- source MAC
 *          type       -- Ethernet frame type or IEEE802.3 length field
 *          tag_info   -- 2 bytes tagged information excluding 0x8100 tag type
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          lport_list -- port list including unit and port number
 *                        It's a bit map and port 1 is LSB of first byte.
 *                        tagged_list - port tagged list for the corresponding port list.
 *                        It's a bit map and port 1 is LSB of first byte.
 *          cos_value  -- the cos value when sending this packet
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:
 *  1. If lport_list is not specified (i.e. lport_list == NULL), sent pkt to all vlan members.
 *  2. tag_info can't be SYS_TYPE_IGNORE_VID_CHECK.
 */
void L2MUX_MGR_SendMultiPacketByVlan(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T dst_mac[6],
    UI8_T src_mac[6],
    UI16_T type,
    UI16_T tag_info,
    UI32_T packet_length,
    UI8_T *lport_list,
    UI32_T cos_value)
{
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
    UI32_T vid;
    UI32_T byte_idx;
    UI8_T *active_lport_list, *untagged_list;
    BOOL_T has_lport;

    vid = tag_info & 0x0fff;

    if (!VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info))
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

    /* init active_lport_list/untagged_list by vlan info.
     */
    active_lport_list = vlan_info.dot1q_vlan_current_egress_ports;
    untagged_list = vlan_info.dot1q_vlan_current_untagged_ports;
    has_lport = FALSE;

    /* filter inactive port out of active_lport_list.
     *
     * NOTE:
     *   active port is intersection of vlan members and lport_list.
     *
     * if lport_list is not specified, send pkt to all vlan members.
     */
    for (byte_idx = 0; byte_idx < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST; byte_idx++)
    {
        if (lport_list)
        {
            active_lport_list[byte_idx] &= lport_list[byte_idx];
        }

        if (active_lport_list[byte_idx] == 0)
        {
            continue;
        }

        has_lport = TRUE;
    }

    /* no egress port, free mref and do nothing.
     */
    if (!has_lport)
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

    /* sent packet.
     */
    L2MUX_MGR_SendMultiPacket(mref_handle_p, dst_mac, src_mac, type, tag_info, packet_length, active_lport_list, untagged_list, cos_value);
}


#if (SYS_CPNT_ITRI_MIM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - L2MUX_MGR_ITRI_MIM_SetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : lport
 *           status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T L2MUX_MGR_ITRI_MIM_SetStatus(UI32_T lport, BOOL_T status)
{
    l2mux_mgr_itri_mim_status[lport-1] = status;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L2MUX_MGR_ITRI_MIM_GetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : lport
 * OUTPUT  : status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T L2MUX_MGR_ITRI_MIM_GetStatus(UI32_T lport, BOOL_T *status_p)
{
    *status_p = l2mux_mgr_itri_mim_status[lport-1];

    return TRUE;
}
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */


/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for l2mux mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T L2MUX_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    L2MUX_MGR_IPCMsg_T *mgr_msg_p;
    BOOL_T              need_resp = FALSE;

    if(ipcmsg_p == NULL)
        return FALSE;

    mgr_msg_p = (L2MUX_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
#if 0
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        mgr_msg_p->type.result = L2MUX_TYPE_RESULT_FAIL;
        ipcmsg_p->msg_size = L2MUX_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }
#endif

    switch(mgr_msg_p->type.cmd)
    {
        case L2MUX_MGR_IPC_SEND_BPDU:
            L2MUX_MGR_SendBPDU((L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mgr_msg_p->data.SendPacket_data.mref_handle_offset),
                               mgr_msg_p->data.SendPacket_data.dst_mac,
                               mgr_msg_p->data.SendPacket_data.src_mac,
                               mgr_msg_p->data.SendPacket_data.type,
                               mgr_msg_p->data.SendPacket_data.tag_info,
                               mgr_msg_p->data.SendPacket_data.packet_length,
                               mgr_msg_p->data.SendPacket_data.lport,
                               mgr_msg_p->data.SendPacket_data.is_tagged,
                               mgr_msg_p->data.SendPacket_data.cos_value,
                               mgr_msg_p->data.SendPacket_data.is_send_to_trunk_members);
            break;
        case L2MUX_MGR_IPC_SEND_PACKET:
            L2MUX_MGR_SendPacket((L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mgr_msg_p->data.SendPacket_data.mref_handle_offset),
                               mgr_msg_p->data.SendPacket_data.dst_mac,
                               mgr_msg_p->data.SendPacket_data.src_mac,
                               mgr_msg_p->data.SendPacket_data.type,
                               mgr_msg_p->data.SendPacket_data.tag_info,
                               mgr_msg_p->data.SendPacket_data.packet_length,
                               mgr_msg_p->data.SendPacket_data.lport,
                               mgr_msg_p->data.SendPacket_data.is_tagged,
                               mgr_msg_p->data.SendPacket_data.cos_value,
                               mgr_msg_p->data.SendPacket_data.is_send_to_trunk_members);
            break;
        case L2MUX_MGR_IPC_SEND_MULTI_PACKET:
            L2MUX_MGR_SendMultiPacket((L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mgr_msg_p->data.SendMulti_data.mref_handle_offset),
                                      mgr_msg_p->data.SendMulti_data.dst_mac,
                                      mgr_msg_p->data.SendMulti_data.src_mac,
                                      mgr_msg_p->data.SendMulti_data.type,
                                      mgr_msg_p->data.SendMulti_data.tag_info,
                                      mgr_msg_p->data.SendMulti_data.packet_length,
                                      mgr_msg_p->data.SendMulti_data.lport_list,
                                      mgr_msg_p->data.SendMulti_data.untagged_list,
                                      mgr_msg_p->data.SendMulti_data.cos_value);
            break;

        case L2MUX_MGR_IPC_SEND_MULTI_PACKET_BY_VLAN:
            L2MUX_MGR_SendMultiPacketByVlan((L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mgr_msg_p->data.SendMulti_data.mref_handle_offset),
                                      mgr_msg_p->data.SendMulti_data.dst_mac,
                                      mgr_msg_p->data.SendMulti_data.src_mac,
                                      mgr_msg_p->data.SendMulti_data.type,
                                      mgr_msg_p->data.SendMulti_data.tag_info,
                                      mgr_msg_p->data.SendMulti_data.packet_length,
                                      mgr_msg_p->data.SendMulti_data.lport_list_is_valid ?
                                          mgr_msg_p->data.SendMulti_data.lport_list: NULL,
                                      mgr_msg_p->data.SendMulti_data.cos_value);
            break;
        case L2MUX_MGR_IPC_SEND_PACKET_PIPELINE:
            L2MUX_MGR_SendPacketPipeline((L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mgr_msg_p->data.SendPacket_data.mref_handle_offset),
                               mgr_msg_p->data.SendPacket_data.packet_length,
                               mgr_msg_p->data.SendPacket_data.lport /* in_port */);
            break;				
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            mgr_msg_p->type.result = L2MUX_TYPE_RESULT_FAIL;
            ipcmsg_p->msg_size = L2MUX_MGR_MSGBUF_TYPE_SIZE;
            need_resp = TRUE;
    }

    return need_resp;
}


/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: L2MUX_MGR_GetPortOperStatus
 *----------------------------------------------------------------------------------
 * PURPOSE: This function is used to check if port operation status is up or down.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 *
 * OUTPUT:  lport  -- logical port number
 * RETURN:  TRUE : port operation status is UP
 *          FALSE: port doesn't exist or port operation status is DOWN.
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T L2MUX_MGR_GetPortOperStatus(UI32_T unit_no, UI32_T port_no, UI32_T *lport)
{
    /* BODY */

#if (SYS_CPNT_STACKING == TRUE)

    /* Get lport for stacking port
     */
    if ( SYS_ADPT_STACKING_PORT == port_no )
    {
         *lport = SYS_ADPT_STACKING_PORT;  /* for stacking port */
         return TRUE;
    }

#endif /* (SYS_CPNT_STACKING == TRUE) */

    /* Mapping user port ID to logical port ID
     */
    if ( SWCTRL_UserPortToLogicalPort(unit_no, port_no, lport) == SWCTRL_LPORT_UNKNOWN_PORT )
    {
         /* this port is an unknown port
          */
         return FALSE;
    }

    /* Check port operation status.
     */
    if ( SWCTRL_isPortLinkUp(*lport) == FALSE )
    {
       /* link status is down
        */
       return FALSE;
    }

    /* Return successfully
     */
    return TRUE;

}  /* End of L2MUX_MGR_GetPortOperStatus() */

/* FUNCTION NAME: L2MUX_MGR_IsDstMacFiltered
 *----------------------------------------------------------------------------------
 * PURPOSE: This function is used to check if destination address is filtered or not
 *          if bridge desn't implement the applications
 *----------------------------------------------------------------------------------
 * INPUT:
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T L2MUX_MGR_IsDstMacFiltered(UI8_T dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    /* 802.1D-2004: 7.12.6 Reserved addresses
     *
     *   01-80-c2-00-00-0x
     */
    if (dst_mac[0] == 0x01 &&
        dst_mac[1] == 0x80 &&
        dst_mac[2] == 0xc2 &&
        dst_mac[3] == 0x00 &&
        dst_mac[4] == 0x00 &&
        dst_mac[5] >= 0x00 &&
        dst_mac[5] <= 0x0f)
    {
        return TRUE;
    }

    return FALSE;
}

/*
 * Callback Functions Declaration
 */

/* FUNCTION NAME: L2MUX_MGR_STA_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request STA callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_STA_CallbackFunc(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no)
{
    UI32_T               lport = 0;

    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_STA_PACKET);
#endif
#if 0
    if(FALSE == SYS_CALLBACK_MGR_ReceiveStaPacketCallback(SYS_MODULE_L2MUX,
                                              mref_handle_p,
                                              dst_mac,
                                              src_mac,
                                              tag_info,
                                              type,
                                              pkt_length,
                                              lport))
        L_MM_Mref_Release(&mref_handle_p);
#endif
    /* Call callback function to handle this packet
     */
   /*
    if ( L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_STA_PACKET] )
    {
         L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_STA_PACKET](mref_handle_p,
                                                         dst_mac,
                                                         src_mac,
                                                         tag_info,
                                                         type,
                                                         pkt_length,
                                                         lport);
    }
    else
    {
         L_MM_Mref_Release(&mref_handle_p);
    }*/

    return;

}  /* End of L2MUX_MGR_STA_CallbackFunc() */


#if (SYS_CPNT_CLUSTER == TRUE)
void L2MUX_MGR_CLUSTER_CallbackFunc(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no)
{
    UI32_T               lport = 0;
	/* add by zz hong test message */
	/* printf("receive packet in l2mux\n"); */
    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_CLUSTER_PACKET);
#endif

#if 0

    /* Call callback function to handle this packet
     */
    if ( L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_CLUSTER_PACKET] )
    {
        L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_CLUSTER_PACKET](     mref_handle_p  ,
                                                                 dst_mac        ,
                                                                 src_mac        ,
                                                                 tag_info       ,
                                                                 type           ,
                                                                 pkt_length     ,
                                                                 lport          );
    }
    else
    {
         L_MM_Mref_Release(&mref_handle_p);
    }
#endif

    SYS_CALLBACK_MGR_ReceiveClusterPacketCallback(SYS_MODULE_L2MUX,
                                                  mref_handle_p,
                                                  dst_mac,
                                                  src_mac,
                                                  tag_info,
                                                  type,
                                                  pkt_length,
                                                  lport);
    return;

}  /* End of L2MUX_MGR_STA_CallbackFunc() */

#endif

/* FUNCTION NAME: L2MUX_MGR_GVRP_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request GVRP callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_GVRP_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                        UI8_T     *dst_mac,
                                        UI8_T     *src_mac,
                                        UI16_T    tag_info,
                                        UI16_T    type,
                                        UI32_T    pkt_length,
                                        UI32_T    unit_no,
                                        UI32_T    port_no)
{
    UI32_T               lport = 0;

    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_GVRP_PACKET);
#endif
#if 0
    if(FALSE == SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback(SYS_MODULE_L2MUX,
                                               mref_handle_p,
                                               dst_mac,
                                               src_mac,
                                               tag_info,
                                               type,
                                               pkt_length,
                                               lport))
        L_MM_Mref_Release(&mref_handle_p);
#endif

    /* Call callback function to handle this packet
     */
    /*if ( L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_GVRP_PACKET] )
    {
         L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_GVRP_PACKET](mref_handle_p,
                                                          dst_mac,
                                                          src_mac,
                                                          tag_info,
                                                          type,
                                                          pkt_length,
                                                          lport);
    }
    else
    {
         L_MM_Mref_Release(&mref_handle_p);
    }*/

    return;

}  /* End of L2MUX_MGR_GVRP_CallbackFunc() */


typedef struct L2MUX_IpPktFormat_S
{
    UI8_T   ver_len;
    UI8_T   tos;
    UI16_T  length;
    UI32_T  i_dont_use;
    UI8_T   ttl;
    UI8_T   protocol;
    UI16_T  checksum;
    UI32_T  srcIp;
    UI32_T  dstIp;
} __attribute__((packed, aligned(1))) L2MUX_IpPktFormat_T;

#define L2MUX_IPPROTO_IGMP        2           /* group mgmt protocol          */
#define L2MUX_IPPROTO_PIM         103         /* PIM Dense Mode               */

#define L2MUX_IP_HANDLE_TYPE_IGMPSNP    1
#define L2MUX_IP_HANDLE_TYPE_TCPIPSTK   2
#define L2MUX_IP_HANDLE_TYPE_BOTH       3


/* FUNCTION NAME: L2MUX_MGR_IP_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request IP callback function to handle this
 *          packet. This function is used by both IML and IGMPSNP.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_IP_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                      UI8_T     *dst_mac,
                                      UI8_T     *src_mac,
                                      UI16_T    tag_info,
                                      UI16_T    type,
                                      UI32_T    pkt_length,
                                      UI32_T    unit_no,
                                      UI32_T    port_no)
{
    L_MM_Mref_Handle_T*  mref_igmpsnp;
    UI32_T               lport = 0;
    UI32_T               pdu_len;

#if (SYS_CPNT_L2MCAST == TRUE)
    int     handle_type;

    void* src_pkt_p;
    void* dst_pkt_p;

    L2MUX_IpPktFormat_T * iphdr;
    UI32_T  size_before_pdu, length;

#if (SYS_CPNT_STACKING == TRUE)
    UI16_T iuc_ethernet_header_len = 0;
    UI16_T isc_header_len = 0;
    UI16_T stacking_header_len = 0;
    UI16_T ethernet_header_len = 0;
#endif /* SYS_CPNT_STACKING */
#endif /* SYS_CPNT_L2MCAST */

    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */
    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_IP_PACKET);
#endif


#if (SYS_CPNT_L2MCAST == TRUE)
    /* If this IP packet is a multicast packet, it should be sent to IGMPSNP,
     * as well as IML. If it is an unicast packet, send it to IML ONLY.
     */

    /* Check DA to see if this packet is for IGMPSNP(multicast)
     */
    if ( ((*(UI32_T *)(dst_mac)) & 0xffffff00) == 0x01005e00 )
    {
        /* Get PDU */
        if((src_pkt_p=L_MM_Mref_GetPdu(mref_handle_p, &pdu_len))==NULL)
        {
            L_MM_Mref_Release(&mref_handle_p);
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fail", __FUNCTION__);
            return;
        }

        iphdr = src_pkt_p;
        if( iphdr->protocol == L2MUX_IPPROTO_IGMP || iphdr->protocol == L2MUX_IPPROTO_PIM)
        {
            /*
             * For IGMP, MRD, DVMRP and PIM packet: if IPMC is disabled, we
             * don't need send the packet to TCP/IP Stack.
             */
#if (SYS_CPNT_SUPPORTINT_MULTICAST_ROUTING == TRUE)
            handle_type = L2MUX_IP_HANDLE_TYPE_BOTH;
#else
            handle_type = L2MUX_IP_HANDLE_TYPE_IGMPSNP;
#endif
        }
        else if ((iphdr->dstIp & 0xffffff00) == 0xe0000000)
        {
            /*
             * For known multicast packet, e.g., OSPF, RIP and so on, IGMPSNP
             * doesn't care it.
             */
            handle_type = L2MUX_IP_HANDLE_TYPE_TCPIPSTK;
        }
        else
        {
            /*
             * For multicast data stream: if IPMC is disabled, we don't need
             * send the packet to TCP/IP Stack.
             */
#if (SYS_CPNT_SUPPORTINT_MULTICAST_ROUTING == TRUE)
            handle_type = L2MUX_IP_HANDLE_TYPE_BOTH;
#else
            handle_type = L2MUX_IP_HANDLE_TYPE_IGMPSNP;
#endif
        }
    }
    else
    {
        /* Unicast IP packet, it is sent to TCP/IP Stack only. */
        handle_type = L2MUX_IP_HANDLE_TYPE_TCPIPSTK;
    }

    if (handle_type == L2MUX_IP_HANDLE_TYPE_IGMPSNP)
    {
         /* Call IGMPSNP callback function
         */
        if(FALSE == SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback(SYS_MODULE_L2MUX,
                mref_handle_p,
                dst_mac,
                src_mac,
                tag_info,
                type,
                pkt_length,
                lport))
            L_MM_Mref_Release(&mref_handle_p);
    }
    else if (handle_type == L2MUX_IP_HANDLE_TYPE_BOTH)
    {
        /*
         * Copy MREF, and send the original MREF to IGMPSNP
         */
        size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);

#if (SYS_CPNT_STACKING == TRUE)
        ICU_GetIUCEthHeaderLen(&iuc_ethernet_header_len);
        ISC_GetISCHeaderLen(&isc_header_len);
        LAN_GetStackingHeaderLen(&stacking_header_len);
        LAN_GetEthHeaderLen(FALSE, &ethernet_header_len);

        if ((size_before_pdu > ethernet_header_len) &&
                (size_before_pdu <= (ethernet_header_len + iuc_ethernet_header_len +
                                     isc_header_len + stacking_header_len)))
        {
            size_before_pdu -= (iuc_ethernet_header_len + isc_header_len + stacking_header_len);
        }
#endif /* SYS_CPNT_STACKING */
        length = pdu_len + size_before_pdu;

        /* create another MREF for TCP/IP Stack.
         */
        mref_igmpsnp = L_MM_AllocateTxBufferForPktForwarding(mref_handle_p, length,
                L_MM_USER_ID2(SYS_MODULE_L2MUX, L2MUX_TYPE_TRACE_ID_L2MUX_MGR_IP_CALLBACKFUNC));
        if(mref_igmpsnp == NULL)
        {
            L_MM_Mref_Release(&mref_handle_p);
            SYSFUN_Debug_Printf("\r\n%s():L_MM_AllocateTxBufferForPktForwarding fail", __FUNCTION__);
            return;
        }

        /* copy the packet from MAC layer. */
        dst_pkt_p = L_MM_Mref_GetPdu(mref_igmpsnp, &length);

        if (NULL == L_MM_Mref_MovePdu(mref_handle_p, (0 - size_before_pdu), &length))
        {
            printf("%s %d: invalid mref mref_handle_p=%p\n",__FUNCTION__,__LINE__,mref_handle_p);
        }
        src_pkt_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        memcpy(dst_pkt_p, src_pkt_p, length);

        /* Call IGMPSNP callback function
         */
        /* IGMPSNP parses the packet from IP header. */
        L_MM_Mref_MovePdu(mref_handle_p, size_before_pdu, &pdu_len);
        if(FALSE == SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback(SYS_MODULE_L2MUX,
                                                       mref_handle_p,
                                                       dst_mac,
                                                       src_mac,
                                                       tag_info,
                                                       type,
                                                       pkt_length,
                                                       lport))
            L_MM_Mref_Release(&mref_handle_p);

       /* Call IML callback function to handle this packet
        */
        if (NULL == L_MM_Mref_MovePdu(mref_igmpsnp, size_before_pdu, &length))
        {
            printf("%s %d: invalid mref mref_handle_p=%p\n",__FUNCTION__,__LINE__,mref_igmpsnp);
        }

        L2MUX_GROUP_ImlReceivePacketCallbackHandler(mref_igmpsnp, dst_mac, src_mac, tag_info,
                                                type, pkt_length, lport);
    }
    else /* To TCP/IP Stack only */
    {
        /* Call IML callback function to handle this packet
         */
        L2MUX_GROUP_ImlReceivePacketCallbackHandler(mref_handle_p,dst_mac,src_mac,tag_info,
                                                type,pkt_length,lport);
    }
#else /* SYS_CPNT_L2MCAST */

    /* Call IML callback function to handle this packet
     */
    L2MUX_GROUP_ImlReceivePacketCallbackHandler(mref_handle_p,dst_mac,src_mac,tag_info,
                                                type,pkt_length,lport);
#endif /* SYS_CPNT_L2MCAST */

   return;
}  /* End of L2MUX_MGR_IP_CallbackFunc() */

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - L2MUX_MGR_IntrusionMac_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: notifiy intrusion msg to upper layer
 * INPUT   : mref_handle_p, dst_mac, ...
 * OUTPUT  : None
 * RETURN  : TRUE -- upper layer treat it as intrusion / FALSE -- not intrusion, go ahead
 * NOTE    : ** no need to L_MM_Mref_Release() if return FALSE
 * -------------------------------------------------------------------------*/
static BOOL_T L2MUX_MGR_IntrusionMac_CallBack(
        L_MM_Mref_Handle_T *mref_handle_p,
        UI8_T 	    *dst_mac,
        UI8_T       *src_mac,
        UI16_T      tag_info,       /* raw tagged info      */
        UI16_T      ether_type,     /* packet type          */
        UI32_T      pdu_length,     /* pdu length           */
        UI32_T      src_unit,       /* source unit number   */
        UI32_T      src_port,       /* source port number   */
        UI32_T      reason)         /* why packet trap to CPU */
{
    SYS_TYPE_CallBack_T     *fun_list;
    UI32_T                  lport;

    if (IntrusionMsg_callbacklist == NULL)
    {
        SYSFUN_Debug_Printf("L2MUX_MGR_IntrusionMac_CallBack:IntrusionMsg_callbacklist==NULL");
        return FALSE;
    }

    if (SWCTRL_UserPortToLogicalPort(src_unit, src_port, &lport) == SWCTRL_LPORT_UNKNOWN_PORT )
    {
        SYSFUN_Debug_Printf("L2MUX_MGR_IntrusionMac_CallBack:SWCTRL_UserPortToLogicalPort return SWCTRL_LPORT_UNKNOWN_PORT, unit=%lu, port=%lu", src_unit, src_port);
        return FALSE;
    }

    for (fun_list = IntrusionMsg_callbacklist; fun_list; fun_list = fun_list->next)
    {
        if (TRUE == ((L2MUX_MGR_IntrusionCallBackFunction_T)fun_list->func)(mref_handle_p, dst_mac,
                src_mac, tag_info, ether_type, pdu_length, lport, reason))
        {
            /* only one component can handle intrusion action */
            return TRUE;
        }
    }

    return FALSE;
}
#endif/*end of #if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)*/

/* FUNCTION NAME: L2MUX_MGR_DOT1XMAC_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request DOT1XMAC callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T *mref_handle_p -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_DOT1XMAC_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no)
{
    UI32_T               lport = 0;

    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_DOT1XMAC_PACKET);
#endif


    /* Call callback function to handle this packet
     */
    /*SYS_CALLBACK_MGR_ReceiveDOT1XPacketCallback(mref_handle_p,
                                                dst_mac,
                                                src_mac,
                                                tag_info,
                                                type,
                                                pkt_length,
                                                lport); */


    if ( L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_DOT1XMAC_PACKET] )
    {
         L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_DOT1XMAC_PACKET](mref_handle_p,
                                                         dst_mac,
                                                         src_mac,
                                                         tag_info,
                                                         type,
                                                         pkt_length,
                                                         lport);
    }
    else
    {
         L_MM_Mref_Release(&mref_handle_p);
    }

    return;

}  /* End of L2MUX_MGR_DOT1XMAC_CallbackFunc() */

#if (SYS_CPNT_LLDP==TRUE)
/* FUNCTION NAME: L2MUX_MGR_LLDP_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request STA callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T  *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_LLDP_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                        UI8_T     *dst_mac,
                                        UI8_T     *src_mac,
                                        UI16_T    tag_info,
                                        UI16_T    type,
                                        UI32_T    pkt_length,
                                        UI32_T    unit_no,
                                        UI32_T    port_no)
{
    UI32_T               lport = 0;

    /* BODY */

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_LLDP_PACKET);
#endif
#if 0

    if(FALSE == SYS_CALLBACK_MGR_ReceiveLldpPacketCallback(SYS_MODULE_L2MUX,
                                               mref_handle_p,
                                               dst_mac,
                                               src_mac,
                                               tag_info,
                                               type,
                                               pkt_length,
                                               lport))
        L_MM_Mref_Release(&mref_handle_p);

    /* Call callback function to handle this packet
     */
    if ( L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_LLDP_PACKET] )
    {
         L2MUX_MGR_AnnouncePktFunc[L2MUX_MGR_LLDP_PACKET](mref_handle_p,
                                                         dst_mac,
                                                         src_mac,
                                                         tag_info,
                                                         type,
                                                         pkt_length,
                                                         lport);
    }
    else
    {
         L_MM_Mref_Release(&mref_handle_p);
    }
#endif

    return;

}  /* End of L2MUX_MGR_LLDP_CallbackFunc() */
#endif

#if (SYS_CPNT_CFM == TRUE)
void L2MUX_MGR_CFM_CallbackFunc(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no)
{
    UI32_T               lport = 0;
    UI32_T frame_len;
    UI8_T *pdu_p = NULL;

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              L2MUX_MGR_CFM_PACKET);
#endif

    if ((pdu_p = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &frame_len)) == NULL)
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

    if(pdu_p[1] != 0x39 /*APS*/)
    {
      SYS_CALLBACK_MGR_ReceiveCfmPacketCallback(SYS_MODULE_L2MUX,
                                              mref_handle_p,
                                              dst_mac,
                                              src_mac,
                                              tag_info,
                                              type,
                                              pkt_length,
                                              lport);
    }

    return;
}  /* End of L2MUX_MGR_MLDSNP_CallbackFunc() */
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
/* FUNCTION NAME: L2MUX_MGR_PPPOED_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a PPPoE Discovery packet,it calls
 *          this function to request PPPoE IA callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T  *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
void L2MUX_MGR_PPPOED_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              unit_no,
    UI32_T              port_no)
{
    UI32_T               lport = 0;

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */

    DebugReceivePacketMessage(
        mref_handle_p,
        dst_mac,
        src_mac,
        tag_info,
        type,
        pkt_length,
        lport,
        L2MUX_MGR_PPPOED_PACKET);
#endif

    SYS_CALLBACK_MGR_ReceivePppoedPacketCallback(
        SYS_MODULE_L2MUX,
        mref_handle_p,
        dst_mac,
        src_mac,
        tag_info,
        type,
        pkt_length,
        lport);
}  /* End of L2MUX_MGR_PPPOED_CallbackFunc() */
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

/* FUNCTION NAME: L2MUX_MGR_PreprocessPkt_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a R-APS packet,it calls
 *          this function to request ERPS callback function to handle this
 *          packet.
 *----------------------------------------------------------------------------------
 * INPUT:
 *      L_MM_Mref_Handle_T  *mref_handle_p   -- packet buffer and return buffer function pointer.
 *      UI8_T    *dst_mac   -- the destination MAC address of this packet.
 *      UI8_T    *src_mac   -- the source MAC address of this packet.
 *      UI16_T   tag_info   -- tag information
 *      UI16_T   type       -- packet type, e.g.,IP, ARP, or RARP .
 *      UI32_T   pkt_length -- the length of the packet payload.
 *      UI32_T   unit_no    -- user view unit number
 *      UI32_T   port_no    -- user view port number
 *      UI32_T   packet_class -- packet type classified by LAN
 * OUTPUT:  None
 * RETURN:  TURE  - MREF processing is finished and released
 *          FALSE - MREF still need to be processed
 *----------------------------------------------------------------------------------
 * NOTES:   This function is used to dispatch packet to upper layer.
 */
BOOL_T L2MUX_MGR_PreprocessPkt_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              unit_no,
    UI32_T              port_no,
    UI32_T              packet_class)
{
    L2MUX_MGR_PreprocessPktFunction_T *func_p;
    UI32_T               lport = 0;

    l2mux_mgr_counter_rx++;

    if (l2mux_mgr_drop_rx)
    {
         L_MM_Mref_Release(&mref_handle_p);
         return TRUE;
    }

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( L2MUX_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         return FALSE;
    }

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */
    DebugReceivePacketMessage(mref_handle_p,
                              dst_mac,
                              src_mac,
                              tag_info,
                              type,
                              pkt_length,
                              lport,
                              (packet_class | 0x80));
#endif

    for (func_p = l2mux_mgr_preprocess_func_list; *func_p; func_p++)
    {
        if ((*func_p)(
            mref_handle_p,
            dst_mac,
            src_mac,
            tag_info,
            type,
            pkt_length,
            lport,
            packet_class))
        {
            return TRUE;
        }
    }

    return FALSE;
}  /* End of L2MUX_MGR_ERPS_CallbackFunc() */


/* TX functions on L2MUX_MGR module
 */

/* FUNCTION NAME: L2MUX_MGR_SendPkt
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to a specified logical port.
 *          (this function will check the STA state, packet will be drop if not in forwarding state)
 *          (this fun)
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          dst_mac    -- destination MAC
 *          src_mac    -- source MAC
 *          type       -- Ethernet frame type or IEEE802.3 length field
 *          tag_info   -- 2 bytes tagged information excluding 0x8100 tag type
 *          packet_length -- the length of PDU instead of ethernet total packet length
 *                           this function will add the ethernet header length automatically
 *          lport      -- the logical port ( trunk port or NM port )
 *          is_tagged  -- if the packet needs to be sent with tagged
 *          cos_value  -- the cos value when sending this packet
 *          is_send_to_trunk_members -- to tell if this packet is for all trunk members
 *                                      or only for the primary port in this trunk.
 *                                      TRUE: send this packet to all trunk members
 *                                      FALSE: send this packet only to primary port of this trunk
 *          vid        -- vlan ID or SYS_TYPE_IGNORE_VID_CHECK
 *                         if vid != SYS_TYPE_IGNORE_VID_CHECK(just filled in VLAN ID): this function
 *                         will check STA state base on this vid,
 *                         packet will be drop if not in forwarding state
 *
 *                         if vid = SYS_TYPE_IGNORE_VID_CHECK: this funcion will not check STA state, but instead of checking
 *                         if the port is in oper up state, packet will be drop if not in oper up state
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:  1. This function will translate lport to uport. If this lport is a trunk port,
 *           it will also get all trunk members or a primary port for this trunk port.
 *        2. If packet will be sent to all trunk members, this function will regenerate
 *           a new untagged list and multicast the packet.
 */
static void L2MUX_MGR_SendPkt(L_MM_Mref_Handle_T *mref_handle_p,
                          UI8_T       dst_mac[6],
                          UI8_T       src_mac[6],
                          UI16_T      type,
                          UI16_T      tag_info,
                          UI32_T      packet_length,
                          UI32_T      lport,
                          BOOL_T      is_tagged,
                          UI32_T      cos_value,
                          BOOL_T      is_send_to_trunk_members,
                          UI16_T      vid)
{
    UI32_T               unit;
    UI32_T               uport;
    UI8_T                uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T                untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T                active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    SYS_TYPE_Uport_T     active_port;
    SWCTRL_Lport_Type_T  lport_type;
    TRK_MGR_TrunkEntry_T trunk_entry;
    UI32_T               pvid_ifindex, pvid;
    UI32_T               rx_ifindex;

    /* BODY */

    /* Following function is used to check port status.
     * If port operation status is down, do not transmit this packet.
     */

    /* Check port operation status.
     */
    if ( SWCTRL_isPortLinkUp(lport) == FALSE )
    {
        /* link status is down
         */
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (VLAN_OM_GetVlanPortPvid(lport, &pvid_ifindex) == FALSE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
    VLAN_IFINDEX_CONVERTTO_VID(pvid_ifindex, pvid);

    if (is_tagged == FALSE)
    {
        if (VLAN_OM_IsPortVlanInactiveMember(pvid, lport) == TRUE)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }
    else
    {
        if (VLAN_OM_IsPortVlanInactiveMember(vid, lport) == TRUE)
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    if (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_UserPortToLogicalPort(
            mref_handle_p->pkt_info.rx_unit,
            mref_handle_p->pkt_info.rx_port,
            &rx_ifindex))
    {
        if (SWCTRL_IsPortEgressBlock(rx_ifindex, lport))
        {
            L2MUX_MGR_DEBUG_PRINT("%s:%d: EgressBlock: rx_ifindex:%lu lport:%lu\n", __func__, __LINE__, rx_ifindex, lport);
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

    /* Check if this lport is a trunk port
     */
    if ( SWCTRL_LogicalPortIsTrunkPort(lport) )
    {
         /* This is a trunk port. Check to see if we have to send out the packet
          * to all trunk members
          */
         if ( is_send_to_trunk_members )     /* send out packet to all trunk members */
         {
              /* Get port list for all trunk members by trunk ID, in order to multicast
               * packet to all members.
               */
              memset(&trunk_entry, 0, sizeof(TRK_MGR_TrunkEntry_T));

              /* Get trunk ID for this trunk port
               */
              lport_type = SWCTRL_LogicalPortToUserPort(lport, &unit, &uport, &trunk_entry.trunk_index);

              /* Get trunk member(lport list) by trunk ID
               */
              if ( TRK_PMGR_GetTrunkEntry(&trunk_entry) == FALSE )
              {
                  L_MM_Mref_Release(&mref_handle_p);
                  return;
              }

              /* Get uport list and port count(per unit) by trunk lport list
               */
              if( SWCTRL_LportListToActiveUportListExt( vid,
                                                     trunk_entry.trunk_ports,
                                                     active_uport_count_per_unit,
                                                     uport_list) == FALSE )
              {
                  L_MM_Mref_Release(&mref_handle_p);
                  return;
              }

              /* Clear untagged list (default:all 1s)
               * for untagged list -  1:untagged
               *                      0:tagged
               */
              memset(untagged_list, 0xFF, sizeof(untagged_list));

              /* Check if packet has to be tagged or not.
               * If so, all packets should be tagged.
               */
              if (is_tagged)
              {
                  int       i;

                  /* Generate untagged list from uport list for all trunk members
                   */
                  for (i=0; i<SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST;i++)
                       untagged_list[i] ^= uport_list[i];
              }

#ifdef L2MUX_MGR_DEBUG /* print out debugging message */
              DebugMultiPacketMessage ( mref_handle_p,
                                        dst_mac,
                                        src_mac,
                                        type,
                                        tag_info,
                                        packet_length,
                                        uport_list,
                                        untagged_list,
                                        active_uport_count_per_unit,
                                        cos_value );
#endif
              /* Send out packet to all members(multicast)
               */
              LAN_SendMultiPacket(mref_handle_p,
                                  dst_mac,
                                  src_mac,
                                  type,
                                  tag_info,
                                  packet_length,
                                  uport_list,
                                  untagged_list,
                                  active_uport_count_per_unit,
                                  cos_value);

              /* Return successfully
               */
              return;
         }
         else    /* if (is_send_to_trunk_members) */
         {
              /* Get primary port of this trunk from lport
               */
              if ( SWCTRL_LportToActiveUport(vid, lport, &active_port) == FALSE )
              {
                   /* Cannot get the primary port of a trunk
                    */
                   L_MM_Mref_Release(&mref_handle_p);
                   return;
              }

              unit = active_port.unit;
              uport = active_port.port;

         } /* End of if (is_send_to_trunk_members) */

    }
    else   /* if (SWCTRL_LogicalPortIsTrunkPort(lport)) */
    {
         /* This is not a trunk port. Get its uport from lport.
          */
         if ( SWCTRL_LportToActiveUport(vid, lport, &active_port) == FALSE )
         {
             L_MM_Mref_Release(&mref_handle_p);
             return;
         }
         unit = active_port.unit;
         uport = active_port.port;

    } /* End of if (SWCTRL_LogicalPortIsTrunkPort(lport)) */

#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */
             DebugSinglePacketMessage(mref_handle_p,
                                       dst_mac,
                                       src_mac,
                                       type,
                                       tag_info,
                                       packet_length,
                                       unit,
                                       uport,
                                       is_tagged,
                                       cos_value);
#endif

    if (is_tagged == FALSE)
    {
        tag_info = pvid;
    }

    /* Send out packet to this uport(unicast).
     */
    LAN_SendPacket(mref_handle_p,
                   dst_mac,
                   src_mac,
                   type,
                   tag_info,
                   packet_length,
                   unit,
                   uport,
                   is_tagged,
                   cos_value);

    /* Return successfully
     */
    return;

}  /* End of L2MUX_MGR_SendPacket() */

/* TX functions on L2MUX_MGR module
 */

/* FUNCTION NAME: L2MUX_MGR_SendPktPipeline
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to a ASIC pipeline
 *----------------------------------------------------------------------------------------
 * INPUT:   mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                        and memory reference number
 *          packet_length -- ethernet total packet length (DA+SA....)
 *          in_port       -- the physical port we pretend the packet arrived on.(not consider stacking here)
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE: 
 */
static void L2MUX_MGR_SendPktPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port)
{
    /* BODY */
#ifdef L2MUX_MGR_DEBUG  /* print out debugging message */
    //printf("send packet to pipeline packet_length=%d, inpot=%d\n",packet_length,in_port);
          /*   DebugSinglePacketMessage(mref_handle_p,
                                       dst_mac,
                                       src_mac,
                                       type,
                                       tag_info,
                                       packet_length,
                                       0,
                                       in_lport,
                                       is_tagged,
                                       cos_value);*/
#endif

    /* Send out packet to pipeline.
     */
    LAN_SendPacketPipeline(mref_handle_p,
                   packet_length,
                   in_port);

    /* Return successfully
     */
    return;

}  /* End of L2MUX_MGR_SendPktPipeline() */

#if (SYS_CPNT_ITRI_MIM == TRUE)
/* FUNCTION NAME: L2MUX_MGR_ITRI_MIM_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: Packet preprocessing function to implement ITRI MAC-in-MAC
 *----------------------------------------------------------------------------------
 */
static BOOL_T L2MUX_MGR_ITRI_MIM_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              type,
    UI32_T              pkt_length,
    UI32_T              lport,
    UI32_T              packet_class)
{
    if (!l2mux_mgr_itri_mim_status[lport-1])
    {
        return FALSE;
    }

    /* check frame validity
     */
    {
        UI8_T cpu_mac[SYS_ADPT_MAC_ADDR_LEN];

        if (!SWCTRL_GetCpuMac(cpu_mac))
        {
            return FALSE;
        }

        /* check dst_mac == cpu_mac
         */
        if (memcmp(cpu_mac, dst_mac, sizeof(cpu_mac)) != 0)
        {
            return FALSE;
        }

        /* check src_mac[0:2] != 0 and src[3:5] != 0
         */
        if (!(src_mac[0] || src_mac[1] || src_mac[2]) ||
            !(src_mac[3] || src_mac[4] || src_mac[5]))
        {
            return FALSE;
        }
    }

    /*                               0        3        6       9      12 bytes
     * Received encapsulated frame   +--------+--------+-------+-------+
     * dst_mac: CPU MAC              | dst CPU MAC     |src A:B:C:X:Y:Z|
     * src_mac: A:B:C:X:Y:Z          +--------+--------+-------+-------+
     *                                                /       / \       \
     *                                              /       /     \       \
     * Decapsulated frame to forward       +-------+-------+-------+-------+
     * dst_mac: 0:0:0:A:B:C                |dst 0:0:0:A:B:C|src 0:0:0:X:Y:Z|
     * src_mac: 0:0:0:X:Y:Z                +-------+-------+-------+-------+
     *                                     0       3       6       9      12 bytes
     */
    {
        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
        UI32_T vid;

        /* decapsulate dst_mac and src_mac
         */
        dst_mac[0] = dst_mac[1] = dst_mac[2] = 0;
        dst_mac[3] = src_mac[0];
        dst_mac[4] = src_mac[1];
        dst_mac[5] = src_mac[2];
        src_mac[0] = src_mac[1] = src_mac[2] = 0;

        if ((vid = tag_info & 0xfff) == 0)
        {
            VLAN_OM_Dot1qPortVlanEntry_T vlan_port_entry;

            VLAN_OM_GetDot1qPortVlanEntry(lport, &vlan_port_entry);
            VLAN_IFINDEX_CONVERTTO_VID(vlan_port_entry.dot1q_pvid_index, vid);

            tag_info |= vid;
        }

        /* if no member, just release MREF
         */
        if (!VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return TRUE;
        }

        L_BITMAP_PORT_UNSET(vlan_entry.dot1q_vlan_current_egress_ports, lport);

        mref_handle_p->current_usr_id = SYS_MODULE_L2MUX;
        mref_handle_p->next_usr_id    = SYS_MODULE_LAN;

        L2MUX_MGR_SendMultiPacket(
            mref_handle_p,
            dst_mac,
            src_mac,
            type,
            tag_info,
            pkt_length,
            vlan_entry.dot1q_vlan_current_egress_ports,
            vlan_entry.dot1q_vlan_current_untagged_ports,
            (tag_info >> 13));
    }

    return TRUE;

}  /* End of L2MUX_MGR_ITRI_MIM_CallbackFunc() */
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_MGR_RxSnoopDhcp_CallbackFunc
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle dhcp snooping protocol
 * INPUT:
 *    mref_handle_p    --  L_MM_Mref descriptor
 *    dst_mac          --  destination mac address
 *    src_mac          --  source mac address
 *    tag_info         --  vlan tag infomation
 *    pkt_length       --  packet length
 *    lport            --  ingress logical port
 *    packet_class     --  lan packet class
 * OUTPUT:
 *    None.
 * RETURN:
 *    TRUE       --  packet will be handled by dhcp snooping
 *    FALSE      --  packet will be handled by L2MUX
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T
L2MUX_MGR_RxSnoopDhcp_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI16_T              tag_info,
    UI16_T              ether_type,
    UI32_T              pkt_length,
    UI32_T              lport,
    UI32_T              packet_class)
{
    void   *pdu_p=NULL;
    UI32_T pdu_len=0;

#define IPPROTO_UDP         17
#define BOOTP_PORT_S        67          /* Bootp server udp port number */
#define BOOTP_PORT_C        68          /* Bootp client udp port number */
    pdu_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if(NULL == pdu_p)
        return FALSE;

    if(packet_class == LAN_TYPE_IP_PACKET)
    {
        if(ether_type == L2MUX_IPV4_FORMAT)
        {
            L2MUX_MGR_Ipv4PktFormat_T *ipv4_hdr_p =(L2MUX_MGR_Ipv4PktFormat_T*)pdu_p;
            UI32_T                     ip_hdr_len = (0x0f & ipv4_hdr_p->ver_len) * 4;

           /* for fragmented IPv4 packet, it should be sent to kernel.
             * flags:| 00 | 01 | 02 |
             *       |  R | DF | MF | 
             * R: Reserved, DF: Don't Fragment, MF: More Fragement
             * We check if MF is 0 and framgent offset is 0
             */
            if(((ipv4_hdr_p->flags & 0x1)!=0) || (ipv4_hdr_p->fragment_offset != 0))
            {
                return FALSE;
            }
            
            if(ipv4_hdr_p->protocol == IPPROTO_UDP)
            {
                L2MUX_MGR_Udphdr_T *udp_hdr_p = (L2MUX_MGR_Udphdr_T *)(pdu_p + ip_hdr_len);
                if((L_STDLIB_Ntoh16(udp_hdr_p->uh_dport) == BOOTP_PORT_S)||
                   (L_STDLIB_Ntoh16(udp_hdr_p->uh_dport) == BOOTP_PORT_C))
                {
                    if(TRUE == SYS_CALLBACK_MGR_RxSnoopDhcpPacket(
                                   SYS_MODULE_L2MUX,
                                   mref_handle_p,
                                   dst_mac,
                                   src_mac,
                                   tag_info,
                                   ether_type,
                                   pkt_length,
                                   lport))
                    {
                        return TRUE;
                    }
                    else
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    return FALSE;
}


#endif

#ifdef L2MUX_MGR_DEBUG
/* length of hex_str must be triple length of buffer
 * return hex_str
 */
#define BUFFER_TO_HEX_STR(buffer, len, hex_str)             \
({                                                          \
    UI32_T i;                                               \
    sprintf((hex_str), "%02x", (buffer)[0]);                \
    for (i = 1; i < (len); i++)                             \
        sprintf((hex_str) + (i*3-1), " %02x", (buffer)[i]); \
    (hex_str);                                              \
})
#define UPORT_LIST_TO_HEX_STR(port_list)    BUFFER_TO_HEX_STR((port_list), SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, l2mux_mgr_debug_tmp_str)
#define LPORT_LIST_TO_HEX_STR(port_list)    BUFFER_TO_HEX_STR((port_list), SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, l2mux_mgr_debug_tmp_str)
#define MAC_ADDR_TO_HEX_STR(mac)            BUFFER_TO_HEX_STR((mac), SYS_ADPT_MAC_ADDR_LEN, l2mux_mgr_debug_tmp_str)

static char l2mux_mgr_debug_tmp_str[(SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST | 8 /* min size 8 for mac addr */) * 3];

/* FUNCTION NAME: L2MUX_MGR_BackdoorMenu
 *----------------------------------------------------------------------------------
 * PURPOSE: Backdoor main menu
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static void L2MUX_MGR_BackdoorMenu(void)
{
    int ch;

    while (1)
    {
        BACKDOOR_MGR_Print("\r\n 0. exit");
        BACKDOOR_MGR_Printf("\r\n 1. l2mux_mgr_debug_rx: %d", l2mux_mgr_debug_rx);
        BACKDOOR_MGR_Printf("\r\n 2. l2mux_mgr_debug_tx: %d", l2mux_mgr_debug_tx);
        BACKDOOR_MGR_Printf("\r\n 3. l2mux_mgr_debug_msg: %d", l2mux_mgr_debug_msg);
        BACKDOOR_MGR_Printf("\r\n 4. l2mux_mgr_error_msg: %d", l2mux_mgr_error_msg);
        BACKDOOR_MGR_Printf("\r\n 5. l2mux_mgr_drop_rx: %d", l2mux_mgr_drop_rx);
        BACKDOOR_MGR_Printf("\r\n 6. l2mux_mgr_counter_rx: %d", l2mux_mgr_counter_rx);
        BACKDOOR_MGR_Print("\r\n select = ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\r\n", (isprint(ch) ? ch : '?'));

        switch (ch)
        {
            case '0':
                return;
            case '1':
                l2mux_mgr_debug_rx = !l2mux_mgr_debug_rx;
                break;
            case '2':
                l2mux_mgr_debug_tx = !l2mux_mgr_debug_tx;
                break;
            case '3':
                l2mux_mgr_debug_msg = !l2mux_mgr_debug_msg;
                break;
            case '4':
                l2mux_mgr_error_msg = !l2mux_mgr_error_msg;
                break;
            case '5':
                l2mux_mgr_drop_rx = !l2mux_mgr_drop_rx;
                break;
            case '6':
                l2mux_mgr_counter_rx = 0;
                break;
        }
    }
}

/* FUNCTION NAME: DebugReceivePacketMessage
 *----------------------------------------------------------------------------------
 * PURPOSE: print out the information of the packet we received.
 *----------------------------------------------------------------------------------
 * INPUT:   protocol: packet type, e.g., STA, LACP, IP, GVRP, SNPMSNP...
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static void DebugReceivePacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                      UI8_T       dst_mac[6],
                                      UI8_T       src_mac[6],
                                      UI16_T      tag_info,
                                      UI16_T      type,
                                      UI32_T      packet_length,
                                      UI32_T      lport,
                                      UI8_T       protocol)
{
    if (!l2mux_mgr_debug_rx)
    {
        return;
    }

    BACKDOOR_MGR_Printf("\r\nL2MUX_MGR_PacketReceived - %s:%d",
        ((protocol & 0x80) == 0x80 ? "LAN_TYPE_PacketClass_E" : "L2MUX_MGR_PACKET_TYPE_E"),
        (protocol & 0x7f));
    BACKDOOR_MGR_Printf("\r\n-------------------------------------");

    BACKDOOR_MGR_Printf("\r\n        mem_ref : %p", mref_handle_p);
    BACKDOOR_MGR_Printf("\r\n        dst_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    BACKDOOR_MGR_Printf("\r\n        src_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
    BACKDOOR_MGR_Printf("\r\n       tag_info : 0x%4x", tag_info);
    BACKDOOR_MGR_Printf("\r\n           type : 0x%4x", type);
    BACKDOOR_MGR_Printf("\r\n  packet_length : 0x%lx", packet_length);
    BACKDOOR_MGR_Printf("\r\n          lport : 0x%lx", lport);
    BACKDOOR_MGR_Printf("\r\n-------------------------------------\r\n");
}


/* FUNCTION NAME: DebugSinglePacketMessage
 *----------------------------------------------------------------------------------
 * PURPOSE: print out the information of the packet for transmission.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static void DebugSinglePacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                     UI8_T       dst_mac[6],
                                     UI8_T       src_mac[6],
                                     UI16_T      type,
                                     UI16_T      tag_info,
                                     UI32_T      packet_length,
                                     UI32_T      unit,
                                     UI32_T      uport,
                                     BOOL_T      is_tagged,
                                     UI32_T      cos_value)
{
    if (!l2mux_mgr_debug_tx)
    {
        return;
    }

    BACKDOOR_MGR_Printf("\r\nL2MUX_MGR_SendPacket(unicast)");
    BACKDOOR_MGR_Printf("\r\n----------------------------------");

    BACKDOOR_MGR_Printf("\r\n        mem_ref : %p", mref_handle_p);
    BACKDOOR_MGR_Printf("\r\n        dst_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    BACKDOOR_MGR_Printf("\r\n        src_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
    BACKDOOR_MGR_Printf("\r\n           type : 0x%4x", type);
    BACKDOOR_MGR_Printf("\r\n       tag_info : 0x%4x", tag_info);
    BACKDOOR_MGR_Printf("\r\n  packet_length : 0x%lx", packet_length);
    BACKDOOR_MGR_Printf("\r\n           unit : 0x%lx", unit);
    BACKDOOR_MGR_Printf("\r\n          uport : 0x%lx", uport);
    BACKDOOR_MGR_Printf("\r\n      is_tagged : 0x%x", is_tagged);
    BACKDOOR_MGR_Printf("\r\n      cos_value : 0x%lx", cos_value);
    BACKDOOR_MGR_Printf("\r\n----------------------------------\r\n");
}

/* FUNCTION NAME: DebugMultiPacketMessage
 *----------------------------------------------------------------------------------
 * PURPOSE: print out the information of the packet for transmission.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:
 */
static void DebugMultiPacketMessage(L_MM_Mref_Handle_T *mref_handle_p,
                                    UI8_T             dst_mac[6],
                                    UI8_T             src_mac[6],
                                    UI16_T            type,
                                    UI16_T            tag_info,
                                    UI32_T            packet_length,
                                    UI8_T             *uport_list,
                                    UI8_T             *untagged_list,
                                    UI8_T             *tx_port_count_per_unit,
                                    UI32_T            cos_value)
{
    if (!l2mux_mgr_debug_tx)
    {
        return;
    }

    BACKDOOR_MGR_Printf("\r\nL2MUX_MGR_SendMultiPacket(multicast)");
    BACKDOOR_MGR_Printf("\r\n-----------------------------------");

    BACKDOOR_MGR_Printf("\r\n        mem_ref : %p", mref_handle_p);
    BACKDOOR_MGR_Printf("\r\n        rx_port : %lu/%lu",
        mref_handle_p->pkt_info.rx_unit, mref_handle_p->pkt_info.rx_port);
    BACKDOOR_MGR_Printf("\r\n        dst_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
    BACKDOOR_MGR_Printf("\r\n        src_mac : %02x:%02x:%02x:%02x:%02x:%02x",
        src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
    BACKDOOR_MGR_Printf("\r\n           type : 0x%4x", type);
    BACKDOOR_MGR_Printf("\r\n       tag_info : 0x%4x", tag_info);
    BACKDOOR_MGR_Printf("\r\n  packet_length : 0x%lx", packet_length);
    BACKDOOR_MGR_Printf("\r\n     uport_list : %s", UPORT_LIST_TO_HEX_STR(uport_list));
    BACKDOOR_MGR_Printf("\r\n  untagged_list : %s", UPORT_LIST_TO_HEX_STR(untagged_list));
    BACKDOOR_MGR_Printf("\r\n      cos_value : 0x%lx", cos_value);
    BACKDOOR_MGR_Printf("\r\n-----------------------------------\r\n");
}

#endif  /* #ifdef L2MUX_MGR_DEBUG */

