/* Module Name: IML_MGR.H
 * Purpose:
 *      IML plays the role between Layer-2 (Data Link Layer) and Layer-3 (Network Layer),
 *      knows what is routing interface, what is VLAN, and what is port, here is lport.
 *      For incoming packet, IML inspects received packet, filters unneeded packet,
 *      then dispatchs to proper layer-3 components (ARP packet handler, IP packet handler).
 *      For outgoing packet, IML finds out what's the member of the routing interface,
 *      discard unknown routing interface packet to reduce unnecessary transmittion.
 *
 *
 * Notes:
 *      1.
 *
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.01.24  --  William,    Created
 *      2003.03.31  --  Garfield,   Add Callback functions for HSRP
 *      2003.04.03  --  Garfield,   Add Callback functions for VRRP
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002.
 */

#ifndef     _IML_MGR_H
#define     _IML_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "vlan_type.h"
#include "l_mm.h"
#include "leaf_es3626a.h"               /*  for IP Address Mode/Method  */

/* NAME CONSTANT DECLARATIONS
 */
#define IML_MGR_MSGBUF_TYPE_SIZE        sizeof(union IML_MGR_IPCMsg_Type_U)

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in L2MUX_OM_IPCMsg_T.data
 */
#define IML_MGR_GET_MSGBUFSIZE(struct_name) (IML_MGR_MSGBUF_TYPE_SIZE + sizeof(struct struct_name))

/* DATA TYPE DECLARATIONS
 */
typedef struct IML_MGR_RecvPktFromTCPIPStackArgs_S
{
    UI16_T  tag_info;
    UI16_T  packet_type;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   src_mac[SYS_ADPT_MAC_ADDR_LEN];
} IML_MGR_RecvPktFromTCPIPStackArgs_T;

typedef struct
{
    union IML_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;         /* for sending IPC request */
        UI32_T result;      /* for response            */
        BOOL_T result_bool; /* respond bool return     */
    } type;
    
    union
    {
        struct IML_MGR_IPCMsg_SendPacket_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T ifindex;
            UI32_T packet_length;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI16_T packet_type;
            BOOL_T forward;
        }SendPacket_data;
        
        struct IML_MGR_IPCMsg_BroadcastBootPPacket_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T vid_ifindex;
            UI32_T packet_length;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
        }BroadcastBootPPacket_data;
        
        struct IML_MGR_IPCMsg_SendBootPPacketByDhcpSnp_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T vid;
            UI32_T packet_length;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
            VLAN_OM_Dot1qVlanCurrentEntry_T dot1q_entry;
        }SendBootPPacketByDhcpSnp_data;
        
        struct IML_MGR_IPCMsg_ManagementVid_Data_S
        {
            UI32_T vid_ifindex;
        }ManagementVid_data;    

        struct IML_MGR_IPCMsg_WebauthClient_Data_S
        {
            UI32_T  src_ip;
            UI32_T  org_dip;
            UI32_T  lport;
            UI16_T  src_tcpport;
        }WebauthClient_data;

        struct IML_MGR_IPCMsg_WebauthStatus_Data_S
        {
            UI32_T  lport;
            BOOL_T  status;
        }WebauthStatus_data;

        struct IML_MGR_IPCMsg_ArpInspection_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T vid;
            UI32_T packet_length;
            UI32_T src_lport_ifindex;
            UI16_T packet_type;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
            
            
        }ArpInspection_data;

        struct IML_MGR_IPCMsg_Dhcp_Relay_Option82_Broadcast_Except_Inport_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T vid_ifindex; 
            UI32_T packet_length;
            UI32_T src_lport_ifindex;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
        }BroadcastExceptInport_data;

        struct IML_MGR_IPCMsg_Dhcp_Relay_Option82_Send_Pkt_To_Port_Data_S
        {
            I32_T  mref_handle_offset;
            UI32_T vid;
            UI32_T packet_length;
            UI32_T out_lport;
            UI16_T packet_type;
            UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T  src_mac[SYS_ADPT_MAC_ADDR_LEN];
        }SendPktToPort_data;
    } data;
} IML_MGR_IPCMsg_T;

/* definitions of command in IML which will be used in ipc message
 */
enum
{
    IML_MGR_IPC_GET_MANAGEMENT_VID = 0,
    IML_MGR_IPC_SET_MANAGEMENT_VID,
    IML_MGR_IPC_SEND_PACKET,
    IML_MGR_IPC_SEND_ARP_INSPECTION_PACKET,
    IML_MGR_IPC_GETORGDIPNLOPORTBYSIPSTCPPORT,
    IML_MGR_IPC_SETWEBAUTHSTATUS,
    IML_MGR_IPC_BROADCAST_EXCEPT_IN_PORT,
    IML_MGR_IPC_SEND_TO_PORT,
    IML_MGR_IPC_SEND_PACKET_BY_VLAN_ENTRY,
};


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : IML_MGR_Init
 * PURPOSE:
 *      Initialize IML_MGR used system resource, eg. protection semaphore.
 *      Clear all working space.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void    IML_MGR_Init(void);


#if (SYS_CPNT_DHCPV6SNP == TRUE)
/* FUNCTION NAME: IML_MGR_SendPktByVlanEntry
 * PURPOSE: Send multiple packet by DHCPV6SNP according to vlan entry
 * INPUT:  mref_handle_p -- L_MREF descriptor
 *             packet_length  -- packet length
 *             vid                  -- vid
 *             dst_mac_p      -- destination mac address
 *             src_mac_p      -- source mac address
 *             vlan_entry_p      -- the main purpose is to get the egress port list.
 * OUTPUT: none
 * RETURN: TRUE/FALSE
 * NOTES:  called by DHCPV6SNP  which is in dhcpsnp_engine.c
 */
BOOL_T IML_MGR_SendPktByVlanEntry(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, 
      UI32_T vid,UI8_T *dst_mac_p, UI8_T *src_mac_p, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry_p);
#endif

/* FUNCTION NAME: IML_MGR_GetSystemPerformanceCounter
 * PURPOSE: export function to get the packet counters in IML
 * INPUT:  ip_packet_from_lan   -- ip packet from lan by taken lan context
 *         ip_packet_to_p2      -- ip packet send to P2 by taken IML context
 *         arp_packet_from_lan  -- arp packet from lan by taken lan context
 *         arp_packet_to_p2     -- arp packet send to P2 by taken IML context
 *         arp_packet_to_amtrl3 -- arp packet send to P2 by taken IML context
 *         queue_full_drop      -- the counter of dropping in IML when msg queue full.
 * OUTPUT: none
 * RETURN: TRUE  -- success to get
 *         FALSE -- fail to get
 * NOTES:
 */
BOOL_T IML_MGR_GetSystemPerformanceCounter(int *ip_packet_from_lan,
                                           int *ip_packet_to_p2,
                                           int *arp_packet_from_lan,
                                           int *arp_packet_to_p2,
                                           int *arp_packet_to_amtrl3,
                                           int *queue_full_drop);    

/* FUNCTION NAME: IML_MGR_ClearSystemPerformanceCounter
 * PURPOSE: export function to clear the packet counters in IML
 * INPUT:  none
 * OUTPUT: none
 * RETURN: void
 * NOTES:
 */
void IML_MGR_ClearSystemPerformanceCounter(void);    

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
/* FUNCTION NAME : IML_MGR_IsMgmtPacket
 * PURPOSE:
 *      Check if the packet is management packet (TELNET/WEB/SNMP).
 * INPUT:
 *      payload     -- payload of frame, excluded frame header
 *      in_or_out_pkt -- IML_OUT_MGMT_PKT : Check source port
 *                       IML_IN_MGMT_PKT  : Check dest port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE --- The packet is management packet
 *      FALSE ---- The packet is not management packet
 *
 * NOTES:
 *      None.
 */
BOOL_T IML_MGR_IsMgmtPacket(void *payload, UI8_T in_or_out_pkt);

#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

/*........................................
 *  Mercury Switch API -- for backward compatible,
 *  all functions will be obsoleted after layer-3 merged.
 *........................................
 */

void IML_MGR_InitiateProcessResources(void);
void IML_MGR_InitiateSystemResources(void);
void IML_MGR_AttachSystemResources(void);
void IML_MGR_Create_InterCSC_Relation(void);
BOOL_T IML_MGR_CreateTask(void); 
void IML_MGR_SetTransitionMode(void);
void IML_MGR_EnterTransitionMode (void);
void IML_MGR_EnterMasterMode (void);
void IML_MGR_EnterSlaveMode (void);
void IML_MGR_GetManagementVid(UI32_T *vid_ifIndex);
UI32_T IML_MGR_SetManagementVid(UI32_T vid_ifIndex);
void IML_MGR_RecvPacket(L_MM_Mref_Handle_T *mref_handle_p, UI8_T dst_mac[6], UI8_T src_mac[6],
                                UI16_T tag_info, UI16_T etherType, UI32_T packet_length, UI32_T l_port);
void IML_MGR_RxLanPacket(L_MM_Mref_Handle_T  *mref_handle_p,
                              UI8_T     *dst_mac,
                              UI8_T     *src_mac,
                              UI16_T    tag_info,
                              UI16_T    type,
                              UI32_T    pkt_length,
                              UI32_T    unit_no,
                              UI32_T    port_no);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : IML_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for iml mgr.
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
BOOL_T IML_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);
 
#if (SYS_CPNT_DAI == TRUE)
/* FUNCTION NAME: IML_MGR_ARP_Inspection_SendPkt
 * PURPOSE: 
 *      Send packet.
 * INPUT:  
 *         *mem_ref -- L_MREF descriptor
 *         vid -- the vlan id
 *         packet_length -- packet length
 *         *dst_mac --
 *         *src_mac -- 
 *         packet_type --
 * OUTPUT: 
 *      None.
 * RETURN: 
 *      successful (0), failed (-1)
 * NOTES:
 *      None.   
 */
UI32_T IML_MGR_ARP_Inspection_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid, UI32_T packet_length, UI8_T *dst_mac, UI8_T *src_mac,
    UI16_T packet_type,UI32_T src_lport_ifIndex);
#endif
/*void IML_Init (void);
void IML_Create_InterCSC_Relation(void);
BOOL_T IML_CreateTask(void);
void IML_SetTransitionMode(void);
void IML_EnterMasterMode (void);
void IML_EnterSlaveMode (void);
void IML_EnterTransitionMode (void);
void IML_GetManagementVid(UI32_T *vid_ifIndex);
UI32_T IML_SetManagementVid(UI32_T vid_ifIndex);*/


#endif   /* _IML_MGR_H */
