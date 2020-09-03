/*-----------------------------------------------------------------------------
 * Module Name: L2MUX_MGR.H   (LAYER 2 MULTIPLEXING)
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of l2mux_mgr.c
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
 *    07/05/2002 - Benson Hsu, Check DA for multicast(IGMPSNP).
 *    10/11/2002 - Benson Hsu, Merge l2mux_tx.h and l2mux_rx.h to one l2mux_mgr.h.
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef	L2MUX_MGR_H
#define	L2MUX_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "l_mm.h"
#include "sys_adpt.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define L2MUX_MGR_MSGBUF_TYPE_SIZE sizeof(union L2MUX_MGR_IPCMsg_Type_U)


/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in L2MUX_OM_IPCMsg_T.data
 */
#define L2MUX_MGR_GET_MSGBUFSIZE(struct_name) \
        (L2MUX_MGR_MSGBUF_TYPE_SIZE + sizeof(struct struct_name))

/* DATA TYPE DECLARATIONS
 */
typedef void (*L2MUX_MGR_AnnouncePktFunction_T)(L_MM_Mref_Handle_T	*mref_handle_p,
                                                UI8_T 	        dst_mac[6],
                                                UI8_T 	        src_mac[6],
                                                UI16_T 	        tag_info,   /* raw tagged info   */
                                                UI16_T          type,       /* packet type       */
                                                UI32_T          pkt_length, /* pdu length        */
                                                UI32_T 	        lport);     /* source lport ID   */

#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)

#define L2MUX_MGR_RECV_REASON_INTRUDER      0x1
#define L2MUX_MGR_RECV_REASON_STATION_MOVE  0x2

typedef BOOL_T (*L2MUX_MGR_IntrusionCallBackFunction_T)(
        L_MM_Mref_Handle_T *mref_handle_p,
        UI8_T 	    *dst_mac,
        UI8_T       *src_mac,
        UI16_T      tag_info,       /* raw tagged info      */
        UI16_T      ether_type,     /* packet type          */
        UI32_T      pdu_length,     /* pdu length           */
        UI32_T      src_lport,
        UI32_T      reason);        /* why packet trap to CPU */
#endif

enum L2MUX_MGR_PACKET_TYPE_E
{
    L2MUX_MGR_IP_PACKET = 0,
    L2MUX_MGR_DOT1XMAC_PACKET,
    L2MUX_MGR_STA_PACKET,
    L2MUX_MGR_GVRP_PACKET,
    L2MUX_MGR_IGMPSNP_PACKET,
    L2MUX_MGR_CLUSTER_PACKET,
    L2MUX_MGR_LLDP_PACKET,
    L2MUX_MGR_MLDSNP_PACKET,
    L2MUX_MGR_CFM_PACKET,
    L2MUX_MGR_RAPS_PACKET,
    L2MUX_MGR_ERPS_HEALTH_PACKET,
    L2MUX_MGR_PPPOED_PACKET,
    L2MUX_MGR_PTP_PACKET,
    L2MUX_MGR_OF_PACKET,
    L2MUX_MGR_MAX_CLIENT_NO
};

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    L2MUX_TYPE_TRACE_ID_L2MUX_MGR_IP_CALLBACKFUNC = 0,
};

/* definitions of command in L2MUX which will be used in ipc message
 */
enum
{
    L2MUX_MGR_IPC_SEND_BPDU = 0,
    L2MUX_MGR_IPC_SEND_PACKET,
    L2MUX_MGR_IPC_SEND_MULTI_PACKET,
    L2MUX_MGR_IPC_SEND_MULTI_PACKET_BY_VLAN,
    L2MUX_MGR_IPC_SEND_PACKET_PIPELINE,
};

/* The following two types are used by IGMPSNP and should be reviewed by SAs
 * to see if we still need this structure for IGMPSNP, Benson, 07/05/2002
 */
typedef struct L2MUX_MGR_DispatchItem_S
{
	UI32_T	dispatchItem;	/* Protocol dispatch items */
	void	*option;	/* Advance condition of dispatch item */
} L2MUX_MGR_DispatchItem_T;


typedef struct
{
    union L2MUX_MGR_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request */
        UI32_T result; /* for response            */
    } type;

    union
    {
        struct L2MUX_MGR_IPCMsg_SendPacket_Data_S
        {
            I32_T       mref_handle_offset;
            UI32_T      packet_length;
            UI32_T      lport;
            UI32_T      cos_value;
            UI16_T      type;
            UI16_T      tag_info;
            UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN];
            BOOL_T      is_tagged;
            BOOL_T      is_send_to_trunk_members;
        }SendPacket_data;
        struct L2MUX_MGR_IPCMsg_SendMulti_Data_S
        {
            I32_T       mref_handle_offset;
            UI32_T      packet_length;
            UI32_T      cos_value;
            UI16_T      type;
            UI16_T      tag_info;
            UI8_T       dst_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T       src_mac[SYS_ADPT_MAC_ADDR_LEN];
            UI8_T       lport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            UI8_T       untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
            BOOL_T      lport_list_is_valid;
        }SendMulti_data;
    } data;
} L2MUX_MGR_IPCMsg_T;

/* EXPORTED SUBPROGRAM DECLARATION
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
void L2MUX_MGR_InitiateProcessResources(void);


/* FUNCTION NAME: L2MUX_MGR_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: Initializes all resources for L2MUX_MGR
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void L2MUX_MGR_InitiateSystemResources(void);


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: L2MUX_MGR_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for L2MUX in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void L2MUX_MGR_AttachSystemResources(void);


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
void L2MUX_MGR_Create_InterCSC_Relation(void);


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
void L2MUX_MGR_SetTransitionMode(void);


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
void L2MUX_MGR_EnterTransitionMode(void);


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
void L2MUX_MGR_EnterMasterMode(void);


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
void L2MUX_MGR_EnterSlaveMode(void);

/* TX functions on L2MUX_MGR module
 */
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
                          BOOL_T      is_send_to_trunk_members);

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
                          BOOL_T      is_send_to_trunk_members);


/* FUNCTION NAME: L2MUX_MGR_SendPacketPipeline
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure sends packet to ASIC's packet processing pipeline.
 *          (this function will check the STA state, packet will be drop if not in forwarding state)
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
void L2MUX_MGR_SendPacketPipeline(L_MM_Mref_Handle_T *mref_handle_p,
                          UI32_T      packet_length,
                          UI32_T      in_port);

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
 *          cos_value  -- the cos value when sending this packet
 * OUTPUT: none
 * RETURN: none
 *----------------------------------------------------------------------------------------
 * NOTE:
 *  1. This function is used for upper layer to multicast packet.
 */
void L2MUX_MGR_SendMultiPacket(L_MM_Mref_Handle_T *mref_handle_p,
                               UI8_T             dst_mac[6],
                               UI8_T             src_mac[6],
                               UI16_T            type,
                               UI16_T            tag_info,
                               UI32_T            packet_length,
                               UI8_T             *lport_list,
                               UI8_T             *untagged_list,
                               UI32_T            cos_value);

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
    UI32_T cos_value);


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
BOOL_T L2MUX_MGR_ITRI_MIM_SetStatus(UI32_T lport, BOOL_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - L2MUX_MGR_ITRI_MIM_GetStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set status of ITRI MAC-in-MAC
 * INPUT   : lport
 * OUTPUT  : status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T L2MUX_MGR_ITRI_MIM_GetStatus(UI32_T lport, BOOL_T *status_p);
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */


/*------------------------------------------------------------------------------
 * ROUTINE NAME : L2MUX_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T L2MUX_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);


void L2MUX_MGR_DOT1XMAC_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no);

void L2MUX_MGR_STA_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no);

void L2MUX_MGR_CLUSTER_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no);

void L2MUX_MGR_GVRP_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                        UI8_T     *dst_mac,
                                        UI8_T     *src_mac,
                                        UI16_T    tag_info,
                                        UI16_T    type,
                                        UI32_T    pkt_length,
                                        UI32_T    unit_no,
                                        UI32_T    port_no);

void L2MUX_MGR_IP_CallbackFunc(L_MM_Mref_Handle_T  *mref_handle_p,
                                      UI8_T     *dst_mac,
                                      UI8_T     *src_mac,
                                      UI16_T    tag_info,
                                      UI16_T    type,
                                      UI32_T    pkt_length,
                                      UI32_T    unit_no,
                                      UI32_T    port_no);

#if (SYS_CPNT_LLDP == TRUE)
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
                                        UI32_T    port_no);
#endif
#if (SYS_CPNT_CFM == TRUE)
/* FUNCTION NAME: L2MUX_MGR_CFM_CallbackFunc
 *----------------------------------------------------------------------------------
 * PURPOSE: As long as Network Interface received a BPDU packet,it calls
 *          this function to request CFM callback function to handle this
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
void L2MUX_MGR_CFM_CallbackFunc(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     *dst_mac,
                                       UI8_T     *src_mac,
                                       UI16_T    tag_info,
                                       UI16_T    type,
                                       UI32_T    pkt_length,
                                       UI32_T    unit_no,
                                       UI32_T    port_no);
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
    UI32_T              port_no);
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
    UI32_T              packet_class);

#endif /* L2MUX_MGR_H */

