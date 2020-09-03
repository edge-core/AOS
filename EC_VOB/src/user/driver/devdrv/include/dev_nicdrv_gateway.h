#ifndef DEV_NICDRV_GATEWAY_H
#define DEV_NICDRV_GATEWAY_H

#include "l_mm.h"

#define    DEV_NICDRV_GATEWAY_BC_PKT             0xFFFF
#define    DEV_NICDRV_GATEWAY_MC_PKT             0xFFF0
#define    DEV_NICDRV_GATEWAY_CPU_PKT            0xFF80
#define    DEV_NICDRV_GATEWAY_IUC_ETHERNET_TYPE  0xFFFF

#define DEV_NICDRV_GATEWAY_RX_PACKET_INFO_MAGIC_WORD    0x35792468

typedef struct free_info
{
     UI8_T dev_id;
     UI8_T q_id;
     UI8_T *buf;
     UI8_T *next_info;
}free_info_t;

typedef struct
{
    UI32_T magic;       /* DEV_NICDRV_GATEWAY_RX_PACKET_INFO_MAGIC_WORD */

    UI32_T module_id;
    UI32_T device_id;
    UI32_T phy_port;
    UI32_T reason;
    UI32_T cos;
    UI32_T pkt_len;
    void *pkt_buf;
    void *cookie;

    UI32_T ingress_vlan;
    BOOL_T is_single_inner_tagged;
    BOOL_T rx_is_tagged;
    BOOL_T pkt_is_truncated;
    BOOL_T workaround_vlan_xlate;   /* TRUE if need to perform vlan xlate by sw */

    UI64_T rx_timestamp;
} DEV_NICDRV_GATEWAY_RxPacketInfo_T;

BOOL_T DEV_NICDRV_GATEWAY_FreePacket(void *buf);

/*
 * Function: DEV_NICDRV_GATEWAY_ReceivePacketReason
 * Purpose:
 *	Convert receive packet reason
 * Parameters:
 *	[in]reason          - packet receive from chip to CPU with reason
 * Returns: 
 *	DEV_NICDRV_GATEWAY_RecvReason_T - return reason type
 * Notes:   It's dependent by chip 
 */
UI32_T DEV_NICDRV_GATEWAY_ReceivePacketReason(UI32_T reason);

/* This is for the translation for cos_value when we send a pakcet or packets.
 * Broadcom's nic is always taken the cos_value has been shifted 3 bits left
 * when they recieve the argument.
 */
UI32_T DEV_NICDRV_GATEWAY_CosValue(UI32_T val);

#define HIGIG_HDR_SIZE	         ((int) sizeof(soc_higig_hdr_t))

/*
 * Function: DEV_NICDRV_GATEWAY_Init
 * Purpose:
 *	Initialize the BCM PMUX driver
 * Parameters:
 *	[in]packet_size         - packet size use for allocating incoming packets.
 *	[in]f_alloc             - allocate routine for data buffers.
 *	[in]f_free              - free routine for data buffers.
 *	[in]callback_function   - function to call for that driver.
 *	[in]cookie              - cookie passed to driver when packet arrives.
 * Returns: 
 *	None
 * Notes: 
 */
void DEV_NICDRV_GATEWAY_Init(UI32_T packet_size, 
                     void *f_alloc, 
                     void *f_free, 
                     void *callback_function, 
                     void *cookie);

/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketToPort
 * Purpose:
 *	Gateway API for Sending a packet asynchronously.
 * Parameters:
 *	[in]unit                - The unit to send.
 *	[in]port                - the port to send .
 *	[in]is_tagged           - tag or not.
 *	[in]tag_info            - vlan tag .
 *	[in]packet              - physical address of packet.
 *	[in]length              - length (in bytes) of packet.
 *	[in]cos_value           - the cos value when sending this packet.
 *	[in]callback_function   - callback function to release buffer address.
 *  [in]cookie              - mem_re    
 * Returns: 
 *	None.
 * Notes: 
 */
BOOL_T DEV_NICDRV_GATEWAY_SendPacketToPort(UI32_T unit, 
                                   UI32_T port,
                                   BOOL_T is_tagged,
                                   UI16_T  tag_info,
                                   void   *packet,
                                   UI32_T length,                                 
                                   UI32_T cos_value,
                                   void   *callback_function, 
                                   void   *cookie);

/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketByPort
 * Purpose:
 *	Gateway API for Sending a packet asynchronously.
 * Parameters:
 *	[in]unit                - The unit to send.
 *	[in]port                - the port to send .
 *	[in]is_tagged           - tag or not.
 *	[in]packet              - physical address of packet.
 *	[in]length              - length (in bytes) of packet.
 *	[in]cos_value           - the cos value when sending this packet.
 *	[in]callback_function   - callback function to release buffer address.
 *  [in]cookie              - mem_re    
 * Returns: 
 *	None.
 * Notes: 
 */
BOOL_T DEV_NICDRV_GATEWAY_SendPacketByPort(UI32_T unit, 
                                   UI32_T port,
                                   BOOL_T is_tagged,
                                   void   *packet,
                                   UI32_T length,                                 
                                   UI32_T cos_value,
                                   void   *callback_function, 
                                   void   *cookie);

/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketByPortList
 * Purpose:
 *	Gateway API for Sending a packet asynchronously.
 * Parameters:
 *	[in]port_count          - Total number of ports to send.
 *	[in]port_list           - StrataSwitch port bit mask.
 *	[in]untagged_port_list  - StrataSwitch untag port bit mask.
 *	[in]callback_function   - callback function to release buffer address.
 *	[in]cookie              - not used.
 *	[in]packet              - physical address of packet.
 *	[in]length              - length (in bytes) of packet.
 *	[in]cos_value           - the cos value when sending this packet.
 * Returns: 
 *	None.
 * Notes: 
 */
BOOL_T DEV_NICDRV_GATEWAY_SendPacketByPortList(UI32_T port_count, 
                                       UI8_T *port_list, 
                                       UI8_T *untagged_port_list, 
                                       void *packet, 
                                       UI32_T length, 
                                       UI32_T cos_value,
                                       void   *callback_function, 
                                       void   *cookie);

/* To send external loop back packet in manufacture mode
 * added by mikeliu.081004 */
BOOL_T DEV_NICDRV_GATEWAY_SendLoopBackPacketByPortList(UI32_T port_count, 
                                               UI8_T *port_list, 
                                               UI8_T *untagged_port_list, 
                                               void *packet, 
                                               UI32_T length, 
                                               UI32_T cos_value,
                                               void   *callback_function, 
                                               void   *cookie);

/* FUNCTION NAME:   DEV_NICDRV_GATEWAY_SendHigigPackPacket
 *----------------------------------------------------------------------------------
 * PURPOSE: Pack A higig header and Ethernet Header , Send a  packet
 *----------------------------------------------------------------------------------
 * INPUT:   mref_handle_p - points to the data block that holds the packet content
 *          dst_unit      - destination unit 
 *          uplink_downlink
 * OUTPUT:  None
 * RETUEN:  TRUE - success
 *          FALSE - fail
 *----------------------------------------------------------------------------------
 * NOTES: 
 */

BOOL_T DEV_NICDRV_GATEWAY_SendHigigPackPacket(L_MM_Mref_Handle_T *mref_handle_p,UI16_T dst_unit,UI8_T uplink_downlink);

#ifndef INCLUDE_DIAG
/* FUNCTION NAME:   DEV_NICDRV_GATEWAY_SendIUCPacket
 *----------------------------------------------------------------------------------
 * PURPOSE: Pack A higig header and Ethernet Header , Send a  packet
 *----------------------------------------------------------------------------------
 * INPUT:   mref_handle_p - points to the data block that holds the packet content
 *          dst_unit - destination unit 
 *             DEV_NICDRV_GATEWAY_BC_PKT: send packet by boardcast
 *             DEV_NICDRV_GATEWAY_MC_PKT: send packet by multicast
 *             DEV_NICDRV_GATEWAY_CPU_PKT: send packet to cpu
 *             1-16: driver unit id, range=1-16
 *          uplink_downlink: uplink or downlink port for 5670
 *          priority: packet proirity                   
 * OUTPUT:  None
 * RETUEN:  TRUE - success
 *          FALSE - fail
 *----------------------------------------------------------------------------------
 * NOTES: 
 */

BOOL_T DEV_NICDRV_GATEWAY_SendIUCPacket(L_MM_Mref_Handle_T *mref_handle_p, UI16_T dst_unit,UI8_T uplink_downlink, UI32_T priority);

#endif

/*
 * Function: DEV_NICDRV_GATEWAY_SendXbarPacketByPortList
 * Purpose:
 *	Gateway API for Sending a packet asynchronously.
 * Parameters:
 *	[in]port_count          - Total number of ports to send.
 *	[in]port_list           - StrataSwitch port bit mask.
 *	[in]untagged_port_list  - StrataSwitch untag port bit mask.
 *	[in]callback_function   - callback function to release buffer address.
 *	[in]cookie              - not used.
 *	[in]packet              - physical address of packet.
 *	[in]length              - length (in bytes) of packet.
 *	[in]cos_value           - the cos value when sending this packet.
 * Returns: 
 *	None.
 * Notes: The frame we pass will always be tagged.
 */
BOOL_T DEV_NICDRV_GATEWAY_SendXbarPacketByPortList(UI32_T port_count, 
                                           UI8_T  *port_list, 
                                           UI8_T  *untagged_port_list, 
                                           void   *packet, 
                                           UI32_T length, 
                                           UI32_T cos_value,
                                           void   *callback_function, 
                                           void   *cookie);/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketByVid
 * Purpose:
 *	Gateway API for Sending a packet by vlan ID
 * Parameters:
 *	[in]tag_info            - Specific VLAN to send.
 *	[in]packet              - packet.
 *	[in]length              - packet length.
 *	[in]cos_value           - class of service for packet.
 *	[in]callback_function   - callback function to release buffer address.
 *	[in]cookie              - not used.
 * Returns: 
 *	None.
 * Notes: The frame we pass will always be tagged.
 */
BOOL_T DEV_NICDRV_GATEWAY_SendPacketByVid( UI32_T  tag_info,
                                   void   *packet,
                                   UI32_T length,                                 
                                   UI32_T cos_value,
                                   void   *callback_function, 
                                   void   *cookie);

/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketToCraftPort
 * Purpose:
 *	Gateway API for sending a packet to craft port
 * Parameters:
 *	[in]packet              - physical address of the packet.
 *	[in]length              - length (in bytes) of the packet.
 *	[in]mref_handle_p       - mref handle of the packet.
 * Returns:
 *	None.
 * Notes: This function is only defined when SYS_CPNT_CRAFT_PORT is TRUE
 *        and SYS_CPNT_CRAFT_PORT_MODE is SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT.
 */
void DEV_NICDRV_GATEWAY_SendPacketToCraftPort(void *packet,
                                              UI32_T length,
                                              L_MM_Mref_Handle_T *mref_handle_p);

/*
 * Function: DEV_NICDRV_GATEWAY_SendPacketToPipeline
 * Purpose:
 *	Gateway API for Sending a packet to ASIC packet process pipeline -- like rx from a front port.
 * Parameters:
 *	[in]in_port             - pretended input port
 *	[in]cookie              - cookie.
 *	[in]packet              - physical address of packet.
 *	[in]length              - length (in bytes) of packet.
 * Returns:
 *	None.
 * Notes:
 */
BOOL_T DEV_NICDRV_GATEWAY_SendPacketToPipeline(
                                   UI32_T port,
                                   UI8_T  *packet,
                                   UI32_T length,
                                   void   *cookie);

#if (SYS_CPNT_CPU_CONTROL_PACKET_RATE_LIMIT == TRUE)
/*
 * Function: DEV_NICDRV_GATEWAY_ResetCpuControlPacketRateLimit
 * Purpose:
 *	    Reset CPU control packet rate limit control to original status
 * Parameter:
 *      None
 * Return:
 *	    None
 * Note: 
 *      This function is called from DEV_NICDRV_Task when timer event occurs
 *      CPU control packet currently include ICMP redirect and L3 show path
 */
void DEV_NICDRV_GATEWAY_ResetCpuControlPacketRateLimit(void);
#endif
/* exported for a driver_proc to support backdoor from linux shell without using Simba/CLI  backdoor
 */
void DEV_NICDRV_GATEWAY_BackDoor_Menu(void);

#endif
