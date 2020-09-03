/* -------------------------------------------------------------------------------------
 * FILE NAME : lan_type.h
 * -------------------------------------------------------------------------------------
 * PURPOSE   : Define the common constant that used in dev_nicdrv_gateway.c, dev_nicdrv.c
 *             and lan.c
 *
 * NOTES      :
 *
 *
 * MODIFICATION HISTORY :
 * Modifier     Date         Version     Description
 * -------------------------------------------------------------------------------------
 * kh_shi       2-9-2005     V1.0        First Created
 *
 * -------------------------------------------------------------------------------------
 * Copyright(C)                         Accton Technology Corp. 2005
 * -------------------------------------------------------------------------------------
 */

#ifndef LAN_TYPE_H
#define LAN_TYPE_H

/* Definition of receive packet reason
 * NOTE: the following definitions is based on BCM SDK 5.2.2
 */
#define  LAN_TYPE_RECV_REASON_INTRUDER          (0x1<<1)      /* SLF */
#define  LAN_TYPE_RECV_REASON_DA_MISS           (0x1<<2)      /* DSF */
#define  LAN_TYPE_RECV_REASON_STATION_MOVE      (0x1<<3)      /* L2 Station movement       */
#define  LAN_TYPE_RECV_REASON_L2CPU             (0x1<<4)      /* L2 CPU */
#define  LAN_TYPE_RECV_REASON_INGRESS_SAMPLED   (0x1<<5)      /* Ingress sampled packtes          */
#define  LAN_TYPE_RECV_REASON_EGRESS_SAMPLED    (0x1<<6)      /* Egress sampled packtes           */
#define  LAN_TYPE_RECV_REASON_L3DST_MISS        (0x1<<8)      /* L3 DIP Miss            */
#define  LAN_TYPE_RECV_REASON_FILTER_MATCH      (0x1<<12)     /* Filter match           */
#define  LAN_TYPE_RECV_REASON_ICMP_REDIRECT     (0x1<<19)     /* Need to send ICMP redirect       */
#define  LAN_TYPE_RECV_REASON_L3_SLOW_PATH      (0x1<<20)     /* L3 slow path (e.g. TTL expire)   */
#define  LAN_TYPE_NOT_PRESENT_STACKING_PORT 0xffffffff  /* no stacking port, any illegal port number is ok */

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    LAN_TYPE_TRACE_ID_LAN_ADM_RECVPACKET = 0,
    LAN_TYPE_TRACE_ID_LAN_BUFFERADJUSTMENT,
    LAN_TYPE_TRACE_ID_LAN_SENDMULTIPACKET,
    LAN_TYPE_TRACE_ID_LAN_SENDPACKETBYVLANID,
    LAN_TYPE_TRACE_ID_LAN_SENDPACKETTOREMOTEUNIT,
    LAN_TYPE_TRACE_ID_LAN_SETOAMLOOPBACKOFREMOTEUNIT,
    LAN_TYPE_TRACE_ID_LAN_SETPORTTPID,
    LAN_TYPE_TRACE_ID_LAN_SETPORTLEARNING,
    LAN_TYPE_TRACE_ID_LAN_SETPORTSECURITY,
    LAN_TYPE_TRACE_ID_LAN_SETPORTDISCARDUNTAGGEDFRAME,
    LAN_TYPE_TRACE_ID_LAN_SETPORTDISCARDTAGGEDFRAME,
    LAN_TYPE_TRACE_ID_LAN_ISCREPLY,
    LAN_TYPE_TRACE_ID_LAN_STACKING_SENDIPPKT,
    LAN_TYPE_TRACE_ID_LAN_STACKING_SENDSTAPKT,
    LAN_TYPE_TRACE_ID_LAN_STACKING_SENDGVRPPKT,
    LAN_TYPE_TRACE_ID_LAN_STACKING_SENDLACPPKT,
    LAN_TYPE_TRACE_ID_LAN_STACKING_SENDMULTIIPPKT,
    LAN_TYPE_TRACE_ID_LAN_SETVLANLEARNINGSTATUS,
};

/* Definitions of packet type for receive packet.
 * These type should be project independent.
 */
typedef enum LAN_TYPE_PacketType_E
{
    LAN_TYPE_LOCAL_NETWORK_PACKET = 0,    /* Packet from local port               */
    LAN_TYPE_REMOTE_NETWORK_PACKET,       /* Packet relay by LAN                  */
    LAN_TYPE_ISC_REPLY_PACKET,            /* Reply packet type of ISC             */
    LAN_TYPE_ISC_INTERNAL_STK_TPLG,       /* Packet of STKTPLG                    */
    LAN_TYPE_ISC_INTERNAL_OTHERS,         /* Packet of ISC but not lan or STKTPLG */
    /* Please add your new types here */
    LAN_TYPE_INVALID_PACKET_TYPE          /* Total number of types                */
} LAN_TYPE_PacketType_T;

/* don't change it or otherwise modify corresponding code in LAN_AnnounceRxPktToUpperLayer
 * i.e. modify array type_to_module_id[] in LAN_AnnounceRxPktToUpperLayer accordingly
 */
typedef enum LAN_TYPE_PacketClass_E
{
    LAN_TYPE_IP_PACKET = 0,
    LAN_TYPE_STA_PACKET,
    LAN_TYPE_GVRP_PACKET,
    LAN_TYPE_LACP_PACKET,
    LAN_TYPE_OAM_PACKET,
    LAN_TYPE_DOT1X_PACKET,
    LAN_TYPE_LLDP_PACKET,
    LAN_TYPE_CLUSTER_PACKET,
    LAN_TYPE_INTERNAL_LOOPBACK_PACKET,
    LAN_TYPE_CFM_PACKET,
    LAN_TYPE_SFLOW_PACKET,
    LAN_TYPE_RAPS_PACKET,
    LAN_TYPE_OAM_LOOPBACK_PACKET,
    LAN_TYPE_ITRI_MIM_PACKET,               /* SYS_CPNT_ITRI_MIM */
    LAN_TYPE_L2PT_PACKET,                   /* SYS_CPNT_QINQ_L2PT */
    LAN_TYPE_ERPS_HEALTH_PACKET,
    LAN_TYPE_PPPOED_PACKET,
    LAN_TYPE_LBD_PACKET,
    LAN_TYPE_UDLD_PACKET,
    LAN_TYPE_PTP_PACKET,
    LAN_TYPE_ESMC_PACKET,
#if(SYS_CPNT_MLAG == TRUE)    
    LAN_TYPE_MLAG_PACKET,
#endif
    LAN_TYPE_UNKNOWN_PACKET,
    LAN_TYPE_NUMBER_OF_PACKET_CLASS
} LAN_TYPE_PacketClass_T;

/* The following defines the ISC packets' forwarding directions on stacking links
 */
typedef enum LAN_TYPE_TXDirection_E
{
    LAN_TYPE_TX_UP_LINK = 1,      /* LAN_TYPE_TX_UP_LINK and LAN_TYPE_TX_DOWN_LINK */
    LAN_TYPE_TX_DOWN_LINK,        /* must be 1 & 2 (bit 1 on and bit 2 on)         */
    LAN_TYPE_TX_UP_DOWN_LINKS,
    LAN_TYPE_TX_MAINBRD_TO_EXPANSION,
    LAN_TYPE_TX_EXPANSION_TO_MAINBRD,
    LAN_TYPE_TX_INVALID_DIRECTION
} LAN_TYPE_TXDirection_T;

#endif

