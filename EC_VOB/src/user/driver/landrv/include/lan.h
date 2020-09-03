/*-----------------------------------------------------------------------------
 * Module Name: lan.h
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of lan.c
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. This header file is used for lan.c and also for the those files which
 *    needs to send packet out through lan.c.
 * 2. All those files which needs to receive packet need to register his
 *    receiving API function to lan.c
 * 3. lan.c will not call relative upper layer API to notify that the relative
 *    packet he is interested has arrived.
 * 4. In order to reduce memory copy, we pass MREF_BufDesc_T as an argument.
 * 5. For receiving, we asked NIC driver to allocate buffer by our MAlloc().
 *    The NIC driver can pass the buffer to LAN.C and then we can repackage
 *    the buffer to be MRef_Buffer which allow different layer component to
 *    see the portion what they really care(by changing pointer of pdu) without
 *    changing the real buf pointer(this is really free routine wants)
 * 6. For transimittion, we will ask that upper layer component to allocate
 *    buffer by supported utility and pass the MREF_BufDesc_T to lower lever
 *    code.  And also needs to reserve enough space for lower layer component
 *    to add their header.
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/26/2001 - Jason Hsue, Created
 *    10/31/2002 - Add to support 802.1x
 *    06/21/2006 - Add to support lldp and cluster
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2006
 *-----------------------------------------------------------------------------
 */

#ifndef LAN_H
#define LAN_H

#include "sys_cpnt.h"
#include "l_mm.h"
#include "isc.h"
#include "l_threadgrp.h"
#include "sysrsc_mgr.h"

/* DATA TYPE DECLARATIONS
 */
typedef BOOL_T (*LAN_AnnouncePktFunction_T)
        (UI32_T              src_csc_id,
         L_MM_Mref_Handle_T  *mref_handle_p,
         UI8_T               dst_mac[6],
         UI8_T               src_mac[6],
         UI16_T              tag_info,       /* raw tagged info      */
         UI16_T              packet_type,    /* packet type          */
         UI32_T              pdu_length,     /* pdu length           */
         UI32_T              src_unit,       /* source unit number   */
         UI32_T              src_port,       /* source port number   */
         UI32_T              packet_class);

typedef void (*LAN_AnnouncePktDebugFunction_T)(UI32_T port, UI8_T *pkt, UI32_T size);

typedef BOOL_T (*LAN_IntrusionCallBackFunction_T)(
        UI8_T   *dst_mac,
        UI8_T   *src_mac,
        UI16_T  vid,            /* vlan id              */
        UI16_T  ether_type,     /* packet type          */
        UI32_T  src_unit,       /* source unit number   */
        UI32_T  src_port);      /* source port number   */



BOOL_T LAN_CallByAgent_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_p);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_Register_NA_N_SecurityCheck_Handler
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The function is called by AMTR task to claim his callback routine
 *            to LAN.
 * INPUT    : fun -- the callback function whenever LAN.C receives
 *            an intrusion (include NA packet) packet.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE:    1. Only after AMTR task registers his handler, LAN will pass the intrusion
 *             packet (include NA packet) to AMTR.
 * ----------------------------------------------------------------------------------*/
void LAN_Register_NA_N_SecurityCheck_Handler(LAN_IntrusionCallBackFunction_T fun);


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_Init
 * ----------------------------------------------------------------------------------
 * PURPOSE  : init all resources for LAN task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. LAN.c will not take care the intervention MAC address set and get.
 *               It will be done by system manager.
 * ----------------------------------------------------------------------------------*/
//void LAN_Init (void);
void LAN_InitiateSystemResources(void);

void LAN_AttachSystemResources(void);

void LAN_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


/* FUNCTION NAME: LAN_Create_InterCSC_Relation
 *---------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 *---------------------------------------------------------------------
 * INPUT:   None
 * RETURN:  None.
 *---------------------------------------------------------------------
 * NOTE:
 */
void LAN_Create_InterCSC_Relation(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_CreateTask
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Init create task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. LAN has no task now, we create task in DEV_NICDRV
 * ----------------------------------------------------------------------------------*/
void LAN_CreateTask(void);;

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetTransitionMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : the LAN set into transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_SetTransitionMode(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterTransitionMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterTransitionMode(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterMasterMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as master mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterMasterMode(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterSlaveMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as slave mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterSlaveMode(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This procedure send packet to the specified port.
 * INPUT    : mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                             and memory reference number
 *            dst_mac       -- destination MAC
 *            src_mac       -- source MAC
 *            type          -- Ethernet frame type or IEEE802.3 length field
 *            tag_info      -- 2 bytes tagged information excluding 0x8100 tag type
 *            packet_length -- the length of PDU instead of ethernet total packet length
 *                             this function will add the ethernet header length automatically
 *            unit          -- which unit the packet needs to be sent
 *            port_no:      -- the port number that packet to go
 *            is_tagged     -- if the packet needs to be sent with tagged
 *            cos_value     -- the cos value when sending this packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 * 1. If the packet is BPDU, it will just send the packet to the port.  If the packet is
 *    not BPDU, then we have to get the egress port number by dst_mac either through our
 *    lookup table or by NIC. (For Broadcom, they could get the port number by themselves.
 * 2. If somebody has no idea what the NIC api it will be, here is the description and
 *    example to clarify.
 *    a. the mref_p is a descriptor pointer which can tell us the real buffer address.
 *    b. the real buffer address is allocated from upper layer. The upper layer will
 *       allocate another descriptor as a reference to the buffer.
 *    c. The descriptor will contain the buffer pointer, payload pointer and reference
 *       count, ...
 *    d. Form LAN driver point of view, the caller needs to reserve 14 or 18 byte
 *       for the buffer first 14 or 18 bytes.  And then say the pay load to point to
 *       the offset 14 or 18 of buffer.
 *    e. the rough idea is as followed.
 *
 *          +-------------------+      +-->  +-------------+
 *          |  *buffer          | -----+     |  DA/SA      |
 *          +-------------------+            +-------------+
 *          |  *pdu (payload)   | ---+       | (taginfo)   |
 *          +-------------------+    |       +-------------+
 *          |  :                |    |       |  type       |
 *          |  :                |    +-----> +-------------+
 *          +-------------------+            |  packet     |
 *              mref_p                      |             |
 *                                           |             |
 *                                           +-------------+
 *                                               buffer
 *
 *    f. The idea to implement for LAN driver in Accton is filling DA/SA/(TagInfo)/type
 *       as frame header, and LAN will pass the whole packet frame(including DA/SA..)
 *       to NIC driver.
 *
 * 3. The suggested API for nic will be
 *        XXXNIC_SendPacket(UI8_T   *frame,
 *                          UI32_T  packet_length,
 *                          UI32_T  port,
 *                          void    *packet_free_function,
 *                          void    *packet_free_argument,
 *                          UI32_T  cos_value)
 *
 * 4. NIC driver doesn't need to take care if the packet is tagged or not, LAN will
 *    take care of that.  Therefore, NIC just send it out.
 * 5. Once the packet has been transmitted out, and the frame buffer is no more
 *    need to use, NIC driver needs to call packet_free_function(packet_free_argument)
 *    to free the buffer.
 * ----------------------------------------------------------------------------------*/
 void LAN_SendPacket(L_MM_Mref_Handle_T  *mref_handle_p,
                     UI8_T               dst_mac[6],
                     UI8_T               src_mac[6],
                     UI16_T              type,
                     UI16_T              tag_info,
                     UI32_T              packet_length,
                     UI32_T              unit,
                     UI32_T              port,
                     BOOL_T              is_tagged,
                     UI32_T              cos_value);


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendPacketPipeline
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This procedure send packet to ASIC's packet processing pipeline -- like rx the packet from a front port
 * INPUT    : mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                             and memory reference number
 *            packet_length -- the length of PDU instead of ethernet total packet length
 *                             this function will add the ethernet header length automatically
 *            in_port       -- the physical port we pretend the packet arrived on.(not consider stacking here)
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 * ----------------------------------------------------------------------------------*/
void LAN_SendPacketPipeline(L_MM_Mref_Handle_T  *mref_handle_p,
                     UI32_T              packet_length,
                     UI32_T              in_port);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendMultiPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This procedure send packet to the specify port list.
 * INPUT    : mref_handle_p  -- a descriptor which has a pointer to indicate packet buffer
 *                              and memory reference number
 *            dst_mac        -- destination MAC
 *            src_mac        -- source MAC
 *            type           -- Ethernet frame type or IEEE802.3 length field
 *            tag_info       -- 2 bytes tagged information excluding 0x8100 tag type
 *            packet_length  -- the length of PDU instead of ethernet total packet length
 *                              this function will add the ethernet header length automatically
 *            uport_list     -- port list including unit and port number
 *                              It's a bit map and port 1 is LSB of first byte.
 *                              tagged_list - port tagged list for the corresponding port list.
 *                              It's a bit map and port 1 is LSB of first byte.
 *             untagged_list -- tell the specified uport_list should be sent with tagged or not
 *       port_count_per_unit -- number of port that have to send for each unit
 *            cos_value      -- the cos value when sending this packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE:
 *  1. This function is for upper layer to send multicast packet.
 *  2. Please refer the note of LAN_SendPacket.  The purpose of this API is almost the
 *     the same as previous one except we try to send packet to multi-port for one
 *     specific VLAN at one time.(no matter tagged or not)
 *  3. We expect that NIC driver can transmit the packet through 1Q egress rule to
 *     determine if the packet should be tagged or not through different transmitted
 *     port.
 *  4. The NIC API is expected as follows.
 *        XXXNIC_SendMultiPacket(UI8_T   *frame,
 *                               UI32_T  packet_length,
 *                               UI8_T   *port_list,
 *                               UI8_T   *port_tagged_list,
 *                               void    *packet_free_function,
 *                               void    *packet_free_argument,
 *                               UI32_T  cos_value)
 *  5. NIC driver may not need port_tagged_list, if NIC can determine if the packet
 *     needs to be tagged or not through the output port.  Here is only for the case
 *     if some chip is not designed as the way we thought.
 *  6. The frame we pass will always be tagged.
 * ----------------------------------------------------------------------------------*/
void LAN_SendMultiPacket (L_MM_Mref_Handle_T  *mref_handle_p,
                          UI8_T               dst_mac[6],
                          UI8_T               src_mac[6],
                          UI16_T              type,
                          UI16_T              tag_info,            /* 1p & vid */
                          UI32_T              packet_length,
                          UI8_T               *uport_list,
                          UI8_T               *uport_tagged_list,
                          UI8_T               *tx_port_count_per_unit,
                          UI32_T              cos_value);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendPacketByVlanId
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This procedure send packet to the specified vlan.
 * INPUT    : mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                             and memory reference number
 *            dst_mac       -- destination MAC
 *            src_mac       -- source MAC
 *            type          -- Ethernet frame type or IEEE802.3 length field
 *            tag_info      -- 2 bytes tagged information excluding 0x8100 tag type
 *            packet_length -- the length of PDU instead of ethernet total packet length
 *                             this function will add the ethernet header length automatically
 *            is_tagged     -- if the packet needs to be sent with tagged
 *            cos_value     -- the cos value when sending this packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE:
 * 1. If somebody has no idea what the NIC api it will be, here is the description and
 *    example to clarify.
 *    a. the mem_ref is a descriptor pointer which can tell us the real buffer address.
 *    b. the real buffer address is allocated from upper layer. The upper layer will
 *       allocate another descriptor as a reference to the buffer.
 *    c. The descriptor will contain the buffer pointer, payload pointer and reference
 *       count, ...
 *    d. Form LAN driver point of view, the caller needs to reserve 14 or 18 byte
 *       for the buffer first 14 or 18 bytes.  And then say the pay load to point to
 *       the offset 14 or 18 of buffer.
 *    e. the rough idea is as followed.
 *
 *          +-------------------+      +-->  +-------------+
 *          |  *buffer          | -----+     |  DA/SA      |
 *          +-------------------+            +-------------+
 *          |  *pdu (payload)   | ---+       | (taginfo)   |
 *          +-------------------+    |       +-------------+
 *          |  :                |    |       |  type       |
 *          |  :                |    +-----> +-------------+
 *          +-------------------+            |  packet     |
 *              mem_ref                      |             |
 *                                           |             |
 *                                           +-------------+
 *                                               buffer
 *
 *    f. The idea to implement for LAN driver in Accton is filling DA/SA/(TagInfo)/type
 *       as frame header, and LAN will pass the whole packet frame(including DA/SA..)
 *       to NIC driver.
 *
 * 2. NIC driver doesn't need to take care if the packet is tagged or not, LAN will
 *    take care of that.  Therefore, NIC just send it out.
 * 3. Once the packet has been transmitted out, and the frame buffer is no more
 *    need to use, NIC driver needs to call packet_free_function(packet_free_argument)
 *    to free the buffer.
 * 4. This function is work on broadcast packet and send packet by hardware.
 * ----------------------------------------------------------------------------------*/
 void LAN_SendPacketByVlanId(L_MM_Mref_Handle_T  *mref_handle_p,
                             UI8_T               dst_mac[6],
                             UI8_T               src_mac[6],
                             UI16_T              type,
                             UI16_T              tag_info,
                             UI32_T              packet_length,
                             BOOL_T              is_tagged,
                             UI32_T              cos_value);

/* FUNCTION NAME:   LAN_GetStackingHeaderLen
 * PURPOSE: 
 *          Get the Stacking Header length of ISC packet
 * INPUT:   
 *          None
 * OUTPUT:  
 *          length
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T LAN_GetStackingHeaderLen(UI16_T *length);


/* FUNCTION NAME:   LAN_GetEthHeaderLen
 * PURPOSE: 
 *          Get the Ethernet Header length of packet
 * INPUT:   
 *          untagged    -- If TRUE, get the untagged ethernet header. 
 *                         Otherwise, get the untagged ethernet header
 *                         plus one tag.
 *
 * OUTPUT:  
 *          length
 * RETURN:  
 *          TRUE        -- success
 *          FALSE       -- fail
 * NOTES:   
 */
BOOL_T LAN_GetEthHeaderLen(BOOL_T untagged, UI16_T *length);


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_CallByAgent_ISC_Handler
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This function will be call by ISC_AGENT if receive ISC packet for LAN
 * INPUT    : key     -- key of ISC
 *            mref_p  -- a descriptor which has a pointer to indicate packet buffer
 *                       and memory reference number.
 * OUTPUT   : None
 * RETURN   : TRUE    -- process success
 *            FALSE   -- process fail
 * NOTE     : 1.This function will be call if service id is ISC_LAN_CALLBYAGENT_SID
 * ----------------------------------------------------------------------------------*/
//kh_shi BOOL_T LAN_CallByAgent_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_p);

#if (SYS_CPNT_RUNTIME_SWITCH_DIAG == TRUE)
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_Register_Debug_Handler
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The function is called by SWDIAG task to claim his callback routine
 *            to LAN.
 * INPUT    : callBackFunction -- the callback function whenever LAN.C receives a packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. Only after SWDIAG task registers his handler, LAN will pass the packet
 *               to SWDIAG
 * ----------------------------------------------------------------------------------*/
void LAN_Register_Debug_Handler(LAN_AnnouncePktDebugFunction_T callBackFunction);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RxDebug_Enable
 * ----------------------------------------------------------------------------------
 * PURPOSE  : set lan_debug_rx_int to TRUE, all packet arrive LAN will pass to SWDIAG
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_RxDebug_Enable(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RxDebug_Enable
 * ----------------------------------------------------------------------------------
 * PURPOSE  : set lan_debug_rx_int to FASLE, packets arrive LAN will not pass to SWDIAG
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_RxDebug_Disable(void);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendLoopBackPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This procedure send packet to the own unit to perform test
 * INPUT    : mref_handle_p -- a descriptor which has a pointer to indicate packet buffer
 *                             and memory reference number
 *            dst_mac       -- destination MAC
 *            src_mac       -- source MAC
 *            type          -- Ethernet frame type or IEEE802.3 length field
 *            tag_info      -- 2 bytes tagged information excluding 0x8100 tag type
 *            packet_length -- the length of PDU instead of ethernet total packet length
 *                             this function will add the ethernet header length automatically
 *            cos_value     -- the cos value when sending this packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_SendLoopBackPacket (L_MM_Mref_Handle_T  *mref_handle_p,
                             UI8_T               dst_mac[6],
                             UI8_T               src_mac[6],
                             UI16_T              type,
                             UI16_T              tag_info,            /* 1p & vid */
                             UI32_T              packet_length,
                             UI8_T               *uport_list,
                             UI8_T               *uport_untagged_list,
                             UI8_T               *tx_port_count_per_unit,
                             UI32_T              cos_value);
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetOamLoopback
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable a port into OAM loopback mode
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- TRUE: enable loopback, FALSE: disable loopback
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetOamLoopback(UI32_T unit, UI32_T port, BOOL_T enable);
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetInternalLoopback
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable a port into internal loopback mode
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- TRUE: enable internal loopback, FALSE: disable internal loopback
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetInternalLoopback(UI32_T unit, UI32_T port, BOOL_T enable);
#endif

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetPortLearning
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Set the port learning status of a port
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- port learning status
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetPortLearning(UI32_T unit, UI32_T port, BOOL_T enable);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetVlanLearningStatus
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid       -- target vid
 *            learning  -- learning status
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------
 */
BOOL_T LAN_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetPortSecurity
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Set the port security status of a port
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- port security status
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetPortSecurity(UI32_T unit, UI32_T port, BOOL_T enable);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetPortDiscardUntaggedFrame
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Set the port discard untagged frame or not
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- discard untagged frame or not
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetPortDiscardTaggedFrame
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Set the port discard tagged frame or not
 * INPUT    : unit      -- target unit
 *            port:     -- target port
 *            enable    -- discard tagged frame or not
 * OUTPUT   : None
 * RETURN   : TRUE   -- success
 *            FALSE  -- fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
BOOL_T LAN_SetPortDiscardTaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_DispatchPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Anounce packet to upper layer
 * INPUT    : mref_p      -- memory reference of receive packet
 *            dst_mac     -- Destination address
 *            src_mac     -- Source address
 *            tag_info    -- raw tagged info of the packet
 *            ether_type  -- packet type
 *            pdu_length  -- pdu length
 *            src_unit    -- source unit
 *            src_port    -- source port
 *            packet_type -- packet type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_DispatchPacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI16_T tag_info,
    UI16_T ether_type,
    UI32_T pdu_length,
    UI32_T src_unit,
    UI32_T src_port,
    UI32_T packet_type);

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_MREF_ReleaseReference
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This function is used by dev_nicdrv_gateway tx related APIs as
 *            free packet callback function.
 * INPUT    : unit        -- not used, do not care
 *            packet      -- tx buffer to be freed
 *            cookie      -- pointer to L_MM_Mref_Handle_T for the corresponding
 *                           tx buffer to be freed
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_MREF_ReleaseReference(I32_T unit, void *packet, void *cookie);

/* exported for a driver_proc to support backdoor from linux shell without using Simba/CLI  backdoor
 */
void LAN_BackdoorEntrance(void);

#endif

